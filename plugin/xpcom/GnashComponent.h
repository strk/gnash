// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef _MY_COMPONENT_H_
#define _MY_COMPONENT_H_

#include "iGnashComponent.h"
#include "nsStringAPI.h"

#define GNASH_COMPONENT_CONTRACTID "@getgnash.org/updates/GnashComponent;1"
#define GNASH_COMPONENT_CID  {0x9ff0fae7, 0x8ffa, 0x4489, { 0xb3, 0x3d, 0xb7, 0xe8, 0x17, 0xb1, 0x1b, 0xf6 }}

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
