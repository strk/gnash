// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef GNASH_ASOBJ_SOUND_H
#define GNASH_ASOBJ_SOUND_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "impl.h"
#include "as_object.h" // for inheritance
#include "NetConnection.h"

#include <boost/scoped_ptr.hpp>

/// Forward declarations
namespace gnash {
	class CharacterProxy;
	namespace media {
		class sound_handler;
	}
}

namespace gnash {

// Forward declarations
class fn_call;
  
class Sound : public as_object {
public:
	Sound();

	/// Make this sound control the given character
	//
	/// NOTE: 0 is accepted, to implement an "invalid"
	///       controller type.
	///
	void attachCharacter(character* attachedChar);

	virtual ~Sound() {}
	virtual void attachSound(int si, const std::string& name);

	/// Get number of bytes loaded from the external sound (if any)
	virtual long getBytesLoaded();

	/// Get total number of bytes in the external sound being loaded
	//
	/// @return -1 if unknown
	///
	virtual long getBytesTotal();

	virtual void getPan();
	virtual void getTransform();

	/// Get volume from associated resource
	//
	/// @return true of volume was obtained, false
	///         otherwise (for example if the associated
	///         character was unloaded).
	///
	bool getVolume(int& volume);
	void setVolume(int volume);

	virtual void loadSound(const std::string& file, bool streaming);
	virtual void setPan();
	virtual void setTransform();
	virtual void start(int offset, int loops);
	virtual void stop(int si);
	virtual unsigned int getDuration();
	virtual unsigned int getPosition();

	std::string soundName;

protected:

#ifdef GNASH_USE_GC
	/// Mark all reachable resources of a Sound, for the GC
	//
	/// Reachable resources are:
	///	- associated NetConnection object (connection)
	///	- attached character object (attachedCharacter)
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC

	bool _duration;
	bool _id3;
	bool _onID3;
	bool _onLoad;
	bool _onComplete;
	bool _position;

	boost::intrusive_ptr<NetConnection> connection;

	boost::scoped_ptr<CharacterProxy> attachedCharacter;
	int soundId;
	bool externalSound;
	std::string externalURL;
	bool isStreaming;

	media::sound_handler* _soundHandler;

};

void sound_class_init(as_object& global);

} // end of gnash namespace

// __SOUND_H__
#endif

