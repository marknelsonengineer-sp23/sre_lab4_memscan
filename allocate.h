///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Allocate memory for `--local`, `--malloc` and `--shared` options
///
/// @file   allocate.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Allocate pre-scan memory
extern void allocatePreScanMemory() ;

/// Write to pre-scan memory
extern void fillPreScanMemory() ;

/// Release the pre-scan memory resources
extern void releasePreScanMemory() ;
