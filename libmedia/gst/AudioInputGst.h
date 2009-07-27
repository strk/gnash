// AudioInput.h: Audio input processing using Gstreamer.
// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_AUDIOINPUTGST_H
#define GNASH_AUDIOINPUTGST_H

#include <boost/cstdint.hpp> // for C99 int types
#include "gst/gst.h"
#include "AudioInput.h"
#include <vector>

namespace gnash {
namespace media {
namespace gst {

/// \class GnashAudio
///
/// \brief Contains information about audio input devices while enumerating
///  information about attached hardware. This class is also referred back to
///  by GnashAudioPrivate to re-access enumerated information.
class GnashAudio {
    public:
        /// \brief Accessor to retreive a the private _element variable
        ///       from the GnashAudio class which contains a pointer
        ///       to the audio source element.
        ///
        /// @return GstElement* to the audio source element
        GstElement* getElementPtr() {return _element;};
        
        /// \brief Accessor to set the private _element variable from
        ///       the GnashAudio class.
        ///
        /// @param element The GstElement pointer to the audio source element.
        void setElementPtr(GstElement* element) {_element = element;};
        
        /// \brief Accessor to get the private _devLocation variable from
        ///       the GnashAudio class.
        ///
        /// @return The _devLocation private variable from GnashAudio class.
        gchar* getDevLocation() {return _devLocation;};
        
        /// \brief Accessor to set the private _devLocation variable from
        ///       the GnashAudio class.
        ///
        /// @param l A gchar* containing the physical location of the audio
        ///       input hardware device.
        void setDevLocation(gchar *l) {_devLocation = l;};
        
        /// \brief Accessor to return the private _gstreamerSrc variable
        ///       from the GnashAudio class.
        ///
        /// @return The _gstreamerSrc variable from the GnashAudio class.
        ///        which should contain the type of the Gstreamer audio source
        ///        element (e.g. pulsesrc).
        gchar* getGstreamerSrc() {return _gstreamerSrc;};
        
        /// \brief Accessor to set the private _gstreamerSrc variable
        ///       from the GnashAudio class.
        ///
        /// @param s A gchar* containing the type of the Gstreamer source
        ///         element type (e.g. pulsesrc)
        void setGstreamerSrc(gchar *s) {_gstreamerSrc = s;};
        
        /// \brief Accessor to get the private _productName variable
        ///       from the GnashAudio class.
        ///
        /// @return A gchar* containing the audio input's hardware name
        ///       (e.g. HDA Intel).
        gchar* getProductName() {return _productName;};
        
        /// \brief Accessor to set the private _productName variable
        ///       from the GnashAudio class.
        ///
        /// @param n A gchar* to the hardware input device's hardware name
        ///         (e.g. HDA Intel).
        void setProductName(gchar *n) {_productName = n;};

        /// Constructor for the GnashAudio class.
        GnashAudio();
        
    private:
        /// \var GnashAudio::_element
        /// \brief GstElement* which points to the audio source
        ///       element.
        GstElement* _element;
        
        /// \var GnashAudio::_devLocation
        /// \brief Contains the physical location of the audio input device
        gchar* _devLocation;
        
        /// \var GnashAudio::_gstreamerSrc
        /// \brief Contains a gchar* which describes the gstreamer source
        ///       type (e.g. pulseaudiosrc or jackaudiosrc).
        gchar* _gstreamerSrc;
        
        /// \var GnashAudio::_productName
        /// \brief Contains a gchar* which describes the name of the hardware
        ///      device (e.g. Built-In Microphone or HDA Intel).
        gchar* _productName;
};

GnashAudio::GnashAudio() {
    _element = NULL;
    _devLocation = NULL;
    _gstreamerSrc = NULL;
    _productName = NULL;
}

/// \class GnashAudioPrivate
///
/// \brief This class is initialized once a hardware input device is chosen
///   it is more robust than GnashAudio because it has additional room to store
///   important Gstreamer information (pipelines, references to elements, etc.)
class GnashAudioPrivate {
    public:
        /// \var audioSource
        /// \brief A pointer to the Gstreamer element corresponding to the 
        ///   audio source (e.g. a built-in or usb microphone).
        GstElement *audioSource;
        
