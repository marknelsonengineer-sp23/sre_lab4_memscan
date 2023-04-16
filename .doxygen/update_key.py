#!/usr/bin/python3

#  #############################################################################
#  University of Hawaii, College of Engineering
#  Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
#
## Update the key statement from the actual program's `--key` option
##
## @file   update_key.py
## @author Mark Nelson <marknels@hawaii.edu>
#  #############################################################################

import re
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
	print("")
	print("@brief Documents the key to the flags memscan outputs with the")
	print("       `--pfn` and `--phys` command line options")
	print("")
	print("````")

	## Run the program and collect the output
	s = subprocess.check_output(INVOKE_USAGE, shell=True)

	## Remove ANSI color strings
	ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
	s_without_ansi = ansi_escape.sub('', s.decode("utf-8"))
	print(s_without_ansi)

	print("````")
