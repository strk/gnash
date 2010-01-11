// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef GNASH_COMPONENT_H
#define GNASH_COMPONENT_H

#include "iGnashComponent.h"
#include "nsStringAPI.h"

#define GNASH_COMPONENT_CONTRACTID "@getgnash.org/updates/GnashComponent;1"
// 2b70f2b1-fc72-4734-bb81-4eb2a7713e49
#define GNASH_COMPONENT_CID  {0x2b70f2b1, 0xfc72, 0xbb81, { 0x4e, 0xb2, 0xa7, 0x71, 0x3e, 0x49 }}

class GnashComponent : public iGnashComponent
{
public:
    NS_DECL_ISUPPORTS
    NS_DECL_IGNASHCOMPONENT
    
    GnashComponent();
    // additional members
    virtual ~GnashComponent();
};

#endif
