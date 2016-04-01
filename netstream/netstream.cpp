/*!
 * \file netstream.cpp
 * \author Peter C. Chapin
 * \brief Main implementation of the netstream library.
 */

#include <iostream>
#include <stdexcept>

#include "environ.h"
#include "netstream.hpp"

#if eOPSYS == eWIN32
#include <windows.h>
#endif

using namespace std;

namespace netstream {

    Init::Init( )
    {
        #if eOPSYS == eWIN32
        WSADATA     winsock_information;

        // Try to intialize Winsock.
        if ( WSAStartup( MAKEWORD( 2, 0 ), &winsock_information ) != 0 ) {
            throw runtime_error( "Unable to initialize Winsock" );
        }
        if ( !( LOBYTE( winsock_information.wVersion ) == 2 &&
                HIBYTE( winsock_information.wVersion ) == 0 ) ) {
            WSACleanup( );
            throw runtime_error( "Available Winsock does not support version 2.0" );
        }
        #endif
    }

    Init::~Init( )
    {
        #if eOPSYS == eWIN32
        WSACleanup( );
        #endif
    }

#ifdef BROKEN
    //
    // int netstream::overflow(int ch)
    //
    // Used to dump the buffer when it is full and a new character needs to be added.
    //
    int netstream::overflow( int ch )
    {
        // Send the characters that are buffered. (Errors? What are those?)
        channel.write( start, static_cast<int>( pptr( ) - pbase( ) ) );

        // Reset the put pointers.
        setp( pbase( ), epptr( ) );

        // Put the given character into the buffer.
        if ( ch != EOF ) {
            *pptr( ) = Ch;
            pbump( 1 );
        }

        return 0;
    }

    //
    // int netstream::underflow()
    //
    // Used to reload the buffer when it runs out of characters. This function is tricky because
    // of the need to copy the last character currently in the buffer to the putback position.
    // Also we need to deal with the fact that we might not get all the characters we want from
    // the network connection.
    //
    int netstream::underflow()
    {
        // I need to know where the buffer is, how it is allocated, and how I can get at it. I
        // need this information so that I can set up the get area pointers properly if they are
        // null (first call to under- flow()?) and when I get fewer than the desired number of
        // characters from the stream. My copy of the draft standard is quite lacking in its
        // description of that material.
    }
#endif

}
