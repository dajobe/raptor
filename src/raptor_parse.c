/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rapier_parse.c - Redland Parser for RDF (RAPIER)
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

extern int errno;

#define RAPIER_INTERNAL

#ifdef LIBRDF_INTERNAL
/* if inside Redland */
#include <librdf.h>

#include <rdf_parser.h>
#include <rdf_node.h>
#include <rdf_stream.h>
#include <rdf_statement.h>

#else
/* else standalone */

#define LIBRDF_MALLOC(type, size) malloc(size)
#define LIBRDF_CALLOC(type, size, count) calloc(size, count)
#define LIBRDF_FREE(type, ptr)   free((void*)ptr)

#ifdef RAPIER_DEBUG
/* Debugging messages */
#define LIBRDF_DEBUG1(function, msg) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function); } while(0)
#define LIBRDF_DEBUG2(function, msg, arg1) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1);} while(0)
#define LIBRDF_DEBUG3(function, msg, arg1, arg2) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2);} while(0)
#define LIBRDF_DEBUG4(function, msg, arg1, arg2, arg3) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2, arg3);} while(0)

#else
/* DEBUGGING TURNED OFF */

/* No debugging messages */
#define LIBRDF_DEBUG1(function, msg)
#define LIBRDF_DEBUG2(function, msg, arg1)
#define LIBRDF_DEBUG3(function, msg, arg1, arg2)
#define LIBRDF_DEBUG4(function, msg, arg1, arg2, arg3)

#endif

#endif


/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPIER_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#undef HAVE_STDLIB_H
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif


/* XML parser includes */
#ifdef NEED_EXPAT
#include <xmlparse.h>
#endif

#ifdef NEED_LIBXML
#ifdef HAVE_GNOME_XML_PARSER_H
#include <gnome-xml/parser.h>
/* translate names from expat to libxml */
#define XML_Char xmlChar
#else
#include <parser.h>
#endif
#endif


/* Rapier includes */
#include <rapier.h>

/* Rapier structures */
/* namespace stack node */
typedef struct rapier_ns_map_s rapier_ns_map;

typedef enum {
  /* Not in RDF grammar yet - searching for a start element.
   * This can be <rdf:RDF> (goto 6.1) but since it is optional,
   * the start element can also be <Description> (goto 6.3), 
   * <rdf:Seq> (goto 6.25) <rdf:Bag> (goto 6.26) or <rdf:Alt> (goto 6.27)
   * OR from 6.3 can have ANY other element matching
   * typedNode (6.13) - goto 6.3
   * CHOICE: Search for <rdf:RDF> node before starting match
   * OR assume RDF content, hence go straight to production
   */
  RAPIER_STATE_UNKNOWN = 0,

  /* Met production 6.1 (RDF) <rdf:RDF> element seen and can now
   * expect  <rdf:Description> (goto 6.3), <rdf:Seq> (goto 6.25)
   * <rdf:Bag> (goto 6.26) or <rdf:Alt> (goto 6.27) OR from 6.3 can
   * have ANY other element matching typedNode (6.13) - goto 6.3
   */
  RAPIER_STATE_IN_RDF   = 6010,

  /* No need for 6.2 - already chose 6.3, 6.25, 6.26 or 6.27 */
  RAPIER_STATE_NOT_USED_1   = 6020,

  /* Met production 6.3 (description) <rdf:Description> element
   * OR 6.13 (typedNode) [pretty much anything else]
   * CHOICE: Create a bag here (always? even if no bagId given) 
   * CHOICES: Match rdf:resource/resource, ID/rdf:ID attributes etc.
   */
  RAPIER_STATE_DESCRIPTION = 6030,

  /* Matching part of 6.3 (description) inside a <Description> 
   * where either a list of propertyElt elements is expected or 
   * an empty element 
   */
  RAPIER_STATE_IN_DESCRIPTION = 6031,



  /* met production 6.12 (propertyElt)
   */
  RAPIER_STATE_PROPERTYELT = 6120,
  
  /* met production 6.13 (typedNode)
   */
  RAPIER_STATE_TYPED_NODE = 6130,
  


  /* Met production 6.25 (sequence) <rdf:Seq> element seen. Goto 6.28  */
  RAPIER_STATE_SEQ = 6250,

  /* Met production 6.26 (bag) <rdf:Bag> element seen. Goto 6.28  */
  RAPIER_STATE_BAG = 6260,

  /* Met production 6.27 (alternative) <rdf:Alt> element seen. Goto 6.28 */
  RAPIER_STATE_ALT = 6270,
  
  /* Met production 6.28 (member) 
   * Now expect <rdf:li> element and if it empty, with resource attribute
   * goto 6.29 otherwise goto 6.30
   * CHOICE: Match rdf:resource/resource
   */
  RAPIER_STATE_MEMBER = 6280,

  /* met production 6.29 (referencedItem)
   * Found a container item with reference - <rdf:li (rdf:)resource=".."/> */
  RAPIER_STATE_REFERENCEDITEM = 6290,

  /* met production 6.30 (inlineItem)
   * Found a container item with content - <rdf:li> */
  RAPIER_STATE_INLINEITEM = 6300,

  /* ******************************************************************* */
  /* Additional non-M&S states */

  /* Another kind of container, not Seq, Bag or Alt */
  RAPIER_STATE_CONTAINER = 7000,

} rapier_state;


/* Forms:
 * 1) prefix=NULL uri=<URI>      - default namespace defined
 * 2) prefix=NULL, uri=NULL      - no default namespace
 * 3) prefix=<prefix>, uri=<URI> - regular pair defined <prefix>:<URI>
 */
struct rapier_ns_map_s {
  struct rapier_ns_map_s* next; /* next down the stack, NULL at bottom */
  const char *prefix;           /* NULL means is the default namespace */
  int prefix_length;            /* needed to safely compare prefixed-names */
#ifdef LIBRDF_INTERNAL
  librdf_uri *uri;
#else  
  const char *uri;
  int uri_length;
#endif
  int depth;             /* parsing depth that this ns was added.  It will
                            be deleted when the parser leaves this depth */
  int is_rdf_ms;         /* Non 0 if is RDF M&S Namespace */
  int is_rdf_schema;     /* Non 0 if is RDF Schema Namespace */
};
 

/* 
 * Rapier XML-namespaced name, for elements or attributes 
 */

/* There are three forms
 * namespace=NULL                                - un-namespaced name
 * namespace=defined, namespace->prefix=NULL     - (default ns) name
 * namespace=defined, namespace->prefix=defined  - ns:name
 */
typedef struct {
  const rapier_ns_map *namespace;
  const char *qname;
#ifdef LIBRDF_INTERNAL
  librdf_uri *uri;
#else  
  const char *uri;   /* URI of namespace+qname or NULL if not defined */
#endif
  const char *value; /* optional value - used when name is an attribute */
} rapier_ns_name;


typedef enum {
  RDF_ATTR_about           = 0, /* value of rdf:about attribute */
  RDF_ATTR_aboutEach       = 1, /* " rdf:aboutEach       */
  RDF_ATTR_aboutEachPrefix = 2, /* " rdf:aboutEachPrefix */
  RDF_ATTR_ID              = 3, /* " rdf:ID */
  RDF_ATTR_bagID           = 4, /* " rdf:bagID */
  RDF_ATTR_reference       = 5, /* " rdf:reference */
  RDF_ATTR_type            = 6, /* " rdf:type */
  RDF_ATTR_parseType       = 7, /* " rdf:parseType */

  RDF_ATTR_LAST            = RDF_ATTR_parseType
} rdf_attr;

static const char * const rdf_attr_names[]={
  "about",
  "aboutEach",
  "aboutEachPrefix",
  "ID",
  "bagID",
  "reference",
  "type",
  "parseType",
};


/*
 * Rapier Element/attributes on stack 
 */
