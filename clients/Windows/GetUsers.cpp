/****************************************************************************
FILE          : GetUsers.cpp
LAST REVISION : 2007-03-25
SUBJECT       : The function that produces the current user list.
PROGRAMMER    : (C) Copyright 2000-2007 by The VTC Computer Club

LICENSE

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANT-
ABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

Please send comments or bug reports to

     VTC Computer Club
     c/o Peter C. Chapin
     Vermont Technical College
     Randolph Center, VT 05061
     vtc3@ecet.vtc.edu
****************************************************************************/

// Standard
#include <cstdlib>
#include <string>
#include <vector>
using namespace std;

// Platform
#include <windows.h>

// Xerces
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
namespace Xerces = xercesc_3_0;

// RTWho
#include <connection.h>
#include "config.h"
#include "RTWho.h"
#include "XMLHandler.h"

struct server {
  string         fqdn;
  unsigned short port;
};
static vector<server> server_list;   // List of servers to check.
static vector<UserData> user_info;  // Users actively being displayed.

//
// initalize_Xerces
//
// This function prepares the Xerces library. It returns true if successful,
// false otherwise. Right now this function doesn't appear to do a great
// deal. In the future we might want to set some additional options, etc.
//
static bool initialize_Xerces()
{
  try {
    Xerces::XMLPlatformUtils::Initialize();
  }
  catch (const Xerces::XMLException &) {
    // It might be nice to inform the user about the reason for the failure.
    MessageBox(NULL, "Unable to initialize Xerces library!", "RTWho v2.0", MB_OK|MB_ICONEXCLAMATION);
    return false;
  }
  return true;
  // In theory we should call Xerces::XMLPlatformUtils::Terminate() before
  // the program exits. However, with the current version of Xerces, that
  // call is documented as "optional." Finding a good place to put it is
  // a little awkward so we'll just ignore the issue for now.
}


//
// build_server_list
//
// This function parses the configuration data to construct a usable
// list of servers. This is necessary because in the configuration
// file all servers are listed on a single line. That line must be
// broken down into the individual servers.
//
static void build_server_list()
{
  string::size_type  current = 0, end_pos;
  string            *raw_list = spica::lookup_parameter("SERVERS");
  server             temp;

  if (raw_list == NULL) return;
  while ((end_pos = raw_list->find_first_of(" \t", current)) != string::npos) {
    string server_information(*raw_list, current, end_pos - current);
    string::size_type separator = server_information.find(':');
    temp.fqdn = server_information.substr(0, separator);
    temp.port = atoi(server_information.substr(separator + 1).c_str());
    server_list.push_back(temp);

    current = raw_list->find_first_not_of(" \t", end_pos);
  }

  string server_information(*raw_list, current);
  string::size_type separator = server_information.find(':');
  temp.fqdn = server_information.substr(0, separator);
  temp.port = atoi(server_information.substr(separator + 1).c_str());
  server_list.push_back(temp);
}

//
// process_server_data
//
// This function parses the XML document in server_document and indirectly
// populates the user list with the information it finds there.
//
static void process_server_data(const string &fqdn, const string &server_document)
{
  try {
    XMLHandler handler(fqdn, &user_info);

    // Prepare a memory buffer in the right form that contains the data.
    Xerces::MemBufInputSource memory_buffer(
      (const XMLByte *)server_document.c_str(), server_document.size(), "http://www.memory.buffer/"
    );

    // Create a SAX parser object and parse the memory buffer.
    Xerces::SAXParser parser;
    parser.setDocumentHandler(&handler);
    parser.setErrorHandler(&handler);
    parser.parse(memory_buffer);
  }
  // Eventually information about the specific error (in e) should be shown.
  catch (const Xerces::XMLException &) {
    MessageBox(NULL, "XMLException during parse of server data", "RTWho v2.0", MB_OK|MB_ICONEXCLAMATION);
  }
  catch (const Xerces::SAXParseException &) {
    MessageBox(NULL, "SAXParseException during parse of server data", "RTWho v2.0", MB_OK|MB_ICONEXCLAMATION);
  }
  catch (...) {
    MessageBox(NULL, "Unknown exception during parse of server data", "RTWho v2.0", MB_OK|MB_ICONEXCLAMATION);
  }
}

//
// get_from_servers
//
// This function communicates with servers running rtwhod to collect
// information about the users logged into those hosts. If this function
// encounters some kind of problem it simply ignores it and returns.
// Better error handling could be imagined (for example the user will
// want to know if there is some issue contacting an expected host).
//
static void get_from_servers()
{
  int                 count;
  const  int          BUFFER_SIZE = 128;
  char                buffer[BUFFER_SIZE + 1];
  string              server_document;

  // For each server to consider...
  for (vector<server>::size_type i = 0; i < server_list.size(); ++i) {
    server_document.clear();

    netstream::TCP_Connection server_connection(server_list[i].fqdn.c_str(), server_list[i].port);
    if (!server_connection.isOk()) {
      // Should probably inform the user about the failed connection but
      // we only want to do this the first time there is a failure and
      // not every time the servers are polled.
      continue;
    }

    // Keep reading until we've read it all.
    while ((count = server_connection.read(buffer, BUFFER_SIZE)) > 0) {
      buffer[count] = '\0';
      server_document.append(buffer);
    }

    // Process this data.
    process_server_data(server_list[i].fqdn, server_document);
  }
}

//
// This function constructs a fresh vector of users, drawing informatin from a
// variety of sources. It returns a pointer to the constructed vector for use
// by the GUI code.
//
vector<UserData> *get_users()
{
  static bool first_time = true;
  if (first_time) {
    if (!initialize_Xerces()) return &user_info;
    build_server_list();
    first_time = false;
  }

  user_info.clear();
  get_from_servers();
  return &user_info;
}
