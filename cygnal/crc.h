// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef __CRC_H__
#define __CRC_H__

#include <string>
#include <iostream> // for output operator

#include "rc.h"

/// \namespace cygnal
///
/// This namespace is for all the Cygnal specific classes not used by
/// anything else in Gnash.
namespace cygnal {
    
/// \class cygnal::CRcInitFile
///	This class handles reading values from the Cygnal
///	configuration file, .cygnalrc, and into a form we can use in
///	Cygnal.
class DSOEXPORT CRcInitFile : public gnash::RcInitFile
{
public:
    /// Construct only by getDefaultInstance()
    CRcInitFile();
    /// Never destroy (TODO: add a destroyDefaultInstance)
    ~CRcInitFile();
    
    /// \brief Return the default instance of RC file,
    static CRcInitFile& getDefaultInstance();
    
    /// \brief Load all the configuration files.
    ///		This includes parsing the default .gnashrc file for
    ///		Gnash settings that control the swf parser and virtual
    ///		machine. These setting can be overridden in the
    ///		.cygnalrc file, plus the Cygnal specific file has
    ///		options only used by Cygnal.
    bool loadFiles();
    
    /// \brief Parse and load configuration file
    ///
    /// @param filespec The path and file name of the disk file to parse.
    ///
    /// @return True if the file was parsed successfully, false if not.
    bool parseFile(const std::string& filespec);

    /// Accessors

    /// \brief Get the port offset.
    int getPortOffset() { return _port_offset; };

    // \brief Set the port offset
    void setPortOffset(int x) { _port_offset = x; };

    /// \brief Get the number of file descriptors per thread.
    int getFDThread() { return _fdthread; };
    /// \brief Set the number of file descriptors per thread.
    void setFDThread(int x) { _fdthread = x; };

    /// \brief Get the special testing output option.
    bool getTestingFlag() { return _testing; };
    /// \brief Set the special testing output option.
    void setTestingFlag(bool x) { _testing = x; };

    /// \brief Get the flag for whether to enable threading.
    bool getThreadingFlag() { return _threading; };
    /// \brief Set the flag for whether to enable threading.
    void setThreadingFlag(bool x) { _threading = x; };

    /// \brief Get the flag for whether to enable internal debugging messages.
    bool getNetDebugFlag() const { return _netdebug; }
    /// \brief Set the flag for whether to enable internal debugging messages.
    void setNetDebugFlag(bool x) { _netdebug = x; }    

    /// \brief Get the flag for whether to enable the administration thread.
    bool getAdminFlag() const { return _admin; }
    /// \brief Set the  flag for whether to enable the administration thread.
    void setAdminFlag(bool x) { _admin = x; }

    void setDocumentRoot(const std::string &x) { _wwwroot = x; }
    std::string getDocumentRoot() { return _wwwroot; }
    
    void setCgiRoot(const std::string &x) { _cgiroot = x; }
    std::string getCgiRoot() { return _cgiroot; }
    
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

    /// \brief Dump the internal data of this class in a human readable form.
    /// @remarks This should only be used for debugging purposes.
    void dump() const { dump(std::cerr); }
    
    /// \overload dump(std::ostream& os) const
    void dump(std::ostream& os) const;
    
  private:
    /// \var _wwwroot
    ///		The root path for the streaming server to find al files.
    std::string _wwwroot;
    
    /// \var _cgiroot;
    ///		This specifies the default directory for all cgi (exeutables).
    std::string _cgiroot;

    /// \var _port_offset
    ///		This is an offset applied to all priviledged tcp/ip
    ///		ports. This enables the port number to be shifted into
    ///		the unpriviledged range (anything about 1024) so one
    ///		doesn't have to be root.
    int _port_offset;

    /// \var _testing
    ///		Turn on special output format to support Gnash
    ///		testing.
    bool _testing;

    /// \var _threading
    ///		Disable threading in the server I/O as much as
    ///		possible to make debugging easier. This is to only be
    ///		used by developers
    bool _threading;

    /// \var _fdthread
    ///		The number of file descriptors to be watched by each
    ///		dispatch thread. When threading is disabled, this is
    ///		also disabled, as all the file descriptors are watched
    ///		by one one thread as an aid to debugging.
    size_t _fdthread;
    
    /// \var _netdebug
    ///	Toggles very verbose debugging info from the network Network
    ///	class.
    bool _netdebug;

    /// \var _admin
    ///		This toggles whether the admin thread is started or
    ///		not, also to reduce complecity when debugging.
    bool _admin;

    /// \var _certfile
    ///		This is the name of the server certificate file
    std::string _certfile;

    /// \var _certdir
    ///		This is the path to the directory containing cert files
    std::string _certdir;

};

/// \brief Dump to the specified output stream.
inline std::ostream& operator << (std::ostream& os, const CRcInitFile& crcini)
{
	crcini.dump(os);
	return os;
}

// End of gnash namespace 
}

// __CRC_H__
#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
