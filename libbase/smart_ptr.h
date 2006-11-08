// smart_ptr.h	-- by Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Smart (ref-counting) pointer classes.  Uses "intrusive" approach:
// the types pointed to must have add_ref() and drop_ref() methods.
// Typically this is done by inheriting from a ref_counted class,
// although the nice thing about templates is that no particular
// ref-counted class is mandated.

/* $Id: smart_ptr.h,v 1.13 2006/11/08 09:37:56 strk Exp $ */

#ifndef SMART_PTR_H
#define SMART_PTR_H

#include "tu_config.h"
#include "utility.h"


/// A smart (strong) pointer asserts that the pointed-to object will
/// not go away as long as the strong pointer is valid.  "Owners" of an
/// object should keep strong pointers; other objects should use a
/// strong pointer temporarily while they are actively using the
/// object, to prevent the object from being deleted.
template<class T>
class DSOEXPORT smart_ptr
{
public:

	smart_ptr(T* ptr)
		:
		m_ptr(ptr)
	{
		if (m_ptr)
		{
			m_ptr->add_ref();
		}
		testInvariant();
	}

	smart_ptr()
		:
		m_ptr(NULL)
	{
		testInvariant();
	}

	smart_ptr(const smart_ptr<T>& s)
		:
		m_ptr(s.m_ptr)
	{
		if (m_ptr)
		{
			m_ptr->add_ref();
		}
		testInvariant();
	}

	~smart_ptr()
	{
		testInvariant();
		if (m_ptr)
		{
			m_ptr->drop_ref();
		}
	}

	//operator bool() const { return m_ptr != NULL; }
	void	operator=(const smart_ptr<T>& s)
	{
		set_ref(s.m_ptr);
		testInvariant();
	}

	void	operator=(T* ptr)
	{
		set_ref(ptr);
		testInvariant();
	}

	T*	operator->() const
	{
		assert(m_ptr);
		testInvariant();
		return m_ptr;
	}

	const T& operator*() const
	{
		assert(m_ptr);
		testInvariant();
		return *m_ptr;
	}

	T& operator*()
	{
		assert(m_ptr);
		testInvariant();
		return *m_ptr;
	}

	T*	get_ptr() const
	{
		testInvariant();
		return m_ptr;
	}

	bool	operator==(const smart_ptr<T>& p) const
	{
		testInvariant();
		return m_ptr == p.m_ptr;
	}

	bool	operator!=(const smart_ptr<T>& p) const
	{
		testInvariant();
		return m_ptr != p.m_ptr;
	}

	bool	operator==(T* p) const
	{
		testInvariant();
		return m_ptr == p;
	}

	bool	operator!=(T* p) const
	{
		testInvariant();
		return m_ptr != p;
	}

	// Provide work-alikes for static_cast, dynamic_cast, implicit up-cast?  ("gentle_cast" a la ajb?)

	/// Check invariant of the smart pointer.
	//	
	/// This function is called as first thing by every public 
	/// inspector function and as last thing by every mutator
	/// function. If you build with NDEBUG defined all such
	/// calls will be removed (in case you're worried about
	/// overhead).
	///
	/// Leaving the calls in will increase the *probability*
	/// to detect memory corruption errors earier.
	///
	/// To further improve this we might have the smart_ptr
	/// testInvariant function call the pointed-to testInvariant
	/// function. I dind't push it so far though (yet) :)
	///
	void testInvariant() const
	{
		// If we have a pointer, check that it's refcount
		// is greater then 0, as if it is not that means
		// that someone deleted it
		assert( m_ptr != NULL || m_ptr->get_ref_count() > 0 );
	}

private:

	void	set_ref(T* ptr)
	{
		if (ptr != m_ptr)
		{
			if (m_ptr)
			{
				m_ptr->drop_ref();
			}
			m_ptr = ptr;

			if (m_ptr)
			{
				m_ptr->add_ref();
			}
		}

		testInvariant();
	}

	T*	m_ptr;
};


#endif // SMART_PTR_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
