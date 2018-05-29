void log_s(char *msg)
{
#ifdef DEBUG
	printf("%s\n", msg);
#endif
}

void log_b(char *prefix, void *ptr, int n)
{
#ifdef DEBUG
	int i;
	printf("%s %3d:", prefix, n); for (i=0; i<n; i++) printf(" %02X", *((unsigned char*)ptr + i)); printf("\n");
#endif
}
