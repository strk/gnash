// sound_handler_ahi.cpp: Sound handling using standard AHI
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Based on sound_handler_sdl.cpp by Thatcher Ulrich http://tulrich.com 2003
// which has been donated to the Public Domain.


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "sound_handler_ahi.h"
#include "SoundInfo.h"
#include "EmbedSound.h"
#include "AuxStream.h" // for use..
#include "../../libcore/vm/VM.h"

#include "log.h" // will import boost::format too
#include "GnashException.h" // for SoundException

//#include <cmath>
#include <vector>
#include <boost/scoped_array.hpp>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/ahi.h>

#include <devices/ahi.h>
#include <exec/memory.h>

#define PLAYERTASK_NAME       "Gnash audio task"
#define PLAYERTASK_PRIORITY   2
#define RESET_TIME 30 * 1000

#define BUFSIZE  		7056 * 2
#define AHI_BUF_SIZE 	7056u

// Define this to get debugging call about pausing/unpausing audio
//#define GNASH_DEBUG_AOS4_AUDIO_PAUSING

// Mixing and decoding debugging
//#define GNASH_DEBUG_MIXING

static int
audioTaskWrapper()
{
	class gnash::sound::AOS4_sound_handler *obj = (gnash::sound::AOS4_sound_handler *)IDOS->GetEntryData();
	return obj->audioTask();
}

namespace { // anonymous

// Header of a wave file
// http://ftp.iptel.org/pub/sems/doc/full/current/wav__hdr_8c-source.html
typedef struct{
     char rID[4];            // 'RIFF'
     long int rLen;
     char wID[4];            // 'WAVE'
     char fId[4];            // 'fmt '
     long int pcm_header_len;   // varies...
     short int wFormatTag;
     short int nChannels;      // 1,2 for stereo data is (l,r) pairs
     long int nSamplesPerSec;
     long int nAvgBytesPerSec;
     short int nBlockAlign;
     short int nBitsPerSample;
} WAV_HDR;

// Chunk of wave file
// http://ftp.iptel.org/pub/sems/doc/full/current/wav__hdr_8c-source.html
typedef struct{
    char dId[4];            // 'data' or 'fact'
    long int dLen;
} CHUNK_HDR;

} // end of anonymous namespace

