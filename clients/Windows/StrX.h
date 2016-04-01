/*
 * Copyright 1999-2000,2004 The Apache Software Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef STRX_H
#define STRX_H

#include <iosfwd>
#include <xercesc/util/XMLString.hpp>

class StrX {
public :

  StrX(const XMLCh* const toTranscode)
    { fLocalForm = Xerces::XMLString::transcode(toTranscode); }

 ~StrX()
   { Xerces::XMLString::release(&fLocalForm); }

  const char* localForm() const
    { return fLocalForm; }

private:
    char*   fLocalForm;
};


inline std::ostream &operator<<(std::ostream &target, const StrX &toDump)
{
  target << toDump.localForm();
  return target;
}

#endif
