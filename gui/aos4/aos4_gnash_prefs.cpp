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

#include "aos4_gnash_prefs.h"

#include <string.h>

Object *Objects[OBJ_NUM];
#define OBJ(x) Objects[x]
#define GAD(x) (struct Gadget *)Objects[x]

CONST_STRPTR PageLabels_1[] = {"Logging", "Security", "Network", "Media", "Player", NULL};

#define SPACE LAYOUT_AddChild, SpaceObject, End

Object *win;

struct MsgPort *AppPort;

Object *
make_window(struct GnashPrefs *preferences)
{
	char verb_level[3];

/*
	printf("1)%s - %d\n",preferences->logfilename,strlen(preferences->logfilename));
	printf("2)%s - %d\n",preferences->sharedobjdir,strlen(preferences->sharedobjdir));
	printf("3)%s - %d\n",preferences->savemediadir,strlen(preferences->savemediadir));
	printf("4)%s - %d\n",preferences->playerversion,strlen(preferences->playerversion));
	printf("5)%s - %d\n",preferences->detectedos,strlen(preferences->detectedos));
	printf("6)%s - %d\n",preferences->urlopener,strlen(preferences->urlopener));
*/
    Object
            *page1 = NULL,
            *page2 = NULL,
            *page3 = NULL,
            *page4 = NULL,
            *page5 = NULL;

	snprintf(verb_level,3,"%d",preferences->verbosity);

    page1 = (Object*) VLayoutObject,
                LAYOUT_BevelStyle,		BVS_GROUP,
                LAYOUT_Label,			"Logging options",
				LAYOUT_LabelPlace,		BVJ_TOP_CENTER,
               	LAYOUT_AddChild, VLayoutObject,
	                LAYOUT_BevelStyle,		BVS_NONE,
	                LAYOUT_Label,			"Verbosity level:",
					LAYOUT_LabelPlace,		BVJ_TOP_LEFT,
					LAYOUT_AddChild,	OBJ(OBJ_SCROLLER_VALUE) = (Object*) ButtonObject,
							GA_Text,      			verb_level,
							GA_ID,        			OBJ_SCROLLER_VALUE,
							GA_RelVerify, 			TRUE,
							BUTTON_Transparent, 	TRUE,
							BUTTON_BevelStyle,		BVS_NONE,
							BUTTON_Justification, 	BCJ_LEFT,
					End,
					LAYOUT_AddChild, OBJ(OBJ_SCROLLER) = (Object*) ScrollerObject,
						GA_RelVerify, TRUE,
						SCROLLER_Total, 		11,
						SCROLLER_Top,			preferences->verbosity,
						SCROLLER_Arrows, 		FALSE,
						SCROLLER_Orientation, 	SORIENT_HORIZ,
					End,
					LAYOUT_AddChild, OBJ(OBJ_LOGTOFILE) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_LOGTOFILE,
							GA_RelVerify, 		TRUE,
							GA_Text, 			"Log to _file",
							CHECKBOX_TextPlace, PLACETEXT_RIGHT,
							CHECKBOX_Checked, 	(preferences->logtofile==1) ? TRUE : FALSE,
					End,
					LAYOUT_AddChild, OBJ(OBJ_LOGFILENAME_VALUE) = (Object*) StringObject,
							GA_ID, 				OBJ_LOGFILENAME_VALUE,
							GA_RelVerify, 		TRUE,
							GA_TabCycle, 		TRUE,
							STRINGA_MaxChars, 	254,
							STRINGA_MinVisible,	10,
							//STRINGA_TextVal,	preferences->logfilename,
					End,
					CHILD_Label, LabelObject,	LABEL_Text, "Logfile name:", LabelEnd,
					LAYOUT_AddChild, OBJ(OBJ_LOGPARSER) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_LOGPARSER,
							GA_RelVerify, 		TRUE,
							GA_Text, 			"Log _parser output",
							CHECKBOX_TextPlace,	PLACETEXT_RIGHT,
							CHECKBOX_Checked,	(preferences->logparser==1) ? TRUE : FALSE,
					End,
					LAYOUT_AddChild, OBJ(OBJ_LOGSWF) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_LOGSWF,
							GA_RelVerify, 		TRUE,
							GA_Text, 			"Log SWF _actions",
							CHECKBOX_TextPlace,	PLACETEXT_RIGHT,
							CHECKBOX_Checked,	(preferences->logswf==1) ? TRUE : FALSE,
					End,
					LAYOUT_AddChild, OBJ(OBJ_LOGMALFORMEDSWF) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_LOGMALFORMEDSWF,
							GA_RelVerify, 		TRUE,
							GA_Text, 			"Log malformed SWF _errors",
							CHECKBOX_TextPlace, PLACETEXT_RIGHT,
							CHECKBOX_Checked, 	(preferences->logmalformedswf==1) ? TRUE : FALSE,
					End,
					LAYOUT_AddChild, OBJ(OBJ_LOGACTIONSCRIPT) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_LOGACTIONSCRIPT,
							GA_RelVerify, 		TRUE,
							GA_Text, 			"Log ActionScript _coding errors",
							CHECKBOX_TextPlace, PLACETEXT_RIGHT,
							CHECKBOX_Checked, 	(preferences->logactionscript==1) ? TRUE : FALSE,
					End,
					LAYOUT_AddChild, OBJ(OBJ_LOGLOCALCONNECTION) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_LOGLOCALCONNECTION,
							GA_RelVerify, 		TRUE,
							GA_Text, 			"Log _Local Connection activity",
							CHECKBOX_TextPlace, PLACETEXT_RIGHT,
							CHECKBOX_Checked, 	(preferences->loglocalconn==1) ? TRUE : FALSE,
					End,
			    End,
            End;  // VLayout
    page2 = (Object*) VLayoutObject,
                LAYOUT_BevelStyle,  BVS_NONE,
               	LAYOUT_AddChild, VLayoutObject,
					LAYOUT_BevelStyle,  BVS_GROUP,
					LAYOUT_LabelPlace,	BVJ_TOP_CENTER,
               		LAYOUT_Label,       "Security",
					LAYOUT_AddChild, OBJ(OBJ_CONNECTLOCALHOST) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_CONNECTLOCALHOST,
							GA_RelVerify, 		TRUE,
							GA_Text, 			"Connect only to local _host",
							CHECKBOX_TextPlace, PLACETEXT_RIGHT,
							CHECKBOX_Checked, 	(preferences->connectlocalhost==1) ? TRUE : FALSE,
					End, //CheckBoxObject
	                CHILD_WeightedHeight, 0,
					LAYOUT_AddChild, OBJ(OBJ_CONNECTLOCALDOMAIN) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_CONNECTLOCALDOMAIN,
							GA_RelVerify,		TRUE,
							GA_Text, 			"Connect only to local _domain",
							CHECKBOX_TextPlace, PLACETEXT_RIGHT,
							CHECKBOX_Checked, 	(preferences->connectlocaldomain==1) ? TRUE : FALSE,
					End, //CheckBoxObject
	                CHILD_WeightedHeight, 0,
					LAYOUT_AddChild, OBJ(OBJ_DISABLESSL) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_DISABLESSL,
							GA_RelVerify, 		TRUE,
							GA_Text, 			"Disable SSL _verification",
							CHECKBOX_TextPlace, PLACETEXT_RIGHT,
							CHECKBOX_Checked, 	(preferences->disablessl==1) ? TRUE : FALSE,
					End, //CheckBoxObject
	                CHILD_WeightedHeight, 0,
                    SPACE,
           		End, //VLayoutObject
				LAYOUT_AddChild, VLayoutObject,
					LAYOUT_BevelStyle,  BVS_GROUP,
					LAYOUT_LabelPlace,	BVJ_TOP_CENTER,
					LAYOUT_Label,       "Privacy",
	                CHILD_WeightedHeight, 0,
					LAYOUT_AddChild, OBJ(OBJ_SHAREDOBJDIR_VALUE) = (Object*) StringObject,
							GA_ID, 				OBJ_SHAREDOBJDIR_VALUE,
							GA_RelVerify,		TRUE,
							GA_TabCycle, 		TRUE,
							STRINGA_MaxChars, 	254,
							STRINGA_MinVisible,	10,
							//STRINGA_TextVal,	preferences->sharedobjdir,
					End, //StringObject
					CHILD_Label, LabelObject,	LABEL_Text, "Shared objects directory:", LabelEnd,
	                CHILD_WeightedHeight, 0,
					LAYOUT_AddChild, OBJ(OBJ_DONTWRITESHAREDOBJ) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_DONTWRITESHAREDOBJ,
							GA_RelVerify, 		TRUE,
							GA_Text, 			"Do _not write Shared Object files",
							CHECKBOX_TextPlace, PLACETEXT_RIGHT,
							CHECKBOX_Checked, 	(preferences->dontwriteso==1) ? TRUE : FALSE,
					End, //CheckBoxObject
	                CHILD_WeightedHeight, 0,
					LAYOUT_AddChild, OBJ(OBJ_ONLYLOCALSHAREDOBJ) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_ONLYLOCALSHAREDOBJ,
							GA_RelVerify, 		TRUE,
							GA_Text, 			"Only _access local Shared Object files",
							CHECKBOX_TextPlace, PLACETEXT_RIGHT,
							CHECKBOX_Checked, 	(preferences->onlylocalso==1) ? TRUE : FALSE,
					End, //CheckBoxObject
	                CHILD_WeightedHeight, 0,
					LAYOUT_AddChild, OBJ(OBJ_DISABLELOCALCONNOBJ) = (Object*) CheckBoxObject,
							GA_ID, 				OBJ_DISABLELOCALCONNOBJ,
							GA_RelVerify, 		TRUE,
							GA_Text, 			"Disable Local _Connection object",
							CHECKBOX_TextPlace, PLACETEXT_RIGHT,
							CHECKBOX_Checked, 	(preferences->disablelocal==1) ? TRUE : FALSE,
					End, //CheckBoxObject
	                CHILD_WeightedHeight, 0,
                    SPACE,
				End,  // VLayoutObject
           	End;  // VLayoutObject

    page3 = (Object*) VLayoutObject,
					LAYOUT_BevelStyle,    BVS_GROUP,
					LAYOUT_LabelPlace,	BVJ_TOP_CENTER,
					LAYOUT_Label,         "Network",
					LAYOUT_AddChild,    OBJ(OBJ_NETWORKTIMEOUT) = (Object*) IntegerObject,
							GA_ID,				OBJ_NETWORKTIMEOUT,
							GA_RelVerify,		TRUE,
							GA_TabCycle,		TRUE,
							INTEGER_Arrows,		TRUE,
							INTEGER_MaxChars,	3,
							INTEGER_Minimum,	0,
							INTEGER_Maximum,	120,
							INTEGER_Number,		preferences->nettimeout,
					End,
					CHILD_NominalSize,	TRUE,
					CHILD_Label, LabelObject,	LABEL_Text, "Network timeout in secs (0 for no timeout)", LabelEnd,
	                CHILD_WeightedHeight, 0,
	                SPACE,
             End;  // VLayout

    page4 = (Object*) VLayoutObject,
						LAYOUT_BevelStyle,		BVS_NONE,
						LAYOUT_AddChild, VLayoutObject,
							LAYOUT_BevelStyle,		BVS_GROUP,
							LAYOUT_Label,			"Sound",
							LAYOUT_LabelPlace,		BVJ_TOP_CENTER,
							LAYOUT_AddChild, VLayoutObject,
								LAYOUT_AddChild, OBJ(OBJ_USESOUNDHANDLER) = (Object*) CheckBoxObject,
										GA_ID, 				OBJ_USESOUNDHANDLER,
										GA_RelVerify, 		TRUE,
										GA_Text, 			"Use sound _handler",
										CHECKBOX_TextPlace, PLACETEXT_RIGHT,
										CHECKBOX_Checked, 	(preferences->usesound==1) ? TRUE : FALSE,
								End, //CheckBoxObject
								CHILD_WeightedHeight, 0,
							End,
						End,
						LAYOUT_AddChild, VLayoutObject,
								LAYOUT_BevelStyle,		BVS_GROUP,
								LAYOUT_Label,			"Media Streams",
								LAYOUT_LabelPlace,		BVJ_TOP_CENTER,
								LAYOUT_AddChild, OBJ(OBJ_SAVEMEDIASTREAMS) = (Object*) CheckBoxObject,
										GA_ID, 				OBJ_SAVEMEDIASTREAMS,
										GA_RelVerify, 		TRUE,
										GA_Text, 			"Save media streams to disk",
										CHECKBOX_TextPlace, PLACETEXT_RIGHT,
										CHECKBOX_Checked, 	(preferences->savemedia==1) ? TRUE : FALSE,
								End, //CheckBoxObject
								LAYOUT_AddChild, OBJ(OBJ_SEVEDYNAMICSTREAMS) = (Object*) CheckBoxObject,
										GA_ID, 				OBJ_SEVEDYNAMICSTREAMS,
										GA_RelVerify, 		TRUE,
										GA_Text, 			"Save dynamically loaded media to disk",
										CHECKBOX_TextPlace, PLACETEXT_RIGHT,
										CHECKBOX_Checked, 	(preferences->savedynamic==1) ? TRUE : FALSE,
								End, //CheckBoxObject
								SPACE,
								LAYOUT_AddChild, OBJ(OBJ_MEDIASAVEDIR_VALUE) = (Object*) StringObject,
										GA_ID, 				OBJ_MEDIASAVEDIR_VALUE,
										GA_RelVerify,		TRUE,
										GA_TabCycle, 		TRUE,
										STRINGA_MaxChars, 	254,
										STRINGA_MinVisible, 10,
										//STRINGA_TextVal,	preferences->savemediadir,
								End, //StringObject
								CHILD_NominalSize,	TRUE,
								CHILD_Label, LabelObject,	LABEL_Text, "Save media directory:", LabelEnd,
								CHILD_WeightedHeight, 0,
						End,
	             End;  // VLayout

    page5 = (Object*) VLayoutObject,
						LAYOUT_BevelStyle,		BVS_NONE,
						LAYOUT_AddChild, VLayoutObject,
							LAYOUT_BevelStyle,		BVS_GROUP,
							LAYOUT_Label,			"Player description",
							LAYOUT_LabelPlace,		BVJ_TOP_CENTER,
							LAYOUT_AddChild, VLayoutObject,
								LAYOUT_AddChild, OBJ(OBJ_PLAYERVERSION_VALUE) = (Object*) StringObject,
										GA_ID, 				OBJ_PLAYERVERSION_VALUE,
										GA_RelVerify,		TRUE,
										GA_TabCycle, 		TRUE,
										STRINGA_MaxChars, 	31,
										STRINGA_MinVisible, 10,
										//STRINGA_TextVal,	preferences->playerversion,
								End, //StringObject
								CHILD_NominalSize,	TRUE,
								CHILD_Label, LabelObject,	LABEL_Text, "Player version:", LabelEnd,
								CHILD_WeightedHeight, 0,
								LAYOUT_AddChild, OBJ(OBJ_OS_VALUE) = (Object*) StringObject,
										GA_ID, 				OBJ_OS_VALUE,
										GA_RelVerify, 		TRUE,
										GA_TabCycle, 		TRUE,
										STRINGA_MinVisible,	10,
										STRINGA_MaxChars,	31,
										//STRINGA_TextVal,	preferences->detectedos,
								End, //StringObject
								CHILD_NominalSize,	TRUE,
								CHILD_Label, LabelObject,	LABEL_Text, "Operating system:", LabelEnd,
								CHILD_WeightedHeight, 0,
								LAYOUT_AddChild, OBJ(OBJ_URLOPENER_VALUE) = (Object*) StringObject,
										GA_ID, 				OBJ_URLOPENER_VALUE,
										GA_RelVerify, 		TRUE,
										GA_TabCycle, 		TRUE,
										STRINGA_MinVisible, 10,
										STRINGA_MaxChars, 	254,
										//STRINGA_TextVal,	preferences->urlopener,
								End, //StringObject
								CHILD_NominalSize,	TRUE,
								CHILD_Label, LabelObject,	LABEL_Text, "URL opener:", LabelEnd,
								CHILD_WeightedHeight, 0,
							End,
						End,
						LAYOUT_AddChild, VLayoutObject,
								LAYOUT_BevelStyle,		BVS_GROUP,
								LAYOUT_Label,			"Performance",
								LAYOUT_LabelPlace,		BVJ_TOP_CENTER,
								LAYOUT_AddChild,    OBJ(OBJ_SIZEMOVIELIB) = (Object*) IntegerObject,
										GA_ID,				OBJ_SIZEMOVIELIB,
										GA_RelVerify,		TRUE,
										GA_TabCycle,		TRUE,
										INTEGER_Arrows,		TRUE,
										INTEGER_MaxChars,	4,
										INTEGER_Minimum,	0,
										INTEGER_Maximum,	9999,
										INTEGER_Number,		preferences->maxsizemovielib,
								End, //Integer
								CHILD_NominalSize,	TRUE,
								CHILD_Label, LabelObject,	LABEL_Text, "Max size of movie library", LabelEnd,
								CHILD_WeightedHeight, 0,
								LAYOUT_AddChild, OBJ(OBJ_STARTINPAUSE) = (Object*) CheckBoxObject,
										GA_ID, 				OBJ_STARTINPAUSE,
										GA_RelVerify, 		TRUE,
										GA_Text, 			"Start _Gnash in pause mode",
										CHECKBOX_TextPlace, PLACETEXT_RIGHT,
										CHECKBOX_Checked, 	(preferences->startpaused==1) ? TRUE : FALSE,
								End, //CheckBoxObject
								SPACE,
						End, // VLayout
	             End;  // VLayout

    OBJ(OBJ_CLICKTAB_MAIN) = (Object*) ClickTabObject,
        GA_Text,            PageLabels_1,
        CLICKTAB_Current,   0,
        CLICKTAB_PageGroup, PageObject,
            PAGE_Add,       page1,
            PAGE_Add,       page2,
            PAGE_Add,       page3,
            PAGE_Add,       page4,
            PAGE_Add,       page5,
        PageEnd,
    ClickTabEnd;

    return (Object*) WindowObject,
        WA_ScreenTitle,        "Gnash preferences",
        WA_Title,              "Gnash preferences",
        WA_DragBar,            TRUE,
        WA_CloseGadget,        TRUE,
        WA_SizeGadget,         FALSE,
        WA_DepthGadget,        TRUE,
        WA_Activate,           TRUE,
        WINDOW_IconifyGadget,  TRUE,
        WINDOW_IconTitle,      "Gnash",
        WINDOW_AppPort,        AppPort,
        WINDOW_Position,       WPOS_CENTERSCREEN,
        WINDOW_Layout,         VLayoutObject,
            LAYOUT_AddChild,       OBJ(OBJ_CLICKTAB_MAIN),
			LAYOUT_AddChild, HLayoutObject,
				SPACE,
				SPACE,
				LAYOUT_AddChild,       Button("_OK",OBJ_OK),
				LAYOUT_AddChild,       Button("_Quit",OBJ_CANCEL),
	            CHILD_WeightedHeight,  0,
			End,
        End,   // VLayout
    WindowEnd;
}
