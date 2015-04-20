#include "json.h"

#include <stdio.h>
#include <string.h>

#define CASE_NUMBER \
	case '0':   \
	case '1':   \
	case '2':   \
	case '3':   \
	case '4':   \
	case '5':   \
	case '6':   \
	case '7':   \
	case '8':   \
	case '9'

#define CASE_HEX     \
	CASE_NUMBER: \
	case 'a':    \
	case 'A':    \
	case 'b':    \
	case 'B':    \
	case 'c':    \
	case 'C':    \
	case 'd':    \
	case 'D':    \
	case 'e':    \
	case 'E':    \
	case 'f':    \
	case 'F'

#define CASE_BLANK \
	case ' ':  \
	case '\t': \
	case '\r': \
	case '\n'


int json_parse_true(json_parser_t *parser, json_value_t *super, json_value_t *value) {
	int i;
	int c;
	int s;

	enum {
		status_begin,
		status_t,
		status_tr,
		status_tru,
		status_ok
	};

	s = status_begin;
	for (i = parser->off; i < parser->len; i++) {
		c = parser->buf[i];

		switch (s) {
		case status_ok:
			value->len  = i - value->off;
			value->type = JSON_TRUE;
			parser->off = i;
			return JSON_PARSER_OK;
		case status_begin:
			if (c != 't')
				return JSON_PARSER_ERR;
			value->off = i;
			s = status_t;
			break;
		case status_t:
			if (c != 'r')
				return JSON_PARSER_ERR;
			s = status_tr;
			break;
		case status_tr:
			if (c != 'u')
				return JSON_PARSER_ERR;
			s = status_tru;
			break;
		case status_tru:
			if (c != 'e')
				return JSON_PARSER_ERR;
			s = status_ok;
			break;
		default:
			return JSON_PARSER_ERR;
		}
	}
	return JSON_PARSER_ERR;
}

int json_parse_false(json_parser_t *parser, json_value_t *super, json_value_t *value) {
	int i;
	int c;
	int s;

	enum {
		status_begin,
		status_f,
		status_fa,
		status_fal,
		status_fals,
		status_ok
	};

	s = status_begin;
	for (i = parser->off; i < parser->len; i++) {
		c = parser->buf[i];

		switch (s) {
		case status_ok:
			value->len  = i - value->off;
			value->type = JSON_FALSE;
			parser->off = i;
			return JSON_PARSER_OK;
		case status_begin:
			if (c != 'f')
				return JSON_PARSER_ERR;
			value->off = i;
			s = status_f;
			break;
		case status_f:
			if (c != 'a')
				return JSON_PARSER_ERR;
			s = status_fa;
			break;
		case status_fa:
			if (c != 'l')
				return JSON_PARSER_ERR;
			s = status_fal;
			break;
		case status_fal:
			if (c != 's')
				return JSON_PARSER_ERR;
			s = status_fals;
			break;
		case status_fals:
			if (c != 'e')
				return JSON_PARSER_ERR;
			s = status_ok;
			break;
		default:
			return JSON_PARSER_ERR;
		}
	}
	return JSON_PARSER_ERR;
}

int json_parse_null(json_parser_t *parser, json_value_t *super, json_value_t *value) {
	int i;
	int c;
	int s;

	enum {
		status_begin,
		status_n,
		status_nu,
		status_nul,
		status_ok
	};

	s = status_begin;
	for (i = parser->off; i < parser->len; i++) {
		c = parser->buf[i];

		switch (s) {
		case status_ok:
			value->len  = i - value->off;
			value->type = JSON_NULL;
			parser->off = i;
			return JSON_PARSER_OK;
		case status_begin:
			if (c != 'n')
				return JSON_PARSER_ERR;
			value->off = i;
			s = status_n;
			break;
		case status_n:
			if (c != 'u')
				return JSON_PARSER_ERR;
			s = status_nu;
			break;
		case status_nu:
			if (c != 'l')
				return JSON_PARSER_ERR;
			s = status_nul;
			break;
		case status_nul:
			if (c != 'l')
				return JSON_PARSER_ERR;
			s = status_ok;
			break;
		default:
			return JSON_PARSER_ERR;
		}
	}
	return JSON_PARSER_ERR;
}

