


#ifndef GNASH_NAMINGPOLICY_H
#define GNASH_NAMINGPOLICY_H

#include "URL.h"

#include <string>

namespace gnash {


struct NamingPolicy
{
    NamingPolicy() {}
    virtual ~NamingPolicy() {}
    virtual std::string operator()(const URL&) const
    {
        return std::string();
    }
};


/// Make a non-unique cachefile name from the supplied name.
/// If the directory cannot be created, return an empty string.
struct OverwriteExisting : public NamingPolicy
{
    virtual std::string operator()(const URL&) const;
};

/// Make a unique cachefile name from the supplied name.
/// If all possible filenames are taken, return an empty string.
struct IncrementalRename : public NamingPolicy
{
    IncrementalRename(const URL& baseURL);
    virtual std::string operator()(const URL& url) const;
private:
    const URL _baseURL;
};

} // namespace gnash

#endif
