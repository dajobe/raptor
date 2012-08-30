/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_libxml.c - Raptor libxml functions
 *
 * Copyright (C) 2000-2009, David Beckett http://www.dajobe.org/
 * Copyright (C) 2000-2004, University of Bristol, UK http://www.bristol.ac.uk/
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
#include "raptor2.h"
#include "raptor_internal.h"


#ifdef RAPTOR_XML_LIBXML


/* prototypes */
static void raptor_libxml_warning(void* user_data, const char *msg, ...) RAPTOR_PRINTF_FORMAT(2, 3);
static void raptor_libxml_error_common(void* user_data, const char *msg, va_list args,  const char *prefix, int is_fatal) RAPTOR_PRINTF_FORMAT(2, 0);
static void raptor_libxml_error(void *context, const char *msg, ...) RAPTOR_PRINTF_FORMAT(2, 3);
static void raptor_libxml_fatal_error(void *context, const char *msg, ...) RAPTOR_PRINTF_FORMAT(2, 3);

static void raptor_libxml_xmlStructuredError_handler_global(void *user_data, xmlErrorPtr err);
static void raptor_libxml_xmlStructuredError_handler_parsing(void *user_data, xmlErrorPtr err);



static const char* const xml_warning_prefix="XML parser warning - ";
static const char* const xml_error_prefix="XML parser error - ";
static const char* const xml_generic_error_prefix="XML error - ";
static const char* const xml_fatal_error_prefix="XML parser fatal error - ";
static const char* const xml_validation_error_prefix="XML parser validation error - ";
static const char* const xml_validation_warning_prefix="XML parser validation warning - ";


#ifdef HAVE_XMLSAX2INTERNALSUBSET
/* SAX2 - 2.6.0 or later */
#define libxml2_internalSubset xmlSAX2InternalSubset
#define libxml2_externalSubset xmlSAX2ExternalSubset
#define libxml2_isStandalone xmlSAX2IsStandalone
#define libxml2_hasInternalSubset xmlSAX2HasInternalSubset
#define libxml2_hasExternalSubset xmlSAX2HasExternalSubset
#define libxml2_resolveEntity xmlSAX2ResolveEntity
#define libxml2_getEntity xmlSAX2GetEntity
#define libxml2_getParameterEntity xmlSAX2GetParameterEntity
#define libxml2_entityDecl xmlSAX2EntityDecl
#define libxml2_unparsedEntityDecl xmlSAX2UnparsedEntityDecl
#define libxml2_startDocument xmlSAX2StartDocument
#define libxml2_endDocument xmlSAX2EndDocument
#else
/* SAX1 - before libxml2 2.6.0 */
#define libxml2_internalSubset internalSubset
#define libxml2_externalSubset externalSubset
#define libxml2_isStandalone isStandalone
#define libxml2_hasInternalSubset hasInternalSubset
#define libxml2_hasExternalSubset hasExternalSubset
#define libxml2_resolveEntity resolveEntity
#define libxml2_getEntity getEntity
#define libxml2_getParameterEntity getParameterEntity
#define libxml2_entityDecl entityDecl
#define libxml2_unparsedEntityDecl unparsedEntityDecl
#define libxml2_startDocument startDocument
#define libxml2_endDocument endDocument
#endif


static void
raptor_libxml_internalSubset(void* user_data, const xmlChar *name,
                             const xmlChar *ExternalID, const xmlChar *SystemID) {
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  libxml2_internalSubset(sax2->xc, name, ExternalID, SystemID);
}


#ifdef RAPTOR_LIBXML_XMLSAXHANDLER_EXTERNALSUBSET
static void
raptor_libxml_externalSubset(void* user_data, const xmlChar *name,
                             const xmlChar *ExternalID, const xmlChar *SystemID)
{
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  libxml2_externalSubset(sax2->xc, name, ExternalID, SystemID);
}
#endif


static int
raptor_libxml_isStandalone (void* user_data) 
{
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  return libxml2_isStandalone(sax2->xc);
}


static int
raptor_libxml_hasInternalSubset (void* user_data) 
{
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  return libxml2_hasInternalSubset(sax2->xc);
}


static int
raptor_libxml_hasExternalSubset (void* user_data) 
{
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  return libxml2_hasExternalSubset(sax2->xc);
}


