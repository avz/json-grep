#ifndef PARSER_H
#define	PARSER_H

#include <sys/types.h>

#define ERROR_UNEXPECTED_EOD 1
#define ERROR_UNEXPECTED_TOKEN 2
#define ERROR_STRING_KEY_EXPECTED 3
#define ERROR_COLON_EXPECTED 4

struct ParserHandlers {
	int (*onBool)(void *arg, char bool);
	void *onBoolArg;

	int (*onNull)(void *arg);
	void *onNullArg;

	int (*onNumber)(void *arg, const unsigned char *number, size_t len);
	void *onNumberArg;

	int (*onString)(void *arg, const unsigned char *string, size_t len);
	void *onStringArg;

	int (*onListStart)(void *arg);
	void *onListStartArg;

	int (*onListEnd)(void *arg);
	void *onListEndArg;

	int (*onHashStart)(void *arg);
	void *onHashStartArg;

	int (*onHashKey)(void *arg, const unsigned char *string, size_t len);
	void *onHashKeyArg;

	int (*onHashEnd)(void *arg);
	void *onHashEndArg;
};

struct Parser {
	const unsigned char *start;
	const unsigned char *end;

	const unsigned char *cur;

	struct ParserHandlers handlers;
};

void parser_init(struct Parser *parser);
int parser_parse(struct Parser *parser, unsigned char *buf, size_t len);

#endif	/* PARSER_H */
