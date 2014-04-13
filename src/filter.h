#ifndef FILTER_H
#define	FILTER_H

#include <sys/types.h>

#include "parser.h"

struct FilterSegment;

struct Filter {
	struct Parser parser;
	struct FilterSegment *path;
	int pathSize;

	char anyValue;

	const unsigned char *value;
	size_t valueLen;

	int currentPathLevel;
	int lastMatchedPathLevel;

	char currentItemMatched;
};

void filter_init(struct Filter *filter);
void filter_free(struct Filter *filter);
void filter_equals(struct Filter *filter, const unsigned char *pathString, size_t pathStringLen, const unsigned char *value, size_t valueLen);
void filter_exists(struct Filter *filter, const unsigned char *pathString, size_t pathStringLen);

char filter_test(struct Filter *filter, unsigned char *buf, size_t len);

#endif	/* FILTER_H */
