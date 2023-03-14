///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// Comprehensive test of general string-trimming functions
///
/// @file   test_trim.cpp
/// @author Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

extern "C" {
   #include "../trim.h"
}

using namespace std ;

/// Size of the test string buffer
#define STRING_BUFFER_SIZE 1024

/// Allocate a character buffer for testing
char buffer[ STRING_BUFFER_SIZE ] ;

/// Clear the buffer and then set it
///
/// @param newContents Set the buffer to this string
void clearAndSetBuffer( const char newContents[] ) {
   memset( buffer, 'X', sizeof( buffer ) ) ;  // Fill it full of X's
   strncpy( buffer, newContents, sizeof( buffer ) ) ;
}



/* ****************************************************************************
   White Box Test Declarations

   These declarations may contain duplicate code or code that needs to be in
   sync with the code under test.  Because it's white box tests, it's on the
   tester to ensure the code is in sync.                                     */

enum CharClass {
   ASCII       ///< The character is a printable ASCII character
   ,WHITESPACE  ///< The character is neither an ASCII nor END character
   ,END         ///< The character is the null terminator `'\0'`
};

extern "C" CharClass classifyChar( char inChar ) ;

extern "C" char* shift_left( char* string_base, size_t current_position, size_t distance ) ;

/* ***************************************************************************/



