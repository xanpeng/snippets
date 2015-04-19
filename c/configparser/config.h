#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include <stdbool.h>

#define INICONFIG_MAX_LINELEN 4096

struct IniConfig {
	FILE *fp;
	char curr_section[INICONFIG_MAX_LINELEN];
};

bool iniconfig_open(struct IniConfig *config, char *filename);
bool iniconfig_close(struct IniConfig *config);
bool iniconfig_parse(struct IniConfig *config, char *section, char *name, char *value);

#endif /* __CONFIG_H__ */
