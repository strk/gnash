/* WIN32 includes.  We don't want to have to include windows.h because
 * it's such a pig, so #define a couple things that are required to
 * make the gl.h stuff work.
 */

#ifndef _INC_WINDOWS

# define WINAPI	__stdcall
# define APIENTRY WINAPI
# define CALLBACK __stdcall
# define DECLSPEC_IMPORT __declspec(dllimport)

# if !defined(_GDI32_)
#   define WINGDIAPI DECLSPEC_IMPORT
# else
#   define WINGDIAPI
# endif

#else
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#ifndef _WCHAR_T_DEFINED
     typedef unsigned short wchar_t;
#    define _WCHAR_T_DEFINED
#endif

#include <string.h>

static int optind = 1;
static char *optarg;
static int optopt;

int
getopt(int argc, char * const argv[], char *optstring)
{
  static int sp = 1;
  register int c;
  register char *cp;
  
  if (sp == 1) {
    if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0') {
      return (-1);
    } else
      if (strcmp(argv[optind], "--") == 0) {
        optind++;
        return (-1);
      }
  }
  optopt = c = argv[optind][sp];
  if (c == ':' || (cp = strchr(optstring, c)) == NULL) {
    if (argv[optind][++sp] == '\0') {
      optind++;
      sp = 1;
    }
    return ('?');
  }
  if (*++cp == ':') {
    if (argv[optind][sp + 1] != '\0') {
      optarg = &argv[optind++][sp + 1];
    } else
      if (++optind >= argc) {
        sp = 1;
        return ('?');
      } else {
        optarg = argv[optind++];
        sp = 1;
      }
  } else {
    if (argv[optind][++sp] == '\0') {
      sp = 1;
      optind++;
    }
    optarg = NULL;
  }
  return (c);
}

