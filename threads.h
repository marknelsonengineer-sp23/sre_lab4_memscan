///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Implement the `--threads` option
///
/// @file   threads.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Create worker threads for the `--threads` option
extern void createThreads() ;

/// Shutdown and release any thread resources
extern void closeThreads() ;
