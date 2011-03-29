#ifndef GNASH_JEMALLOC_H
#define GNASH_JEMALLOC_H

#ifdef HAVE_CONFIG_H
# include "gnashconfig.h"
#endif

/* Compiling for user mode application, not operating system */
#define MOZ_MEMORY

#ifdef LINUX_HOST
#define MOZ_MEMORY_LINUX
#endif

#ifdef WIN32_HOST
#define MOZ_MEMORY_WINDOWS
#endif

/* OpenBSD and others are excluded here to mimic what Mozilla does. */
#if defined(FREEBSD_HOST) || defined(NETBSD_HOST)
#define MOZ_MEMORY_BSD
#endif

#ifdef SOLARIS_HOST
#define MOZ_MEMORY_SOLARIS
#endif

#ifdef WINCE_HOST
#define MOZ_MEMORY_WINCE
#endif

#ifdef WINCE6_HOST
#define MOZ_MEMORY_WINCE6
#endif

#ifdef ANDROID_HOST
#define MOZ_MEMORY_ANDROID
#endif


#include "jemalloc.h"

#endif /* GNASH_JEMALLOC_H */
