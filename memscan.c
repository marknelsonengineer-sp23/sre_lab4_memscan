///////////////////////////////////////////////////////////////////////////////
//   University of Hawaii, College of Engineering
//   Lab 4 - Memory Scanner - EE 491F (Software Reverse Engineering) - Spr 2023
//
/// Basic memory scanner
///
/// @file   memscan.c
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <locale.h>  // For set_locale() LC_NUMERIC
#include <stdio.h>   // For printf() fprintf()
#include <stdlib.h>  // For EXIT_SUCCESS and EXIT_FAILURE

#include "config.h"   // For processOptions()
#include "maps.h"     // For readEntries() scanEntries()
#include "memscan.h"  // Just cuz
#include "pagemap.h"  // For closePagemap()


/// A basic memory scanner
///
/// @param argc The number of arguments passed to `memscan`
/// @param argv An array of arguments passed to `memscan`
/// @return The program's return code
int main( int argc, char* argv[] ) {
   processOptions( argc, argv ) ;

   char* sRetVal;
   sRetVal = setlocale( LC_NUMERIC, "" ) ;
   if( sRetVal == NULL ) {
      FATAL_ERROR( "Unable to set locale" ) ;
   }

   print_iomem() ;

   exit(0);

   readMaps() ;

   scanMaps() ;

   closePagemap() ;

   return EXIT_SUCCESS ;
} // main()
