///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Comprehensive test of `pagemap`, `pageflags` and `pagecount`
///
/// We've developed a lot of code to test `pagemap`, `pageflags` and `pagecount`
/// data.  These tests are important as we want to understand and
/// characterize the data coming back from the Kernel.  As we run these tests
/// on more diverse systems, a test may fail and we can reason about why.
/// Without a rigorous test suite like this, we would not have visibility to
/// what the Kernel is doing with these flags.  In other words, we'd be accepting
/// the flags without testing them.
///
/// The end result is to write an expressive test like this:
///
///       BOOST_CHECK( validatePageInfo( &somePage, VALID, PRESENT, NOT_EXCLUSIVE, NEUTRAL_LRU, COUNT_IS_PLURAL, OTHERWISE_0 ) ) ;
///
/// ... where every value in PageInfo is considered.
///
/// @file   test_pagemap.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////
/// @cond Suppress Doxygen warnings

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <cstdarg>   // For va_list va_start, et. al.
#include <iostream>  // For cout endl

extern "C" {
   #include "../config.h"  // For getProgramName()
   #include "../pagemap.h"
}

using namespace std ;

enum FlagState {
    UNDEFINED    = 0
   ,FLAG_SET     = 128
   ,FLAG_CLEAR   = 129
   ,FLAG_NEUTRAL = 130
};


/// Make the PageInfoValidation enumerations
#define MAKE_ENUMS( flag )    flag, NOT_##flag, NEUTRAL_##flag \

/// Enumerations for identifying how to test the PageInfo structure
///
/// PageInfo members that are **not** tested include:
///   - The `soft_dirty` flag as it will always be neutral.
///   - The swap flags `swap_type`, `swap_offset` as the pages should never be
///     swapped and it would also be very difficult to reliably predict what
///     these values would be.  We are testing the `swap` flag.
///   - `pfn` as any value is possible.
enum PageInfoValidation {
    VALID              ,NOT_VALID,     NEUTRAL_VALID
   ,MAKE_ENUMS( EXCLUSIVE )     ,MAKE_ENUMS( FILE_MAPPED )
   ,MAKE_ENUMS( SWAPPED )       ,MAKE_ENUMS( PRESENT )
   ,MAKE_ENUMS( LOCKED )        ,MAKE_ENUMS( ERROR )
   ,MAKE_ENUMS( REFERENCED )    ,MAKE_ENUMS( UPTODATE )
   ,MAKE_ENUMS( DIRTY )         ,MAKE_ENUMS( LRU )
   ,MAKE_ENUMS( ACTIVE )        ,MAKE_ENUMS( SLAB )
   ,MAKE_ENUMS( WRITEBACK )     ,MAKE_ENUMS( RECLAIM )
   ,MAKE_ENUMS( BUDDY )         ,MAKE_ENUMS( MMAP )
   ,MAKE_ENUMS( ANON )          ,MAKE_ENUMS( SWAPCACHE )
   ,MAKE_ENUMS( SWAPBACKED )    ,MAKE_ENUMS( COMP_HEAD )
   ,MAKE_ENUMS( COMP_TAIL )     ,MAKE_ENUMS( HUGE )
   ,MAKE_ENUMS( UNEVICTABLE )   ,MAKE_ENUMS( HWPOISON )
   ,MAKE_ENUMS( NOPAGE )        ,MAKE_ENUMS( KSM )
   ,MAKE_ENUMS( THP )           ,MAKE_ENUMS( BALLOON )
   ,MAKE_ENUMS( ZERO_PAGE )     ,MAKE_ENUMS( IDLE )
   ,MAKE_ENUMS( PGTABLE )
   ,COUNT_IS_0         ,COUNT_IS_1
   ,COUNT_IS_PLURAL    ,COUNT_NEUTRAL
   ,OTHERWISE_1       = FLAG_SET
   ,OTHERWISE_0       = FLAG_CLEAR
   ,OTHERWISE_NEUTRAL = FLAG_NEUTRAL
   ,VALIDATION_LAST
} ;


