/* $Id: getopt_win32.h,v 1.2 2006/09/25 11:35:48 nihilus Exp $ */

#ifndef GETOPT_WIN32_H
#define GETOPT_WIN32_H

extern int optind;
extern char *optarg;
extern int optopt;

int
getopt(int argc, char * const argv[], char *optstring);

extern "C" {

#include <string.h>

int optind = 1;
char *optarg;
int optopt;

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

}

#endif
