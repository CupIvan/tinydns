#include "common.h"

void parse_buf(THeader *buf)
{
	char domain[255];
	char *current = domain;

	strcpy(domain, (char*)(buf+1));

	int n = current[0];
	while (n > 0)
	{
		current[0] = '.';
		current += n + 1;
		n = current[0];
	}
	log_s(domain + 1); // COMMENT: skip first dot
}
