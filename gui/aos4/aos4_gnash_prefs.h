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

#ifndef __AOS4_GNASH_PREFS_H__
#define __AOS4_GNASH_PREFS_H__

#define ALL_REACTION_CLASSSES
#include <reaction/reaction_macros.h>
#include <reaction/reaction_prefs.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <images/label.h>
#include <intuition/screens.h>

#include <classes/window.h>

#include <gadgets/layout.h>
#include <gadgets/clicktab.h>
#include <gadgets/scroller.h>
#include <gadgets/checkbox.h>
#include <gadgets/string.h>
#include <gadgets/integer.h>
#include <gadgets/button.h>

#include <proto/exec.h>
#include <proto/intuition.h>

#include <proto/window.h>
#include <proto/layout.h>
#include <proto/space.h>
#include <proto/clicktab.h>
#include <proto/scroller.h>
#include <proto/label.h>
#include <proto/checkbox.h>
#include <proto/string.h>
#include <proto/integer.h>
#include <proto/button.h>

#include <libraries/gadtools.h>

#include <stdio.h>
#include <cstdio>

//#define DEBUG_GUI 1
struct GnashPrefs
{
	int verbosity;
	int logtofile;
	char logfilename[255];
	int logparser;
	int logswf;
	int logmalformedswf;
	int logactionscript;
	int loglocalconn;
	int connectlocalhost;
	int connectlocaldomain;
	int disablessl;
	char sharedobjdir[255];
	int dontwriteso;
	int onlylocalso;
	int disablelocal;
	int nettimeout;
	int usesound;
	int savemedia;
	int savedynamic;
	char savemediadir[255];
	char playerversion[32];
	char detectedos[32];
	char urlopener[255];
	int maxsizemovielib;
	int startpaused;
};


enum
{
    // *** Main ***
    OBJ_CLICKTAB_MAIN = -1,
    //
    // *** page 1 ***
    OBJ_SCROLLER,
    OBJ_SCROLLER_VALUE,
    OBJ_LOGTOFILE,
	OBJ_LOGFILENAME_VALUE,
    OBJ_LOGPARSER,
    OBJ_LOGSWF,
    OBJ_LOGMALFORMEDSWF,
    OBJ_LOGACTIONSCRIPT,
    OBJ_LOGLOCALCONNECTION,
    // *** page 2 ***
    OBJ_CONNECTLOCALHOST,
    OBJ_CONNECTLOCALDOMAIN,
    OBJ_DISABLESSL,
    OBJ_SHAREDOBJDIR_VALUE,
    OBJ_DONTWRITESHAREDOBJ,
    OBJ_ONLYLOCALSHAREDOBJ,
    OBJ_DISABLELOCALCONNOBJ,
    // *** page 3 ***
    OBJ_NETWORKTIMEOUT,
    OBJ_LABELNETWORKTIMEOUT,
    // *** page 4 ***
    OBJ_USESOUNDHANDLER,
    OBJ_SAVEMEDIASTREAMS,
    OBJ_SEVEDYNAMICSTREAMS,
	OBJ_MEDIASAVEDIR_VALUE,
    // *** page 5 ***
    OBJ_PLAYERVERSION_VALUE,
    OBJ_OS_VALUE,
    OBJ_URLOPENER_VALUE,
	OBJ_SIZEMOVIELIB,
	OBJ_STARTINPAUSE,
    // *** other ***
    OBJ_OK,
    OBJ_CANCEL,
    OBJ_NUM
};

Object *make_window(struct GnashPrefs *preferences);

#endif //__AOS4_GNASH_PREFS_H__