int json_parse_string(json_parser_t *parser, json_value_t *super, json_value_t *value) {
	int i;
	int c;
	int s;

	enum {
		status_begin,
		status_string,
		status_escape,
		status_unicode1,
		status_unicode2,
		status_unicode3,
		status_unicode4,
		status_ok
	};

	s = status_begin;
	for (i = parser->off; i < parser->len; i++) {
		c = parser->buf[i];

		switch (s) {
		case status_ok:
			value->len  = i - value->off;
			value->type = JSON_STRING;
			parser->off = i;
			return JSON_PARSER_OK;
		case status_begin:
			if (c != '\"')
				return JSON_PARSER_ERR;
			value->off = i;
			s = status_string;
			break;
		case status_string:
			switch (c) {
			case '\\':
				s = status_escape;
				break;
			case '\"':
				s = status_ok;
				break;
			}
			break;
		case status_escape:
			switch (c) {
			case 'u':
				s = status_unicode1;
				break;
			case 'b':
			case 'f':
			case 't':
			case 'r':
			case 'n':
			case '/':
			case '\"':
			case '\\':
				s = status_string;
				break;
			default:
				return JSON_PARSER_ERR;
			}
			break;
		case status_unicode1:
			switch (c) {
			CASE_HEX:
				s = status_unicode2;
				break;
			default:
				return JSON_PARSER_ERR;
			}

			break;
		case status_unicode2:
			switch (c) {
			CASE_HEX:
				s = status_unicode3;
				break;
			default:
				return JSON_PARSER_ERR;
			}
			break;
		case status_unicode3:
			switch (c) {
			CASE_HEX:
				s = status_unicode4;
				break;
			default:
				return JSON_PARSER_ERR;
			}
			break;
		case status_unicode4:
			switch (c) {
			CASE_HEX:
				s = status_string;
				break;
			default:
				return JSON_PARSER_ERR;
			}
			break;
		default:
			return JSON_PARSER_ERR;
		}
	}
	return JSON_PARSER_ERR;
}

int json_parse_number(json_parser_t *parser, json_value_t *super, json_value_t *value) {
	int i;
	int c;
	int s;

	enum {
		status_begin,
		status_number,
		status_ok
	};

	s = status_begin;
	for (i = parser->off; i < parser->len; i++) {
		c = parser->buf[i];

		switch (s) {
		case status_ok:
			value->len  = i - value->off;
			value->type = JSON_NUMBER;
			parser->off = i;
			return JSON_PARSER_OK;
		case status_begin:
			switch (c) {
			CASE_NUMBER:
				break;
			default:
				return JSON_PARSER_ERR;
			}
			value->off = i;
			s = status_number;
			break;
		case status_number:
			switch (c) {
			CASE_NUMBER:
				break;
			default:
				i--;
				s = status_ok;
			}

			break;
		default:
			return JSON_PARSER_ERR;
		}

	}
	return JSON_PARSER_ERR;
}

int json_parse(json_parser_t *parser, json_value_t *super, json_value_t *value) {
	int i;
	int c;

	for (i = parser->off; i < parser->len; i++) {
		c = parser->buf[i];
		parser->off = i;

		switch (c) {
		CASE_BLANK:
			break;
		case '{':
			value->off  = i;
			value->len  = parser->len - i;
			value->type = JSON_OBJECT;
			return JSON_PARSER_OK;
		case '[':
			value->off  = i;
			value->len  = parser->len - i;
			value->type = JSON_ARRAY;
			return JSON_PARSER_OK;
		case '\"':
			return json_parse_string(parser, super, value);
		CASE_NUMBER:
			return json_parse_number(parser, super, value);
		case 't':
			return json_parse_true(parser, super, value);
		case 'f':
			return json_parse_false(parser, super, value);
		case 'n':
			return json_parse_null(parser, super, value);
		default: 
			return JSON_PARSER_ERR;
		}
	}
	return JSON_PARSER_ERR;
}

int json_parser_parse(json_parser_t *parser, json_value_t *super, char *buf, int len) {
	parser->off = 0;
	parser->len = len;
	parser->buf = buf;

	parser->off = 0;
	super->len = len;

	return json_parse(parser, super, super);
}

int json_parser_value(json_parser_t *parser, json_value_t *super, json_value_t *value) {
	int i;
	int c;
	int s;

	enum {
		status_first,
		status_next,
		status_begin,
		status_value,
		status_end,
		status_ok
	};

	if (super->type != JSON_ARRAY)
		return JSON_PARSER_ERR;

	if (parser->off == super->off)
		s = status_first;
	else
		s = status_next;

	for (i = parser->off; i < parser->len; i++) {
		c = parser->buf[i];

		switch (s) {
		case status_ok:
			parser->off = i;
			return JSON_PARSER_OK;
		case status_end:
			parser->off = i;
			return JSON_PARSER_END;
		case status_first:
			switch (c) {
			CASE_BLANK:
				break;
			case '[':
				s = status_begin;
				break;
			default:
				return JSON_PARSER_ERR;
			}
			break;
		case status_next:
			switch (c) {
			CASE_BLANK:
				break;
			case ']':
				s = status_end;
				break;
			case ',':
				s = status_begin;
				break;
			default:
				return JSON_PARSER_ERR;
			}
			break;
		case status_begin:
			switch (c) {
			CASE_BLANK:
				break;
			case ']':
				s = status_end;
				break;
			case ',':
				return JSON_PARSER_ERR;
			default:
				parser->off = i;
				s = status_value;
				break;
			}
			break;
		case status_value:
			if (json_parse(parser, super, value) != JSON_PARSER_OK)
				return JSON_PARSER_ERR;
			i = parser->off - 1; /* for next round */
			s = status_ok;
			break;
		default:
			return JSON_PARSER_ERR;
		}
	}
	return JSON_PARSER_ERR;
}

