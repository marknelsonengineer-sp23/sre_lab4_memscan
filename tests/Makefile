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

TARGET = boost_tests

all:  $(TARGET) lint

CC     = g++
CFLAGS = -Wall -Wextra $(DEBUG_FLAGS)
LINTER = clang-tidy --quiet

debug: DEBUG_FLAGS = -g -DDEBUG
debug: clean $(TARGET)

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

test_iomap.o: ../colors.h
%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) -lboost_system -lboost_thread -lboost_unit_test_framework

lint: $(TARGET)
	$(LINTER) $(SRC) --

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o
