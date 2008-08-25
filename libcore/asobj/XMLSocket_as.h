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

#ifndef GNASH_XMLSOCKET_H
#define GNASH_XMLSOCKET_H

#include "xml.h"
#include "impl.h"
#include "network.h"
#include "dsodefs.h"

#include <string>
#include <boost/thread.hpp>

namespace gnash {
  
class XMLSocket_as : public Network, public as_object {

public:

    typedef std::vector<std::string> MessageList;

    XMLSocket_as();
    ~XMLSocket_as();
    
    bool connect(const std::string& host, short port);

    // Actionscript doesn't care about the result of either of these
    // operations.
    void send(std::string str);
    void close();

	void checkForIncomingData();

private:

    bool fillMessageList(MessageList& msgs);

	/// Return the as_function with given name, converting case if needed
	boost::intrusive_ptr<as_function> getEventHandler(const std::string& name);

    bool _data;

    MessageList _messages;

    std::string _remainder;
    
    boost::mutex _dataMutex;

};

void xmlsocket_class_init(as_object& global);

} // end of gnash namespace

#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
