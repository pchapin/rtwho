/****************************************************************************
FILE          : XMLHandler.cpp
LAST REVISION : 2007-03-25
SUBJECT       : Implementation of the SAX handler class.
PROGRAMMER    : (C) Copyright 2000-2007 by The VTC Computer Club

This class processes SAX events from the XML parser.

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

#include <cstring>
#include <sstream>
#include <string>
using namespace std;

#include <windows.h>

#include "XMLHandler.h"
#include "StrX.h"

// This function exists to deal with character set transcoding issues.
static bool get_attribute_value(
  Xerces::AttributeList &attrs, const char *name, string &value)
{
  XMLCh *attr_name = Xerces::XMLString::transcode(name);
  const XMLCh *attr_value = attrs.getValue(attr_name);
  if (attr_value == NULL) {
    Xerces::XMLString::release(&attr_name);
    return false;
  }
  char *raw_attr_value = Xerces::XMLString::transcode(attr_value);
  value = raw_attr_value;

  Xerces::XMLString::release(&attr_name);
  Xerces::XMLString::release(&raw_attr_value);
  return true;
}

// ----------------------------------------
// XMLHandler: Constructors and Destructor
// ----------------------------------------

XMLHandler::XMLHandler(const string &name, vector<UserData> *info)
  : fqdn(name), user_info(info)
{
}


XMLHandler::~XMLHandler()
{
}


// ----------------------------------------------------------------
// XMLHandler: Implementation of the SAX DocumentHandler interface
// ----------------------------------------------------------------

void XMLHandler::startElement(const XMLCh* const name, Xerces::AttributeList& attrs)
{
  content.erase();

  StrX element_name(name);

  // If this is the start of a user element, look for the name attribute.
  if (strcmp(element_name.localForm(), "user") == 0) {
    struct UserData temp;

    get_attribute_value(attrs, "name", temp.login_name);
    temp.host_name   = fqdn;
    temp.real_name   = "?";
    get_attribute_value(attrs, "real-name", temp.real_name);
    temp.location    = "?";
    temp.object_type = "?";
    user_info->push_back(temp);
  }
}


void XMLHandler::characters(const XMLCh* const chars, const unsigned int length)
{
  char *buffer = new char[length + 1];
  Xerces::XMLString::transcode(chars, buffer, length);
  buffer[length] = '\0';
  content += buffer;
  delete [] buffer;
}


void XMLHandler::endElement(const XMLCh *const name)
{
}


// --------------------------------------------------------
// SAXHandler: Overrides of the SAX ErrorHandler interface
// --------------------------------------------------------

void XMLHandler::error(const Xerces::SAXParseException &e)
{
  stringstream formatter;
  formatter << "RTWho protocol parsing error:\n" << StrX(e.getMessage());
  MessageBox(NULL, formatter.str().c_str(), MBOX_CAPTION, MB_OK|MB_ICONEXCLAMATION);
}


void XMLHandler::fatalError(const Xerces::SAXParseException &e)
{
  stringstream formatter;
  formatter << "RTWho protocol parsing fatal error:\n" << StrX(e.getMessage());
  MessageBox(NULL, formatter.str().c_str(), MBOX_CAPTION, MB_OK|MB_ICONEXCLAMATION);
}


void XMLHandler::warning(const Xerces::SAXParseException &e)
{
  stringstream formatter;
  formatter << "RTWho protocol parsing warning:\n" << StrX(e.getMessage());
  MessageBox(NULL, formatter.str().c_str(), MBOX_CAPTION, MB_OK|MB_ICONEXCLAMATION);
}
