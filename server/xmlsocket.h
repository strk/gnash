// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifndef __XMLSOCKET_H__
#define __XMLSOCKET_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"
#include "xml.h"
#include "impl.h"
#include "container.h"

#ifdef HAVE_LIBXML

namespace gnash {

extern const int SOCKET_DATA;
  
class DSOLOCAL XMLSocket {
 public:
  XMLSocket();
  ~XMLSocket();
  
  bool connect(const char *host, int port);
  bool send(tu_string str);
  void close();

  
  bool anydata(char **msgs);
  bool anydata(int sockfd, char **msgs);
  bool connected() { return _connect; };
  bool fdclosed() { return _closed; }
  bool xmlmsg() { return _xmldata; }
  
  void messagesClear()      { _messages.clear(); }
  void messageRemove(int x) { _messages.erase(_messages.begin() + x); }
  int messagesCount()       { return _messages.size(); }
  tu_string operator [] (int x)  { return _messages[x]; }
  
  bool processingData();
  void processing(bool x);
 
  // Event Handlers
  void onClose(tu_string);
  void onConnect(tu_string);
  void onData(tu_string);
  void onXML(tu_string);

  // These handle the array of XML nodes
  void push(as_object *obj);
  void clear();
  int  count();

  int checkSockets(void);
  int checkSockets(int x);

 private:
  tu_string     _host;
  short         _port;
  int           _sockfd;
  bool          _data;
  bool          _xmldata;
  bool          _closed;
  bool          _connect;
  bool          _processing;
  std::vector<tu_string> _messages;
  std::vector<as_object *>  _nodes;
};


class DSOLOCAL xmlsocket_as_object : public gnash::as_object
{
public:
  XMLSocket obj;
};

DSOEXPORT void xmlsocket_connect(const fn_call& fn);
DSOEXPORT void xmlsocket_send(const fn_call& fn);
DSOEXPORT void xmlsocket_xml_new(const fn_call& fn);
DSOEXPORT void xmlsocket_new(const fn_call& fn);
DSOEXPORT void xmlsocket_close(const fn_call& fn);

// These are the event handlers called for this object
DSOEXPORT void xmlsocket_event_ondata(const fn_call& fn);
DSOEXPORT void xmlsocket_event_close(const fn_call& fn);
DSOEXPORT void xmlsocket_event_connect(const fn_call& fn);
DSOEXPORT void xmlsocket_event_xml(const fn_call& fn);

DSOEXPORT int check_sockets(int fd);
 
} // end of gnash namespace

// HAVE_LIBXML
#endif

// __XMLSOCKETSOCKET_H__
#endif

