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

/* Ensure we link with raptor version */
#define getopt raptor_getopt
#define optind raptor_optind
#define optarg raptor_optarg
#define opterr raptor_opterr
#define optopt raptor_optopt

#endif
