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
#include "NetStream_as.h"
#include "render.h"
#include "Range2d.h"
#include "builtin_function.h" // for getter/setter properties
#include "VM.h"
#include "Object.h"
#include "MediaHandler.h" // for setting up embedded video decoder 
#include "VideoDecoder.h" // for setting up embedded video decoder
#include "namedStrings.h"

// Define this to get debug logging during embedded video decoding
//#define DEBUG_EMBEDDED_VIDEO_DECODING

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
	o.init_property(NSV::PROP_uX, *gettersetter, *gettersetter);

	gettersetter = &character::y_getset;
	o.init_property(NSV::PROP_uY, *gettersetter, *gettersetter);

	gettersetter = &character::xscale_getset;
	o.init_property(NSV::PROP_uXSCALE, *gettersetter, *gettersetter);

	gettersetter = &character::yscale_getset;
	o.init_property(NSV::PROP_uYSCALE, *gettersetter, *gettersetter);

	gettersetter = &character::xmouse_get;
	o.init_readonly_property(NSV::PROP_uXMOUSE, *gettersetter);

	gettersetter = &character::ymouse_get;
	o.init_readonly_property(NSV::PROP_uYMOUSE, *gettersetter);

	gettersetter = &character::alpha_getset;
	o.init_property(NSV::PROP_uALPHA, *gettersetter, *gettersetter);

	gettersetter = &character::visible_getset;
	o.init_property(NSV::PROP_uVISIBLE, *gettersetter, *gettersetter);

	gettersetter = &character::width_getset;
	o.init_property(NSV::PROP_uWIDTH, *gettersetter, *gettersetter);

	gettersetter = &character::height_getset;
	o.init_property(NSV::PROP_uHEIGHT, *gettersetter, *gettersetter);

	gettersetter = &character::rotation_getset;
	o.init_property(NSV::PROP_uROTATION, *gettersetter, *gettersetter);

	gettersetter = &character::parent_getset;
	o.init_property(NSV::PROP_uPARENT, *gettersetter, *gettersetter);

	gettersetter = &character::target_getset;
	o.init_property(NSV::PROP_uTARGET, *gettersetter, *gettersetter);
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

	boost::intrusive_ptr<NetStream_as> ns = boost::dynamic_pointer_cast<NetStream_as>(fn.arg(0).to_object());
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

/*private*/
void
video_stream_instance::initializeDecoder()
{

	media::MediaHandler* mh = media::MediaHandler::get();
	if ( ! mh )
	{
		LOG_ONCE( log_error(_("No Media handler registered, "
			"won't be able to decode embedded video")) );
		return;
	}

	media::VideoInfo* info = m_def->getVideoInfo();
	if ( ! info )
	{
		log_error(_("No Video info in video definition"));
		return;
	}

    try
    {
	    _decoder = mh->createVideoDecoder(*info);
	}
	catch (MediaException &e)
	{
	    log_error("Could not create Video Decoder: %s", e.what());
	}
}

video_stream_instance::video_stream_instance(video_stream_definition* def,
		character* parent, int id)
	:
	character(parent, id),
	m_def(def),
	//m_video_source(NULL),
	_ns(NULL),
	_embeddedStream(false),
	_lastDecodedVideoFrameNum(-1),
	_lastDecodedVideoFrame()
{
	//log_debug("video_stream_instance %p ctor", (void*)this);

	if ( m_def )
	{
		_embeddedStream = true;
		attachVideoProperties(*this);
		initializeDecoder();
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

	SWFMatrix m = getWorldMatrix();
	const rect& bounds = m_def->get_bound();

	image::ImageBase* img = getVideoFrame();
	if (img)
	{
		gnash::render::drawVideoFrame(img, &m, &bounds);
	}

	clear_invalidated();
}

image::ImageBase*
video_stream_instance::getVideoFrame()
{


	// If this is a video from a NetStream_as object, retrieve a video frame from there.
	if (_ns)
	{
		std::auto_ptr<image::ImageBase> tmp = _ns->get_video();
		if ( tmp.get() ) _lastDecodedVideoFrame = tmp;
	}

	// If this is a video from a VideoFrame tag, retrieve a video frame from there.
	else if (_embeddedStream)
	{

        // Don't try to do anything if there is no decoder. If it was
        // never constructed (most likely), we'll return nothing,
        // otherwise the last decoded frame.
        if (!_decoder.get()) return _lastDecodedVideoFrame.get();

		int current_frame = get_ratio(); 

#ifdef DEBUG_EMBEDDED_VIDEO_DECODING
		log_debug("Video instance %s need display video frame (ratio) %d",
			getTarget(), current_frame);
#endif

		// If current frame is the same then last decoded
		// we don't need to decode more
		if ( _lastDecodedVideoFrameNum == current_frame )
		{
#ifdef DEBUG_EMBEDDED_VIDEO_DECODING
			log_debug("  current frame == _lastDecodedVideoFrameNum (%d)", current_frame);
#endif
			return _lastDecodedVideoFrame.get();
		}

		int from_frame = _lastDecodedVideoFrameNum < 0 ? 0 : _lastDecodedVideoFrameNum+1;

		// If current frame is smaller then last decoded frame
		// we restart decoding from scratch
		if ( current_frame < _lastDecodedVideoFrameNum )
		{
#ifdef DEBUG_EMBEDDED_VIDEO_DECODING
			log_debug("  current frame (%d) < _lastDecodedVideoFrameNum (%d)", current_frame, _lastDecodedVideoFrameNum);
#endif
			from_frame = 0;
		}

		// Reset last decoded video frame number now, so it's correct 
		// on early return (ie: nothing more to decode)
		_lastDecodedVideoFrameNum = current_frame;

#ifdef DEBUG_EMBEDDED_VIDEO_DECODING
		log_debug("  decoding embedded frames from %d to %d for video instance %s",
			from_frame, current_frame, getTarget());
#endif

		typedef std::vector<media::EncodedVideoFrame*> EncodedFrames;

		EncodedFrames toDecode;
		m_def->getEncodedFrameSlice(from_frame, current_frame, toDecode);

		// Nothing more to decode, return last decoded (possibly null)
		if ( toDecode.empty() )
		{
#ifdef DEBUG_EMBEDDED_VIDEO_DECODING
			log_debug("    no defined frames, we'll return last one");
#endif
			return _lastDecodedVideoFrame.get();
		}

		for (EncodedFrames::iterator it=toDecode.begin(), itEnd=toDecode.end(); it!=itEnd; ++it)
		{
			media::EncodedVideoFrame* frame = *it;
#ifdef DEBUG_EMBEDDED_VIDEO_DECODING
			log_debug("    pushing frame %d to decoder", frame->frameNum());
#endif
			_decoder->push(*frame);
		}

		_lastDecodedVideoFrame = _decoder->pop();
	}

	return _lastDecodedVideoFrame.get();
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
		//_ns->advance();
		if (_ns->newFrameReady()) set_invalidated(); // NOTE: only needed for gstreamer !!
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
	bounds.expand_to_transformed_rect(getWorldMatrix(), m_def->get_bound());
	
	ranges.add(bounds.getRange());            
}

void
video_stream_instance::setStream(boost::intrusive_ptr<NetStream_as> ns)
{
	_ns = ns;
	_ns->setInvalidatedVideo(this);
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

rect
video_stream_instance::getBounds() const
{
	if (_embeddedStream) return m_def->get_bound();

	// TODO: return the bounds of the dynamically
	//       loaded video if not embedded ?
	return rect();
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
