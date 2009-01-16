// Video.cpp:  Draw individual video frames, for Gnash.
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


#include "MovieClip.h"
#include "Video.h"
#include "DefineVideoStreamTag.h"
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

namespace {    
    as_object* getVideoInterface(as_object& where);
    void attachPrototypeProperties(as_object& o);
    void attachVideoInterface(as_object& o);
    void attachVideoProperties(as_object& o);
    as_value video_ctor(const fn_call& fn);
    as_value video_attach(const fn_call& fn);
    as_value video_clear(const fn_call& fn);
    as_value video_deblocking(const fn_call& fn);
    as_value video_smoothing(const fn_call& fn);
    as_value video_width(const fn_call& fn);
    as_value video_height(const fn_call& fn);
}

Video::Video(SWF::DefineVideoStreamTag* def,
		character* parent, int id)
	:
	character(parent, id),
	m_def(def),
	_ns(0),
	_embeddedStream(m_def ? true : false),
	_lastDecodedVideoFrameNum(-1),
	_lastDecodedVideoFrame(),
    _smoothing(false)
{

	set_prototype(getVideoInterface(*this));
	if (_embeddedStream)
	{
		attachVideoProperties(*this);
		initializeDecoder();
        
        attachPrototypeProperties(*get_prototype());
	}
}

Video::~Video()
{
}

void
Video::initializeDecoder()
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

int
Video::width() const
{
    if (_ns) return _ns->videoWidth();
    return 0;
}

int
Video::height() const
{
    if (_ns) return _ns->videoHeight();
    return 0;
}

void
Video::clear()
{
    // Clear the current image only if paused.
    if (_ns && _ns->playbackState() == PlayHead::PLAY_PAUSED)
    {
        set_invalidated();
        _lastDecodedVideoFrame.reset();
    }
}

void
Video::display()
{
	// if m_def is NULL we've been constructed by 'new Video', in this
	// case I think display() would never be invoked on us...
	assert(m_def);

	SWFMatrix m = getWorldMatrix();
	const rect& bounds = m_def->get_bound();

	GnashImage* img = getVideoFrame();
	if (img)
	{
		gnash::render::drawVideoFrame(img, &m, &bounds, _smoothing);
	}

	clear_invalidated();
}

GnashImage*
Video::getVideoFrame()
{


	// If this is a video from a NetStream_as object, retrieve a video
    // frame from there.
	if (_ns)
	{
		std::auto_ptr<GnashImage> tmp = _ns->get_video();
		if ( tmp.get() ) _lastDecodedVideoFrame = tmp;
	}

	// If this is a video from a VideoFrame tag, retrieve a video frame
    // from there.
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
			log_debug("  current frame == _lastDecodedVideoFrameNum (%d)",
                    current_frame);
#endif
			return _lastDecodedVideoFrame.get();
		}

		int from_frame = _lastDecodedVideoFrameNum < 0 ?
            0 : _lastDecodedVideoFrameNum + 1;

		// If current frame is smaller then last decoded frame
		// we restart decoding from scratch
		if ( current_frame < _lastDecodedVideoFrameNum )
		{
#ifdef DEBUG_EMBEDDED_VIDEO_DECODING
			log_debug("  current frame (%d) < _lastDecodedVideoFrameNum (%d)",
                    current_frame, _lastDecodedVideoFrameNum);
#endif
			from_frame = 0;
		}

		// Reset last decoded video frame number now, so it's correct 
		// on early return (ie: nothing more to decode)
		_lastDecodedVideoFrameNum = current_frame;

#ifdef DEBUG_EMBEDDED_VIDEO_DECODING
		log_debug("  decoding embedded frames from %d to %d for Video "
                "object %s", from_frame, current_frame, getTarget());
#endif

		typedef SWF::DefineVideoStreamTag::EmbeddedFrames EncodedFrames;

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

		for (EncodedFrames::iterator it=toDecode.begin(),
                itEnd=toDecode.end(); it!=itEnd; ++it)
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
Video::stagePlacementCallback(as_object* initObj)
{

    // A Video cannot be created with an initObj
    assert(!initObj);

    saveOriginalTarget(); // for softref

    // Register this video instance as a live character
    _vm.getRoot().addLiveChar(this);
}


