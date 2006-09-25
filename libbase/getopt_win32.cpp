/* $Id: getopt_win32.cpp,v 1.2 2006/09/25 13:17:50 nihilus Exp $ */

#ifdef __cplusplus
extern "C"
{
#endif

  char *optarg;			// global argument pointer
  int optind = 0;		// global argv index

  int getopt (int argc, char *argv[], char *optstring)
  {
    char c;
    char *cp;
    static char *next = NULL;
    if (optind == 0)
      next = NULL;

      optarg = NULL;

    if (next == NULL || *next == '\0')
      {
	if (optind == 0)
	  optind++;

	if (optind >= argc || argv[optind][0] != '-'
	    || argv[optind][1] == '\0')
	  {
	    optarg = NULL;
	    if (optind < argc)
	      optarg = argv[optind];
	    return EOF;
	  }

	if (strcmp (argv[optind], "--") == 0)
	  {
	    optind++;
	    optarg = NULL;
	    if (optind < argc)
	      optarg = argv[optind];
	    return EOF;
	  }

	next = argv[optind] + 1;
	optind++;
      }

    c = *next++;
    cp = strchr (optstring, c);

    if (cp == NULL || c == ':')
      return '?';

    cp++;
    if (*cp == ':')
      {
	if (*next != '\0')
	  {
	    optarg = next;
	    next = NULL;
	  }
	else if (optind < argc)
	  {
	    optarg = argv[optind];
	    optind++;
	  }
	else
	  {
	    return '?';
	  }
      }

    return c;
  }

#ifdef __cplusplus
}
#endif