static xmlParserInputPtr
raptor_libxml_resolveEntity(void* user_data, 
                            const xmlChar *publicId, const xmlChar *systemId)
{
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  xmlParserCtxtPtr ctxt = sax2->xc;
  const unsigned char *uri_string = NULL;
  xmlParserInputPtr entity_input = NULL;
  int load_entity = 0;

  if(ctxt->input)
    uri_string = RAPTOR_GOOD_CAST(const unsigned char *, ctxt->input->filename);

  if(!uri_string)
    uri_string = RAPTOR_GOOD_CAST(const unsigned char *, ctxt->directory);

  load_entity = RAPTOR_OPTIONS_GET_NUMERIC(sax2, RAPTOR_OPTION_LOAD_EXTERNAL_ENTITIES);
  if(load_entity)
    load_entity = raptor_sax2_check_load_uri_string(sax2, uri_string);

  if(load_entity) {
    entity_input = xmlLoadExternalEntity(RAPTOR_GOOD_CAST(const char*, uri_string),
                                         RAPTOR_GOOD_CAST(const char*, publicId),
                                         ctxt);
  } else {
    RAPTOR_DEBUG4("Not loading entity URI %s by policy for publicId '%s' systemId '%s'\n", uri_string, publicId, systemId);
  }
  
  return entity_input;
}


static xmlEntityPtr
raptor_libxml_getEntity(void* user_data, const xmlChar *name)
{
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  xmlParserCtxtPtr xc = sax2->xc;
  xmlEntityPtr ret = NULL;

  if(!xc)
    return NULL;

  if(!xc->inSubset) {
    /* looks for hardcoded set of entity names - lt, gt etc. */
    ret = xmlGetPredefinedEntity(name);
    if(ret) {
      RAPTOR_DEBUG2("Entity '%s' found in predefined set\n", name);
      return ret;
    }
  }

  /* This section uses xmlGetDocEntity which looks for entities in
   * memory only, never from a file or URI 
   */
  if(xc->myDoc && (xc->myDoc->standalone == 1)) {
    RAPTOR_DEBUG2("Entity '%s' document is standalone\n", name);
    /* Document is standalone: no entities are required to interpret doc */
    if(xc->inSubset == 2) {
      xc->myDoc->standalone = 0;
      ret = xmlGetDocEntity(xc->myDoc, name);
      xc->myDoc->standalone = 1;
    } else {
      ret = xmlGetDocEntity(xc->myDoc, name);
      if(!ret) {
        xc->myDoc->standalone = 0;
        ret = xmlGetDocEntity(xc->myDoc, name);
        xc->myDoc->standalone = 1;
      }
    }
  } else {
    ret = xmlGetDocEntity(xc->myDoc, name);
  }

  if(ret && !ret->children &&
    (ret->etype == XML_EXTERNAL_GENERAL_PARSED_ENTITY)) {
    /* Entity is an external general parsed entity. It may be in a
     * catalog file, user file or user URI
     */
    int val = 0;
    xmlNodePtr children;
    int load_entity = 0;

    load_entity = RAPTOR_OPTIONS_GET_NUMERIC(sax2, RAPTOR_OPTION_LOAD_EXTERNAL_ENTITIES);
    if(load_entity)
      load_entity = raptor_sax2_check_load_uri_string(sax2, ret->URI);

    if(!load_entity) {
      RAPTOR_DEBUG2("Not getting entity URI %s by policy\n", ret->URI);
      children = xmlNewText((const xmlChar*)"");
    } else {
      /* Disable SAX2 handlers so that the SAX2 events do not all get
       * sent to callbacks during dealing with the entity parsing.
       */
      sax2->enabled = 0;
      val = xmlParseCtxtExternalEntity(xc, ret->URI, ret->ExternalID, &children);
      sax2->enabled = 1;
    }
    
    if(!val) {
      xmlAddChildList((xmlNodePtr)ret, children);
    } else {
      xc->validate = 0;
      return NULL;
    }
    
    ret->owner = 1;

#if LIBXML_VERSION >= 20627
    /* Checked field was released in 2.6.27 on 2006-10-25
     * http://git.gnome.org/browse/libxml2/commit/?id=a37a6ad91a61d168ecc4b29263def3363fff4da6
     *
     */

    /* Mark this entity as having been checked - never do this again */
    if(!ret->checked)
      ret->checked = 1;
#endif
  }

  return ret;
}


