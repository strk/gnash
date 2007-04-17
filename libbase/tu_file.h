// tu_file.h	-- Ignacio Castaño, Thatcher Ulrich 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A very generic file class that can be customized with callbacks.


#ifndef TU_FILE_H
#define TU_FILE_H


#include "tu_config.h"
#include <cstdio>
#include "tu_types.h"
#include "tu_swap.h"
#include "utility.h"

class membuf;
struct SDL_RWops;

enum
{
    TU_FILE_NO_ERROR = 0,
    TU_FILE_OPEN_ERROR,
    TU_FILE_READ_ERROR,
    TU_FILE_WRITE_ERROR,
    TU_FILE_SEEK_ERROR,
    TU_FILE_CLOSE_ERROR
};


// a file abstraction that can be customized with callbacks.
// Designed to be easy to hook up to FILE*, SDL_RWops*, or
// whatever stream type(s) you might use in your game or
// libraries.
class DSOEXPORT tu_file
{
public:
    typedef int (* read_func)(void* dst, int bytes, void* appdata);
    typedef int (* write_func)(const void* src, int bytes, void* appdata);
    typedef int (* seek_func)(int pos, void* appdata);
    typedef int (* seek_to_end_func)(void* appdata);
    typedef int (* tell_func)(void* appdata);
    typedef bool (* get_eof_func)(void* appdata);
    typedef int (* get_err_func)(void* appdata);
    typedef long (* get_stream_size_func)(void* appdata);
    typedef int (* close_func)(void* appdata);
    
    // The generic constructor; supply functions for the implementation.
    tu_file(
	void * appdata,
	read_func rf,
	write_func wf,
	seek_func sf,
	seek_to_end_func ef,
	tell_func tf,
	get_eof_func gef,
	get_err_func ger,
	get_stream_size_func gss,
	close_func cf=NULL);
    
    // Make a file from an ordinary FILE*.
    tu_file(FILE* fp, bool autoclose);
    
    // Optional: if you're using SDL, this is a constructor to create
    // a tu_file from an SDL_RWops* stream.
    tu_file(SDL_RWops* sdl_stream, bool autoclose);
    
    // Open a file using ordinary fopen().  Automatically closes the
    // file when we are destroyed.
    tu_file(const char* name, const char* mode);
    
    // Make a memory-buffer file for read/write.
    enum memory_buffer_enum { memory_buffer };
    tu_file(memory_buffer_enum m);
    
    // A read-only memory-buffer with predefined data.
    tu_file(memory_buffer_enum m, int size, void* data);
    
    ~tu_file();
    
    /// Copy remaining contents of *in into *this.
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void copy_from(tu_file* in);

    /// Copy remaining contents of *this into *out.
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void copy_to(membuf* out);
    
    /// Copy a fixed number of bytes from *in to *this.
    //
    /// Returns number of bytes copied.
    ///
    /// TODO: define what happens when either one of the streams
    ///       is in error condition, see get_error().
    ///
    int	copy_bytes(tu_file* in, int bytes);
    

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    uint64_t	read_le64();

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    uint32_t 	read_le32();

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    uint16_t 	read_le16();

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    uint64_t 	read_be64();

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    uint32_t 	read_be32();

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    uint16_t 	read_be16();

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void 	write_le64(uint64_t u);

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void 	write_le32(uint32_t u);

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void 	write_le16(uint16_t u);

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void	write_be64(uint64_t u);

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void 	write_be32(uint32_t u);

    /// \brief
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void 	write_be16(uint16_t u);
    
    /// Read a single byte from the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    uint8_t 	read_byte() { return read8(); }

    /// write a single byte to the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void	write_byte(uint8_t u) { write8(u); }
    
    /// Read the given number of bytes from the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    int 	read_bytes(void* dst, int num) { return m_read(dst, num, m_data); }

    /// Write the given number of bytes to the stream
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    int 	write_bytes(const void* src, int num) { return m_write(src, num, m_data); }
    
    /// Write a 0-terminated string.
    //
    /// TODO: define what happens when the stream
    ///       is in error condition, see get_error().
    ///
    void write_string(const char* src);
    
    /// \brief
    /// Read up to max_length characters, returns the number of characters 
    /// read, or -1 if the string length is longer than max_length.
    //
    /// Stops at the first \0 character if it comes before max_length.
    ///
    /// Guarantees termination of the string.
    ///
    /// @return ??
    ///
    /// TODO: define what to return when the stream
    ///       is in error condition, see get_error().
    ///
    int	read_string(char* dst, int max_length);
    
    /// \brief
    /// TODO: define what to return when the stream
    ///       is in error condition, see get_error().
    void	write_float32(float value);

    /// \brief
    /// TODO: define what to return when the stream
    ///       is in error condition, see get_error().
    void	write_double64(double value);

    /// \brief
    /// TODO: define what to return when the stream
    ///       is in error condition, see get_error().
    float	read_float32();

    /// \brief
    /// TODO: define what to return when the stream
    ///       is in error condition, see get_error().
    double	read_double64();
    
    /// Return current stream position
    //
    /// TODO: define what to return when the stream
    ///       is in error condition, see get_error().
    ///
    int	get_position() const { return m_tell(m_data); }

    /// Seek to the specified position
    //
    /// 
    /// TODO: define what happens when an error occurs, or
    ///       when we're already in an error condition
    ///
    void 	set_position(int p) { m_seek(p, m_data); }

