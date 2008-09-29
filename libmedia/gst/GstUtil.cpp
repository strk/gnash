// GstUtil.cpp: Generalized Gstreamer utilities for pipeline configuration.
//
//   Copyright (C) 2008 Free Software Foundation, Inc.
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

#include "GstUtil.h"
#include "log.h"

#include <gst/gst.h>
#include <sstream>


namespace gnash {
namespace media {

GstElement* GstUtil::get_audiosink_element()
{   
    //MUST be static to get a numbered name for each non-trivial pipeline created 
    static int numGnashRcSinks = 0;
    
    /*These MAY be static for CPU optimization
     *But, the memory cost at global scope is probably
     *worse overall than the CPU cost at initialization time.*/
    const std::string GNASHRCSINK = "gnashrcsink";
    const std::string sAudioSink =
      RcInitFile::getDefaultInstance().getGstAudioSink();
    
    //Can't be static. One of these must be created for each call
    GstElement* element;
    
    if(sAudioSink.find('!') != std::string::npos) //Found a non-trivial pipeline - bin it
    {
        element = gst_parse_bin_from_description(sAudioSink.c_str(), true, NULL);
        if(element != NULL)
        {
           std::ostringstream o;
           o << numGnashRcSinks++;
           gst_element_set_name(element, (GNASHRCSINK + o.str()).c_str());
        }
    }
    else //Found a trivial pipeline that doesn't need a bin
    {
        element = gst_element_factory_make(sAudioSink.c_str(), NULL);
    }
    
    if(!element)
    {
        log_debug(_("Unable to retrieve a valid audio sink from ~/.gnashrc"));
        
        element = gst_element_factory_make("autoaudiosink", NULL);
        
        if(!element)
        {
            log_debug(_("Unable to retrieve a valid audio sink from autoaudiosink"));
            
            element = gst_element_factory_make("gconfaudiosink", NULL);
            
            if(!element)
                log_error(_("Unable to retrieve a valid audio sink from gconfaudiosink\n%s"),
                        _("Sink search exhausted: you won't be able to hear sound!"));
        }
    }
    
    if(element)
    {
        log_debug(_("Got a non-NULL audio sink; its wrapper name is: %s"), _(GST_ELEMENT_NAME(element)));
    }
    
    return element;
}


// FIXME: decide on a single style for this file...
void
GstUtil::ensure_plugin_registered(const char* name, GType type)
{
  GstElementFactory* factory = gst_element_factory_find (name);

  if (!factory) {
    if (!gst_element_register (NULL, name, GST_RANK_PRIMARY,
          type)) {
      log_error("Failed to register our plugin %s. This may inhibit media "
                "playback.", name);
    }
  } else {
    gst_object_unref(GST_OBJECT(factory));
  }

  log_debug("element %s should now be registered", name);
}


} // gnash.media namespace 
} // namespace gnash

// Local Variables:
// mode: C++
// End:

