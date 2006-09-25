// WIN32 includes

/* $Id: getopt_win32.h,v 1.1 2006/09/25 10:52:34 nihilus Exp $ */

#ifndef GETOPT_WIN32_H
#define GETOPT_WIN32_H

extern int optind;
extern char *optarg;
extern int optopt;

int
getopt(int argc, char * const argv[], char *optstring);

#endif
