#include <string.h>
#include <stdio.h>

#include "parser.h"

static int parser_readNextToken(struct Parser *parser);

static void parser_earnSpaces(struct Parser *parser) {
	while(parser->cur < parser->end && (*parser->cur == ' ' || *parser->cur == '\t' || *parser->cur == '\r'))
		parser->cur++;
}

static int parser_walkOverCharWithEscaping(struct Parser *parser, unsigned char ch) {
	char nextCharacterEscaped = 0;

	while(parser->cur < parser->end) {
		unsigned char chr = *parser->cur;
		parser->cur++;

		if(nextCharacterEscaped) {
			nextCharacterEscaped = 0;
			continue;
		}

		if(chr == '\\') {
			nextCharacterEscaped = 1;
		} else if(chr == ch) {
			return 0;
		}
	}

	return ERROR_UNEXPECTED_EOD;
}

static int parser_readNumber(struct Parser *parser) {
	/*
	 * /^-?[0-9]+\.?[0-9]*([eE][+-]?[0-9]+)?$/
	 */

	const unsigned char *start = parser->cur;

	while(parser->cur < parser->end) {
		unsigned char chr = *parser->cur;

		if(!(
			(chr >= '0' && chr <= '9')
			|| chr == '-' || chr == '+' || chr == 'E' || chr == 'e' || chr == '.'
		)) {
			break;
		}

		parser->cur++;
	}

	if(parser->handlers.onNumber)
		parser->handlers.onNumber(parser->handlers.onNumberArg, start, (size_t)(parser->cur - start));

	return 0;
}

static int parser_readString(struct Parser *parser) {
	const unsigned char *start = parser->cur + 1;
	int err;

	parser->cur++;

	err = parser_walkOverCharWithEscaping(parser, '"');

	if(err)
		return err;

	if(parser->handlers.onString)
		parser->handlers.onString(parser->handlers.onStringArg, start, (size_t)(parser->cur - 1 - start));

	return 0;
}

static int parser_readMapKey(struct Parser *parser) {
	const unsigned char *start = parser->cur + 1;
	int err;

	parser->cur++;

	err = parser_walkOverCharWithEscaping(parser, '"');

	if(err)
		return err;

	if(parser->handlers.onMapKey)
		parser->handlers.onMapKey(parser->handlers.onMapKeyArg, start, (size_t)(parser->cur - 1 - start));

	return 0;
}

static int parser_readBool(struct Parser *parser) {
	if(*parser->cur == 't') { /* true */
		if(parser->end - parser->cur < 4)
			return ERROR_UNEXPECTED_EOD;

		if(parser->cur[1] == 'r' && parser->cur[2] == 'u' && parser->cur[3] == 'e') {
			if(parser->handlers.onBool)
				parser->handlers.onBool(parser->handlers.onBoolArg, 1);

			parser->cur += 4;

			return 0;
		}

	} else { /* false */
		if(parser->end - parser->cur < 5)
			return ERROR_UNEXPECTED_EOD;

		if(parser->cur[1] == 'a' && parser->cur[2] == 'l' && parser->cur[3] == 's' && parser->cur[4] == 'e') {
			if(parser->handlers.onBool)
				parser->handlers.onBool(parser->handlers.onBoolArg, 0);

			parser->cur += 5;

			return 0;
		}
	}

	return ERROR_UNEXPECTED_TOKEN;
}

static int parser_readNull(struct Parser *parser) {
	if(parser->end - parser->cur < 4)
		return ERROR_UNEXPECTED_EOD;

	if(parser->cur[1] == 'u' && parser->cur[2] == 'l' && parser->cur[3] == 'l') {
		if(parser->handlers.onNull)
			parser->handlers.onNull(parser->handlers.onNullArg);

		parser->cur += 4;

		return 0;
	}

	return ERROR_UNEXPECTED_TOKEN;
}

static int parser_readList(struct Parser *parser) {
	int err;

	parser->cur++;

	if(parser->handlers.onListStart)
		parser->handlers.onListStart(parser->handlers.onListStartArg);

	parser_earnSpaces(parser);

	if(*parser->cur == ']') {
		if(parser->handlers.onListEnd)
			parser->handlers.onListEnd(parser->handlers.onListEndArg);

		parser->cur++;

		return 0;
	}

	while(parser->cur < parser->end) {
		err = parser_readNextToken(parser);
		if(err)
			return err;

		parser_earnSpaces(parser);

		if(*parser->cur == ']') {
			if(parser->handlers.onListEnd)
				parser->handlers.onListEnd(parser->handlers.onListEndArg);

			parser->cur++;

			return 0;
		}

		if(*parser->cur != ',')
			return ERROR_UNEXPECTED_TOKEN;

		parser->cur++;
	}

	return ERROR_UNEXPECTED_EOD;
}

static int parser_readMap(struct Parser *parser) {
	int err;

	if(parser->handlers.onMapStart)
		parser->handlers.onMapStart(parser->handlers.onMapStartArg);

	parser->cur++;

	parser_earnSpaces(parser);

	if(*parser->cur == '}') {
		if(parser->handlers.onMapEnd)
			parser->handlers.onMapEnd(parser->handlers.onMapEndArg);

		parser->cur++;

		return 0;
	}

	while(parser->cur < parser->end) {
		parser_earnSpaces(parser);

		if(*parser->cur != '"')
			return ERROR_STRING_KEY_EXPECTED;

		err = parser_readMapKey(parser);
		if(err)
			return err;

		parser_earnSpaces(parser);

		if(*parser->cur != ':')
			return ERROR_COLON_EXPECTED;

		parser->cur++;

		err = parser_readNextToken(parser);
		if(err)
			return err;

		if(*parser->cur == '}') {
			if(parser->handlers.onMapEnd)
				parser->handlers.onMapEnd(parser->handlers.onMapEndArg);

			parser->cur++;

			return 0;
		}

		if(*parser->cur != ',')
			return ERROR_UNEXPECTED_TOKEN;

		parser->cur++;
	}

	return 0;
}


static int parser_readNextToken(struct Parser *parser) {
	parser_earnSpaces(parser);

	switch(*parser->cur) {
		case '"':
			return parser_readString(parser);
		break;
		case 't':
		case 'f':
			return parser_readBool(parser);
		break;
		case 'n':
			return parser_readNull(parser);
		break;
		case '[':
			return parser_readList(parser);
		break;
		case '{':
			return parser_readMap(parser);
		break;
		default:
			if((*parser->cur >= '0' && *parser->cur <= '9') || *parser->cur == '-')
				return parser_readNumber(parser);

			return ERROR_UNEXPECTED_EOD;
	}

	return 0;
}

void parser_init(struct Parser *parser) {
	memset(parser, 0, sizeof(*parser));
}

int parser_parse(struct Parser *parser, unsigned char *buf, size_t len) {
	parser->start = buf;
	parser->end = buf + len;
	parser->cur = buf;

	if(len)
		return parser_readNextToken(parser);

	return 0;
}
