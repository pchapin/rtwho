/*****************************************************************************
FILE          : rtwhod-win.cpp
LAST REVISION : 2007-02-15
PROGRAMMER    : (C) Copyright 2007 by the VTC Computer Club

This program is an RTWho daemon for Windows. It listens on the given port
for connections from RTWho clients and then reports a list of all users who
are logged into the current Windows domain.

Please send comments or bug reports to

     VTC Computer Club
     c/o Peter C. Chapin
     Vermont Technical College
     Randolph Center, VT 05061
     vtc3@ecet.vtc.edu
*****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <fstream>
#include <string>
#include <vector>

using namespace std;

#include <process.h>
#include <windows.h>

#include <netstream.hpp>
#include "sem.h"

#define BUFFER_SIZE 128

// The vector of user data.
typedef vector<string> userdata_t;
userdata_t       user_info;
spica::mutex_sem user_lock;

//
// This function monitors the currently logged in users and maintains a
// list of them for use by the connection processor threads. It is run
// in its own thread.
//
unsigned __stdcall monitor_users(void *)
{
  while (1) {
    Sleep(10000);
    printf("Checking logged in users...\n");
    { 
      spica::mutex_sem::grabber lock_manager(user_lock);
      user_info.clear();
      // FILL IN HERE WITH ACTUAL USER INFORMATION!!
    }
  }

  return 0;
}

//
// This function outputs the XML boiler plate.
//
void spew_XMLheader(netstream::Connection *client)
{
  static const char *header =
    "<?xml version =\"1.0\" encoding=\"UTF-8\"?>\n"
    "<RTWho xmlns=\"http://solstice.vtc.edu/XML/RTWho_0.0\"\n"
    "       xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
    "       xsi:schemaLocation=\"http://solstice.vtc.edu/XML/RTWho_0.0 RTWho.xsd\">\n"
    "  <serverInformation>\n"
 //   "    <name>vtc.vsc.edu</name>\n"
 //   "    <OS type="Windows">WinXP</OS>\n"
    "  </serverInformation>\n";

  client->write(header, strlen(header));
}

//
// This function wraps up the XML output.
//
void spew_XMLfooter(netstream::Connection *client)
{
  static const char *footer = "</RTWho>\n";

  client->write(footer, strlen(footer));
}

//
// This function deals with a single connection. A new thread is created
// for each connection that arrives and that thread executes this
// function. Thus there might be multiple activations of this function
// occuring at the same time.
//
unsigned __stdcall connection_processor(void *arg)
{
  netstream::Connection *client = static_cast<netstream::Connection *>(arg);

  spew_XMLheader(client);
  client->write("  <userList>\n", 13);
  {
    spica::mutex_sem::grabber lock_manager(user_lock);
    for (userdata_t::size_type i = 0; i < user_info.size(); ++i) {
      client->write("    <user name=\"", 16);
      client->write(user_info[i].c_str(), user_info[i].length());
      client->write("\"/>\n", 4);
    }
  }
  client->write("  </userList>\n", 14);
  spew_XMLfooter(client);

  delete client;
  return 0;
}


//
// This function is the main server loop. It accepts connections and
// starts a thread to handle each one. This function returns zero if
// it ended normally; one otherwise.
//
int main_loop(netstream::Server *acceptor)
{
  HANDLE  connection_thread; // ID of connection handling thread.

  while (1) {
    netstream::Connection *client = acceptor->accept( );
    connection_thread = reinterpret_cast<HANDLE>(
        _beginthreadex(NULL, 0, connection_processor, client, 0, NULL));
    CloseHandle(connection_thread);
  }
  return 0;
}


//
// Main Program
//
int main(int argc, char **argv)
{
  unsigned short      port = 0;          // Port number to listen on.
  HANDLE              monitoring_thread; // ID of user monitoring thread.

  if (argc != 2) {
    fprintf(stderr, "Usage: %s port\n", argv[0]);
    return 1;
  }
  port = atoi(argv[1]);

  netstream::Init network_initializer;
  netstream::TCP_Server acceptor(port);
  if (!acceptor.isOk()) {
    WSACleanup();
    return 1;
  }

  monitoring_thread = reinterpret_cast<HANDLE>(
      _beginthreadex(NULL, 0, monitor_users, NULL, 0, NULL));
  CloseHandle(monitoring_thread);

  // Deal with network connections.
  int rc = main_loop(&acceptor);

  return rc;
}
