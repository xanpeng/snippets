#include "config.h"

int main(int argc, char *argv[]) {
	char section[INICONFIG_MAX_LINELEN];
	char name[INICONFIG_MAX_LINELEN];
	char value[INICONFIG_MAX_LINELEN];

	struct IniConfig conf;
	iniconfig_open(&conf, argv[1]);
	while (iniconfig_parse(&conf, section, name, value)) {
		printf("section: ``%s'', name: ``%s'', value: ``%s''\n", section, name, value);
	}
	iniconfig_close(&conf);
	return 0;
}

