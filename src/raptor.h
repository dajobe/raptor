/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rapier.h - Redland Parser for RDF (Rapier) interfaces and definition
 *
 * $Id$
 *
 * Copyright (C) 2000 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology, University of Bristol.
 *
 *    This package is Free Software available under either of two licenses
 *    (see FAQS.html to see why):
 * 
 * 1. The GNU Lesser General Public License (LGPL)
 * 
 *    See http://www.gnu.org/copyleft/lesser.html or COPYING.LIB for the
 *    full license text.
 *      _________________________________________________________________
 * 
 *      Copyright (C) 2000 David Beckett, Institute for Learning and
 *      Research Technology, University of Bristol. All Rights Reserved.
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public License
 *      as published by the Free Software Foundation; either version 2 of
 *      the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library; if not, write to the Free Software
 *      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *      USA
 *      _________________________________________________________________
 * 
 *    NOTE - under Term 3 of the LGPL, you may choose to license the entire
 *    library under the GPL. See COPYING for the full license text.
 * 
 * 2. The Mozilla Public License
 * 
 *    See http://www.mozilla.org/MPL/MPL-1.1.html or MPL.html for the full
 *    license text.
 * 
 *    Under MPL section 13. I declare that all of the Covered Code is
 *    Multiple Licensed:
 *      _________________________________________________________________
 * 
 *      The contents of this file are subject to the Mozilla Public License
 *      version 1.1 (the "License"); you may not use this file except in
 *      compliance with the License. You may obtain a copy of the License
 *      at http://www.mozilla.org/MPL/
 * 
 *      Software distributed under the License is distributed on an "AS IS"
 *      basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 *      the License for the specific language governing rights and
 *      limitations under the License.
 * 
 *      The Initial Developer of the Original Code is David Beckett.
 *      Portions created by David Beckett are Copyright (C) 2000 David
 *      Beckett, Institute for Learning and Research Technology, University
 *      of Bristol. All Rights Reserved.
 * 
 *      Alternatively, the contents of this file may be used under the
 *      terms of the GNU Lesser General Public License, in which case the
 *      provisions of the LGPL License are applicable instead of those
 *      above. If you wish to allow use of your version of this file only
 *      under the terms of the LGPL License and not to allow others to use
 *      your version of this file under the MPL, indicate your decision by
 *      deleting the provisions above and replace them with the notice and
 *      other provisions required by the LGPL License. If you do not delete
 *      the provisions above, a recipient may use your version of this file
 *      under either the MPL or the LGPL License.
 */



#ifndef RAPIER_H
#define RAPIER_H


#ifdef __cplusplus
extern "C" {
#endif

/* Public structure */
typedef struct rapier_parser_s rapier_parser;

typedef enum { RAPIER_SUBJECT_TYPE_RESOURCE } rapier_subject_type;
typedef enum { RAPIER_PREDICATE_TYPE_PREDICATE } rapier_predicate_type;
typedef enum { RAPIER_OBJECT_TYPE_RESOURCE } rapier_object_type;
  

typedef struct {
  const char *uri;
  const char *file;
  int line;
  int column;
  int byte;  
} rapier_locator;


typedef enum {
  RAPIER_FEATURE_SCANNING,
  RAPIER_FEATURE_RDF_NON_NS_ATTRIBUTES
} rapier_feature;


/* Public functions */

/* Create */
rapier_parser* rapier_new(void);
/* Destroy */
void rapier_free(rapier_parser *rdf_parser);

void rapier_parser_set_fatal_error(rapier_parser* parser, void *user_data, void (*fatal_error_fn)(void *user_data, rapier_locator *locator, const char *msg, ...));
void rapier_parser_set_error(rapier_parser* parser, void *user_data, void (*error_fn)(void *user_data, rapier_locator *locator, const char *msg, ...));
void rapier_parser_set_warning(rapier_parser* parser, void *user_data, void (*warning_fn)(void *user_data, rapier_locator *locator, const char *msg, ...));
  
void rapier_set_triple_handler(rapier_parser* parser, void *userData, void (*triple_handler)(void *userData, const char *subject, rapier_subject_type subject_type, const char *predicate, rapier_predicate_type predicate_type, const char *object, rapier_object_type object_type));
int rapier_parse_file(rapier_parser* rdf_parser,  const char *uri, const char *base_uri);
void rapier_print_locator(FILE *stream, rapier_locator* locator);

void rapier_set_feature(rapier_parser *parser, rapier_feature feature, int value);

#define RAPIER_RDF_MS_URI "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RAPIER_RDF_SCHEMA_URI "http://www.w3.org/2000/01/rdf-schema#"

#ifdef __cplusplus
}
#endif

#endif