struct rapier_element_s {
  struct rapier_element_s *parent;  /* NULL at bottom of stack */
  rapier_ns_name *name;
  rapier_ns_name **attributes;
  int attribute_count;
  const char * rdf_attr[RDF_ATTR_LAST+1];     /* attributes declared in M&S */
  int rdf_attr_count;                         /* how many of above seen */

  rapier_state state; /* state that this production matches */

  /* CDATA content of element and checks for mixed content */
  char *content_cdata;
  int content_element_seen;
  int content_cdata_seen;
  int content_cdata_length;

};

typedef struct rapier_element_s rapier_element;


/*
 * Rapier parser object
 */
struct rapier_parser_s {
  /* XML parser specific stuff */
#ifdef NEED_EXPAT
  XML_Parser xp;
#endif
#ifdef NEED_LIBXML
  /* structure holding sax event handlers */
  xmlSAXHandler sax;
  /* parser context */
  /* xmlParserCtxtPtr xc; */
#endif  

  /* element depth */
  int depth;

  /* stack of namespaces, most recently added at top */
  rapier_ns_map *namespaces;

  /* can be filled with error location information */
  rapier_locator locator;

  /* stack of elements - elements add after current_element */
  rapier_element *root_element;
  rapier_element *current_element;
  
  /* non 0 if parser had fatal error and cannot continue */
  int failed;

  /* non 0 if scanning for <rdf:RDF> element, else assume doc is RDF */
  int scanning_for_rdf_RDF;
  
  /* non 0 to forbid non-namespaced resource, ID etc attributes
   * on RDF namespaced-elements
   */
  int forbid_rdf_non_ns_attributes; /* negative logic - FIXME */

  /* stuff for our user */
  void *user_data;

  void *fatal_error_user_data;
  void *error_user_data;
  void *warning_user_data;

  rapier_message_handler fatal_error_handler;
  rapier_message_handler error_handler;
  rapier_message_handler warning_handler;

  rapier_container_test_handler container_test_handler;

  /* parser callbacks */
  rapier_triple_handler triple_handler;
};




/* static variables */

#ifndef LIBRDF_INTERNAL
static const char * const rapier_rdf_ms_uri=RAPIER_RDF_MS_URI;
static const char * const rapier_rdf_schema_uri=RAPIER_RDF_SCHEMA_URI;
#endif
static const char * const rapier_xml_uri="http://www.w3.org/XML/1998/namespace";


/* Prototypes for common expat/libxml parsing event-handling functions */
static void rapier_xml_start_element_handler(void *user_data, const XML_Char *name, const XML_Char **atts);
static void rapier_xml_end_element_handler(void *user_data, const XML_Char *name);
/* s is not 0 terminated. */
static void rapier_xml_cdata_handler(void *user_data, const XML_Char *s, int len);
#ifdef HAVE_XML_SetNamespaceDeclHandler
static void rapier_start_namespace_decl_handler(void *user_data, const XML_Char *prefix, const XML_Char *uri);
static void rapier_end_namespace_decl_handler(void *user_data, const XML_Char *prefix);
#endif

/* libxml-only prototypes */
#ifdef NEED_LIBXML
static void rapier_xml_warning(void *context, rapier_locator *locator, const char *msg, ...);
static void rapier_xml_error(void *context, rapier_locator *locator, const char *msg, ...);
static void rapier_xml_fatal_error(void *context, rapier_locator *locator, const char *msg, ...);
#endif


/* Prototypes for local functions */
static char * rapier_file_uri_to_filename(const char *uri);
static void rapier_parser_fatal_error(rapier_parser* parser, const char *message, ...);
static void rapier_parser_error(rapier_parser* parser, const char *message, ...);
static void rapier_parser_warning(rapier_parser* parser, const char *message, ...);



/* prototypes for namespace and name/qname functions */
static void rapier_init_namespaces(rapier_parser *rdf_parser);
static void rapier_start_namespace(rapier_parser *rdf_parser, const char *prefix, const char *namespace, int depth);
static void rapier_free_namespace(rapier_parser *rdf_parser,  rapier_ns_map* namespace);
static void rapier_end_namespace(rapier_parser *rdf_parser, const char *prefix, const char *namespace);
static void rapier_end_namespaces_for_depth(rapier_parser *rdf_parser);
static rapier_ns_name* rapier_make_namespaced_name(rapier_parser *rdf_parser, const char *name, const char *value, int is_element);
#ifdef RAPIER_DEBUG
static void rapier_print_ns_name(FILE *stream, rapier_ns_name* name);
#endif
static void rapier_free_ns_name(rapier_ns_name* name);
static int rapier_ns_names_equal(rapier_ns_name *name1, rapier_ns_name *name2);


/* prototypes for element functions */
static rapier_element* rapier_element_pop(rapier_parser *rdf_parser);
static void rapier_element_push(rapier_parser *rdf_parser, rapier_element* element);
static void rapier_free_element(rapier_element *element);
#ifdef RAPIER_DEBUG
static void rapier_print_element(rapier_element *element, FILE* stream);
#endif


/* prototypes for grammar functions */
static void rapier_start_element_grammar(rapier_parser *parser, rapier_element *element);
static void rapier_end_element_grammar(rapier_parser *parser, rapier_element *element);


#ifdef LIBRDF_INTERNAL
#define IS_RDF_MS_CONCEPT(name, uri, qname) librdf_uri_equals(uri, librdf_concept_uris[LIBRDF_CONCEPT_MS_##qname])
#else
#define IS_RDF_MS_CONCEPT(name, uri, qname) !strcmp(name, #qname)
#endif




/*
 * Namespaces in XML
 * http://www.w3.org/TR/1999/REC-xml-names-19990114/#nsc-NSDeclared
 * (section 4) says:
 *
 * --------------------------------------------------------------------
 *   The prefix xml is by definition bound to the namespace name 
 *   http://www.w3.org/XML/1998/namespace
 * --------------------------------------------------------------------
 *
 * Thus should define it in the table of namespaces before we start.
 *
 * We *can* also define others, but let's not.
 *
 */
static void
rapier_init_namespaces(rapier_parser *rdf_parser) {
  /* defined at level -1 since always 'present' when inside the XML world */
  rapier_start_namespace(rdf_parser, "xml", rapier_xml_uri, -1);
}


static void
rapier_start_namespace(rapier_parser *rdf_parser, 
                       const char *prefix, const char *namespace, int depth)
{
  int prefix_length=0;
#ifndef LIBRDF_INTERNAL
  int uri_length=0;
#endif
  int len;
  rapier_ns_map *map;
  void *p;
  
  LIBRDF_DEBUG4(rapier_start_namespace,
                "namespace prefix %s uri %s depth %d\n",
                prefix ? prefix : "(default)", namespace, depth);

  /* Convert an empty namespace string "" to a NULL pointer */
  if(!*namespace)
    namespace=NULL;
  
  len=sizeof(rapier_ns_map);
#ifndef LIBRDF_INTERNAL
  if(namespace) {
    uri_length=strlen(namespace);
    len+=uri_length+1;
  }
#endif
  if(prefix) {
    prefix_length=strlen(prefix);
    len+=prefix_length+1;
  }
  
  /* Just one malloc for map structure + namespace (maybe) + prefix (maybe)*/
  map=(rapier_ns_map*)LIBRDF_CALLOC(rapier_ns_map, len, 1);
  if(!map) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }
  
  p=(void*)map+sizeof(rapier_ns_map);
#ifndef LIBRDF_INTERNAL
  if(namespace) {
    map->uri=strcpy((char*)p, namespace);
    map->uri_length=uri_length;
    p+= uri_length+1;
  }
#else
  map->uri=librdf_new_uri(namespace);
  if(!map->uri) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    LIBRDF_FREE(rapier_ns_map, map);
    return;
  }
