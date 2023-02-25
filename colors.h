///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// ANSI terminal color codes
///
/// @see https://en.wikipedia.org/wiki/ANSI_escape_code
/// @see https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
/// @see https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
///
/// @file   colors.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

#define ANSI_COLOR_RED     "\x1b[31m"   ///< The ANSI sequence for making the foreground color red
#define ANSI_COLOR_GREEN   "\x1b[32m"   ///< The ANSI sequence for making the foreground color green
#define ANSI_COLOR_YELLOW  "\x1b[33m"   ///< The ANSI sequence for making the foreground color yellow
#define ANSI_COLOR_BLUE    "\x1b[34m"   ///< The ANSI sequence for making the foreground color blue
#define ANSI_COLOR_MAGENTA "\x1b[35m"   ///< The ANSI sequence for making the foreground color magenta
#define ANSI_COLOR_CYAN    "\x1b[36m"   ///< The ANSI sequence for making the foreground color cyan
#define ANSI_COLOR_RESET   "\x1b[0m"    ///< The ANSI sequence to reset the colors
