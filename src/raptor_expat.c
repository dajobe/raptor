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


XML_Parser
raptor_expat_init(raptor_parser *rdf_parser) {
  XML_Parser xp=XML_ParserCreate(NULL);

  /* create a new parser in the specified encoding */
  XML_SetUserData(xp, rdf_parser);

  /* XML_SetEncoding(xp, "..."); */

  XML_SetElementHandler(xp, 
                        (XML_StartElementHandler)raptor_xml_start_element_handler,
                        (XML_EndElementHandler)raptor_xml_end_element_handler);
  XML_SetCharacterDataHandler(xp, 
                              (XML_CharacterDataHandler)raptor_xml_characters_handler);

  XML_SetCommentHandler(xp,
                        (XML_CommentHandler)raptor_xml_comment_handler);


  XML_SetUnparsedEntityDeclHandler(xp, raptor_xml_unparsed_entity_decl_handler);

  XML_SetExternalEntityRefHandler(xp, (XML_ExternalEntityRefHandler)raptor_xml_external_entity_ref_handler);

  return xp;
}

/* end if RAPTOR_XML_EXPAT */
#endif
