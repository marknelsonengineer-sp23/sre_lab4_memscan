###############################################################################
# University of Hawaii, College of Engineering
# EE 491F - Software Reverse Engineering
# Lab 04a - Memory Scanner
#
# @file    Makefile
# @version 1.0 - Initial implementation
#
# @author Mark Nelson <marknels@hawaii.edu>
# @brief  Lab 04a - Memory Scanner - EE 491F - Spr 2021
# @date   28 Jun 2021
###############################################################################

all:  clean memscan test

clean:
	rm -f memscan *.o

memscan: memscan.c
	gcc -o memscan memscan.c

test: memscan
	./memscan