static xmlEntityPtr
raptor_libxml_getParameterEntity(void* user_data, const xmlChar *name) {
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  return libxml2_getParameterEntity(sax2->xc, name);
}


static void
raptor_libxml_entityDecl(void* user_data, const xmlChar *name, int type,
                         const xmlChar *publicId, const xmlChar *systemId, 
                         xmlChar *content) {
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  libxml2_entityDecl(sax2->xc, name, type, publicId, systemId, content);
}


static void
raptor_libxml_unparsedEntityDecl(void* user_data, const xmlChar *name,
                                 const xmlChar *publicId, const xmlChar *systemId,
                                 const xmlChar *notationName) {
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  libxml2_unparsedEntityDecl(sax2->xc, name, publicId, systemId, notationName);
}


static void
raptor_libxml_startDocument(void* user_data) {
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  libxml2_startDocument(sax2->xc);
}


static void
raptor_libxml_endDocument(void* user_data) {
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  xmlParserCtxtPtr xc = sax2->xc;

  libxml2_endDocument(sax2->xc);

  if(xc->myDoc) {
    xmlFreeDoc(xc->myDoc);
    xc->myDoc = NULL;
  }
}



static void
raptor_libxml_set_document_locator(void* user_data, xmlSAXLocatorPtr loc)
{
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  sax2->loc = loc;
}


void
raptor_libxml_update_document_locator(raptor_sax2* sax2,
                                      raptor_locator* locator)
{
  /* for storing error info */
  xmlSAXLocatorPtr loc = sax2 ? sax2->loc : NULL;
  xmlParserCtxtPtr xc= sax2 ? sax2->xc : NULL;

  if(xc && xc->inSubset)
    return;

  if(!locator) 
    return;
  
  locator->line= -1;
  locator->column= -1;

  if(!xc)
    return;

  if(loc) {
    locator->line = loc->getLineNumber(xc);
    /* Seems to be broken */
    /* locator->column = loc->getColumnNumber(xc); */
  }

}
  

static void
raptor_libxml_warning(void* user_data, const char *msg, ...) 
{
  raptor_sax2* sax2 = NULL;
  va_list args;
  int prefix_length = RAPTOR_BAD_CAST(int, strlen(xml_warning_prefix));
  int length;
  char *nmsg;
  int msg_len;

  /* Work around libxml2 bug - sometimes the sax2->error
   * returns a ctx, sometimes the userdata
   */
  if(((raptor_sax2*)user_data)->magic == RAPTOR_LIBXML_MAGIC)
    sax2 = (raptor_sax2*)user_data;
  else
    /* user_data is not userData */
    sax2 = (raptor_sax2*)((xmlParserCtxtPtr)user_data)->userData;

  va_start(args, msg);

  raptor_libxml_update_document_locator(sax2, sax2->locator);

  msg_len = RAPTOR_BAD_CAST(int, strlen(msg));
  length = prefix_length + msg_len + 1;
  nmsg = RAPTOR_MALLOC(char*, length);
  if(nmsg) {
    memcpy(nmsg, xml_warning_prefix, prefix_length); /* Do not copy NUL */
    memcpy(nmsg + prefix_length, msg, msg_len + 1); /* Copy NUL */
    if(nmsg[length-2] == '\n')
      nmsg[length-2]='\0';
  }
  
  raptor_log_error_varargs(sax2->world,
                           RAPTOR_LOG_LEVEL_WARN,
                           sax2->locator, 
                           nmsg ? nmsg : msg, 
                           args);
  if(nmsg)
    RAPTOR_FREE(char*, nmsg);
  va_end(args);
}


