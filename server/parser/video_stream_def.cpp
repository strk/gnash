#include "video_stream_def.h"
#include "video_stream_instance.h"

namespace gnash {


void
video_stream_definition::read(stream* in, SWF::tag_type tag, movie_definition* m)
{
	// Character ID has been read already 

	assert(tag == SWF::DEFINEVIDEOSTREAM ||
					tag == SWF::VIDEOFRAME);

	if (tag == SWF::DEFINEVIDEOSTREAM)
	{

		uint16_t numframes = in->read_u16();
		m_frames.resize(numframes);

		m_width = in->read_u16();
		m_height = in->read_u16();
		uint8_t reserved_flags = in->read_uint(5);
		m_deblocking_flags = in->read_uint(2);
		m_smoothing_flags = in->read_uint(1) ? true : false;

		m_codec_id = in->read_u8();
	}
	else
	if (tag == SWF::VIDEOFRAME)
	{
		uint16_t n = in->read_u16();
		m_frames[n] = NULL;
	}

}


character*
video_stream_definition::create_character_instance(character* parent, int id)
{
	character* ch = new video_stream_instance(this, parent, id);
	return ch;
}

}

