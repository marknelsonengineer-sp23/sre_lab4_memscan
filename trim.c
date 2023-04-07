///////////////////////////////////////////////////////////////////////////////
//         University of Hawaii, College of Engineering
//         sre_lab4_memscan - EE 205 - Spr 2023
//
/// General string-trimming functions
///
/// There are *so many ways* to implement string trimming...
///   - Maximize clarity and understandability
///   - Maximize efficiency
///
/// This implementation is designed to try to balance both requirements.
///
/// @file    trim.c
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include <ctype.h>   // For isspace()
#include <stdio.h>   // For printf()
#include <string.h>  // For strlen()

#include "config.h"  // For ASSERT()
#include "trim.h"    // Just cuz


// Forward declaration
char* shift_left( char* string_base, size_t current_position, size_t distance ) ;
enum CharClass classifyChar( char inChar ) ;


char* trim_left( char inString[] ) {
   unsigned long i = 0 ;

   /// @API{ isspace, https://en.cppreference.com/w/c/string/byte/isspace }
   while( isspace( inString[ i ] ) ) {
      i++ ;
   }

   if( i == 0 ) {  // Nothing to do, so exit
      return inString ;
   }

   // Shift the contents of inString to the left by i characters
   shift_left( inString, i, i ) ;

   return inString ;
}


char* trim_right( char inString[] ) {
   /// @API{ isspace, https://en.cppreference.com/w/c/string/byte/isspace }
   unsigned long i = strlen( inString ) ;
   while( i > 0 ) {
      /// @API{ strlen, https://en.cppreference.com/w/c/string/byte/strlen }
      if( isspace( inString[ i-1 ] ) ) {
         i-- ;
         continue ;
      }
      // We have found a non-space character, so quit the loop
      break ;
   }
   inString[i] = '\0' ;

   return inString ;
}


char* trim_edges( char inString[] ) {
   /// This trim function does not modify the interior of the string
   trim_left( inString ) ;
   trim_right( inString ) ;
   return inString ;
}


/// The nature of a given character
///
/// @see https://en.cppreference.com/w/c/string/byte/isspace
/// @API{ enum, https://en.cppreference.com/w/c/language/enum }
enum CharClass {
   ASCII       ///< The character is a graphical ASCII character
  ,WHITESPACE  ///< The character is neither an ASCII nor END character
  ,END         ///< The character is the null terminator `'\0'`
} ;


/// Classify a character into a #CharClass
///
/// @param inChar The character to classify
/// @return       The #CharClass of `inChar`
enum CharClass classifyChar( const char inChar ) {
   if( inChar == '\0' ) {
      return END ;
   }

   /// @API{ isspace, https://en.cppreference.com/w/c/string/byte/isspace }
   if( isspace( inChar ) ) {
      return WHITESPACE ;
   }

   return ASCII ;
}


/// Shift characters in a string to the left
///
/// @param string_base      The base address of a null-terminated string
/// @param current_position The index into `string_base` where it will start moving characters
/// @param distance         The number of characters to shift left.  Distance must be `<= current_position`
/// @return                 The final string
char* shift_left( char* string_base, size_t current_position, const size_t distance ) {

   // printf( "Shifting c=%ld   d=%ld\n", current_position, distance ) ;

   /// `distance` must be >= 0
   // ASSERT( distance >= 0 ) ;    // Commented out right now because `size_t` is unsigned

   /// `distance` must be less than or equal to `current_position`
   ASSERT( distance <= current_position ) ;

   do {
      string_base [current_position - distance] = string_base[ current_position ] ;
   } while( string_base[ current_position++ ] != '\0' ) ;

   return string_base ;
}


char* trim( char inString[] ) {
   enum State { START, IN_WORD, ON_NOTICE, WORK_TO_DO, SHIFT, DONE } state = START ;
   size_t index    = 0 ;  // The current position
   size_t distance = 0 ;  // The distance

   #ifdef DEBUG
      printf( "Before=[%s]\n", inString) ;
   #endif

   while( state != DONE ) {
      #ifdef DEBUG
         switch( state ) {
            case START:      printf( "State=START   " ) ;       break ;
            case IN_WORD:    printf( "State=IN_WORD   " ) ;     break ;
            case ON_NOTICE:  printf( "State=ON_NOTICE   " ) ;   break ;
            case WORK_TO_DO: printf( "State=WORK_TO_DO   " ) ;  break ;
            case SHIFT:      printf( "State=SHIFT   " ) ;       break ;
            case DONE:       printf( "State=DONE   " ) ;        break ;
         }

         printf( "index=%lu   distance=%lu  inString=[%s]\n", index, distance, inString ) ;
      #endif

      switch( state ) {
         case START:
            switch( classifyChar( inString[index] )) {
               case END:
                  state = DONE ;
                  continue ;
               case ASCII:
                  state = IN_WORD ;
                  index++ ;
                  continue ;
               case WHITESPACE:
                  index++ ;
                  distance++ ;
                  state = WORK_TO_DO;
                  continue ;
            }
            break ;
         case IN_WORD:
            switch( classifyChar( inString[index] )) {
               case END:
                  state = DONE ;
                  continue ;
               case ASCII:
                  state = IN_WORD ;
                  index++ ;
                  continue ;
               case WHITESPACE:
                  inString[ index ] = ' ';  // Jam a space into this position
                  index++ ;
                  state = ON_NOTICE ;
                  continue ;
            }
            break ;
         case ON_NOTICE:
            switch( classifyChar( inString[index] )) {
               case END:
                  state = SHIFT ;
                  distance++ ;
                  continue ;
               case ASCII:
                  state = IN_WORD ;
                  index++ ;
                  continue ;
               case WHITESPACE:
                  index++ ;
                  distance=2 ;
                  state = WORK_TO_DO ;
                  continue ;
            }
            break;
         case WORK_TO_DO:
            switch( classifyChar( inString[index] )) {
               case ASCII:
               case END:
                  state = SHIFT;
                  continue ;
               case WHITESPACE:
                  index++ ;
                  distance++ ;
                  continue ;
            }
            break ;
         case SHIFT:
            switch( classifyChar( inString[index] )) {
               case END:
                  shift_left( inString, index, distance ) ;
                  index = index - distance ;
                  distance = 0 ;
                  state = DONE ;
                  continue ;
               case ASCII:
                  if( index - distance > 0 ) {  // If not the first character...
                     distance-- ;               // then make room for a space
                  }
                  shift_left( inString, index, distance ) ;
                  index = index - distance ;
                  distance = 0 ;
                  state = IN_WORD ;
                  continue ;
               case WHITESPACE:
                  ASSERT( false ) ;  // Raise an exception
                  continue ;
            }
            break ;
         case DONE:
            continue ;
      }
   }

   #ifdef DEBUG
      printf( "State=DONE   index=%lu   distance=%lu  inString=[%s]\n", index, distance, inString ) ;
   #endif

   return inString ;
}
