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

CXX       = g++
CXXFLAGS  = -Wall -Wextra $(DEBUG_FLAGS)
LDLIBS    = -lm -lcap -lpthread
LINT      = clang-tidy
LINTFLAGS = --quiet -extra-arg=-std=c++17

debug: DEBUG_FLAGS = -g -DDEBUG
debug: $(TARGET)

test: CFLAGS += -DTESTING

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
TEST_OBJS = $(filter-out ../memscan.o, $(wildcard ../*.o))

test_iomap.o: ../colors.h
%.o: %.c
	$(CXX) $(CXXFLAGS) -c $<

$(TARGET): $(OBJ) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(TEST_OBJS) $(LDLIBS)

lint: $(TARGET)
	$(LINT) $(LINTFLAGS) $(SRC) --

test: $(TARGET)
	./$(TARGET) --build_info --color_output --show_progress --report_format=HRF --report_level=short

clean:
	rm -f $(TARGET) *.o
