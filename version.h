///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Program version and build tracker
///
/// Use Semantic Versioning 2.0.0:  https://semver.org/
///
/// #VERSION_BUILD is modified by update_version.py
///
/// @file    version.h
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Increments with major functional changes
#define VERSION_MAJOR    2

/// Increments with minor functional changes and bugfixes
#define VERSION_MINOR    2

/// Increments with bugfixes
#define VERSION_PATCH    0

/// Monotonic counter that tracks the number of compilations
#define VERSION_BUILD 10726

#ifndef STRINGIFY_VALUE
/// C preprocesor trick that converts values into strings at compile time
/// @see https://stackoverflow.com/questions/12844364/stringify-c-preprocess
#define STRINGIFY_VALUE(a)  STRINGIFY_VALUE_(a)

/// Second step of the stringify process
#define STRINGIFY_VALUE_(a) #a
#endif

/// The full version number as a narrow string
#define FULL_VERSION     STRINGIFY_VALUE( VERSION_MAJOR ) \
                     "." STRINGIFY_VALUE( VERSION_MINOR ) \
                     "." STRINGIFY_VALUE( VERSION_PATCH ) \
                     "+" STRINGIFY_VALUE( VERSION_BUILD )

/// The full version number as a wide string
#define FULL_VERSION_W L"" FULL_VERSION
