all:  clean memscan test

clean:
	rm -f memscan *.o

memscan: memscan.c
	gcc -o memscan memscan.c

test: memscan
	./memscan