#endif
  if(prefix) {
    map->prefix=strcpy((char*)p, prefix);
    map->prefix_length=prefix_length;
  }
  map->depth=depth;

  /* set convienience flags when there is a defined namespace URI */
  if(namespace) {
#ifdef LIBRDF_INTERNAL
    if(librdf_uri_equals(map->uri, librdf_concept_ms_namespace_uri))
      map->is_rdf_ms=1;
    else if(librdf_uri_equals(map->uri, librdf_concept_schema_namespace_uri))
      map->is_rdf_schema=1;
#else
    if(!strcmp(namespace, rapier_rdf_ms_uri))
      map->is_rdf_ms=1;
    else if(!strcmp(namespace, rapier_rdf_schema_uri))
      map->is_rdf_schema=1;
#endif
  }
  
  if(rdf_parser->namespaces)
    map->next=rdf_parser->namespaces;
  rdf_parser->namespaces=map;
}


static void 
rapier_free_namespace(rapier_parser *rdf_parser,  rapier_ns_map* namespace)
{
#ifdef LIBRDF_INTERNAL
  if(namespace->uri)
    librdf_free_uri(namespace->uri);
#endif
  LIBRDF_FREE(rapier_ns_map, namespace);
}


static void 
rapier_end_namespace(rapier_parser *rdf_parser, 
                     const char *prefix, const char *namespace) 
{
  LIBRDF_DEBUG3(rapier_end_namespace, "prefix %s uri \"%s\"\n", 
                prefix ? prefix : "(default)", namespace);
}


static void 
rapier_end_namespaces_for_depth(rapier_parser *rdf_parser) 
{
  while(rdf_parser->namespaces &&
        rdf_parser->namespaces->depth == rdf_parser->depth) {
    rapier_ns_map* ns=rdf_parser->namespaces;
    rapier_ns_map* next=ns->next;

#ifdef LIBRDF_INTERNAL
    rapier_end_namespace(rdf_parser, ns->prefix, 
                         librdf_uri_as_string(ns->uri));
#else  
    rapier_end_namespace(rdf_parser, ns->prefix, ns->uri);
#endif
    rapier_free_namespace(rdf_parser, ns);

    rdf_parser->namespaces=next;
  }

}



/*
 * Namespaces in XML
 * http://www.w3.org/TR/1999/REC-xml-names-19990114/#defaulting
 * says:
 *
 * --------------------------------------------------------------------
 *  5.2 Namespace Defaulting
 *
 *  A default namespace is considered to apply to the element where it
 *  is declared (if that element has no namespace prefix), and to all
 *  elements with no prefix within the content of that element. 
 *
 *  If the URI reference in a default namespace declaration is empty,
 *  then unprefixed elements in the scope of the declaration are not
 *  considered to be in any namespace.
 *
 *  Note that default namespaces do not apply directly to attributes.
 *
 * [...]
 *
 *  5.3 Uniqueness of Attributes
 *
 *  In XML documents conforming to this specification, no tag may
 *  contain two attributes which:
 *
 *    1. have identical names, or 
 *
 *    2. have qualified names with the same local part and with
 *    prefixes which have been bound to namespace names that are
 *    identical.
 * --------------------------------------------------------------------
 */

static rapier_ns_name*
rapier_make_namespaced_name(rapier_parser *rdf_parser, const char *name,
                            const char *value, int is_element) 
{
  rapier_ns_name* ns_name;
  const char *p;
  char *new_value=NULL;
  rapier_ns_map* ns;
  char* new_name;
  int prefix_length;
  int qname_length=0;
  
#if RAPIER_DEBUG > 1
  LIBRDF_DEBUG2(rapier_make_namespaced_name,
                "name %s\n", name);
#endif  

  ns_name=(rapier_ns_name*)LIBRDF_CALLOC(rapier_ns_name, sizeof(rapier_ns_name), 1);
  if(!ns_name) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    return NULL;
  }

  if(value) {
    new_value=(char*)LIBRDF_MALLOC(cstring, strlen(value)+1);
    if(!new_value) {
      rapier_parser_fatal_error(rdf_parser, "Out of memory");
      LIBRDF_FREE(rapier_ns_name, ns_name);
      return NULL;
    } 
    strcpy(new_value, value);
    ns_name->value=new_value;
  }

  /* Find : */
  for(p=name; *p && *p != ':'; p++)
    ;

  if(!*p) {
    /* No : - pick up default namespace, if there is one */
    new_name=(char*)LIBRDF_MALLOC(cstring, strlen(name)+1);
    if(!new_name) {
      rapier_parser_fatal_error(rdf_parser, "Out of memory");
      rapier_free_ns_name(ns_name);
      return NULL;
    }
    strcpy(new_name, name);
    ns_name->qname=new_name;

    /* Find a default namespace */
    for(ns=rdf_parser->namespaces; ns && ns->prefix; ns=ns->next)
      ;

    if(ns) {
      ns_name->namespace=ns;
#if RAPIER_DEBUG > 1
    LIBRDF_DEBUG2(rapier_make_namespaced_name,
                  "Found default namespace %s\n", ns->uri);
#endif
    } else {
      /* failed to find namespace - now what? FIXME */
      /* rapier_parser_warning(rdf_parser, "No default namespace defined - cannot expand %s", name); */
#if RAPIER_DEBUG > 1
    LIBRDF_DEBUG1(rapier_make_namespaced_name,
                  "No default namespace defined\n");
#endif
    }

  } else {
    /* There is a namespace prefix */

    prefix_length=p-name;
    p++; /* move to start of qname */
    qname_length=strlen(p);
    new_name=(char*)LIBRDF_MALLOC(cstring, qname_length+1);
    if(!new_name) {
      rapier_parser_fatal_error(rdf_parser, "Out of memory");
      rapier_free_ns_name(ns_name);
      return NULL;
    }
    strcpy(new_name, p);
    ns_name->qname=new_name;

    /* Find the namespace */
    for(ns=rdf_parser->namespaces; ns ; ns=ns->next)
      if(ns->prefix && prefix_length == ns->prefix_length && 
         !strncmp(name, ns->prefix, prefix_length))
        break;

    if(!ns) {
      /* failed to find namespace - now what? */
      rapier_parser_error(rdf_parser, "Failed to find namespace in %s", name);
      rapier_free_ns_name(ns_name);
      return NULL;
    }

#if RAPIER_DEBUG > 1
    LIBRDF_DEBUG3(rapier_make_namespaced_name,
                  "Found namespace prefix %s URI %s\n", ns->prefix, ns->uri);
#endif
    ns_name->namespace=ns;
  }



  /* If namespace has a URI and a qname is defined, create the URI
   * for this element 
   */
  if(ns_name->namespace && ns_name->namespace->uri && qname_length) {
#ifdef LIBRDF_INTERNAL
    librdf_uri* uri=librdf_new_uri_from_uri_qname(ns_name->namespace->uri,
                                                  new_name);
    if(!uri) {
      rapier_free_ns_name(ns_name);
      return NULL;
    }
    ns_name->uri=uri;
#else
    char *uri_string=(char*)LIBRDF_MALLOC(cstring, 
                                          ns_name->namespace->uri_length + 
                                          qname_length + 1);
    if(!uri_string) {
      rapier_free_ns_name(ns_name);
      return NULL;
    }
    strcpy(uri_string, ns_name->namespace->uri);
    strncpy(uri_string+ns_name->namespace->uri_length, new_name, qname_length);
    ns_name->uri=uri_string;
#endif
  }

  
  return ns_name;
}


#ifdef RAPIER_DEBUG
static void
rapier_print_ns_name(FILE *stream, rapier_ns_name* name) 
{
  if(name->namespace) {
    if(name->namespace->prefix)
      fprintf(stream, "%s:%s", name->namespace->prefix, name->qname);
    else
      fprintf(stream, "(default):%s", name->qname);
  } else
    fputs(name->qname, stream);
}
#endif

