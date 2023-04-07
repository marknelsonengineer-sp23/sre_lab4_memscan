#!/usr/bin/python3

#  #############################################################################
#  University of Hawaii, College of Engineering
#  Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
#
## Generate project statistics
##
## This code depends on the Git Python module.  To install it, login as `root`:
## ````
##      # apt-get install python3-pip
##      # pip install GitPython
## ````
##
## @see https://github.com/gitpython-developers/GitPython
## @see https://gitpython.readthedocs.io/en/stable/
##
## @file   stats.py
## @author Mark Nelson <marknels@hawaii.edu>
#  #############################################################################

import sys
import datetime
import subprocess
import git
import pathlib
import os
import locale
import re

## All output is redirected to this file
OUTPUT_FILE = "STATISTICS.md"

## The GitHub username that owns this repository
GITHUB_USERNAME = "marknelsonengineer-sp23"

## The GitHub repository name
GITHUB_REPO = "sre_lab4_memscan"

## Use this style when printing content from shields_io
##
## @see https://shields.io
STYLE = "social"


locale.setlocale(locale.LC_ALL, '')  # Use '' for auto

## The current Git Repository (as a global)
repo = git.Repo(".")

# Initialize the sets of file	types	we	will process

## The set of `.h` files from Git
h_src_files = set()

## The set of `.c` files from Git
c_src_files = set()

## The set of `.cpp` files from Git
cpp_src_files = set()

## The set of Boost Unit Test source files from Git
##
## @see https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/index.html
unit_test_files = set()

## The set of Python `.py` files from Git
py_files = set()

## The set of Web / HTML pages from Git
web_files = set()

## The set of Assembly Language source files from Git
asm_files = set()

## The set of Markdown `.md` files from Git
markdown_files = set()

## The set of image files from Git
image_files = set()

## The set of any other files in Git that are not in another set
other_files = set()

## The number of test cases #stats.unit_test_files
number_of_test_cases = 0

## The number of test assertions in #stats.unit_test_files
number_of_test_assertions = 0


## Print a title for #stats.OUTPUT_FILE
##
## @return Nothing
def print_title():
	print("Project Statistics")
	print("==================")
	print("")
	print("@brief Documents high-level statistics about the source code and")
	print("       git repository")
	print("")


