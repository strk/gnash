// smart_ptr.h	-- by Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Smart (ref-counting) pointer classes.  Uses "intrusive" approach:
// the types pointed to must have add_ref() and drop_ref() methods.
// Typically this is done by inheriting from a ref_counted class,
// although the nice thing about templates is that no particular
// ref-counted class is mandated.


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
	}

	smart_ptr() : m_ptr(NULL) {}
	smart_ptr(const smart_ptr<T>& s)
		:
		m_ptr(s.m_ptr)
	{
		if (m_ptr)
		{
			m_ptr->add_ref();
		}
	}

	~smart_ptr()
	{
		if (m_ptr)
		{
			m_ptr->drop_ref();
		}
	}

//	operator bool() const { return m_ptr != NULL; }
	void	operator=(const smart_ptr<T>& s) { set_ref(s.m_ptr); }
	void	operator=(T* ptr) { set_ref(ptr); }
//	void	operator=(const weak_ptr<T>& w);
	T*	operator->() const { assert(m_ptr); return m_ptr; }
	T*	get_ptr() const { return m_ptr; }
	bool	operator==(const smart_ptr<T>& p) const { return m_ptr == p.m_ptr; }
	bool	operator!=(const smart_ptr<T>& p) const { return m_ptr != p.m_ptr; }
	bool	operator==(T* p) const { return m_ptr == p; }
	bool	operator!=(T* p) const { return m_ptr != p; }

	// Provide work-alikes for static_cast, dynamic_cast, implicit up-cast?  ("gentle_cast" a la ajb?)

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
	}

//	friend weak_ptr;

	T*	m_ptr;
};


/// Helper for making objects that can have weak_ptr's.
class DSOLOCAL weak_proxy
{
public:
	weak_proxy()
		:
		m_ref_count(0),
		m_alive(true)
	{
	}

	/// weak_ptr's call this to determine if their pointer is valid or not.
	bool	is_alive() const { return m_alive; }

	/// Only the actual object should call this.
	void	notify_object_died() { m_alive = false; }

	void	add_ref()
	{
		assert(m_ref_count >= 0);
		m_ref_count++;
	}
	void	drop_ref()
	{
		assert(m_ref_count > 0);

		m_ref_count--;
		if (m_ref_count == 0)
		{
			// Now we die.
			delete this;
		}
	}

private:
	// Don't use these.
	weak_proxy(const weak_proxy& /*w*/) { assert(0); }
	void	operator=(const weak_proxy& /*w*/) { assert(0); }

	int	m_ref_count;
	bool	m_alive;
};


/// A weak pointer points at an object, but the object may be deleted
/// at any time, in which case the weak pointer automatically becomes
/// NULL.  The only way to use a weak pointer is by converting it to a
/// strong pointer (i.e. for temporary use).
///
/// The class pointed to must have a "weak_proxy* get_weak_proxy()" method.
///
/// Usage idiom:
///
/// if (smart_ptr<my_type> ptr = m_weak_ptr_to_my_type) { ... use ptr->whatever() safely in here ... }
///
template<class T>
class DSOEXPORT weak_ptr
{
public:
	weak_ptr()
		:
		m_ptr(0)
	{
	}

	weak_ptr(T* ptr)
		:
		m_ptr(0)
	{
		operator=(ptr);
	}

	weak_ptr(const smart_ptr<T>& ptr)
	{
		operator=(ptr.get_ptr());
	}
	
	~weak_ptr()
	{
	}
	
	// Default constructor and assignment from weak_ptr<T> are OK.

	void	operator=(T* ptr)
	{
		m_ptr = ptr;
		if (m_ptr)
		{
			m_proxy = m_ptr->get_weak_proxy();
			assert(m_proxy != NULL);
			assert(m_proxy->is_alive());
		}
		else
		{
			m_proxy = NULL;
		}
	}

	void	operator=(const smart_ptr<T>& ptr) { operator=(ptr.get_ptr()); }

	// Conversion to smart_ptr.
	operator smart_ptr<T>()
	{
		check_proxy();
		return smart_ptr<T>(m_ptr);
	}

	bool	operator==(T* ptr) { check_proxy(); return m_ptr == ptr; }
	bool	operator==(const smart_ptr<T>& ptr) { check_proxy(); return m_ptr == ptr.get_ptr(); }

private:
	void check_proxy()
	// Set m_ptr to NULL if the object died.
	{
		if (m_ptr)
		{
			assert(m_proxy != NULL);
			if (m_proxy->is_alive() == false)
			{
				// Underlying object went away.
				m_proxy = NULL;
				m_ptr = NULL;
			}
		}
	}

	smart_ptr<weak_proxy>	m_proxy;
	T*	m_ptr;
};



/// \brief noref_ptr Pointer without ref counting
/// noref_ptr does not count references; the pointer is deleted when the object
/// goes out of scope.
//
// TODO: noref_ptr<classname> ptr = new classname(); currently throws a compiler
//       error. Figure out why, and fix it!
//
// XXX:  When a function returns (by reference) a noref_ptr, if we choose to
//       assign it like so:
//         noref_ptr<classname> ptr = someclass.get_classname_norefptr();
//       Does that mean the ownership of ptr.m_ptr will transfer to ptr?

template<class T>
    class DSOEXPORT noref_ptr
{
  public:
    noref_ptr(T* ptr)
    : m_ptr(ptr)
    {
    }

    noref_ptr()
    : m_ptr(NULL)
    {
    }

    noref_ptr(noref_ptr<T>& s)
    : m_ptr(s.disown())
    {
    }

    ~noref_ptr()
    {
        delete m_ptr;
    }

    noref_ptr<T>&
    disown()
    {
        T* tmp = m_ptr;
        m_ptr  = NULL;

        return tmp;
    }

    // Transfers "ownwership" of anoter pointer to |this|.
    noref_ptr<T>&
    operator=(noref_ptr<T>& s)
    {
        delete m_ptr;
        m_ptr = s.m_ptr;

        return s.disown();
    }

    noref_ptr<T>&
    operator=(T* ptr)
    {
        delete m_ptr;
        m_ptr = ptr;
        return m_ptr;
    }

    T*
    operator->() const
    {
        assert(m_ptr);
        return m_ptr;
    }
    
    operator T*() const
    {
        return operator->();
    }

    T*
    get_ptr() const
    {
        return operator->();
    }

    bool
    operator==(const noref_ptr<T>& p) const
    {
        return m_ptr == p.m_ptr;
    }

    bool
    operator!=(const smart_ptr<T>& p) const
    {
        return m_ptr != p.m_ptr;
    }

    bool
    operator==(T* p) const 
    {
        return m_ptr == p;
    }

    bool
    operator!=(T* p) const
    {
        return m_ptr != p;
    }

  private:
    T*  m_ptr;
};

// Example ref_counted class:
// 
#if 0
ref_counted
{
	mutable int	m_ref_count
	mutable weak_proxy* m_weak_proxy;

	ref_counted()
		:
		m_ref_count(0),
		m_weak_proxy(0)
	{
	}

	// @@ usual ref-counted stuff here for add_ref() and drop_ref()

	virtual ~ref_counted()
	{
		assert(m_ref_count == 0);
		if (m_weak_proxy)
		{
			m_weak_proxy->notify_object_died();
			m_weak_proxy->drop_ref();
			m_weak_proxy = NULL;
		}
	}

	weak_proxy* get_weak_proxy() const
	{
		if (m_weak_proxy == NULL)
		{
			m_weak_proxy = new weak_proxy;
			m_weak_proxy->add_ref();
		}
	}
};
#endif // 0


#endif // SMART_PTR_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
