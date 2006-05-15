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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "URL.h"

#include <string>
//#include <cstring>
#include <vector>
#include <stdexcept>
#include <cassert>
#include <sstream>
#include <algorithm>

// these are for stat(2)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <limits.h>

using namespace std;

namespace gnash {

/*private*/
void
URL::init_absolute(const string& in)
{
	// Find protocol
	string::size_type pos = in.find("://");
	if ( pos != string::npos )
	{
		// copy initial part to protocol
		_proto = in.substr(0, pos);

		// advance input pointer to past the :// part
		pos += 3;
		if ( pos == in.size() )
		{
			std::cerr << "protocol-only url!" << std::endl;
			throw runtime_error("protocol-only url");
		}

		// Find host 
		string::size_type pos1 = in.find('/', pos);
		if ( pos1 == string::npos )
		{
			// no slashes ? all hostname, I presume
			_host = in.substr(pos);
			_path = "/";
			return;
		}

		// copy hostname
		_host = in.substr(pos, pos1-pos);

		// next come path
		_path = in.substr(pos1);
	}
	else
	{
		_proto = "file";
		_path = in;
	}

	assert ( _path[0] == '/' );

	normalize_path(_path);
}

/*public*/
URL::URL(const string& absolute_url)
{
	//cerr << "URL(" << absolute_url << ")" << endl;
	if ( ( absolute_url.size() && absolute_url[0] == '/' )
		|| absolute_url.find("://") != string::npos )
	{
		//cerr << "It's absolute" << endl;
		init_absolute(absolute_url);
	}
	else
	{
		//cerr << "It's relative" << endl;
		char buf[PATH_MAX+1];
		if ( ! getcwd(buf, PATH_MAX) )
		{
			stringstream err;
			err << "getcwd failed: " << strerror(errno);
			throw std::runtime_error(err.str());
		}
		char* ptr = buf+strlen(buf);
		*ptr = '/';
		++ptr;
		*ptr = '\0';
		URL cwd(buf);
		init_relative(absolute_url, cwd);
	}
}

struct DupSlashes
{
	bool operator() (char a, char b) const
	{
		return ( a == '/' && b == '/' );
	}
};

/*private*/
void
URL::normalize_path(string& path)
{

	assert(path[0] == '/');

	//cerr << "path=" << path << endl;

	vector<string> components;

	string::iterator prev=path.begin();
	for (string::iterator curr=prev+1;
			curr != path.end();
			++curr )
	{
		if ( *curr == '/' )
		{
			string comp = string(prev+1, curr);
			//cerr << "comp:" << comp << endl;
			prev = curr;

			if ( comp == "" || comp == "." ) continue;
			if ( comp == ".." ) components.pop_back();
			else components.push_back(comp);
		}
	}
	// add last component 
	components.push_back(string(prev+1, path.end()));

	//cerr << "number of dir components:" << components.size() << endl;
	path = "";
	for (vector<string>::iterator i=components.begin(),
			e=components.end();
			i!=e; ++i)
	{
		path += "/" + *i;
		//cerr << "component:" << *i << endl;
	}

	//cerr << "after normalization: path=" << path << endl;
}

/*public*/
URL::URL(const string& relative_url, const URL& baseurl)
{
	init_relative(relative_url, baseurl);
}

/*private*/
void
URL::init_relative(const string& relative_url, const URL& baseurl)
{
	// If has a protocol, call absolute_url ctor
	if ( relative_url.find("://") != string::npos )
	{
		init_absolute(relative_url);
		return;
	}

//fprintf(stderr, " input=%s\n", in.c_str());

	// use protocol and host from baseurl
	_proto = baseurl._proto;
	_host = baseurl._host;

	if ( relative_url.size() && relative_url[0] == '/' ) 
	{
		// get path from here
		//_path.assign(in, strlen(in));
		_path = relative_url;
	}

	else // path-relative
	{
		string in = relative_url;

		// see how many dirs we want to take
		// off the baseurl path
		int dirsback=0;
		string::size_type pos;
		while ( ( pos = in.find("../") ) == 0 ) 
		{
			++dirsback;
			pos+=3;
			while (in[pos] == '/')
			{
				++pos;
			}
			in = in.substr(pos);
		}

//fprintf(stderr, "dirsback=%d, in=%s\n", dirsback, in.c_str());

		// find dirsback'th slash from end of
		// baseurl path
		string basedir = baseurl._path.substr(0,
			baseurl._path.find_last_of("/")+1);

//fprintf(stderr, "basedir=%s\n", basedir.c_str());

		assert(basedir[0] == '/');
		assert(*(basedir.rbegin()) == '/');

		string::size_type lpos =  basedir.size()-1;
		for (int i=0; i<dirsback; ++i)
		{
			if ( lpos == 0 ) break;
			string::size_type pos = basedir.rfind('/', lpos-1);
//fprintf(stderr, "slash %d at offset %d (rfind from %d)\n", i, pos, lpos-1);
			// no more slashes found, break and set at 1
			if ( pos == string::npos ) lpos = 1;
			else lpos = pos;
		}
		basedir.resize(lpos+1);

		// get dirname from basurl path
		//_path = basedir + relative_url;
		_path = basedir + in;

		normalize_path(_path);

	}

}

/*public*/
string
URL::str() const
{
	string ret = _proto + "://" + _host + _path;
	return ret;
}

ostream& operator<< (ostream& o, const URL& u)
{
	return o << u.str();
}

} // end of gnash namespace

