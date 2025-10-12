/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * fsp_config.h: Raptor configure wrapper for libfsp
 *
 * Copyright (C) 2025, David Beckett https://www.dajobe.org/
 *
 * This package is Free Software and part of Redland http://librdf.org/
 *
 * It is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 *
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 *
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 *
 */


#ifndef FSP_CONFIG_H
#define FSP_CONFIG_H


#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <win32_raptor_config.h>
#else
#include <raptor_config.h>
#endif

/* Rename libfsp functions to avoid conflicts when embedded in Raptor */
#define fsp_create raptor_fsp_create
#define fsp_destroy raptor_fsp_destroy
#define fsp_buffer_append raptor_fsp_buffer_append
#define fsp_buffer_available raptor_fsp_buffer_available
#define fsp_buffer_compact raptor_fsp_buffer_compact
#define fsp_buffer_grow raptor_fsp_buffer_grow
#define fsp_parse_chunk raptor_fsp_parse_chunk
#define fsp_read_input raptor_fsp_read_input
#define fsp_set_user_data raptor_fsp_set_user_data
#define fsp_get_user_data raptor_fsp_get_user_data

#ifdef RAPTOR_DEBUG
#define FSP_DEBUG 1
#endif

#ifdef __cplusplus
}
#endif


#endif
