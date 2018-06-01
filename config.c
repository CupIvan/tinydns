#define CONFIG_TYPE_STRING 1
#define CONFIG_TYPE_INT    2

typedef struct TConfig
{
	char     *server_ip;
	char     *dns;
	char     *data;
} TConfig;

TConfig config = {"127.0.0.1", "8.8.8.8"};

char* config_param(char* s, void* res, uint type)
{
	int state = 1, x;
	while (*s)
	{
		switch (state)
		{
			case 1: if (*s == ':') { state = (type==2)?5:2; }         break;
			case 2: if (*s == '"') { state = 3; *(char**)res = s+1; } break;
			case 3: if (*s == '"') { state = 4; *s = 0; }             break;
			case 4: return s;
			case 5: if (*s >= '0' && *s <= '9') { state = 4; sscanf(s, "%d", &x); *(uint16_t*)res = x; } break;
		}
		s++;
	}
	return s;
}

void config_parse(char* s)
{
	char* ptr = s;
	while (*ptr)
	{
		if (memcmp(ptr, "server_ip", 9) == 0) ptr = config_param(ptr, &config.server_ip, CONFIG_TYPE_STRING);
		if (memcmp(ptr, "dns",       3) == 0) ptr = config_param(ptr, &config.dns,       CONFIG_TYPE_STRING);
		ptr++;
	}
}

void config_load()
{
	FILE* f;
	f = fopen("tinydns.conf", "rb");
	if (!f)
	f = fopen("/etc/tinydns.conf", "rb");
	if (!f) return;

	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);
	rewind(f);

	config.data = (char*)malloc(fsize);
	if (!config.data) exit(1);

	fread(config.data, 1, fsize, f);
	fclose(f);

	config_parse(config.data);
}
