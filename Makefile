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

all:  $(TARGET)

CC     = gcc
CFLAGS = -Wall -Wextra $(DEBUG_FLAGS)
LINTER = clang-tidy --quiet


debug: DEBUG_FLAGS = -g -DDEBUG
debug: clean $(TARGET)


memscan.o: memscan.c
	$(CC) $(CFLAGS) -c $^
	$(LINTER) memscan.c --


memscan: memscan.o
	$(CC) $(CFLAGS) -o $(TARGET) $^


test: $(TARGET)
	./$(TARGET)


clean:
	rm -f $(TARGET) *.o
