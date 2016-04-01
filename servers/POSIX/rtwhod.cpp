/*****************************************************************************
FILE          : rtwhod.cpp
LAST REVISION : 2007-02-13
PROGRAMMER    : (C) Copyright 2007 by the VTC Computer Club

This program is an RTWho daemon for Unix. It listens on port 9001 for
connections from RTWho clients and then reports a list of all users who
are logged into the local Unix system.

Please send comments or bug reports to

     VTC Computer Club
     c/o Peter C. Chapin
     Vermont Technical College
     Randolph Center, VT 05061
     vtc3@ecet.vtc.edu
*****************************************************************************/

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <map>
#include <string>
#include <vector>
using namespace std;

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "connection.h"
#include "sem.h"

#define BUFFER_SIZE 128

// The vector of user data.
typedef vector<string> userdata_t;
userdata_t       user_info;
spica::mutex_sem user_lock;

// Mapping from user name to real name.
map<string, string> real_names;

//
// This function reads the passwd file to find the real name of each user.
//
void read_passwd_file()
{
  struct passwd *pp;

  // Read the user database.
  while ((pp = getpwent()) != NULL) {
    real_names[pp->pw_name] = pp->pw_gecos;
  }
  endpwent();
}

//
// This function creates a pid file for this process.
//
void create_pidfile()
{
  ofstream outfile("/var/run/rtwhod.pid");

  if (!outfile) return;
  outfile << getpid() << "\n";
}


//
// This function erases the pid file. It should be called when this
// program terminates, but currently this program never terminates
// (cleanly).
//
void remove_pidfile()
{
  remove("/var/run/rtwhod.pid");
}

//
// This function detaches this process from its controlling terminal.
// See "Advanced Programming in the Unix Environment" by W. Richard
// Stevens, Chapter 13, for more information.
//
int daemonize()
{
  pid_t pid;
 
  if ((pid = fork()) < 0) return -1;
  else if (pid != 0) exit(0);         // Terminate parent process.

  setsid();    // Become session leader.
  chdir("/");  // Switch to "well known" directory.
  umask(0);    // Plan to set all output file permissions explicitly.
  return 0;
}

//
// This function monitors the currently logged in users and maintains a
// list of them for use by the connection processor threads. It is run
// in its own thread.
//
void *monitor_users(void *)
{
  FILE *who_command;
  char  line_buffer[BUFFER_SIZE];

  while (1) {
    sleep(10);
    // printf("Checking logged in users...\n");
    if ((who_command = popen("who", "r")) == NULL) {
      printf("Unable to open 'who' command!\n");
    }
    else {
      {
        spica::mutex_sem::grabber lock_manager(user_lock);
        user_info.clear();
        while (fgets(line_buffer, BUFFER_SIZE, who_command) != NULL) {
          char *name_end = strchr(line_buffer, ' ');
          if (name_end != NULL) {
            *name_end = '\0';
            user_info.push_back(string(line_buffer));
          }
        }
      }
      pclose(who_command);
    }
  }
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
 //   "    <name>solstice.vtc.edu</name>\n"
 //   "    <OS type="Linux">Fedora Core 4</OS>\n"
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
void *connection_processor(void *arg)
{
  netstream::Connection *client = static_cast<netstream::Connection *>(arg);

  spew_XMLheader(client);
  client->write("  <userList>\n", 13);
  {
    spica::mutex_sem::grabber lock_manager(user_lock);
    for (userdata_t::size_type i = 0; i < user_info.size(); ++i) {
      client->write("    <user name=\"", 16);
      client->write(user_info[i].c_str(), user_info[i].length());
      client->write("\" ", 2);
      map<string, string>::iterator name_info = real_names.find(user_info[i]);
      if (name_info != real_names.end()) {
        client->write("real-name=\"", 11);
        client->write(name_info->second.c_str(), name_info->second.length());
        client->write("\" ", 2);
        
      }
      client->write("/>\n", 3);
    }
  }
  client->write("  </userList>\n", 14);
  spew_XMLfooter(client);

  delete client;
  return NULL;
}


//
// This function is the main server loop. It accepts connections and
// starts a thread to handle each one. This function returns zero if
// it ended normally; one otherwise.
//
int main_loop(netstream::Server *acceptor)
{
  pthread_t  connection_thread; // ID of connection handling thread.

  while (1) {
    netstream::Connection *client = acceptor->accept();
    pthread_create(&connection_thread, NULL, connection_processor, client);
    pthread_detach(connection_thread);
  }
  return 0;
}


//
// Main Program
//
int main(int argc, char **argv)
{
  unsigned short      port = 0;          // Port number to listen on.
  pthread_t           monitoring_thread; // ID of user monitoring thread.

  if (argc != 2) {
    fprintf(stderr, "Usage: %s port\n", argv[0]);
    return 1;
  }
  port = atoi(argv[1]);

  // Do various initializations.
  read_passwd_file();
  if (daemonize() == -1) return 1;
  netstream::TCP_Server acceptor(port);
  if (!acceptor.isOk()) return 1;
  // create_pidfile();
  pthread_create(&monitoring_thread, NULL, monitor_users, NULL);

  // Deal with network connections.
  return main_loop(&acceptor);
}
