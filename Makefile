###############################################################################
#  University of Hawaii, College of Engineering
#  Memory Scanner 2 - EE 491F (Software Reverse Engineering) - Spr 2024
#
## Build and test a memory scanning program
##
## @see     https://www.gnu.org/software/make/manual/make.html
##
## Memscan2 was developed under gcc and it does not work with clang
##
## @file   Makefile
## @author Mark Nelson <marknels@hawaii.edu>
###############################################################################

TARGET = memscan

all: $(TARGET)

CC        = gcc
CFLAGS    = -std=c17 -Wall -Wextra -Werror -march=native -mtune=native -DTARGET=\"$(TARGET)\" $(DEBUG_FLAGS)
LDLIBS    = -lm -lcap -lpthread
LINT      = clang-tidy
LINTFLAGS = --quiet -extra-arg=-std=c++17
MAKE      = make

debug: DEBUG_FLAGS = -g -DDEBUG -O0
debug: clean $(TARGET)

test:     CFLAGS += -DTESTING
valgrind: CFLAGS += -DTESTING -g -O0 -fno-inline -march=x86-64 -mtune=generic

export FULL_VERSION = $(shell .doxygen/update_version.py)

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

maps.o: colors.h memscan.h
memscan.o: maps.h config.h
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDLIBS)

static: $(OBJ)
	$(CC) -static -static-libgcc -L/usr/local/lib $(CFLAGS) -o $(TARGET)-static $(OBJ) $(LDLIBS)

lint: $(TARGET)
	$(LINT) $(LINTFLAGS) $(SRC) --
	cd tests && $(MAKE) lint

doc: $(TARGET)
	.doxygen/stats.py
	.doxygen/update_usage.py
	.doxygen/update_key.py
	doxygen .doxygen/Doxyfile
	rsync --recursive --mkpath --checksum --delete .doxygen/images  .doxygen/docs/html/.doxygen
	rsync --recursive --mkpath --checksum --delete  examples/images .doxygen/docs/html/examples

publish: doc
	rsync --recursive --checksum --delete --compress --stats --chmod=o+r,Do+x .doxygen/docs/html/ marknels@uhunix.hawaii.edu:~/public_html/sre/memscan

test: $(OBJ)
	cd tests && $(MAKE) -j 4 test

valgrind: $(OBJ)
	cd tests && $(MAKE) valgrind

clean:
	rm -f $(TARGET) *.o
	cd tests && $(MAKE) clean
	rm -fr --preserve-root .doxygen/docs/html
