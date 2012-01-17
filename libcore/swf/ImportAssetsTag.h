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

#ifndef GNASH_SWF_IMPORTASSETSTAG_H
#define GNASH_SWF_IMPORTASSETSTAG_H

#include <vector>
#include <utility>
#include <string>
#include <memory>

#include "ControlTag.h"
#include "Movie.h"
#include "MovieClip.h"
#include "SWFStream.h"
#include "MovieFactory.h"
#include "log.h"
#include "StreamProvider.h"

namespace gnash {
namespace SWF {

class ImportAssetsTag : public ControlTag
{
public:

    typedef std::pair<int, std::string> Import;
    typedef std::vector<Import> Imports;
    
    static void loader(SWFStream& in, TagType tag, movie_definition& m,
            const RunResources& r)
    {
        assert(tag == SWF::IMPORTASSETS || tag == SWF::IMPORTASSETS2);

        boost::intrusive_ptr<ControlTag> p(new ImportAssetsTag(tag, in, m, r));
        m.addControlTag(p);
    }


    /// Execute an ImportAssetsTag.
    //
    /// Executing this tag adds the imported definition with an id to the 
    /// list of known characters. This id may be different from the
    /// genuine id of the DefinitionTag.
    virtual void executeState(MovieClip* m, DisplayList& /*l*/) const {
        Movie* mov = m->get_root();
        for (Imports::const_iterator it = _imports.begin(), e = _imports.end();
                it != e; ++it) {
            mov->addCharacter(it->first);
        }
    }

private:

    ImportAssetsTag(TagType t, SWFStream& in, movie_definition& m,
            const RunResources& r)
    {
        read(t, in, m, r);
    }

    void read(TagType t, SWFStream& in, movie_definition& m,
            const RunResources& r) {

        std::string source_url;
        in.read_string(source_url);

        // Resolve relative urls against baseurl
        URL abs_url(source_url, r.streamProvider().baseURL());

        unsigned char import_version = 0;

        if (t == SWF::IMPORTASSETS2) {
            in.ensureBytes(2);
            import_version = in.read_uint(8);
            boost::uint8_t reserved = in.read_uint(8);
            UNUSED(reserved);
        }

        in.ensureBytes(2);
        const boost::uint16_t count = in.read_u16();

        IF_VERBOSE_PARSE(
            log_parse(_("  import: version = %u, source_url = %s (%s), "
                "count = %d"), import_version, abs_url.str(), source_url,
                count);
        );

        // Try to load the source movie into the movie library.
        boost::intrusive_ptr<movie_definition> source_movie;

        try {
            source_movie = MovieFactory::makeMovie(abs_url, r);
        }
        catch (gnash::GnashException& e) {
            log_error(_("Exception: %s"), e.what());
        }

        if (!source_movie) {
            // Give up on imports.
            log_error(_("can't import movie from url %s"), abs_url.str());
            return;
        }

        // Quick consistency check, we might as well do
        // something smarter, if we agree on semantic
        if (source_movie == &m) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("Movie attempts to import symbols from "
                        "itself."));
            );
            return;
        }
        
        // Get the imports.
        for (size_t i = 0; i < count; ++i)
        {
            in.ensureBytes(2);
            const boost::uint16_t id = in.read_u16();

            // We don't consider 0 valid.
            if (!id) continue;

            std::string symbolName;
            in.read_string(symbolName);
            IF_VERBOSE_PARSE (
                log_parse(_("  import: id = %d, name = %s"), id, symbolName);
            );
            _imports.push_back(std::make_pair(id, symbolName));
        }
        
        m.importResources(source_movie, _imports);
    }

private:

    Imports _imports;

};

} // namespace SWF
} // namespace gnash

#endif
