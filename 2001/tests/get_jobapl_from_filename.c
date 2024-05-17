#include <stdio.h>
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

int main(void) {
	const char* filename = "1236969-big-file.txt";

	int jobapl = get_jobapl_from_filename(filename);

	printf("%s: jobapl = %d\n", filename, jobapl);
}
