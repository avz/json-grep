#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "filter.h"

struct FilterSegment {
	const unsigned char *name;
	size_t nameLen;

	/**
	 * Если тру, то любое значение проходит
	 */
	char isAny;
};

void filter_init(struct Filter *filter) {
	memset(filter, 0, sizeof(*filter));
}

void filter_free(struct Filter *filter) {
	if(filter->path)
		free(filter->path);

	memset(filter, 0, sizeof(*filter));
}

void filter_set(struct Filter *filter, const unsigned char *pathString, size_t pathStringLen, const unsigned char *value, size_t valueLen) {
	size_t i;
	const unsigned char *segmentStart, *cur;
	int segmentOffset = 0;
	int segmentsCount = 0;

	if(!pathStringLen)
		return;

	for(i = 0; i < pathStringLen; i++) {
		if(pathString[i] == '.')
			segmentsCount++;
	}

	segmentsCount++;

	filter->path = (struct FilterSegment *)calloc((size_t)segmentsCount, sizeof(*filter->path));
	filter->pathSize = segmentsCount;
	filter->value = value;
	filter->valueLen = valueLen;

	segmentStart = pathString;

	for(i = 0; i < pathStringLen; i++) {
		cur = pathString + i;

		if(*cur == '.') {
			if(cur - segmentStart == 1 && *segmentStart == '*') {
				filter->path[segmentOffset].isAny = 1;
				printf("segment: *\n");
			} else {
				filter->path[segmentOffset].name = segmentStart;
				filter->path[segmentOffset].nameLen = (size_t)(cur - segmentStart);

				printf("segment: ");
				fwrite(filter->path[segmentOffset].name, filter->path[segmentOffset].nameLen, 1, stdout);
				printf("\n");
			}

			segmentStart = pathString + i + 1;

			segmentOffset++;
		}
	}

	filter->path[segmentOffset].name = segmentStart;
	filter->path[segmentOffset].nameLen = (size_t)(pathString + pathStringLen - segmentStart);

	printf("segment: ");
	fwrite(filter->path[segmentOffset].name, filter->path[segmentOffset].nameLen, 1, stdout);
	printf("\n");
}

void filter_initParserHandlers(struct Filter *filter) {

}
