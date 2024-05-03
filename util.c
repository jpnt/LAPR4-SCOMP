#include <regex.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

/**
 * NOTE: Using Vec as a FIFO
 *
 * vec_push(vec, str1);
 * vec_push(vec, str2);
 *
 * vec_remove(vec, 0); will return str1
 * vec_remove(vec, 0); will return str2
 */
Vec* vec_create(size_t capacity) {
	Vec* vec = (Vec*) malloc(sizeof(Vec));
	if (vec == NULL) {
		die("malloc:");
	}

	vec->items = (char**) calloc(capacity, sizeof(char*));
	if (vec->items == NULL) {
		free(vec);
		die("calloc:");
	}

	vec->size = 0;
	vec->capacity = capacity;
	return vec;
}

void vec_destroy(Vec* vec) {
	if (vec != NULL) {
		free(vec->items);
		free(vec);
	}
}

void vec_grow(Vec* vec) {
	size_t new_capacity = vec->capacity * 2;
	char** new_items = realloc(vec->items, new_capacity * sizeof(char*));
	if (new_items == NULL) {
		die("realloc:");
	}

	vec->items = new_items;
	vec->capacity = new_capacity;
}

void vec_insert(Vec* vec, size_t item_idx, char* item) {
	while (item_idx >= vec->capacity) {
		vec_grow(vec);
	}

	vec->items[item_idx] = item;

	vec->size++;
}

char* vec_remove(Vec* vec, size_t item_idx) {
	if (item_idx >= vec->capacity) {
		return NULL;
	}

	char* removed = vec->items[item_idx];
	vec->items[item_idx] = NULL;

	// Move items left after removing
	size_t i;
	size_t capacity = vec->capacity;
	size_t limit = capacity - 1;
	for (i = item_idx; i < limit; i+=2) {
		vec->items[i] = vec->items[i+1];
		vec->items[i+1] = vec->items[i+2];
	}
	for (; i < capacity; i++) {
		vec->items[i] = vec->items[i+1];
	}

	vec->size--;

	return removed;
}

void vec_push(Vec* vec, char* item) {
	while (vec->size >= vec->capacity) {
		vec_grow(vec);
	}

	vec->items[vec->size] = item;
	vec->size+=1;
}

char* vec_pop(Vec* vec) {
	if (vec->size == 0) {
		return NULL;
	}

	char* popped = vec->items[vec->size-1];
	vec->items[vec->size-1] = NULL;
	vec->size-=1;

	return popped;
}

st_workers* st_workers_create(int num_workers) {
	st_workers* ws = (st_workers*)malloc(sizeof(st_workers));
	if (ws == NULL) {
		die("malloc:");
	}

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

	ws->worker_pipes = worker_pipes;
	if (ws->worker_pipes == NULL) {
		die("st_workers->worker_pipes: cannot be NULL");
	}

	/* does not fill pids, done later after the creation of workers */
	ws->pids = (pid_t*)malloc(num_workers * sizeof(pid_t));
	if (ws->pids == NULL) {
		die("malloc:");
	}

	long size = num_workers * sizeof(int);
	ws->ready = (int*)malloc(size);
	if (ws->ready == NULL) {
		die("malloc:");
	}
	/* set all workers to ready state */
	memset(ws->ready, 1, size);

	return ws;
}

void st_workers_destroy(st_workers* ws, int num_workers) {
	for (int i = 0; i < num_workers; i++) {
		free(ws->worker_pipes[i*2]); /* parent fd[1] --> worker fd[0] */
		free(ws->worker_pipes[i*2+1]); /* parent fd[0] <-- worker fd[1] */
	}
	free(ws->worker_pipes);
	free(ws->pids);
	free(ws->ready);
}

/* Check if a directory exists; returns 1 if it does, 0 if not */
int dir_exists(const char* dir) {
	struct stat statbuf;
	return stat(dir, &statbuf) == 0 && S_ISDIR(statbuf.st_mode);
}

int mkdir_if_need(const char* dir) {
	if (!dir_exists(dir)) {
		if (mkdir(dir, 0755) == -1) {
			perror("mkdir");
			return -1;
		}
	}
	return 0;
}


int matches_regex(const char* str, const char* regex_pattern) {
	regex_t reg;
	int err;

	err = regcomp(&reg, regex_pattern, REG_EXTENDED);
	if (err != 0) {
		char error_buffer[1024];
		regerror(err, &reg, error_buffer, sizeof(error_buffer));
		fprintf(stderr, "Regex compilation error: %s\n",
				error_buffer);
		return -1;
	}

	err = regexec(&reg, str, 0, NULL, 0);

	regfree(&reg);

	/* 1: match found, 0: no match */
	return err == 0 ? 1 : 0;
}


void cleanup(st_workers* ws, int num_workers, pid_t pid_monitor, int parent_fd[2]) {
	for (int i = 0; i < num_workers; i++) {
		close(ws->worker_pipes[i*2][1]);
		close(ws->worker_pipes[i*2+1][0]);
		kill(ws->pids[i], SIGKILL);
		waitpid(ws->pids[i], NULL, 0);
	}

	kill(pid_monitor, SIGKILL);
	waitpid(pid_monitor, NULL, 0);

	close(parent_fd[0]);

	die("cleanup:");
}

void die(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}
