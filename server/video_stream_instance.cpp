// video_stream_instance.cpp:  Draw individual video frames, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// 

// $Id: video_stream_instance.cpp,v 1.32 2007/07/09 13:33:30 strk Exp $

#include "sprite_instance.h"
#include "video_stream_instance.h"
#include "video_stream_def.h"
#include "fn_call.h"
#include "as_value.h"
#include "NetStream.h"
#include "render.h"
#include "Range2d.h"
#include "builtin_function.h" // for getter/setter properties
#include "VM.h"

namespace gnash {

static as_object* getVideoInterface();
static void attachVideoInterface(as_object& o);
static void attachVideoProperties(as_object& o);
static as_value video_ctor(const fn_call& fn);
static as_value video_attach(const fn_call& fn);
static as_value video_clear(const fn_call& fn);

static as_object* getVideoInterface()
{
	static boost::intrusive_ptr<as_object> proto;
	if ( proto == NULL )
	{
		proto = new as_object();
		attachVideoInterface(*proto);
		//proto->init_member("constructor", new builtin_function(video_ctor));
	}
	return proto.get();
}

static void attachVideoInterface(as_object& o)
{
	o.init_member("attachVideo", new builtin_function(video_attach));
	o.init_member("clear", new builtin_function(video_clear));
}

static void attachVideoProperties(as_object& o)
{
	//int target_version = o.getVM().getSWFVersion();

	boost::intrusive_ptr<builtin_function> gettersetter;

	gettersetter = new builtin_function(&character::x_getset, NULL);
	o.init_property("_x", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::y_getset, NULL);
	o.init_property("_y", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::xscale_getset, NULL);
	o.init_property("_xscale", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::yscale_getset, NULL);
	o.init_property("_yscale", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::xmouse_get, NULL);
	o.init_readonly_property("_xmouse", *gettersetter);

	gettersetter = new builtin_function(&character::ymouse_get, NULL);
	o.init_readonly_property("_ymouse", *gettersetter);

	gettersetter = new builtin_function(&character::alpha_getset, NULL);
	o.init_property("_alpha", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::visible_getset, NULL);
	o.init_property("_visible", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::width_getset, NULL);
	o.init_property("_width", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::height_getset, NULL);
	o.init_property("_height", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::rotation_getset, NULL);
	o.init_property("_rotation", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::parent_getset, NULL);
	o.init_property("_parent", *gettersetter, *gettersetter);

	gettersetter = new builtin_function(&character::target_getset, NULL);
	o.init_property("_target", *gettersetter, *gettersetter);
}

static as_value
video_attach(const fn_call& fn)
{
	boost::intrusive_ptr<video_stream_instance> video = ensureType<video_stream_instance>(fn.this_ptr);

	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("attachVideo needs 1 arg"));
		);
		return as_value();
	}

	boost::intrusive_ptr<NetStream> ns = boost::dynamic_pointer_cast<NetStream>(fn.arg(0).to_object());
	if (ns)
	{
		video->setStream(ns);
	}
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("attachVideo(%s) first arg is not a NetStream instance"),
			fn.arg(0).to_debug_string().c_str());
		);
	}
	return as_value();
}

static as_value
video_clear(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

static as_value
video_ctor(const fn_call& /* fn */)
{
	log_debug("new Video() TESTING !");

	// I'm not sure We can rely on the def and parent values being accepted  as NULL
	// Not till we add some testing...
	boost::intrusive_ptr<as_object> obj = new video_stream_instance(NULL, NULL, -1);
	return as_value(obj.get()); // will keep alive
}

video_stream_instance::video_stream_instance(video_stream_definition* def,
		character* parent, int id)
	:
	character(parent, id),
	m_def(def),
	//m_video_source(NULL),
	_ns(NULL),
	m_decoder(NULL) // don't abort if m_def is null
{
	log_debug("video_stream_instance %p ctor", (void*)this);
	if ( m_def )
	{
		m_decoder = m_def->get_decoder();
	}

	set_prototype(getVideoInterface());
			
	attachVideoProperties(*this);
}

video_stream_instance::~video_stream_instance()
{
}

void
video_stream_instance::display()
{
	// if m_def is NULL we've been constructed by 'new Video', in this
	// case I think display() would never be invoked on us...
	assert(m_def);

	matrix m = get_world_matrix();
	const rect& bounds = m_def->get_bound();

	// If this is a video from a NetStream object, retrieve a video frame from there.
	if (_ns)
	{
		boost::intrusive_ptr<NetStream> nso = _ns;

		std::auto_ptr<image::image_base> i ( nso->get_video() );
		if (i.get())
		{
			gnash::render::drawVideoFrame(i.get(), &m, &bounds);
		}

	// If this is a video from a VideoFrame tag, retrieve a video frame from there.
	} else if (m_decoder.get()) {
		uint8_t* data = 0;
		int size = 0;
		character* parent = get_parent();
		assert(parent);
		sprite_instance* sprite = parent->to_movie();
		assert(sprite);

		int current_frame = sprite->get_current_frame();
		assert(m_def);
		m_def->get_frame_data(current_frame, &data, &size);

		std::auto_ptr<image::image_base> i ( m_decoder->decodeFrame(data, size) );
		if (i.get())
		{
			gnash::render::drawVideoFrame(i.get(), &m, &bounds);
		} else {
			log_error(_("An error occured while decoding video frame"));
		}

	}

	clear_invalidated();
	//do_display_callback(); <-- are we still relying on this ?
}

void
video_stream_instance::advance(float /*delta_time*/)
{
	if (_ns) {
		_ns->advance();
		if (_ns->newFrameReady()) set_invalidated();
	}
}

void
video_stream_instance::add_invalidated_bounds(InvalidatedRanges& ranges, 
	bool force)
{	
	if (!force && !m_invalidated) return; // no need to redraw
    
	ranges.add(m_old_invalidated_ranges);
	
	// NOTE: do not use m_def->get_bounds()

	// if m_def is NULL we've been constructed by 'new Video', in this
	// case I think add_invalidated_bouns would never be invoked on us...
	assert ( m_def );

	rect bounds;	
	bounds.expand_to_transformed_rect(get_world_matrix(), m_def->get_bound());
	
	ranges.add(bounds.getRange());            
}

void
video_stream_instance::setStream(boost::intrusive_ptr<NetStream> ns)
{
	_ns = ns;
}

// extern (used by Global.cpp)
void video_class_init(as_object& global)
{
	// This is going to be the global Video "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&video_ctor, getVideoInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		//attachVideoInterface(*cl);
	}

	// Register _global.Video
	global.init_member("Video", cl.get());
}

#ifdef GNASH_USE_GC
void
video_stream_instance::markReachableResources() const
{
	if ( _ns ) _ns->setReachable();
}
#endif // GNASH_USE_GC

} // end of namespace gnash
