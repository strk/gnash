// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include <string>
#include <map>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

class Proc {
public:
    Proc (void);
    ~Proc (void);

    // These flags control whether the stdout of the child process gets displayed
    bool SetOutput (std::string, bool);
    bool GetOutput (std::string);

    // This starts the process
    bool Start (void);
    bool Start (std::string);
    bool Start (std::string, bool);

    // This finds the process
    int Find (std::string);

    // This stop the process
    bool Stop (void);
    bool Stop (std::string);
private:
    std::map<std::string, bool> _output;
    std::map<std::string, int> _pids;
    std::map<std::string, int> _cons;
    
    boost::mutex	_mutex;
};