bool lastArg( enum PageInfoValidation rule ) {
   switch( rule ) {
      case OTHERWISE_1:
      case OTHERWISE_0:
      case OTHERWISE_NEUTRAL:
         return true ;
      default:
         return false ;
   }
}


/// Make the setState() `case:` statements
#define MAKE_STATE( flag ) \
   case flag:             flagState[ flag ]       = FLAG_SET     ; break ;  \
   case NOT_##flag:       flagState[ flag ]       = FLAG_CLEAR   ; break ;  \
   case NEUTRAL_##flag:   flagState[ flag ]       = FLAG_NEUTRAL ; break


void setState( enum PageInfoValidation arg, enum FlagState flagState[] ) {
   switch( arg ) {
      MAKE_STATE( VALID ) ;
      MAKE_STATE( EXCLUSIVE ) ;     MAKE_STATE( FILE_MAPPED ) ;
      MAKE_STATE( SWAPPED ) ;       MAKE_STATE( PRESENT ) ;
      MAKE_STATE( LOCKED ) ;        MAKE_STATE( ERROR ) ;
      MAKE_STATE( REFERENCED ) ;    MAKE_STATE( UPTODATE ) ;
      MAKE_STATE( DIRTY ) ;         MAKE_STATE( LRU ) ;
      MAKE_STATE( ACTIVE ) ;        MAKE_STATE( SLAB ) ;
      MAKE_STATE( WRITEBACK ) ;     MAKE_STATE( RECLAIM ) ;
      MAKE_STATE( BUDDY ) ;         MAKE_STATE( MMAP ) ;
      MAKE_STATE( ANON ) ;          MAKE_STATE( SWAPCACHE ) ;
      MAKE_STATE( SWAPBACKED ) ;    MAKE_STATE( COMP_HEAD ) ;
      MAKE_STATE( COMP_TAIL ) ;     MAKE_STATE( HUGE ) ;
      MAKE_STATE( UNEVICTABLE ) ;   MAKE_STATE( HWPOISON ) ;
      MAKE_STATE( NOPAGE ) ;        MAKE_STATE( KSM ) ;
      MAKE_STATE( THP ) ;           MAKE_STATE( BALLOON ) ;
      MAKE_STATE( ZERO_PAGE ) ;     MAKE_STATE( IDLE ) ;
      MAKE_STATE( PGTABLE ) ;

      case COUNT_IS_0:      flagState[ COUNT_IS_0 ]      = FLAG_SET ; break ;
      case COUNT_IS_1:      flagState[ COUNT_IS_1 ]      = FLAG_SET ; break ;
      case COUNT_IS_PLURAL: flagState[ COUNT_IS_PLURAL ] = FLAG_SET ; break ;
      case COUNT_NEUTRAL:   flagState[ COUNT_NEUTRAL ]   = FLAG_SET ; break ;

      case OTHERWISE_1:
      case OTHERWISE_0:
      case OTHERWISE_NEUTRAL:
         for( int i = 0 ; i < VALIDATION_LAST ; i++ ) {
            if( flagState[ i ] == UNDEFINED ) {
               flagState[ i ] = (enum FlagState) arg ;
               // cout << "Setting flag " << i << " to " << arg << endl;
            }
         }
         break ;
      default:
         assert( false );
   }
}


#define CHECK_FLAG( flag, member )                                                             \
   /* cout << "flag = " #flag "   member = " #member << "   flagState[flag] = " << flagState[ flag ] << "   value = " << pPage->member << endl; */  \
   if( flagState[ flag ] == FLAG_SET && ! pPage->member ) {                                    \
      cout << getProgramName() << ":  Flag " #flag " is clear when it shouldn't be"  << endl;  \
      return false ;                                                                           \
   }                                                                                           \
   if( flagState[ flag ] == FLAG_CLEAR && pPage->member )   {                                  \
      cout << getProgramName() << ":  Flag " #flag " is set when it shouldn't be"  << endl;    \
      return false ;                                                                           \
   }