        /// \var audioEnc
        /// \brief A pointer to the audio encoder element of the Gstreamer
        ///   pipeline. The only format currently supported format is vorbis
        GstElement *audioEnc;
        
        /// Constructor for the GnashAudioPrivate class.
        GnashAudioPrivate();
        
        /// \brief This function sets the private _audioDevice element in
        ///   the GnashAudioPrivate class
        /// @param d A pointer to the GnashAudio class that you want to
        ///   use for audio input.
        void setAudioDevice(GnashAudio* d) {_audioDevice = d;}
        
        /// \brief This function returns the private _audioDevice element pointer
        ///   from the GnashAudioPrivate class.
        /// @return The GnashAudio* stored in the _audioDevice variable
        GnashAudio* getAudioDevice() {return _audioDevice;}
        
        /// \brief This function sets the private _deviceName element in the
        ///   GnashAudioPrivate class.
        /// @param n A gchar* describing the input hardware (e.g. HDA Intel)
        void setDeviceName(gchar* n) {_deviceName = n;}
        
        /// \brief This function returns the private _deviceName variable from the
        ///   GnashAudioPrivate class.
        /// @return The gchar* describing the physical device's name (e.g. HDA Intel)
        gchar* getDeviceName() {return _deviceName;}
    
    //FIXME: I can't figure out why this isn't working right. Since I made 
    // AudioInputGst inherit from GnashAudioPrivate it should be able to access
    // protected variables, but I can't get it to work!    
    //protected:
        /// \var _audioDevice
        /// \brief A pointer to the GnashAudio class of the selected hardware device
        ///   This info should be stored to the GnashAudioPrivate class in the
        ///   transferToPrivate function.
        GnashAudio* _audioDevice;
        
        /// \var _deviceName
        /// \brief A gchar* describing the physical input device's name
        ///   (e.g. HDA Intel or Built-In Microphone)
        gchar* _deviceName;
        
        /// \var _pipeline
        /// \brief A pointer to the main Gstreamer pipeline that all
        ///      created elements and bins will be dropped into.
        GstElement* _pipeline;
        
        /// \var _audioMainBin
        /// The main bin is set up to handle any number of connections to be made
        /// later in the program. The basic pipeline design is as follows:
        /// (sink ghostpad) tee ! audioPlaybackQueue
        /// (sink ghostpad) tee ! audioSaveQueue
        /// The source bin is dropped into the main bin and will eventually be
        /// fed into the tee element
        GstElement* _audioMainBin;
        
        /// \var _audioSourceBin
        /// The audio source bin contains the source device and a restriction on
        /// its capabilities. Currently a lot of stuff in here is hardcoded and
        /// will probably eventually be made into options that can be changed
        /// using setters. The basic pipeline design is as follows:
        /// <selected audio source> ! capsfiter
        /// The source bin is dropped into the _audioMainBin.
        GstElement* _audioSourceBin;
        
        /// \var _audioPlaybackBin
        /// The audio playback bin contains the elements necessary to playback
        /// the audio being captured by the selected device. Note that if you
        /// create the playback bin it will not automatically link up to the
        /// playback queue. To do that you need to call makeAudioSourcePlaybackLink()
        /// function. The basic pipeline design is as follows:
        /// autoaudiosink ! NULL
        GstElement* _audioPlaybackBin;
        
        /// \var _audioSaveBin
        /// The audio save bin contains the elements necessary to save the audio
        /// being captured to a file (currently just to an ogg file). Note that if
        /// you create the save bin it will not automatically link up to the
        /// save queue in the main bin. To do that you need to call the
        /// makeAudioSourceSaveLink() function. The basic pipeline structure is
        /// as follows:
        /// audioconvert ! vorbinenc ! oggmux ! filesink
        GstElement* _audioSaveBin;
        
        /// \var _mux
        /// \brief A direct link to the oggmux element in the _audioSaveBin for
        /// use with linking up to a video muxer so that audio and video are both
        /// muxed out to the same file.
        GstElement* _mux;
        
        /// \var _pipelineIsPlaying
        /// \brief A boolean value which stores whether or not the _pipeline
        /// element is currently in it's 'playing' state or not
        gboolean _pipelineIsPlaying;
        