    /// Seek to the end of the stream
    //
    /// TODO: define what happens when an error occurs
    ///
    void	go_to_end() { m_seek_to_end(m_data); }

    /// Return true if the end of the stream has been reached.
    //
    /// TODO: define what to return when in error condition
    /// see get_error().
    ///
    bool	get_eof() { return m_get_eof(m_data); }
    
    /// Return non-zero if the stream is in an error state
    //
    /// When the stream is in an error state there's nothing
    /// you can do about it, just delete it and log the error.
    ///
    /// There are some rough meaning for possible returned values
    /// but I don't think they make much sense currently.
    ///
    int	get_error() { return m_get_err(m_data); }
    

	// get the size of the stream
	int get_size() { return m_get_stream_size(m_data); }

    // printf-style convenience function.
    int	printf(const char* fmt, ...);
    
    // UNSAFE back door, for testing only.
    void*	get_app_data_DEBUG() { return m_data; }
    
    
private:
    uint64_t	read64() {
	uint64_t u;
	m_read(&u, 8, m_data);
	return u;
    }
    uint32_t	read32() {
	uint32_t u;
	m_read(&u, 4, m_data);
	return u;
    }
    uint16_t	read16() {
	uint16_t u;
	m_read(&u, 2, m_data);
	return u;
    }
    uint8_t	read8() {
	uint8_t u;
	m_read(&u, 1, m_data);
	return u;
    }
    
    void	write64(uint64_t u) {
	m_write(&u, 8, m_data);
    }
    void	write32(uint32_t u) {
	m_write(&u, 4, m_data);
    }
    void	write16(uint16_t u) {
	m_write(&u, 2, m_data);
    }
    void	write8(uint8_t u) {
	m_write(&u, 1, m_data);
    }
    
    void	close();
    
    void *		m_data;
    read_func		m_read;
    write_func		m_write;
    seek_func		m_seek;
    seek_to_end_func 	m_seek_to_end;
    tell_func		m_tell;
    get_eof_func	m_get_eof;
    get_err_func	m_get_err;
    get_stream_size_func	m_get_stream_size;
    close_func		m_close;
};


//
// Some inline stuff.
//


#if _TU_LITTLE_ENDIAN_
inline uint64_t	tu_file::read_le64() { return read64(); }
inline uint32_t	tu_file::read_le32() { return read32(); }
inline uint16_t	tu_file::read_le16() { return read16(); }
inline uint64_t	tu_file::read_be64() { return swap64(read64()); }
inline uint32_t	tu_file::read_be32() { return swap32(read32()); }
inline uint16_t	tu_file::read_be16() { return swap16(read16()); }
inline void	tu_file::write_le64(uint64_t u) { write64(u); }
inline void	tu_file::write_le32(uint32_t u) { write32(u); }
inline void	tu_file::write_le16(uint16_t u) { write16(u); }
inline void	tu_file::write_be64(uint64_t u) { write64(swap64(u)); }
inline void	tu_file::write_be32(uint32_t u) { write32(swap32(u)); }
inline void	tu_file::write_be16(uint16_t u) { write16(swap16(u)); }
#else // not _TU_LITTLE_ENDIAN_
inline uint64_t	tu_file::read_le64() { return swap64(read64()); }
inline uint32_t	tu_file::read_le32() { return swap32(read32()); }
inline uint16_t	tu_file::read_le16() { return swap16(read16()); }
inline uint64_t	tu_file::read_be64() { return read64(); }
inline uint32_t	tu_file::read_be32() { return read32(); }
inline uint16_t	tu_file::read_be16() { return read16(); }
inline void	tu_file::write_le64(uint64_t u) { write64(swap64(u)); }
inline void	tu_file::write_le32(uint32_t u) { write32(swap32(u)); }
inline void	tu_file::write_le16(uint16_t u) { write16(swap16(u)); }
inline void	tu_file::write_be64(uint64_t u) { write64(u); }
inline void	tu_file::write_be32(uint32_t u) { write32(u); }
inline void	tu_file::write_be16(uint16_t u) { write16(u); }
#endif	// not _TU_LITTLE_ENDIAN_


inline void	tu_file::write_float32(float value)
// Write a 32-bit little-endian float to this file.
{
    union alias {
	float	f;
	uint32_t	i;
    } u;
    compiler_assert(sizeof(alias) == sizeof(uint32_t));
    
    u.f = value;
    write_le32(u.i);
}


inline float	tu_file::read_float32()
// Read a 32-bit little-endian float from this file.
{
    union {
	float	f;
	uint32_t	i;
    } u;
    compiler_assert(sizeof(u) == sizeof(u.i));
    
    u.i = read_le32();
    return u.f;
}

#if 0
inline void		tu_file::write_double64(double value)
// Write a 64-bit little-endian double to this file.
{
    union {
	double	d;
	uint64_t	l;
    } u;
    compiler_assert(sizeof(u) == sizeof(u.l));
    
    u.d = value;
    write_le64(u.l);
}


inline double	tu_file::read_double64()
// Read a little-endian 64-bit double from this file.
{
    union {
	double	d;
	uint64_t	l;
    } u;
    compiler_assert(sizeof(u) == sizeof(u.l));
    
    u.l = read_le64();
    return u.d;
}
#endif

#endif // TU_FILE_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
