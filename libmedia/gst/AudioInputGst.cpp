// AudioInputGst.cpp: Audio input processing using Gstreamer.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gst/gst.h"
#include "AudioInputGst.h"
#include "log.h"
#include "rc.h"

#include <cstring>
#include <gst/interfaces/propertyprobe.h>

namespace {
    //get rc file for default mic selection
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

namespace gnash {
namespace media {
namespace gst {

GnashAudio::GnashAudio() {
    _element = NULL;
    _devLocation = NULL;
    _gstreamerSrc = NULL;
    _productName = NULL;
}

GnashAudioPrivate::GnashAudioPrivate() {
    audioSource = NULL;
    audioEnc = NULL;
    _audioDevice = NULL;
    _deviceName = NULL;
    _pipeline = NULL;
    _audioMainBin = NULL;
    _audioPlaybackBin = NULL;
    _audioSourceBin = NULL;
    _audioSaveBin = NULL;
    _pipelineIsPlaying = false;
    _mux = NULL;
}

AudioInputGst::AudioInputGst() 
    :
    _activityLevel(-1),
    _gain(50),
    _index(0),
    _muted(true),
    _rate(8000),
    _silenceLevel(10),
    _silenceTimeout(2000), 
    _useEchoSuppression(false)
{
    gst_init(NULL,NULL);
    
    findAudioDevs();
    
    int devSelection = makeAudioDevSelection();
    _index = devSelection;
    
    transferToPrivate(devSelection);
    audioCreateMainBin(_globalAudio);
    audioCreatePlaybackBin(_globalAudio);
    audioCreateSaveBin(_globalAudio);
}

AudioInputGst::~AudioInputGst()
{
}

void
AudioInputGst::findAudioDevs() 
{
    
    //enumerate audio test sources
    GstElement *element;
    element = gst_element_factory_make ("audiotestsrc", "audtestsrc");
    
    if (element == NULL) {
        log_error("%s: Could not create audio test source", __FUNCTION__);
	return;
    } else {
        _audioVect.push_back(new GnashAudio);
        _audioVect.back()->setElementPtr(element);
        _audioVect.back()->setGstreamerSrc(g_strdup_printf("audiotestsrc"));
        _audioVect.back()->setProductName(g_strdup_printf("audiotest"));
    }
    
#ifdef HAS_GSTREAMER_PLUGINS_BASE
    //detect pulse audio sources
    GstPropertyProbe *probe;
    GValueArray *devarr;
    element = NULL;

    element = gst_element_factory_make ("pulsesrc", "pulsesrc");
    if ( ! element ) {
        log_error("%s: Could not create pulsesrc element", __FUNCTION__);
        return;
    }
    probe = GST_PROPERTY_PROBE (element);
    if ( ! probe ) {
        log_error("%s: Could not get property probe from pulsesrc element",
            __FUNCTION__);
        return;
    }
    devarr = gst_property_probe_probe_and_get_values_name (probe, "device");
    for (size_t i = 0; devarr != NULL && i < devarr->n_values; ++i) {
        GValue *val;
        gchar *dev_name = NULL;
        
        val = g_value_array_get_nth (devarr, i);
        g_object_set (element, "device", g_value_get_string (val), NULL);
        gst_element_set_state (element, GST_STATE_PLAYING);
        g_object_get (element, "device-name", &dev_name, NULL);
        gst_element_set_state (element, GST_STATE_NULL);
        if ((strcmp(dev_name, "null") == 0) ||
                (std::strstr(dev_name, "Monitor") != NULL)) {
            log_debug("No pulse audio input devices.");
        }
        else { 
            _audioVect.push_back(new GnashAudio);
            _audioVect.back()->setElementPtr(element);
            _audioVect.back()->setGstreamerSrc(g_strdup_printf("pulsesrc"));
            _audioVect.back()->setProductName(dev_name);
            
            gchar *location;
            g_object_get (element, "device", &location , NULL);
            _audioVect.back()->setDevLocation(location);
        }
    }
    if (devarr) {
        g_value_array_free (devarr);
    }
#endif
}

bool
AudioInputGst::checkSupportedFormats(GstCaps *caps) 
{
    gint num_structs;
    
    num_structs = gst_caps_get_size (caps);
    bool ok = false;
    
    for (gint i = 0; i < num_structs; i++) {
        GstStructure *structure;
        
        //this structure is used to probe the source for information
        structure = gst_caps_get_structure (caps, i);
        
        //check to see if x-raw-int and/or x-raw-float are available to
        //use with the selected microphone
        if (!gst_structure_has_name (structure, "audio/x-raw-int") &&
            !gst_structure_has_name (structure, "audio/x-raw-float")) 
        {
          continue;
        } else {
            ok = true;
        }
    }
    return ok;
}

void
AudioInputGst::getSelectedCaps(int devselect)
{

    if (devselect < 0 ||
            (static_cast<size_t>(devselect) >= _audioVect.size())) {
        log_error("%s: passed an invalid devselect argument", __FUNCTION__);
        exit(EXIT_FAILURE);
    }

    GstElement *pipeline;
    gchar *command;
    GError *error = NULL;
    GstStateChangeReturn return_val;
    GstBus *bus;
    GstMessage *message;
    
    GnashAudio *data_struct = _audioVect[devselect];
    
    //create tester pipeline to enumerate properties
    command = g_strdup_printf ("%s name=src device=%s ! fakesink",
        data_struct->getGstreamerSrc(), data_struct->getDevLocation());
    pipeline = gst_parse_launch(command, &error);
    if ((pipeline != NULL) && (error == NULL)) {
        //Wait at most 5 seconds for the pipeline to start playing
        gst_element_set_state (pipeline, GST_STATE_PLAYING);
        return_val = 
            gst_element_get_state (pipeline, NULL, NULL, 5 * GST_SECOND);
        
        //errors on bus?
        bus = gst_element_get_bus (pipeline);
        message = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0);
        
        if (GST_IS_OBJECT(bus)){
            gst_object_unref (bus);
        } else {
            log_error("%s: Pipeline bus isn't an object for some reason",
                __FUNCTION__);
        }
        //if everything above worked properly, begin probing for values
        if ((return_val == GST_STATE_CHANGE_SUCCESS) && (message == NULL)) {
            GstElement *src;
            GstPad *pad;
            GstCaps *caps;
            
            gst_element_set_state(pipeline, GST_STATE_PAUSED);
            
            src = gst_bin_get_by_name(GST_BIN(pipeline), "src");
            
            //get the pad, find the capabilities for probing in supported formats
            pad  = gst_element_get_pad (src, "src");
            caps = gst_pad_get_caps (pad);
            if (GST_IS_OBJECT(pad)) {
                gst_object_unref (pad);
            } else {
                log_error("%s: Template pad isn't an object for some reason",
                    __FUNCTION__);
            }
            bool ok = checkSupportedFormats(caps);
            if (ok) {
                log_error("The input device you selected isn't supported (yet)");
            } else {
                gst_caps_unref(caps);
            }
        }
        gst_element_set_state (pipeline, GST_STATE_NULL);
        if (GST_IS_OBJECT(pipeline)){
            gst_object_unref (pipeline);
        } else {
            log_error("%s: pipeline isn't an object for some reason",
                __FUNCTION__);
        }
    }
   