/// Callers must end this with one of the 3 OTHERWISE enums
/// NOLINTNEXTLINE( cert-dcl50-cpp ):  Using a C-style variadic
bool validatePageInfo( void* virtualAddress, PageInfo* pPage, ... ) {

   size_t pageMask = ~( ( 1 << getPageSizeInBits() ) - 1 ) ;

   if( (size_t) pPage->virtualAddress != ((size_t) virtualAddress & pageMask ) ) {
      cout << getProgramName() << ":  Page address " << pPage->virtualAddress
           << " is not in the same page as " << virtualAddress
           << "  mask = " << hex << pageMask << dec << endl ;
      return false ;
   }

   va_list args ;
   va_start( args, pPage ) ;
   enum PageInfoValidation result;
   enum FlagState flagState[ VALIDATION_LAST ] ;

   memset( flagState, 0, sizeof( flagState ) ) ;  // Set all flagStates to UNDEFINED

   do {
      result = (enum PageInfoValidation) va_arg( args, int );
      setState( result, flagState ) ;
   } while( !lastArg( result ) ) ;
   va_end( args );

   CHECK_FLAG( VALID, valid ) ;
   CHECK_FLAG( EXCLUSIVE, exclusive ) ;     CHECK_FLAG( FILE_MAPPED, file_mapped ) ;
   CHECK_FLAG( SWAPPED, swapped ) ;         CHECK_FLAG( PRESENT, present ) ;
   CHECK_FLAG( LOCKED, locked ) ;           CHECK_FLAG( ERROR, error ) ;
   CHECK_FLAG( REFERENCED, referenced ) ;   CHECK_FLAG( UPTODATE, uptodate ) ;
   CHECK_FLAG( DIRTY, dirty ) ;             CHECK_FLAG( LRU, lru ) ;
   CHECK_FLAG( ACTIVE, active ) ;           CHECK_FLAG( SLAB, slab ) ;
   CHECK_FLAG( WRITEBACK, writeback ) ;     CHECK_FLAG( RECLAIM, reclaim ) ;
   CHECK_FLAG( BUDDY, buddy ) ;             CHECK_FLAG( MMAP, mmap ) ;
   CHECK_FLAG( ANON, anon ) ;               CHECK_FLAG( SWAPCACHE, swapcache ) ;
   CHECK_FLAG( SWAPBACKED, swapbacked ) ;   CHECK_FLAG( COMP_HEAD, comp_head ) ;
   CHECK_FLAG( COMP_TAIL, comp_tail ) ;     CHECK_FLAG( HUGE, huge ) ;
   CHECK_FLAG( UNEVICTABLE, unevictable ) ; CHECK_FLAG( HWPOISON, hwpoison ) ;
   CHECK_FLAG( NOPAGE, nopage ) ;           CHECK_FLAG( KSM, ksm ) ;
   CHECK_FLAG( THP, thp ) ;                 CHECK_FLAG( BALLOON, balloon ) ;
   CHECK_FLAG( ZERO_PAGE, zero_page ) ;     CHECK_FLAG( IDLE, idle ) ;
   CHECK_FLAG( PGTABLE, pgtable ) ;

   if( flagState[ COUNT_IS_0 ] == FLAG_SET && pPage->page_count != 0 ) {
      cout << getProgramName() << ":  Count is " << pPage->page_count << " when it should be 0" << endl;
      return false ;
   }

   if( flagState[ COUNT_IS_1 ] == FLAG_SET && pPage->page_count != 1 ) {
      cout << getProgramName() << ":  Count is " << pPage->page_count << " when it should be 1" << endl;
      return false ;
   }

   if( flagState[ COUNT_IS_PLURAL ] == FLAG_SET && pPage->page_count <= 1 ) {
      cout << getProgramName() << ":  Count is " << pPage->page_count << " when it should be greater than 1" << endl;
      return false ;
   }

   return true ;
}


