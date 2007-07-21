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

/**	\file Handle.hpp
 *	\brief A handle is a type-safe, encapsulated integer that's opaque to its ordinary users.
 *		A handled class is one whose constructor adds it to a singleton registry,
 *			allowing handles rather than pointers where appropriate.
 */

#pragma once
#ifndef __Handle_hpp___
#define __Handle_hpp___

#include "Aspect.hpp"
#include <vector>

namespace ACT {
	//-------------------------
	/**	\class Handle
	 *	\brief A marked, encapsulate ordinal, used as an index in offset operations.
	 *
	 *	- template parameter \c T: parametric so it can match both <tt>std::vector< X >::size_type</tt> and others
	 *	- template parameter \c Marker: creates type-distinction to prevent intermixing of indices
	 *
	 *	\pre
	 *	- class T is an integral type
	 */
	template< class T, class Marker >
	class Handle
	{
		// friend classes
		template< class nT, class nMarker > friend class Vector_with_Handle_Index ;

		/// A handle is an encapsulated integral type
		T the_index ;

		/// Unwrapper is friends-only
		inline T get() { return the_index ; }
	public:
		/// Expose Marker so that multiple vectors can share a common handle type
		typedef Marker marker_type ;

		/// Explicit constructor is required to avoid accidental indexing into non-compatible objects.
		explicit Handle( T x )
			: the_index( x ) {}

		inline bool operator<( Handle x ) { return the_index < x.the_index ; }
		inline bool operator<=( Handle x ) { return the_index <= x.the_index ; }
		inline bool operator==( Handle x ) { return the_index == x.the_index ; }
		inline bool operator>=( Handle x ) { return the_index >= x.the_index ; }
		inline bool operator>( Handle x ) { return the_index > x.the_index ; }
		inline bool operator!=( Handle x ) { return the_index != x.the_index ; }

		/// Addition is not Handle-to-Handle, but Handle-to-integer, because Handle is ordinal.
		inline Handle operator+( unsigned int n ) { return Handle( the_index + n ) ; }
	} ;

	//-------------------------
	/**	\class Vector_with_Handle_Index
	 *	\brief A wrapper around std::vector, with its integral index replaced by a handle.
	 */
	template< class T, class Marker >
	class Vector_with_Handle_Index
		: public std::vector< T >
	{
	private:
		typedef std::vector< T > Base ;		/// Our base class, defined for legibility.

	public:
		typedef typename Base::reference reference ;
		typedef typename Base::const_reference const_reference ;
		typedef typename Base::size_type size_type ;

	private:
		inline reference operator[]( size_type n ) { throw std::exception() ; }				/// Prohibit offset operation with unwrapped index
		inline const_reference operator[]( size_type n ) const { throw std::exception() ; }	/// Prohibit offset operation with unwrapped index
		inline reference at( size_type n ) { throw std::exception() ; }						/// Prohibit offset operation with unwrapped index
		inline const_reference at( size_type n ) const { throw std::exception() ; }			/// Prohibit offset operation with unwrapped index
		inline void resize( size_type, T ) { throw std::exception() ; }			/// Prohibit resize operation with unwrapped index
		inline void reserve( size_type ) { throw std::exception() ; }			/// Prohibit reserve operation with unwrapped index

	public:
		/// Declaration of index type
		typedef Handle< size_type, Marker > handle_type ;

		inline reference operator[]( handle_type n ) { return Base::operator[]( n.get() ) ; }				/// Replacement unchecked offset operator
		inline const_reference operator[]( handle_type n ) const { return Base::operator[]( n.get() ) ; }	/// Replacement unchecked offset operator
		inline reference at( handle_type n ) { return Base::at( n.get() ) ; }								/// Replacement checked offset operator
		inline const_reference at( handle_type n ) const { return Base::at( n.get() ) ; }					/// Replacement checked offset operator

		inline handle_type size() const { return handle_type( Base::size() ) ; }							/// Replacement size returning handle
		inline void resize( handle_type n, T val = T() ) { return Base::resize( n.get(), val ) ; }			/// Replacement resize with handle
		inline void reserve( handle_type n ) { return Base::reserve( n.get() ) ; }							/// Replacement reserve with handle
		inline handle_type capacity() const { return handle_type( Base::capacity() ) ; }					/// Replacement capacity returning handle
	} ;

