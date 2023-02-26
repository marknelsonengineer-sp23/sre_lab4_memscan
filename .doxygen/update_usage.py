#!/usr/bin/python

###############################################################################
##   University of Hawaii, College of Engineering
##   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
##
### Update usage statement
###
### @file   update_usage.py
### @author Mark Nelson <marknels@hawaii.edu>
###############################################################################

import sys
import subprocess

INVOKE_USAGE="./memscan --help"

OUTFILE="USAGE.md"


# The	Main Program

with open(OUTFILE, 'w') as redirected_output:
	sys.stdout = redirected_output #	Redirect	stdout

	print( "Usage" )
	print( "=====" )
	print( "````" )

	s = subprocess.check_output(INVOKE_USAGE, shell = True)
	print(s.decode("utf-8"))

	print( "````" )
