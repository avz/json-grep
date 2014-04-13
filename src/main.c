#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "parser.h"
#include "filter.h"


int indent = 0;
int isFirstItem = 0;
int inListOrMap = 0;

void makeIndent() {
	int i;

	for(i = 0; i < indent; i++)
		printf("\t");
}

int onBool(void *a, char bool) {
	makeIndent();

	if(bool)
		printf("true\n");
	else
		printf("false\n");

	return 0;
}

int onNull(void *a) {
	makeIndent();

	printf("null\n");

	return 0;
}

int onNumber(void *a, const unsigned char *number, size_t len) {
	makeIndent();

	fwrite(number, len, 1, stdout);
	printf("\n");

	return 0;
}

int onString(void *a, const unsigned char *string, size_t len) {
	makeIndent();

	printf("\"");
	fwrite(string, len, 1, stdout);
	printf("\"\n");

	return 0;
}

int onListStart(void *a) {
	makeIndent();

	printf("[\n");

	indent++;

	return 0;
}

int onListEnd(void *a) {
	makeIndent();

	printf("]\n");

	indent--;

	return 0;
}

int onMapStart(void *a) {
	makeIndent();

	printf("{\n");

	indent++;

	return 0;
}

int onMapKey(void *a, const unsigned char *string, size_t len) {
	makeIndent();

	printf("\"");
	fwrite(string, len, 1, stdout);
	printf("\": ");

	return 0;
}

int onMapEnd(void *a) {
	makeIndent();

	printf("}\n");

	indent--;

	return 0;
}

int main(int argc, char** argv) {
	struct Filter f;
	size_t ci;
	char hasEqualsFilter = 0;

	if(argc != 2)
		exit(255);

	filter_init(&f);

	for(ci = 0; ci < strlen(argv[1]); ci++) {
		char c = argv[1][ci];

		if(c == '=') {
			filter_equals(&f, (unsigned char *)argv[1], ci, (unsigned char *)argv[1] + ci + 1, strlen(argv[1]) - ci - 1);
			hasEqualsFilter = 1;
			break;
		}
	}

	if(!hasEqualsFilter)
		filter_exists(&f, (unsigned char *)argv[1], strlen(argv[1]));

	unsigned char buf[8 * 1024];
	ssize_t bufStart = 0;
	ssize_t bufEnd = 0;
	ssize_t readed;
	ssize_t i;
	int matched;

	int number = 0;

	while((readed = read(STDIN_FILENO, buf + bufEnd, sizeof(buf) - (size_t)bufEnd)) > 0) {
		for(i = 0; i < readed; i++) {
			if(buf[bufEnd] == '\n') {
				ssize_t len = bufEnd - bufStart;

				matched = filter_test(&f, buf + bufStart, (size_t)len);

				if(matched) {
					fwrite(buf + bufStart, (size_t)len, 1, stdout);
					printf("\n");
				}

				bufStart += len + 1;
				number++;
			}

			bufEnd++;
		}

		if(bufEnd == sizeof(buf)) {
			if(bufStart < bufEnd) {
				memmove(buf, buf + bufStart, (size_t)(bufEnd - bufStart));
				bufEnd -= bufStart;
			} else {
				bufEnd = 0;
			}

			bufStart = 0;
		}
	}

	filter_free(&f);

	return EXIT_SUCCESS;
}