static void
raptor_libxml_error_common(void* user_data, const char *msg, va_list args, 
                           const char *prefix, int is_fatal)
{
  raptor_sax2* sax2 = NULL;
  int prefix_length = RAPTOR_BAD_CAST(int, strlen(prefix));
  int length;
  char *nmsg;
  int msg_len;
  raptor_world* world = NULL;
  raptor_locator* locator = NULL;

  if(user_data) {
    /* Work around libxml2 bug - sometimes the sax2->error
     * returns a user_data, sometimes the userdata
     */
    if(((raptor_sax2*)user_data)->magic == RAPTOR_LIBXML_MAGIC)
      sax2 = (raptor_sax2*)user_data;
    else
      /* user_data is not userData */
      sax2 = (raptor_sax2*)((xmlParserCtxtPtr)user_data)->userData;
  }

  if(sax2) {
    world = sax2->world;
    locator = sax2->locator;
    
    if(locator)
      raptor_libxml_update_document_locator(sax2, sax2->locator);
  }

  msg_len = RAPTOR_BAD_CAST(int, strlen(msg));
  length = prefix_length + msg_len + 1;
  nmsg = RAPTOR_MALLOC(char*, length);
  if(nmsg) {
    memcpy(nmsg, prefix, prefix_length); /* Do not copy NUL */
    memcpy(nmsg + prefix_length, msg, msg_len + 1); /* Copy NUL */
    if(nmsg[length-1] == '\n')
      nmsg[length-1]='\0';
  }

  if(is_fatal)
    raptor_log_error_varargs(world,
                             RAPTOR_LOG_LEVEL_FATAL,
                             locator, 
                             nmsg ? nmsg : msg, 
                             args);
  else
    raptor_log_error_varargs(world,
                             RAPTOR_LOG_LEVEL_ERROR,
                             locator, 
                             nmsg ? nmsg : msg, 
                             args);
  
  if(nmsg)
    RAPTOR_FREE(char*, nmsg);
}


static void
raptor_libxml_error(void* user_data, const char *msg, ...) 
{
  va_list args;

  va_start(args, msg);
  raptor_libxml_error_common(user_data, msg, args, xml_error_prefix, 0);
  va_end(args);
}



void
raptor_libxml_generic_error(void* user_data, const char *msg, ...) 
{
  raptor_world* world = (raptor_world*)user_data;
  va_list args;
  const char* prefix = xml_generic_error_prefix;
  int prefix_length = RAPTOR_BAD_CAST(int, strlen(prefix));
  int length;
  char *nmsg;
  int msg_len;
  
  va_start(args, msg);

  msg_len = RAPTOR_BAD_CAST(int, strlen(msg));
  length = prefix_length + msg_len + 1;
  nmsg = RAPTOR_MALLOC(char*, length);
  if(nmsg) {
    memcpy(nmsg, prefix, prefix_length); /* Do not copy NUL */
    memcpy(nmsg + prefix_length, msg, msg_len + 1); /* Copy NUL */
    if(nmsg[length-1] == '\n')
      nmsg[length-1]='\0';
  }

  raptor_log_error_varargs(world, RAPTOR_LOG_LEVEL_ERROR,
                           /* locator */ NULL,
                           nmsg ? nmsg : msg, 
                           args);
  
  if(nmsg)
    RAPTOR_FREE(char*, nmsg);

  va_end(args);
}


static void
raptor_libxml_fatal_error(void* user_data, const char *msg, ...) 
{
  va_list args;

  va_start(args, msg);
  raptor_libxml_error_common(user_data, msg, args, xml_fatal_error_prefix, 1);
  va_end(args);
}


void
raptor_libxml_validation_error(void* user_data, const char *msg, ...) 
{
  va_list args;

  va_start(args, msg);
  raptor_libxml_error_common(user_data, msg, args, 
                             xml_validation_error_prefix, 1);
  va_end(args);
}


void
raptor_libxml_validation_warning(void* user_data, const char *msg, ...) 
{
  va_list args;
  raptor_sax2* sax2 = (raptor_sax2*)user_data;
  int prefix_length = RAPTOR_GOOD_CAST(int, strlen(xml_validation_warning_prefix));
  int length;
  char *nmsg;
  int msg_len;
  
  va_start(args, msg);

  raptor_libxml_update_document_locator(sax2, sax2->locator);

  msg_len = RAPTOR_BAD_CAST(int, strlen(msg));
  length = prefix_length + msg_len + 1;
  nmsg = RAPTOR_MALLOC(char*, length);
  if(nmsg) {
    memcpy(nmsg, xml_validation_warning_prefix, prefix_length); /* Do not copy NUL */
    memcpy(nmsg + prefix_length, msg, msg_len + 1); /* Copy NUL */
    if(nmsg[length-2] == '\n')
      nmsg[length-2]='\0';
  }

  raptor_log_error_varargs(sax2->world,
                           RAPTOR_LOG_LEVEL_WARN,
                           sax2->locator, 
                           nmsg ? nmsg : msg, 
                           args);
  if(nmsg)
    RAPTOR_FREE(char*, nmsg);
  va_end(args);
}


