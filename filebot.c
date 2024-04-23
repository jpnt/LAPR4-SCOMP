#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/inotify.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

volatile sig_atomic_t time_to_go;

void read_config_file(const char *filename, char* input_dir, char* output_dir, int* num_workers, int* interval_sec) {
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		die("Error opening file %s:", filename);
		/* just to suppress a useless warning */
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
			} else if (strcmp(key, "interval_sec") == 0) {
				*interval_sec = atoi(value);
			} else {
				die("Error in configuration file: %s is not a valid option", key);
			}
		}
	}

	/* invalid values */
	if (input_dir == NULL) {
		die("Error in configuration file: input_dir is null");
	}
	if (output_dir == NULL) {
		die("Error in configuration file: output_dir is null");
	}
	if (*num_workers <= 0) {
		die("Error in configuration file: num_workers must be > 0");
	}
	if (*interval_sec <= 0) {
		die("Error in configuration file: interval_sec must be > 0");
	}

	/* print values, for debugging */
	printf("=========================\n");
	printf("Config file read:\n");
	printf("input_dir = %s\n", input_dir);
	printf("output_dir = %s\n", output_dir);
	printf("num_workers = %d\n", *num_workers);
	printf("interval_sec = %d\n", *interval_sec);
	printf("=========================\n");

	fclose(file);
}

pid_t create_monitor_process(int* is_parent, int* is_monitor) {
	pid_t pid;

	if(!*is_parent) {
		die("create_monitor_process: Only the parent can enter");
	}

	pid = fork();
	if(pid == -1) {
		die("fork:");
	}
	if (pid == 0) {
		/* child */
		*is_parent = 0;
		*is_monitor = 1;
		printf("Monitor process created (PID: %d)\n", getpid());
		/* always return pid of monitor */
		return getpid();
	}
	else {
		/* parent */
		wait(NULL);
		printf("Parent process: Monitor Process created with pid %d\n", pid);
		return pid;
	}
}

pid_t create_worker_process(int idx, int* is_parent, int* is_worker) {
	pid_t pid;

	if (*is_parent) {
		pid = fork();
		if (pid == -1) {
			die("fork:");
		}
		if (pid == 0) {
			/* worker */
			*is_parent = 0;
			*is_worker = 1;
			printf("Worker process %d created (PID: %d)\n", idx, getpid());
			/* always return pid of worker */
			return getpid();
		}
		else {
			/* parent */
			return pid;
		}
	}
	
	/* previous worker wont know pid of next workers */
	/* also, monitor wont have info about any pid of workers */
	return 0;
}

void create_worker_processes(int num_workers, int* is_parent, int* is_worker) {
	pid_t pid;

	for(int i = 0; i < num_workers; i++) {
		if (*is_parent) {
			pid = fork();
			if (pid == -1) {
				die("fork:");
			}
			if (pid == 0) {
				/* child */
				*is_parent = 0;
				*is_worker = 1;
				printf("Worker process %d created (PID: %d)\n", i, getpid());
			}
			else {
				/* parent */
				/* exit(0); */
			}
		}
	}
}

void monitor_input_dir(const char* input_dir, int interval_sec, int is_monitor) {
	/* who monitors? monitor */
	if (!is_monitor) {
		return;
	}

	/* file/watch descriptor */
	int fd, wd;
	char buf[1024];

	/* inotify */
	fd = inotify_init();
	if (fd == -1) {
		die("inotify_init:");
	}

	wd = inotify_add_watch(fd, input_dir, IN_CREATE);
	if (wd == -1) {
		die("inotify_add_watch:");
	}

	while(!time_to_go) {
		printf("Monitoring directory %s for new files...\n", input_dir);
		/* write(STDOUT_FILENO, "Monitoring directory for new files...\n", 38); */

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
			}
		}

		sleep(interval_sec);
	}

	close(fd);
}

void handle_signal(const int signo) {
	/* upon receiving a signal, the parent process should distribute the new files */
	if (signo == SIGUSR1) {
		write(STDOUT_FILENO,"Parent process: received SIGUSR1\n",33);
	}
	/* once a child has finished copying all files for a candidate, it should inform its parent */
	if (signo == SIGUSR2) {
		write(STDOUT_FILENO,"Parent process: received SIGUSR1\n",33);
	}
	/* to terminate the application, the parent process must handle the SIGINT signal */
	if (signo == SIGINT) {
		write(STDOUT_FILENO,"Parent process: received SIGINT\n",32);
		time_to_go = 1;
	}
}

void distribute_workers(pid_t* pid_workers);

void copy_files(const char* input_dir, const char* output_dir, int appl_num);

void generate_report_file(const char* output_dir);


int main() {
	struct sigaction act;
	pid_t pid_monitor;
	pid_t* pid_workers;
	/* int status; */
	/* flags */
	int is_parent = 1, is_monitor = 0, is_worker = 0;
	time_to_go = 0;
	/* configuration variables */
	char input_dir[256], output_dir[256];
	int num_workers = 0, interval_sec = 0;

	read_config_file("filebot.conf", input_dir, output_dir, &num_workers, &interval_sec);

	/* dynamic array */
	pid_workers = (pid_t*)calloc(num_workers, sizeof(pid_t));
	if (pid_workers == NULL) {
		die("calloc:");
	}

	/* zeroes sigaction structure */
	memset(&act, 0, sizeof(struct sigaction));
	/* block all signals during signal handling */
	if(sigfillset(&act.sa_mask) != 0) {
		die("sigfillset:");
	}
	/* unblock signals */
	sigdelset(&act.sa_mask, SIGUSR1);
	sigdelset(&act.sa_mask, SIGUSR2);
	sigdelset(&act.sa_mask, SIGINT);

	/* act.sa_handler = handle_signal; */
	act.sa_flags = SA_RESTART;
	/* register signal handlers */
	sigaction(SIGUSR1, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);
	sigaction(SIGINT, &act, NULL);

	pid_monitor = create_monitor_process(&is_parent, &is_monitor);

	for (int i = 0; i < num_workers; i++) {
		pid_workers[i] = create_worker_process(i, &is_parent, &is_worker);
	}

	monitor_input_dir(input_dir, interval_sec, is_monitor);

	/* terminate the application TODO */
	if (is_parent) {
		kill(pid_monitor, SIGINT);
		waitpid(pid_monitor, NULL, 0);

		for (int i = 0; i < num_workers; i++) {
			kill(pid_workers[i], SIGINT);
		}

		for (int i = 0; i < num_workers; i++) {
			waitpid(pid_workers[i], NULL, 0);
		}
	}
	
	return 0;
}
