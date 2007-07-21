// 
// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of GNU Cygnal.
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

/// \file cygnal_test.hpp
/// \brief All the generic inclusions for unit test in one place, plus error suppression for MSVC.

#pragma once
#ifndef __cygnal_test_hpp__
#define __cygnal_test_hpp__

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4180 4224 )
#endif

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/function.hpp>

#ifdef _MSC_VER
#pragma warning( pop )
#endif


#endif	// end of inclusion protection
