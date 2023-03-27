///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Read, parse and process the `/proc/$PID/maps` file
///
/// @file   maps.h
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Hold the original (and some processed data) from each line of
/// `/proc/$PID/maps`
struct MapEntry {
   char*   szLine ;         ///< Malloc'd string buffer for the entire line.
                            ///  All of the string pointers in this struct will
                            ///  point to strings in this buffer after they've
                            ///  been tokenized
   char*  sAddressStart ;   ///< String pointer to the start of the address range
   char*  sAddressEnd ;     ///< String pointer to the end of the address range
   void*  pAddressStart ;   ///< Pointer to the start of the memory mapped region
   void*  pAddressEnd ;     ///< Pointer to the byte just *after* the end of
                            ///  the memory mapped region
   char*  sPermissions ;    ///< String pointer to the permissions
   char*  sOffset ;         ///< String pointer to the offset
   char*  sDevice ;         ///< String pointer to the device name
   char*  sInode ;          ///< String pointer to the iNode number
   char*  sPath ;           ///< String pointer to the path (may be `NULL`)
   bool   include ;         ///< `true` if the entry should be processed.  `false` if it should be excluded.
   size_t numBytes ;        ///< Number of bytes in this region
   size_t numPages ;        ///< The number of #PageInfo records allocated for this region
   struct PageInfo* pages ; ///< Pointer to a #PageInfo array
   size_t numBytesFound ;   ///< Number of #byteToScanFor bytes found in this region when #scanForByte is set
   double shannonEntropy ;  ///< Shannon Entropy of the region when #scanForShannon is set
} ;


/// Parse each line from #mapsFilePath and put the data into a #MapEntry
/// field.  This function makes heavy use of [strtok()][1].
///
/// Anything that changes #mapsFilePath should be done before calling
/// getMaps()
///
/// [1]: https://man7.org/linux/man-pages/man3/strtok_r.3.html
extern void getMaps() ;

/// Scan all readable memory regions.
///
/// Anything that changes the physical pagemap information should be done
/// before calling scanMaps()
extern void scanMaps() ;

/// Read data from `pagemap`, `pageflags` and `pagecount` and get information
/// about the physical pages referenced in `maps`
extern void readPagemapInfo() ;

/// Print the map, results of the scan and physical page info
extern void printMaps() ;
