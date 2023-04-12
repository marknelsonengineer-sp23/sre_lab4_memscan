///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Get more information about a virtual address
///
/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt)...
///
///     This file lets a userspace process find out which physical frame each
///     virtual page is mapped to.  It contains one 64-bit value for each
///     virtual page.
///
///     * Bits 0-54  page frame number (PFN) if present
///     * Bits 0-4   swap type if swapped
///     * Bits 5-54  swap offset if swapped
///     * Bit  55    pte is soft-dirty (see Documentation/vm/soft-dirty.txt)
///     * Bit  56    page exclusively mapped (since 4.2)
///     * Bits 57-60 zero
///     * Bit  61    page is file-page or shared-anon (since 3.5)
///     * Bit  62    page swapped
///     * Bit  63    page present
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
/// @see https://www.kernel.org/doc/html/latest/admin-guide/mm/pagemap.html
/// @see http://fivelinesofcode.blogspot.com/2014/03/how-to-translate-virtual-to-physical.html
/// @see https://stackoverflow.com/questions/5748492/is-there-any-api-for-determining-the-physical-address-from-virtual-address-in-li/45128487#45128487
///
/// @file   pagemap.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdbool.h>    // For bool
#include <stddef.h>     // For size_t
#include <stdint.h>     // For uint8_t unit64_t
#include <stdio.h>      // For FILE

#include "typedefs.h"   // For pfn_t pagecount_t


/// Holds information about a physical page
///
/// Page data comes from several sources, including `pagemap`,
/// `kpagecount`, `kpageflags` and `iomem`.
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
struct PageInfo {
   void*    virtualAddress ;  ///< The virtual address that this structure represents
   bool     valid ;           ///< `true` if the data was successfully read and decoded from #pagemapFilePath
   pfn_t    pfn ;             ///< The Page Frame Number (if present).  Valid when `!swapped`.
   uint8_t  swap_type ;       ///< A 5-bit index into a table of swapfiles.  Valid when `swapped`.
   void*    swap_offset ;     ///< A 50-bit index into a swapfile.  Valid when `swapped`.
   bool     soft_dirty ;      ///< PTE is soft-dirty (see [Documentation/vm/soft-dirty.txt](https://www.kernel.org/doc/Documentation/vm/soft-dirty.txt))
   bool     exclusive ;       ///< Page exclusively mapped
   bool     file_mapped ;     ///< `true` if the page is `file_mapped`.  Pages are either mapped to a file (file_mapped) or not (anonymously mapped).  See https://stackoverflow.com/questions/13024087/what-are-memory-mapped-page-and-anonymous-page
   bool     swapped ;         ///< `true` if the page is swapped
   bool     present ;         ///< `true` if the page is present
   pagecount_t page_count ;   ///< From `/proc/kpagecount`.  This file contains a 64-bit count of the number of times each page is mapped, indexed by PFN.
   bool     locked ;          ///< From `/proc/kpageflags`.  The page is locked for exclusive access, e.g. by undergoing read/write IO.
   bool     error ;           ///< From `/proc/kpageflags`.  IO error occurred.
   bool     referenced ;      ///< From `/proc/kpageflags`.  The page has been referenced since last LRU list enqueue/requeue.
   bool     uptodate ;        ///< From `/proc/kpageflags`.  The page has up-to-date data. ie. for file backed page: (in-memory data revision >= on-disk one)
   bool     dirty ;           ///< From `/proc/kpageflags`.  The page has been written to, hence contains new data. i.e. for file backed page: (in-memory data revision > on-disk one)
   bool     lru ;             ///< From `/proc/kpageflags`.  The page is in one of the LRU lists.
   bool     active ;          ///< From `/proc/kpageflags`.  The page is in the active LRU list.
   bool     slab ;            ///< From `/proc/kpageflags`.  The page is managed by the SLAB/SLOB/SLUB/SLQB kernel memory allocator. When compound page is used, SLUB/SLQB will only set this flag on the head page; SLOB will not flag it at all.
   bool     writeback ;       ///< From `/proc/kpageflags`.  The page is being synced to disk.
   bool     reclaim ;         ///< From `/proc/kpageflags`.  The page will be reclaimed soon after its pageout IO completed.
   bool     buddy ;           ///< From `/proc/kpageflags`.  A free memory block managed by the buddy system allocator. The buddy system organizes free memory in blocks of various orders. An order N block has 2^N physically contiguous pages, with the BUDDY flag set for and _only_ for the first page.
   bool     mmap ;            ///< From `/proc/kpageflags`.  A memory mapped page.
   bool     anon ;            ///< From `/proc/kpageflags`.  A memory mapped page that is not part of a file.
   bool     swapcache ;       ///< From `/proc/kpageflags`.  The page is mapped to swap space, i.e. has an associated swap entry.
   bool     swapbacked ;      ///< From `/proc/kpageflags`.  The page is backed by swap/RAM.
   bool     comp_head ;       ///< From `/proc/kpageflags`.  A compound page with order N consists of 2^N physically contiguous pages. A compound page with order 2 takes the form of "HTTT", where H donates its head page and T donates its tail page(s). The major consumers of compound pages are hugeTLB pages (HugeTLB Pages), the SLUB etc. memory allocators and various device drivers. However in this interface, only huge/giga pages are made visible to end users.
   bool     comp_tail ;       ///< From `/proc/kpageflags`.  A compound page tail (see description above).
   bool     huge ;            ///< From `/proc/kpageflags`.  This is an integral part of a HugeTLB page.
   bool     unevictable ;     ///< From `/proc/kpageflags`.  The page is in the unevictable (non-)LRU list It is somehow pinned and not a candidate for LRU page reclaims, e.g. ramfs pages, shmctl(SHM_LOCK) and mlock() memory segments.
   bool     hwpoison ;        ///< From `/proc/kpageflags`.  Hardware detected memory corruption on this page: don’t touch the data!
   bool     nopage ;          ///< From `/proc/kpageflags`.  No page frame exists at the requested address.
   bool     ksm ;             ///< From `/proc/kpageflags`.  Kernel Samepage Merging:  Identical memory pages dynamically shared between one or more processes.
   bool     thp ;             ///< From `/proc/kpageflags`.  Contiguous pages which construct transparent hugepages.
   bool     balloon ;         ///< From `/proc/kpageflags`.  Balloon compaction page
   bool     zero_page ;       ///< From `/proc/kpageflags`.  Zero page for pfn_zero or huge_zero page.
   bool     idle ;            ///< From `/proc/kpageflags`.  The page has not been accessed since it was marked idle (see Idle Page Tracking).
   bool     pgtable ;         ///< From `/proc/kpageflags`.  The page is in use as a page table.
   double   shannon ;         ///< The Shannon Entropy of the page when `--shannon` is set.
} ;


