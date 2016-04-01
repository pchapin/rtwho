/****************************************************************************
FILE          : rtwho.h
LAST REVISION : 2007-03-25
SUBJECT       : Declaration of global functions for RTWho
PROGRAMMER    : (C) Copyright 2000-2007 by the VTC Computer Club

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

#ifndef RTWHO_H
#define RTWHO_H

#include <string>
#include <vector>

// =================
// Macro definitions
// =================

#define MBOX_CAPTION "RTWho v2.0"

// ================
// Type definitions
// ================

struct UserData {
  std::string  login_name;      // Name of user logged in.
  std::string  real_name;       // The real name of the user.
  std::string  host_name;       // Name of host on which user is logged in.
  std::string  location;        // Description of user's physical location.
  std::string  object_type;     // ???
};

// =================================
// Functions defined in location.cpp
// =================================

// This function that will open the data file containing the MAC
// addresses and the locations to be assigned to them. It returns the
// location or, if no location is found, the MAC address. NOTE: Location
// can be up to 80 characters
//
int location_test(char *MAC_address, char *location);

// Reads in the card map file containing the locations linked to the MAC
// addresses. It then fills the two lists MAC_address_list and
// location_list.
//
int location_read();

// =================================
// Functions defined in GetUsers.cpp
// =================================

// Reads all of the users logged in to the network.
//
std::vector<UserData> *get_users();

#endif
