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

#define WIN32_LEAD_AND_MEAN 1
#define HAVE_STDLIB_H 1

/* use expat on win32 */
#define RAPTOR_XML_EXPAT 1
#define HAVE_XMLPARSE_H 1

#define HAVE_STRCMPI

#include <windows.h>

#ifdef __cplusplus
}
#endif
