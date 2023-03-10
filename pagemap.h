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
   bool     locked;          ///< From `/proc/kpageflags`.  The page is locked for exclusive access, e.g. by undergoing read/write IO.
   bool     error;           ///< From `/proc/kpageflags`.  IO error occurred.
   bool     referenced;      ///< From `/proc/kpageflags`.  The page has been referenced since last LRU list enqueue/requeue.
   bool     uptodate;        ///< From `/proc/kpageflags`.  The page has up-to-date data. ie. for file backed page: (in-memory data revision >= on-disk one)
   bool     dirty;           ///< From `/proc/kpageflags`.  The page has been written to, hence contains new data. i.e. for file backed page: (in-memory data revision > on-disk one)
   bool     lru;             ///< From `/proc/kpageflags`.  The page is in one of the LRU lists.
   bool     active;          ///< From `/proc/kpageflags`.  The page is in the active LRU list.
   bool     slab;            ///< From `/proc/kpageflags`.  The page is managed by the SLAB/SLOB/SLUB/SLQB kernel memory allocator. When compound page is used, SLUB/SLQB will only set this flag on the head page; SLOB will not flag it at all.
   bool     writeback;       ///< From `/proc/kpageflags`.  The page is being synced to disk.
   bool     reclaim;         ///< From `/proc/kpageflags`.  The page will be reclaimed soon after its pageout IO completed.
   bool     buddy;           ///< From `/proc/kpageflags`.  A free memory block managed by the buddy system allocator. The buddy system organizes free memory in blocks of various orders. An order N block has 2^N physically contiguous pages, with the BUDDY flag set for and _only_ for the first page.
   bool     mmap;            ///< From `/proc/kpageflags`.  A memory mapped page.
   bool     anon;            ///< From `/proc/kpageflags`.  A memory mapped page that is not part of a file.
   bool     swapcache;       ///< From `/proc/kpageflags`.  The page is mapped to swap space, i.e. has an associated swap entry.
   bool     swapbacked;      ///< From `/proc/kpageflags`.  The page is backed by swap/RAM.
   bool     comp_head;       ///< From `/proc/kpageflags`.  A compound page with order N consists of 2^N physically contiguous pages. A compound page with order 2 takes the form of "HTTT", where H donates its head page and T donates its tail page(s). The major consumers of compound pages are hugeTLB pages (HugeTLB Pages), the SLUB etc. memory allocators and various device drivers. However in this interface, only huge/giga pages are made visible to end users.
   bool     comp_tail;       ///< From `/proc/kpageflags`.  A compound page tail (see description above).
   bool     huge;            ///< From `/proc/kpageflags`.  This is an integral part of a HugeTLB page.
   bool     unevictable;     ///< From `/proc/kpageflags`.  The page is in the unevictable (non-)LRU list It is somehow pinned and not a candidate for LRU page reclaims, e.g. ramfs pages, shmctl(SHM_LOCK) and mlock() memory segments.
   bool     hwpoison;        ///< From `/proc/kpageflags`.  Hardware detected memory corruption on this page: don’t touch the data!
   bool     nopage;          ///< From `/proc/kpageflags`.  No page frame exists at the requested address.
   bool     ksm;             ///< From `/proc/kpageflags`.  Identical memory pages dynamically shared between one or more processes.
   bool     thp;             ///< From `/proc/kpageflags`.  Contiguous pages which construct transparent hugepages.
   bool     balloon;         ///< From `/proc/kpageflags`.  Balloon compaction page
   bool     zero_page;       ///< From `/proc/kpageflags`.  Zero page for pfn_zero or huge_zero page.
   bool     idle;            ///< From `/proc/kpageflags`.  The page has not been accessed since it was marked idle (see Idle Page Tracking).
   bool     pgtable;         ///< From `/proc/kpageflags`.  The page is in use as a page table.
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
