/* 
 * Public Domain getopt header
 *
 * $Id$
 *
 */

#ifndef RAPTOR_GETOPT_H
#define RAPTOR_GETOPT_H

int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;

#endif
