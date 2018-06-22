main: main.o cache.o config.o parse.o log.o help.o
	cc -o tinydns *.o
clean:
	rm -f *.o tinydns