/*
 * Initialise libxml for a particular SAX2 setup
*/
void
raptor_libxml_sax_init(raptor_sax2* sax2)
{
  xmlSAXHandler *sax = &sax2->sax;

  sax->internalSubset = raptor_libxml_internalSubset;
  sax->isStandalone = raptor_libxml_isStandalone;
  sax->hasInternalSubset = raptor_libxml_hasInternalSubset;
  sax->hasExternalSubset = raptor_libxml_hasExternalSubset;
  sax->resolveEntity = raptor_libxml_resolveEntity;
  sax->getEntity = raptor_libxml_getEntity;
  sax->getParameterEntity = raptor_libxml_getParameterEntity;
  sax->entityDecl = raptor_libxml_entityDecl;
  sax->attributeDecl = NULL; /* attributeDecl */
  sax->elementDecl = NULL; /* elementDecl */
  sax->notationDecl = NULL; /* notationDecl */
  sax->unparsedEntityDecl = raptor_libxml_unparsedEntityDecl;
  sax->setDocumentLocator = raptor_libxml_set_document_locator;
  sax->startDocument = raptor_libxml_startDocument;
  sax->endDocument = raptor_libxml_endDocument;
  sax->startElement= raptor_sax2_start_element;
  sax->endElement= raptor_sax2_end_element;
  sax->reference = NULL;     /* reference */
  sax->characters= raptor_sax2_characters;
  sax->cdataBlock= raptor_sax2_cdata; /* like <![CDATA[...]> */
  sax->ignorableWhitespace= raptor_sax2_cdata;
  sax->processingInstruction = NULL; /* processingInstruction */
  sax->comment = raptor_sax2_comment;      /* comment */
  sax->warning = (warningSAXFunc)raptor_libxml_warning;
  sax->error = (errorSAXFunc)raptor_libxml_error;
  sax->fatalError = (fatalErrorSAXFunc)raptor_libxml_fatal_error;
  sax->serror = (xmlStructuredErrorFunc)raptor_libxml_xmlStructuredError_handler_parsing;

#ifdef RAPTOR_LIBXML_XMLSAXHANDLER_EXTERNALSUBSET
  sax->externalSubset = raptor_libxml_externalSubset;
#endif

#ifdef RAPTOR_LIBXML_XMLSAXHANDLER_INITIALIZED
  sax->initialized = 1;
#endif
}


void
raptor_libxml_free(xmlParserCtxtPtr xc) {
  libxml2_endDocument(xc);

  if(xc->myDoc) {
    xmlFreeDoc(xc->myDoc);
    xc->myDoc = NULL;
  }

  xmlFreeParserCtxt(xc);
}


int
raptor_libxml_init(raptor_world* world)
{
  xmlInitParser();

  if(world->libxml_flags & RAPTOR_WORLD_FLAG_LIBXML_STRUCTURED_ERROR_SAVE) {
    world->libxml_saved_structured_error_context = xmlGenericErrorContext;
    world->libxml_saved_structured_error_handler = xmlStructuredError;
    /* sets xmlGenericErrorContext and xmlStructuredError */
    xmlSetStructuredErrorFunc(world, 
                              (xmlStructuredErrorFunc)raptor_libxml_xmlStructuredError_handler_global);
  }
  
  if(world->libxml_flags & RAPTOR_WORLD_FLAG_LIBXML_GENERIC_ERROR_SAVE) {
    world->libxml_saved_generic_error_context = xmlGenericErrorContext;
    world->libxml_saved_generic_error_handler = xmlGenericError;
    /* sets xmlGenericErrorContext and xmlGenericError */
    xmlSetGenericErrorFunc(world, 
                           (xmlGenericErrorFunc)raptor_libxml_generic_error);
  }

  return 0;
}


void
raptor_libxml_finish(raptor_world* world)
{
  if(world->libxml_flags & RAPTOR_WORLD_FLAG_LIBXML_STRUCTURED_ERROR_SAVE)
    xmlSetStructuredErrorFunc(world->libxml_saved_structured_error_context,
                              world->libxml_saved_structured_error_handler);

  if(world->libxml_flags & RAPTOR_WORLD_FLAG_LIBXML_GENERIC_ERROR_SAVE)
    xmlSetGenericErrorFunc(world->libxml_saved_generic_error_context,
                           world->libxml_saved_generic_error_handler);

  xmlCleanupParser();
}


