*NOTICE* This repo is copied from https://github.com/zhicheng/json.
Thanks zhicheng, his c code is elegant.

How to write a json parser: (http://www.liaoxuefeng.com/article/0014211269349633dda29ee3f29413c91fa65c372585f23000)


JSON Parser in C

4 function

json_parser_parse
json_parser_value
json_parser_pair
json_parser_ignore

usage:

json_parser_parse(root)

if (root.type == JSON_OBJECT) {
	for (;;) {
		json_parser_pair(name, value)
	}
} else if (root.type == JSON_ARRAY) {
	for (;;) {
		json_parser_value(element)
	}
}

if don't need object or array call `json_parser_ignore`

more demo see main.c require libcurl

License:
Public Domain
