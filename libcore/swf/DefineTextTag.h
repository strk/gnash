// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_SWF_DEFINETEXTTAG_H
#define GNASH_SWF_DEFINETEXTTAG_H

#include <vector>
#include "DefinitionTag.h" // for inheritance
#include "SWFRect.h" // for composition
#include "SWF.h"
#include "SWFMatrix.h"
#include "TextRecord.h"

namespace gnash {
    class movie_definition;
    class SWFStream;
    class RunResources;
    class StaticText;
    class Transform;
}

namespace gnash {
namespace SWF {


/// Static text definition tag
class DefineTextTag : public DefinitionTag
{
public:

    static void loader(SWFStream& in, TagType tag, movie_definition& m, 
            const RunResources& r);

	/// Draw the string.
    void display(Renderer& renderer, const Transform& xform) const;
	
	const SWFRect& bounds() const {
        // TODO: There is a _matrix field in the definition(!) that's
        // currently ignored. Don't know if it needs to be transformed... 
        return _rect; 
    }

    /// Extract static text from TextRecords.
    //
    /// @param to   Will be filled with pointers to TextRecords
    ///             if any are present
    /// @param size Will contain the number of DisplayObjects in this
    ///             StaticText definition.
    bool extractStaticText(std::vector<const TextRecord*>& to, size_t& size)
      const;

    virtual DisplayObject* createDisplayObject(Global_as& gl,
            DisplayObject* parent) const;

private:

    /// DefineText2Tag::loader also constructs a DefineTextTag.
    friend class DefineText2Tag;

    /// Construct a DefineTextTag.
    //
    /// This should only be constructed using the loader() functions.
	DefineTextTag(SWFStream& in, movie_definition& m, TagType tag,
            boost::uint16_t id)
        :
        DefinitionTag(id)
    {
        read(in, m, tag);
    }

	SWFRect _rect;

        SWFMatrix _matrix;

	void read(SWFStream& in, movie_definition& m, TagType tag);
	
    TextRecord::TextRecords _textRecords;
};

/// Parse a DefineText2Tag.
//
/// This creates a DefineTextTag, as there are only minor differences.
class DefineText2Tag
{
public:
    static void loader(SWFStream& in, TagType tag, movie_definition& m, 
            const RunResources& r);
};

} // namespace SWF
} // namespace gnash

#endif 
