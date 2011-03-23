// Video.cpp:  Draw individual video frames, for Gnash.
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
// 

#include "Video.h"

#include <boost/bind.hpp>
#include <cassert>

#include "MovieClip.h"
#include "DefineVideoStreamTag.h"
#include "fn_call.h"
#include "as_value.h"
#include "NetStream_as.h"
#include "NativeFunction.h" 
#include "movie_root.h"
#include "VM.h"
#include "MediaHandler.h" // for setting up embedded video decoder 
#include "VideoDecoder.h" // for setting up embedded video decoder
#include "namedStrings.h"
#include "Global_as.h"
#include "Renderer.h"
#include "RunResources.h"
#include "Transform.h"

// Define this to get debug logging during embedded video decoding
//#define DEBUG_EMBEDDED_VIDEO_DECODING

namespace gnash {

namespace {    
    void attachPrototypeProperties(as_object& o);
    void attachVideoInterface(as_object& o);
    as_value video_ctor(const fn_call& fn);
    as_value video_attach(const fn_call& fn);
    as_value video_clear(const fn_call& fn);
    as_value video_deblocking(const fn_call& fn);
    as_value video_smoothing(const fn_call& fn);
    as_value video_width(const fn_call& fn);
    as_value video_height(const fn_call& fn);
}

Video::Video(as_object* object,
        const SWF::DefineVideoStreamTag* def, DisplayObject* parent)
	:
	DisplayObject(getRoot(*object), object, parent),
	m_def(def),
	_ns(0),
	_embeddedStream(m_def),
	_lastDecodedVideoFrameNum(-1),
	_lastDecodedVideoFrame(),
    _smoothing(false)
{
    assert(object);
    assert(def);

    media::MediaHandler* mh = getRunResources(*object).mediaHandler();
	if (!mh) {
		LOG_ONCE( log_error(_("No Media handler registered, "
			"won't be able to decode embedded video")) );
		return;
	}

	media::VideoInfo* info = m_def->getVideoInfo();
	if (!info) return;

    try {
	    _decoder = mh->createVideoDecoder(*info);
	}
	catch (const MediaException& e) {
	    log_error("Could not create Video Decoder: %s", e.what());
	}
}

Video::~Video()
{
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
Video::display(Renderer& renderer, const Transform& base)
{
	assert(m_def);

    const DisplayObject::MaskRenderer mr(renderer, *this);

    const Transform xform = base * transform();
	const SWFRect& bounds = m_def->bounds();

    image::GnashImage* img = getVideoFrame();
	if (img) {
		renderer.drawVideoFrame(img, xform, &bounds, _smoothing);
	}

	clear_invalidated();
}

image::GnashImage*
Video::getVideoFrame()
{
	// If this is a video from a NetStream_as object, retrieve a video
    // frame from there.
	if (_ns) {
		std::auto_ptr<image::GnashImage> tmp = _ns->get_video();
		if (tmp.get()) _lastDecodedVideoFrame = tmp;
	}

	// If this is a video from a VideoFrame tag, retrieve a video frame
    // from there.
	else if (_embeddedStream) {

        // Don't try to do anything if there is no decoder. If it was
        // never constructed (most likely), we'll return nothing,
        // otherwise the last decoded frame.
        if (!_decoder.get()) {
		    LOG_ONCE(log_error(_("No Video info in video definition")));
            return _lastDecodedVideoFrame.get();
        }

        const boost::uint16_t current_frame = get_ratio(); 

#ifdef DEBUG_EMBEDDED_VIDEO_DECODING
		log_debug("Video instance %s need display video frame (ratio) %d",
			getTarget(), current_frame);
#endif

		// If current frame is the same then last decoded
		// we don't need to decode more
		if (_lastDecodedVideoFrameNum >= 0 &&
                _lastDecodedVideoFrameNum == current_frame) {
#ifdef DEBUG_EMBEDDED_VIDEO_DECODING
			log_debug("  current frame == _lastDecodedVideoFrameNum (%d)",
                    current_frame);
#endif
			return _lastDecodedVideoFrame.get();
		}

        // TODO: find a better way than using -1 to show that no
        // frames have been decoded yet.
        assert(_lastDecodedVideoFrameNum >= -1);
        boost::uint16_t from_frame = _lastDecodedVideoFrameNum + 1;

		// If current frame is smaller then last decoded frame
		// we restart decoding from scratch
		if (current_frame < static_cast<size_t>(_lastDecodedVideoFrameNum)) {
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

        const size_t frames = m_def->visitSlice(
                boost::bind(boost::mem_fn(&media::VideoDecoder::push),
                    _decoder.get(), _1),
                from_frame, current_frame);

        if (!frames) return _lastDecodedVideoFrame.get();

		_lastDecodedVideoFrame = _decoder->pop();
	}

	return _lastDecodedVideoFrame.get();
}

void
Video::construct(as_object* /*init*/)
{
    // For soft references.
    saveOriginalTarget();
}


void
Video::add_invalidated_bounds(InvalidatedRanges& ranges, bool force)
{	
	if (!force && !invalidated()) return; // no need to redraw
    
	ranges.add(m_old_invalidated_ranges);
	
	assert(m_def);

	SWFRect bounds;	
	bounds.expand_to_transformed_rect(getWorldMatrix(*this), m_def->bounds());
	
	ranges.add(bounds.getRange());            
}

void
Video::setStream(NetStream_as* ns)
{
	_ns = ns;
	_ns->setInvalidatedVideo(this);
}

// extern (used by Global.cpp)
void
video_class_init(as_object& global, const ObjectURI& uri)
{
	// This is going to be the global Video "class"/"function"
    Global_as& gl = getGlobal(global);
    as_object* proto = createObject(gl);
    as_object* cl = gl.createClass(&video_ctor, proto);
    attachVideoInterface(*proto);

	// Register _global.Video
	global.init_member(uri, cl, as_object::DefaultFlags);
}

void
registerVideoNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(video_ctor, 667, 0);
    vm.registerNative(video_attach, 667, 1);
    vm.registerNative(video_clear, 667, 2);
}

SWFRect
Video::getBounds() const
{
	if (_embeddedStream) return m_def->bounds();

	// TODO: return the bounds of the dynamically
	//       loaded video if not embedded ?
	return SWFRect();
}

void
Video::markOwnResources() const
{
	if (_ns) _ns->setReachable();
}

as_object*
createVideoObject(Global_as& gl)
{
    // TODO: how to use this for AS3 as well?
    // Turn into constructBuiltin()
    as_object* obj = getObjectWithPrototype(gl, NSV::CLASS_VIDEO);
    as_object* proto = obj->get_prototype();
    if (proto) attachPrototypeProperties(*proto);
    return obj;
}

namespace {

void
attachVideoInterface(as_object& o)
{
    VM& vm = getVM(o);
	o.init_member("attachVideo", vm.getNative(667, 1));
	o.init_member("clear", vm.getNative(667, 2));
}

void
attachPrototypeProperties(as_object& proto)
{
    const int protect = PropFlags::dontDelete;
    
    proto.init_property("deblocking", &video_deblocking, &video_deblocking,
            protect);
    proto.init_property("smoothing", &video_smoothing, &video_smoothing,
            protect);
    
    const int flags = PropFlags::dontDelete |
        PropFlags::readOnly;

    proto.init_property("height", &video_height, &video_height, flags);
    proto.init_property("width", &video_width, &video_width, flags);
}

as_value
video_attach(const fn_call& fn)
{
	Video* video = ensure<IsDisplayObject<Video> >(fn);

	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("attachVideo needs 1 arg"));
		);
		return as_value();
	}

    as_object* obj = toObject(fn.arg(0), getVM(fn));
	NetStream_as* ns;

    if (isNativeType(obj, ns)) {
		video->setStream(ns);
	}
	else {
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
	Video* video = ensure<IsDisplayObject<Video> >(fn);
    UNUSED(video);

    log_unimpl("Video.deblocking");
    return as_value();
}

as_value
video_smoothing(const fn_call& fn)
{
	Video* video = ensure<IsDisplayObject<Video> >(fn);

    if (!fn.nargs) return as_value(video->smoothing());

    const bool smooth = toBool(fn.arg(0), getVM(fn));
    video->setSmoothing(smooth);

    return as_value();
}

as_value
video_width(const fn_call& fn)
{
	Video* video = ensure<IsDisplayObject<Video> >(fn);
    return as_value(video->width());
}

as_value
video_height(const fn_call& fn)
{
	Video* video = ensure<IsDisplayObject<Video> >(fn);
    return as_value(video->height());
}

as_value
video_clear(const fn_call& fn)
{
	Video* video = ensure<IsDisplayObject<Video> >(fn);

    video->clear();
    return as_value();
}

as_value
video_ctor(const fn_call& /* fn */)
{
	return as_value();
}

} // anonymous namespace

} // end of namespace gnash



