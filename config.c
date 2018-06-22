#include "common.h"

TConfig config = {"127.0.0.1", "8.8.8.8", 6*3600};

char rr_buf[0xFFF] = {0};

char* config_param(char* s, void* res, uint type)
{
	THeader *rr = (THeader*)rr_buf; rr->QRCOUNT = htons(1);
	char   *rr_ptr, *rr_dot;
	int state = 1, x, rr_size, rr_uid = 1;
	if (type == CONFIG_TYPE_RR) state = 6;
	while (*s)
	{
		switch (state)
		{
			case 1: if (*s == ':') { state = (type==CONFIG_TYPE_INT)?5:2; } break;
			case 2: if (*s == '"') { state = 3; *(char**)res = s+1; } break;
			case 3: if (*s == '"') { state = 4; *s = 0; }             break;
			case 4: return s;
			case 5: if (*s >= '0' && *s <= '9') { state = 4; sscanf(s, "%d", &x); *(uint16_t*)res = x; } break;
			// add query/answer from config
			case 6: if (*s == ']') state = 4; if (*s == '"') { state = 7; rr_ptr = rr_dot = s; x = 0; } break;
			case 7:
				if (*s == '.' || *s == '"') { *rr_dot = x; rr_dot = s; x = 0; } else x++;
				// construct question
				if (*s == '"')
				{
					state = 8; *s = 0;
					strcpy((char*)(rr + 1), rr_ptr);
					rr->uid     = rr_uid++;
					rr->RD      = 1;
					rr->QR      = 0;
					rr->RA      = 0;
					rr->ANCOUNT = 0;
					rr_size = sizeof(THeader) + s - rr_ptr + 1;
					rr_buf[rr_size++] = 0x00; rr_buf[rr_size++] = 0x01; // TYPE
					rr_buf[rr_size++] = 0x00; rr_buf[rr_size++] = 0x01; // CLASS
					log_b("A-->", rr_buf, rr_size);
					cache_question(rr_buf, rr_size);
				}
				break;
			case 8: if (*s == '"') { state = 9; rr_ptr = s + 1; } break;
			case 9:
				// construct answer
				if (*s == '"')
				{
					state = 6; *s = 0;
					rr->RD      = 1;
					rr->QR      = 1;
					rr->RA      = 1;
					rr->ANCOUNT = htons(1);
					rr_buf[rr_size++] = 0xC0;
					rr_buf[rr_size++] = 0x0C;
					rr_buf[rr_size++] = 0x00; rr_buf[rr_size++] = 0x01; // TYPE
					rr_buf[rr_size++] = 0x00; rr_buf[rr_size++] = 0x01; // CLASS
					rr_buf[rr_size++] = 0x00; rr_buf[rr_size++] = 0x00; // TTL
					rr_buf[rr_size++] = 0xAA; rr_buf[rr_size++] = 0xAA; // TTL
					rr_buf[rr_size++] = 0x00; rr_buf[rr_size++] = 0x04; // RDLENGTH
					inet_aton(rr_ptr, (struct in_addr *)&rr_buf[rr_size]); rr_size += 4;
					log_b("<--A", rr_buf, rr_size);
					cache_answer(rr_buf, rr_size);
				}
				break;
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
		if (memcmp(ptr, "server_ip",   9) == 0) ptr = config_param(ptr, &config.server_ip,  CONFIG_TYPE_STRING);
		if (memcmp(ptr, "dns",         3) == 0) ptr = config_param(ptr, &config.dns,        CONFIG_TYPE_STRING);
		if (memcmp(ptr, "cache_time", 10) == 0) ptr = config_param(ptr, &config.cache_time, CONFIG_TYPE_INT);
		if (memcmp(ptr, "debug_level",11) == 0) ptr = config_param(ptr, &config.debug_level,CONFIG_TYPE_INT);
		if (memcmp(ptr, "rr",          2) == 0) ptr = config_param(ptr, NULL,               CONFIG_TYPE_RR);
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
	if (!config.data) error("Can't allocate memory for config!");

	fread(config.data, 1, fsize, f);
	fclose(f);

	config_parse(config.data);
}
