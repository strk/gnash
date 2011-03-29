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

#ifdef USE_STATS_MEMORY

/* Enable statistics tracking in jemalloc. */
#define MALLOC_STATS

/* Borrowed from malloc.h, as this is Linux specific. This has been
 * added to jemalloc so the existing memory profiling in Gnash will
 * continue to work. Most of these fields aren't used by the Gnash
 * memory profiling, but we leave them here for a semblance of
 * portability. The only fields Gnash uses are arena, uordblks. and
 * fordblks.
 */
struct mallinfo {
  int arena;    /* non-mmapped space allocated from system */
  int ordblks;  /* number of free chunks UNUSED */
  int smblks;   /* number of fastbin blocks UNUSED */
  int hblks;    /* number of mmapped regions UNUSED */
  int hblkhd;   /* space in mmapped regions UNUSED */
  int usmblks;  /* maximum total allocated space UNUSED */
  int fsmblks;  /* space available in freed fastbin blocks UNUSED */
  int uordblks; /* total allocated space */
  int fordblks; /* total free space */
  int keepcost; /* top-most, releasable space UNUSED */
};

static
struct mallinfo
mallinfo(void)
{
    struct mallinfo mi;
    jemalloc_stats_t stats;
    
    jemalloc_stats(&stats);

    /* clear unused fields */
    mi.arena = mi.ordblks = mi.smblks = mi.usmblks = mi.fsmblks =
        mi.hblks = mi.hblkhd = 0;
  
    mi.uordblks = stats.allocated;
    mi.fordblks = stats.mapped - mi.uordblks;
    mi.keepcost = stats.mapped;
    
    return mi;
}

#endif /* USE_STATS_MEMORY */

#endif /* GNASH_JEMALLOC_H */
