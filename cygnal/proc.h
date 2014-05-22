// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
//

#ifndef __PROC_H__
#define __PROC_H__ 1

#include <string>
#include <map>

#include <mutex>

#include "network.h"
#include "dsodefs.h"

namespace cygnal
{
  
class Proc : public gnash::Network {
public:
    DSOEXPORT Proc (void);
    DSOEXPORT ~Proc (void);
    static Proc& getDefaultInstance();
    
    // These flags control whether the stdout of the child process gets displayed
    bool setOutput (const std::string &output, bool outflag);
    bool getOutput (const std::string &output);

    // This starts the process running via the usual fork() & exec()
    bool startCGI (void);
    bool startCGI (const std::string &filespec);
    bool startCGI (const std::string &filespec, std::uint16_t port);
    bool startCGI (const std::string &filespec, bool output);
    bool startCGI (const std::string &filespec, bool output, std::uint16_t port);

    void setDocroot(const std::string &path) { _docroot = path; } ;
    std::string &getDocroot() { return _docroot; };
    
    // This opens a network connection to the process
    bool connectCGI (const std::string &host, std::uint16_t port);

    // This finds the process
    int findCGI (const std::string &filespec);

    // This stop the process
    bool stopCGI (void);
    bool stopCGI (const std::string &filespec);
private:
    std::map<std::string, bool> _output;
    std::map<std::string, int>  _pids;
    std::map<std::string, int>  _cons;
    std::string                 _docroot;
    
    std::mutex	_mutex;
};

} // end of cygnal namespace

#endif  // end of __PROC_H__

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
