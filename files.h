///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Process File I/O pre-scan options: `--block`, `--stream`, `--map_file` and
/// `--read`
///
/// @file   files.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Open the pre-scan files
extern void openPreScanFiles() ;

/// Read the pre-scan files
extern void readPreScanFiles() ;

/// Close the pre-scan files
extern void closePreScanFiles() ;
