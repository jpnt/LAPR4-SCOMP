#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define BUFMAX 512

volatile sig_atomic_t terminate = 0;
volatile sig_atomic_t distfiles = 0;

void handle_signal(const int signo) {
	/* if new files are detected, a signal should be sent to the parent process */
	if (signo == SIGUSR1) {
		write(STDOUT_FILENO,"New files detected\n", 19);
		distfiles = 1;
	}
	/* to terminate the application, the parent process must handle the SIGINT signal */
	if (signo == SIGINT) {
		write(STDOUT_FILENO,"Received SIGINT, terminating...\n",32);
		terminate = 1;
	}
}

void sigaction_setup(struct sigaction* act) {
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

int get_jobapl_from_filename(const char* filename) {
	int num_apl = 0;
	int base = 1;
	/* number before '-' */
	for (int i = 0; filename[i] != '-' && filename[i] != '\0'; i++) {
		if (filename[i] < '0' || filename[i] > '9') {
			return -1;
		}
		num_apl = num_apl * base + (filename[i] - '0');
		base = 10;
	}
	return num_apl;
}

int get_jobref_from_ca_data(char* jobref, size_t nbytes, char* ca_data) {
	FILE* fp = fopen(ca_data, "r");
	if (fp == NULL) {
		return -1;
	}
	if (fgets(jobref, nbytes, fp) == NULL) {
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

/* cp input_dir/jpbapl-* output_dir/jobref/Application_jobapl */
int copy_all_files(const char* input_dir, const char* output_dir,
		const char* jobref, int jobapl) {

	if (mkdir_if_need(output_dir) == -1) {
		return -1;
	}

	char output_dir_jobref[512];
	snprintf(output_dir_jobref, sizeof(output_dir_jobref), "%s/%s",
			output_dir, jobref);
	if (mkdir_if_need(output_dir_jobref) == -1) {
		return -1;
	}

	char output_dir_jobref_jobapl[1024];
	snprintf(output_dir_jobref_jobapl, sizeof(output_dir_jobref_jobapl), "%s/Application_%d",
			output_dir_jobref, jobapl);

	if (mkdir_if_need(output_dir_jobref_jobapl) == -1) {
		return -1;
	}

	DIR* dir = opendir(input_dir);
	if (!dir) {
		perror("opendir");
		return -1;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			/* skip "." and ".." */
			continue;
		}

		char prefix[512];
		snprintf(prefix, sizeof(prefix), "^%d-", jobapl);
		if (matches_regex(entry->d_name, prefix) != 1) {
			continue;
		}

		char input_dir_file[512];
		snprintf(input_dir_file, sizeof(input_dir_file), "%s/%s",
				input_dir, entry->d_name);
		char output_dir_jobref_jobapl_file[2048];
		snprintf(output_dir_jobref_jobapl_file, sizeof(output_dir_jobref_jobapl_file), "%s/%s",
				output_dir_jobref_jobapl, entry->d_name);

		
		/* use rename to move files, exec only works for one at a time */
		if (rename(input_dir_file, output_dir_jobref_jobapl_file) == -1) {
			perror("rename");
			fprintf(stderr, "Error: Failed to move '%s' to '%s'\n", 
					input_dir_file, output_dir_jobref_jobapl_file);
			/* die("rename %s:"); */
			closedir(dir);
			return -1;
		}

		write(STDOUT_FILENO, strcat(output_dir_jobref_jobapl_file, "\n"),
				strlen(output_dir_jobref_jobapl_file)+1);
	}

	closedir(dir);
	return 0;
}

int create_monitor() {
	pid_t pid;
	pid = fork();
	if (pid == 0) {
		write(STDOUT_FILENO, "Monitor process created\n", 24);
	}
	return pid;
}

int create_workers(int num_workers, st_workers* ws) {
	pid_t pid = -1;
	/* create N workers */
	for (int i = 0; i < num_workers; i++) {
		pid = fork();
		if (pid == -1) {
			return pid;
		}
		else if (pid == 0) {
			close(ws->worker_pipes[i*2][1]);
			close(ws->worker_pipes[i*2+1][0]);
			return pid;
		}
		else {
			/* set worker pid in st_workers */
			ws->pids[i] = pid;
			close(ws->worker_pipes[i*2][0]);
			close(ws->worker_pipes[i*2+1][1]);
			return pid;
		}
	}

	return pid;
}

void parent_process(st_workers* ws, int num_workers, pid_t pid_monitor, int parent_fd[2]) {
	int apl;
	
	while(!terminate) {
		if (distfiles) {
			ssize_t n_bytes = read(parent_fd[0], &apl, sizeof(int));
			if (n_bytes == -1) {
				perror("read");
			}

			char ca_data[256];
			snprintf(ca_data, sizeof(ca_data), "%d-candidate-data.txt",
					apl);


			

		}
	}

	cleanup(ws, num_workers, pid_monitor, parent_fd);
	exit(0);
}

void monitor_process(const char* input_dir, int interval_ms, int parent_fd[2]) {
	/* close read parent fd */
	close(parent_fd[0]);
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
		die("inotify_add_watch: %s:", input_dir);
	}

	write(STDOUT_FILENO, "Monitoring directory for new files...\n", 38);

	while(!terminate) {
		/* monitor the directory, send signal to parent if detects a change */
		ssize_t len = read(fd, buf, sizeof(buf));
		if (len == -1) {
			die("read:");
		}

		struct inotify_event *event;
		for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
			event = (struct inotify_event *) ptr;
			if (event->mask & IN_CREATE) {
				/* send application_id trough the pipe */
				int jobapl = get_jobapl_from_filename(event->name);

				ssize_t n_bytes = write(parent_fd[1], &jobapl, sizeof(int));
				if (n_bytes == -1) {
					perror("write");
				}
				/* send signal */
				kill(SIGUSR1, getppid());
			}
		}
		usleep(interval_ms * 1000);
	}

	close(parent_fd[1]);
	close(fd);
	exit(0);
}

