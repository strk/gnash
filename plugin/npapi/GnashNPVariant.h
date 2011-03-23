// 
//   Copyright (C) 2010, 2011 Free Software Foundation, Inc
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

#ifndef GNASH_NPVARIANT_H
#define GNASH_NPVARIANT_H

#if NPAPI_VERSION == 190
#include "npupp.h"
#else
#include "npapi.h"
#include "npruntime.h"
#endif

namespace gnash {

inline const uint32_t&
GetNPStringLen(const NPString& str)
{
#if NPAPI_VERSION == 192
     return str.UTF8Length;
#else
     return str.utf8length;
#endif
}

inline const NPUTF8*
GetNPStringChars(const NPString& str)
{
#if NPAPI_VERSION == 192
    return str.UTF8Characters;
#else
    return str.utf8characters;
#endif
}

/// Makes a deep copy of a NPVariant.
/// @param from The source NPVariant to copy values from.
/// @param to The destination NPVariant.
inline void
CopyVariantValue(const NPVariant& from, NPVariant& to)
{
    // First, we'll make a shallow copy, which is fine for most variant types.
    to = from;

    // For deep copies for strings we obviously have to duplicate the string,
    // and object pointer copies need to have their reference count increased.
    switch(from.type) {
        case NPVariantType_String:
        {
            const NPString& fromstr = NPVARIANT_TO_STRING(from);
            const uint32_t& len = GetNPStringLen(fromstr);

            NPUTF8* tostr = static_cast<NPUTF8*>(NPN_MemAlloc(len));
            std::copy(GetNPStringChars(fromstr),
                      GetNPStringChars(fromstr)+ GetNPStringLen(fromstr), tostr);

            STRINGN_TO_NPVARIANT(tostr, len, to);
            break;
        }
        case NPVariantType_Object:
            NPN_RetainObject(NPVARIANT_TO_OBJECT(to));
            break;
        default:
        {}
    }
}

/// Construct a std::string from an NPString.
//
/// NPStrings are not guaranteed to be NULL-terminated.
inline std::string
NPStringToString(const NPString& str)
{
    return std::string(GetNPStringChars(str), GetNPStringLen(str));
}


/// This class holds ownership of (a copy of) an NPVariant.
//
/// The user of this class must keep in mind that it does not take
/// ownership of already-allocated resources. The user must supply
/// an NPVariant to construct a GnashNPVariant, and must subsequently
/// release any resources associated with the original NPVariant.
///
/// When an object of type GnashNPVariant goes out of scope, the resources
/// associated with the copied NPVariant will be released.
class GnashNPVariant
{
public:
    GnashNPVariant()
    {
        NULL_TO_NPVARIANT(_variant);
    }

    GnashNPVariant(const GnashNPVariant& var)
    {
        CopyVariantValue(var._variant, _variant);
    }

    /// Construct a GnashNPVariant by copying an NPVariant.
    /// @param var The NPVariant to copy from.
    GnashNPVariant(const NPVariant& var)
    {
        CopyVariantValue(var, _variant);
    }

    GnashNPVariant& operator= (const GnashNPVariant& var)
    {
	// Avoid destroying self
	if ( &var == this ) return *this; 

        NPN_ReleaseVariantValue(&_variant);

        CopyVariantValue(var._variant, _variant);

        return *this;
    }

    /// Copy the contained NPVariant into another NPVariant.
    //
    /// This is useful to return a GnashNPVariant to an external API.
    /// @param dest The NPVariant to copy the value into.
    void
    copy(NPVariant& dest) const
    {
        CopyVariantValue(_variant, dest);
    }

    /// Obtain a reference to the contained NPVariant.
    //
    /// This method returns a const reference to avoid the temptation
    /// to modify ownership, which could lead to memory errors. Use copy() if
    /// you want to alter the contained NPVariant.
    const NPVariant& get() const { return _variant; }

    ~GnashNPVariant()
    {
        NPN_ReleaseVariantValue(&_variant);
    }

private:
    NPVariant _variant;
};

} // namespace gnash

#endif // GNASH_NPVARIANT_H

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
