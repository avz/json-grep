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
int inListOrHash = 0;

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

int onHashStart(void *a) {
	makeIndent();

	printf("{\n");

	indent++;

	return 0;
}

int onHashKey(void *a, const unsigned char *string, size_t len) {
	makeIndent();

	printf("\"");
	fwrite(string, len, 1, stdout);
	printf("\": ");

	return 0;
}

int onHashEnd(void *a) {
	makeIndent();

	printf("}\n");

	indent--;

	return 0;
}

int main(int argc, char** argv) {
	struct Filter f;
	const unsigned char filter[] = "hello.world.*.aaa";
	const unsigned char value[] = "foobar";

	filter_init(&f);
	filter_set(&f, filter, sizeof(filter) - 1, value, sizeof(value) - 1);
	filter_free(&f);

	/*
	struct Parser parser;
	unsigned char buf[8 * 1024];
	ssize_t bufStart = 0;
	ssize_t bufEnd = 0;
	ssize_t readed;
	ssize_t i;
	int err;

	int number = 0;

	parser_init(&parser);

	parser.handlers.onBool = onBool;
	parser.handlers.onNull = onNull;
	parser.handlers.onString = onString;
	parser.handlers.onNumber = onNumber;
	parser.handlers.onListStart = onListStart;
	parser.handlers.onListEnd = onListEnd;
	parser.handlers.onHashStart = onHashStart;
	parser.handlers.onHashEnd = onHashEnd;
	parser.handlers.onHashKey = onHashKey;

	while((readed = read(STDIN_FILENO, buf + bufEnd, sizeof(buf) - (size_t)bufEnd)) > 0) {
		for(i = 0; i < readed; i++) {
			if(buf[bufEnd] == '\n') {
				ssize_t len = bufEnd - bufStart;

				err = parser_parse(&parser, buf + bufStart, (size_t)len);

				if(err) {
					fprintf(stderr, "Error parsing JSON: %d\n", err);
					exit(1);
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

	printf("%d\n", number);*/
	return EXIT_SUCCESS;
}
