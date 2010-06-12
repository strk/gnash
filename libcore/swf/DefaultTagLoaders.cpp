// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
#include "DefineFontAlignZonesTag.h"
#include "DefineShapeTag.h"
#include "DefineButtonCxformTag.h"
#include "CSMTextSettingsTag.h"
#include "DefineFontTag.h"
#include "DefineButtonTag.h"
#include "DefineScalingGridTag.h"
#include "DefineTextTag.h"
#include "PlaceObject2Tag.h"
#include "RemoveObjectTag.h"
#include "DoActionTag.h"
#include "DoInitActionTag.h"
#include "DefineEditTextTag.h"
#include "SetBackgroundColorTag.h"
#include "SetTabIndexTag.h"
#include "StartSoundTag.h"
#include "StreamSoundBlockTag.h"
#include "DefineButtonSoundTag.h"
#include "DefineMorphShapeTag.h"
#include "DefineVideoStreamTag.h"
#include "DefineFontNameTag.h"
#include "VideoFrameTag.h"
#include "ImportAssetsTag.h"
#include "ExportAssetsTag.h"
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

typedef TagLoadersTable::Loaders::value_type TagPair;

class AddLoader
{
public:

    AddLoader(TagLoadersTable& table)
        :
        _table(table)
    {}

    void operator()(const TagPair& p)
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

    const std::vector<TagPair> tags = boost::assign::list_of
        (TagPair(SWF::END, end_loader))
        (TagPair(SWF::DEFINESHAPE, DefineShapeTag::loader))
        (TagPair(SWF::FREECHARACTER, fixme_loader)) // 03
        (TagPair(SWF::PLACEOBJECT, PlaceObject2Tag::loader))
        (TagPair(SWF::REMOVEOBJECT, RemoveObjectTag::loader)) // 05
        (TagPair(SWF::DEFINEBITS, define_bits_jpeg_loader))
        (TagPair(SWF::DEFINEBUTTON, DefineButtonTag::loader))
        (TagPair(SWF::JPEGTABLES, jpeg_tables_loader))
        (TagPair(SWF::SETBACKGROUNDCOLOR, SetBackgroundColorTag::loader))
        (TagPair(SWF::DEFINEFONT, DefineFontTag::loader))
        (TagPair(SWF::DEFINETEXT, DefineTextTag::loader))
        (TagPair(SWF::DOACTION,  DoActionTag::loader))
        (TagPair(SWF::DEFINEFONTINFO, DefineFontInfoTag::loader))
        // 62
        (TagPair(SWF::DEFINEFONTINFO2, DefineFontInfoTag::loader))
        (TagPair(SWF::DEFINESOUND, define_sound_loader))
        (TagPair(SWF::STARTSOUND, StartSoundTag::loader))
        // 89
        (TagPair(SWF::STARTSOUND2, StartSound2Tag::loader))

        (TagPair(SWF::STOPSOUND, fixme_loader)) // 16 

        // 17
        (TagPair(SWF::DEFINEBUTTONSOUND, DefineButtonSoundTag::loader))
        // 18
        (TagPair(SWF::SOUNDSTREAMHEAD, sound_stream_head_loader))
        // 19
        (TagPair(SWF::SOUNDSTREAMBLOCK, StreamSoundBlockTag::loader))
        (TagPair(SWF::DEFINELOSSLESS, define_bits_lossless_2_loader))
        (TagPair(SWF::DEFINEBITSJPEG2, define_bits_jpeg2_loader))
        (TagPair(SWF::DEFINESHAPE2,  DefineShapeTag::loader))
        (TagPair(SWF::DEFINEBUTTONCXFORM, DefineButtonCxformTag::loader)) // 23
        // "protect" tag; we're not an authoring tool so we don't care.
        // (might be nice to dump the password instead..)
        (TagPair(SWF::PROTECT, null_loader))
        (TagPair(SWF::PATHSAREPOSTSCRIPT, fixme_loader)) // 25
        (TagPair(SWF::PLACEOBJECT2,  PlaceObject2Tag::loader))
        // 27 - _UNKNOWN_ unimplemented
        (TagPair(SWF::REMOVEOBJECT2, RemoveObjectTag::loader)) // 28
        (TagPair(SWF::SYNCFRAME, fixme_loader)) // 29
        // 30 - _UNKNOWN_ unimplemented
        (TagPair(SWF::FREEALL, fixme_loader)) // 31
        (TagPair(SWF::DEFINESHAPE3,  DefineShapeTag::loader))
        (TagPair(SWF::DEFINETEXT2, DefineText2Tag::loader))
        // 37
        (TagPair(SWF::DEFINEBUTTON2, DefineButton2Tag::loader))
        (TagPair(SWF::DEFINEBITSJPEG3, define_bits_jpeg3_loader))
        (TagPair(SWF::DEFINELOSSLESS2, define_bits_lossless_2_loader))
        (TagPair(SWF::DEFINEEDITTEXT, DefineEditTextTag::loader))
        (TagPair(SWF::DEFINEVIDEO, fixme_loader)) // 38
        (TagPair(SWF::DEFINESPRITE,  sprite_loader))
        (TagPair(SWF::NAMECHARACTER, fixme_loader)) // 40
        (TagPair(SWF::SERIALNUMBER,  serialnumber_loader)) // 41
        (TagPair(SWF::DEFINETEXTFORMAT, fixme_loader)) // 42
        (TagPair(SWF::FRAMELABEL,  frame_label_loader)) // 43

