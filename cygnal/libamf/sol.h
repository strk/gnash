// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef GNASH_SOL_H
#define GNASH_SOL_H

#include <boost/cstdint.hpp>
#include <string>
#include <vector>

#include "dsodefs.h" //DSOEXPORT
#include "amf.h"

// It comprises of a magic number, followed by the file length, a
// filetype, which appears to always be "TCSO", and what appears to be
// a marker at the end of the header block.
// After the SOL header, the rest is all AMF objects.

// Magic Number - 2 bytes (always 0x00bf)
// Length       - 4 bytes (the length of the file including the Marker bytes)
// Marker       - 10 bytes (always "TCSO0x000400000000")
// Object Name  - variable (the name of the object as an AMF encoded string)
// Padding      - 4 bytes
// After this is a series of AMF objects

/// \namespace cygnal
///
/// This namespace is for all the AMF specific classes in libamf.
namespace cygnal
{

// Forward declarations
class Element;

/// \class SOL
///	This class is for accessing the data in SharedObject files,
///	also called "Flash cookies". These .sol files are just a
///	collection of AMF0 data, with a simple file header.
class DSOEXPORT SOL {
public:
    SOL();
    ~SOL();

    /// \brief Get the number of Elements in this class.
    ///
    /// @return The count of Elements.
    size_t size() const { return _amfobjs.size(); }
 
    size_t fileSize() const { return _filesize; }

    /// \brief Extract the header from the file.
    ///
    /// @param data a reference to a vector of bytes that contains the
    ///		.sol file data.
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool extractHeader(const std::vector<boost::uint8_t> &data);

    /// \brief Extract the header from the file.
    ///
    /// @param filespec The name and path of the .sol file to parse.
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool extractHeader(const std::string &filespec);

    /// \brief Create the file header.
    ///
    /// @param data a reference to a vector of bytes that contains the
    ///		.sol file data.
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool formatHeader(const std::vector<boost::uint8_t> &data);

    /// \brief Create the file header.
    ///
    /// @param name The name of the SharedObject for this file.
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool formatHeader(const std::string &name);

    /// \brief Create the file header.
    ///
    /// @param name The name of the SharedObject for this file.
    ///
    /// @param filesize The size of the file.
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool formatHeader(const std::string &name, int filesize);

    /// \brief Write the data to disk as a .sol file
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool writeFile();

    /// \brief Write the data to disk as a .sol file
    ///
    /// @param filespec The name and path of the .sol file to parse.
    ///
    /// @param objname The name of the SharedObject for this file.
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool writeFile(const std::string &filespec, const std::string &objname);
    
    /// \brief Read a .sol file from disk
    ///
    /// @param filespec The name and path of the .sol file to parse.
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool readFile(const std::string &filespec);

    /// \brief Get the stored copy of the header
    ///
    /// @return A vector of raw bytes that is a binary form of the header.
    std::vector<boost::uint8_t> getHeader() { return _header; };

    /// \brief Add the AMF objects that are the data of the file
    //
    /// @param el A smart pointer to the Element to add to the .sol file.
    ///
    /// @return nothing.
    void addObj(boost::shared_ptr<Element> el);

    /// \brief Return a reference to the elements in this object
    ///
    /// @return A smart pointer to the array of properities for this
    ///		.sol file.
    std::vector<boost::shared_ptr<cygnal::Element> > &getElements() { return _amfobjs; }

    /// \brief Get an element referenced by index in the array
    ///
    /// @param size The index of the property to retrieve.
    ///
    /// @return A smart pointer to the element at the specified location.
    boost::shared_ptr<Element> getElement(size_t size)
    {
        assert(size<_amfobjs.size());
        return _amfobjs[size];
    }

    /// \brief Set the filespec for the .sol file.
    ///		Set's the full path and file name to the .sol file to
    ///		be read or written.
    ///
    /// @param filespec The name and path of the .sol file to parse.
    ///
    /// @return nothing.
    void setFilespec(const std::string &filespec) { _filespec = filespec; };

    /// \brief Get the filespec of the .sol file.
    ///
    /// @return A string which contains the full path and name of the
    ///		.sol file.
    const std::string &getFilespec() const { return _filespec; };

    /// \brief Set the name of the SharedObject.
    ///
    /// @param objname The name of the SharedObject in the .sol file.
    ///
    /// @return nothing.
    void setObjectName(const std::string &objname) { _objname = objname; };

    /// \brief Get the filespec of the .sol file.
    ///
    /// @return A string which contains the name of the SharedObject
    ///		in the .sol file.
    const std::string &getObjectName() const { return _objname; };
        
    bool updateSO(boost::shared_ptr<cygnal::Element> &el);
    bool updateSO(int index, boost::shared_ptr<cygnal::Element> &el);
    
    ///  \brief Dump the internal data of this class in a human readable form.
    ///
    /// @remarks This should only be used for debugging purposes.
    void dump();

 private:
    /// \var SOL::_header
    ///		A stored copy of the SOL file header.
    std::vector<boost::uint8_t> _header;

    /// \var SOL::_data
    ///		The vector that contains the raw dats for this .sol file.
    std::vector<boost::uint8_t> _data;

    /// \var SOL::_objname
    ///		The name of the SharedObject in the .sol file.
    std::string      _objname;

    /// \var SOL::_filespec
    ///		The full path and name of the .sol file.
    std::string      _filespec;

    /// \var SOL::_filesize
    ///		The size of the .sol file.
    int              _filesize;

 protected:
    /// \var SOL::_amfobjs
    ///		The array of elements in this SharedObject.
    std::vector<boost::shared_ptr<Element> > _amfobjs;
    
  };

 
} // end of amf namespace

// end of _SOL_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
