#include <stdio.h>

int g_debug = 1;

void log_s(char *msg)
{
	if (!g_debug) return;
	printf("%s\n", msg);
}

void log_b(char *prefix, void *ptr, int n)
{
	int i;
	if (!g_debug) return;
	printf("%s %3d:", prefix, n); for (i=0; i<n; i++) printf(" %02X", *((unsigned char*)ptr + i)); printf("\n");
}
