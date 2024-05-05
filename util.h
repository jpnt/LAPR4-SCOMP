#ifndef UTIL_H
#define UTIL_H

#include <signal.h>
#include <stddef.h>

/* structure for FIFO */
typedef struct {
	char** items;
	size_t size;
	size_t capacity;
} Vec;


/* structure for managing worker info */
typedef struct {
	int** worker_pipes;	/* int fd[2N][2] */
	pid_t* pids;		/* pid_t pid[N] */
	int* ready; 		/* int ready[N] */
} st_workers;


Vec* vec_create(size_t capacity);
void vec_destroy(Vec* vec);
void vec_grow(Vec* vec);
void vec_insert(Vec* vec, size_t item_idx, char* item);
char* vec_remove(Vec* vec, size_t item_idx);
void vec_push(Vec* vec, char* item);
char* vec_pop(Vec* vec);

st_workers* st_workers_create(int num_workers);
void st_workers_destroy(st_workers* ws, int num_workers);

int dir_exists(const char* dir);
int mkdir_if_need(const char* dir);
int matches_regex(const char* str, const char* regex_pattern);
int generate_report_file(const char* output_dir);

void cleanup(st_workers* ws, int num_workers, pid_t pid_monitor);
void die(const char *fmt, ...);

#endif /* !UTIL_H */
