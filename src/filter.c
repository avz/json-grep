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


static int onScopeStart(void *filterPtr) {
	struct Filter *filter = (struct Filter *)filterPtr;
	if(filter->currentItemMatched)
		return 0;

	if(filter->currentPathLevel == filter->lastMatchedPathLevel) {
		if(filter->lastMatchedPathLevel + 1 < filter->pathSize) {
			if(filter->path[filter->lastMatchedPathLevel + 1].isAny)
				filter->lastMatchedPathLevel++;
		}
	}

	filter->currentPathLevel++;

	return 0;
}

static int onMapKey(void *filterPtr, const unsigned char *key, size_t len) {
	struct Filter *filter = (struct Filter *)filterPtr;
	struct FilterSegment *s;

	if(filter->currentItemMatched)
		return 0;

	/* матчить не нужно т.к. не заматчился какой-то из предыдущих уровней */
	if(filter->lastMatchedPathLevel + 1 != filter->currentPathLevel)
		return 0;

	if(filter->lastMatchedPathLevel + 1 >= filter->pathSize)
		return 0;

	s = &filter->path[filter->lastMatchedPathLevel + 1];

	if(s->isAny || (s->nameLen == len && memcmp(s->name, key, len) == 0)) {
		/* property name matched */
		filter->lastMatchedPathLevel++;

		if(filter->lastMatchedPathLevel + 1 == filter->pathSize && filter->anyValue)
			filter->currentItemMatched = 1;
	}

	return 0;
}

static int onScalar(void *filterPtr, const unsigned char *string, size_t len) {
	struct Filter *filter = (struct Filter *)filterPtr;

	if(filter->currentItemMatched)
		return 0;

	if(filter->lastMatchedPathLevel != filter->currentPathLevel)
		return 0;

	if(filter->lastMatchedPathLevel != filter->pathSize - 1)
		return 0;

	if(filter->anyValue || filter->valueIsNull)
		filter->currentItemMatched = 1;

	return 0;
}

static int onBool(void *filterPtr, char b) {
	struct Filter *filter = (struct Filter *)filterPtr;

	if(filter->currentItemMatched)
		return 0;

	if(filter->lastMatchedPathLevel != filter->currentPathLevel)
		return 0;

	if(filter->lastMatchedPathLevel != filter->pathSize - 1)
		return 0;

	if(filter->anyValue || (b && filter->valueIsTrue) || (!b && filter->valueIsFalse))
		filter->currentItemMatched = 1;

	return 0;
}

static int onNull(void *filterPtr) {
	struct Filter *filter = (struct Filter *)filterPtr;

	if(filter->currentItemMatched)
		return 0;

	if(filter->lastMatchedPathLevel != filter->currentPathLevel)
		return 0;

	if(filter->lastMatchedPathLevel != filter->pathSize - 1)
		return 0;

	if(filter->anyValue || (filter->valueLen == 4 && memcmp(filter->value, "null", 4) == 0))
		filter->currentItemMatched = 1;

	return 0;
}

static int onScopeEnd(void *filterPtr) {
	struct Filter *filter = (struct Filter *)filterPtr;

	if(filter->currentItemMatched)
		return 0;

	filter->currentPathLevel--;
	if(filter->lastMatchedPathLevel > filter->currentPathLevel) {
		if(filter->currentPathLevel)
			filter->lastMatchedPathLevel = filter->currentPathLevel;
		else
			filter->lastMatchedPathLevel = 0;
	}

	return 0;
}

void filter_init(struct Filter *filter) {
	memset(filter, 0, sizeof(*filter));

	parser_init(&filter->parser);

	filter->parser.handlers.onMapStart = onScopeStart;
	filter->parser.handlers.onMapStartArg = filter;

	filter->parser.handlers.onMapEnd = onScopeEnd;
	filter->parser.handlers.onMapEndArg = filter;

	filter->parser.handlers.onListStart = onScopeStart;
	filter->parser.handlers.onListStartArg = filter;

	filter->parser.handlers.onListEnd = onScopeEnd;
	filter->parser.handlers.onListEndArg = filter;

	filter->parser.handlers.onMapKey = onMapKey;
	filter->parser.handlers.onMapKeyArg = filter;

	filter->parser.handlers.onString = onScalar;
	filter->parser.handlers.onStringArg = filter;

	filter->parser.handlers.onNumber = onScalar;
	filter->parser.handlers.onNumberArg = filter;

	filter->parser.handlers.onBool = onBool;
	filter->parser.handlers.onBoolArg = filter;

	filter->parser.handlers.onNull = onNull;
	filter->parser.handlers.onNullArg = filter;
}

char filter_test(struct Filter *filter, unsigned char *buf, size_t len) {
	int err;

	filter->currentItemMatched = 0;
	filter->currentPathLevel = 0;
	filter->lastMatchedPathLevel = 0;

	err = parser_parse(&filter->parser, buf, len);

	if(err) {
		fprintf(stderr, "JSON parser error [%d] near: \"", err);
		fwrite(filter->parser.cur, (size_t)(filter->parser.end - filter->parser.cur), 1, stderr);
		fprintf(stderr, "\"\n");
		return 0;
	}

	return filter->currentItemMatched;
}

void filter_free(struct Filter *filter) {
	if(filter->path)
		free(filter->path);

	memset(filter, 0, sizeof(*filter));
}

static void filter_fillPathFromString(struct Filter *filter, const unsigned char *pathString, size_t pathStringLen) {
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

	filter->path = (struct FilterSegment *)calloc((size_t)segmentsCount + 1, sizeof(*filter->path));
	filter->pathSize = segmentsCount + 1;

	/* фейковая нода для облегчения логики при матчинге */
	filter->path[0].isAny = 1;

	segmentStart = pathString;
	segmentOffset++;

	for(i = 0; i < pathStringLen; i++) {
		cur = pathString + i;

		if(*cur == '.') {
			if(cur - segmentStart == 1 && *segmentStart == '*') {
				filter->path[segmentOffset].isAny = 1;
			} else {
				filter->path[segmentOffset].name = segmentStart;
				filter->path[segmentOffset].nameLen = (size_t)(cur - segmentStart);
			}

			segmentStart = pathString + i + 1;

			segmentOffset++;
		}
	}

	if(pathString + pathStringLen - segmentStart == 1 && *segmentStart == '*') {
		/* отдельный кейс для масок, состоящих только из "*" */
		filter->path[segmentOffset].isAny = 1;
	} else {
		filter->path[segmentOffset].name = segmentStart;
		filter->path[segmentOffset].nameLen = (size_t)(pathString + pathStringLen - segmentStart);
	}
}

void filter_equals(struct Filter *filter, const unsigned char *pathString, size_t pathStringLen, const unsigned char *value, size_t valueLen) {
	filter_fillPathFromString(filter, pathString, pathStringLen);

	filter->value = value;
	filter->valueLen = valueLen;

	if(filter->valueLen == 4 && memcmp(value, "null", 4) == 0)
		filter->valueIsNull = 1;

	if(filter->valueLen == 4 && memcmp(value, "true", 4) == 0)
		filter->valueIsTrue = 1;

	if(filter->valueLen == 5 && memcmp(value, "false", 5) == 0)
		filter->valueIsFalse = 1;
}

void filter_exists(struct Filter *filter, const unsigned char *pathString, size_t pathStringLen) {
	filter_fillPathFromString(filter, pathString, pathStringLen);

	filter->anyValue = 1;
}
