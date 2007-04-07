// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

// 
// $Id: video_stream_instance.cpp,v 1.18 2007/04/07 11:55:50 tgc Exp $

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

	static as_value
	attach_video(const fn_call& fn)
	{
		boost::intrusive_ptr<video_stream_instance> video = ensureType<video_stream_instance>(fn.this_ptr);
	
		if (fn.nargs < 1)
		{
			IF_VERBOSE_ASCODING_ERRORS(
	    		log_aserror("attachVideo needs 1 arg");
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
	    		log_aserror("attachVideo(%s) first arg is not a NetStream instance.",
				fn.arg(0).to_debug_string().c_str());
			);
		}
		return as_value();
	}

	video_stream_instance::video_stream_instance(
		video_stream_definition* def, character* parent, int id)
	:
	character(parent, id),
	m_def(def),
	m_video_source(NULL),
	_ns(NULL)
//	m_source(NULL)
{
	assert(m_def);
	// FIXME: use new layout
	init_member("attachVideo", new builtin_function(attach_video));
	m_decoder = m_def->get_decoder();
}

video_stream_instance::~video_stream_instance()
{
	delete m_decoder;
}

void
video_stream_instance::display()
{
	matrix m = get_world_matrix();
	rect bounds(0.0f, 0.0f, PIXELS_TO_TWIPS(m_def->m_width), PIXELS_TO_TWIPS(m_def->m_height));

	// If this is a video from a NetStream object, retrieve a video frame from there.
	if (_ns)
	{
		boost::intrusive_ptr<NetStream> nso = _ns;

		if (nso->playing())
		{
			image::image_base* i = nso->get_video();
			if (i)
			{
				gnash::render::drawVideoFrame(i, &m, &bounds);
			}
		}

	// If this is a video from a VideoFrame tag, retrieve a video frame from there.
	} else if (m_decoder) {
		uint8_t* data = 0;
		int size = 0;
		int current_frame = get_parent()->to_movie()->get_current_frame();
		m_def->get_frame_data(current_frame, &data, &size);

		image::image_base* i = m_decoder->decodeFrame(data, size);
		if (i)
		{
			gnash::render::drawVideoFrame(i, &m, &bounds);
			delete i;
		} else {
			log_warning("An error occured while decoding video frame.");
		}

	}
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
	bool /*force*/)
{
	geometry::Range2d<float> bounds; 
	bounds.setWorld();
	ranges.add(bounds);
}

void
video_stream_instance::setStream(boost::intrusive_ptr<NetStream> ns)
{
	_ns = ns;
}

} // end of namespace gnash

