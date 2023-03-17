///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Utilities for miscellaneous data conversions
///
/// @file   convert.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stddef.h> // For size_t

/// Get an unsigned number from a string (with units)
///
/// Memscan has options like:  `--local=NUM[K|M|G]`.  This function parses
/// NUM with the units/scale.  The following are allowed:
///
/// | Unit |              Scale |
/// |:----:|-------------------:|
/// |   k  |              1,000 |
/// |   K  |              1,024 |
/// |   m  |          1,000,000 |
/// |   M  |        1024 * 1024 |
/// |   g  |      1,000,000,000 |
/// |   G  | 1024 * 1024 * 1024 |
///
/// @see https://en.wikipedia.org/wiki/Binary_prefix
///
/// @param inString The input string (this must be modifiable)
/// @return The number or a fatal error
extern unsigned long long stringToUnsignedLongLongWithScale( char* inString ) ;


/// Get an unsigned number from a string (with base prefix)
///
/// Memscan has options like `--threads` and `--map_addr`.  This function parses
/// the numbers with prefixes like `0x`, `0b` or no prefix and returns an
/// unsigned number.
///
/// | Prefix | Base |
/// |:------:|-----:|
/// |  [0b]  |    2 |
/// |        |   10 |
/// |  [0x]  |   16 |
///
/// [0b]: https://en.wikipedia.org/wiki/0B
/// [0x]: https://en.wikipedia.org/wiki/0X
///
/// @see https://en.cppreference.com/w/c/language/integer_constant
///
/// @param inString The input string (this must be modifiable)
/// @return The number or a fatal error
extern unsigned long long stringToUnsignedLongLongWithBasePrefix( char* inString ) ;
