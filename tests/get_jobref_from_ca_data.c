#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <stdlib.h>
#include <string.h>

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

int main(void) {
	char jobref[256];

	get_jobref_from_ca_data(jobref, sizeof(jobref), 
			"./Email-Bot-Output-Example/1-candidate-data.txt");

	printf("%s\n", jobref);

	return 0;
}
