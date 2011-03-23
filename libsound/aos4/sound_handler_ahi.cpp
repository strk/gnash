// sound_handler_ahi.cpp: Sound handling using standard AHI
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "sound_handler_ahi.h"
#include "SoundInfo.h"
#include "EmbedSound.h"
#include "AuxStream.h" // for use..

#include "log.h" // will import boost::format too
#include "GnashException.h" // for SoundException
#include "GnashSleep.h" // for gnashSleep

//#include <cmath>
#include <vector>
#include <boost/scoped_array.hpp>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/ahi.h>

#include <devices/ahi.h>
#include <exec/memory.h>

#define PLAYERTASK_NAME       "Gnash audio task"
#define PLAYERTASK_PRIORITY   20
#define RESET_TIME 20 * 1000

#define BUFSIZE  		7056 * 4
#define AHI_BUF_SIZE 	28224u

// Define this to get debugging call about pausing/unpausing audio
//#define GNASH_DEBUG_AOS4_AUDIO_PAUSING

// Mixing and decoding debugging
//#define GNASH_DEBUG_MIXING

/* The volume ranges from 0 - 128 */
#define MIX_MAXVOLUME 128
#define ADJUST_VOLUME(s, v)	(s = (s*v)/MIX_MAXVOLUME)
#define ADJUST_VOLUME_U8(s, v)	(s = (((s-128)*v)/MIX_MAXVOLUME)+128)

int audioTaskID;

static int
audioTaskWrapper()
{
	class gnash::sound::AOS4_sound_handler *obj = (gnash::sound::AOS4_sound_handler *)IDOS->GetEntryData();
	return obj->audioTask();
}

namespace gnash {
namespace sound {

AOS4_sound_handler::AOS4_sound_handler(media::MediaHandler* m)
    :
    sound_handler(m),
    _audioOpened(false),
	_closing(false)
{
    initAudio();
}

AOS4_sound_handler::~AOS4_sound_handler()
{
    boost::mutex::scoped_lock lock(_mutex);

	// on class destruction we must kill the Audio thread
	_closing = true;
	sound_handler::pause();

	log_debug("Killing Audio Task..");

	IExec->Signal((struct Task*)AudioPump,SIGBREAKF_CTRL_C);

	IExec->WaitPort(_DMreplyport);
	IExec->GetMsg(_DMreplyport);

	if (_dmsg)			IExec->FreeSysObject(ASOT_MESSAGE, _dmsg); _dmsg = 0;
	if (_DMreplyport) 	IExec->FreeSysObject(ASOT_PORT, _DMreplyport); _DMreplyport = 0;

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
	
	_DMreplyport = (struct MsgPort*) IExec->AllocSysObjectTags(ASOT_PORT, TAG_DONE);
	_dmsg 		 = (struct DeathMessage*) IExec->AllocSysObjectTags(ASOT_MESSAGE,
																	ASOMSG_Size, sizeof(struct DeathMessage),
																	ASOMSG_ReplyPort, _DMreplyport,
																	TAG_DONE);
	
	if (!_dmsg || !_DMreplyport)
	{
		_audioOpened = false;
		log_error(_("Unable to create Death Message for child!!"));
		throw SoundException("Unable to create Death Message for child!!");
	}
	_dmsg->dm_Msg.mn_ReplyPort = _DMreplyport;
	_dmsg->dm_Msg.mn_Length    = (uint16)sizeof(*_dmsg);
	
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
									NP_NotifyOnDeathMessage, _dmsg,
		   							NP_EntryData, 	this,
						  			TAG_DONE);

