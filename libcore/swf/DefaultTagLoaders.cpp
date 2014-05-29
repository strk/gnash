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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "DefaultTagLoaders.h"

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
    const std::vector<TagPair> tags = {
        // 01: nothing to do for an end tag.
        {SWF::END,ignore},
        // 02
        {SWF::DEFINESHAPE,DefineShapeTag::loader},
        // 03
        {SWF::FREECHARACTER,unexpected},
        {SWF::PLACEOBJECT,PlaceObject2Tag::loader},
        {SWF::REMOVEOBJECT,RemoveObjectTag::loader},// 05
        {SWF::DEFINEBITS,DefineBitsTag::loader},
        {SWF::DEFINEBUTTON,DefineButtonTag::loader},
        {SWF::JPEGTABLES,jpeg_tables_loader},
        {SWF::SETBACKGROUNDCOLOR,SetBackgroundColorTag::loader},
        {SWF::DEFINEFONT,DefineFontTag::loader},
        {SWF::DEFINETEXT,DefineTextTag::loader},
        {SWF::DOACTION,DoActionTag::loader},
        {SWF::DEFINEFONTINFO,DefineFontInfoTag::loader},
        {SWF::DEFINESOUND,define_sound_loader},
        {SWF::STARTSOUND,StartSoundTag::loader},
        {SWF::STOPSOUND,unexpected},// 16
        // 17
        {SWF::DEFINEBUTTONSOUND,DefineButtonSoundTag::loader},
        // 18
        {SWF::SOUNDSTREAMHEAD,SoundStreamHeadTag::loader},
        // 19
        {SWF::SOUNDSTREAMBLOCK,StreamSoundBlockTag::loader},
        {SWF::DEFINELOSSLESS,DefineBitsTag::loader},
        {SWF::DEFINEBITSJPEG2,DefineBitsTag::loader},
        {SWF::DEFINESHAPE2,DefineShapeTag::loader},
        {SWF::DEFINEBUTTONCXFORM,DefineButtonCxformTag::loader},// 23
        {SWF::PROTECT,ignore},
        {SWF::PATHSAREPOSTSCRIPT,unexpected},// 25
        {SWF::PLACEOBJECT2,PlaceObject2Tag::loader},
        // 28
        {SWF::REMOVEOBJECT2,RemoveObjectTag::loader},
        {SWF::SYNCFRAME,unexpected},// 29
        {SWF::FREEALL,unexpected},// 31
        {SWF::DEFINESHAPE3,DefineShapeTag::loader},
        {SWF::DEFINETEXT2,DefineText2Tag::loader},
        // 37
        {SWF::DEFINEBUTTON2,DefineButton2Tag::loader},
        {SWF::DEFINEBITSJPEG3,DefineBitsTag::loader},
        {SWF::DEFINELOSSLESS2,DefineBitsTag::loader},
        {SWF::DEFINEEDITTEXT,DefineEditTextTag::loader},
        {SWF::DEFINEVIDEO,unexpected},// 38
        {SWF::DEFINESPRITE,sprite_loader},
        {SWF::NAMECHARACTER,unexpected},// 40
        {SWF::SERIALNUMBER,serialnumber_loader},// 41
        {SWF::DEFINETEXTFORMAT,unexpected},// 42
        {SWF::FRAMELABEL,frame_label_loader},// 43
        {SWF::DEFINEBEHAVIOR,unexpected},// 44
        // 45
        {SWF::SOUNDSTREAMHEAD2,SoundStreamHeadTag::loader},
        // 46
        {SWF::DEFINEMORPHSHAPE,DefineMorphShapeTag::loader},
        // 47
        {SWF::FRAMETAG,unexpected},
        // 48
        {SWF::DEFINEFONT2,DefineFontTag::loader},
        {SWF::GENCOMMAND,unexpected},// 49
        {SWF::DEFINECOMMANDOBJ,unexpected},// 50
        {SWF::CHARACTERSET,unexpected},// 51
        {SWF::FONTREF,unexpected},// 52
        {SWF::DEFINEFUNCTION,unexpected},// 53
        {SWF::PLACEFUNCTION,unexpected},// 54
        {SWF::GENTAGOBJECT,unexpected},// 55
        {SWF::EXPORTASSETS,ExportAssetsTag::loader},// 56
        {SWF::IMPORTASSETS,ImportAssetsTag::loader},// 57
        {SWF::ENABLEDEBUGGER,ignore},   // 58
        // 59
        {SWF::INITACTION,DoInitActionTag::loader},
        // 60
        {SWF::DEFINEVIDEOSTREAM,DefineVideoStreamTag::loader},
        // 61
        {SWF::VIDEOFRAME,VideoFrameTag::loader},
        // 62
        {SWF::DEFINEFONTINFO2,DefineFontInfoTag::loader},
        // 63
        {SWF::DEBUGID,ignore},
        // 64
        {SWF::ENABLEDEBUGGER2,ignore},
        {SWF::SCRIPTLIMITS,ScriptLimitsTag::loader},//65
        {SWF::SETTABINDEX,SetTabIndexTag::loader},//66
        // TODO: Alexis reference says these are 83,84. The 67,68 comes from
        // Tamarin. Figure out which one is correct (possibly both are).
        // 67
        {SWF::DEFINESHAPE4_,DefineShapeTag::loader},
        // 68
        {SWF::DEFINEMORPHSHAPE2_,DefineMorphShapeTag::loader},
        // 69
        {SWF::FILEATTRIBUTES,file_attributes_loader},
        // 70
        {SWF::PLACEOBJECT3,PlaceObject2Tag::loader},
        // 71
        {SWF::IMPORTASSETS2,ImportAssetsTag::loader},
        // 73
        {SWF::DEFINEALIGNZONES,DefineFontAlignZonesTag::loader},
        // 74
        {SWF::CSMTEXTSETTINGS,CSMTextSettingsTag::loader},
        // 75
        {SWF::DEFINEFONT3,DefineFontTag::loader},
        // 77
        {SWF::METADATA,metadata_loader},
        // 78
        {SWF::DEFINESCALINGGRID,DefineScalingGridTag::loader},
        // 83
        {SWF::DEFINESHAPE4,DefineShapeTag::loader},
        // 84
        {SWF::DEFINEMORPHSHAPE2,DefineMorphShapeTag::loader},
        // 88
        {SWF::DEFINEFONTNAME,DefineFontNameTag::loader},
        // 89
        {SWF::STARTSOUND2,StartSound2Tag::loader},
        // 90
        {SWF::DEFINEBITSJPEG4,DefineBitsTag::loader},
        // 777
        {SWF::REFLEX,reflex_loader}
#ifdef ENABLE_AVM2
        // The following tags are AVM2 only.
        // 72 -- AS3 codeblock.
        ,{SWF::DOABC,DoABCTag::loader},
        // 76
        {SWF::SYMBOLCLASS,SymbolClassTag::loader},
        // 82
        {SWF::DOABCDEFINE,DoABCTag::loader},
        // 86
        {SWF::DEFINESCENEANDFRAMELABELDATA,
                DefineSceneAndFrameLabelDataTag::loader}
#endif
        };

    std::for_each(tags.begin(), tags.end(), AddLoader(table));

}

} // namespace SWF
} // namespace gnash

