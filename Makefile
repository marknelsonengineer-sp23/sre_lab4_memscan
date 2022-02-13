###############################################################################
###          University of Hawaii, College of Engineering
### @brief   Lab 03a - Memory Scanner - EE 491F - Spr 2022
###
### @file    Makefile
### @version 1.0 - Initial implementation
###
### Build and test an energy unit conversion program
###
### @author  Mark Nelson <marknels@hawaii.edu>
### @date    5_Feb_2022
###
### @see     https://www.gnu.org/software/make/manual/make.html
###############################################################################


TARGET = memscan


all:  $(TARGET)


CC     = gcc
CFLAGS = -Wall -Wextra $(DEBUG_FLAGS)


debug: DEBUG_FLAGS = -g -DDEBUG
debug: clean $(TARGET)


memscan.o: memscan.c
	$(CC) $(CFLAGS) -c memscan.c


memscan: memscan.o
	$(CC) $(CFLAGS) -o $(TARGET) memscan.o


test: $(TARGET)
	./$(TARGET)


clean:
	rm -f $(TARGET) *.o
