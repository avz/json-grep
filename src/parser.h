#ifndef PARSER_H
#define	PARSER_H

#include <sys/types.h>

#define ERROR_UNEXPECTED_EOD 1
#define ERROR_UNEXPECTED_TOKEN 2
#define ERROR_STRING_KEY_EXPECTED 3
#define ERROR_COLON_EXPECTED 4

struct ParserHandlers {
	int (*onStart)(void *arg);
	void *onStartArg;

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

	int (*onMapStart)(void *arg);
	void *onMapStartArg;

	int (*onMapKey)(void *arg, const unsigned char *key, size_t len);
	void *onMapKeyArg;

	int (*onMapEnd)(void *arg);
	void *onMapEndArg;
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
