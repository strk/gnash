// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
#ifndef __MOVIE_H__
#define __MOVIE_H__

#include "button.h"
#include "log.h"
#include "action.h"
#include "impl.h"

namespace gnash {
  struct mcl {
    int bytes_loaded;
    int bytes_total;
  };

#if 0
struct MovieClipLoader : public character
{
  MovieClipLoader(movie* parent, int id) :
      character(parent, id)
  {
    log_msg("%s: \n", __FUNCTION__);
  }
#else
  class MovieClipLoader
  {
#endif
  public:
    MovieClipLoader();

  ~MovieClipLoader();

  void load(const tu_string& filespec);
  
  struct mcl *getProgress(as_object *ao);

  bool loadClip(const tu_string& str, void *);
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

struct moviecliploader_as_object : public gnash::as_object
{
  MovieClipLoader mov_obj;
};

struct mcl_as_object : public gnash::as_object
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

// __MOVIE_H__
#endif
