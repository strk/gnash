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

/**	\file Aspect.hpp
 *	\brief An aspect is a hook for augmenting the behavior of a class at compile time.
 */

#pragma once
#ifndef __Aspect_hpp___
#define __Aspect_hpp___

namespace aspect {

	//--------------------------------------------------
	// Base null aspect classes for specialization

	/// Base Null_Aspect class for class that would otherwise be ordinary classes (no template)
	template< class Owner >
	class Null_Aspect_0 
	{} ;

	/// Base Null_Aspect class for class that would otherwise have a single template parameter
	template< class T, class Owner >
	class Null_Aspect_1
	{} ;

	/// Base Null_Aspect class for class that would otherwise have two template parameters
	template< class T1, class T2, class Owner >
	class Null_Aspect_2
	{} ;

	//--------------------------------------------------
	/**	\class Null_Aspect_Base
	 *
	 *	Specializations of null aspect templates, required to create hook points, do not inherit from their generic template.
	 *	This class provides such a base.
	 *	In order that null templates may derive from it, it does not define any data.
	 *
	 *	This base is needed to provide a null definition of the hooks provided by other add-in classes within this module.
	 *	The first one of these is \c set_owner(), needed for an aspect to gain access its owner's internal data.
	 *	When other add-ins are defined, put a null function definition here for each function in the add-in.
	 */
	template< class Owner >
	class Null_Aspect_Base
	{
	public:
		/// The owner type is the class of which this class is an aspect.
		typedef Owner owner_type ;

		/// For the null aspect set_owner does nothing.
		inline void set_owner( Owner * ) {}
	} ;

	//--------------------------------------------------
	/** \class Aspect_Has_Access_To_Owner
	 *	\brief Base class for aspects that require access to their owner.
	 *
	 *	Not all aspects require access to their owner class.
	 *	Universally, null aspects don't, because the operations they define don't do anything.
	 *	Access to an owner requires storing a pointer, so a null aspect couldn't have one anyway,
	 *		since a null aspect must not define storage (otherwise it's not null).
	 */
	template< class Owner >
	class Aspect_Has_Access_To_Owner
	{
		/// This class is a wrapper around a pointer to its owner.
		Owner * the_owner ;

	public:
		/// Default constructor
		Aspect_Has_Access_To_Owner()
			: the_owner( 0 ) {}

		/// Accessor to owner
		inline Owner * owner() const { return the_owner ; }

		/// Owner must be set after construction, because 'this' isn't known before the aspect instance is constructed.
		inline void set_owner( Owner * x ) { the_owner = x ; }
	} ;

	//--------------------------------------------------
	/** \class Aspect_Has_Const_Access_To_Owner
	 *	\brief Base class for aspects that require read access to their owner, but not write access.
	 *
	 *	Not all aspects require access to their owner class.
	 *	Universally, null aspects don't, because the operations they define don't do anything.
	 *	Access to an owner requires storing a pointer, so a null aspect couldn't have one anyway,
	 *		since a null aspect must not define storage (otherwise it's not null).
	 */
	template< class Owner >
	class Aspect_Has_Const_Access_To_Owner
	{
		/// This class is a wrapper around a pointer to its owner.
		const Owner * the_owner ;

	public:
		/// Default constructor
		Aspect_Has_Const_Access_To_Owner( const Owner * x )
			: the_owner( x ) {}

		/// Accessor to owner
		inline const Owner * owner() const { return the_owner ; }
	} ;

	//--------------------------------------------------
	/**
	 */
} // end namespace aspect


// This should move to config.h, perhaps
#ifdef _MSC_VER
// Disable the warning about "'this' uses in base member initializer list
#	pragma warning(disable:4355)
#else
#endif

#endif	// end of inclusion protection
