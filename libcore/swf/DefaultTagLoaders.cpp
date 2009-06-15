// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "SWF.h"
#include "DefaultTagLoaders.h"
#include "TagLoadersTable.h"
#include "tag_loaders.h" 
#include "ScriptLimitsTag.h"
#include "BitmapMovieDefinition.h"
#include "DefineFontAlignZonesTag.h"
#include "DefineShapeTag.h"
#include "DefineButtonCxformTag.h"
#include "CSMTextSettingsTag.h"
#include "DefineFontTag.h"
#include "DefineButtonTag.h"
#include "DefineTextTag.h"
#include "PlaceObject2Tag.h"
#include "RemoveObjectTag.h"
#include "DoActionTag.h"
#include "DoInitActionTag.h"
#include "DefineEditTextTag.h"
#include "SetBackgroundColorTag.h"
#include "StartSoundTag.h"
#include "StreamSoundBlockTag.h"
#include "DefineButtonSoundTag.h"
#include "DefineMorphShapeTag.h"
#include "DefineVideoStreamTag.h"
#include "DefineFontNameTag.h"
#include "VideoFrameTag.h"
#ifdef ENABLE_AVM2
# include "SymbolClassTag.h"
# include "DoABCTag.h"
# include "DefineSceneAndFrameLabelDataTag.h"
#endif

#include <boost/assign.hpp>
#include <boost/bind.hpp>

