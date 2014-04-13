#ifndef FILTER_H
#define	FILTER_H

#include <sys/types.h>

#include "parser.h"

struct FilterSegment;

struct Filter {
	struct FilterSegment *path;
	int pathSize;

	const unsigned char *value;
	size_t valueLen;


};

void filter_init(struct Filter *filter);
void filter_free(struct Filter *filter);
void filter_set(struct Filter *filter, const unsigned char *pathString, size_t pathStringLen, const unsigned char *value, size_t valueLen);
void filter_initParserHandlers(struct Filter *filter);

#endif	/* FILTER_H */
