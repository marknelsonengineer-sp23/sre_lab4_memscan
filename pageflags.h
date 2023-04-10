///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get flags for each physical page from `/proc/kpageflags`
///
/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt)...
///
///     This file contains a 64-bit set of flags for each page, indexed by PFN
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
/// @see https://www.kernel.org/doc/html/latest/admin-guide/mm/pagemap.html
///
/// @file   pageflags.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "pagemap.h"  // For PageInfo


/// Get physical frame flags from #PAGEFLAG_FILE and put them into #PageInfo
///
/// @param page The page to update
extern void getPageflags( struct PageInfo* page ) ;


/// Close any open pageflags resources
extern void closePageflags() ;
