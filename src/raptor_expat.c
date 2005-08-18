/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_libxml.c - Raptor libxml functions
 *
 * $Id$
 *
 * Copyright (C) 2000-2004, David Beckett http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology http://www.ilrt.bristol.ac.uk/
 * University of Bristol, UK http://www.bristol.ac.uk/
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
 * 
 */


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_raptor_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


#ifdef RAPTOR_XML_EXPAT


void
raptor_expat_init(raptor_sax2* sax2, raptor_uri *base_uri)
{
  XML_Parser xp=XML_ParserCreate(NULL);

  /* create a new parser in the specified encoding */
  XML_SetUserData(xp, sax2->user_data);

  XML_SetBase(xp, raptor_uri_as_string(base_uri));

  /* XML_SetEncoding(xp, "..."); */

  XML_SetElementHandler(xp, 
                        (XML_StartElementHandler)sax2->start_element_handler,
                        (XML_EndElementHandler)sax2->end_element_handler);
  XML_SetCharacterDataHandler(xp, 
                              (XML_CharacterDataHandler)sax2->characters_handler);

  XML_SetCommentHandler(xp, (XML_CommentHandler)sax2->comment_handler);

  XML_SetUnparsedEntityDeclHandler(xp, sax2->unparsed_entity_decl_handler);

  XML_SetExternalEntityRefHandler(xp, (XML_ExternalEntityRefHandler)sax2->external_entity_ref_handler);

  sax2->xp=xp;
}

/* end if RAPTOR_XML_EXPAT */
#endif