#if LIBXML_VERSION >= 20632
#define XML_LAST_DL XML_FROM_SCHEMATRONV
#else
#if LIBXML_VERSION >= 20621
#define XML_LAST_DL XML_FROM_I18N
#else
#if LIBXML_VERSION >= 20617
#define XML_LAST_DL XML_FROM_WRITER
#else
#if LIBXML_VERSION >= 20616
#define XML_LAST_DL XML_FROM_CHECK
#else
#if LIBXML_VERSION >= 20615
#define XML_LAST_DL XML_FROM_VALID
#else
#define XML_LAST_DL XML_FROM_XSLT
#endif
#endif
#endif
#endif
#endif

/* All other symbols not specifically below noted were added during
 * the period 2-10 October 2003 which is before the minimum libxml2
 * version 2.6.8 release date of Mar 23 2004.
 *
 * When the minimum libxml2 version goes up, the #ifdefs for
 * older versions can be removed.
 */
static const char* const raptor_libxml_domain_labels[XML_LAST_DL+2]= {
  NULL,                /* XML_FROM_NONE */
  "parser",            /* XML_FROM_PARSER */
  "tree",              /* XML_FROM_TREE */
  "namespace",         /* XML_FROM_NAMESPACE */
  "validity",          /* XML_FROM_DTD */
  "HTML parser",       /* XML_FROM_HTML */
  "memory",            /* XML_FROM_MEMORY */
  "output",            /* XML_FROM_OUTPUT */
  "I/O" ,              /* XML_FROM_IO */
  "FTP",               /* XML_FROM_FTP */
#if LIBXML_VERSION >= 20618
  /* 2005-02-13 - v2.6.18 */
  "HTTP",              /* XML_FROM_HTTP */
#endif
  "XInclude",          /* XML_FROM_XINCLUDE */
  "XPath",             /* XML_FROM_XPATH */
  "parser",            /* XML_FROM_XPOINTER */
  "regexp",            /* XML_FROM_REGEXP */
  "Schemas datatype",  /* XML_FROM_DATATYPE */
  "Schemas parser",    /* XML_FROM_SCHEMASP */
  "Schemas validity",  /* XML_FROM_SCHEMASV */
  "Relax-NG parser",   /* XML_FROM_RELAXNGP */
  "Relax-NG validity", /* XML_FROM_RELAXNGV */
  "Catalog",           /* XML_FROM_CATALOG */
  "C14",               /* XML_FROM_C14N */
  "XSLT",              /* XML_FROM_XSLT */
#if LIBXML_VERSION >= 20615
  /* 2004-10-07 - v2.6.15 */
  "validity",          /* XML_FROM_VALID */
#endif
#if LIBXML_VERSION >= 20616
  /* 2004-11-04 - v2.6.16 */
  "checking",          /* XML_FROM_CHECK */
#endif
#if LIBXML_VERSION >= 20617
  /* 2005-01-04 - v2.6.17 */
  "writer",            /* XML_FROM_WRITER */
#endif
#if LIBXML_VERSION >= 20621
  /* 2005-08-24 - v2.6.21 */
  "module",            /* XML_FROM_MODULE */
  "encoding",          /* XML_FROM_I18N */
#endif
#if LIBXML_VERSION >= 20632
  /* 2008-04-08 - v2.6.32 */
  "schematronv",        /* XML_FROM_SCHEMATRONV */
#endif
  NULL
};
  

