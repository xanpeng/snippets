// rewrite from https://github.com/zhicheng/configparser
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

bool iniconfig_open(struct IniConfig *conf, char *filename) {
	conf->fp = fopen(filename, "r");
	if (conf->fp == NULL)
		return false;
	memset(conf->curr_section, 0, sizeof(conf->curr_section));
	return true;
}

bool iniconfig_close(struct IniConfig *conf) {
	fclose(conf->fp);
	return true;
}

// advanced uses of scanf (http://iitdu.forumsmotion.com/t1781-some-advanced-uses-of-scanf)
// - reading some predefined chars
//   scanf("%[BANGLADESH]", s);  // %[...] is called scanset
//   read a string with only chars 'BANGLADESH', reading will stop if encounter other chars
//   e.x. input "ABDXBSDFS", scanf will read 'ABD'
// - reading a line containing whitespaces
//   scanf("%[^\n", s); // this one is easy.
// - skipping some chars when reading
//   scanf("%d:%d", &h, &m);   // read a int, skip a ':', then read another int
//   scanf("%d%*c%d", &h, &m); // read a int, skip a unknown (%*) char (c), then read another int
// - reading a fixed number of chars
//   scanf("%10s", s);

// parse one line one time
bool iniconfig_parse(struct IniConfig *conf, char *section, char *name, char *value) {
	int len;
	char buf[INICONFIG_MAX_LINELEN];

	memset(name, 0, INICONFIG_MAX_LINELEN);
	memset(value, 0, INICONFIG_MAX_LINELEN);
	memset(section, 0, sizeof(conf->curr_section));
	while (fgets(buf, INICONFIG_MAX_LINELEN, conf->fp) != NULL) {
		if (strlen(buf) == 0)
			continue;
		if (buf[strlen(buf) - 1] != '\n') 	// ignore too long line
			break;
		if (buf[0] == '[') { 				// [section]
			// first '[': skip a '['
			// %[^]]:  a scanset
			sscanf(buf, "[%[^]]", conf->curr_section);
			continue;
		}
		if (!isalpha(buf[0])) 				// ignore line
			continue;
		// name:value or name=value
		// %[^:=\t]: read until a : or = or \t or whitespace
		// %*[:=\t]: skip a unknow data, : or = or \t
		// %[^;n]s: read utils a ; or \n
		if (sscanf(buf, "%[^:=\t ]%*[:=\t ]%[^;\n]s", name, value) != 2)
			continue;

		/* right trim value */
		for (len = strlen(value); len > 0 && isspace(value[len - 1]); len--)
			;
		value[len] = '\0';
		memcpy(section, conf->curr_section, INICONFIG_MAX_LINELEN);

		return true;
	}
	return false;
}
