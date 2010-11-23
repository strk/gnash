
#ifndef GNASH_SWF_CTYPE_H
#define GNASH_SWF_CTYPE_H


#include <locale>

namespace gnash {

class SWFCtype : public std::ctype<wchar_t>
{
public:

    SWFCtype(size_t refs = 0) : std::ctype<wchar_t>(refs) {}
    typedef std::ctype<wchar_t>::char_type char_type;

protected:
    virtual char_type do_toupper(char_type) const;
    virtual const char_type* do_toupper(char_type* low, const char_type* high) const;
    virtual char_type do_tolower(char_type) const;
    virtual const char_type* do_tolower(char_type* low, const char_type* high) const;
};

} // namespace gnash

#endif
