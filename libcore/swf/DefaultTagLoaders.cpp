// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "DefaultTagLoaders.h"

#include <boost/assign.hpp>
#include <set>

#include "SWF.h"
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
#include "DefineBitsTag.h"
#include "DefineButtonSoundTag.h"
#include "DefineMorphShapeTag.h"
#include "DefineVideoStreamTag.h"
#include "DefineFontNameTag.h"
#include "VideoFrameTag.h"
#include "ImportAssetsTag.h"
#include "ExportAssetsTag.h"
#include "SoundStreamHeadTag.h"
#ifdef ENABLE_AVM2
# include "SymbolClassTag.h"
# include "DoABCTag.h"
# include "DefineSceneAndFrameLabelDataTag.h"
#endif

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

    void operator()(const TagPair& p) {
        _table.registerLoader(p.first, p.second);
    }
private:
    TagLoadersTable& _table;
};

// This is only for tags mentioned in documentation but thought to to exist.
void
unexpected(SWFStream&, TagType tag, movie_definition&, const RunResources&)
{
    static std::set<TagType> warned;
    if (warned.insert(tag).second) {
        log_unimpl(_("Undocumented tag %s encountered. Please report this to "
            "the Gnash developers!"), tag);
    }
}

// Silently ignore the contents of this tag.
void
ignore(SWFStream&, TagType, movie_definition&, const RunResources&)
{
}

} // unnamed namespace

void
addDefaultLoaders(TagLoadersTable& table)
{

    // Note: all the tags given an 'unexpected' here are
    // expected not to be used. They are generally documented
    // by Alexis to exist (without any known structure), but not
    // by Adobe.
    const std::vector<TagPair> tags = boost::assign::list_of
        // 01: nothing to do for an end tag.
        (TagPair(SWF::END, ignore))
        // 02
        (TagPair(SWF::DEFINESHAPE, DefineShapeTag::loader))
        // 03
        (TagPair(SWF::FREECHARACTER, unexpected)) 
        (TagPair(SWF::PLACEOBJECT, PlaceObject2Tag::loader))
        (TagPair(SWF::REMOVEOBJECT, RemoveObjectTag::loader)) // 05
        (TagPair(SWF::DEFINEBITS, DefineBitsTag::loader))
        (TagPair(SWF::DEFINEBUTTON, DefineButtonTag::loader))
        (TagPair(SWF::JPEGTABLES, jpeg_tables_loader))
        (TagPair(SWF::SETBACKGROUNDCOLOR, SetBackgroundColorTag::loader))
        (TagPair(SWF::DEFINEFONT, DefineFontTag::loader))
        (TagPair(SWF::DEFINETEXT, DefineTextTag::loader))
        (TagPair(SWF::DOACTION, DoActionTag::loader))
        (TagPair(SWF::DEFINEFONTINFO, DefineFontInfoTag::loader))
        (TagPair(SWF::DEFINESOUND, define_sound_loader))
        (TagPair(SWF::STARTSOUND, StartSoundTag::loader))
        (TagPair(SWF::STOPSOUND, unexpected)) // 16 
        // 17
        (TagPair(SWF::DEFINEBUTTONSOUND, DefineButtonSoundTag::loader))
        // 18
        (TagPair(SWF::SOUNDSTREAMHEAD, SoundStreamHeadTag::loader))
        // 19
        (TagPair(SWF::SOUNDSTREAMBLOCK, StreamSoundBlockTag::loader))
        (TagPair(SWF::DEFINELOSSLESS, DefineBitsTag::loader))
        (TagPair(SWF::DEFINEBITSJPEG2, DefineBitsTag::loader))
        (TagPair(SWF::DEFINESHAPE2, DefineShapeTag::loader))
        (TagPair(SWF::DEFINEBUTTONCXFORM, DefineButtonCxformTag::loader)) // 23
        (TagPair(SWF::PROTECT, ignore))
        (TagPair(SWF::PATHSAREPOSTSCRIPT, unexpected)) // 25
        (TagPair(SWF::PLACEOBJECT2, PlaceObject2Tag::loader))
        // 28
        (TagPair(SWF::REMOVEOBJECT2, RemoveObjectTag::loader)) 
        (TagPair(SWF::SYNCFRAME, unexpected)) // 29
        (TagPair(SWF::FREEALL, unexpected)) // 31
        (TagPair(SWF::DEFINESHAPE3, DefineShapeTag::loader))
        (TagPair(SWF::DEFINETEXT2, DefineText2Tag::loader))
        // 37
        (TagPair(SWF::DEFINEBUTTON2, DefineButton2Tag::loader))
        (TagPair(SWF::DEFINEBITSJPEG3, DefineBitsTag::loader))
        (TagPair(SWF::DEFINELOSSLESS2, DefineBitsTag::loader))
        (TagPair(SWF::DEFINEEDITTEXT, DefineEditTextTag::loader))
        (TagPair(SWF::DEFINEVIDEO, unexpected)) // 38
        (TagPair(SWF::DEFINESPRITE, sprite_loader))
        (TagPair(SWF::NAMECHARACTER, unexpected)) // 40
        (TagPair(SWF::SERIALNUMBER, serialnumber_loader)) // 41
        (TagPair(SWF::DEFINETEXTFORMAT, unexpected)) // 42
        (TagPair(SWF::FRAMELABEL, frame_label_loader)) // 43
        (TagPair(SWF::DEFINEBEHAVIOR, unexpected)) // 44
        // 45
        (TagPair(SWF::SOUNDSTREAMHEAD2, SoundStreamHeadTag::loader))
        // 46
        (TagPair(SWF::DEFINEMORPHSHAPE, DefineMorphShapeTag::loader))
        // 47
        (TagPair(SWF::FRAMETAG, unexpected))
        // 48
        (TagPair(SWF::DEFINEFONT2, DefineFontTag::loader))
        (TagPair(SWF::GENCOMMAND, unexpected)) // 49
        (TagPair(SWF::DEFINECOMMANDOBJ, unexpected)) // 50
        (TagPair(SWF::CHARACTERSET, unexpected)) // 51
        (TagPair(SWF::FONTREF, unexpected)) // 52
        (TagPair(SWF::DEFINEFUNCTION, unexpected)) // 53 
        (TagPair(SWF::PLACEFUNCTION, unexpected)) // 54 
        (TagPair(SWF::GENTAGOBJECT, unexpected)) // 55 
        (TagPair(SWF::EXPORTASSETS, ExportAssetsTag::loader)) // 56
        (TagPair(SWF::IMPORTASSETS, ImportAssetsTag::loader)) // 57
        (TagPair(SWF::ENABLEDEBUGGER, ignore))    // 58
        // 59
        (TagPair(SWF::INITACTION, DoInitActionTag::loader)) 
        // 60
        (TagPair(SWF::DEFINEVIDEOSTREAM, DefineVideoStreamTag::loader))
        // 61
        (TagPair(SWF::VIDEOFRAME, VideoFrameTag::loader))
        // 62
        (TagPair(SWF::DEFINEFONTINFO2, DefineFontInfoTag::loader))
        // 63
        (TagPair(SWF::DEBUGID, ignore))
        // 64
        (TagPair(SWF::ENABLEDEBUGGER2, ignore))
        (TagPair(SWF::SCRIPTLIMITS, ScriptLimitsTag::loader)) //65
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
        // 89
        (TagPair(SWF::STARTSOUND2, StartSound2Tag::loader))
        // 90
        (TagPair(SWF::DEFINEBITSJPEG4, DefineBitsTag::loader))
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

