#include <stdlib.h>

#define CONFIG_TYPE_STRING 1
#define CONFIG_TYPE_INT    2
#define CONFIG_TYPE_RR     3

typedef struct TConfig
{
	char    *server_ip;
	char    *dns;
	uint32_t cache_time;
	uint8_t  debug_level;
	char    *data;
} TConfig;

extern TConfig config;

void config_load();