static void
rapier_free_ns_name(rapier_ns_name* name) 
{
  if(name->qname)
    LIBRDF_FREE(cstring, (void*)name->qname);

#ifdef LIBRDF_INTERNAL
  if(name->uri)
    librdf_free_uri(name->uri);
#else
  if(name->uri)
    LIBRDF_FREE(cstring, name->uri);
#endif
  
  if(name->value)
    LIBRDF_FREE(cstring, (void*)name->value);
  LIBRDF_FREE(rapier_ns_name, name);
}


static int
rapier_ns_names_equal(rapier_ns_name *name1, rapier_ns_name *name2)
{
#ifdef LIBRDF_INTERNAL
  if(name1->uri && name2->uri)
    return librdf_uri_equals(name1->uri, name2->uri);
#else
  if(name1->namespace != name2->namespace)
    return 0;
#endif
  if(strcmp(name1->qname, name2->qname))
    return 0;
  return 1;
}


static rapier_element*
rapier_element_pop(rapier_parser *rdf_parser) 
{
  rapier_element *element=rdf_parser->current_element;

  if(!element)
    return NULL;
  
  rdf_parser->current_element=element->parent;
  if(rdf_parser->root_element == element) /* just deleted root */
    rdf_parser->root_element=NULL;
    
  return element;
}
 

static void
rapier_element_push(rapier_parser *rdf_parser, rapier_element* element) 
{
  element->parent=rdf_parser->current_element;
  rdf_parser->current_element=element;
  if(!rdf_parser->root_element)
    rdf_parser->root_element=element;
}
 

static void
rapier_free_element(rapier_element *element)
{
  int i;
  
  for (i=0; i < element->attribute_count; i++)
    if(element->attributes[i])
      rapier_free_ns_name(element->attributes[i]);

  if(element->attributes)
    LIBRDF_FREE(rapier_ns_name_array, element->attributes);

  /* Free special RDF M&S attributes */
  for(i=0; i<= RDF_ATTR_LAST; i++) 
    if(element->rdf_attr[i])
      LIBRDF_FREE(cstring, (void*)element->rdf_attr[i]);
  
  if(element->content_cdata_length)
    LIBRDF_FREE(rapier_ns_name_array, element->content_cdata);

  rapier_free_ns_name(element->name);
  LIBRDF_FREE(rapier_element, element);
}



#ifdef RAPIER_DEBUG
static void
rapier_print_element(rapier_element *element, FILE* stream)
{
  int i;
  
  rapier_print_ns_name(stream, element->name);
  fputc('\n', stream);

  if(element->attribute_count) {
    fputs(" attributes: ", stream);
    for (i = 0; i < element->attribute_count; i++) {
      if(i)
        fputc(' ', stream);
      rapier_print_ns_name(stream, element->attributes[i]);
      fprintf(stream, "='%s'", element->attributes[i]->value);
    }
    fputc('\n', stream);
  }
}
#endif
 

static void
rapier_xml_start_element_handler(void *user_data,
                                 const XML_Char *name, const XML_Char **atts)
{
  rapier_parser* rdf_parser=(rapier_parser*)user_data;
  int all_atts_count=0;
  int ns_attributes_count=0;
  rapier_ns_name** named_attrs=NULL;
  int i;
  rapier_ns_name* element_name;
  rapier_element* element=NULL;
#ifdef NEED_EXPAT
  rapier_locator *locator=&rdf_parser->locator; /* for storing error info */
#endif

#ifdef NEED_EXPAT
  locator->line=XML_GetCurrentLineNumber(rdf_parser->xp);
  locator->column=XML_GetCurrentColumnNumber(rdf_parser->xp);
  locator->byte=XML_GetCurrentByteIndex(rdf_parser->xp);
#endif    

  rdf_parser->depth++;

  if (atts) {
    /* Round 1 - find special attributes, at present just namespaces */
    for (i = 0; atts[i]; i+=2) {
      all_atts_count++;

      /* synthesise the XML NS events */
      if(!strncmp(atts[i], "xmlns", 5)) {
        /* there is more i.e. xmlns:foo */
        const char *prefix=atts[i][5] ? &atts[i][6] : NULL;

        rapier_start_namespace(user_data, prefix, atts[i+1],
                               rdf_parser->depth);
        atts[i]=NULL; /* Is it allowed to zap XML parser array things? FIXME */
        continue;
      }

      ns_attributes_count++;
    }
  }
  

  /* Now can recode element name with a namespace */

  element_name=rapier_make_namespaced_name(rdf_parser, name, NULL, 1);
  if(!element_name) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }

  
  /* Create new element structure */
  element=(rapier_element*)LIBRDF_CALLOC(rapier_element, 
                                         sizeof(rapier_element), 1);
  if(!element) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    rapier_free_ns_name(element_name);
    return;
  } 


  element->name=element_name;

  /* Prepare for possible element content */
  element->content_element_seen=0;
  element->content_cdata_seen=0;
  element->content_cdata_length=0;
  



  if(ns_attributes_count) {
    int offset = 0;
    
    /* Round 2 - turn attributes into namespaced-attributes */
    
    /* Allocate new array to hold namespaced-attributes */
    named_attrs=(rapier_ns_name**)LIBRDF_CALLOC(rapier_ns_name-array, sizeof(rapier_ns_name*), ns_attributes_count);
    if(!named_attrs) {
      rapier_parser_fatal_error(rdf_parser, "Out of memory");
      LIBRDF_FREE(rapier_element, element);
      rapier_free_ns_name(element_name);
      return;
    }
    
    for (i = 0; i < all_atts_count; i++) {
      rapier_ns_name* attribute;
      
      /* Skip previously processed attributes */
      if(!atts[i<<1])
        continue;
      
      /* namespace-name[i] stored in named_attrs[i] */
      attribute=rapier_make_namespaced_name(rdf_parser, atts[i<<1],
                                            atts[(i<<1)+1], 0);
      if(!attribute) { /* failed - tidy up and return */
        int j;
        
        for (j=0; j < i; j++)
          LIBRDF_FREE(rapier_ns_name, named_attrs[j]);
        LIBRDF_FREE(rapier_ns_name_array, named_attrs);
        LIBRDF_FREE(rapier_element, element);
        rapier_free_ns_name(element_name);
        return;
      }
      
      /* Save pointers to some RDF M&S attributes */

      /* If RDF M&S namespace-prefixed attributes */
      if(attribute->namespace && attribute->namespace->is_rdf_ms) {
        const char *attr_name=attribute->qname;
        int j;
        
        for(j=0; j<= RDF_ATTR_LAST; j++)
          if(!strcmp(attr_name, rdf_attr_names[j])) {
            element->rdf_attr[j]=attribute->value;
            element->rdf_attr_count++;            
            /* Delete it if it was stored elsewhere */
#if RAPIER_DEBUG
            LIBRDF_DEBUG3(rapier_xml_start_element_handler,
                          "Found RDF M&S attribute %s URI %s\n",
                          attr_name, attribute->value);
#endif
            /* make sure value isn't deleted from ns_name structure */
            attribute->value=NULL;
            rapier_free_ns_name(attribute);
            attribute=NULL;
          }
      } /* end if RDF M&S namespaced-prefixed attributes */


      /* If non namespace-prefixed RDF M&S attributes found on
       * rdf namespace-prefixed element
       */
      if(!rdf_parser->forbid_rdf_non_ns_attributes &&
         attribute && !attribute->namespace &&
         element_name->namespace && element_name->namespace->is_rdf_ms) {
        const char *attr_name=attribute->qname;
        int j;
        
        for(j=0; j<= RDF_ATTR_LAST; j++)
          if(!strcmp(attr_name, rdf_attr_names[j])) {
            element->rdf_attr[j]=attribute->value;
            /* Delete it if it was stored elsewhere */
#if RAPIER_DEBUG
            LIBRDF_DEBUG3(rapier_xml_start_element_handler,
                          "Found non-namespaced RDF M&S attribute %s URI %s\n",
                          attr_name, attribute->value);
#endif
            /* make sure value isn't deleted from ns_name structure */
            attribute->value=NULL;
            rapier_free_ns_name(attribute);
            attribute=NULL;
        }
      } /* end if non-namespace prefixed RDF M&S attributes */
      

      if(attribute)
        named_attrs[offset++]=attribute;
    }
    
    /* set actual count from attributes that haven't been skipped */
    ns_attributes_count=offset;
    if(!offset && named_attrs) {
      /* all attributes were RDF M&S or other specials and deleted
       * so delete array and don't store pointer */
      LIBRDF_FREE(rapier_ns_name_array, named_attrs);
      named_attrs=NULL;
    }
      
  } /* end if ns_attributes_count */

  element->attributes=named_attrs;
  element->attribute_count=ns_attributes_count;

  
  rapier_element_push(rdf_parser, element);


  if(element->parent) {
    if(++element->parent->content_element_seen == 1 &&
       element->parent->content_cdata_seen == 1) {
      /* Uh oh - mixed content, the parent element has cdata too */
      rapier_parser_warning(rdf_parser, "element %s has mixed content.", 
                            element->parent->name->qname);
    }
  }
  

