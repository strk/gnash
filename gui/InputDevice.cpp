// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "log.h"
#include "InputDevice.h"

namespace gnash {

InputDevice::InputDevice()
    : _type(InputDevice::UNKNOWN),
      _fd(-1),
      _x(0),
      _y(0),
      _button(0),
      _position(0)
{
    GNASH_REPORT_FUNCTION;
}

InputDevice::~InputDevice()
{
    GNASH_REPORT_FUNCTION;
}

bool
InputDevice::init(InputDevice::devicetype_e type)
{
    GNASH_REPORT_FUNCTION;
    return init(type, std::string(), DEFAULT_BUFFER_SIZE);
}

bool
InputDevice::init(InputDevice::devicetype_e type, size_t size)
{
    GNASH_REPORT_FUNCTION;

    return init(type, std::string(), size);
}

bool
InputDevice::init(InputDevice::devicetype_e type, const std::string &filespec,
                  size_t size)
{
    GNASH_REPORT_FUNCTION;

    _type = type;
    
    return init(std::string(), size);
}

// Read data into the Device input buffer.
int
InputDevice::readData()
{
    GNASH_REPORT_FUNCTION;

    if (_fd < 0) {
        return 0;   // no mouse available
    }
    
    int count;  
    
    unsigned char *ptr;
    
    ptr = _buffer.get() + _position;
    
    count = read(_fd, _buffer.get() + _position, _position - DEFAULT_BUFFER_SIZE);
    
    if (count <= 0) {
        return count;
    }
    
    /*
      printf("read data: ");
      int i;
      for (i=0; i<count; i++) 
      printf("%02x ", ptr[i]);
      printf("\n");
    */
    
    _position += count;

    return count;
}   

// end of gnash namespace
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