	//-------------------------
	template< class T, template< class, class > class > class Handled ;
	//-------------------------
	/**	\class Handle_Registry_Leader
	 *	\brief A registry that maps handles to instances of a handled class.
	 *
	 *	This registry is the normative one.
	 *	Each instance of a class with a handle (that is, deriving from \c Handled) appears within this registry.
	 *	In other words, this registry provides a mechanism for operationalizing the referent of a handle.
	 */
	template< class T, template< class, class > class Aspect = aspect::Null_Aspect_1 >
	class Handle_Registry_Leader
	{
		/// The aspect type as actually used.
		typedef Aspect< T, Handle_Registry_Leader > aspect_type ;

		/// The aspect class is a friend
		friend class aspect_type ;

		/// The cooperative Handled aspect is also a friend.
		friend typename Handled< T, Aspect >::aspect_type ;

		/// Type declaration of the class registry
		typedef Vector_with_Handle_Index< T *, Handle_Registry_Leader > vector_type ;

	public:
		/// Proxy the declaration of handle type.
		typedef typename vector_type::handle_type handle_type ;

	private:
		/// Vector holding pointers to constructed items.
		vector_type the_registry ;

		/// Stack of free indices
		std::vector< handle_type > free_handles ;

		/// Aspect instance
		aspect_type aspect ;

	public:
		/// Default constructor
		Handle_Registry_Leader( aspect_type aspect = aspect_type() )
			: aspect( aspect )
		{}

		/// Add an object to the registry, returning its handle.
		handle_type add( T * ) ;

		/// Remove a handle from the registry.
		void remove( handle_type ) ;

		/// Offset access is simple because this class issues all its own indices.
		inline T * operator[]( handle_type x ) { return the_registry[ x ] ; }
	} ;
}

namespace aspect {
	//-------------------------
	/**	Null_Aspect_1
	 *	\brief Partial specialization of generic aspect holds the hooks for derived aspects
	 */
	template< class T >
	class Null_Aspect_1< T, ACT::Handle_Registry_Leader< T, Null_Aspect_1 > >
	{
	public:
		/// Called in \c Handle_Registry_Leader::add upon successful addition that expands the internal size of the registry
		void add_in_new_place() {}

		/// Called in \c Handle_Registry_Leader::add upon successful addition that reuses an existing entry within the registry
		void add_in_old_place() {}
	} ;
}

namespace ACT {
	//-------------------------
	/**	\class Handle_Registry_Follower
	 *	\brief A registry indexed by handles issued by a Handle_Registry_Leader
	 */
	template< class T, class Leader, template< class, class, class > class Aspect = aspect::Null_Aspect_2 >
	class Handle_Registry_Follower
	{
		/// The aspect type as actually used.
		typedef Aspect< T, Leader, Handle_Registry_Follower > aspect_type ;

		/// The aspect class is a friend
		friend class aspect_type ;

		/// Marker class derived from Leader
		typedef typename Leader::handle_type::marker_type marker_type ;

		/// Type declaration of the class registry
		typedef Vector_with_Handle_Index< T, marker_type > vector_type ;

		/// Vector holding pointers to constructed items.
		vector_type the_registry ;

	public:
		/// Proxy the declaration of handle type.
		typedef typename vector_type::handle_type handle_type ;

		/// Aspect instance
		aspect_type aspect ;

	public:
		/// Default constructor
		Handle_Registry_Follower( aspect_type aspect = aspect_type() )
			: aspect( aspect )
		{}

