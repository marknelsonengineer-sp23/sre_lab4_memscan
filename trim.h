///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// General string-trimming functions
///
/// @file   trim.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

/// Remove all leading whitespace from `inString`
///
/// @param inString The string to trim
/// @return A string with the leading whitespace removed
extern char* trim_left( char inString[] ) ;

/// Remove all trailing whitespace from `inString`
///
/// @param inString The string to trim
/// @return A string with the trailing whitespace removed
extern char* trim_right( char inString[] ) ;

/// Remove all leading and trailing whitespace from `inString`
///
/// @param inString The string to trim
/// @return A string with the leading & trailing whitespace removed
extern char* trim_edges( char inString[] ) ;

/// Trim leading & trailing whitespace and collapse consecutive interior whitespace into `" "`
///
/// This function does the following:
///
/// @see https://en.cppreference.com/w/c/string/byte/isspace
///
///    - Trim the leading whitespace
///    - Trim trailing whitespace
///    - Replace consecutive whitespace characters inside the string with
///      a single `" "`
///
/// @param inString The string to trim
/// @return A fully trimmed string
extern char* trim( char inString[] ) ;
