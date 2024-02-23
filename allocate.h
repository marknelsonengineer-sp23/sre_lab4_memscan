///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Allocate and optionally populate memory for `--local`, `--numLocal`,
/// `--malloc`, `--numMalloc`, `--mem_map`, `--mapAddr` and `--fill`
///
/// @file   allocate.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once


/// Allocate pre-scan memory
///
/// -  `--local` memory is allocated in a separate thread from `main()` called
///    #localAllocationThread and synchronized with #mutex.  Local allocations
///    are stored in #localAllocations.
/// - `--malloc` memory is stored in #heapAllocations.
/// - `--map_mem` memory is stored in #mappedAllocation.
extern void allocatePreScanMemory() ;

/// Write to pre-scan memory when `--fill` is set
extern void fillPreScanMemory() ;

/// Release the pre-scan memory resources
extern void releasePreScanMemory() ;