## Print the [Shields.io](https://shields.io) tags
##
## @see https://shields.io
##
## @return Nothing
def print_shields_io_tags():
	repo1 = f"![GitHub code size	in	bytes](https://img.shields.io/github/languages/code-size/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	repo2 = f"![GitHub repo size](https://img.shields.io/github/repo-size/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	repo3 = f"![GitHub contributors](https://img.shields.io/github/contributors/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"

	commit1 = f"![GitHub commit activity](https://img.shields.io/github/commit-activity/w/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	commit2 = f"![GitHub last	commit](https://img.shields.io/github/last-commit/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	issue1 = f"![GitHub	issues](https://img.shields.io/github/issues-raw/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	issue2 = f"![GitHub	closed issues](https://img.shields.io/github/issues-closed-raw/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	social1 = f"![GitHub forks](https://img.shields.io/github/forks/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"
	social2 = f"![GitHub Repo	stars](https://img.shields.io/github/stars/{GITHUB_USERNAME}/{GITHUB_REPO}?style={STYLE})"

	print()
	print("## GitHub Statistics")

	# print_number_of_commits

	print("| Repository                           | Commits                   | Issues                  | Social                    |")
	print("|--------------------------------------|---------------------------|-------------------------|---------------------------|")
	print(f"| {repo1} <br/> {repo2} <br/> {repo3} | {commit1} <br/> {commit2} | {issue1} <br/> {issue2} | {social1} <br/> {social2} |")


## Print a table of Git tags (if any)
##
## @return Nothing
def print_tags():
	# Use	the following to get	the tag/release history
	# $ git tag	--sort=v:refname --format="%(tag)|%(contents)|%(creator)"
	print("")
	print("## Tags")

	tags = repo.tags

	if len(tags) == 0:
		print("No tags")
	else:
		print("| Tag | Date | Author |")
		print("|-----|------|--------|")

		# git for-each-ref --sort=creatordate --format '|%(refname:strip=2)|%(creatordate)|%(authorname)|' refs/tags
		result = subprocess.run(['git', 'for-each-ref', '--sort=creatordate', '--format', '|%(refname:strip=2)|%(creatordate)|%(authorname)|', 'refs/tags'], stdout=subprocess.PIPE)
		print(result.stdout.decode('utf-8'))

	print("")


## Print `Automatically generated on` and the date
##
## @return Nothing
def print_date():
	print("")
	now = datetime.datetime.now()
	print("Automatically generated on " + now.strftime("%c"))


## Find single-line comments (like `//` and `#`) and remove them
##
## @param line The line with a mix of working source and comments
## @param sep The single-line comment delimiter
## @return `line` stripped of any comments
def remove_comments(line, sep="#"):
	line_str = str(line, 'UTF-8')
	i = line_str.find(sep)  # Find the position of the separator
	if i >= 0:
		line_str = line_str[0:i]
	return line_str.rstrip()


## Process the set of shell-script files in `file_list` and report their statistics
##   - Count the number of files
##   - Count the total number of lines & working lines (no comments)
##   - Count the number of bytes & working bytes (no comments)
##
## Working source is text that is stripped of content after a `#`
##
## @param row_name The label under the `Type` heading in the main statistics table
## @param file_list The set of files to process
## @return Nothing
def process_script_files(row_name, file_list):
	if len(file_list) == 0:
		return

	num_files = len(file_list)

	total_bytes = 0
	total_lines = 0
	total_working_bytes = 0
	total_working_lines = 0
	for f in file_list:
		total_bytes += os.path.getsize(f)
		total_lines += sum(1 for _ in open(f, "rb"))
		for line in open(f, "rb"):
			line = remove_comments(line, "#")
			if len(line) <= 0:
				continue
			total_working_lines += 1
			total_working_bytes += len(line)
			# print( line )

	print(f"|{row_name}|{num_files:n}|{total_lines:n}|{total_bytes:n}|{total_working_lines:n}|{total_working_bytes:n}|")


## Process the set of gcc files in `file_list` and report their statistics
##   - Count the number of files
##   - Count the total number of lines & working lines (no comments)
##   - Count the number of bytes & working bytes (no comments)
##
## Use gcc's `-fpreprocessed` option to remove comments
##
## @param row_name The label under the `Type` heading in the main statistics table
## @param file_list The set of files to process
## @return Nothing
def process_gcc_files(row_name, file_list):
	if len(file_list) == 0:
		return

	num_files = len(file_list)

	total_bytes = 0
	total_lines = 0
	total_working_bytes = 0
	total_working_lines = 0
	for f in file_list:
		total_bytes += os.path.getsize(f)
		total_lines += sum(1 for _ in open(f, "rb"))
		result = subprocess.run(['gcc', '-fpreprocessed', '-dD', '-w', '-E', f], stdout=subprocess.PIPE)

		all_output = ""
		# lineStr = str(result, 'UTF-8')
		for char in result.stdout.decode('ascii'):
			all_output += char

		for line in all_output.splitlines():
			line = line.strip()

			if len(line) <= 0:
				continue
			total_working_lines += 1
			total_working_bytes += len(line)
			# print( line )

	print(f"|{row_name}|{num_files:n}|{total_lines:n}|{total_bytes:n}|{total_working_lines:n}|{total_working_bytes:n}|")


## Process the set of text files in `file_list` and report their statistics
##   - Count the number of files
##   - Count the number of lines
##   - Count the number of bytes
##
## @param row_name The label under the `Type` heading in the main statistics table
## @param file_list The set of files to process
## @return Nothing
def process_text_files(row_name, file_list):
	if len(file_list) == 0:
		return

	num_files = len(file_list)

	total_bytes = 0
	total_lines = 0
	for f in file_list:
		total_bytes += os.path.getsize(f)
		total_lines += sum(1 for _ in open(f, "rb"))

	print(f"|{row_name}|{num_files:n}|{total_lines:n}|{total_bytes:n}|n/a|n/a|")


## Process the set of data files in `file_list` and report their statistics
##   - Count the number of files
##   - Count the number of bytes
##
## @param row_name The label under the `Type` heading in the main statistics table
## @param file_list The set of files to process
## @return Nothing
def process_data_files(row_name, file_list):
	if len(file_list) == 0:
		return

	num_files = len(file_list)

	total_bytes = 0
	for f in file_list:
		total_bytes += os.path.getsize(f)

	print(f"|{row_name}|{num_files:n}|n/a|{total_bytes:n}|n/a|n/a|")


## Print the main Project Statistics table.
##   - For each file in the repository...
##     - Classify the file into the appropriate `Type` using file extensions,
##       folders, permissions, etc.
##     - Process each file type and print a summary
##
## @return Nothing
def process_files_in_git():
	print("| Type | Files | Lines | Bytes | Working Lines | Working Bytes |")
	print("|------|------:|------:|------:|--------------:|--------------:|")

	global number_of_test_cases
	global number_of_test_assertions

	for (_path, _stage), entry in repo.index.entries.items():
		# print( entry )

		if not os.path.isfile(_path):
			continue

		file_extension = pathlib.Path(_path).suffix
		# print( "Path = " + _path + "    Name = " + "XXXX" + "   Extension = " +	file_extension	)

		# If it's a C++ unit test, then count the test cases and test points
		if _path[0:6] == "tests/" and file_extension in ['.h', '.c', '.cpp']:
			unit_test_files.add(_path)
			file = open(_path, "r")
			for line in file:
				if re.search("BOOST_.*_TEST_CASE", line):
					number_of_test_cases += 1
					# print( line )
				if re.search("BOOST_CHECK|BOOST_REQUIRE", line):
					number_of_test_assertions += 1
					# print( line )

		elif file_extension == '.h':
			h_src_files.add(_path)
		elif file_extension == '.c':
			c_src_files.add(_path)
		elif file_extension == '.cpp':
			cpp_src_files.add(_path)
		elif file_extension == '.py':
			py_files.add(_path)
		elif file_extension in ['.html', '.css']:
			web_files.add(_path)
		elif file_extension == '.asm':
			asm_files.add(_path)
		elif file_extension == '.md':
			markdown_files.add(_path)
		elif file_extension in ['.svg', '.png', '.ico', '.png', '.jpg', '.jpeg', '.dot']:
			image_files.add(_path)
		else:
			other_files.add(_path)

	process_gcc_files(".h Source", h_src_files)
	process_gcc_files(".c Source", c_src_files)
	process_gcc_files(".cpp Source", cpp_src_files)
	process_gcc_files("Unit Tests", unit_test_files)
	process_script_files("Python", py_files)
	process_text_files("HTML", web_files)
	process_text_files("Assembly", asm_files)
	process_text_files("Markdown", markdown_files)
	process_data_files("Images", image_files)
	process_data_files("Other	Files", other_files)


## Print the number of commits to the repo
##
## @return Nothing
def print_number_of_commits():
	print("")
	number_of_commits = len(list(repo.iter_commits('HEAD')))
	print(f"Number of commits:  {number_of_commits}")


## Print the statistics related to Boost Tests in the repository
##
## @see https://www.boost.org/doc/libs/1_81_0/libs/test/doc/html/index.html
##
## @return Nothing
def print_test_stats():
	global number_of_test_cases
	global number_of_test_assertions

	if number_of_test_cases == 0:
		return

	print("")
	print(f"Number of test cases:  {number_of_test_cases}")
	print("")
	print(f"Number of test assertions:  {number_of_test_assertions}")


# The	Main Program

with open(OUTPUT_FILE,	'w') as redirected_output:
	## Redirect `stdout`
	sys.stdout = redirected_output
	print_title()

	process_files_in_git()

	print_tags()

	print("## Software Engineering Metrics")

	print_number_of_commits()

	print_test_stats()

	print_shields_io_tags()

	print_date()