static void 
raptor_libxml_xmlStructuredError_handler_common(raptor_world *world,
                                                raptor_locator *locator,
                                                xmlErrorPtr err)
{
  raptor_stringbuffer* sb;
  char *nmsg;
  raptor_log_level level = RAPTOR_LOG_LEVEL_ERROR;

  if(err == NULL || err->code == XML_ERR_OK || err->level == XML_ERR_NONE)
    return;

  /* Do not warn about things with no location */
  if(err->level == XML_ERR_WARNING && !err->file)
    return;

  /* XML fatal errors never cause an abort */
  if(err->level == XML_ERR_FATAL)
    err->level = XML_ERR_ERROR;
  

  sb = raptor_new_stringbuffer();
  if(err->domain != XML_FROM_HTML)
    raptor_stringbuffer_append_counted_string(sb, (const unsigned char*)"XML ",
                                              4, 1);
  
  if(err->domain != XML_FROM_NONE && err->domain < XML_LAST_DL) {
    const unsigned char* label;
    label = (const unsigned char*)raptor_libxml_domain_labels[(int)err->domain];
    raptor_stringbuffer_append_string(sb, label, 1);
    raptor_stringbuffer_append_counted_string(sb, 
                                              (const unsigned char*)" ", 1, 1);
  }
  
  if(err->level == XML_ERR_WARNING)
    raptor_stringbuffer_append_counted_string(sb, 
                                              (const unsigned char*)"warning: ", 
                                              9, 1);
  else /*  XML_ERR_ERROR or XML_ERR_FATAL */
    raptor_stringbuffer_append_counted_string(sb, (const unsigned char*)"error: ", 
                                              7, 1);
  
  if(err->message) {
    unsigned char* msg;
    size_t len;
    msg = (unsigned char*)err->message;
    len= strlen((const char*)msg);
    if(len && msg[len-1] == '\n')
      msg[--len]='\0';
    
    raptor_stringbuffer_append_counted_string(sb, msg, len, 1);
  }

#if LIBXML_VERSION >= 20618
  /* 2005-02-13 - v2.6.18 */

  /* str1 has the detailed HTTP error */
  if(err->domain == XML_FROM_HTTP && err->str1) {
    unsigned char* msg;
    size_t len;
    msg = (unsigned char*)err->str1;
    len= strlen((const char*)msg);
    if(len && msg[len-1] == '\n')
      msg[--len]='\0';
    
    raptor_stringbuffer_append_counted_string(sb, (const unsigned char*)" - ",
                                              3, 1);
    raptor_stringbuffer_append_counted_string(sb, msg, len, 1);
  }
#endif
  
  /* When err->domain == XML_FROM_XPATH then err->int1 is
   * the offset into err->str1, the line with the error
   */
  if(err->domain == XML_FROM_XPATH && err->str1) {
    raptor_stringbuffer_append_counted_string(sb, (const unsigned char*)" in ",
                                              4, 1);
    raptor_stringbuffer_append_string(sb, (const unsigned char*)err->str1, 1);
  }

  nmsg = (char*)raptor_stringbuffer_as_string(sb);
  if(err->level == XML_ERR_FATAL)
    level = RAPTOR_LOG_LEVEL_FATAL;
  else if(err->level == XML_ERR_ERROR)
    level = RAPTOR_LOG_LEVEL_ERROR;
  else
    level = RAPTOR_LOG_LEVEL_WARN;

  raptor_log_error(world, level, locator, nmsg);
  
  raptor_free_stringbuffer(sb);
}


/* user_data is a raptor_world* */
static void 
raptor_libxml_xmlStructuredError_handler_global(void *user_data,
                                                xmlErrorPtr err)
{
  raptor_world *world = NULL;
  
  /* user_data may point to a raptor_world* */
  if(user_data) {
    world = (raptor_world*)user_data;
    if(world->magic != RAPTOR2_WORLD_MAGIC)
      world = NULL;
  }

  raptor_libxml_xmlStructuredError_handler_common(world, NULL, err);
}


/* user_data may be a raptor_sax2; err->ctxt->userData may point to a
 * raptor_sax2* */
static void 
raptor_libxml_xmlStructuredError_handler_parsing(void *user_data,
                                                 xmlErrorPtr err)
{
  raptor_sax2* sax2 = NULL;

  /* user_data may point to a raptor_sax2* */
  if(user_data) {
    sax2 = (raptor_sax2*)user_data;
    if(sax2->magic != RAPTOR_LIBXML_MAGIC)
      sax2 = NULL;
  }
  
  /* err->ctxt->userData may point to a raptor_sax2* */
  if(err && err->ctxt) {
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr)err->ctxt;
    if(ctxt->userData) {
      sax2 = (raptor_sax2*)ctxt->userData;
      if(sax2->magic != RAPTOR_LIBXML_MAGIC)
        sax2 = NULL;
    }
  }

  if(sax2)
    raptor_libxml_xmlStructuredError_handler_common(sax2->world, sax2->locator,
                                                    err);
  else
    raptor_libxml_xmlStructuredError_handler_common(NULL, NULL, err);
}


/* end if RAPTOR_XML_LIBXML */
#endif