        /// \var _loop
        /// \brief A GMainLoop to keep recording input from the device.
        GMainLoop* _loop;
};

GnashAudioPrivate::GnashAudioPrivate() {
    audioSource = NULL;
    audioEnc = NULL;
    _audioDevice = NULL;
    _deviceName = NULL;
    _pipeline = NULL;
    _audioSourceBin = NULL;
    _audioSaveBin = NULL;
    _pipelineIsPlaying = false;
    _loop = g_main_loop_new(NULL, false);
    _mux = NULL;
};

/// \class AudioInputGst
/// \brief The main AudioInputGst class, which actually doesn't store too
/// much important information (most of that is stored in the GnashAudio
/// and GnashAudioPrivate classes)
class AudioInputGst : public AudioInput, public GnashAudioPrivate {
	
public:
    /// \brief AudioInputGst class constructor
	AudioInputGst();

    /// \brief AudioInputGst class destructor
	~AudioInputGst();
    
    /// \brief This function enumerates information about the audio input devices
    /// attached to the machine and stores them in the _audioVect vector.
    /// @param Nothing.
    /// @return Nothing. All important information is now stored in the _audioVect
    ///   element.
    void findAudioDevs();
    
    /// \brief This function is currently the workhorse of this function. It
    /// looks in the gnashrc file and checks for a default microphone input device.
    /// If one is not selected, the audiotestsrc is used by default. This function
    /// also currently calls the functions to make the GstBins, pipelines and
    /// element connections.
    int makeAudioDevSelection();
    
    /// \brief This function grabs information about the selected audio input
    /// device. It also calls checkSupportedFormats to make sure that Gnash
    /// can handle the input formats supported by the source device.
    /// @param devselect The integer value describing the selected microphone
    ///  This should probably be changed eventually to a more robust selection
    ///  method (rather than a seemingly random integer value)
    void getSelectedCaps(int devselect);
    
    /// \brief This function checks the format information enumerated in
    ///  getSelectedCaps and makes sure that Gnash can handle said input.
    /// @param aud A pointer to a GnashAudio class that represents the
    ///  selected device.
    /// @param caps A pointer to the capabilities of the device as enumerated
    ///  in the getSelectedCaps function
    /// @return A boolean value (true means that the device has at least one 
    ///  supported format, false means that the device has no supported formats)
    bool checkSupportedFormats(GnashAudio *aud, GstCaps *caps);
    
    /// \brief This function transfers the selected audio device from a GnashAudio
    ///  class to a GnashAudioPrivate class. This function is called once the
    ///  device selection has been made.
    /// @param devselect The integer value describing the selected microphone.
    ///   This should probably be changed eventually to a more robust selection
    ///   method (rather than a seemingly random integer value)
    GnashAudioPrivate* transferToPrivate(int devselect);
    
    /// This function creates the main audio bin. A reference to this bin is
    /// stored in a GnashWebcamPrivate class structure under the _audioMainBin
    /// variable. See the description of _audioMainBin for a pipeline description.
    /// @param audio A pointer to the GnashAudioPrivate class structure of the
    ///  selected audio input device.
    /// @return True if the bin was created successfully, false otherwise.
    gboolean audioCreateMainBin (GnashAudioPrivate *audio);
    
    /// This function creates the audio source bin. A reference to this bin is
    /// stored in a GnashWebcamPrivate class structure under the _audioSourceBin
    /// variable. See the description of _audioSourceBin for a pipeline description.
    /// @param audio A pointer to the GnashAudioPrivate class structure of the
    ///   selected audio input device.
    /// @return True if the bin was created successfully, false otherwise.
    gboolean audioCreateSourceBin (GnashAudioPrivate *audio);
    
    /// This function creates the audio playback bin. A reference to this bin is
    /// stored in a GnashWebcamPrivate class structure under the _audioPlaybackBin
    /// variable. See the description of _audioPlaybackBin for a pipeline description.
    /// IMPORTANT: If you make the playback bin, it's not automatically linked up
    ///  and activated. You must call the makeAudioSourcePlaybackLink() function.
    /// @param audio A pointer to the GnashAudioPrivate class structure of the
    ///   selected audio input device.
    /// @return True if the bin was created successfully, false otherwise.
    gboolean audioCreatePlaybackBin (GnashAudioPrivate *audio);
    
