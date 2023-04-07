#!/usr/bin/python3

#  #############################################################################
#  University of Hawaii, College of Engineering
#  Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
#
## Update the usage statement from the actual program's `--help` option
##
## A usage statement typically looks like this:
## ````
## wc [OPTION]... [FILE]...
##
## DESCRIPTION
## Print newline, word, and byte counts for each FILE.
##
## The options below may be used to select which counts are printed.
## -c, --bytes    print the byte counts
## -l, --lines    print the newline counts
## ````
##
## @file   update_usage.py
## @author Mark Nelson <marknels@hawaii.edu>
#  #############################################################################

import sys
import subprocess

## Command to execute that generates a usage statement
INVOKE_USAGE = "./memscan --help"

## Copy the usage statement to this file
OUTFILE = "USAGE.md"


# The Main Program

with open(OUTFILE, 'w') as redirected_output:
	## All output is redirected to this file
	sys.stdout = redirected_output  # Redirect stdout

	print("Usage")
	print("=====")
	print("")
	print("@brief Documents the command line parameters for memscan")
	print("")
	print("````")

	## Run the program and collect the output
	s = subprocess.check_output(INVOKE_USAGE, shell=True)
	print(s.decode("utf-8"))

	print("````")
