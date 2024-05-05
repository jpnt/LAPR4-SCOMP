#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int generate_report_file(const char* output_dir) {
	int fd = open("report.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1) {
		perror("generate_report_file: open");
		return -1;
	}

	dup2(fd, STDOUT_FILENO);
	close(fd);
	execv("/usr/bin/tree", (char*[]){"tree", (char*)output_dir, NULL});
	perror("generate_report_file: execv");
	return -1;
}

int main(void) {
	char out[] = "out/";
	generate_report_file(out);
}
