#include "video_stream_instance.h"
#include "video_stream_def.h"
#include "fn_call.h"
#include "as_value.h"
#include "NetStream.h"
#include "render.h"
//#include <SDL.h>

namespace gnash {

	static void
	attach_video(const fn_call& fn)
	{
		assert(dynamic_cast<video_stream_instance*>(fn.this_ptr));
		video_stream_instance* video = static_cast<video_stream_instance*>(fn.this_ptr);
	
		if (fn.nargs != 1)
		{
			IF_VERBOSE_ASCODING_ERRORS(
	    		log_aserror("attachVideo needs 1 arg");
			);
			return;
		}

		NetStream* ns = dynamic_cast<NetStream*>(fn.arg(0).to_object());
		if (ns)
		{
			video->setStream(ns);
		}
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
	init_member("attachVideo", &attach_video);
}

video_stream_instance::~video_stream_instance()
{
}

void
video_stream_instance::display()
{
	if (_ns)
	{
		matrix m = get_world_matrix();
		rect bounds(0.0f, 0.0f, PIXELS_TO_TWIPS(m_def->m_width), PIXELS_TO_TWIPS(m_def->m_height));

		NetStream* nso = _ns;

		if (nso->playing())
		{
			image::image_base* i = nso->get_video();
			if (i)
			{
				gnash::render::drawVideoFrame(i, &m, &bounds);
			}
		}
	}
}

void
video_stream_instance::advance(float /*delta_time*/)
{
}

void
video_stream_instance::get_invalidated_bounds(rect* bounds, bool /*force*/)
{
	bounds->expand_to_point(-1e10f, -1e10f);
	bounds->expand_to_point(1e10f, 1e10f);
}

} // end of namespace gnash