namespace gnash {
namespace sound {

AOS4_sound_handler::AOS4_sound_handler(const std::string& wavefile)
    :
    _audioOpened(false),
	_closing(false),
	_paused(true),
	_started(false)
{

    initAudio();

    if (! wavefile.empty() ) {
        file_stream.open(wavefile.c_str());
        if (file_stream.fail()) {
            std::cerr << "Unable to write file '" << wavefile << std::endl;
            exit(1);
        } else {
                write_wave_header(file_stream);
                std::cout << "# Created 44100 16Mhz stereo wave file:" << std::endl <<
                    "AUDIOFILE=" << wavefile << std::endl;
        }
    }

}

AOS4_sound_handler::AOS4_sound_handler()
    :
    _audioOpened(false),
	_closing(false),
	_paused(true),
	_started(false)
{
    initAudio();
}

AOS4_sound_handler::~AOS4_sound_handler()
{
    boost::mutex::scoped_lock lock(_mutex);

	// on class destruction we must kill the Audio thread
	IExec->Signal((struct Task*)AudioPump,SIGBREAKF_CTRL_C);

    lock.unlock();

    // we already locked, so we call
    // the base class (non-locking) deleter
    delete_all_sounds();

    unplugAllInputStreams();

    closeAudio();

    if (file_stream) file_stream.close();
}

void
AOS4_sound_handler::initAudio()
{
    openAudio();
}

void
AOS4_sound_handler::openAudio()
{
    if ( _audioOpened ) return; // nothing to do

	log_debug(_("AOS4: Spawn Audio Process.."));

	AudioPump = (struct Process *) IDOS->CreateNewProcTags (
		   							NP_Entry, 		(ULONG) audioTaskWrapper,
						   			NP_Name,	   	PLAYERTASK_NAME,
						   			NP_Input, 		IDOS->Input(),
									NP_Output, 		IDOS->Output(),
									NP_CloseInput, 	FALSE,
									NP_CloseOutput, FALSE,
									NP_StackSize,   262144,
									NP_Child,		TRUE,
									NP_Priority,	PLAYERTASK_PRIORITY,
		   							NP_EntryData, 	this,
						  			TAG_DONE);

	if (AudioPump)
	{
		_audioOpened = true;
		log_debug(_("AOS4: Audio Process spawned.."));
	}
	else
	{
		_audioOpened = false;
		log_error(_("Unable to create Audio Process!!"));
		throw SoundException("Unable to create Audio Process!!");
	}

}

void
AOS4_sound_handler::closeAudio()
{
	if (_closing == false)
	{
		_closing = true;
		log_debug(_("AOS4: Cleaning Audio Stuff.."));

		if (AHIios[0])
		{
			if (!IExec->CheckIO((struct IORequest *) AHIios[0]))
			{
				IExec->AbortIO((struct IORequest *) AHIios[0]);
				IExec->WaitIO((struct IORequest *) AHIios[0]);
			}
		}

		if(AHIios[1])
		{ // Only if the second request was started
			if (!IExec->CheckIO((struct IORequest *) AHIios[1]))
			{
	    		IExec->AbortIO((struct IORequest *) AHIios[1]);
	    		IExec->WaitIO((struct IORequest *) AHIios[1]);
			}
		}


		if(!AHIDevice)
			if (AHIio) 	IExec->CloseDevice((struct IORequest *)AHIio);

		if (AHIio) 		IExec->FreeSysObject(ASOT_IOREQUEST,(struct IORequest *)AHIio); AHIio = 0;
  		if (AHIiocopy) 	IExec->FreeMem(AHIiocopy,sizeof(struct AHIRequest)); AHIiocopy = 0;

	  	if (AHImp)		IExec->FreeSysObject(ASOT_PORT, AHImp); AHImp = 0;

		if (IAHI) 		IExec->DropInterface((struct Interface*) IAHI); IAHI = 0;

		if (PlayBuffer[0]) IExec->FreeMem(PlayBuffer[0],BUFSIZE); PlayBuffer[0] = 0;
		if (PlayBuffer[1]) IExec->FreeMem(PlayBuffer[1],BUFSIZE); PlayBuffer[1] = 0;

		log_debug(_("AOS4: Exit Audio Thread.."));
	}

    _audioOpened = false;
}

void
AOS4_sound_handler::reset()
{
    boost::mutex::scoped_lock lock(_mutex);
    sound_handler::delete_all_sounds();
    sound_handler::stop_all_sounds();
}

int
AOS4_sound_handler::create_sound(std::auto_ptr<SimpleBuffer> data,
                                std::auto_ptr<media::SoundInfo> sinfo)
{
    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::create_sound(data, sinfo);
}

sound_handler::StreamBlockId
AOS4_sound_handler::addSoundBlock(unsigned char* data,
        unsigned int dataBytes, unsigned int nSamples,
        int streamId)
{

    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::addSoundBlock(data, dataBytes, nSamples, streamId);
}

void
AOS4_sound_handler::stop_sound(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    sound_handler::stop_sound(soundHandle);
}


void
AOS4_sound_handler::delete_sound(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    sound_handler::delete_sound(soundHandle);
}

void
AOS4_sound_handler::stop_all_sounds()
{
    boost::mutex::scoped_lock lock(_mutex);
    sound_handler::stop_all_sounds();
}


int
AOS4_sound_handler::get_volume(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::get_volume(soundHandle);
}


void
AOS4_sound_handler::set_volume(int soundHandle, int volume)
{
    boost::mutex::scoped_lock lock(_mutex);
    sound_handler::set_volume(soundHandle, volume);
}

media::SoundInfo*
AOS4_sound_handler::get_sound_info(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::get_sound_info(soundHandle);
}

unsigned int
AOS4_sound_handler::get_duration(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::get_duration(soundHandle);
}

unsigned int
AOS4_sound_handler::tell(int soundHandle)
{
    boost::mutex::scoped_lock lock(_mutex);
    return sound_handler::tell(soundHandle);
}

sound_handler*
create_sound_handler_aos4()
// Factory.
{
    return new AOS4_sound_handler;
}

sound_handler*
create_sound_handler_aos4(const std::string& wave_file)
// Factory.
{
    return new AOS4_sound_handler(wave_file);
}

void
AOS4_sound_handler::write_wave_header(std::ofstream& outfile)
{

  // allocate wav header
  WAV_HDR wav;
  CHUNK_HDR chk;

  // setup wav header
  std::strncpy(wav.rID, "RIFF", 4);
  std::strncpy(wav.wID, "WAVE", 4);
  std::strncpy(wav.fId, "fmt ", 4);

  wav.nBitsPerSample = 16;
  wav.nSamplesPerSec = 44100;
  wav.nAvgBytesPerSec = 44100;
  wav.nAvgBytesPerSec *= wav.nBitsPerSample / 8;
  wav.nAvgBytesPerSec *= 2;
  wav.nChannels = 2;

  wav.pcm_header_len = 16;
  wav.wFormatTag = 1;
  wav.rLen = sizeof(WAV_HDR) + sizeof(CHUNK_HDR);
  wav.nBlockAlign = 2 * wav.nBitsPerSample / 8;

  // setup chunk header
  std::strncpy(chk.dId, "data", 4);
  chk.dLen = 0;

  /* write riff/wav header */
  outfile.write((char *)&wav, sizeof(WAV_HDR));

  /* write chunk header */
  outfile.write((char *)&chk, sizeof(CHUNK_HDR));

}

void
AOS4_sound_handler::fetchSamples(boost::int16_t* to, unsigned int nSamples)
{
	if (_closing == false)
	{
	    boost::mutex::scoped_lock lock(_mutex);
    	sound_handler::fetchSamples(to, nSamples);

	    // TODO: move this to base class !
    	if (file_stream)
	    {
    	    // NOTE: if muted, the samples will be silent already
        	boost::uint8_t* stream = reinterpret_cast<boost::uint8_t*>(to);
	        unsigned int len = nSamples*2;

    	    file_stream.write((char*) stream, len);

        	// now, mute all audio
	        std::fill(to, to+nSamples, 0);
    	}

	    // If nothing is left to play there is no reason to keep polling.
    	if ( ! hasInputStreams() )
	    {
#ifdef GNASH_DEBUG_AOS4_AUDIO_PAUSING
    	    log_debug("Pausing AOS4 Audio...");
#endif
			_paused = true;
    	}
	}
}

void
AOS4_sound_handler::mix(boost::int16_t* outSamples, boost::int16_t* inSamples, unsigned int nSamples, float volume)
{
    unsigned int nBytes = nSamples*2;
	if (_closing == false)
	{
		memcpy(BufferPointer, inSamples, nBytes);
		BufferPointer += nBytes;
		BufferFill    += nBytes;

		if (BufferFill >= BUFSIZE)
		{
			//while (!AHIReqSent[AHICurBuf] || IExec->CheckIO((struct IORequest *) AHIios[AHICurBuf]))
			{
				//printf("playing: Buffer:%d - Bufsize: %d\n",BufferFill, BUFSIZE);
				AHIRequest *req = AHIios[AHICurBuf];
				UWORD AHIOtherBuf = AHICurBuf^1;

				if (AHIReqSent[AHICurBuf])
				{
					if (req->ahir_Std.io_Data)
					{
						IExec->WaitIO((struct IORequest *)req);
						req->ahir_Std.io_Data = NULL;

						IExec->GetMsg(AHImp);
						IExec->GetMsg(AHImp);
					}
				}

				req->ahir_Std.io_Message.mn_Node.ln_Pri = 127;
				req->ahir_Std.io_Command  	= CMD_WRITE;
				req->ahir_Std.io_Offset   	= 0;
				req->ahir_Frequency       	= (ULONG) 44100;
				req->ahir_Volume          	= 0x10000;          // Full volume
				req->ahir_Position        	= 0x8000;           // Centered
				req->ahir_Std.io_Data     	= PlayBuffer[AHIOtherBuf];
				req->ahir_Std.io_Length  	= (ULONG) BUFSIZE;
				req->ahir_Type 			 	= AHIST_S16S;
				req->ahir_Link            	= (AHIReqSent[AHIOtherBuf] && !IExec->CheckIO((IORequest *) AHIios[AHIOtherBuf])) ? AHIios[AHIOtherBuf] : NULL;

				IExec->SendIO( (struct IORequest *) req);

				AHIReqSent[AHICurBuf] = true;
				AHICurBuf = AHIOtherBuf;

				BufferPointer = PlayBuffer[AHICurBuf];
				BufferFill = 0;
			}
			//signals = IExec->Wait(1 << AHImp->mp_SigBit);
		}
	}
}

void
AOS4_sound_handler::plugInputStream(std::auto_ptr<InputStream> newStreamer)
{
    boost::mutex::scoped_lock lock(_mutex);

    sound_handler::plugInputStream(newStreamer);

    { // TODO: this whole block should only be executed when adding
      // the first stream.

#ifdef GNASH_DEBUG_AOS4_AUDIO_PAUSING
        log_debug("Unpausing AOS4 Audio on inpust stream plug...");
#endif
		_paused = false;
	}
}

void
AOS4_sound_handler::mute()
{
    boost::mutex::scoped_lock lock(_mutedMutex);
    sound_handler::mute();
}

void
AOS4_sound_handler::unmute()
{
    boost::mutex::scoped_lock lock(_mutedMutex);
    sound_handler::unmute();
}

bool
AOS4_sound_handler::is_muted() const
{
    boost::mutex::scoped_lock lock(_mutedMutex);
    return sound_handler::is_muted();
}

void
AOS4_sound_handler::pause()
{
    //closeAudio();
	log_debug(_("AOS4: AOS4_sound_handler::pause"));
	_paused = true;
	log_debug(_("AOS4: paused"));

    sound_handler::pause();
}

void
AOS4_sound_handler::unpause()
{
    if ( hasInputStreams() )
    {
		log_debug(_("AOS4: AOS4_sound_handler::unpause"));
		_paused = false;
		log_debug(_("AOS4: unpaused"));
    }

    sound_handler::unpause();
}

void
AOS4_sound_handler::TimerExit(void)
{
	if (_timerio)
	{
		if (!IExec->CheckIO((struct IORequest *)_timerio))
		{
			IExec->AbortIO((struct IORequest *)_timerio);
			IExec->WaitIO((struct IORequest *)_timerio);
		}
	}

	if (_port)
	{
		IExec->FreeSysObject(ASOT_PORT, _port);
		_port = 0;
	}

	if (_timerio && _timerio->Request.io_Device)
	{
		IExec->CloseDevice((struct IORequest *)_timerio);
		IExec->FreeSysObject(ASOT_IOREQUEST, _timerio);
		_timerio = 0;
	}

	if (ITimer)
	{
		IExec->DropInterface((struct Interface *)ITimer);
		ITimer = 0;
	}
}

bool
AOS4_sound_handler::TimerInit(void)
{
	_port = (struct MsgPort *)IExec->AllocSysObject(ASOT_PORT, NULL);
	if (!_port) return FALSE;

	_timerSig = 1L << _port->mp_SigBit;

	_timerio = (struct TimeRequest *)IExec->AllocSysObjectTags(ASOT_IOREQUEST,
		ASOIOR_Size,		sizeof(struct TimeRequest),
		ASOIOR_ReplyPort,	_port,
	TAG_DONE);

	if (!_timerio) return FALSE;

	if (!IExec->OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *)
		_timerio, 0))
	{
		ITimer = (struct TimerIFace *)IExec->GetInterface((struct Library *)
			_timerio->Request.io_Device, "main", 1, NULL);
		if (ITimer) return TRUE;
	}