/// Get the size of a memory page in bytes.  Must not be less than 1.
///
/// CPUs normally support the following page sizes
///
/// | Architecture | Page Sizes                           |
/// |--------------|--------------------------------------|
/// | x86          | 4K, 2M, 1G                           |
/// | AArch64      | 4K, 64K                              |
/// | ia64         | 4K, 8K, 64K, 256K, 1M, 4M, 16M, 256M |
/// | ppc64        | 4K, 16M                              |
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
/// @param vAddr    The address to analyze (usually the starting address of a
///                 page frame, but it doesn't have to be).
/// @param okToRead `true` if the page can be read.  `false` if this should
///                 just find flags
/// @return The #PageInfo structure for `vAddr`
extern struct PageInfo getPageInfo( void* vAddr, const bool okToRead ) ;


/// Print a #PageInfo record with the Page Frame Number
///
/// @see https://www.kernel.org/doc/Documentation/vm/pagemap.txt
///
/// @param page The page to print
extern void printFullPhysicalPage( const struct PageInfo* page ) ;


/// Print a summary of de-duplicated #PageInfo records without the Page Frame Number
///
/// @param page     An array of pages to print
/// @param numPages The number of elements in the page array
extern void printPageSummary(
        const struct PageInfo page[]
       ,const size_t          numPages ) ;


/// Close any open pagemap resources
extern void closePagemap() ;

/// Print the key to `--pfn` and `--phys` physical page flags
///
/// @param outStream The output stream (usually `stderr` or `stdout`) to print to
extern void printKey( FILE* outStream ) ;