namespace gnash {
namespace SWF {

namespace {
    class AddLoader
    {
    public:
        AddLoader(TagLoadersTable& table)
            :
            _table(table)
        {}
        void operator()(const TagLoadersTable::Loaders::value_type& p)
        {
            _table.registerLoader(p.first, p.second);
        }
    private:
        TagLoadersTable& _table;
    };
}

void
addDefaultLoaders(TagLoadersTable& table)
{

    const std::map<TagType, TagLoadersTable::TagLoader> tags =
        boost::assign::map_list_of

    (SWF::END, end_loader)
    (SWF::DEFINESHAPE, DefineShapeTag::loader)
    (SWF::FREECHARACTER, fixme_loader) // 03
    (SWF::PLACEOBJECT, PlaceObject2Tag::loader)
    (SWF::REMOVEOBJECT, RemoveObjectTag::loader) // 05
    (SWF::DEFINEBITS, define_bits_jpeg_loader)
    (SWF::DEFINEBUTTON, DefineButtonTag::loader)
    (SWF::JPEGTABLES, jpeg_tables_loader)
    (SWF::SETBACKGROUNDCOLOR, SetBackgroundColorTag::loader)
    (SWF::DEFINEFONT, DefineFontTag::loader)
    (SWF::DEFINETEXT, DefineTextTag::loader)
    (SWF::DOACTION,  DoActionTag::loader)
    (SWF::DEFINEFONTINFO, DefineFontInfoTag::loader)
    // 62
    (SWF::DEFINEFONTINFO2, DefineFontInfoTag::loader)
    (SWF::DEFINESOUND, define_sound_loader)
    (SWF::STARTSOUND, StartSoundTag::loader)
    // 89
    (SWF::STARTSOUND2, StartSound2Tag::loader)

    (SWF::STOPSOUND, fixme_loader) // 16 

    // 17
    (SWF::DEFINEBUTTONSOUND, DefineButtonSoundTag::loader)
    // 18
    (SWF::SOUNDSTREAMHEAD, sound_stream_head_loader)
    // 19
    (SWF::SOUNDSTREAMBLOCK, StreamSoundBlockTag::loader)
    (SWF::DEFINELOSSLESS, define_bits_lossless_2_loader)
    (SWF::DEFINEBITSJPEG2, define_bits_jpeg2_loader)
    (SWF::DEFINESHAPE2,  DefineShapeTag::loader)
    (SWF::DEFINEBUTTONCXFORM, DefineButtonCxformTag::loader) // 23
    // "protect" tag; we're not an authoring tool so we don't care.
    // (might be nice to dump the password instead..)
    (SWF::PROTECT, null_loader)
    (SWF::PATHSAREPOSTSCRIPT, fixme_loader) // 25
    (SWF::PLACEOBJECT2,  PlaceObject2Tag::loader)
    // 27 - _UNKNOWN_ unimplemented
    (SWF::REMOVEOBJECT2, RemoveObjectTag::loader) // 28
    (SWF::SYNCFRAME, fixme_loader) // 29
    // 30 - _UNKNOWN_ unimplemented
    (SWF::FREEALL, fixme_loader) // 31
    (SWF::DEFINESHAPE3,  DefineShapeTag::loader)
    (SWF::DEFINETEXT2, DefineText2Tag::loader)
    // 37
    (SWF::DEFINEBUTTON2, DefineButton2Tag::loader)
    (SWF::DEFINEBITSJPEG3, define_bits_jpeg3_loader)
    (SWF::DEFINELOSSLESS2, define_bits_lossless_2_loader)
    (SWF::DEFINEEDITTEXT, DefineEditTextTag::loader)
    (SWF::DEFINEVIDEO, fixme_loader) // 38
    (SWF::DEFINESPRITE,  sprite_loader)
    (SWF::NAMECHARACTER, fixme_loader) // 40
    (SWF::SERIALNUMBER,  serialnumber_loader) // 41
    (SWF::DEFINETEXTFORMAT, fixme_loader) // 42
    (SWF::FRAMELABEL,  frame_label_loader) // 43

    // TODO: Implement, but fixme_loader breaks tests.
    (SWF::DEFINEBEHAVIOR, fixme_loader) // 44

    (SWF::SOUNDSTREAMHEAD2, sound_stream_head_loader) // 45
    // 46
    (SWF::DEFINEMORPHSHAPE, DefineMorphShapeTag::loader)
    (SWF::FRAMETAG,  fixme_loader) // 47
    // 48
    (SWF::DEFINEFONT2, DefineFontTag::loader)
    (SWF::GENCOMMAND,  fixme_loader) // 49
    (SWF::DEFINECOMMANDOBJ, fixme_loader) // 50
    (SWF::CHARACTERSET,  fixme_loader) // 51
    (SWF::FONTREF, fixme_loader) // 52

    // TODO: Implement, but fixme_loader breaks tests.
    (SWF::DEFINEFUNCTION, fixme_loader) // 53 
    (SWF::PLACEFUNCTION, fixme_loader) // 54 
    (SWF::GENTAGOBJECT, fixme_loader) // 55 

    (SWF::EXPORTASSETS, export_loader) // 56
    (SWF::IMPORTASSETS, import_loader) // 57

    //  We're not an authoring tool so we don't care.
    // (might be nice to dump the password instead..)
    (SWF::ENABLEDEBUGGER, null_loader)    // 58

    // 59
    (SWF::INITACTION, DoInitActionTag::loader) 
    // 60
    (SWF::DEFINEVIDEOSTREAM, DefineVideoStreamTag::loader)
    // 61
    (SWF::VIDEOFRAME, VideoFrameTag::loader)

    // 62, DEFINEFONTINFO2 is done above.
    // We're not an authoring tool.
    (SWF::DEBUGID, null_loader) // 63

    //  We're not an authoring tool so we don't care.
    // (might be nice to dump the password instead..)
    (SWF::ENABLEDEBUGGER2, null_loader)    // 64
    (SWF::SCRIPTLIMITS, ScriptLimitsTag::loader) //65

    // TODO: Fix this, but probably not critical.
    (SWF::SETTABINDEX, fixme_loader) //66 

    // TODO: Alexis reference says these are 83, 84. The 67, 68 comes from
    // Tamarin. Figure out which one is correct (possibly both are).
    // 67
    (SWF::DEFINESHAPE4_, DefineShapeTag::loader)
    // 68
    (SWF::DEFINEMORPHSHAPE2_, DefineMorphShapeTag::loader)
    // 69
    (SWF::FILEATTRIBUTES, file_attributes_loader)
    // 70
    (SWF::PLACEOBJECT3, PlaceObject2Tag::loader)
    // 71
    (SWF::IMPORTASSETS2, import_loader)
    // 73
    (SWF::DEFINEALIGNZONES, DefineFontAlignZonesTag::loader)
    // 74
    (SWF::CSMTEXTSETTINGS, CSMTextSettingsTag::loader)
    // 75
    (SWF::DEFINEFONT3, DefineFontTag::loader)
    // 77
    (SWF::METADATA, metadata_loader)
    // 78
    (SWF::DEFINESCALINGGRID, fixme_loader)
    // 83
    (SWF::DEFINESHAPE4, DefineShapeTag::loader)
    // 84
    (SWF::DEFINEMORPHSHAPE2, DefineMorphShapeTag::loader)
    // 88
    (SWF::DEFINEFONTNAME, DefineFontNameTag::loader)
    // 777
    (SWF::REFLEX, reflex_loader)

#ifdef ENABLE_AVM2
    // The following tags are AVM2 only.
    // 72 -- AS3 codeblock.
    (SWF::DOABC, DoABCTag::loader) 
    // 76
    (SWF::SYMBOLCLASS, SymbolClassTag::loader)
    // 82
    (SWF::DOABCDEFINE, DoABCTag::loader)
    // 86
    (SWF::DEFINESCENEANDFRAMELABELDATA,
            DefineSceneAndFrameLabelDataTag::loader);
#endif

    std::for_each(tags.begin(), tags.end(), AddLoader(table));

}

} // namespace SWF
} // namespace gnash