	return FALSE;
}

void
AOS4_sound_handler::TimerReset(uint32 microDelay)
{
	_timerio->Request.io_Command = TR_ADDREQUEST;
	_timerio->Time.Seconds = 0;
	_timerio->Time.Microseconds = microDelay;
	IExec->SendIO((struct IORequest *)_timerio);
}

int
AOS4_sound_handler::audioTask()
{
	uint32 sigMask;
	unsigned long clockAdvance = 40;
	unsigned int nSamples = (441*clockAdvance) / 10;
	unsigned int toFetch = nSamples*2;
	boost::int16_t samples[AHI_BUF_SIZE];

	_closing = false;

	AHIDevice = -1;

	PlayBuffer[0] = (UBYTE*)IExec->AllocMem(BUFSIZE, MEMF_SHARED|MEMF_CLEAR);
	PlayBuffer[1] = (UBYTE*)IExec->AllocMem(BUFSIZE, MEMF_SHARED|MEMF_CLEAR);
	if (!PlayBuffer[0] || !PlayBuffer[1])
	{
        log_error(_("AOS4: Unable to allocate memory for audio buffer!"));
  	    throw SoundException("AOS4: Unable to allocate memory for audio buffer!");
	}

	if ((AHImp=(struct MsgPort*) IExec->AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
	{
		if ((AHIio=(struct AHIRequest *) IExec->AllocSysObjectTags(ASOT_IOREQUEST,
																	ASOIOR_Size, 		sizeof(struct AHIRequest),
																	ASOIOR_ReplyPort, 	AHImp,
																	TAG_DONE)) != NULL)
		{
			AHIio->ahir_Version = 4;
			AHIDevice = IExec->OpenDevice(AHINAME, 0, (struct IORequest *)AHIio, 0);
			if (AHIDevice)
			{
				if (AHImp) IExec->FreeSysObject(ASOT_PORT, 		AHImp); AHImp = 0;
				if (AHIio) IExec->FreeSysObject(ASOT_IOREQUEST, (struct IORequest *)AHIio); AHIio = 0;
				if (PlayBuffer[0]) IExec->FreeMem(PlayBuffer[0],BUFSIZE); PlayBuffer[0] = 0;
				if (PlayBuffer[1]) IExec->FreeMem(PlayBuffer[1],BUFSIZE); PlayBuffer[1] = 0;

    	        log_error(_("AOS4: Unable to open AHI Device!"));
	    	    throw SoundException("AOS4: Unable to open AHI Device!");
			}
			IAHI = (struct AHIIFace *) IExec->GetInterface( (struct Library *) AHIio->ahir_Std.io_Device, "main", 1, NULL );
		}
		else
		{
			if (PlayBuffer[0]) IExec->FreeMem(PlayBuffer[0],BUFSIZE); PlayBuffer[0] = 0;
			if (PlayBuffer[1]) IExec->FreeMem(PlayBuffer[1],BUFSIZE); PlayBuffer[1] = 0;
			if (AHImp) IExec->FreeSysObject(ASOT_PORT, AHImp); AHImp = 0;
            log_error(_("AOS4: Unable to CreateIORequest!"));
	        throw SoundException("AOS4: Unable to CreateIORequest!");
		}
	}
	else
	{
		if (PlayBuffer[0]) IExec->FreeMem(PlayBuffer[0],BUFSIZE); PlayBuffer[0] = 0;
		if (PlayBuffer[1]) IExec->FreeMem(PlayBuffer[1],BUFSIZE); PlayBuffer[1] = 0;
		log_error(_("AOS4: Unable to CreateMsgPort for AHI Device!"));
		throw SoundException("AOS4: Unable to CreateMsgPort for AHI Device!");
	}

	AHIiocopy = IExec->AllocSysObjectTags(ASOT_IOREQUEST,ASOIOR_Duplicate,AHIio,TAG_DONE);
	if(! AHIiocopy)
	{
		if (AHImp) IExec->FreeSysObject(ASOT_PORT, 		AHImp); AHImp = 0;
		if (AHIio) IExec->FreeSysObject(ASOT_IOREQUEST, (struct IORequest *)AHIio); AHIio = 0;
		if (PlayBuffer[0]) IExec->FreeMem(PlayBuffer[0],BUFSIZE); PlayBuffer[0] = 0;
		if (PlayBuffer[1]) IExec->FreeMem(PlayBuffer[1],BUFSIZE); PlayBuffer[1] = 0;
		log_error(_("AOS4: Not enough memory for AHIiocopy!"));
		throw SoundException("AOS4: Not enough memory for AHIiocopy!");
	}

	AHICurBuf = 0;

	Buffer = 0;
	BufferPointer = PlayBuffer[0];
	BufferFill = 0;

	AHIReqSent[0] = false;
	AHIReqSent[1] = false;

	AHIios[0]=AHIio;
	AHIios[1]=(AHIRequest*)AHIiocopy;

	log_debug(_("AOS4: audioTask:Initialize timer.."));
	TimerInit();

	log_debug(_("AOS4: audioTask:Starting Timer.."));
	TimerReset(RESET_TIME);
    while (true)
    {
		sigMask = SIGBREAKF_CTRL_C | _timerSig;

		uint32 sigGot = IExec->Wait(sigMask);

	    if (sigGot & SIGBREAKF_CTRL_C)
	    {
			log_debug(_("AOS4: Closing Audio Thread.."));
			break;
		}
		if (sigGot & _timerSig)
    	{
			if (_paused == false)
			{
				while (toFetch)
				{
					unsigned int n = std::min(toFetch, AHI_BUF_SIZE);
					fetchSamples((boost::int16_t*)&samples, n);
					toFetch -= n;
				}
				toFetch = nSamples*2;
			}
			TimerReset(RESET_TIME);
	    }
	}
	log_debug(_("AOS4: audioTask:Close timer.."));
	TimerExit();
	return(0);
}

} // gnash.sound namespace
} // namespace gnash

// Local Variables:
// mode: C++
// End:

