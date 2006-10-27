#ifndef GNASH_VIDEO_STREAM_DEF_H
#define GNASH_VIDEO_STREAM_DEF_H

#include "character_def.h"
#include "stream.h" // for read()
#include "movie_definition.h"
#include "swf.h"

namespace gnash {

class video_stream_definition : public character_def
{
public:

//	video_stream_definition();
//	virtual ~video_stream_definition();


	character* create_character_instance(character* parent, int id);
	void	read(stream* in, SWF::tag_type tag, movie_definition* m);
	const rect&	get_bound() const	{
		return m_unused_rect;
	}

	uint16_t m_width;
	uint16_t m_height;

private:

//	uint8_t reserved_flags;
	uint8_t m_deblocking_flags;
	bool m_smoothing_flags;

	// 0: extern file
	// 2: H.263
	// 3: screen video (Flash 7+ only)
	// 4: VP6
	uint8_t m_codec_id;
	std::vector<void*>	m_frames;
	rect m_unused_rect;
};

}	// end namespace gnash


#endif // GNASH_VIDEO_STREAM_DEF_H
