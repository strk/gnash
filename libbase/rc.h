// rc.h:  "Run Command" configuration file declarations, for Gnash.
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

#ifndef GNASH_RC_H
#define GNASH_RC_H

#include "dsodefs.h"
#include <string>
#include <vector>
#include <iosfwd>
#include <sstream>
#include <boost/cstdint.hpp>
#include <boost/tokenizer.hpp>

#include "StringPredicates.h"

#if !defined(_WIN32) && !defined(__HAIKU__) && !defined(ANDROID)
#include <sys/shm.h>
#else
#ifdef _WIN32
  typedef boost::uint32_t key_t;
#endif // _WIN32
#endif // _WIN32 and __HAIKU__

namespace gnash {
  
class DSOEXPORT RcInitFile 
{
public:

    /// Return the default instance of RC file
    static RcInitFile& getDefaultInstance();

    /// Load and parse files, looking in the usual places
    //
    void loadFiles();

    bool parseFile(const std::string& filespec);

    /// \brief    
    /// Writes a valid gnashrc file. If the file already exists,
    /// is is overwritten.
    //
    /// @param filespec the file to write
    /// @return whether the file was successfully written.
    bool updateFile(const std::string& filespec);
    
    /// \brief Writes a gnashrc file to the file specified in the
    /// GNASHRC environment variable OR to the user's home
    /// directory.
    //
    /// @return whether the file was successfully written.
    bool updateFile();
    
    bool useSplashScreen() const { return _splashScreen; }
    void useSplashScreen(bool value);

    bool useActionDump() const { return _actionDump; }
    void useActionDump(bool value);

    bool useParserDump() const { return _parserDump; }
    void useParserDump(bool value);

    bool useWriteLog() const { return _writeLog; }
    void useWriteLog(bool value);

    int getTimerDelay() const { return _delay; }
    void setTimerDelay(int x) { _delay = x; }

    bool showASCodingErrors() const { return _verboseASCodingErrors; }
    void showASCodingErrors(bool value);

    bool showMalformedSWFErrors() const { return _verboseMalformedSWF; }
    void showMalformedSWFErrors(bool value);

    bool showMalformedAMFErrors() const { return _verboseMalformedAMF; }
    void showMalformedAMFErrors(bool value);

    int getMovieLibraryLimit() const { return _movieLibraryLimit; }
    void setMovieLibraryLimit(int value) { _movieLibraryLimit = value; }

    bool enableExtensions() const { return _extensionsEnabled; }

    /// Return true if user is willing to start the gui in "stop" mode
    //
    /// defaults to false.
    bool startStopped() const { return _startStopped; }
    void startStopped(bool value) { _startStopped = value; }

    bool insecureSSL() const { return _insecureSSL; }
    void insecureSSL(bool value) { _insecureSSL = value; }
    
    int qualityLevel() const { return _quality; }
    void qualityLevel(int value) { _quality = value; }
    
    int verbosityLevel() const { return _verbosity; }
    void verbosityLevel(int value) { _verbosity = value; }
    
    void setDebugLog(const std::string &x) { _log = x; }
    const std::string& getDebugLog() const { return _log; }

    void setDocumentRoot(const std::string &x) { _wwwroot = x; }
    std::string getDocumentRoot() { return _wwwroot; }
    
    bool useDebugger() const { return _debugger; }
    void useDebugger(bool value) { _debugger = value; }

    bool useSound() const { return _sound; }
    void useSound(bool value) { _sound = value; }

    // strk: I'd drop this, and allow an -f switch to select
    //       the gnashrc file to use instead
    bool usePluginSound() const { return _pluginSound; }
    void usePluginSound(bool value) { _pluginSound = value; }

    bool popupMessages() const { return _popups; }
    void interfacePopups(bool value) { _popups = value; }

    bool useLocalDomain() const { return _localdomainOnly; }
    void useLocalDomain(bool value);

    /// Whether to restrict access to the local host   
    bool useLocalHost() const { return _localhostOnly; }

    /// Set whether to restrict access to the local host
    void useLocalHost(bool value);

    typedef std::vector<std::string> PathList;

    /// Get the current RcInitFile whitelist of domains to allow
    //
    /// @return a std::vector of strings containing allowed domains 
    const PathList& getWhiteList() const { return _whitelist; }

    /// Sets the RcInitFile whitelist of domains to allow
    //
    /// @param list a std::vector of strings containing domains without protocol
    void setWhitelist (const std::vector<std::string>& list) { _whitelist = list; }

    /// Get the current RcInitFile blacklist of domains to block
    //
    /// @return a std::vector of strings containing blocked domains    
    const PathList& getBlackList() const { return _blacklist; }

