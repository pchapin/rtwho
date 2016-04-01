/****************************************************************************
FILE          : Location.cpp
LAST REVISION : 2006-02-04
SUBJECT       : Functions related to finding a user's location.
PROGRAMMER    : (C) Copyright 2000-2006 by The VTC Computer Club

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
#include <string.h>
#include "RTWho.h"

struct CARD_MAP {
  char MAC_address_list[500][14+1]; // List of MAC address from card map file.
  char location_list[500][80+1];    // List for locations that corresponds to MAC.
};

static struct CARD_MAP card_map;    // Representation of the card map.
static int    entry;                // Counts entries in the card map.

int location_test(char *MAC_address, char *location)
{
  for (int counter = 0; counter <= entry; ++counter) {
    if (strcmp(MAC_address, card_map.MAC_address_list[counter]) == 0) {
      strcpy(location, card_map.location_list[counter]);
      return 0;
    }
  }
  // if not found copy the Mac Address in to the location
  strcpy(location, MAC_address);
  return 0;
}


int location_read()
{

  // The code below appears broken. If it actually tries to read the
  // card map, it dies with a break point exception of some kind in
  // the call to fclose(). I'm not sure what the problem is, exactly,
  // but since this code will need an overhaul anyway, it can probably
  // be ignored for now. --pchapin
#ifdef NEVER
  FILE *data_file;          // File pointer.
  int   end_of_file = 0;    // Flag to see when file has ended.
  int   counter = -1;       // Counter for lists.
  char  temp[17];  	    // String that hold the stuff I don't want at
  			    // the begining of each line in the card map.

  // Open file up that contains the locations and data
  if ((data_file = fopen("s:\\info\\cardmap.txt", "rt"))== NULL) {
    return 1;
  }

  // Read in file until EOF is found
  while(end_of_file != -1) {
    counter++;

    // Read in the begining stuff that means nothing to me.
    if(fgets(temp, 16, data_file) == NULL) end_of_file = -1;

    // We then reading the MAC Address.
    if(fgets(card_map.MAC_address_list[counter], 13, data_file) == NULL) end_of_file = -1;

    // The location is then read in.
    if(fgets(card_map.location_list[counter], 81, data_file) == NULL) end_of_file = -1;

    // Strip out the new line characters from the MacAddress.
    for (size_t x = 0; x < strlen(card_map.MAC_address_list[counter]); x++) {
      if (card_map.MAC_address_list[counter][x] == ' ')
        card_map.MAC_address_list[counter][x] = '0';
      }

    // Strip out the new line characters from the string.
    for (size_t x = 0; x < strlen(card_map.location_list[counter]); x++) {
      if(card_map.location_list[counter][x] == '\n')
        card_map.location_list[counter][x] = '\0';
      }
  }
  fclose(data_file);

  // Update the amount of entries we have found in the cardmap
  entry = counter;
#endif

  return 0;
}

