/* 
 * Public Domain getopt header
 *
 * $Id$
 *
 */

#ifndef RAPTOR_GETOPT_H
#define RAPTOR_GETOPT_H

int raptor_getopt(int argc, char * const argv[], const char *optstring);
extern char *raptor_optarg;
extern int raptor_optind, raptor_opterr, raptor_optopt;

#define getopt raptor_getopt

#endif