		/// Reference to content item of this registry
		typedef typename vector_type::reference reference ;

		/// Offset access for follower requires a possible expansion of the internal registry for an as-yet-unseen index.
		reference operator[]( handle_type x ) ;
	} ;
}
namespace aspect {
	//-------------------------
	/**	Null_Aspect_2
	 *	\brief Partial specialization of generic aspect holds the hooks for derived aspects
	 */
	template< class T, class Leader >
	class Null_Aspect_2< T, Leader, ACT::Handle_Registry_Follower< T, Leader, Null_Aspect_2 > >
	{
	public:
		inline void access_from_existing_slot() {} ;
		inline void expand_capacity_of_vector() {} ;
		inline void enlarge_size_of_vector() {} ;
	} ;
}
namespace ACT {
	//-------------------------
	/**	\class Handled
	 *	\brief A base class for handled classes.
	 *		To use, use the derived class as the template parameter, as follows:
	 *		class X : public class Handled< X > { ... } ;
	 *
	 *	This class has the responsibility of defining a class registry for each class T.
	 */
	template< class T, template< class, class > class Aspect = aspect::Null_Aspect_1 >
	class Handled
	{
	public:
		/// The aspect type as actually used.
		typedef Aspect< T, Handled > aspect_type ;

		/// The aspect class is a friend
		friend class aspect_type ;

	private:
		/// Type declaration of the class registry
		typedef Handle_Registry_Leader< T, Aspect > registry_type ;

		/// Per-class registry.
		///
		///	[EH 2007-06-25]
		///	In MSVC8, this declaration must appear after that for \c aspect_type,
		///		otherwise the compiler says it's not defined in this class.
		/// This is a compiler defect.
		/// The point of instantiation is supposed to be immediately before the declaration containing the first instance,
		///		but at a namespace or global level, not within a class declaration.
		/// That means that \c registry_type is supposed to be instantiated before this class is.
		/// Since visibility has an ordering dependence, I conclude the class is not instantiated then, 
		///		immediately before this class, but rather within the class.
		/// In other words, MSVC8 has their point of instantiation wrong.
		static registry_type our_registry ;

	public:
		/// Proxy the declaration of handle type from registry type.
		typedef typename registry_type::handle_type handle_type ;

	private:
		/// Enclosed handle 
		handle_type the_handle ;

	protected:
		/// Aspect helper instance.
		///
		/// It's protected so that derived classes may access the aspect.
		/// This class is designed to be a base class, so this is a generic need.
		/// If a read-only version of the aspect is desired, 
		///		change the access control of this item to private
		///		and add a \c const accessor function returning a reference.
		aspect_type aspect ;

	public:
		/// Default constructor takes as parameter the \c this pointer of a derived object.
		Handled( T * that, aspect_type aspect = aspect_type() )
			: the_handle( our_registry.add( that ) ),
			aspect( aspect )
		{
			aspect.set_owner( this ) ;
		}

		/// Destructor
		~Handled() { our_registry.remove( the_handle ) ; }

		/// Class method to access registry
		static T * registry_at( handle_type x ) { return our_registry[ x ] ; }

		/// Handle accessor
		inline handle_type handle() const { return the_handle ; }
	} ;

	//-------------------------
	/**	\class Null_Aspect_Handled
	 *	\brief Base for aspect classes of \c Handled
	 */
	template< class T >
	class Null_Aspect_Handled
	{} ;

} // end of namespace ACT

namespace aspect {
	//-------------------------
	/**	Null_Aspect_1
	 *	\brief Default null aspect class, bound through default aspect parameter
	 */
	template< class T >
	class Null_Aspect_1< T, ACT::Handled< T, Null_Aspect_1 > >
		: public ACT::Null_Aspect_Handled< T >,
		public Null_Aspect_Base< ACT::Handled< T, Null_Aspect_1 > >
	{} ;
}

#endif	// end of inclusion protection