    /// Whether to forcibly show the mouse pointer even if the SWF file
    /// disables it. THis allows touchscreen based SWF files to
    /// work on a normal non-touchscreen desktop.
    bool showMouse() const { return _showMouse; }
    
    /// Sets the RcInitFile blacklist of domains to block
    //
    /// @param list a std::vector of strings containing domains without protocol
    void setBlacklist (const std::vector<std::string>& list) { 
        _blacklist = list;
    }

    /// Return the list of directories to be used as the 'local' sandbox
    //
    /// Local sendbox is the set of resources on the filesystem we want to
    /// give the current movie access to.
    ///
    const PathList& getLocalSandboxPath() const { return _localSandboxPath; }

    /// Add a directory to the local sandbox list
    void addLocalSandboxPath(const std::string& dir)
    {
        _localSandboxPath.push_back(dir);
    }

    /// Sets a list of sandbox paths. Gnash will only allow movies access
    /// to files in these paths. The path of the movie playing is automatically
    /// added.
    //
    /// @param list a std::vector of strings containing paths to allow
    void setLocalSandboxPath(const PathList& path)
    {
        _localSandboxPath = path;
    }

    const std::string& getFlashVersionString() const {
        return _flashVersionString;
    }
    
    void setFlashVersionString(const std::string& value) {
        _flashVersionString = value;
    }

    const std::string& getFlashSystemOS() const {
        return _flashSystemOS;
    }
    
    void setFlashSystemOS(const std::string& value) {
        _flashSystemOS = value;
    }

    const std::string& getFlashSystemManufacturer() const {
        return _flashSystemManufacturer;
    }
    
    void setFlashSystemManufacturer(const std::string& value) {
        _flashSystemManufacturer = value;
    }
    
    const std::string& getGstAudioSink() const { return _gstaudiosink; }
    
    void setGstAudioSink(const std::string& value) { _gstaudiosink = value; }

    int getRetries() const { return _retries; }
    
    void setRetries(int x) { _retries = x; }

    /// The number of seconds of inactivity before timing out streams downloads
    double getStreamsTimeout() const { return _streamsTimeout; }

    /// Set seconds of inactivity before timing out streams downloads
    void setStreamsTimeout(const double &x) { _streamsTimeout = x; }

    /// Get the URL opener command format
    //
    /// The %u label will need to be substituted by the actual url
    /// properly escaped.
    ///
    const std::string &getURLOpenerFormat() const
    {
        return _urlOpenerFormat;
    }
    
    void setURLOpenerFormat(const std::string& value)
    {
        _urlOpenerFormat = value;
    }
 
    // Get the name of the hardware acclerator to use for video
    const std::string &getHWAccel() const { return _hwaccel; }

    // Set the name of the hardware acclerator to use for video
    void setHWAccel(const std::string &x) { _hwaccel = x; }

    // Get the name of the renderer to draw the display
    const std::string& getRenderer() const { return _renderer; }

    // Set the name of the renderer to draw the display
    void setRenderer(const std::string& x) { _renderer = x; }

    // Get the name of the media handler to use for video/audio
    const std::string& getMediaHandler() const { return _mediahandler; }

    // Set the name of the media handler to use for video/audio
    void setMediaHandler(const std::string& x) { _mediahandler = x; }

    // Get the location of the sandbox for .sol files
    const std::string &getSOLSafeDir() const { return _solsandbox; }

    // Set the location of the sandbox for .sol files
    void setSOLSafeDir(const std::string &x) { _solsandbox = x; }

    bool getSOLLocalDomain() const { return _sollocaldomain; }
    
    void setSOLLocalDomain(bool x) { _sollocaldomain = x; }
    
    bool getSOLReadOnly() const { return _solreadonly; }
    
    void setSOLReadOnly(bool x) { _solreadonly = x; }
    
    bool getLocalConnection() const { return _lcdisabled; }
    
    void setLocalConnection(bool x) { _lcdisabled = x; }
    
    /// \brief Enable tracing all LocalConnection traffic
    bool getLCTrace() const { return _lctrace; }
    
    void setLCTrace(bool x) { _lctrace = x; }

    key_t getLCShmKey() const { return static_cast<key_t>(_lcshmkey); }
    
    void setLCShmKey(bool x) { _lcshmkey = x; }

    bool ignoreFSCommand() const { return _ignoreFSCommand; }
    
    void ignoreFSCommand(bool value) { _ignoreFSCommand = value; }
    
    void saveStreamingMedia(bool value) { _saveStreamingMedia = value; }

    bool saveStreamingMedia() const { return _saveStreamingMedia; }

