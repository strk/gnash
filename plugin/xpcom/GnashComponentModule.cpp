// 
//   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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
//

#include "nsIGenericFactory.h"
#include "GnashComponent.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(GnashComponent)
NS_COM_GLUE nsresult
NS_NewGenericModule2(nsModuleInfo const *info, nsIModule* *result);

static nsModuleComponentInfo components[] =
{
    {
       "GnashComponent", 
       GNASH_COMPONENT_CID,
       GNASH_COMPONENT_CONTRACTID,
       GnashComponentConstructor,
    }
};

NS_IMPL_NSGETMODULE("GnashComponentsModule", components) 

