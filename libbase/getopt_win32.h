
#ifndef GETOPT_WIN32_H
#define GETOPT_WIN32_H

#ifdef __cplusplus
extern "C"
{
#endif

extern int optind;
extern char *optarg;
extern int optopt;

int getopt (int argc, char *const argv[], char *optstring);

#ifdef __cplusplus
}
#endif

#endif /* GETOPT_WIN32_H */