    if (error) {
      g_error_free (error);
    }
    g_free (command);
}

GnashAudioPrivate*
AudioInputGst::transferToPrivate(int devselect) {

    if (devselect < 0 ||
            (static_cast<size_t>(devselect) >= _audioVect.size())) {

        log_error("%s: Passed a bad devselect value", __FUNCTION__);
        exit (EXIT_FAILURE);
    }
    GnashAudioPrivate *audio = new GnashAudioPrivate;
    if (audio != NULL) {
        audio->setAudioDevice(_audioVect[devselect]);
        audio->setDeviceName(_audioVect[devselect]->getProductName());
        _globalAudio = audio;
    } else {
        log_error("%s: was passed a NULL pointer", __FUNCTION__);
    }
    return audio;
}

gboolean
AudioInputGst::audioChangeSourceBin(GnashAudioPrivate *audio)
{
    GError *error = NULL;
    gchar *command = NULL;
    
    if (audio->_pipelineIsPlaying == true) {
        audioStop(audio);
    }
    
    //delete the old source bin if necessary (please don't delete the == NULL
    //here as it breaks things.)
    if (!(GST_ELEMENT_PARENT(audio->_audioSourceBin) == NULL)) {
        gst_bin_remove(GST_BIN(audio->_audioMainBin),
                audio->_audioSourceBin);
        audio->_audioSourceBin = NULL;
    }
    
    if (strcmp(audio->_deviceName, "audiotest") == 0) {
        log_debug("%s: You don't have any mics chosen, using audiotestsrc",
            __FUNCTION__);
        audio->_audioSourceBin = gst_parse_bin_from_description (
            "audiotestsrc name=audioSource",
            TRUE, &error);
        log_debug("Command: audiotestsrc name=audioSource");
        audio->audioSource = gst_bin_get_by_name (
                GST_BIN (audio->_audioSourceBin), "audioSource");
        return true;
    } 

    command = g_strdup_printf ("%s name=audioSource device=%s ! capsfilter name=capsfilter caps=audio/x-raw-int,signed=true,channels=2,rate=%i;audio/x-raw-float,channels=2,rate=%i ! rgvolume pre-amp=%f",
        audio->_audioDevice->getGstreamerSrc(),
        audio->_audioDevice->getDevLocation(),
        _rate, _rate, gstgain());
    
    log_debug ("GstPipeline command is: %s\n", command);
    
    audio->_audioSourceBin = gst_parse_bin_from_description(command, TRUE,
                                &error);
    if (audio->_audioSourceBin == NULL) {
        log_error ("%s: Creation of the audioSourceBin failed",
            __FUNCTION__);
        log_error ("the error was %s\n", error->message);
        return false;
    }
    g_free(command);
    audio->audioSource = gst_bin_get_by_name(
            GST_BIN (audio->_audioSourceBin), "audioSource");
                
    gboolean result;
    result = gst_bin_add(GST_BIN(audio->_audioMainBin),
            audio->_audioSourceBin);
    if (!result) {
        log_error("%s: couldn't drop the sourcebin back into the main bin",
            __FUNCTION__);
        return false;
    }

    GstElement *tee = gst_bin_get_by_name(GST_BIN(audio->_audioMainBin),
        "tee");
    result = gst_element_link(audio->_audioSourceBin, tee);

    if (!result) {
        log_error("%s: couldn't link up sourcebin and tee", __FUNCTION__);
        return false;
    } 
    _globalAudio = audio;
    return true;
} 

gboolean
AudioInputGst::audioCreateSourceBin(GnashAudioPrivate *audio) 
{
    GError *error = NULL;
    gchar *command = NULL;
    if(strcmp(audio->_deviceName, "audiotest") == 0) {
        log_debug("%s: You don't have any mics chosen, using audiotestsrc",
            __FUNCTION__);
        audio->_audioSourceBin = gst_parse_bin_from_description (
            "audiotestsrc name=audioSource",
            TRUE, &error);
        log_debug("Command: audiotestsrc name=audioSource");
        audio->audioSource = gst_bin_get_by_name (GST_BIN (audio->_audioSourceBin),
                    "audioSource");
        return true;
    } else {
    command = g_strdup_printf ("%s name=audioSource device=%s ! capsfilter name=capsfilter caps=audio/x-raw-int,signed=true,channels=2,rate=%i;audio/x-raw-float,channels=2,rate=%i ! rgvolume pre-amp=%f",
        audio->_audioDevice->getGstreamerSrc(),
        audio->_audioDevice->getDevLocation(),
        _rate, _rate, gstgain());
    
    log_debug ("GstPipeline command is: %s", command);
    
    audio->_audioSourceBin = gst_parse_bin_from_description(command, TRUE,
                                &error);
    if (audio->_audioSourceBin == NULL) {
        log_error ("%s: Creation of the audioSourceBin failed",
            __FUNCTION__);
        log_error ("the error was %s", error->message);
        return false;
    }
    g_free(command);
    audio->audioSource = gst_bin_get_by_name (GST_BIN (audio->_audioSourceBin),
                "audioSource");
    return true;
    }
}

gboolean
AudioInputGst::audioCreateMainBin(GnashAudioPrivate *audio) 
{
    GstElement *tee, *audioPlaybackQueue, *saveQueue;
    gboolean ok;
    GstPad  *pad;
    
    //initialize a new GST pipeline
    audio->_pipeline = gst_pipeline_new("pipeline");
    
    audio->_audioMainBin = gst_bin_new ("audioMainBin");
    
    ok = audioCreateSourceBin(audio);
    if (ok != true) {
        log_error("%s: audioCreateSourceBin failed!", __FUNCTION__);
        return false;
    }
    if ((tee = gst_element_factory_make ("tee", "tee")) == NULL) {
        log_error("%s: problem creating tee element", __FUNCTION__);
        return false;
    }
    if ((saveQueue = gst_element_factory_make("queue", "saveQueue")) == NULL) {
        log_error("%s: problem creating save_queue element", __FUNCTION__);
        return false;
    }
    if ((audioPlaybackQueue = 
        gst_element_factory_make("queue", "audioPlaybackQueue")) == NULL) {
        log_error("%s: problem creating audioPlaybackQueue element", __FUNCTION__);
        return false;
    }
    gst_bin_add_many (GST_BIN (audio->_audioMainBin), audio->_audioSourceBin,
                    tee, saveQueue, audioPlaybackQueue, NULL);
    ok = gst_element_link(audio->_audioSourceBin, tee);
    if (ok != true) {
        log_error("%s: couldn't link audioSourceBin and tee", __FUNCTION__);
        return false;
    }
    ok &= gst_element_link_many (tee, saveQueue, NULL);
    if (ok != true) {
        log_error("%s: couldn't link tee and saveQueue", __FUNCTION__);
        return false;
    }
    ok &= gst_element_link_many (tee, audioPlaybackQueue, NULL);
    if (ok != true) {
        log_error("%s: couldn't link tee and audioPlaybackQueue", __FUNCTION__);
        return false;
    }
    
    gst_bin_add (GST_BIN(audio->_pipeline), audio->_audioMainBin);
   
    //add ghostpad to saveQueue (allows connections between bins)
    pad = gst_element_get_pad (saveQueue, "src");
    if (pad == NULL) {
        log_error("%s: couldn't get saveQueueSrcPad", __FUNCTION__);
        return false;
    }
    gst_element_add_pad (audio->_audioMainBin,
        gst_ghost_pad_new ("saveQueueSrc", pad));
    gst_object_unref (GST_OBJECT (pad));
    
    //add ghostpad to video_display_queue
    pad = gst_element_get_pad (audioPlaybackQueue, "src");
    if (pad == NULL) {
        log_error("%s: couldn't get audioPlaybackQueue", __FUNCTION__);
        return false;
    }
    gst_element_add_pad (audio->_audioMainBin,
        gst_ghost_pad_new ("audioPlaybackQueueSrc", pad));
    gst_object_unref (GST_OBJECT (pad));


    if (!ok) {
        log_error("%s: Unable to create main pipeline", __FUNCTION__);
        return false;
    } else {
        return true;
    }
}

gboolean
AudioInputGst::audioCreatePlaybackBin(GnashAudioPrivate *audio) 
{
    GstElement* autosink;
    GstPad* pad;
    gboolean ok;
    
    audio->_audioPlaybackBin = gst_bin_new("playbackBin");
    
    if ((autosink = gst_element_factory_make ("autoaudiosink", "audiosink")) == NULL) {
         log_error("%s: There was a problem making the audiosink!", __FUNCTION__);
         return false;
    }
    
    ok = gst_bin_add(GST_BIN(audio->_audioPlaybackBin), autosink);
    
    //create ghostpad which can be used to connect this bin to the
    //video_display_queue src ghostpad
    pad = gst_element_get_pad (autosink, "sink");
    gst_element_add_pad (audio->_audioPlaybackBin, gst_ghost_pad_new ("sink", pad));
    gst_object_unref (GST_OBJECT (pad));
    
    return ok;
}

gboolean
AudioInputGst::makeAudioSourcePlaybackLink(GnashAudioPrivate *audio) 
{
    if (gst_bin_get_by_name(GST_BIN(audio->_pipeline), "playbackBin") == NULL) {
        gst_object_ref(audio->_audioPlaybackBin);
        gst_bin_add(GST_BIN(audio->_pipeline), audio->_audioPlaybackBin);
    }
    
    GstPad *audioPlaybackQueueSrc, *audioPlaybackBinSink;
    GstPadLinkReturn padreturn;
    
    audioPlaybackQueueSrc = gst_element_get_pad(audio->_audioMainBin,
        "audioPlaybackQueueSrc");
    audioPlaybackBinSink = gst_element_get_pad(audio->_audioPlaybackBin,
        "sink");
    
    padreturn = gst_pad_link(audioPlaybackQueueSrc, audioPlaybackBinSink);
    
    if (padreturn == GST_PAD_LINK_OK) {
        return true;
    } else {
        log_error("something went wrong in the makeSourcePlaybackLink function");
        return false;
    }
}

gboolean
AudioInputGst::breakAudioSourcePlaybackLink(GnashAudioPrivate *audio) 
{
    if (audio->_pipelineIsPlaying == true) {
        audioStop(audio);
    }
    
    gboolean ok;
    GstPad *audioPlaybackQueueSrc, *audioPlaybackBinSink;
    GstStateChangeReturn state;
    
    audioPlaybackQueueSrc = gst_element_get_pad(audio->_audioMainBin,
        "audioPlaybackQueueSrc");
    audioPlaybackBinSink = gst_element_get_pad(audio->_audioPlaybackBin,
        "sink");
    
    ok = gst_pad_unlink(audioPlaybackQueueSrc, audioPlaybackBinSink);
    if (ok != true) {
        log_error("%s: unlink failed", __FUNCTION__);
        return false;
    } else {
        state = gst_element_set_state(audio->_audioPlaybackBin, GST_STATE_NULL);
        if (state != GST_STATE_CHANGE_FAILURE) {
            //return true;
            ok = gst_bin_remove(GST_BIN(audio->_pipeline), audio->_audioPlaybackBin);
            if (ok != true) {
                log_error("%s: couldn't remove audioPlaybackBin from pipeline",
                    __FUNCTION__);
                return false;
            } else {
                return true;
            }
        } else {
            log_error("%s: changing state of audioPlaybackBin failed", __FUNCTION__);
            return false;
        }
    }
}

//to handle messages while the main capture loop is running
gboolean
audio_bus_call (GstBus* /*bus*/, GstMessage *msg, gpointer /*data*/)
{
  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
        log_debug ("End of stream\n");
        break;
    
    case GST_MESSAGE_ERROR: {
        gchar  *debug;
        GError *error;

        gst_message_parse_error (msg, &error, &debug);
        g_free (debug);

        log_error ("Error: %s\n", error->message);
        g_error_free (error);
        break;
    }
    default:
        break;
  }

