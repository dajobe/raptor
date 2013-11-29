/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_concepts.c - Raptor RDF namespace concepts
 *
 * Copyright (C) 2010, David Beckett http://www.dajobe.org/
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

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


/*
 * http://www.w3.org/TR/rdf-syntax-grammar/#section-grammar-summary
 *
 * coreSyntaxTerms := rdf:RDF | rdf:ID | rdf:about | rdf:bagID | 
                      rdf:parseType | rdf:resource | rdf:nodeID | rdf:datatype
 * syntaxTerms     := coreSyntaxTerms | rdf:Description | rdf:li
 * oldTerms        := rdf:aboutEach | rdf:aboutEachPrefix | rdf:bagID
 *
 * nodeElementURIs       := anyURI - ( coreSyntaxTerms | rdf:li | oldTerms )
 * propertyElementURIs   := anyURI - ( coreSyntaxTerms | rdf:Description | oldTerms )
 * propertyAttributeURIs := anyURI - ( coreSyntaxTerms | rdf:Description | rdf:li | oldTerms )
 *
 * So, forbidden terms in the RDF namespace are:
 * nodeElements 
 *   RDF | ID | about | bagID | parseType | resource | nodeID | datatype |
 *   li | aboutEach | aboutEachPrefix | bagID
 *
 * propertyElements
 *   RDF | ID | about | bagID | parseType | resource | nodeID | datatype |
 *   Description | aboutEach | aboutEachPrefix | bagID
 *
 * propertyAttributes
 *   RDF | ID | about | bagID | parseType | resource | nodeID | datatype |
 *   Description | li | aboutEach | aboutEachPrefix | bagID
 *
 * Information about rdf attributes:
 *   raptor_term_type type
 *     Set when the attribute is a property rather than just syntax
 *     NOTE: raptor_rdfxml_process_property_attributes() expects only 
 *      RAPTOR_TERM_TYPE_NONE,
 *       RAPTOR_TERM_TYPE_LITERAL or RAPTOR_TERM_TYPE_URI
 *   allowed_unprefixed_on_attribute
 *     If allowed for legacy reasons to be unprefixed as an attribute.
 *
 */

/* (number of terms in RDF NS) + 1: for final sentinel row */
const raptor_rdf_ns_term_info raptor_rdf_ns_terms_info[RDF_NS_LAST + 2] = {
  /* term allowed boolean flags:
   *  node element; property element; property attr; unprefixed attr
   */
  /* syntax only */
  { "RDF",             RAPTOR_TERM_TYPE_UNKNOWN, 0, 0, 0, 0 }, /* just root */
  { "Description",     RAPTOR_TERM_TYPE_UNKNOWN, 1, 0, 0, 0 },
  { "li",              RAPTOR_TERM_TYPE_UNKNOWN, 0, 1, 0, 0 },
  { "about",           RAPTOR_TERM_TYPE_UNKNOWN, 0, 0, 0, 1 },
  { "aboutEach",       RAPTOR_TERM_TYPE_UNKNOWN, 0, 0, 0, 0 }, /* deprecated */
  { "aboutEachPrefix", RAPTOR_TERM_TYPE_UNKNOWN, 0, 0, 0, 0 }, /* deprecated */
  { "ID",              RAPTOR_TERM_TYPE_UNKNOWN, 0, 0, 0, 1 },
  { "bagID",           RAPTOR_TERM_TYPE_UNKNOWN, 0, 0, 0, 1 },
  { "resource",        RAPTOR_TERM_TYPE_UNKNOWN, 0, 0, 0, 1 },
  { "parseType",       RAPTOR_TERM_TYPE_UNKNOWN, 0, 0, 0, 1 },
  { "nodeID",          RAPTOR_TERM_TYPE_UNKNOWN, 0, 0, 0, 0 },
  { "datatype",        RAPTOR_TERM_TYPE_UNKNOWN, 0, 0, 0, 0 },

  /* rdf:Property-s */                                     
  { "type",            RAPTOR_TERM_TYPE_URI    , 1, 1, 1, 1 },
  { "value",           RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  { "subject",         RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  { "predicate",       RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  { "object",          RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  { "first",           RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  { "rest",            RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },

  /* rdfs:Class-s */
  { "Seq",             RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  { "Bag",             RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  { "Alt",             RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  { "Statement",       RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  { "Property",        RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  { "List",            RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },

  /* rdfs:Resource-s */                                    
  { "nil",             RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },

  /* datatypes */
  { "XMLLiteral",      RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  { "PlainLiteral",    RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  /* RDF 1.1 */
  /* http://www.w3.org/TR/2013/WD-rdf11-concepts-20130723/#section-html */
  { "HTML",            RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },
  /* http://www.w3.org/TR/2013/WD-rdf11-concepts-20130723/#section-Datatypes */
  { "langString",      RAPTOR_TERM_TYPE_LITERAL, 1, 1, 1, 0 },

  /* internal */
  { NULL ,             RAPTOR_TERM_TYPE_UNKNOWN, 1, 1, 1, 0 }
};



int
raptor_concepts_init(raptor_world* world)
{
  int i;
  
  for(i = 0; i < RDF_NS_LAST + 1; i++) {
    unsigned char* name = (unsigned char*)raptor_rdf_ns_terms_info[i].name;
    world->concepts[i] = raptor_new_uri_for_rdf_concept(world, name);
    if(!world->concepts[i])
      return 1;

    /* only make a term for things that are not syntax-only */
    /* OR use:
       raptor_rdf_ns_terms_info[i].allowed_as_nodeElement ||
       raptor_rdf_ns_terms_info[i].allowed_as_propertyElement ||
       raptor_rdf_ns_terms_info[i].allowed_as_propertyAttribute)
    */
    if(i > RDF_NS_LAST_SYNTAX_TERM) {
      world->terms[i] = raptor_new_term_from_uri(world, world->concepts[i]);
      if(!world->terms[i])
        return 1;
    }
  }

  world->xsd_namespace_uri = raptor_new_uri(world, raptor_xmlschema_datatypes_namespace_uri);
  if(!world->xsd_namespace_uri)
    return 1;

  world->xsd_boolean_uri = raptor_new_uri_from_uri_local_name(world, world->xsd_namespace_uri, (const unsigned char*)"boolean");
  if(!world->xsd_boolean_uri)
    return 1;

  world->xsd_decimal_uri = raptor_new_uri_from_uri_local_name(world, world->xsd_namespace_uri, (const unsigned char*)"decimal");
  if(!world->xsd_decimal_uri)
    return 1;

  world->xsd_double_uri = raptor_new_uri_from_uri_local_name(world, world->xsd_namespace_uri, (const unsigned char*)"double");
  if(!world->xsd_double_uri)
    return 1;

  world->xsd_integer_uri = raptor_new_uri_from_uri_local_name(world, world->xsd_namespace_uri, (const unsigned char*)"integer");
  if(!world->xsd_integer_uri)
    return 1;

  return 0;
}



void
raptor_concepts_finish(raptor_world* world)
{
  int i;
  
  for(i = 0; i < RDF_NS_LAST + 1; i++) {
    raptor_uri* concept_uri = world->concepts[i];
    if(concept_uri) {
      raptor_free_uri(concept_uri);
      world->concepts[i] = NULL;
    }
    if(world->terms[i])
      raptor_free_term(world->terms[i]);
  }

  if(world->xsd_boolean_uri)
    raptor_free_uri(world->xsd_boolean_uri);
  if(world->xsd_decimal_uri)
    raptor_free_uri(world->xsd_decimal_uri);
  if(world->xsd_double_uri)
    raptor_free_uri(world->xsd_double_uri);
  if(world->xsd_integer_uri)
    raptor_free_uri(world->xsd_integer_uri);

  if(world->xsd_namespace_uri)
    raptor_free_uri(world->xsd_namespace_uri);
}
