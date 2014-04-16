#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "parser.h"
#include "filter.h"

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

	unsigned char buf[32 * 1024];
	unsigned char *bufEnd = buf + sizeof(buf);
	unsigned char *bufHead = buf;
	unsigned char *currentLineStart = buf;
	ssize_t readed;

	while((readed = read(STDIN_FILENO, bufHead, (size_t)(bufEnd - bufHead)))) {
		unsigned char *pos = bufHead;
		unsigned char *chunkEnd = bufHead + readed;

		while((pos = memchr(pos, '\n', (size_t)(chunkEnd - pos)))) {
			if(filter_test(&f, currentLineStart, (size_t)(pos - currentLineStart)))
				fwrite(currentLineStart, (size_t)(pos - currentLineStart + 1), 1, stdout);

			pos++;

			currentLineStart = pos;
		}

		bufHead += readed;

		if(bufHead == bufEnd) {
			if(currentLineStart == buf) {
				/* overflow */
				bufHead = buf;
				currentLineStart = buf;
			} else {
				memmove(buf, currentLineStart, (size_t)(bufEnd - currentLineStart));
				bufHead = buf + (bufEnd - currentLineStart);
				currentLineStart = buf;
			}
		}
	}

	filter_free(&f);

	return EXIT_SUCCESS;
}
