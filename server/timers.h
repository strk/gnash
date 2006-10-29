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

// 
//
//

#ifndef __TIMERS_H__
#define __TIMERS_H__

#include "action.h"
#include "impl.h"

#include "tu_timer.h"

namespace gnash {
  
  struct variable {
    tu_string name;
    as_value value;
  };

  class Timer
    {
    public:
      Timer();
      Timer(as_value *obj, int ms);
      Timer(as_value method, int ms);
      ~Timer();
      int setInterval(as_value obj, int ms);
      int setInterval(as_value obj, int ms, as_object *this_ptr, as_environment *env);
      int setInterval(as_value obj, int ms, as_environment *env);
      int setInterval(as_value obj, int ms, std::vector<variable *> *locals);
      void setInterval(int ms) 
      {
        _interval = ms * 0.000001;
      }

      void clearInterval();
      void start();
      bool expired();
      void setObject(as_object *ao) { _object = ao; }
      as_object *getObject() { return _object; }
      
      // Accessors
      const as_value& getASFunction() { return _function;  }
      as_environment *getASEnvironment() { return _env;  }
      as_object *getASObject() { return _object;  }
      std::vector<struct variable *> *getLocals() { return _locals;  }
      int getIntervalID()  { return _which;  }
      void add_local(tu_string name, as_value value) {
        struct variable *var = new struct variable;
        var->name = name;
        var->value = value;
        _locals->push_back(var);
      }
      

    private:
      int             _which;                // Which timer
      double          _interval;
      double          _start;
      as_value        _function;
      as_object      *_object;
      as_environment *_env;
      std::vector<struct variable *> *_locals;
      
    };
  
  class timer_as_object : public gnash::as_object
  {
  public:
    Timer obj;
  };
  
  void timer_setinterval(const fn_call& fn);
  void timer_clearinterval(const fn_call& fn);
  void timer_expire(const fn_call& fn);
  
} // end of namespace gnash

  // __TIMERS_H_
#endif
