// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

// Implementation of ActionScript MovieClipLoader class.

#ifndef GNASH_MOVIECLIPLOADER_H
#define GNASH_MOVIECLIPLOADER_H

#include "button_character_instance.h" // for mouse_state enum
#include "action.h"
//#include "impl.h"

namespace gnash {

	struct mcl {
		int bytes_loaded;
		int bytes_total;
	};

	  class MovieClipLoader
	  {
	  public:

		MovieClipLoader();

		~MovieClipLoader();

		void load(const tu_string& filespec);
	  
		struct mcl *getProgress(as_object *ao);

		/// MovieClip
		bool loadClip(const tu_string& url, void *);

		void unloadClip(void *);
		void addListener(void *);
		void removeListener(void *);

		void	on_button_event(event_id event);
		// Callbacks
		void onLoadStart(void *);
		void onLoadProgress(void *);
		void onLoadInit(void *);
		void onLoadComplete(void *);
		void onLoadError(void *);
		private:
		bool          _started;
		bool          _completed;
		tu_string     _filespec;
		int           _progress;
		bool          _error;
		struct mcl    _mcl;
		mouse_state   _mouse_state;
	};

	/// MovieClipLoader ActionScript object
	class moviecliploader_as_object : public as_object
	{
	public:
		MovieClipLoader mov_obj;
	};

	/// Progress object to use as return of MovieClipLoader.getProgress()
	struct mcl_as_object : public as_object
	{
	  struct mcl data;
	};

	void moviecliploader_loadclip(const fn_call& fn);
	void moviecliploader_unloadclip(const fn_call& fn);
	void moviecliploader_getprogress(const fn_call& fn);
	void moviecliploader_new(const fn_call& fn);
	void moviecliploader_onload_init(const fn_call& fn);
	void moviecliploader_onload_start(const fn_call& fn);
	void moviecliploader_onload_progress(const fn_call& fn);
	void moviecliploader_onload_complete(const fn_call& fn);
	void moviecliploader_onload_error(const fn_call& fn);
	void moviecliploader_default(const fn_call& fn);

} // end of gnash namespace

// GNASH_MOVIECLIPLOADER_H
#endif
