///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get the number of times each physical page is mapped from `/proc/kpagecount`
///
/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt)...
///
///     This file contains a 64-bit count of the number of times each page is
///     mapped, indexed by PFN.
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
/// @see https://www.kernel.org/doc/html/latest/admin-guide/mm/pagemap.html
///
/// @file   pagecount.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>  // For uint64_t

#include "config.h"  // For pfn_t


/// The datatype of the pagecount in `/proc/pagecount`
typedef uint64_t pagecount_t ;


/// The size of each #pagecount_t entry
#define PAGECOUNT_ENTRY sizeof( pagecount_t )


/// Get a 64-bit count of the number of times each physical page is mapped
///
/// @param pfn The Page Frame Number
/// @return    The number of times the PFN is mapped
extern pagecount_t getPagecount( const pfn_t pfn ) ;


/// Close any open pagecount resources
extern void closePagecount() ;
