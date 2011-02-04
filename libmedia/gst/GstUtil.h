// GstUtil.h: Generalized Gstreamer utilities for pipeline configuration.
// 
//   Copyright (C) 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef _GSTUTIL_H
#define	_GSTUTIL_H

#include <gst/gst.h>
#include "dsodefs.h" // DSOEXPORT

// GST_TIME_AS_MSECONDS not defined as of gst 0.10.9
// is defined as of gst 0.10.19
#ifndef GST_TIME_AS_MSECONDS
# define GST_TIME_AS_MSECONDS(time) ((time) / G_GINT64_CONSTANT (1000000))
#endif

namespace gnash {
namespace media {
namespace gst {

/// Generalized Gstreamer utilities for pipeline configuration.
//
/// @warning This class is not guaranteed to be thread-safe.
///
class DSOEXPORT GstUtil {
	
public: 
        
 /**
 * \brief Returns a pointer to GstElement representing a user-configurable
 * audio sink, or NULL if such a sink could not be created.
 * 
 * get_audiosink_element() tries the following strategies to get an audiosink:
 * 1. Looks into ~/.gnashrc for a pipeline description given by the property
 *    GSTAudioSink
 * 2. If none (or invalid) is found, tries "autoaudiosink"
 * 3. If autoaudiosink comes up blank, tries "gconfaudiosink"
 * 4. If gconfaudiosink can't produce a non-NULL element, the method returns
 *    NULL.
 *
 * A pipeline description is specified in one of two ways in ~/.gnashrc:
 * 1. A "trivial" pipeline is simply the name of an element, like "fakesink"
 * 2. A "non-trivial" pipeline is a full sub-pipeline specification, given
 *    in a way that complies with the gst-launch manual page. The audiosink's
 *    sub-pipeline must accept x/raw-{int,float} data and feed it into a sink.
 *    The entire audiosink will be placed in a GstBin named "gnashrcsink" with
 *    a number based on the number of gnashrcsinks that have been allocated so
 *    far.
 */
    static GstElement* get_audiosink_element();


    /// Check for missing plugins and try to install them if necessary.
    //
    /// The installation, if applicable, will happen synchronously!
    ///
    /// @param caps Indicates the type of media to search for.
    /// @return if there is a decoder available to decode the passed type,
    ///         or if we succeeded in installing one, returns true. Otherwise,
    ///         returns false.
    static bool check_missing_plugins(GstCaps* caps);
        
private:

  GstUtil();
  ~GstUtil();
};

} // gnash.media.gst namespace
} // media namespace
} // gnash namespace


#endif	/* _GSTUTIL_H */

