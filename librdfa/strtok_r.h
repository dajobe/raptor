/* This file is in the public domain */

#ifndef HAVE_STRTOK_R
#  define NEED_RDFA_STRTOK_R
#endif

#if defined(WIN32) && defined(_MSC_VER) && _MSC_VER >= 1400
#  define strtok_r(s,d,p) strtok_s(s,d,p)
#  undef NEED_RDFA_STRTOK_R
#endif

#ifdef NEED_RDFA_STRTOK_R
char *rdfa_strtok_r(char *str, const char *delim, char **saveptr);
#  define strtok_r(s,d,p) rdfa_strtok_r(s,d,p)
#endif
