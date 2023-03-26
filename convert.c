///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Utilities for miscellaneous data conversions
///
/// @file   convert.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <errno.h>   // For errno
#include <stdlib.h>  // For strtoull()
#include <string.h>  // For strlen() strcmp()

#include "config.h"  // For ASSERT() and FATAL_ERROR()
#include "convert.h" // Just cuz
#include "trim.h"    // For trim()


unsigned long long stringToUnsignedLongLongWithScale( char* inString ) {
   /// @NOLINTBEGIN(readability-magic-numbers): Magic numbers are allowed in this function

   ASSERT( inString != NULL ) ;
   trim( inString ) ;
   unsigned long long result = 0 ;
   char* strtolRemainder = NULL ;

   if( inString[0] == '-' ) {
      FATAL_ERROR( "negative number not allowed here" ) ;
   }

   ASSERT( strlen( inString ) > 0 ) ;

   errno = 0 ;
   result = strtoull( inString, &strtolRemainder, 10 ) ;
   // printf( "result=%lld  strtolRemainder=%p [%s]  errno=%d\n", result, strtolRemainder, strtolRemainder, errno ) ;

   // If there's an error or excess characters...
   if( result == 0 && errno != 0 ) {
      FATAL_ERROR( "illegal numeric option errno=%d", errno ) ;
   }

   if( strlen( strtolRemainder ) > 0 ) {
      if( strcmp( strtolRemainder, "k" ) == 0 ) {
         result = result * 1000 ;
      } else if( strcmp( strtolRemainder, "K" ) == 0 ) {
         result = result * 1024 ;
      } else if( strcmp( strtolRemainder, "m" ) == 0 ) {
         result = result * 1000 * 1000 ;
      } else if( strcmp( strtolRemainder, "M" ) == 0 ) {
         result = result * 1024 * 1024 ;
      } else if( strcmp( strtolRemainder, "g" ) == 0 ) {
         result = result * 1000 * 1000 * 1000 ;
      } else if( strcmp( strtolRemainder, "G" ) == 0 ) {
         result = result * 1024 * 1024 * 1024 ;
      } else {
         FATAL_ERROR( "illegal numeric suffix" ) ;
      }
   }

   // printf( "result=%ld\n", result ) ;
   return result ;
} // stringToUnsignedLongLongWithScale
// @NOLINTEND(readability-magic-numbers)


unsigned long long stringToUnsignedLongLongWithBasePrefix( char* inString ) {
   /// @NOLINTBEGIN(readability-magic-numbers): Magic numbers are allowed in this function

   ASSERT( inString != NULL ) ;
   trim( inString ) ;

   int base = 10 ;     /// @NOLINT(readability-magic-numbers):  Base 10 is a legit magic number
   if( inString[0] == '0' && inString[1] == 'x' ) {
      base = 16 ;      // NOLINT(readability-magic-numbers)
      inString += 2 ;  // Skip the '0x'
   }
   if( inString[0] == '0' && inString[1] == 'b' ) {
      base = 2 ;
      inString += 2 ;  // Skip the '0b'
   }
   errno = 0 ;
   char* strtolRemainder = NULL ;
   unsigned long long result = strtoull( inString, &strtolRemainder, base ) ;
   // printf( "result=%llu  strtolRemainder=%p [%s]  errno=%d\n", result, strtolRemainder, strtolRemainder, errno ) ;

   // If there's an error or excess characters...
   if( errno != 0 || strlen( strtolRemainder ) > 0 ) {
      FATAL_ERROR( "Illegal format for NUM or ADDR (optional prefix 0x or 0b)" ) ;
   }

   return result ;
} // stringToUnsignedLongLongWithBasePrefix
// @NOLINTEND(readability-magic-numbers)