#ifdef RAPIER_DEBUG
  fprintf(stderr, "rapier_xml_start_element_handler: Start of namespaced-element: ");
  rapier_print_element(element, stderr);
#endif


  /* Right, now ready to enter the grammar */
  rapier_start_element_grammar(rdf_parser, element);
  
}


static void
rapier_xml_end_element_handler(void *user_data, const XML_Char *name)
{
  rapier_parser* rdf_parser=(rapier_parser*)user_data;
  rapier_element* element;
  rapier_ns_name *element_name;

#ifdef NEED_EXPAT
  rapier_locator *locator=&rdf_parser->locator; /* for storing error info */
#endif

#ifdef NEED_EXPAT
  locator->line=XML_GetCurrentLineNumber(rdf_parser->xp);
  locator->column=XML_GetCurrentColumnNumber(rdf_parser->xp);
  locator->byte=XML_GetCurrentByteIndex(rdf_parser->xp);
#endif    

  /* recode element name */

  element_name=rapier_make_namespaced_name(rdf_parser, name, NULL, 1);
  if(!element_name) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }

  
#ifdef RAPIER_DEBUG
  fprintf(stderr, "rapier_xml_end_element_handler: End of namespaced-element: ");
  rapier_print_ns_name(stderr, element_name);
  fputc('\n', stderr);
#endif

  element=rapier_element_pop(rdf_parser);
  if(!rapier_ns_names_equal(element->name, element_name)) {
    /* Hmm, unexpected name - FIXME, should do something! */
    rapier_parser_warning(rdf_parser, "Element %s ended, expected end of element %s\n", name, element->name->qname);
    return;
  }

  rapier_end_element_grammar(rdf_parser, element);

  rapier_free_ns_name(element_name);

  rapier_end_namespaces_for_depth(rdf_parser);
  rapier_free_element(element);

  rdf_parser->depth--;
}



/* cdata (and ignorable whitespace for libxml). 
 * s is not 0 terminated for expat, is for libxml - grrrr.
 */
static void
rapier_xml_cdata_handler(void *user_data, const XML_Char *s, int len)
{
  rapier_parser* rdf_parser=(rapier_parser*)user_data;
  rapier_element* element;
  rapier_state state;
  char *buffer;
  char *ptr;
  int all_whitespace=1;
  int ignore_all_whitespace=0;
  int i;
  
  for(i=0; i<len; i++)
    if(!isspace(s[i])) {
      all_whitespace=0;
      break;
    }
  
  element=rdf_parser->current_element;

  /* cdata never changes the parser state */
  
  state=element->state;
  switch(state) {
    case RAPIER_STATE_UNKNOWN:
      /* Ignore all cdata if still looking for RDF */
      if(rdf_parser->scanning_for_rdf_RDF)
        return;

      /* Ignore all whitespace cdata before first element */
      if(all_whitespace)
        return;

      /* This probably will never happen since that would make the
       * XML not be well-formed
       */
      rapier_parser_warning(rdf_parser, "Found cdata before RDF element.");
      break;

    case RAPIER_STATE_IN_RDF:
    case RAPIER_STATE_IN_DESCRIPTION:
      /* Ignore all whitespace cdata inside <RDF> or <Description> 
       * when it occurs although note it was seen
       */
      ignore_all_whitespace=1;
      break;

    case RAPIER_STATE_DESCRIPTION:
      /* Never reached in any code outside start element 
       * since immediately moves on to RAPIER_STATE_IN_DESCRIPTION
       * or RAPIER_STATE_TYPED_NODE
       */
      abort();
      break;

    case RAPIER_STATE_TYPED_NODE:
    case RAPIER_STATE_SEQ:
    case RAPIER_STATE_BAG:
    case RAPIER_STATE_ALT:
    case RAPIER_STATE_MEMBER:
    case RAPIER_STATE_REFERENCEDITEM:
    case RAPIER_STATE_INLINEITEM:
    case RAPIER_STATE_PROPERTYELT:
      break;

    default:
      rapier_parser_fatal_error(rdf_parser, "Unexpected parser state %d.", 
                                state);
      return;
    } /* end switch */



  if(++element->content_cdata_seen == 1 &&
     element->content_element_seen == 1) {
    /* Uh oh - mixed content, this element has elements too */
    rapier_parser_warning(rdf_parser, "element %s has mixed content.", 
                          element->name->qname);
  }

  if(all_whitespace && ignore_all_whitespace) {
    LIBRDF_DEBUG2(rapier_xml_end_element_handler, "Ignoring whitespace cdata inside element %s\n", element->name->qname);
    return;
  }
  
  buffer=(char*)LIBRDF_MALLOC(cstring, element->content_cdata_length + len + 1);
  if(!buffer) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }

  if(element->content_cdata_length) {
    strncpy(buffer, element->content_cdata, element->content_cdata_length);
    LIBRDF_FREE(cstring, element->content_cdata);
  }
  element->content_cdata=buffer;

  ptr=buffer+element->content_cdata_length; /* append */

  /* adjust stored length */
  element->content_cdata_length += len;

  /* now write new stuff at end of cdata buffer */
  strncpy(ptr, s, len);
  ptr += len;
  *ptr = '\0';

  LIBRDF_DEBUG3(rapier_xml_cdata_handler, 
                "content cdata now: '%s' (%d bytes)\n", 
                buffer, element->content_cdata_length);
}


#ifdef HAVE_XML_SetNamespaceDeclHandler
static void
rapier_start_namespace_decl_handler(void *user_data,
                                    const XML_Char *prefix, const XML_Char *uri)
{
  rapier_parser* rdf_parser=(rapier_parser*)user_data;

#ifdef RAPIER_DEBUG
  fprintf(stderr_parser->locator, "saw namespace %s URI %s\n", prefix, uri);
#endif
}


static void
rapier_end_namespace_decl_handler(void *user_data, const XML_Char *prefix)
{
  rapier_parser* rdf_parser=(rapier_parser*)user_data;

#ifdef RAPIER_DEBUG
  fprintf(stderr_parser->locator, "saw end namespace prefix %s\n", prefix);
#endif
}
#endif


/* This is called for a declaration of an unparsed (NDATA) entity */
static void
rapier_xml_unparsed_entity_decl_handler(void *user_data,
                                        const XML_Char *entityName,
                                        const XML_Char *base,
                                        const XML_Char *systemId,
                                        const XML_Char *publicId,
                                        const XML_Char *notationName) 
{
/*  rapier_parser* rdf_parser=(rapier_parser*)user_data; */
  fprintf(stderr,
          "rapier_xml_unparsed_entity_decl_handler: entityName %s base %s systemId %s publicId %s notationName %s\n",
          entityName, (base ? base : "(None)"), 
          systemId, (publicId ? publicId: "(None)"),
          notationName);
}
  