	if (AudioPump)
	{
		//Get audio task pointer to kill it from the GUI.. it actually use a global variable.. better than nothing
		audioTaskID = (int)AudioPump;
		
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
create_sound_handler_aos4(media::MediaHandler* m)
// Factory.
{
    return new AOS4_sound_handler(m);
}

void
AOS4_sound_handler::fetchSamples(boost::int16_t* to, unsigned int nSamples)
{
	if (!_closing)
	{
		AHIRequest *req = AHIios[AHICurBuf];
		UWORD AHIOtherBuf = AHICurBuf^1;

	    boost::mutex::scoped_lock lock(_mutex);
    	if (!_closing) 
    	{
	    		sound_handler::fetchSamples(to, nSamples);
    	}
		else		
			return;
		
		//memcpy(BufferPointer, inSamples, nBytes);
		memcpy(BufferPointer, to, nSamples*2);

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
		req->ahir_Std.io_Length  	= (ULONG) nSamples*2; //BUFSIZE;
		req->ahir_Type 			 	= AHIST_S16S;
		req->ahir_Link            	= (AHIReqSent[AHIOtherBuf] && !IExec->CheckIO((IORequest *) AHIios[AHIOtherBuf])) ? AHIios[AHIOtherBuf] : NULL;

		IExec->SendIO( (struct IORequest *) req);

		AHIReqSent[AHICurBuf] = true;
		AHICurBuf = AHIOtherBuf;

		BufferPointer = PlayBuffer[AHICurBuf];
	}


    // If nothing is left to play there is no reason to keep polling.
   	if ( ! hasInputStreams() )
    {
#ifdef GNASH_DEBUG_AOS4_AUDIO_PAUSING
   	    log_debug("Pausing AOS4 Audio...");
#endif
		sound_handler::pause();
   	}
}

void 
AOS4_sound_handler::MixAudio (boost::uint8_t *dst, const boost::uint8_t *src, boost::uint32_t len, int volume)
{
	boost::uint16_t format;

	if ( volume == 0 ) 
	{
		return;
	}

	format = AHIST_S16S;

	/* Actually we have a fixed audio format */
	switch (format) 
	{
		case AHIST_S16S:
		{
			boost::int16_t src1, src2;
			int dst_sample;
			const int max_audioval = ((1<<(16-1))-1);
			const int min_audioval = -(1<<(16-1));

			len /= 2;
			while ( len-- ) 
			{
				src1 = ((src[0])<<8|src[1]);
				ADJUST_VOLUME(src1, volume);
				src2 = ((dst[0])<<8|dst[1]);
				src += 2;
				dst_sample = src1+src2;
				if ( dst_sample > max_audioval ) 
				{
					dst_sample = max_audioval;
				} 
				else
				if ( dst_sample < min_audioval ) 
				{
					dst_sample = min_audioval;
				}
				dst[1] = dst_sample & 0xFF;
				dst_sample >>= 8;
				dst[0] = dst_sample & 0xFF;
				dst += 2;
			}
		}
		break;
	}
	
}

void
AOS4_sound_handler::mix(boost::int16_t* outSamples, boost::int16_t* inSamples, unsigned int nSamples, float volume)
{
	if (!_closing)
	{
	    unsigned int nBytes = nSamples*2;

	    boost::uint8_t *out = reinterpret_cast<boost::uint8_t*>(outSamples);
    	boost::uint8_t* in = reinterpret_cast<boost::uint8_t*>(inSamples);

	    MixAudio(out, in, nBytes, static_cast<int>(MIX_MAXVOLUME*volume));
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
		sound_handler::unpause();
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
	sound_handler::pause();
	log_debug(_("AOS4: paused"));

    sound_handler::pause();
}

void
AOS4_sound_handler::unpause()
{
    if ( hasInputStreams() )
    {
		log_debug(_("AOS4: AOS4_sound_handler::unpause"));
		sound_handler::unpause();
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
			_closing = true;
			log_debug(_("AOS4: Closing Audio Thread.."));
			break;
		}
		if (sigGot & _timerSig)
    	{
			IExec->GetMsg(_port);
			if (!sound_handler::isPaused())
			{
				while (toFetch && !_closing)
				{
					unsigned int n = std::min(toFetch, AHI_BUF_SIZE);
					if (!_closing) fetchSamples((boost::int16_t*)&samples, n);
					toFetch -= n;
				}
				toFetch = nSamples*2;
			}
			if (!_closing) TimerReset(RESET_TIME);
	    }
	}

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

	log_debug(_("AOS4: audioTask:Close timer.."));
	TimerExit();
    return(RETURN_OK);
}

} // gnash.sound namespace
} // namespace gnash

// Local Variables:
// mode: C++
// End:

