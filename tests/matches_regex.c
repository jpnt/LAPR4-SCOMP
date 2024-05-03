#include <stdio.h>
#include <regex.h>

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

int main(void) {
	const char* str = "1-candidate-data.txt";
	const char* regex = "^1-";

	if (matches_regex(str, regex) == 1) {
		printf("%s matches '%s'\n", str, regex);
	}
	else {
		printf("%s does not match '%s'\n", str, regex);
	}

	return 0;
}
