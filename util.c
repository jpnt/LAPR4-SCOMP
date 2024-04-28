#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

st_workers* st_workers_init(int** worker_pipes, int num_workers) {
	st_workers* ws = (st_workers*)malloc(sizeof(st_workers));
	if (ws == NULL) {
		die("malloc:");
	}

	ws->worker_pipes = worker_pipes;
	if (ws->worker_pipes == NULL) {
		die("worker_pipes: cannot be NULL");
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