int json_parser_pair(json_parser_t *parser, json_value_t *super, json_value_t *name, json_value_t *value) {
	int i;
	int c;
	int s;

	enum {
		status_first,
		status_next,
		status_begin,
		status_name,
		status_colon,
		status_value,
		status_end,
		status_ok
	};

	if (super->type != JSON_OBJECT)
		return JSON_PARSER_ERR;

	if (parser->off == super->off)
		s = status_first;
	else
		s = status_next;

	for (i = parser->off; i < parser->len; i++) {
		c = parser->buf[i];

		switch (s) {
		case status_ok:
			parser->off = i;
			return JSON_PARSER_OK;
		case status_end:
			parser->off = i;
			return JSON_PARSER_END;
		case status_first:
			switch (c) {
			CASE_BLANK:
				break;
			case '{':
				s = status_begin;
				break;
			default:
				return JSON_PARSER_ERR;
			}
			break;
		case status_next:
			switch (c) {
			CASE_BLANK:
				break;
			case '}':
				s = status_end;
				break;
			case ',':
				s = status_begin;
				break;
			default:
				return JSON_PARSER_ERR;
			}
			break;
		case status_begin:
			switch (c) {
			CASE_BLANK:
				break;
			case '\"':
				s = status_name;
				parser->off = i;
				break;
			case '}':
				s = status_end;
				break;
			default:
				return JSON_PARSER_ERR;
			}
			break;
		case status_name:
			if (json_parse_string(parser, super, name) != JSON_PARSER_OK)
				return JSON_PARSER_ERR;
			i = parser->off - 1; /* for next round */
			s = status_colon;
			break;
		case status_colon:
			switch (c) {
			CASE_BLANK:
				break;
			case ':':
				s = status_value;
				parser->off = i + 1;
				break;
			default:
				return JSON_PARSER_ERR;
			}
			break;
		case status_value:
			if (json_parse(parser, super, value) != JSON_PARSER_OK) {
				return JSON_PARSER_ERR;
			}
			i = parser->off - 1; /* for next round */
			s = status_ok;
			break;
		default:
			return JSON_PARSER_ERR;
		}
	}

	return JSON_PARSER_ERR;
}

int json_parser_ignore(json_parser_t *parser, json_value_t *value) {
	int i;
	int c;
	int s;
	int d;

	enum {
		status_begin,
		status_object,
		status_array,
		status_object_string,
		status_array_string,
		status_ok
	};

	if (value->type != JSON_OBJECT && value->type != JSON_ARRAY)
		return JSON_PARSER_ERR;

	d = 0;
	s = status_begin;
	for (i = value->off; i < parser->len; i++) {
		c = parser->buf[i];

		switch (s) {
		case status_ok:
			parser->off = i;
			return JSON_PARSER_OK;
		case status_begin:
			switch (c) {
			CASE_BLANK:
				break;
			case '{':
				s = status_object;
				d += 1;
				break;
			case '[':
				s = status_array;
				d += 1;
				break;
			}
			break;
		case status_object_string:
		case status_array_string:
			switch (c) {
			case '\"':
				switch (s) {
				case status_object_string:
					s = status_object;
					break;
				case status_array_string:
					s = status_array;
					break;
				}
			case '\\':
				i += 1;
				break;
			}
			break;
		case status_object:
			switch (c) {
			case '{':
				d += 1;
				break;
			case '}':
				d -= 1;
				if (d == 0)
					s = status_ok;
				break;
			case '\"':
				s = status_object_string;
				break;
			}
			break;
		case status_array:
			switch (c) {
			case '[':
				d += 1;
				break;
			case ']':
				d -= 1;
				if (d == 0) {
					s = status_ok;
				}
				break;
			case '\"':
				s = status_array_string;
				break;
			}
			break;
		default:
			return JSON_PARSER_ERR;
		}
	}
	return JSON_PARSER_ERR;
}
