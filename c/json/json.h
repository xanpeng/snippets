#ifndef __JSON_H__
#define __JSON_H__

enum {
	JSON_PARSER_ERR = -1,
	JSON_PARSER_END =  0,
	JSON_PARSER_OK  =  1
};

enum {
	JSON_STRING,
	JSON_NUMBER,
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_TRUE,
	JSON_FALSE,
	JSON_NULL
};

// len isn't available when (type == JSON_OBJECT) or (type == JSON_ARRAY)
typedef struct {
	int off;
	int len;   
	int type;
} json_value_t;

typedef struct {
	int off;
	int len;
	char *buf;
} json_parser_t;

int json_parser_parse(json_parser_t *parser, json_value_t *root, char *buf, int len);
int json_parser_value(json_parser_t *parser, json_value_t *super, json_value_t *value);
int json_parser_pair(json_parser_t *parser, json_value_t *super, json_value_t *name, json_value_t *value);
/* ignore array and object value because len isn't available */
int json_parser_ignore(json_parser_t *parser, json_value_t *value);

#endif /* __JSON_H__ */
