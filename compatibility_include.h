// Dummy header; should get included first into tu-testbed headers.
// This is for manual project-specific configuration.

//
// Some optional general configuration.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// Choose STL containers, or tu-testbed's simplified work-alikes.
// #define _TU_USE_STL 1

// Memory allocation functions.
// #define tu_malloc(size) ...
// #define tu_realloc(old_ptr, new_size, old_size) ...
// #define tu_free(old_ptr, old_size) ...

// @@ TODO operator new stub

// Fatal error handler.
// #define tu_error_exit(error_code, error_message) ...


//
// Some optional gameswf configuration.
//

// For enabling XML/XMLSocket functionality in gameswf, using GNOME
// libxml2
// #define HAVE_LIBXML 1

// #define TU_CONFIG_LINK_TO_LIBPNG 0
