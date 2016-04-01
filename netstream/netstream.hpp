/*!
 * \file netstream.hpp
 * \author Peter C. Chapin
 * \brief Master header for the netstream library.
 *   
 * To simplify use of the netstream library this header is provided. It includes all other
 * netstream headers. Simply include netstream.hpp in your project to access any netstream
 * related component. Precompilation of this header is recommended.
 *
 * This header provides classes that implement an I/O-stream interface to connection oriented
 * network services. Also several utility classes are also provided. Any type of connection
 * supported by the class Connection family will work with a netstream. All the normal iostream
 * operations are also supported.
 */

#ifndef NETSTREAM_HPP
#define NETSTREAM_HPP

#include <iostream>
#include "connection.hpp"

/*!
 * \namespace netstream
 * \brief Name space enclosing the netstream library.
 * 
 * This name space contains all components in the netstream library. No third party or external
 * library components are included in the netstream name space.
 */
namespace netstream {

    //! Encapsulates network initialization tasks.
    class Init {
    public:
        //! Prepares the underlying network library for use.
        Init( );

        //! Shuts down the underlying network library.
       ~Init( );
    };

#ifdef BROKEN
    // class netbuf : public std::streambuf {
    //   public:
    //     netbuf( Connection &c ) : channel( c ) { }
    //
    //   private:
    //     Connection &channel;
    // };

    //
    // class netstream
    //
    // I'm not entirely happy with this format. I don't think streambuf should be an immediate
    // base class of netstream. It doesn't quite make sense. What probably *should* happen is:
    // an intermediate class needs to be defined, say "netstreambase," that contains a netbuf
    // object (see above) and that can serve as a base class for onetstream, inetstream, and
    // netstream. That class could also, perhaps, take responsibility for opening the network
    // connection. On the other hand, I sort of like having the connection object separate. That
    // emphasizes how netstreams can use any type of network connection. See the way the fstream
    // classes are defined for more specific ideas.
    //
    class netstream : public std::streambuf, public std::iostream {
    public:
        netstream( Connection &c ) : iostream( this ), channel( c ) { }

    private:
        Connection &channel;

        //
        // The following functions override the ones in streambuf and define how this object is
        // to get and store characters to the final stream. In this case, of course, we
        // read/write the connection object.
        //
        virtual int overflow( int ch );
        virtual int underflow();
    };
#endif

}

#endif
