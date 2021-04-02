main: main.o cache.o config.o parse.o log.o help.o
	gcc -o tinydns *.o -O0
clean:
	rm -f *.o tinydns
