#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/inotify.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define BUFMAX 1024

volatile sig_atomic_t terminate = 0;
volatile sig_atomic_t dist_files = 0;

void handle_signal(const int signo) {
	/* if new files are detected, a signal should be sent to the parent process */
	if (signo == SIGUSR1) {
		write(STDOUT_FILENO,"received SIGUSR1\n",17);
		dist_files = 1;
	}
	/* to terminate the application, the parent process must handle the SIGINT signal */
	if (signo == SIGINT) {
		write(STDOUT_FILENO,"received SIGINT\n",16);
		terminate = 1;
	}
}

void sigaction_init(struct sigaction* act) {
	/* zeroes sigaction structure */
	memset(act, 0, sizeof(struct sigaction));
	/* block all signals during signal handling */
	if(sigfillset(&act->sa_mask) != 0) {
		die("sigfillset:");
	}
	/* unblock signals */
	sigdelset(&act->sa_mask, SIGUSR1);
	sigdelset(&act->sa_mask, SIGUSR2);
	sigdelset(&act->sa_mask, SIGINT);

	act->sa_handler = handle_signal;
	act->sa_flags = SA_RESTART;
	/* register signal handlers */
	sigaction(SIGUSR1, act, NULL);
	sigaction(SIGUSR2, act, NULL);
	sigaction(SIGINT, act, NULL);
}

void read_config_file(const char *filename, char* input_dir, char* output_dir,
					int* num_workers, int* interval_ms) {
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		die("Error opening file %s:", filename);
		/* all exit(1) after die is just to suppress warning */
		exit(1);
	}

	char line[256];
	char key[128];
	char value[128];

	while (fgets(line, sizeof(line), file) != NULL) {
		if (sscanf(line, "%s = %s", key, value) == 2) {
			if (strcmp(key, "input_dir") == 0) {
				strcpy(input_dir, value);
			} else if (strcmp(key, "output_dir") == 0) {
				strcpy(output_dir, value);
			} else if (strcmp(key, "num_workers") == 0) {
				*num_workers = atoi(value);
			} else if (strcmp(key, "interval_ms") == 0) {
				*interval_ms = atoi(value);
			} else {
				die("Error in configuration file: %s is not a valid option", key);
			}
		}
	}

	/* invalid values */
	if (input_dir == NULL) {
		die("Error in configuration file: input_dir is null");
		exit(1);
	}
	if (output_dir == NULL) {
		die("Error in configuration file: output_dir is null");
		exit(1);
	}
	if (*num_workers <= 0) {
		die("Error in configuration file: num_workers must be > 0");
		exit(1);
	}
	if (*interval_ms <= 0) {
		die("Error in configuration file: interval_ms must be > 0");
		exit(1);
	}

	printf("================================\n");
	printf("Config file read:\n");
	printf("input_dir = %s\n", input_dir);
	printf("output_dir = %s\n", output_dir);
	printf("num_workers = %d\n", *num_workers);
	printf("interval_ms = %d\n", *interval_ms);
	printf("================================\n");

	fclose(file);
}

void monitor_process(const char* input_dir, int interval_ms) {
	/* file/watch descriptor */
	int fd, wd;
	char buf[BUFMAX];

	/* inotify */
	fd = inotify_init();
	if (fd == -1) {
		die("inotify_init:");
	}

	wd = inotify_add_watch(fd, input_dir, IN_CREATE);
	if (wd == -1) {
		die("inotify_add_watch:");
	}

	while(!terminate) {
		write(STDOUT_FILENO, "Monitoring directory for new files...\n", 38);

		/* monitor the directory, send signal to parent if detects a change */
		ssize_t len = read(fd, buf, sizeof(buf));
		if (len == -1) {
			die("read:");
		}

		struct inotify_event *event;

		for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
			event = (struct inotify_event *) ptr;
			if (event->mask & IN_CREATE) {
				printf("New file '%s' detected in directory '%s'\n", event->name, input_dir);
				/* send signal */
				dist_files = 1;
			} else {
				dist_files = 0;
			}
		}
		usleep(interval_ms * 1000);
	}

	close(fd);
	exit(0);
}

int get_jobref_from_file(const char* filename, char* jobref, size_t nbytes) {
	if (filename == NULL || jobref == NULL || nbytes <= 0) {
		return -1;
	}

	const char* patterns[] = {
		"-email.txt",
		"-candidate-data.txt"
	};
	size_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);

	/* if filename matches *-email.txt */

	/* else if filename matches *-candidate-data.txt */

	/* else: try to guess jobref based on output dir, return -1 if fails */
	return -1;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		die("usage: %s [CONFIGFILE]", argv[0]);
	}

	pid_t pid;
	char buf[BUFMAX];
	char input_dir[256];
	char output_dir[256];
	int num_workers = 0;
	int interval_ms = 0;
	char* config_filename = argv[1];

	read_config_file(config_filename, input_dir, output_dir, 
			&num_workers, &interval_ms);

	struct sigaction act;
	sigaction_init(&act);

	int monitor_pipe[2];

	pid = fork();
	if (pid == -1) {
		die("fork:");
	}
	if (pid > 0) {
		/* PARENT */
		close(monitor_pipe[1]);

		/**
		 * 2 pipes for each comunication between parent and a worker
		 * int worker_pipes[2N][2];
		 */
		/* array of pointers */
		int** worker_pipes = (int**)malloc(2 * num_workers * sizeof(int*));
		if (worker_pipes == NULL) {
			die("malloc:");
		}
		/* each pointer points to int fd[2] */
		for(int i = 0; i < 2 * num_workers; i++) {
			worker_pipes[i] = (int*)malloc(2 * sizeof(int));
			if (worker_pipes[i] == NULL) {
				die("malloc:");
			}
		}
		/* create 2N pipes */
		for(int i = 0; i < 2 * num_workers; i++) {
			if(pipe(worker_pipes[i]) == -1) {
				die("pipe:");
			}
		}

		st_workers* ws = st_workers_init(worker_pipes, num_workers);

		/* create N workers */
		for(int i = 0; i < num_workers; i++) {
			pid = fork();
			if (pid == -1) {
				die("fork:");
			}
			if (pid > 0) {
				/* PARENT */
				/* set the worker pids */
				ws->pids[i] = pid;

				close(worker_pipes[i*2][0]);
				close(worker_pipes[i*2+1][1]);

			} else {
				/* WORKER */
				close(monitor_pipe[0]); /* workers dont read from monitor */
				close(worker_pipes[i*2][1]);
				close(worker_pipes[i*2+1][0]);

				while(!terminate) {

				}
				/* exit workers */
				close(worker_pipes[1*2][0]);
				close(worker_pipes[i*2+1][1]);
				exit(0);
			}
		}

		if (pid > 0) {
			/* PARENT */
			while(!terminate) {
				if (dist_files) {
					if (read(monitor_pipe[0], buf, BUFMAX) == -1) {
						die("read from monitor pipe:");
					}
				}
			}
			/* wait for other processes to terminate */
			wait(NULL);

			/* free memory */
			for (int i = 0; i < num_workers; i++) {
				free(worker_pipes[i]);
			}
			
			free(worker_pipes);

			/* exit normally */
			exit(0);
		}
	} else {
		/* MONITOR */
		close(monitor_pipe[0]);
		monitor_process(input_dir, interval_ms);
	}

	die("Error: filebot exited abnormally");
}
