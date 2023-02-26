#!/usr/bin/python

# #############################################################################
#    University of Hawaii, College of Engineering
#    Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
#
### Increment the build number in versioh.h
###
### Usage:  Update the `version.h` file
###
### @file    update_version.py
### @author  Mark Nelson <marknels@hawaii.edu>
###############################################################################

import datetime
import argparse

## Path to version.h in C
VERSION_HEADER_FILE = "./version.h"

## Increments with major functional changes
major_version = 0

## Increments with minor functional changes and bugfixes
minor_version = 0

## Increments with bugfixes
patch_version = 0

## Monotonic counter that tracks the number of compilations
build_version = 0


# print("Starting update_version.py")


## Extract an integer from a line in a file
##
## If the source file had a like like:
##
## `#define VERSION_MINOR    4`
##
## then
##
## `extractInt( "#define VERSION_MINOR", aLine)` would return `4` as an `int`.
##
def extractInt( sKey:str, sLine:str ) -> int:
   i = sLine.find( sKey )  # Find the leading string

   if( i == -1 ):          # If not found, return -1
      return -1

   i2 = i + len( sKey )    # Get the remaining part of the string

   i3 = int( sLine[i2:] )  # Convert it to an int

   return i3


## Get the full version number (as a string) from `version.h`
##
## @returns A string like `1.4.0.2202`
def getFullVersion() -> str:
   global major_version
   global minor_version
   global patch_version
   global build_version

   with open ( VERSION_HEADER_FILE, "rt" ) as versionFile:  # open for reading text
      for aLine in versionFile:              # For each line, read to a string,
         # print(myline)                   # and print the string.
         i = extractInt( "#define VERSION_MAJOR", aLine )
         if i != -1:
            major_version = i

         i = extractInt( "#define VERSION_MINOR", aLine )
         if i != -1:
            minor_version = i

         i = extractInt( "#define VERSION_PATCH", aLine )
         if i != -1:
            patch_version = i

         i = extractInt( "#define VERSION_BUILD", aLine )
         if i != -1:
            build_version = i

   full_version = str(major_version)         \
                + "." + str( minor_version ) \
                + "." + str( patch_version ) \
                + "." + str( build_version ) \

   return( full_version )


## Update the build line in `version.h`
##
## If the old build line was: `#define VERSION_BUILD 1045`
##
## Then the new build line will be:  `#define VERSION_BUILD 1046`
##
## This routine rewrites `version.h`
def updateVersion( sKey:str, sFilename:str ):
   li = []

   with open( sFilename, "rt") as versionFile:
      for aLine in versionFile:
         i = aLine.find( sKey )
         if i >= 0:
            oldBuild = extractInt( sKey, aLine )
            newBuild = oldBuild + 1
            newLine = aLine.replace( str(oldBuild), str(newBuild) )
            li.append( newLine )
         else:
            li.append( aLine )

   j = 0
   with open( sFilename, "wt" ) as versionFile:
      while j < len(li):
         versionFile.write( li[j] )
         j += 1


# The main body of the program
updateVersion( "#define VERSION_BUILD", VERSION_HEADER_FILE )

## Holds the full version as a string: `1.4.0.2022`
full_version = getFullVersion()

print( full_version )