    void saveLoadedMedia(bool value) { _saveLoadedMedia = value; }

    bool saveLoadedMedia() const { return _saveLoadedMedia; }

    void setMediaDir(const std::string& value) { _mediaCacheDir = value; }

    const std::string& getMediaDir() const { return _mediaCacheDir; }
	
    void setWebcamDevice(int value) {_webcamDevice = value;}
    
    int getWebcamDevice() const {return _webcamDevice;}
    
    void setAudioInputDevice(int value) {_microphoneDevice = value;}
    
    int getAudioInputDevice() {return _microphoneDevice;}

    /// \brief Get the Root SSL certificate
    const std::string& getRootCert() const {
        return _rootcert;
    }
    /// \brief Set the Root SSL certificate
    void setRootCert(const std::string& value) {
        _rootcert = value;
    }

    /// \brief Get the Client SSL certificate
    const std::string& getCertFile() const {
        return _certfile;
    }
    /// \brief Set the Client SSL certificate
    void setCertFile(const std::string& value) {
        _certfile = value;
    }

    /// \brief Get the directory for client SSL certificates
    const std::string& getCertDir() const {
        return _certdir;
    }
    /// \brief Set the directory for client SSL certificates
    void setCertDir(const std::string& value) {
        _certdir = value;
    }

    void ignoreShowMenu(bool value) { _ignoreShowMenu=value; }

    bool ignoreShowMenu() const { return _ignoreShowMenu; }

    int getScriptsTimeout() const { return _scriptsTimeout; }

    void setScriptsTimeout(int x) { _scriptsTimeout = x; }

    int getScriptsRecursionLimit() const { return _scriptsRecursionLimit; }

    void setScriptsRecursionLimit(int x) { _scriptsRecursionLimit = x; }

    void lockScriptLimits(bool x) { _lockScriptLimits = x; }

    bool lockScriptLimits() const { return _lockScriptLimits; }

    void dump();    

protected:
    
    // A function only for writing path lists to an outstream.
    void writeList(const PathList& list, std::ostream& o);

    /// Construct only by getDefaultInstance()
    RcInitFile();

    /// Never destroy (TODO: add a destroyDefaultInstance)
    ~RcInitFile();

    /// Substitutes user's home directory for ~ on a path string
    /// according to POSIX standard.
    ///
    /// @param path the path to expand.
    static void expandPath(std::string& path);

    /// \brief
    /// If variable matches pattern (case-insensitive)
    /// set var according to value
    //
    /// @return true if variable matches pattern, false otherwise
    /// @param var the variable to change
    /// @param pattern the pattern for matching
    /// @variable the variable to match to pattern
    /// @value the value to adopt if variable matches pattern.
    static bool extractSetting(bool &var, const std::string& pattern,
                        const std::string &variable, const std::string &value);

    /// \brief
    /// If variable matches pattern (case-insensitive)
    /// set num according to value
    //
    /// @return true if variable matches pattern, false otherwise
    /// @param num the variable to change
    /// @param pattern the pattern for matching
    /// @variable the variable to match to pattern
    /// @value the value to adopt if variable matches pattern.
    template<typename T>
    static bool extractNumber(T& num, const std::string& pattern,
                        const std::string &variable, const std::string &value)
    {

        StringNoCaseEqual noCaseCompare;

        if (noCaseCompare(variable, pattern)) {
            std::istringstream in(value);
            if (in >> num) return true;
            
            num = 0;
            return true;
        }
        
        return false;
    }

    /// \brief
    /// If variable matches pattern (case-insensitive)
    /// set out according to value
    //
    /// @return true if variable matches pattern, false otherwise
    /// @param out the variable to change
    /// @param pattern the pattern for matching
    /// @variable the variable to match to pattern
    /// @value the value to adopt if variable matches pattern.
    static bool extractDouble(double &out, const std::string& pattern,
                        const std::string &variable, const std::string &value);


    /// \brief parses a space-separated list into std::vector list 
    //
    /// @param list the vector to modify or generate.
    /// @param action either 'set' or 'append': whether to add to or
    ///         clear the vector.
    /// @param items string of space-separated values. This gets nuked.
    void parseList(std::vector<std::string>& list, const std::string &action,
			        const std::string &items);

    typedef boost::char_separator<char> Sep;
    typedef boost::tokenizer< Sep > Tok;

    /// The timer delay
    boost::uint32_t  _delay;

    /// Max number of movie clips to store in the library      
    boost::uint32_t  _movieLibraryLimit;   

    /// Enable debugging of this class
    bool _debug;

    /// Enable the Flash movie debugger
    bool _debugger;

