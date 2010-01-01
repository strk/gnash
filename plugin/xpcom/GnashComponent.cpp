// 
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
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

#include "GnashComponent.h"

NS_IMPL_ISUPPORTS1(GnashComponent, iGnashComponent)

GnashComponent::GnashComponent()
{
  /* member initializers and constructor code */
}


GnashComponent::~GnashComponent()
{
  /* destructor code */
}

/* long Add (in long a, in long b); */
NS_IMETHODIMP
GnashComponent::Add(PRInt32 a, PRInt32 b, PRInt32 *_retval)
{
    *_retval = a + b;
    return NS_OK;
}