void worker_process(char* input_dir, char* output_dir, st_workers* ws, int num_workers) {
	char buf[PIPE_BUF];
	while(!terminate) {
		for (int i = 0; i < num_workers; i++) {
			if (ws->pids[i] == getpid()) {
				/* the pipe will have the jobapl */
				if (read(ws->worker_pipes[i*2][0], buf, sizeof(buf)) == -1) {
					perror("read");
				}
				int jobapl = atoi(buf);

				printf("(DEBUG) jobapl = %d", jobapl);

				/* get job reference from x-candidate-data.txt */
				char jobref[128];
				char ca_data[128];
				snprintf(ca_data, sizeof(ca_data), "%d-candidate-data.txt", jobapl);

				int jr = get_jobref_from_ca_data(jobref, sizeof(jobref), ca_data);
				if (jr == -1) {
					fprintf(stderr, "Error: failed to get jobref from %d-candidate-data.txt\n", jobapl);
				}
				
				if (copy_all_files(input_dir, output_dir, jobref, jobapl) == -1) {
					fprintf(stderr, "Error: failed to copy all files\n");
				}
			}
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 2) {
		die("usage: %s [CONFIGFILE]", argv[0]);
	}

	int parent_fd[2];
	st_workers* ws = NULL;
	pid_t pid_monitor, pid;
	char input_dir[BUFMAX];
	char output_dir[BUFMAX];
	int num_workers = 0;
	int interval_ms = 0;

	/* read config file and validate files */
	read_config_file(argv[1], input_dir, output_dir, 
			&num_workers, &interval_ms);

	struct sigaction act;
	sigaction_setup(&act);

	pid_monitor = create_monitor();
	if (pid_monitor == -1) {
		die("create_monitor: fork:");
	}
	else if (pid_monitor == 0) {
		/* MONITOR */
		monitor_process(input_dir, interval_ms, parent_fd);
	}
	else {
		/* PARENT */
		ws = st_workers_create(num_workers);
		pid = create_workers(num_workers, ws);
		if (pid == -1) {
			cleanup(ws, num_workers, pid_monitor, parent_fd);
		}
		else if (pid > 0) {
			/* PARENT */
			parent_process(ws, num_workers, pid_monitor, parent_fd);
		}
		else {
			/* WORKERS */
			worker_process(input_dir, output_dir, ws, num_workers);
		}
	}

	die("Filebot exited abnormally");
}