BOOST_AUTO_TEST_SUITE( test_trim )

   BOOST_AUTO_TEST_CASE( test_trim_left ) {
      clearAndSetBuffer( "" );      BOOST_CHECK_EQUAL( trim_left( buffer ), "" );
      clearAndSetBuffer( "A" );     BOOST_CHECK_EQUAL( trim_left( buffer ), "A" );
      clearAndSetBuffer( "AB" );    BOOST_CHECK_EQUAL( trim_left( buffer ), "AB" );
      clearAndSetBuffer( "ABC" );   BOOST_CHECK_EQUAL( trim_left( buffer ), "ABC" );

      clearAndSetBuffer( " " );     BOOST_CHECK_EQUAL( trim_left( buffer ), "" );
      clearAndSetBuffer( " A" );    BOOST_CHECK_EQUAL( trim_left( buffer ), "A" );
      clearAndSetBuffer( " AB" );   BOOST_CHECK_EQUAL( trim_left( buffer ), "AB" );
      clearAndSetBuffer( " ABC" );  BOOST_CHECK_EQUAL( trim_left( buffer ), "ABC" );

      clearAndSetBuffer( "\t " );    BOOST_CHECK_EQUAL( trim_left( buffer ), "" );
      clearAndSetBuffer( "\t A" );   BOOST_CHECK_EQUAL( trim_left( buffer ), "A" );
      clearAndSetBuffer( "\t AB" );  BOOST_CHECK_EQUAL( trim_left( buffer ), "AB" );
      clearAndSetBuffer( "\t ABC" ); BOOST_CHECK_EQUAL( trim_left( buffer ), "ABC" );

      clearAndSetBuffer( " \t\f\n\r\v " );       BOOST_CHECK_EQUAL( trim_left( buffer ), "" );
      clearAndSetBuffer( " \t\f\n\r\v A  " );    BOOST_CHECK_EQUAL( trim_left( buffer ), "A  " );
      clearAndSetBuffer( " \t\f\n\r\v AB  " );   BOOST_CHECK_EQUAL( trim_left( buffer ), "AB  " );
      clearAndSetBuffer( " \t\f\n\r\v ABC  " );  BOOST_CHECK_EQUAL( trim_left( buffer ), "ABC  " );
   }

   BOOST_AUTO_TEST_CASE( test_trim_right ) {
      clearAndSetBuffer( "" );        BOOST_CHECK_EQUAL( trim_right( buffer ), "" );
      clearAndSetBuffer( "A" );       BOOST_CHECK_EQUAL( trim_right( buffer ), "A" );
      clearAndSetBuffer( "AB" );      BOOST_CHECK_EQUAL( trim_right( buffer ), "AB" );
      clearAndSetBuffer( "ABC" );     BOOST_CHECK_EQUAL( trim_right( buffer ), "ABC" );

      clearAndSetBuffer( " " );       BOOST_CHECK_EQUAL( trim_right( buffer ), "" );
      clearAndSetBuffer( "A " );      BOOST_CHECK_EQUAL( trim_right( buffer ), "A" );
      clearAndSetBuffer( "AB " );     BOOST_CHECK_EQUAL( trim_right( buffer ), "AB" );
      clearAndSetBuffer( "ABC " );    BOOST_CHECK_EQUAL( trim_right( buffer ), "ABC" );

      clearAndSetBuffer( " \t" );     BOOST_CHECK_EQUAL( trim_right( buffer ), "" );
      clearAndSetBuffer( "A \t" );    BOOST_CHECK_EQUAL( trim_right( buffer ), "A" );
      clearAndSetBuffer( "AB \t" );   BOOST_CHECK_EQUAL( trim_right( buffer ), "AB" );
      clearAndSetBuffer( "ABC \t" );  BOOST_CHECK_EQUAL( trim_right( buffer ), "ABC" );

      clearAndSetBuffer( " \t\f\n\r\v " );      BOOST_CHECK_EQUAL( trim_right( buffer ), "" );
      clearAndSetBuffer( " A \t\f\n\r\v " );    BOOST_CHECK_EQUAL( trim_right( buffer ), " A" );
      clearAndSetBuffer( " AB \t\f\n\r\v " );   BOOST_CHECK_EQUAL( trim_right( buffer ), " AB" );
      clearAndSetBuffer( " ABC \t\f\n\r\v " );  BOOST_CHECK_EQUAL( trim_right( buffer ), " ABC" );
   }


   BOOST_AUTO_TEST_CASE( test_trim_edges ) {
      clearAndSetBuffer( "" );        BOOST_CHECK_EQUAL( trim_edges( buffer ), "" );
      clearAndSetBuffer( "A" );       BOOST_CHECK_EQUAL( trim_edges( buffer ), "A" );
      clearAndSetBuffer( "AB" );      BOOST_CHECK_EQUAL( trim_edges( buffer ), "AB" );
      clearAndSetBuffer( "ABC" );     BOOST_CHECK_EQUAL( trim_edges( buffer ), "ABC" );

      clearAndSetBuffer( " " );       BOOST_CHECK_EQUAL( trim_edges( buffer ), "" );
      clearAndSetBuffer( " A" );      BOOST_CHECK_EQUAL( trim_edges( buffer ), "A" );
      clearAndSetBuffer( " AB" );     BOOST_CHECK_EQUAL( trim_edges( buffer ), "AB" );
      clearAndSetBuffer( " ABC" );    BOOST_CHECK_EQUAL( trim_edges( buffer ), "ABC" );

      clearAndSetBuffer( "\t " );     BOOST_CHECK_EQUAL( trim_edges( buffer ), "" );
      clearAndSetBuffer( "\t A" );    BOOST_CHECK_EQUAL( trim_edges( buffer ), "A" );
      clearAndSetBuffer( "\t AB" );   BOOST_CHECK_EQUAL( trim_edges( buffer ), "AB" );
      clearAndSetBuffer( "\t ABC" );  BOOST_CHECK_EQUAL( trim_edges( buffer ), "ABC" );

      clearAndSetBuffer( " \t\f\n\r\v " );     BOOST_CHECK_EQUAL( trim_edges( buffer ), "" );
      clearAndSetBuffer( " \t\f\n\r\v A" );    BOOST_CHECK_EQUAL( trim_edges( buffer ), "A" );
      clearAndSetBuffer( " \t\f\n\r\v AB" );   BOOST_CHECK_EQUAL( trim_edges( buffer ), "AB" );
      clearAndSetBuffer( " \t\f\n\r\v ABC" );  BOOST_CHECK_EQUAL( trim_edges( buffer ), "ABC" );

      clearAndSetBuffer( "A " );      BOOST_CHECK_EQUAL( trim_edges( buffer ), "A" );
      clearAndSetBuffer( "AB " );     BOOST_CHECK_EQUAL( trim_edges( buffer ), "AB" );
      clearAndSetBuffer( "ABC " );    BOOST_CHECK_EQUAL( trim_edges( buffer ), "ABC" );

      clearAndSetBuffer( "A \t" );    BOOST_CHECK_EQUAL( trim_edges( buffer ), "A" );
      clearAndSetBuffer( "AB \t" );   BOOST_CHECK_EQUAL( trim_edges( buffer ), "AB" );
      clearAndSetBuffer( "ABC \t" );  BOOST_CHECK_EQUAL( trim_edges( buffer ), "ABC" );

      clearAndSetBuffer( "A \t\f\n\r\v " );    BOOST_CHECK_EQUAL( trim_edges( buffer ), "A" );
      clearAndSetBuffer( "AB \t\f\n\r\v " );   BOOST_CHECK_EQUAL( trim_edges( buffer ), "AB" );
      clearAndSetBuffer( "ABC \t\f\n\r\v " );  BOOST_CHECK_EQUAL( trim_edges( buffer ), "ABC" );

      clearAndSetBuffer( " A " );    BOOST_CHECK_EQUAL( trim_edges( buffer ), "A" );
      clearAndSetBuffer( " AB " );   BOOST_CHECK_EQUAL( trim_edges( buffer ), "AB" );
      clearAndSetBuffer( " ABC " );  BOOST_CHECK_EQUAL( trim_edges( buffer ), "ABC" );

      clearAndSetBuffer( "\t A \t" );    BOOST_CHECK_EQUAL( trim_edges( buffer ), "A" );
      clearAndSetBuffer( "\t AB \t" );   BOOST_CHECK_EQUAL( trim_edges( buffer ), "AB" );
      clearAndSetBuffer( "\t ABC \t" );  BOOST_CHECK_EQUAL( trim_edges( buffer ), "ABC" );

      clearAndSetBuffer( " \t\f\n\r\v A \t\f\n\r\v " );    BOOST_CHECK_EQUAL( trim_edges( buffer ), "A" );
      clearAndSetBuffer( " \t\f\n\r\v AB \t\f\n\r\v " );   BOOST_CHECK_EQUAL( trim_edges( buffer ), "AB" );
      clearAndSetBuffer( " \t\f\n\r\v ABC \t\f\n\r\v " );  BOOST_CHECK_EQUAL( trim_edges( buffer ), "ABC" );
   }

   BOOST_AUTO_TEST_CASE( test_trim ) {
      clearAndSetBuffer( "" );     BOOST_CHECK_EQUAL( trim( buffer ), "" );
      clearAndSetBuffer( "A" );    BOOST_CHECK_EQUAL( trim( buffer ), "A" );
      clearAndSetBuffer( " A" );   BOOST_CHECK_EQUAL( trim( buffer ), "A" );
      clearAndSetBuffer( "A " );   BOOST_CHECK_EQUAL( trim( buffer ), "A" );
      clearAndSetBuffer( " A " );  BOOST_CHECK_EQUAL( trim( buffer ), "A" );

      clearAndSetBuffer( " " );      BOOST_CHECK_EQUAL( trim( buffer ), "" );
      clearAndSetBuffer( " A " );    BOOST_CHECK_EQUAL( trim( buffer ), "A" );
      clearAndSetBuffer( "  A" );    BOOST_CHECK_EQUAL( trim( buffer ), "A" );
      clearAndSetBuffer( "A  " );    BOOST_CHECK_EQUAL( trim( buffer ), "A" );
      clearAndSetBuffer( "  A  " );  BOOST_CHECK_EQUAL( trim( buffer ), "A" );

      clearAndSetBuffer( "A B" );    BOOST_CHECK_EQUAL( trim( buffer ), "A B" );
      clearAndSetBuffer( "A  B" );   BOOST_CHECK_EQUAL( trim( buffer ), "A B" );
      clearAndSetBuffer( "A   B" );  BOOST_CHECK_EQUAL( trim( buffer ), "A B" );

      clearAndSetBuffer( "AA BB" );    BOOST_CHECK_EQUAL( trim( buffer ), "AA BB" );
      clearAndSetBuffer( "AA  BB" );   BOOST_CHECK_EQUAL( trim( buffer ), "AA BB" );
      clearAndSetBuffer( "AA   BB" );  BOOST_CHECK_EQUAL( trim( buffer ), "AA BB" );

      clearAndSetBuffer( " A B " );        BOOST_CHECK_EQUAL( trim( buffer ), "A B" );
      clearAndSetBuffer( "  A  B  " );     BOOST_CHECK_EQUAL( trim( buffer ), "A B" );
      clearAndSetBuffer( "   A   B   " );  BOOST_CHECK_EQUAL( trim( buffer ), "A B" );

      clearAndSetBuffer( "A B C" );      BOOST_CHECK_EQUAL( trim( buffer ), "A B C" );
      clearAndSetBuffer( "A  B  C" );    BOOST_CHECK_EQUAL( trim( buffer ), "A B C" );
      clearAndSetBuffer( "A   B   C" );  BOOST_CHECK_EQUAL( trim( buffer ), "A B C" );

      clearAndSetBuffer( " A B C " );          BOOST_CHECK_EQUAL( trim( buffer ), "A B C" );
      clearAndSetBuffer( "  A  B  C  " );      BOOST_CHECK_EQUAL( trim( buffer ), "A B C" );
      clearAndSetBuffer( "   A   B   C   " );  BOOST_CHECK_EQUAL( trim( buffer ), "A B C" );

      clearAndSetBuffer( "\t\f\n\r\vA\t\f\n\r\vB\t\f\n\r\vC\t\f\n\r\v" );        BOOST_CHECK_EQUAL( trim( buffer ), "A B C" );
      clearAndSetBuffer( "\t\f\n\r\vAA\t\f\n\r\vBB\t\f\n\r\vCC\t\f\n\r\v" );     BOOST_CHECK_EQUAL( trim( buffer ), "AA BB CC" );
      clearAndSetBuffer( "\t\f\n\r\vAAA\t\f\n\r\vBBB\t\f\n\r\vCCC\t\f\n\r\v" );  BOOST_CHECK_EQUAL( trim( buffer ), "AAA BBB CCC" );
   }


   BOOST_AUTO_TEST_CASE( test_trim_classifyChar_whitebox ) {
      BOOST_CHECK_EQUAL( classifyChar( 'A' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( 'Z' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( 'a' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( 'z' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '0' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '!' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '\"' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '#' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '$' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '%' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '&' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '\'' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '(' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( ')' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '*' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '+' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( ',' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '-' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '.' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '/' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( ':' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( ';' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '<' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '=' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '>' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '?' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '@' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '[' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( ']' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '\\' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '^' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '_' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '`' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '{' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '}' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '|' ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( '~' ), ASCII );

      BOOST_CHECK_EQUAL( classifyChar( '\0' ), END );

      BOOST_CHECK_EQUAL( classifyChar( ' ' ),  WHITESPACE );
      BOOST_CHECK_EQUAL( classifyChar( '\t' ), WHITESPACE );
      BOOST_CHECK_EQUAL( classifyChar( '\n' ), WHITESPACE );
      BOOST_CHECK_EQUAL( classifyChar( '\v' ), WHITESPACE );
      BOOST_CHECK_EQUAL( classifyChar( '\f' ), WHITESPACE );
      BOOST_CHECK_EQUAL( classifyChar( '\r' ), WHITESPACE );
      BOOST_CHECK_EQUAL( classifyChar( 0x01 ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( 0x08 ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( 0x0E ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( 0x1F ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( 0x7F ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( 0x80 ), ASCII );
      BOOST_CHECK_EQUAL( classifyChar( 0xFF ), ASCII );
   }


   BOOST_AUTO_TEST_CASE( test_trim_shift_left_whitebox ) {
      // I'm deliberately choosing not to test the error conditions right now
      clearAndSetBuffer( "" );  BOOST_CHECK_EQUAL( shift_left( buffer, 0, 0 ), "" );
      clearAndSetBuffer( " " );  BOOST_CHECK_EQUAL( shift_left( buffer, 1, 1 ), "" );
      clearAndSetBuffer( "  " );  BOOST_CHECK_EQUAL( shift_left( buffer, 2, 2 ), "" );
      clearAndSetBuffer( "   " );  BOOST_CHECK_EQUAL( shift_left( buffer, 3, 3 ), "" );
      clearAndSetBuffer( "   A" );  BOOST_CHECK_EQUAL( shift_left( buffer, 3, 3 ), "A" );
      clearAndSetBuffer( "   A " );  BOOST_CHECK_EQUAL( shift_left( buffer, 3, 3 ), "A " );
      clearAndSetBuffer( "   AB" );  BOOST_CHECK_EQUAL( shift_left( buffer, 3, 3 ), "AB" );
      clearAndSetBuffer( "   AB CD " );  BOOST_CHECK_EQUAL( shift_left( buffer, 3, 3 ), "AB CD " );
      clearAndSetBuffer( "\t\t\tA" );  BOOST_CHECK_EQUAL( shift_left( buffer, 3, 3 ), "A" );
   }

BOOST_AUTO_TEST_SUITE_END()
