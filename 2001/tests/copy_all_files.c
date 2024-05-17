#include <linux/limits.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

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

		printf("(DEBUG) %s\n", entry->d_name);

		char input_dir_file[512];
		snprintf(input_dir_file, sizeof(input_dir_file), "%s/%s",
				input_dir, entry->d_name);
		char output_dir_jobref_jobapl_file[2048];
		snprintf(output_dir_jobref_jobapl_file, sizeof(output_dir_jobref_jobapl_file), "%s/%s",
				output_dir_jobref_jobapl, entry->d_name);

		printf("(DEBUG) %s\n %s\n", input_dir_file, output_dir_jobref_jobapl_file);
		
		/* use rename to move files, exec only works for one at a time */
		if (rename(input_dir_file, output_dir_jobref_jobapl_file) == -1) {
			perror("rename");
			fprintf(stderr, "Error: Failed to move '%s' to '%s'\n", input_dir_file, output_dir_jobref_jobapl_file);
			/* die("rename %s:"); */
			closedir(dir);
			return -1;
		}
	}

	closedir(dir);
	return 0;
}

int main(void) {
	//char input_dir[] = "Email-Bot-Output-Example/";
	//char output_dir[] = "test_out/IBM-000123/";
	
	char input_dir[] = "Email-Bot-Output-Example";
	char output_dir[] = "out";
	char jobref[] = "IBM-000123";
	int jobapl = 2;

	return copy_all_files(input_dir, output_dir, jobref, jobapl);
}
