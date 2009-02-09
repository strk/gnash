
#include "NamingPolicy.h"
#include "gnashconfig.h"
#include "URL.h"
#include "GnashFileUtilities.h"
#include "rc.h"
#include "log.h"

#include <sstream>
#include <string>
#include <limits>
#include <boost/algorithm/string/replace.hpp>

namespace gnash {

    
namespace {
    std::string urlToDirectory(const std::string& path);
}


std::string
OverwriteExisting::operator()(const URL& url) const
{
    std::string path = url.path().substr(1);
    
    // Replace all slashes with a _ for a flat directory structure.
    boost::replace_all(path, "/", "_");

    const std::string& dir = urlToDirectory(url.hostname() + "/");

    if (dir.empty()) return std::string();

    return dir + path;
}


IncrementalRename::IncrementalRename(const URL& baseURL)
    :
    _baseURL(baseURL)
{
}


std::string
IncrementalRename::operator()(const URL& url) const
{

    const std::string& path = url.path();
    assert(!path.empty());
    assert(path[0] == '/');
    
    // Find the last dot, but not if it's first in the path (after the
    // initial '/').
    std::string::size_type dot = path.rfind('.');
    if (dot == 1) dot = std::string::npos;

    // Take the path from after the initial '/' to the last '.' for
    // manipulation. It doesn't matter if dot is npos.
    std::string pre = path.substr(1, dot - 1);

    // Replace all slashes with a _ for a flat directory structure.
    boost::replace_all(pre, "/", "_");

    const std::string& suffix = (dot == std::string::npos) ? "" : 
        path.substr(dot);

    // Add a trailing slash.
    const std::string& hostname = _baseURL.hostname().empty() ? "localhost" :
        _baseURL.hostname();

    const std::string& dir = urlToDirectory(hostname + "/");
    if (dir.empty()) return std::string();

    std::ostringstream s(dir + pre + suffix);

    size_t i = 0;

    const size_t m = std::numeric_limits<size_t>::max();

    struct stat st;
    while (stat(s.str().c_str(), &st) >= 0 && i < m) {
        s.str("");
        s << dir << pre << i << suffix;
        ++i;
    }

    // If there are no options left, return an empty string.
    if (i == m) {
        return std::string();
    }

    return s.str();

}

namespace {

/// Transform a URL into a directory and create it.
//
/// @return     an empty string if the directory cannot be created, otherwise
///             the name of the created directory with a trailing slash.
/// @param url  The path to transform. Anything after the last '/' is ignored.
std::string
urlToDirectory(const std::string& path)
{

    const RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    const std::string& dir = rcfile.getMediaDir() + "/" + path;
 
    // Create the user-specified directory if possible.
    // An alternative would be to use the 'host' part and create a 
    // directory tree.
    if (!mkdirRecursive(dir)) {
        return std::string();
    }

    return dir;

}

} // anonymous namespace
} // namespace gnash
