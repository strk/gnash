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
	    log_error("attachVideo needs 1 arg\n");
	    return;
		}

//		if (dynamic_cast<netstream_as_object*>(fn.arg(0).to_object()))
		if (video)
		{
			video->m_ns = static_cast<netstream_as_object*>(fn.arg(0).to_object());
		}
	}

	video_stream_instance::video_stream_instance(
		video_stream_definition* def, character* parent, int id)
	:
	character(parent, id),
	m_def(def),
	m_video_source(NULL),
	m_ns(NULL)
//	m_source(NULL)
{
	assert(m_def);
	set_member("attachVideo", &attach_video);
}

video_stream_instance::~video_stream_instance()
{
}

void
video_stream_instance::display()
{
	if (m_ns)
	{
		matrix m = get_world_matrix();
		rect bounds(0.0f, 0.0f, PIXELS_TO_TWIPS(m_def->m_width), PIXELS_TO_TWIPS(m_def->m_height));

		netstream_as_object* nso = static_cast<netstream_as_object*>(m_ns);

		if (nso->obj.playing())
		{
			YUV_video* v = nso->obj.get_video();
			if (v)
			{
				v->display(&m, &bounds);
			}
		}
	}
}

void
video_stream_instance::advance(float delta_time)
{
}

//void
//video_stream_instance::set_member(const tu_stringi& name,	const as_value& val)
//{
//	if (name == "attachVideo")
//	{
//		m_video_source = val.to_object();
//	}
//	set_member(name, val);
//}

//bool
//video_stream_instance::get_member(const tu_stringi& name, as_value* val)
//{
//	if (name == "attachVideo")
//	{
//		m_video_source = val->to_object();
//		val->set_as_object(m_video_source); // >set_bool(this->get_visible());
//		return true;
//	}
//	return false;
//}

} // end of namespace gnash