void
Video::advance()
{
	if (_ns) {
		//_ns->advance();
        
        // NOTE: only needed for gstreamer:
		if (_ns->newFrameReady()) set_invalidated();
	}
}

void
Video::add_invalidated_bounds(InvalidatedRanges& ranges, 
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
Video::setStream(boost::intrusive_ptr<NetStream_as> ns)
{
	_ns = ns;
	_ns->setInvalidatedVideo(this);
}

// extern (used by Global.cpp)
void
video_class_init(as_object& global)
{
	// This is going to be the global Video "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&video_ctor, getVideoInterface(global));
		global.getVM().addStatic(cl.get());
	}

	// Register _global.Video
	global.init_member("Video", cl.get());
}

rect
Video::getBounds() const
{
	if (_embeddedStream) return m_def->get_bound();

	// TODO: return the bounds of the dynamically
	//       loaded video if not embedded ?
	return rect();
}

#ifdef GNASH_USE_GC
void
Video::markReachableResources() const
{
	if ( _ns ) _ns->setReachable();

	// Invoke character's version of reachability mark
	markCharacterReachable();
}
#endif // GNASH_USE_GC

namespace {

as_object*
getVideoInterface(as_object& where)
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

void
attachVideoInterface(as_object& o)
{
	o.init_member("attachVideo", new builtin_function(video_attach));
	o.init_member("clear", new builtin_function(video_clear));
}

void
attachPrototypeProperties(as_object& proto)
{
    const int flags = as_prop_flags::dontDelete |
        as_prop_flags::readOnly;

    proto.init_property("deblocking", &video_deblocking, &video_deblocking,
            flags);
    proto.init_property("smoothing", &video_smoothing, &video_smoothing, flags);
    proto.init_property("height", &video_height, &video_height, flags);
    proto.init_property("width", &video_width, &video_width, flags);
}

void
attachVideoProperties(as_object& o)
{

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

as_value
video_attach(const fn_call& fn)
{
	boost::intrusive_ptr<Video> video = ensureType<Video>(fn.this_ptr);

	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("attachVideo needs 1 arg"));
		);
		return as_value();
	}

	boost::intrusive_ptr<NetStream_as> ns = 
        boost::dynamic_pointer_cast<NetStream_as>(fn.arg(0).to_object());
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

as_value
video_deblocking(const fn_call& fn)
{
	boost::intrusive_ptr<Video> video = ensureType<Video>(fn.this_ptr);
    UNUSED(video);

    log_unimpl("Video.deblocking");
    return as_value();
}

as_value
video_smoothing(const fn_call& fn)
{
	boost::intrusive_ptr<Video> video = ensureType<Video>(fn.this_ptr);

    if (!fn.nargs) return as_value(video->smoothing());

    bool smooth = fn.arg(0).to_bool();

    video->setSmoothing(smooth);

    return as_value();
}

as_value
video_width(const fn_call& fn)
{
	boost::intrusive_ptr<Video> video = ensureType<Video>(fn.this_ptr);
    return as_value(video->width());
}

as_value
video_height(const fn_call& fn)
{
	boost::intrusive_ptr<Video> video = ensureType<Video>(fn.this_ptr);
    return as_value(video->height());
}

as_value
video_clear(const fn_call& fn)
{
	boost::intrusive_ptr<Video> video = ensureType<Video>(fn.this_ptr);

    video->clear();
    return as_value();
}

as_value
video_ctor(const fn_call& /* fn */)
{
	log_debug("new Video() TESTING !");

	// I'm not sure We can rely on the def and parent values being accepted 
    // as NULL. Not till we add some testing...
	boost::intrusive_ptr<character> obj = new Video(NULL, NULL, -1);
	obj->setDynamic();
	return as_value(obj.get()); // will keep alive
}

} // anonymous namespace

} // end of namespace gnash



