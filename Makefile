###############################################################################
### University of Hawaii, College of Engineering
### Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
###
### Build and test a memory scanning program
###
### @see     https://www.gnu.org/software/make/manual/make.html
###
### @file    Makefile
### @author  Mark Nelson <marknels@hawaii.edu>
###############################################################################

TARGET = memscan

all:  $(TARGET) lint doc

CC     = gcc
CFLAGS = -Wall -Wextra $(DEBUG_FLAGS) -std=c17
LINTER = clang-tidy --quiet

debug: DEBUG_FLAGS = -g -DDEBUG
debug: clean $(TARGET)

config.o: config.c config.h version.h
	$(CC) $(CFLAGS) -c $<

maps.o: maps.c maps.h memscan.h
	$(CC) $(CFLAGS) -c $<

memscan.o: memscan.c memscan.h maps.h
	$(CC) $(CFLAGS) -c $<

memscan: memscan.o maps.o
	$(CC) $(CFLAGS) -o $(TARGET) $^

lint: $(TARGET)
	$(LINTER) memscan.c maps.c config.c --

doc: $(TARGET)
	.doxygen/stats.py
	doxygen .doxygen/Doxyfile
	rsync --recursive --mkpath --checksum --delete .doxygen/images .doxygen/docs/html/.doxygen
	rsync --recursive --checksum --delete --compress --stats --chmod=o+r,Do+x .doxygen/docs/html/ marknels@uhunix.hawaii.edu:~/public_html/sre/memscan

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o