  return TRUE;
}

gboolean
AudioInputGst::audioCreateSaveBin(GnashAudioPrivate* audio) 
{
    GstElement *audioConvert, *audioEnc, *filesink;
    GstPad* pad;
    
    audio->_audioSaveBin = gst_bin_new ("audioSaveBin");
    
    if ((audioConvert = gst_element_factory_make("audioconvert", "audio_convert")) == NULL) {
        log_error("%s: Couldn't make audioconvert element", __FUNCTION__);
        return false;
    }
    if ((audioEnc = gst_element_factory_make("vorbisenc", "audio_enc")) == NULL){
        log_error("%s: Couldn't make vorbisenc element", __FUNCTION__);
        return false;
    }
    if ((audio->_mux = gst_element_factory_make("oggmux", "mux")) == NULL) {
        log_error("%s: Couldn't make oggmux element", __FUNCTION__);
        return false;
    }
    if ((filesink = gst_element_factory_make("filesink", "filesink")) == NULL) {
        log_error("%s: Couldn't make filesink element", __FUNCTION__);
        return false;
    } else {
        g_object_set(filesink, "location", "audioOut.ogg", NULL);
    }
    
    gst_bin_add_many(GST_BIN(audio->_audioSaveBin), audioConvert, audioEnc,
        audio->_mux, filesink, NULL);
    
    pad = gst_element_get_pad(audioConvert, "sink");
    gst_element_add_pad(audio->_audioSaveBin, gst_ghost_pad_new ("sink", pad));
    gst_object_unref (GST_OBJECT (pad));
    
    //gst_bin_add (GST_BIN(audio->_pipeline), audio->_audioSaveBin);
    
    bool ok = gst_element_link_many(audioConvert, audioEnc, audio->_mux,
            filesink, NULL);

    if (!ok) {
        log_error("%s: Something went wrong in linking", __FUNCTION__);
        return false;
    }

    return true;
}

gboolean
AudioInputGst::makeAudioSourceSaveLink (GnashAudioPrivate* audio) 
{
    if (gst_bin_get_by_name(GST_BIN(audio->_pipeline), "audioSaveBin") == NULL) {
        gst_object_ref(audio->_audioSaveBin);
        gst_bin_add(GST_BIN(audio->_pipeline), audio->_audioSaveBin);
    }
    
    GstPad *audioSaveQueueSrc, *audioSaveBinSink;
    GstPadLinkReturn padreturn;
    
    audioSaveQueueSrc = gst_element_get_pad(audio->_audioMainBin,
        "saveQueueSrc");
    audioSaveBinSink = gst_element_get_pad(audio->_audioSaveBin,
        "sink");
    
    padreturn = gst_pad_link(audioSaveQueueSrc, audioSaveBinSink);
    
    if (padreturn == GST_PAD_LINK_OK) {
        return true;
    } else {
        log_error("something went wrong in the makeAudioSourceSaveLink function");
        return false;
    }
}

gboolean
AudioInputGst::breakAudioSourceSaveLink (GnashAudioPrivate *audio) 
{
    if (audio->_pipelineIsPlaying == true) {
        audioStop(audio);
    }
    gboolean ok;
    GstPad *audioSaveQueueSrc, *audioSaveBinSink;
    GstStateChangeReturn state;
    
    audioSaveQueueSrc = gst_element_get_pad(audio->_audioMainBin,
        "saveQueueSrc");
    audioSaveBinSink = gst_element_get_pad(audio->_audioSaveBin,
        "sink");
    
    ok = gst_pad_unlink(audioSaveQueueSrc, audioSaveBinSink);
    if (ok != true) {
        log_error("%s: unlink failed", __FUNCTION__);
        return false;
    } else {
        state = gst_element_set_state(audio->_audioSaveBin, GST_STATE_NULL);
        if (state != GST_STATE_CHANGE_FAILURE) {
            ok = gst_bin_remove(GST_BIN(audio->_pipeline), audio->_audioSaveBin);
            if (ok != true) {
                log_error("%s: couldn't remove saveBin from pipeline", __FUNCTION__);
                return false;
            } else {
                return true;
            }
        } else {
            log_error("%s: audioSaveBin state change failed", __FUNCTION__);
            return false;
        }
    }
}

bool
AudioInputGst::audioPlay(GnashAudioPrivate *audio) 
{
    GstStateChangeReturn state;
    GstBus *bus;
    
    //setup bus to watch pipeline for messages
    bus = gst_pipeline_get_bus (GST_PIPELINE (audio->_pipeline));
    gst_bus_add_watch (bus, audio_bus_call, audio);
    gst_object_unref (bus);
    
    state = gst_element_set_state (audio->_pipeline, GST_STATE_PLAYING);
    
    if (state != GST_STATE_CHANGE_FAILURE) {
        audio->_pipelineIsPlaying = true;
        return true;
    } else {
        return false;
    }
}

bool
AudioInputGst::audioStop(GnashAudioPrivate *audio) 
{
    GstStateChangeReturn state;
    
    state = gst_element_set_state (audio->_pipeline, GST_STATE_NULL);
    
    if (state != GST_STATE_CHANGE_FAILURE) {
        audio->_pipelineIsPlaying = false;
        return true;
    } else {
        return false;
    }
}

int
AudioInputGst::makeAudioDevSelection() 
{
    int devselect = -1;
    devselect = rcfile.getAudioInputDevice();
    if (devselect == -1) {
        log_debug("No default audio input device specified, setting to testsrc");
        rcfile.setAudioInputDevice(0);
        devselect = rcfile.getAudioInputDevice();
    } else {
        log_debug("You've specified audio input %d in gnashrc, using that one",
            devselect);
    }
    
    //make sure device selection is a valid input device
    const int audioDevice = rcfile.getAudioInputDevice();

    if (audioDevice < 0 ||
            static_cast<size_t>(audioDevice) >= _audioVect.size()) {
        log_error("You have an invalid microphone selected. Check "
                "your gnashrc file");
        exit(EXIT_FAILURE);
    } else {
        //set _name value for actionscript
        _name = _audioVect[devselect]->getProductName();
        
        getSelectedCaps(devselect);
    
        return devselect;
    }
}

} //gst namespace
} //media namespace
} //gnash namespace