    /// Level of debugging output
    boost::uint32_t  _verbosity;

    /// Command format to use to open urls
    //
    /// The %u label will need to be substituted by the url
    /// (properly escaped)
    ///
    std::string  _urlOpenerFormat;

    /// String to pass as $version in Actionscript
    std::string  _flashVersionString;
    
    /// String representing the first GStreamer audio output pipeline to try
    std::string _gstaudiosink;

    /// \brief String to pass as System.capabilities.os
    /// in Actionscript. If empty, leaves detection
    /// to System.cpp (default).
    std::string  _flashSystemOS;       

    /// \brief String to pass as
    /// System.capabilities.manufacturer
    /// in Actionscript
    std::string  _flashSystemManufacturer;

    /// Enable dumping actionscript classes
    bool _actionDump;

    /// Enable dumping parser data
    bool _parserDump;

    /// Enable ActionScript errors verbosity
    bool _verboseASCodingErrors;

    /// Enable Malformed SWF verbosity
    bool _verboseMalformedSWF;

    /// Enable Malformed AMF verbosity
    bool _verboseMalformedAMF;

    /// Display a splash screen when loading a movie
    bool _splashScreen;

    /// Only access network resources in the local domain
    bool _localdomainOnly;
    
    /// Only access network resources on the local host
    bool _localhostOnly;

    /// Show the mouse pointer
    bool _showMouse;

    /// Allowed domains
    PathList _whitelist;
    
    /// Blocked Domains 
    PathList _blacklist;
    
    /// The name of the debug log
    std::string _log;
    
    /// Enable writing the debug log to disk
    bool _writeLog;
    
    /// The root path for the streaming server        
    std::string _wwwroot;
    
    /// the number of retries for a thread 
    int _retries;
    
    /// Enable the sound handler at startup         
    bool _sound;
    
    /// Enable sound for the plugin
    bool _pluginSound;		

    /// Enable scanning plugin path for extensions
    bool _extensionsEnabled;	

    /// Start the gui in "stop" mode
    bool _startStopped;		

    /// Allow SSL connections without verifying the certificate
    bool _insecureSSL;		

    /// The number of seconds of inactivity triggering download timeout
    double _streamsTimeout;

    /// \brief Local sandbox: the set of resources on the
    /// filesystem we want to give the current movie access to.
    PathList _localSandboxPath;

    /// \brief SOL Sandbox: This is the only dir .sol (Shared Object)
    /// files can be written in, or read from.
    std::string _solsandbox;

    /// Whether SOL files can be written
    bool _solreadonly;
    bool _sollocaldomain;
    
    // Disable local connection
    bool _lcdisabled;
    
    /// Trace local connection activity (in log)
    bool _lctrace;
    
    /// Shared memory segment key (can be set for
    /// compatibility with other players.)
    boost::uint32_t _lcshmkey;
    
    /// Whether the player should respond to fscommands
    /// (showmenu, quit etc)
    bool _ignoreFSCommand;

    /// The quality to display SWFs in. -1 to allow the SWF to override.
    int _quality;

    bool _saveStreamingMedia;
    
    bool _saveLoadedMedia;

    std::string _mediaCacheDir;

    bool _popups;

    bool _useXv;
	
    ///FIXME: this should probably eventually be changed to a more readable
    ///config option instead of an integer
    int _webcamDevice;
    
    int _microphoneDevice;

    /// \var _certfile
    ///		This is the name of the client certificate file
    std::string _certfile;

    /// \var _certdir
    ///		This is the path to the directory containing cert files
    std::string _certdir;

    /// \var _rootcert
    ///		This is the name of the root certificate
    std::string _rootcert;

    /// Whether the player will recognize changes to Stage.showMenu in AS
    /// default value is true
    bool _ignoreShowMenu;

    /// Whether to use HW video decoding support, no value means disabled.
    /// The only currently supported values are: none, vaapi, or xv.  omap
    /// support is coming. 
    std::string _hwaccel;

    /// Which renderer backend to use, no value means use the default.
    /// The currently supported values are agg, opengl, or cairo. AGG
    /// being the default.
    std::string _renderer;

    /// Which media player backend to use, no value means use the default.
    /// The default is set in the MediaFactory initialization table.
    std::string _mediahandler;

    /// The number of seconds after which action execution is
    /// considered to be slow enough to prompt the user for aborting
    int _scriptsTimeout;

    /// The max actionscript function call stack depth
    int _scriptsRecursionLimit;

    /// Whether to ignore SWF ScriptLimits tags 
    bool _lockScriptLimits;
};

// End of gnash namespace 
}

#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