BOOST_AUTO_TEST_SUITE( test_pagemap )

   BOOST_AUTO_TEST_CASE( test_pagecount ) {
      BOOST_CHECK_EQUAL( sizeof( uint64_t ),    PAGECOUNT_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pagecount_t ), PAGECOUNT_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pfn_t ),       PAGECOUNT_ENTRY ) ;
   }

   BOOST_AUTO_TEST_CASE( test_pageflags ) {
      BOOST_CHECK_EQUAL( sizeof( uint64_t ),    PAGEFLAG_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pageflags_t ), PAGEFLAG_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pfn_t ),       PAGEFLAG_ENTRY ) ;
   }

   BOOST_AUTO_TEST_CASE( test_pagemap ) {
      BOOST_CHECK_EQUAL( sizeof( uint64_t ),    PAGEMAP_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pagemap_t ),   PAGEMAP_ENTRY ) ;
      BOOST_CHECK_EQUAL( sizeof( pfn_t ),       PAGEMAP_ENTRY ) ;
   }

   BOOST_AUTO_TEST_CASE( test_getPageSize ) {
      // Get a cue when we run across a non 4K page system
      BOOST_CHECK_EQUAL( getPageSizeInBytes(), 4096 ) ;    // Get into #IFDEFs if we find a nonstandard system
      BOOST_CHECK_EQUAL( getPageSizeInBits(), 12 ) ;

      BOOST_CHECK_EQUAL( getPageSizeInBytes(), 1 << getPageSizeInBits() ) ;
   }


   BOOST_AUTO_TEST_CASE( test_getPageInfo_local ) {
      // aLocalInt is at the top of the stack and should be mapped into memory
      int aLocalInt = 0xff ;
      struct PageInfo stackPage = getPageInfo( &aLocalInt, true ) ;
      BOOST_CHECK( validatePageInfo( &aLocalInt, &stackPage, VALID, PRESENT, COUNT_IS_1, EXCLUSIVE, UPTODATE, LRU, MMAP, ANON, SWAPBACKED, OTHERWISE_0 ) ) ;

      // 2 pages up from the top of the stack should be unmapped
      void* topOfStackPlusTwoPages = &aLocalInt + getPageSizeInBytes()*2 ;
      struct PageInfo stackPage2 = getPageInfo( topOfStackPlusTwoPages, true ) ;
      BOOST_CHECK( validatePageInfo( topOfStackPlusTwoPages, &stackPage2, VALID, NOT_PRESENT, COUNT_IS_0, OTHERWISE_0 ) ) ;
   }


   BOOST_AUTO_TEST_CASE( test_getPageInfo_malloc ) {
      char* smallMalloc = (char*) malloc( 64 ) ;
      smallMalloc[0] = (char) 0xff ;  // Make sure it's paged into memory
      struct PageInfo smallMallocPage = getPageInfo( smallMalloc, true ) ;
      BOOST_CHECK( validatePageInfo( smallMalloc, &smallMallocPage, VALID, PRESENT, COUNT_IS_1, EXCLUSIVE, UPTODATE, NEUTRAL_LRU, NEUTRAL_ACTIVE, MMAP, ANON, SWAPBACKED, OTHERWISE_0 ) ) ;
   }


   BOOST_AUTO_TEST_CASE( test_getPageInfo_readOnlyData ) {
      // This should come from libc's read-only section
      char* errorMessage = strerror( E2BIG ) ;
      char errorCharacter = errorMessage[0] ;  // Make sure it's paged into memory
      (void) errorCharacter ;  // Avoid an unused variable warning
      struct PageInfo errorMessagePage = getPageInfo( errorMessage, true ) ;
      BOOST_CHECK( validatePageInfo( errorMessage, &errorMessagePage, VALID, PRESENT, REFERENCED, FILE_MAPPED, COUNT_IS_PLURAL, NOT_EXCLUSIVE, UPTODATE, LRU, MMAP, OTHERWISE_0 ) ) ;
   }

BOOST_AUTO_TEST_SUITE_END()
/// @endcond
