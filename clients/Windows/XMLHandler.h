/****************************************************************************
FILE          : XMLHandler.h
LAST REVISION : 2006-02
SUBJECT       : Definition of SAX handler class.
PROGRAMMER    : (C) Copyright 2000-2006 by The VTC Computer Club

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

#ifndef XMLHANDLER_H
#define XMLHANDLER_H

#include <string>

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/HandlerBase.hpp>
namespace Xerces = xercesc_3_2;

#include "RTWho.h"

class XMLHandler : public Xerces::HandlerBase {
public:
  XMLHandler(const std::string &name, std::vector<UserData> *);
 ~XMLHandler();
  
  // ----------------------------------------------
  // Handlers for the SAX ContentHandler interface
  // ----------------------------------------------

  void startElement(const XMLCh *const name, Xerces::AttributeList& attrs);
  void characters  (const XMLCh *const chars, const unsigned int length);
  void endElement  (const XMLCh *const name);

  // --------------------------------------------
  // Handlers for the SAX ErrorHandler interface
  // --------------------------------------------

  void warning   (const Xerces::SAXParseException &e);
  void error     (const Xerces::SAXParseException &e);
  void fatalError(const Xerces::SAXParseException &e);

private:
  const std::string      &fqdn;       // Reference to server's host name.
  std::vector<UserData> *user_info;  // Points at the vector holding user information.
  std::string             content;    // Content of the last text-only element.
};

#endif