    /// This function makes the link between the audio playback queue (which
    /// receives an audio stream from the source device) and the playback element.
    /// It's important to note that if you create the playback bin, you must
    /// make sure to call makeAudioSourcePlaybackLink() so that the links are
    /// all made properly.
    /// @param audio A pointer to the GnashAudioPrivate class structure of the
    ///   selected audio input device.
    /// @return True if the link was made successfully, false otherwise.
    gboolean makeAudioSourcePlaybackLink (GnashAudioPrivate *audio);
    
    /// This function makes the link between the audio save queue (which receives
    /// an audio stream from the source device) and the respective save elements.
    /// It's important to note that if you create the save bin you must make sure
    /// to call makeAudioSourceSaveLink() so that the links are all made properly
    /// and it can successfully activate when the pipeline starts.
    /// @param audio A pointer to the GnashAudioPrivate class structure of the
    ///   selected audio input device.
    /// @return True if the link was made successfully, false otherwise.
    gboolean makeAudioSourceSaveLink (GnashAudioPrivate *audio);
    
    /// This function creates the audio save bin. A reference to this bin is
    /// stored in a GnashWebcamPrivate class structure under the _audioSaveBin
    /// variable. See the description of _audioSaveBin for a pipeline description.
    /// IMPORTANT: If you make the save bin, it's not automatically linked up
    ///  and activated. You must call the makeAudioSourceSaveLink() function.
    /// @param audio A pointer to the GnashAudioPrivate class structure of the
    ///   selected audio input device.
    /// @return True if the bin was created successfully, false otherwise.
    gboolean audioCreateSaveBin (GnashAudioPrivate *audio);
    
    /// This function should be called when the desired pipeline is created. This
    /// will do the proper cleanup and activate the pipeline described in the audio
    /// parameter.
    /// @param audio A pointer to the GnashAudioPrivate class structure containing
    ///   the pipeline to start up.
    /// @return True if the pipeline started to play correctly, false otherwise.
    bool audioPlay(GnashAudioPrivate *audio);
    
    /// This function stops the audio pipeline created earlier in code execution.
    /// 
    /// @param audio A pointer tot he GnashAudioPrivate class structure containing
    ///  the pipeline to start up.
    /// @return True if the pipeline successfully stopped, false otherwise.
    bool audioStop(GnashAudioPrivate *audio);
    
    /// \brief Function returns the total number of devices detected (useful in
    ///  iterating through the _audioVect vector.
    /// @return The _numdevs private variable from the AudioInputGst class.
    int getNumdevs() {return _numdevs;}
    
    /// \brief Function sets the private _numdevs variable in the AudioInputGst
    /// class.
    /// @param n The integer value representing the desired value to set _numdevs
    /// to.
    void setNumdevs(int n) {_numdevs = n;}
    
    /// \brief Function returns a pointer to the private _audioVect element from
    ///  AudioInputGst class.
    std::vector<GnashAudio*>* getAudioVect() {return &_audioVect;}
    
    GnashAudioPrivate* getGlobalAudio() {return _globalAudio;}
    
    
private:
    /// \var _numdevs
    /// \brief Contains an integer value representing the number of devices found
    /// on the machine.
    int _numdevs;
    
    /// \var _audioVect
    /// \brief A vector of GnashAudio pointers. This is used when storing information
    ///  about attached devices. This vector is accessed using the integer value
    ///  from the gnashrc file (e.g. set microphoneDevice 2 means you're accessing
    ///  _audioVect[2])
    std::vector<GnashAudio*> _audioVect;
    
    /// \var _globalAudio
    /// \brief A global pointer to the GnashAudioPrivate class for the selected
    ///  device. This is useful so you don't have to keep relying on the _audioVect
    ///  vector.
    GnashAudioPrivate* _globalAudio;

};

} // gst namespace
} // gnash.media namespace 
} // gnash namespace

#endif // __AUDIOINPUT_H__