        // TODO: Implement, but fixme_loader breaks tests.
        (TagPair(SWF::DEFINEBEHAVIOR, fixme_loader)) // 44

        (TagPair(SWF::SOUNDSTREAMHEAD2, sound_stream_head_loader)) // 45
        // 46
        (TagPair(SWF::DEFINEMORPHSHAPE, DefineMorphShapeTag::loader))
        (TagPair(SWF::FRAMETAG,  fixme_loader)) // 47
        // 48
        (TagPair(SWF::DEFINEFONT2, DefineFontTag::loader))
        (TagPair(SWF::GENCOMMAND,  fixme_loader)) // 49
        (TagPair(SWF::DEFINECOMMANDOBJ, fixme_loader)) // 50
        (TagPair(SWF::CHARACTERSET,  fixme_loader)) // 51
        (TagPair(SWF::FONTREF, fixme_loader)) // 52

        // TODO: Implement, but fixme_loader breaks tests.
        (TagPair(SWF::DEFINEFUNCTION, fixme_loader)) // 53 
        (TagPair(SWF::PLACEFUNCTION, fixme_loader)) // 54 
        (TagPair(SWF::GENTAGOBJECT, fixme_loader)) // 55 

        (TagPair(SWF::EXPORTASSETS, ExportAssetsTag::loader)) // 56
        (TagPair(SWF::IMPORTASSETS, ImportAssetsTag::loader)) // 57

        //  We're not an authoring tool so we don't care.
        // (might be nice to dump the password instead..)
        (TagPair(SWF::ENABLEDEBUGGER, null_loader))    // 58

        // 59
        (TagPair(SWF::INITACTION, DoInitActionTag::loader)) 
        // 60
        (TagPair(SWF::DEFINEVIDEOSTREAM, DefineVideoStreamTag::loader))
        // 61
        (TagPair(SWF::VIDEOFRAME, VideoFrameTag::loader))

        // 62, DEFINEFONTINFO2 is done above.
        // We're not an authoring tool.
        (TagPair(SWF::DEBUGID, null_loader)) // 63

        //  We're not an authoring tool so we don't care.
        // (might be nice to dump the password instead..)
        (TagPair(SWF::ENABLEDEBUGGER2, null_loader))    // 64
        (TagPair(SWF::SCRIPTLIMITS, ScriptLimitsTag::loader)) //65

        // TODO: Fix this, but probably not critical.
        (TagPair(SWF::SETTABINDEX, SetTabIndexTag::loader)) //66 

        // TODO: Alexis reference says these are 83, 84. The 67, 68 comes from
        // Tamarin. Figure out which one is correct (possibly both are).
        // 67
        (TagPair(SWF::DEFINESHAPE4_, DefineShapeTag::loader))
        // 68
        (TagPair(SWF::DEFINEMORPHSHAPE2_, DefineMorphShapeTag::loader))
        // 69
        (TagPair(SWF::FILEATTRIBUTES, file_attributes_loader))
        // 70
        (TagPair(SWF::PLACEOBJECT3, PlaceObject2Tag::loader))
        // 71
        (TagPair(SWF::IMPORTASSETS2, ImportAssetsTag::loader))
        // 73
        (TagPair(SWF::DEFINEALIGNZONES, DefineFontAlignZonesTag::loader))
        // 74
        (TagPair(SWF::CSMTEXTSETTINGS, CSMTextSettingsTag::loader))
        // 75
        (TagPair(SWF::DEFINEFONT3, DefineFontTag::loader))
        // 77
        (TagPair(SWF::METADATA, metadata_loader))
        // 78
        (TagPair(SWF::DEFINESCALINGGRID, DefineScalingGridTag::loader))
        // 83
        (TagPair(SWF::DEFINESHAPE4, DefineShapeTag::loader))
        // 84
        (TagPair(SWF::DEFINEMORPHSHAPE2, DefineMorphShapeTag::loader))
        // 88
        (TagPair(SWF::DEFINEFONTNAME, DefineFontNameTag::loader))
        // 777
        (TagPair(SWF::REFLEX, reflex_loader))

#ifdef ENABLE_AVM2
        // The following tags are AVM2 only.
        // 72 -- AS3 codeblock.
        (TagPair(SWF::DOABC, DoABCTag::loader)) 
        // 76
        (TagPair(SWF::SYMBOLCLASS, SymbolClassTag::loader))
        // 82
        (TagPair(SWF::DOABCDEFINE, DoABCTag::loader))
        // 86
        (TagPair(SWF::DEFINESCENEANDFRAMELABELDATA,
                DefineSceneAndFrameLabelDataTag::loader))
#endif
        ;

    std::for_each(tags.begin(), tags.end(), AddLoader(table));

}

} // namespace SWF
} // namespace gnash

