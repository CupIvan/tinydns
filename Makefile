main: main.o cache.o config.o parse.o log.o help.o
	gcc -o tinydns *.o -O2

%.o: %.c
	  gcc -O2 -c -o $@ $<

clean:
	rm -f *.o tinydns

