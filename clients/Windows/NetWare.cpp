/****************************************************************************
FILE          : NetWare.cpp
LAST REVISION : 2006-02-03
SUBJECT       : NetWare-related functions for RTWho.
PROGRAMMER    : (C) Copyright 2000-2006 by The VTC Computer Club

NOTE: to compile you must include in your project the following three
libraries: netwin32.lib, calwin32.lib, clxwin32.lib.

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

#include <stdio.h>

// Netware
#include <ntypes.h>
#include <nwfile.h>
#include <nwconnec.h>
#include <nwclxcon.h>
#include <nwmisc.h>
#include <nwnet.h>
#include <nwcalls.h>

#include "RTWho.h"

struct USER_DATA user_info[500];
int    user_count = 0;            // Counts how many users are logged in.

static NWCONN_HANDLE  connection_handle;

static int setup_network()
 {
    NWCONN_HANDLE  connection_handle;  // Connection handle to NIGHT.
    NWCCODE        ccode;       // Return code for most API functions.
    NWRCODE        rcode;       // Return code.

    // Initialize the Netware API functions and relize the network exists
    printf("Initializing.....\n");
    ccode = NWCallsInit(NULL, NULL);
    if(ccode) {
      printf("Error Initializing the network\nExiting....\n");
      exit(1);
    }

    // Ensure Server Exists
    printf("Finding Night.....\n");
    rcode = NWGetConnectionIDFromName(5, "NIGHT", &connection_handle);
    if (rcode) {
      printf("Can't Find NIGHT Error: %04X\n", rcode);
      exit(1);
    }
    return connection_handle;
 }

int get_users()
{
  NWCCODE           ccode;              // Return code for most API functions.
  NWRCODE           rcode;              // Return code.
  NWINET_ADDR       address;            // Address structure for NWGetInetAddr.
  int               connection_number;  // Generic connection number.
  char              MAC_address[12+1];  // Hold MAC address.
  char              location[80+1];     // Hold location of connection.
  int               counter = 0;

  // BUG: This should only be done the first time get_users is called.
  // (In the past get_users took a NetWare connection handle as a
  // parameter. However, that was changed to make the caller of
  // get_users network independent. Consequently get_users must now
  // initialize the network the first (and only) time it is invoked.
  // This change has not been fully reflected here.
  //
  connection_handle = setup_network();

  printf("Obtaining list of users logged in......\n");
  for(connection_number = 1; connection_number <= 500; ++connection_number) {
    NWGetConnectionInformation (
      /* > NWCONN_HANDLE    */ connection_handle,
      /* > NWCONN_NUM       */ connection_number,
      /* < pnstr8   objType */ (pnstr8)user_info[counter].login_name,
      /* < pnuint16 objType */ &user_info[counter].Object_Type,
      /* < pnuint16 objID   */ &user_info[counter].Object_ID,
      /* < pnuint8 loginTime*/ NULL);
    user_info[counter].connection_number = counter;
    ccode = NWGetInetAddr(connection_handle, connection_number,&address);
    if (ccode) {
      sprintf(MAC_address,"NOT_FOUND");
    }
    else {
      sprintf(MAC_address, "%2X%2X%2X%2X%2X%2X",
        address.netNodeAddr[0],
        address.netNodeAddr[1],
        address.netNodeAddr[2],
        address.netNodeAddr[3],
        address.netNodeAddr[4],
        address.netNodeAddr[5]);

      /* Now take all spaces and replace with zero's this is only done
         for compatability with myaddr. */
      for (int x = 0; x <= 12; x++) {
        if (MAC_address[x] == ' ') MAC_address[x] = '0';
      }
    }

    /* This is where we are going to fill in where they really are, if
       not in data file we will then use the mac address for the
       location. */
    location_test(MAC_address, location);
    strcpy(user_info[counter].MAC, MAC_address);
    strcpy(user_info[counter].location, location);

    /* Test so we don't print or handle the connections that have no one
       logged in on them. */
    if((strcmp(user_info[counter].login_name, "NOT_LOGGED_IN") == 0) ||
       (strcmp(user_info[counter].MAC, "NOT_FOUND") == 0)            ||
       (strcmp(user_info[counter].MAC, "000000000001") == 0)) {
    }
    else counter++;
  }

  user_count = counter;

  /* Sort the list of names. This should be done in the listview control
     to allow users to switch between ascending and descending but I
     don't have the time right now. */
  int cnt1;
  int cnt2;

  struct USER_DATA temp;
  for (cnt1 = 0; cnt1 < user_count; cnt1++) {
    for (cnt2 = 0; cnt2 < user_count; cnt2++) {
      if(strcmp(user_info[cnt1].login_name, user_info[cnt2].login_name) <= 0) {
        temp = user_info[cnt2];
        user_info[cnt2] = user_info[cnt1];
        user_info[cnt1] = temp;
      }
    }
  }
  return user_count;
}
