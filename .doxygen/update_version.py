#!/usr/bin/python3

#  #############################################################################
#  University of Hawaii, College of Engineering
#  Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
#
## Increment the build number in `version.h`
##
## /// Use Semantic Versioning 2.0.0:  https://semver.org/
##
## Usage:  Update the `version.h` file
##
## @file   update_version.py
## @author Mark Nelson <marknels@hawaii.edu>
#  #############################################################################

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


## Extract an integer from a `line` in a file
##
## If the source file had a line like:
##
## `#define VERSION_MINOR    4`
##
## then
##
## `extract_int( "#define VERSION_MINOR", line)` would return `4` as an `int`.
##
## @param key The keyword to search for (needle)
## @param line The string to be searched (haystack)
## @return The integer that follows the keyword
def extract_int(key: str, line: str) -> int:
	i = line.find(key)    # Find the leading string

	if i == -1:        # If not found, return -1
		return -1

	i2 = i + len(key)     # Get the remaining part of the string

	i3 = int(line[i2:])   # Convert it to an int

	return i3


## Get the full version number (as a string) from `version.h`
##
## @return A string like `1.4.0+2202`
def get_full_version() -> str:
	global major_version
	global minor_version
	global patch_version
	global build_version

	with open(VERSION_HEADER_FILE, "rt") as versionFile:  # open for reading text
		for aLine in versionFile:              # For each line, read to a string,
			# print(myline)                     # and print the string.
			i = extract_int("#define VERSION_MAJOR", aLine)
			if i != -1:
				major_version = i

			i = extract_int("#define VERSION_MINOR", aLine)
			if i != -1:
				minor_version = i

			i = extract_int("#define VERSION_PATCH", aLine)
			if i != -1:
				patch_version = i

			i = extract_int("#define VERSION_BUILD", aLine)
			if i != -1:
				build_version = i

	full_version = str(major_version)
	full_version += "." + str(minor_version)
	full_version += "." + str(patch_version)
	full_version += "+" + str(build_version)

	return full_version


## Update the build line in `version.h`
##
## If the old build line was: `#define VERSION_BUILD 1045`
##
## Then the new build line will be:  `#define VERSION_BUILD 1046`
##
## This routine rewrites `version.h`
##
## @param key The keyword to search for (needle)
## @param filename The file to be searched (haystack)
## @return Nothing
def update_version(key: str, filename: str):
	li = []

	with open(filename, "rt") as versionFile:
		for line in versionFile:
			i = line.find(key)
			if i >= 0:
				old_build = extract_int(key, line)
				new_build = old_build + 1
				new_line = line.replace(str(old_build), str(new_build))
				li.append(new_line)
			else:
				li.append(line)

	j = 0
	with open(filename, "wt") as version_file:
		while j < len(li):
			version_file.write(li[j])
			j += 1


# The main body of the program
update_version("#define VERSION_BUILD", VERSION_HEADER_FILE)

print(get_full_version())
