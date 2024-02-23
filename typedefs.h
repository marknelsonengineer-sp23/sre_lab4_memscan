///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Consolidate all datatype definitions in one place.  In C, header files
/// *should* form Directional Acyclic Graphs (DAGs) but the preprocessor will
/// allow programmers to create cycles.  Good library definitions usually
/// stratify into 3 layers:
///   - Type declarations
///   - Type definitions (`class`, `struct`, etc.), forward declarations of
///     functions and variables
///   - Implementations
///
/// @file   typedefs.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>     // For unit64_t
#include <inttypes.h>   // For PRIx64


/// The type definition for PFNs (Page Frame Numbers)
///
/// Per [Kernel.org](https://www.kernel.org/doc/Documentation/vm/pagemap.txt)...
///
///     /proc/pid/pagemap lets a userspace process find out which physical
///     frame each virtual page is mapped to.  It contains one 64-bit value
///     for each virtual page
///
///        * Bits 0-54  page frame number (PFN) if present
///
/// Normally, this would be a pointer, but on 32-bit systems, we need 55 bits
/// to store the PFN.  Therefore, we are storing it in a `uint64_t`.
/// Memscan will never dereference the PFN.
typedef uint64_t pfn_t ;

/// The type definition for a constant PFN (Page Frame Number)
typedef const pfn_t const_pfn_t ;

/// The macro definition for formatting a PFN to print
///
/// `PRIx64` is defined in `<inttypes.h>`
#define PFN_FORMAT PRIx64

/// A constant where bits 0-54 are set
#define PFN_MASK 0x7FFFFFFFFFFFFFlu


/// The datatype of the pagemap data in #pagemapFilePath
typedef uint64_t pagemap_t ;

/// The size of each #pagemap_t entry
#define PAGEMAP_ENTRY sizeof( pagemap_t )


/// The datatype of the pagecount in #PAGECOUNT_FILE
typedef uint64_t pagecount_t ;

/// The size of each #pagecount_t entry
#define PAGECOUNT_ENTRY sizeof( pagecount_t )


/// The datatype of the pageflag data in #PAGEFLAG_FILE
typedef uint64_t pageflags_t ;

/// The size of each #pageflags_t entry
#define PAGEFLAG_ENTRY sizeof( pageflags_t )
