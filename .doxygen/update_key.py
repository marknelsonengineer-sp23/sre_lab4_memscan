#!/usr/bin/python

#  #############################################################################
#  University of Hawaii, College of Engineering
#  Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
#
## Update the key statement from the actual program's `--key` option
##
## @file   update_key.py
## @author Mark Nelson <marknels@hawaii.edu>
#  #############################################################################

import sys
import subprocess

## Command to execute that generates a key statement
INVOKE_USAGE = "./memscan --key"

## Copy the usage statement to this file
OUTFILE = "KEY.md"


# The Main Program

with open(OUTFILE, 'w') as redirected_output:
	## All output is redirected to this file
	sys.stdout = redirected_output  # Redirect stdout

	print("Key to Flags")
	print("============")
	print("````")

	## Run the program and collect the output
	s = subprocess.check_output(INVOKE_USAGE, shell=True)
	print(s.decode("utf-8"))

	print("````")
