#!/usr/bin/python

###############################################################################
##                University of Hawaii, College of Engineering
##                ee205_animal_farm - EE 205 - Spring 2023
##
### Generate project statistics
###
### @file   stats.py
### @author Mark Nelson <marknels@hawaii.edu>
###############################################################################

import sys
import datetime
import subprocess
import git
from git	import Repo
import pathlib
import os
import locale
import re


GITHUB_USERNAME="marknelsonengineer"
GITHUB_REPO="ee205_animal_farm_reference"
STYLE="social"


locale.setlocale(locale.LC_ALL, '')  # Use '' for auto

# Get	the current	Git Repository	(as a	global)
repo = git.Repo(".")

# Initialize the sets of file	types	we	will process
h_src_files = set()
c_src_files = set()
cpp_src_files = set()
unit_test_files = set()
py_files	= set()
web_files =	set()
asm_files =	set()
markdown_files	= set()
image_files	= set()
other_files	= set()

number_of_unit_tests = 0
number_of_test_points = 0


def print_title():
	print( "Project Statistics" )
	print( "==================" )
	print( "" )


def print_shields_io_tags():
	repo1=f"![GitHub code size	in	bytes](https://img.shields.io/github/languages/code-size/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	repo2=f"![GitHub repo size](https://img.shields.io/github/repo-size/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	repo3=f"![GitHub contributors](https://img.shields.io/github/contributors/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"

	commit1=f"![GitHub commit activity](https://img.shields.io/github/commit-activity/w/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	commit2=f"![GitHub last	commit](https://img.shields.io/github/last-commit/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	issue1=f"![GitHub	issues](https://img.shields.io/github/issues-raw/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	issue2=f"![GitHub	closed issues](https://img.shields.io/github/issues-closed-raw/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	social1=f"![GitHub forks](https://img.shields.io/github/forks/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	social2=f"![GitHub Repo	stars](https://img.shields.io/github/stars/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"

	print()
	print( "## GitHub	Statistics"	)
#
#	print_number_of_commits
#
	print(  "| Repository								  | Commits							| Issues						  | Social							|"	)
	print(  "|-------------------------------------|---------------------------|-------------------------|---------------------------|"	)
	print( f"| {repo1} <br/> {repo2}	<br/>	{repo3} | {commit1} <br/> {commit2}	| {issue1} <br/> {issue2} | {social1} <br/> {social2}	|"	)


def print_tags():
# Use	the following to get	the tag/release history
# $ git tag	--sort=v:refname --format="%(tag)|%(contents)|%(creator)"
	print( "" )
	print( "## Tags" )

	tags = repo.tags

	if	len(tags) == 0:
		print( "No tags" )
	else:
		print( "| Tag | Date	| Author	|"	)
		print( "|-----|------|--------|"	)

#		git for-each-ref --sort=creatordate	--format	'|%(refname:strip=2)|%(creatordate)|%(authorname)|' refs/tags
		result =	subprocess.run(['git', 'for-each-ref',	'--sort=creatordate', '--format', '|%(refname:strip=2)|%(creatordate)|%(authorname)|',	'refs/tags'], stdout=subprocess.PIPE)
		print( result.stdout.decode('utf-8') )

	print( "" )



def print_date():
	print( "" )
	now =	datetime.datetime.now()
	print( "Automatically generated on " +	now.strftime("%c") )


def remove_comments( line, sep="#" ):
	lineStr = str(line, 'UTF-8')
	i = lineStr.find( sep ) # Find the posision of the separator
	if i >= 0 :
		lineStr = lineStr[ 0:i ]
	return lineStr.rstrip()


def process_script_files( rowName, fileList ):
	if( len( fileList ) == 0 ):
		return

	numFiles = len( fileList )

	totalBytes = 0
	totalLines = 0
	totalWorkingBytes = 0
	totalWorkingLines = 0
	for f in fileList:
		totalBytes += os.path.getsize( f )
		totalLines += sum(1 for line in open( f, "rb" ))
		for line in open( f, "rb" ):
			line = remove_comments( line, "#" )
			if( len( line ) <= 0 ):
				continue
			totalWorkingLines += 1
			totalWorkingBytes += len( line )
			# print( line )

	print( f"|{rowName}|{numFiles:n}|{totalLines:n}|{totalBytes:n}|{totalWorkingLines:n}|{totalWorkingBytes:n}|" )


def process_gcc_files( rowName, fileList	):
	if( len( fileList ) == 0 ):
		return

	numFiles = len( fileList )

	totalBytes = 0
	totalLines = 0
	totalWorkingBytes = 0
	totalWorkingLines = 0
	for f in fileList:
		totalBytes += os.path.getsize( f )
		totalLines += sum(1 for line in open( f, "rb" ))
		result = subprocess.run(['gcc', '-fpreprocessed', '-dD', '-w', '-E', f], stdout=subprocess.PIPE)

		allOutput = ""
#		lineStr = str(result, 'UTF-8')
		for char in result.stdout.decode('ascii'):
			allOutput += char

		for line in allOutput.splitlines():
			line = line.strip()

			if( len( line ) <= 0 ):
				continue
			totalWorkingLines += 1
			totalWorkingBytes += len( line )
#			print( line )

	print( f"|{rowName}|{numFiles:n}|{totalLines:n}|{totalBytes:n}|{totalWorkingLines:n}|{totalWorkingBytes:n}|" )


def process_text_files( rowName, fileList	):
	if( len( fileList ) == 0 ):
		return

	numFiles = len( fileList )

	totalBytes = 0
	totalLines = 0
	for f in fileList:
		totalBytes += os.path.getsize( f )
		totalLines += sum(1 for line in open( f, "rb" ))

	print( f"|{rowName}|{numFiles:n}|{totalLines:n}|{totalBytes:n}|n/a|n/a|" )


def process_data_files(	rowName,	fileList	):
	if( len( fileList ) == 0 ):
		return

	numFiles = len( fileList )

	totalBytes = 0
	for f in fileList:
		totalBytes += os.path.getsize( f )

	print( f"|{rowName}|{numFiles:n}|n/a|{totalBytes:n}|n/a|n/a|" )


def process_files_in_git():
	print( "| Type | Files | Lines | Bytes | Working Lines | Working Bytes |" )
	print( "|------|------:|------:|------:|--------------:|--------------:|" )

	global number_of_unit_tests
	global number_of_test_points

	for (_path, _stage), entry in repo.index.entries.items():
		# print( entry )

		if	not os.path.isfile( _path ):
			continue

		file_extension	= pathlib.Path( _path ).suffix
		# print( "Path = " + _path + "    Name = " + "XXXX" + "   Extension = " +	file_extension	)

		# If it's a C++ unit test, then count the test cases and test points
		if( _path[0:6] == "tests/" and file_extension in ['.h', '.c', '.cpp'] ):
			unit_test_files.add( _path )
			file = open( _path, "r")
			for line in file:
				if re.search("BOOST_.*_TEST_CASE", line):
					number_of_unit_tests += 1
					# print( line )
				if re.search("BOOST_CHECK|BOOST_REQUIRE", line):
					number_of_test_points += 1
					# print( line )


		elif( file_extension == '.h'	):
			h_src_files.add( _path )
		elif(	file_extension	==	'.c' ):
			c_src_files.add( _path	)
		elif(	file_extension	==	'.cpp' ):
			cpp_src_files.add(	_path )
		elif(	file_extension	==	'.py'	):
			py_files.add( _path )
		elif(	file_extension	in	['.html', '.css']	):
			web_files.add(	_path )
		elif(	file_extension	==	'.asm' ):
			asm_files.add(	_path )
		elif(	file_extension	==	'.md'	):
			markdown_files.add( _path )
		elif(	file_extension	in	['.svg',	'.png', '.ico', '.png',	'.jpg', '.jpeg', '.dot'] ):
			image_files.add( _path )
		else:
			other_files.add( _path )

	process_gcc_files( ".h Source", h_src_files )
	process_gcc_files( ".c Source", c_src_files )
	process_gcc_files( ".cpp Source", cpp_src_files )
	process_gcc_files( "Unit Tests", unit_test_files )
	process_script_files( "Python", py_files )
	process_text_files( "HTML", web_files )
	process_text_files( "Assembly", asm_files )
	process_text_files( "Markdown", markdown_files )
	process_data_files( "Images", image_files )
	process_data_files( "Other	Files", other_files )


def print_number_of_commits():
	print( "" )
	numberOfCommits = len(list(repo.iter_commits('HEAD')))
	print( f"Number of commits:  {numberOfCommits}" )


def print_test_stats():
	global number_of_unit_tests
	global number_of_test_points

	if( number_of_unit_tests == 0 ):
		return

	print( "" )
	print( f"Number of unit tests:  {number_of_unit_tests}" )
	print( "" )
	print( f"Number of test points:  {number_of_test_points}" )


# The	Main Program

with open('STATISTICS.md',	'w') as redirected_output:
	sys.stdout = redirected_output #	Redirect	stdout
	print_title()

	process_files_in_git()

	print_tags()

	print( "## Software Engineering Metrics" )

	print_number_of_commits()

	print_test_stats()

	print_shields_io_tags()

	print_date()
