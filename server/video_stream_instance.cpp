// video_stream_instance.cpp:  Draw individual video frames, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
#include "Object.h"

namespace gnash {

static as_object* getVideoInterface(as_object& where);
static void attachVideoInterface(as_object& o);
static void attachVideoProperties(as_object& o);
static as_value video_ctor(const fn_call& fn);
static as_value video_attach(const fn_call& fn);
static as_value video_clear(const fn_call& fn);

static as_object* getVideoInterface(as_object& where)
{
	static boost::intrusive_ptr<as_object> proto;
	if ( proto == NULL )
	{
		proto = new as_object(getObjectInterface());
		where.getVM().addStatic(proto.get());

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

	as_c_function_ptr gettersetter;

	gettersetter = &character::x_getset;
	o.init_property("_x", *gettersetter, *gettersetter);

	gettersetter = &character::y_getset;
	o.init_property("_y", *gettersetter, *gettersetter);

	gettersetter = &character::xscale_getset;
	o.init_property("_xscale", *gettersetter, *gettersetter);

	gettersetter = &character::yscale_getset;
	o.init_property("_yscale", *gettersetter, *gettersetter);

	gettersetter = &character::xmouse_get;
	o.init_readonly_property("_xmouse", *gettersetter);

	gettersetter = &character::ymouse_get;
	o.init_readonly_property("_ymouse", *gettersetter);

	gettersetter = &character::alpha_getset;
	o.init_property("_alpha", *gettersetter, *gettersetter);

	gettersetter = &character::visible_getset;
	o.init_property("_visible", *gettersetter, *gettersetter);

	gettersetter = &character::width_getset;
	o.init_property("_width", *gettersetter, *gettersetter);

	gettersetter = &character::height_getset;
	o.init_property("_height", *gettersetter, *gettersetter);

	gettersetter = &character::rotation_getset;
	o.init_property("_rotation", *gettersetter, *gettersetter);

	gettersetter = &character::parent_getset;
	o.init_property("_parent", *gettersetter, *gettersetter);

	gettersetter = &character::target_getset;
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
			fn.arg(0));
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
	boost::intrusive_ptr<character> obj = new video_stream_instance(NULL, NULL, -1);
	obj->setDynamic();
	return as_value(obj.get()); // will keep alive
}

video_stream_instance::video_stream_instance(video_stream_definition* def,
		character* parent, int id)
	:
	character(parent, id),
	m_def(def),
	//m_video_source(NULL),
	_ns(NULL),
	_embeddedStream(false)
{
	//log_debug("video_stream_instance %p ctor", (void*)this);

	if ( m_def )
	{
		_embeddedStream = true;
		attachVideoProperties(*this);
	}

	set_prototype(getVideoInterface(*this));
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
	} else if (_embeddedStream) {
		character* parent = get_parent();
		assert(parent);
		sprite_instance* sprite = parent->to_movie();
		assert(sprite);

		int current_frame = sprite->get_current_frame();
		assert(m_def);

		// The returned image is owned by "m_def"
		std::auto_ptr<image::image_base> img = m_def->get_frame_data(current_frame);
		if (img.get())
		{
			gnash::render::drawVideoFrame(img.get(), &m, &bounds);
		} else {
			log_debug(_("Video frame data is missing in frame %d"),current_frame);
		}
	}

	clear_invalidated();
}

void
video_stream_instance::stagePlacementCallback()
{
    saveOriginalTarget(); // for softref

    // Register this video instance as a live character
    _vm.getRoot().addLiveChar(this);
}


void
video_stream_instance::advance()
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
		cl=new builtin_function(&video_ctor, getVideoInterface(global));
		global.getVM().addStatic(cl.get());

		// replicate all interface to class, to be able to access
		// all methods as static functions
		//attachVideoInterface(*cl);
	}

	// Register _global.Video
	global.init_member("Video", cl.get());
}

geometry::Range2d<float>
video_stream_instance::getBounds() const
{
	if (_embeddedStream) return m_def->get_bound().getRange();

	geometry::Range2d<float> bounds; // null bounds..

	// TODO: return the bounds of the dynamically
	//       loaded video if not embedded ?
	return bounds;
}

#ifdef GNASH_USE_GC
void
video_stream_instance::markReachableResources() const
{
	if ( _ns ) _ns->setReachable();

	// Invoke character's version of reachability mark
	markCharacterReachable();
}
#endif // GNASH_USE_GC

} // end of namespace gnash
