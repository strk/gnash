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

namespace gnash {

extern const int SOCKET_DATA;
  
class DSOLOCAL XMLSocket : public Network {
public:
    XMLSocket();
    ~XMLSocket();
    
    bool connect(const char *host, short port);
    bool send(std::string str);
    void close();

    typedef std::vector<std::string> MessageList;

    bool anydata(MessageList& msgs);
    bool anydata(int sockfd, MessageList& msgs);
    
    bool fdclosed() { return _closed; }
    bool xmlmsg() { return _xmldata; }
    
    void messagesClear()      { _messages.clear(); }
    void messageRemove(int x) { _messages.erase(_messages.begin() + x); }
    int messagesCount()       { return _messages.size(); }
    std::string operator [] (int x)  { return _messages[x]; }
    
    bool processingData();
    void processing(bool x);
    
    // Event Handlers
    void onClose(std::string);
    void onConnect(std::string);
    void onData(std::string);
    void onXML(std::string);
    
    // These handle the array of XML nodes
    //void push(as_object *obj) { _nodes.push_back(obj); }
    //void clear() { _nodes.clear(); }
    //int  count() { return _nodes.size(); }
    
    int checkSockets(void);
    int checkSockets(int x);
    
private:
    bool          _data;
    bool          _xmldata;
    bool          _closed;
    bool          _processing;
    std::vector<std::string> _messages;
    //std::vector< boost::intrusive_ptr<as_object> >  _nodes;

};

void xmlsocket_class_init(as_object& global);

} // end of gnash namespace


// __XMLSOCKETSOCKET_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
