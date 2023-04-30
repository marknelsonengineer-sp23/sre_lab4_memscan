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
# Boost tests compile-run faster without -DBOOST_TEST_DYN_LINK
CXXFLAGS  = -Wall -Wextra $(DEBUG_FLAGS) -std=c++17 -march=native -mtune=native
LDLIBS    = -lm -lcap -lpthread
LINT      = clang-tidy
LINTFLAGS = --quiet -extra-arg=-std=c++17

debug: DEBUG_FLAGS = -g -DDEBUG
debug: $(TARGET)

test:     CFLAGS   += -DTESTING
test:     CXXFLAGS += -DTESTING

valgrind: CFLAGS   += -DTESTING -g -O0 -fno-inline
valgrind: CXXFLAGS +=           -g -O0 -fno-inline -march=x86-64 -mtune=generic

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
	sudo ./$(TARGET) --build_info --color_output --show_progress --report_format=HRF --report_level=short

valgrind: $(TARGET)
	sudo DEBUGINFOD_URLS="https://debuginfod.archlinux.org"                    \
	valgrind --leak-check=full --track-origins=yes --error-exitcode=1 --quiet  \
	./$(TARGET) --build_info --color_output --show_progress --report_format=HRF --report_level=short

clean:
	rm -f $(TARGET) *.o
