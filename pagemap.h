///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get more information about a virtual address
///
/// @see http://fivelinesofcode.blogspot.com/2014/03/how-to-translate-virtual-to-physical.html
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
///
/// @file   pagemap.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Hold all of the information about the physical page
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
struct PhysicalPage {
   size_t pfn;  ///< Page Frame Number (if present)
   bool   soft_dirty_bit;  ///< pte is soft-dirty (see [Documentation/vm/soft-dirty.txt](https://www.kernel.org/doc/Documentation/vm/soft-dirty.txt))
   bool   page_exclusively_mapped;  ///< Page exclusively mapped (since 4.2)
   bool   file_mapped;  ///< `true` if the page is `file_mapped`.  Pages are either mapped to a file (file_mapped) or not (anonymously mapped).  See https://stackoverflow.com/questions/13024087/what-are-memory-mapped-page-and-anonymous-page
   bool   swapped;  ///< `true` if the page is swapped
   bool   present;  ///< `true` if the page is present
   size_t page_count;  ///< From /proc/kpagecount.  This file contains a 64-bit count of the number of times each page is mapped, indexed by PFN.
   // There are 26 other flags to get into (later)
} ;


/// Get the size of a memory page in bytes.  Must not be less than 1.
///
/// @return The size of a memory page in bytes
extern size_t getPageSizeInBytes() ;


/// Get the size of a memory page in bits.  If the page size is 4,096 then
/// this would return 12.
///
/// @return The size of a memory page in bits
extern unsigned char getPageSizeInBits() ;


extern void doPagemap( void* pAddr ) ;


/// Close any open pagemap resources
extern void closePagemap() ;
