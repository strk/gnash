#ifndef GNASH_VIDEO_STREAM_INSTANCE_H
#define GNASH_VIDEO_STREAM_INSTANCE_H

#include "character.h" // for inheritance
#include "video_stream_def.h"

namespace gnash {

class video_stream_instance : public character
{

public:

//	const as_object* m_source;
	video_stream_definition*	m_def;
	
	// m_video_source - A Camera object that is capturing video data or a NetStream object.
	// To drop the connection to the Video object, pass null for source.
	as_object* m_video_source;

	video_stream_instance(video_stream_definition* def,
			character* parent, int id);

	~video_stream_instance();

	virtual void	advance(float delta_time);
	void	display();

	bool can_handle_mouse_event() {	return false; }
	void get_invalidated_bounds(rect* bounds, bool force);

//	void set_source(const as_object* source)
//	{
//		m_source = source;
//	}

	//
	// ActionScript overrides
	//

//	virtual void set_member(const tu_stringi& name, const as_value& val);
//	virtual bool get_member(const tu_stringi& name, as_value* val);

	as_object* m_ns;

};

}	// end namespace gnash


#endif // GNASH_VIDEO_STREAM_INSTANCE_H
