/*!
 * \file connect.cpp
 * \author Peter C. Chapin
 * \brief Implementation of the connection classes.
 */

#include <cstring>
#include <stdexcept>

#include "environ.h"
#include "connection.hpp"

using namespace std;

#if eOPSYS == ePOSIX

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

namespace netstream {

    //------------------------------------
    //           TCP_Connection
    //------------------------------------

    TCP_Connection::TCP_Connection( const char *host, unsigned short port )
        : is_open( false )
    {
        struct hostent *host_info;
        struct sockaddr_in server_address;

        if( ( connection_handle = socket( PF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
            failure_reason = "Unable to create communications socket";
            return;
        }
        
        if( ( host_info = gethostbyname( host ) ) == 0 ) {
            failure_reason = "Unable to resolve remote host's IP address";
            close( connection_handle );
            return;
        }

        memset( &server_address, 0, sizeof( server_address ) );
        server_address.sin_family = host_info->h_addrtype;
        server_address.sin_port   = htons( port );
        memcpy( &server_address.sin_addr,
                host_info->h_addr_list[0],
                host_info->h_length );

        if( connect( connection_handle,
                     reinterpret_cast<sockaddr *>( &server_address ),
                     sizeof( server_address ) ) == -1 ) {
            failure_reason = "Unable to connect to remote host";
            close( connection_handle );
            return;
        }

        is_open = true;
    }

    TCP_Connection::~TCP_Connection( )
    {
        if ( is_open ) close( connection_handle );
    }

    int TCP_Connection::read( char *buffer, int count )
    {
        if( !is_open ) return( 0 );
        return( ::read( connection_handle, buffer, count) );
    }

    int TCP_Connection::write( const char *buffer, int count )
    {
        if( !is_open ) return( 0 );
        return( ::write( connection_handle, buffer, count ) );
    }

    //--------------------------------
    //           TCP_Server
    //--------------------------------

    TCP_Server::TCP_Server( unsigned short port )
        : is_listening( false )
    {
        struct sockaddr_in server_address;

        // Create the server socket.
        if( ( listen_handle = socket( PF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
            failure_reason = "Unable to create communications socket";
            return;
        }

        // Prepare the server socket address structure.
        memset( &server_address, 0, sizeof( server_address ) );
        server_address.sin_family      = AF_INET;
        server_address.sin_port        = htons( port );
        server_address.sin_addr.s_addr = htonl( INADDR_ANY );

        // Bind the server socket.
        if( bind( listen_handle,
                  reinterpret_cast<sockaddr *>( &server_address ),
                  sizeof( server_address ) ) == -1 ) {
            failure_reason = "Unable to bind server socket";
            close( listen_handle );
            return;
        }

        // Allow incoming connections.
        if( listen( listen_handle, 32 ) == -1 ) {
            failure_reason = "Unable to listen";
            close( listen_handle );
            return;
        }

        is_listening = true;
    }

    TCP_Server::~TCP_Server( )
    {
        if( is_listening ) close( listen_handle );
    }

    TCP_Connection *TCP_Server::accept( )
    {
        if( !is_listening )
            throw runtime_error( "Use of non-listening socket in call to netstream::TCP_Server::accept( )" );

        int    connection_handle; // Socket handle for each connection.
        struct sockaddr_in  client_address;    // Remote address.
        socklen_t           client_length;     // Size of remote address.

        memset( &client_address, 0, sizeof( client_address ) );
        client_length = sizeof( client_address );

        // Block until a client comes along.
        connection_handle =
            ::accept( listen_handle,
                      reinterpret_cast<sockaddr *>( &client_address ),
                      &client_length );
        if( connection_handle == -1 ) {
            failure_reason = "Unable to accept connection";
            throw runtime_error( "Failure in netstream::TCP_Server::accept( )" );
        }

        // It would be better to return a smart pointer of some kind.
        return( new TCP_Connection( connection_handle ) );
    }
}

#endif

#if eOPSYS == eWIN32

#include <windows.h>

namespace netstream {

    //------------------------------------
    //           TCP_Connection
    //------------------------------------

    TCP_Connection::TCP_Connection( const char *host, unsigned short port )
        : is_open( false )
    {
        struct hostent *ip_address;
        struct sockaddr_in server_address;

        // Create a TCP socket.
        connection_handle = socket( PF_INET, SOCK_STREAM, 0 );
        if( connection_handle == INVALID_SOCKET ) {
            failure_reason = "Unable to create communications socket";
            return;
        }

        // Look up the host's address.
        ip_address = gethostbyname( host );
        if( ip_address == NULL) {
            failure_reason = "Unable to resolve remote host's IP address";
            closesocket( connection_handle );
            return;
        }

        // Prepare the address of the server.
        memset( &server_address, 0, sizeof( server_address ) );
        server_address.sin_family = AF_INET;
        server_address.sin_port   = htons( port );
        memcpy( &server_address.sin_addr.S_un.S_addr, ip_address->h_addr_list[0], 4 );

        // Connect to the server.
        if( connect( connection_handle, reinterpret_cast<sockaddr *>( &server_address ), sizeof( server_address ) ) == SOCKET_ERROR ) {
            failure_reason = "Unable to connect to remote host";
            closesocket( connection_handle );
            return;
        }

        is_open = true;
    }

    TCP_Connection::~TCP_Connection( )
    {
        if( is_open ) closesocket( connection_handle );
    }

    int TCP_Connection::read( char *buffer, int count )
    {
        if( !is_open ) return( 0 );
        return( recv( connection_handle, buffer, count, 0 ) );
    }

    int TCP_Connection::write( const char *buffer, int count )
    {
        if( !is_open ) return( 0 );
        return( send( connection_handle, buffer, count, 0 ) );
    }

    //--------------------------------
    //           TCP_Server
    //--------------------------------

    TCP_Server::TCP_Server( unsigned short port )
        : is_listening( false )
    {
        struct sockaddr_in server_address;

        // Create the server socket.
        if( ( listen_handle = socket( PF_INET, SOCK_STREAM, 0 ) ) == INVALID_SOCKET ) {
            failure_reason = "Unable to create communications socket";
            return;
        }

        // Prepare the server socket address structure.
        memset( &server_address, 0, sizeof( server_address ) );
        server_address.sin_family      = AF_INET;
        server_address.sin_port        = htons( port );
        server_address.sin_addr.S_un.S_addr = htonl( INADDR_ANY );

        // Bind the server socket.
        if( bind( listen_handle, reinterpret_cast<sockaddr *>( &server_address ), sizeof( server_address ) ) == SOCKET_ERROR ) {
            failure_reason = "Unable to bind server socket";
            closesocket( listen_handle );
            return;
        }

        // Allow incoming connections.
        if( listen( listen_handle, 32 ) == SOCKET_ERROR ) {
            failure_reason = "Unable to listen";
            closesocket( listen_handle );
            return;
        }

        is_listening = true;
    }

    TCP_Server::~TCP_Server( )
    {
        if( is_listening ) closesocket( listen_handle );
    }

    TCP_Connection *TCP_Server::accept( )
    {
        if( !is_listening )
            throw runtime_error( "Use of non-listening socket in call to netstream::TCP_Server::accept( )" );

        SOCKET              connection_handle; // Socket handle for each connection.
        struct sockaddr_in  client_address;    // Remote address.
        int                 client_length;     // Size of remote address.

        memset(&client_address, 0, sizeof(client_address));
        client_length = sizeof(client_address);

        // Block until a client comes along.
        connection_handle = ::accept( listen_handle, reinterpret_cast<sockaddr *>( &client_address ), &client_length );
        if( connection_handle == INVALID_SOCKET ) {
            failure_reason = "Unable to accept connection";
            throw runtime_error( "Failure in netstream::TCP_Server::accept( )" );
        }

        // It would be better to return a smart pointer of some kind.
        return( new TCP_Connection( connection_handle ) );
    }

}

#endif