static int 
rapier_xml_external_entity_ref_handler(XML_Parser parser,
                                       const XML_Char *context,
                                       const XML_Char *base,
                                       const XML_Char *systemId,
                                       const XML_Char *publicId)
{
/*  rapier_parser* rdf_parser=(rapier_parser*)user_data; */
  fprintf(stderr,
          "rapier_xml_external_entity_ref_handler: context %s base %s systemId %s publicId %s\n",
          context, (base ? base : "(None)"), 
          systemId, (publicId ? publicId: "(None)"));

  /* "The handler should return 0 if processing should not continue
   * because of a fatal error in the handling of the external entity."
   */
  return 1;
}



#ifdef NEED_LIBXML
#include <stdarg.h>

static const char* xml_warning_prefix="XML parser warning - ";
static const char* xml_error_prefix="XML parser error - ";
static const char* xml_fatal_error_prefix="XML parser fatal error - ";

static void
rapier_xml_warning(void *ctx, const char *msg, ...) 
{
  va_list args;
  int length;
  char *nmsg;

  va_start(args, msg);
  length=strlen(xml_warning_prefix)+strlen(msg)+1;
  msg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!msg) {
    /* just pass on, might be out of memory error */
    rapier_parser_warning(parser, msg, args);
  } else {
    strcpy(nmsg, xml_warning_prefix);
    strcat(nmsg, msg);
    rapier_parser_warning(parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
rapier_xml_error(void *ctx, const char *msg, ...) 
{
  va_list args;
  int length;
  char *nmsg;

  va_start(args, msg);
  length=strlen(xml_error_prefix)+strlen(msg)+1;
  msg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!msg) {
    /* just pass on, might be out of memory error */
    rapier_parser_error(parser, msg, args);
  } else {
    strcpy(nmsg, xml_error_prefix);
    strcat(nmsg, msg);
    rapier_parser_error(parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
rapier_xml_fatal_error(void *ctx, const char *msg, ...) 
{
  va_list args;
  int length;
  char *nmsg;

  va_start(args, msg);
  length=strlen(xml_fatal_error_prefix)+strlen(msg)+1;
  msg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!msg) {
    /* just pass on, might be out of memory error */
    rapier_parser_fatal_error(parser, msg, args);
  } else {
    strcpy(nmsg, xml_error_prefix);
    strcat(nmsg, msg);
    rapier_parser_fatal_error(parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}

#endif



/**
 * rapier_file_uri_to_filename - Convert a URI representing a file (starting file:) to a filename
 * @uri: URI of string
 * 
 * Return value: the filename or NULL on failure
 **/
static char *
rapier_file_uri_to_filename(const char *uri) 
{
  int length;
  char *filename;
  
  if (strncmp(uri, "file:", 5))
    return NULL;
  
  /* FIXME: unix version of URI -> filename conversion */
  length=strlen(uri) -5 +1;
  filename=LIBRDF_MALLOC(cstring, length);
  if(!filename)
    return NULL;
  
  strcpy(filename, uri+5);
  return filename;
}


/*
 * rapier_parser_fatal_error - Error from a parser - Internal
 **/
static void
rapier_parser_fatal_error(rapier_parser* parser, const char *message, ...)
{
  va_list arguments;

  parser->failed=1;
  
  if(parser->fatal_error_handler) {
    parser->fatal_error_handler(parser->fatal_error_user_data, 
                                &parser->locator, message);
    abort();
  }

  va_start(arguments, message);

  rapier_print_locator(stderr, &parser->locator);
  fprintf(stderr, " rapier fatal error - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  va_end(arguments);

  abort();
}


/*
 * rapier_parser_error - Error from a parser - Internal
 **/
static void
rapier_parser_error(rapier_parser* parser, const char *message, ...)
{
  va_list arguments;

  if(parser->error_handler) {
    parser->error_handler(parser->error_user_data, 
                          &parser->locator, message);
    return;
  }
  
  va_start(arguments, message);

  rapier_print_locator(stderr, &parser->locator);
  fprintf(stderr, " rapier error - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  va_end(arguments);
}


/*
 * rapier_parser_warning - Warning from a parser - Internal
 **/
static void
rapier_parser_warning(rapier_parser* parser, const char *message, ...)
{
  va_list arguments;

  if(parser->warning_handler) {
    parser->warning_handler(parser->warning_user_data,
                            &parser->locator, message);
    return;
  }
  
  va_start(arguments, message);

  rapier_print_locator(stderr, &parser->locator);
  fprintf(stderr, " rapier warning - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  va_end(arguments);
}


#ifdef NEED_LIBXML
/* from http://www.daa.com.au/~james/gnome/xml-sax/implementing.html */
#include <parserInternals.h>

static int myXmlSAXParseFile(xmlSAXHandlerPtr sax, void *user_data, const char *filename);

static int
myXmlSAXParseFile(xmlSAXHandlerPtr sax, void *user_data, const char *filename)
{
  int ret = 0;
  xmlParserCtxtPtr ctxt;
  
  ctxt = xmlCreateFileParserCtxt(filename);
  if (ctxt == NULL) return -1;
  ctxt->sax = sax;
  ctxt->userData = user_data;
  
  xmlParseDocument(ctxt);
  
  if (ctxt->wellFormed)
    ret = 0;
  else
    ret = -1;
  if (sax)
    ctxt->sax = NULL;
  xmlFreeParserCtxt(ctxt);
  
  return ret;
}
#endif




/* PUBLIC FUNCTIONS */

/**
 * rapier_new - Initialise the Rapier RDF parser
 *
 * Return value: non 0 on failure
 **/
rapier_parser*
rapier_new(void) 
{
  rapier_parser* rdf_parser;
#ifdef NEED_EXPAT
  XML_Parser xp;
#endif

  rdf_parser=LIBRDF_CALLOC(rapier_parser, 1, sizeof(rapier_parser));

  if(!rdf_parser)
    return NULL;
  
#ifdef NEED_EXPAT
  xp=XML_ParserCreate(NULL);

  /* create a new parser in the specified encoding */
  XML_SetUserData(xp, rdf_parser);

  /* XML_SetEncoding(xp, "..."); */

  XML_SetElementHandler(xp, rapier_xml_start_element_handler,
                        rapier_xml_end_element_handler);
  XML_SetCharacterDataHandler(xp, rapier_xml_cdata_handler);

  XML_SetUnparsedEntityDeclHandler(xp,
                                   rapier_xml_unparsed_entity_decl_handler);

  XML_SetExternalEntityRefHandler(xp,
                                  rapier_xml_external_entity_ref_handler);


#ifdef HAVE_XML_SetNamespaceDeclHandler
  XML_SetNamespaceDeclHandler(xp,
                              rapier_start_namespace_decl_handler,
                              rapier_end_namespace_decl_handler);
#endif
  rdf_parser->xp=xp;
#endif

#ifdef NEED_LIBXML
  xmlDefaultSAXHandlerInit();
  rdf_parser->sax.startElement=rapier_xml_start_element_handler;
  rdf_parser->sax.endElement=rapier_xml_end_element_handler;

  rdf_parser->sax.characters=rapier_xml_cdata_handler;
  rdf_parser->sax.ignorableWhitespace=rapier_xml_cdata_handler;

  rdf_parser->sax.warning=rapier_xml_warning;
  rdf_parser->sax.error=rapier_xml_error;
  rdf_parser->sax.fatalError=rapier_xml_fatal_error;

  /* xmlInitParserCtxt(&rdf_parser->xc); */
#endif

  rapier_init_namespaces(rdf_parser);

  return rdf_parser;
}




/**
 * rapier_free - Free the Rapier RDF parser
 * @rdf_parser: parser object
 * 
 **/
void
rapier_free(rapier_parser *rdf_parser) 
{
  rapier_element* element;
  rapier_ns_map* ns;
  
  ns=rdf_parser->namespaces;
  while(ns) {
    rapier_ns_map* next_ns=ns->next;

    rapier_free_namespace(rdf_parser, ns);
    ns=next_ns;
  }

  while((element=rapier_element_pop(rdf_parser))) {
    rapier_free_element(element);
  }

  LIBRDF_FREE(rapier_parser, rdf_parser);
}


/**
 * rapier_set_fatal_error_handler - Set the parser error handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser fails.
 * 
 **/
void
rapier_set_fatal_error_handler(rapier_parser* parser, void *user_data,
                               rapier_message_handler handler)
{
  parser->fatal_error_user_data=user_data;
  parser->fatal_error_handler=handler;
}


/**
 * rapier_set_error_handler - Set the parser error handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser fails.
 * 
 **/
void
rapier_set_error_handler(rapier_parser* parser, void *user_data,
                         rapier_message_handler handler)
{
  parser->error_user_data=user_data;
  parser->error_handler=handler;
}


/**
 * rapier_set_warning_handler - Set the parser warning handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser gives a warning.
 * 
 **/
void
rapier_set_warning_handler(rapier_parser* parser, void *user_data,
                           rapier_message_handler handler)
{
  parser->warning_user_data=user_data;
  parser->warning_handler=handler;
}


void
rapier_set_triple_handler(rapier_parser* parser,
                          void *user_data, 
                          rapier_triple_handler handler)
{
  parser->user_data=user_data;
  parser->triple_handler=handler;
}




  
/**
 * rapier_parse_file - Retrieve the RDF/XML content at URI
 * @rdf_parser: parser
 * @uri: URI of RDF content
 * @base_uri: the base URI to use (or NULL if the same)
 * 
 * Return value: non 0 on failure
 **/
int
rapier_parse_file(rapier_parser* rdf_parser,  const char *uri,
                  const char *base_uri) 
{
#ifdef NEED_EXPAT
  XML_Parser xp;
#endif
#ifdef NEED_LIBXML
  /* parser context */
  xmlParserCtxtPtr xc;
#endif
#define RBS 1024
  FILE *fh;
  char buffer[RBS];
  int rc=1;
  int len;
  const char *filename;
  rapier_locator *locator=&rdf_parser->locator; /* for storing error info */

  /* initialise fields */
  rdf_parser->depth=0;
  rdf_parser->root_element= rdf_parser->current_element=NULL;
  rdf_parser->failed=0;
  

 
#ifdef NEED_EXPAT
  xp=rdf_parser->xp;

  XML_SetBase(xp, base_uri);
#endif


  filename=rapier_file_uri_to_filename(uri);
  if(!filename)
    return 1;

  locator->file=filename;
  locator->uri=base_uri;
  
  fh=fopen(filename, "r");
  if(!fh) {
    rapier_parser_error(rdf_parser, "file open failed - %s", strerror(errno));
#ifdef NEED_EXPAT
    XML_ParserFree(xp);
#endif /* EXPAT */
    LIBRDF_FREE(cstring, (void*)filename);
    return 1;
  }
  
#ifdef NEED_LIBXML
  /* libxml needs at least 4 bytes from the XML content to allow
   * content encoding detection to work */
  len=fread(buffer, 1, 4, fh);
  if(len>0) {
    xc = xmlCreatePushParserCtxt(&rdf_parser->sax, rdf_parser,
                                 buffer, len, filename);
  } else {
    fclose(fh);
    fh=NULL;
  }
  
#endif

  while(fh && !feof(fh)) {
    len=fread(buffer, 1, RBS, fh);
    if(len <= 0) {
#ifdef NEED_EXPAT
      XML_Parse(xp, buffer, 0, 1);
#endif
#ifdef NEED_LIBXML
      xmlParseChunk(xc, buffer, 0, 1);
#endif
      break;
    }
#ifdef NEED_EXPAT
    rc=XML_Parse(xp, buffer, len, (len < RBS));
    if(len < RBS)
      break;
    if(!rc) /* expat: 0 is failure */
      break;
#endif
#ifdef NEED_LIBXML
    rc=xmlParseChunk(xc, buffer, len, 0);
    if(len < RBS)
      break;
    if(rc) /* libxml: non 0 is failure */
      break;
#endif
  }
  fclose(fh);

#ifdef NEED_EXPAT
  if(!rc) {
    int xe=XML_GetErrorCode(xp);

    locator->line=XML_GetCurrentLineNumber(xp);
    locator->column=XML_GetCurrentColumnNumber(xp);
    locator->byte=XML_GetCurrentByteIndex(xp);

    rapier_parser_error(rdf_parser, "XML Parsing failed - %s",
                        XML_ErrorString(xe));
    rc=1;
  } else
    rc=0;

  XML_ParserFree(xp);
#endif /* EXPAT */
#ifdef NEED_LIBXML
  if(rc) {
    rapier_parser_error(parser, "XML Parsing failed");
#endif

  LIBRDF_FREE(cstring, (void*)filename);

  return (rc != 0);
}


void
rapier_print_locator(FILE *stream, rapier_locator* locator) 
{
  if(!locator)
    return;
  
  if(locator->uri)
    fprintf(stream, "URI %s", locator->uri);
  else if (locator->file)
    fprintf(stream, "file %s", locator->file);
  else
    return;
  if(locator->line) {
    fprintf(stream, ":%d", locator->line);
    if(locator->column)
      fprintf(stream, " column %d", locator->column);
  }
}
 


void
rapier_set_feature(rapier_parser *parser, rapier_feature feature, int value) {
  switch(feature) {
    case RAPIER_FEATURE_SCANNING:
      parser->scanning_for_rdf_RDF=value;
      break;

    case RAPIER_FEATURE_RDF_NON_NS_ATTRIBUTES:
      parser->forbid_rdf_non_ns_attributes=!value; /* negative logic - FIXME */
      break;

    default:
      break;
  }
}



static void
rapier_start_element_grammar(rapier_parser *rdf_parser,
                             rapier_element *element) 
{
  int finished;
  rapier_state state;
  
  finished= 0;
  if(element->parent)
    state=element->parent->state;
  else
    state=RAPIER_STATE_UNKNOWN;

  while(!finished) {
    const char *el_name=element->name->qname;
    int element_in_rdf_ns=(element->name->namespace && 
                           element->name->namespace->is_rdf_ms);
    
    switch(state) {
      case RAPIER_STATE_UNKNOWN:
        if(element_in_rdf_ns && 
          IS_RDF_MS_CONCEPT(el_name, element->name->uri,RDF)) {
          state=RAPIER_STATE_IN_RDF;
          /* need more content before can continue */
          finished=1;
          break;
        }
        /* If scanning for element, can continue */
        if(rdf_parser->scanning_for_rdf_RDF) {
          finished=1;
          break;
        }
        /* Otherwise choice of next state can be made from the current
         * element by IN_RDF state */

        state=RAPIER_STATE_IN_RDF;
        break;

      case RAPIER_STATE_IN_RDF:
        if(element_in_rdf_ns) {
          if(IS_RDF_MS_CONCEPT(el_name, element->name->uri,Description)) {
            state=RAPIER_STATE_DESCRIPTION;
            break;
          } else if(IS_RDF_MS_CONCEPT(el_name, element->name->uri,Seq)) {
            state=RAPIER_STATE_SEQ;
            break;
          } else if(IS_RDF_MS_CONCEPT(el_name, element->name->uri,Bag)) {
            state=RAPIER_STATE_BAG;
            break;
          } else if(IS_RDF_MS_CONCEPT(el_name, element->name->uri,Alt)) {
            state=RAPIER_STATE_ALT;
            break;
          } else if(rdf_parser->container_test_handler) {
            if(rdf_parser->container_test_handler(element->name->uri)) {
              state=RAPIER_STATE_CONTAINER;
              break;
            }
          }


          /* Unexpected rdf: element at outer layer */
          rapier_parser_error(rdf_parser, "Unexpected RDF M&S element %s in <rdf:RDF> - from productions 6.2, 6.3 and 6.4 expected rdf:Description, rdf:Seq, rdf:Bag or rdf:Alt only.", el_name);
          finished=1;
        }
        
        /* Hmm, must be a typedNode, handled by the description state 
         * so that ID, BagID are handled in one place.
         */
        state=RAPIER_STATE_DESCRIPTION;          
        break;


      /* No need for 6.2 - already chose 6.3, 6.25, 6.26 or 6.27 */


      case RAPIER_STATE_DESCRIPTION:
        /* choices here from production 6.3 (description)
         * <rdf:Description idAboutAttr? bagIdAttr? propAttr* >
         *   Attributes: (ID|about|aboutEach|aboutEachPrefix)? bagID? propAttr*
         * <typeName idAboutAttr? bagIdAttr? propAttr*>
         *   Attributes: (ID|about|aboutEach|aboutEachPrefix)? bagID? propAttr*
         * (either may have no content, that is tested in the end element code)
         */

        /* lets add booleans - isn't C wonderful! */
        if((element->rdf_attr[RDF_ATTR_ID] != NULL) +
           (element->rdf_attr[RDF_ATTR_about] != NULL) +
           (element->rdf_attr[RDF_ATTR_aboutEach] != NULL) +
           (element->rdf_attr[RDF_ATTR_aboutEachPrefix] != NULL) > 1) {
          rapier_parser_warning(rdf_parser, "More than one of RDF ID, about, aboutEach or aboutEachPrefix attributes on element %s - from productions 6.5, 6.6, 6.7 and 6.8 expect at most one.", el_name);
        }
        

        /* has to be rdf:Description OR typedNode - checked above */
        if(element_in_rdf_ns)
          state=RAPIER_STATE_IN_DESCRIPTION;
        else
          /* otherwise must match the typedNode production - checked below */
          state=RAPIER_STATE_TYPED_NODE;

        finished=1;
        break;


      /* Inside a <rdf:Description> so expecting a list of
       * propertyElt elements
       */
      case RAPIER_STATE_IN_DESCRIPTION:
        state=RAPIER_STATE_PROPERTYELT;
        finished=1;
        break;


      /* Expect to meet the typedNode production having 
       * fallen through and not met other productions -
       * 6.3, 6.25, 6.26, 6.27.  This is the last choice.
       *
       * choices here from production 6.13 (typedNode)
       * <typeName idAboutAttr? bagIdAttr? propAttr* />
       *   Attributes: (ID|about|aboutEach|aboutEachPrefix)? bagID?
       * <typeName idAboutAttr? bagIdAttr? propAttr* > propertyElt* </typeName>
       *   Attributes: (ID|about|aboutEach|aboutEachPrefix)? bagID? propAttr*
       */
      case RAPIER_STATE_TYPED_NODE:
        finished=1;
        break;

      case RAPIER_STATE_SEQ:
      case RAPIER_STATE_BAG:
      case RAPIER_STATE_ALT:
      case RAPIER_STATE_CONTAINER:
        /* choices here from production 6.25 (sequence), 6.26 (bag)
         * and 6.27 (alternative)
         * <rdf:Seq idAttr?>...</rdf:Seq> (attribute productions: 6.6)
         *   Attributes: ID?
         * <rdf:Seq idAttr? memberAttr* /> (attribute productions: 6.6, 6.31)
         *   Attributes: ID? rdf:_<n> (where n is an integer)
         *
         * Note: The second case implies the element must have no content.
         */

        /* Enforce allowed rdf: attributes - i.e. ID only (optional) */
        if ((element->rdf_attr - (element->rdf_attr[RDF_ATTR_ID] != NULL))) {
          rapier_parser_warning(rdf_parser, "Illegal rdf: attribute(s) seen on container element %s - from production 6.25, 6.26 and 6.27 expected rdf:ID only.", el_name);
        }

        finished=1;
        break;

      case RAPIER_STATE_MEMBER:
        finished=1;
        break;

      case RAPIER_STATE_REFERENCEDITEM:
        finished=1;
        break;

      case RAPIER_STATE_INLINEITEM:
        finished=1;
        break;

        /* choices here from production 6.12 (propertyElt)
         *   <propName idAttr?> value </propName>
         *     Attributes: ID?
         *   <propName idAttr? parseLiteral> literal </propName>
         *     Attributes: ID? parseType="literal"
         *   <propName idAttr? parseResource> propertyElt* </propName>
         *     Attributes: ID? parseType="resource"
         *   <propName idRefAttr? bagIdAttr? propAttr* />
         *     Attributes: (ID|resource)? bagIdAttr? propAttr*
         */
      case RAPIER_STATE_PROPERTYELT:
        finished=1;
        break;

      default:
        rapier_parser_fatal_error(rdf_parser, "Unexpected parser state %d.", 
                                  state);
        finished=1;
        
    } /* end switch */

    if(state != element->state) {
      element->state=state;
      LIBRDF_DEBUG2(rapier_start_element_grammar, "moved to state %d\n", state);
    }
  
  } /* end while */
}


static void
rapier_end_element_grammar(rapier_parser *rdf_parser,
                           rapier_element *element) 
{
  rapier_state state;
  int finished;

  state=element->state;
  finished= 0;
  while(!finished) {
    const char *el_name=element->name->qname;
    int element_in_rdf_ns=(element->name->namespace && 
                           element->name->namespace->is_rdf_ms);
    
    switch(state) {
      case RAPIER_STATE_UNKNOWN:
        finished=1;
        break;

      case RAPIER_STATE_IN_RDF:
        if(element_in_rdf_ns && 
          IS_RDF_MS_CONCEPT(el_name, element->name->uri,RDF)) {
          /* end of RDF - boo hoo */
          state=RAPIER_STATE_UNKNOWN;
          finished=1;
          break;
        }
        /* When scanning, another element ending is outside the RDF
         * world so this can happen without further work
         */
        if(rdf_parser->scanning_for_rdf_RDF) {
          state=RAPIER_STATE_UNKNOWN;
          finished=1;
          break;
        }
        /* otherwise found some junk after RDF content in an RDF-only 
         * document (probably never get here since this would be
         * a mismatched XML tag and cause an error earlier)
         */
        rapier_parser_warning(rdf_parser, "Element %s ended, expected end of RDF element\n", el_name);
        state=RAPIER_STATE_UNKNOWN;
        finished=1;
        break;

      /* No need for 6.2 - already chose 6.3, 6.25, 6.26 or 6.27 */

      case RAPIER_STATE_DESCRIPTION:
        /* Never reached in any code outside start element 
         * since immediately moves on to RAPIER_STATE_IN_DESCRIPTION
         * or RAPIER_STATE_TYPED_NODE
         */
        abort();
        break;

      case RAPIER_STATE_IN_DESCRIPTION:
        /* Must be end of description production </rdf:Description> */
        state=RAPIER_STATE_IN_RDF;
        finished=1;
        break;

      case RAPIER_STATE_TYPED_NODE:
        /* Must be end of typedNode production element <typeName> */
        state=RAPIER_STATE_IN_RDF;
        finished=1;
        break;

      case RAPIER_STATE_SEQ:
      case RAPIER_STATE_BAG:
      case RAPIER_STATE_ALT:
      case RAPIER_STATE_CONTAINER:

        finished=1;
        break;

      case RAPIER_STATE_MEMBER:
        finished=1;
        break;

      case RAPIER_STATE_REFERENCEDITEM:
        finished=1;
        break;

      case RAPIER_STATE_INLINEITEM:
        finished=1;
        break;

      case RAPIER_STATE_PROPERTYELT:
        finished=1;
        break;

      default:
        rapier_parser_fatal_error(rdf_parser, "Unexpected parser state %d.", 
                                  state);
        finished=1;
        
    } /* end switch */

    if(state != element->state) {
      element->state=state;
      LIBRDF_DEBUG2(rapier_end_element_grammar, "moved to state %d\n", state);
    }
  
  } /* end while */

}
