/*
 * cc buf.c json.c main.c -lcurl -o github
 */
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#include "buf.h"
#include "json.h"

int github(char *buf, int len) {
	int err;
	json_value_t root;
	json_parser_t parser;

	if (json_parser_parse(&parser, &root, buf, len) != JSON_PARSER_OK) {
		fprintf(stderr, "json_parser_parse error\n");
		return 1;
	}

	err = 0;
	for (;;) {
		char namebuf[1024];
		char valuebuf[1024];
		json_value_t name;
		json_value_t value;
		memset(&name, 0, sizeof(name));
		memset(&value, 0, sizeof(name));
		memset(namebuf, 0, sizeof(namebuf));
		memset(valuebuf, 0, sizeof(valuebuf));

		/*
		 * pair  for object name/value
		 * value for array element
		 */
		err = json_parser_pair(&parser, &root, &name, &value);
		if (err != JSON_PARSER_OK) {
			break;
		}

		memcpy(namebuf, buf + name.off, name.len);
		if (value.type == JSON_OBJECT || value.type == JSON_ARRAY) {
			/* ignore object and array that we don't need */
			json_parser_ignore(&parser, &value);
			continue;
		}
		memcpy(valuebuf, buf + value.off, value.len);

		if (strcmp(namebuf, "\"name\"") == 0) {
			printf("name: %s\n", valuebuf);
		} else if (strcmp(namebuf, "\"location\"") == 0) {
			printf("location: %s\n", valuebuf);
		} else if (strcmp(namebuf, "\"public_repos\"") == 0) {
			printf("public_repos: %s\n", valuebuf);
		} else if (strcmp(namebuf, "\"hireadble\"") == 0) {
			printf("hireadble: %s\n", valuebuf);
		}
	}

	if (err == JSON_PARSER_ERR) {
		printf("parser root object error\n");
		return 1;
	}

	return 0;
}

static size_t fetch_data(void *ptr, size_t size, size_t nmemb, void *buf) {
	sbuf_t *sbuf;
	size_t bytes;

	sbuf = buf;
	bytes = size * nmemb;

	sbuf_append(sbuf, ptr, bytes);

	return bytes;
}

int main(void) {
	CURL *curl;
	CURLcode res;

	sbuf_t buf;

	sbuf_alloc(&buf, 4096);

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL,
				"https://api.github.com/users/zhicheng");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "JSON Parser Demo");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fetch_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		}

		github(buf.buf, buf.len + 1);

		sbuf_release(&buf);
		curl_easy_cleanup(curl);
	}
	return 0;
}
