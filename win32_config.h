/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * win32_config.h - Raptor WIN32 hard-coded config
 *
 * $Id$
 *
 * Copyright (C) 2002 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.org/
 * University of Bristol - http://www.bristol.ac.uk/
 * 
 * This package is Free Software or Open Source available under the
 * following licenses (these are alternatives):
 *   1. GNU Lesser General Public License (LGPL)
 *   2. GNU General Public License (GPL)
 *   3. Mozilla Public License (MPL)
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * full license terms.
 * 
 */


#ifndef WIN32_CONFIG_H
#define WIN32_CONFIG_H


#ifdef __cplusplus
extern "C" {
#endif

#define WIN32_LEAN_AND_MEAN 1

/* Compiling inside Raptor */
#define RAPTOR_INTERNAL 1

/* getopt is not in standard win32 C library - define if we have it */
/* #define HAVE_GETOPT_H 1 */

#define HAVE_STDLIB_H 1

/* For using expat on win32 */
#define RAPTOR_XML_EXPAT 1
#define HAVE_EXPAT_H 1

#define HAVE_STRICMP 1

/* MS names for these functions */
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#define access _access
#define stricmp _stricmp
#define strnicmp _strnicmp

#define HAVE_C99_VSNPRINTF 1

/* for access() which is POSIX but doesn't seem to have the defines in VC */
#ifndef R_OK
#define R_OK 4
#endif

/* __func__ doesn't exist in Visual Studio 6 */
#define __func__ ""

/* 
 * Defines that come from config.h
 */

/* Release version as a decimal */
#define RAPTOR_VERSION_DECIMAL 10300

/* Major version number */
#define RAPTOR_VERSION_MAJOR 1

/* Minor version number */
#define RAPTOR_VERSION_MINOR 3

/* Release version number */
#define RAPTOR_VERSION_RELEASE 0

/* Version number of package */
#define VERSION "1.3.0"

#include <windows.h>
#include <io.h>
#include <memory.h>

#ifdef __cplusplus
}
#endif

#endif
