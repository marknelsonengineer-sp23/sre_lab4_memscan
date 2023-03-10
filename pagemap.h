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

#include <stdbool.h>  // For bool
#include <stddef.h>   // For size_t
#include <stdint.h>   // For uint8_t


/// Hold information about a physical page
///
/// Page data comes from several sources, including `pagemap`,
/// `kpagecount`, `kpageflags` and `iomem`.
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
struct PageInfo {
   void*    virtualAddress;  ///< The virtual address that this structure represents
   bool     valid;           ///< `true` if the data was successfully read and decoded from #PAGEMAP_FILE
   void*    pfn;             ///< The Page Frame Number (if present).  Valid when `!swapped`.
   uint8_t  swap_type;       ///< A 5-bit index into a table of swapfiles.  Valid when `swapped`.
   void*    swap_offset;     ///< A 50-bit index into a swapfile.  Valid when `swapped`.
   bool     soft_dirty;      ///< pte is soft-dirty (see [Documentation/vm/soft-dirty.txt](https://www.kernel.org/doc/Documentation/vm/soft-dirty.txt))
   bool     exclusive;       ///< Page exclusively mapped (since 4.2)
   bool     file_mapped;     ///< `true` if the page is `file_mapped`.  Pages are either mapped to a file (file_mapped) or not (anonymously mapped).  See https://stackoverflow.com/questions/13024087/what-are-memory-mapped-page-and-anonymous-page
   bool     swapped;         ///< `true` if the page is swapped
   bool     present;         ///< `true` if the page is present
   uint64_t page_count;      ///< From `/proc/kpagecount`.  This file contains a 64-bit count of the number of times each page is mapped, indexed by PFN.
   // There are 26 other flags to get into (later)
} ;


/// Get the size of a memory page in bytes.  Must not be less than 1.
///
/// @return The size of a memory page in bytes
extern size_t getPageSizeInBytes() ;


/// Get the size of a memory page in bits.  If the page size is `4,096` then
/// this would return `12`.
///
/// @return The size of a memory page in bits
extern unsigned char getPageSizeInBits() ;


/// Populate a #PageInfo record for a virtual address
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
///
/// @param vAddr The address to analyze (usually the starting address of a
///              page frame, but it doesn't have to be).
/// @return The #PageInfo structure for `vAddr`
extern struct PageInfo getPageInfo( void* vAddr ) ;


/// Print a #PageInfo record
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
///
/// @param page The page to print
extern void printPageInfo( const struct PageInfo* page ) ;


/// Close any open pagemap resources
extern void closePagemap() ;
