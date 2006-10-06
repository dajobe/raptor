/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_libxml.c - Raptor libxml functions
 *
 * Copyright (C) 2000-2006, David Beckett http://purl.org/net/dajobe/
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


#ifdef RAPTOR_XML_LIBXML


/* prototypes */
static void raptor_libxml_warning(void* user_data, const char *msg, ...) RAPTOR_PRINTF_FORMAT(2, 3);
static void raptor_libxml_error_common(void* user_data, const char *msg, va_list args,  const char *prefix, int is_fatal) RAPTOR_PRINTF_FORMAT(2, 0);
static void raptor_libxml_error(void *context, const char *msg, ...) RAPTOR_PRINTF_FORMAT(2, 3);
static void raptor_libxml_fatal_error(void *context, const char *msg, ...) RAPTOR_PRINTF_FORMAT(2, 3);
static void raptor_libxml_generic_error(void* user_data, const char *msg, ...) RAPTOR_PRINTF_FORMAT(2, 3);
static void raptor_libxml_fatal_error(void* user_data, const char *msg, ...) RAPTOR_PRINTF_FORMAT(2, 3);



static const char* xml_warning_prefix="XML parser warning - ";
static const char* xml_error_prefix="XML parser error - ";
static const char* xml_generic_error_prefix="XML error - ";
static const char* xml_fatal_error_prefix="XML parser fatal error - ";
static const char* xml_validation_error_prefix="XML parser validation error - ";
static const char* xml_validation_warning_prefix="XML parser validation warning - ";


#ifdef RAPTOR_LIBXML_XMLSAX2INTERNALSUBSET
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
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  libxml2_internalSubset(sax2->xc, name, ExternalID, SystemID);
}


#ifdef RAPTOR_LIBXML_XMLSAXHANDLER_EXTERNALSUBSET
static void
raptor_libxml_externalSubset(void* user_data, const xmlChar *name,
                             const xmlChar *ExternalID, const xmlChar *SystemID)
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  libxml2_externalSubset(sax2->xc, name, ExternalID, SystemID);
}
#endif


static int
raptor_libxml_isStandalone (void* user_data) 
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  return libxml2_isStandalone(sax2->xc);
}


static int
raptor_libxml_hasInternalSubset (void* user_data) 
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  return libxml2_hasInternalSubset(sax2->xc);
}


static int
raptor_libxml_hasExternalSubset (void* user_data) 
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  return libxml2_hasExternalSubset(sax2->xc);
}


static xmlParserInputPtr
raptor_libxml_resolveEntity(void* user_data, 
                            const xmlChar *publicId, const xmlChar *systemId) {
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  return libxml2_resolveEntity(sax2->xc, publicId, systemId);
}


static xmlEntityPtr
raptor_libxml_getEntity(void* user_data, const xmlChar *name) {
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  return libxml2_getEntity(sax2->xc, name);
}


static xmlEntityPtr
raptor_libxml_getParameterEntity(void* user_data, const xmlChar *name) {
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  return libxml2_getParameterEntity(sax2->xc, name);
}


static void
raptor_libxml_entityDecl(void* user_data, const xmlChar *name, int type,
                         const xmlChar *publicId, const xmlChar *systemId, 
                         xmlChar *content) {
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  libxml2_entityDecl(sax2->xc, name, type, publicId, systemId, content);
}


static void
raptor_libxml_unparsedEntityDecl(void* user_data, const xmlChar *name,
                                 const xmlChar *publicId, const xmlChar *systemId,
                                 const xmlChar *notationName) {
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  libxml2_unparsedEntityDecl(sax2->xc, name, publicId, systemId, notationName);
}


static void
raptor_libxml_startDocument(void* user_data) {
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  libxml2_startDocument(sax2->xc);
}


static void
raptor_libxml_endDocument(void* user_data) {
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  xmlParserCtxtPtr xc=sax2->xc;

  libxml2_endDocument(sax2->xc);

  if(xc->myDoc) {
    xmlFreeDoc(xc->myDoc);
    xc->myDoc=NULL;
  }
}



static void
raptor_libxml_set_document_locator(void* user_data, xmlSAXLocatorPtr loc)
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  sax2->loc=loc;
}


void
raptor_libxml_update_document_locator(raptor_sax2* sax2,
                                      raptor_locator* locator)
{
  /* for storing error info */
  xmlSAXLocatorPtr loc=sax2 ? sax2->loc : NULL;
  xmlParserCtxtPtr xc= sax2 ? sax2->xc : NULL;

  if(xc && xc->inSubset)
    return;

  locator->line= -1;
  locator->column= -1;

  if(!xc)
    return;

  if(loc) {
    locator->line=loc->getLineNumber(xc);
    /* Seems to be broken */
    /* locator->column=loc->getColumnNumber(xc); */
  }

}
  

static void raptor_libxml_call_handler(void *data, raptor_message_handler handler, raptor_locator* locator, const char *message,  va_list arguments) RAPTOR_PRINTF_FORMAT(4, 0);

static void
raptor_libxml_call_handler(void *data,
                           raptor_message_handler handler,
                           raptor_locator* locator,
                           const char *message, 
                           va_list arguments)
{
  if(handler) {
    char *buffer=raptor_vsnprintf(message, arguments);
    size_t length;
    if(!buffer) {
      fprintf(stderr, "raptor_libxml_call_handler: Out of memory\n");
      return;
    }
    length=strlen(buffer);
    if(buffer[length-1]=='\n')
      buffer[length-1]='\0';
    handler(data, locator, buffer);
    RAPTOR_FREE(cstring, buffer);
  }
}


static void
raptor_libxml_warning(void* user_data, const char *msg, ...) 
{
  raptor_sax2* sax2=NULL;
  va_list args;
  int prefix_length=strlen(xml_warning_prefix);
  int length;
  char *nmsg;

  /* Work around libxml2 bug - sometimes the sax2->error
   * returns a ctx, sometimes the userdata
   */
  if(((raptor_sax2*)user_data)->magic == RAPTOR_LIBXML_MAGIC)
    sax2=(raptor_sax2*)user_data;
  else
    /* user_data is not userData */
    sax2=(raptor_sax2*)((xmlParserCtxtPtr)user_data)->userData;

  va_start(args, msg);

  raptor_libxml_update_document_locator(sax2, sax2->locator);

  length=prefix_length+strlen(msg)+1;
  nmsg=(char*)RAPTOR_MALLOC(cstring, length);
  if(nmsg) {
    strcpy(nmsg, xml_warning_prefix);
    strcpy(nmsg+prefix_length, msg);
    if(nmsg[length-2]=='\n')
      nmsg[length-2]='\0';
  }
  
  raptor_libxml_call_handler(sax2->warning_data,
                             sax2->warning_handler, sax2->locator, 
                             nmsg ? nmsg : msg, 
                             args);
  if(nmsg)
    RAPTOR_FREE(cstring,nmsg);
  va_end(args);
}


static void
raptor_libxml_error_common(void* user_data, const char *msg, va_list args, 
                           const char *prefix, int is_fatal)
{
  raptor_sax2* sax2=NULL;
  int prefix_length=strlen(prefix);
  int length;
  char *nmsg;

  if(user_data) {
    /* Work around libxml2 bug - sometimes the sax2->error
     * returns a user_data, sometimes the userdata
     */
    if(((raptor_sax2*)user_data)->magic == RAPTOR_LIBXML_MAGIC)
      sax2=(raptor_sax2*)user_data;
    else
      /* user_data is not userData */
      sax2=(raptor_sax2*)((xmlParserCtxtPtr)user_data)->userData;
  }
  
  raptor_libxml_update_document_locator(sax2, sax2->locator);

  length=prefix_length+strlen(msg)+1;
  nmsg=(char*)RAPTOR_MALLOC(cstring, length);
  if(nmsg) {
    strcpy(nmsg, prefix);
    strcpy(nmsg+prefix_length, msg);
    if(nmsg[length-1]=='\n')
      nmsg[length-1]='\0';
  }

  if(is_fatal)
    raptor_libxml_call_handler(sax2->fatal_error_data,
                               sax2->fatal_error_handler, sax2->locator, 
                               nmsg ? nmsg : msg, 
                               args);
  else
    raptor_libxml_call_handler(sax2->error_data,
                               sax2->error_handler, sax2->locator, 
                               nmsg ? nmsg : msg, 
                               args);
  
  if(nmsg)
    RAPTOR_FREE(cstring,nmsg);
}


static void
raptor_libxml_error(void* user_data, const char *msg, ...) 
{
  va_list args;

  va_start(args, msg);
  raptor_libxml_error_common(user_data, msg, args, xml_error_prefix, 0);
  va_end(args);
}



static void
raptor_libxml_generic_error(void* user_data, const char *msg, ...) 
{
  va_list args;

  va_start(args, msg);
  raptor_libxml_error_common(user_data, msg, args, xml_generic_error_prefix, 0);
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
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  int prefix_length=strlen(xml_validation_warning_prefix);
  int length;
  char *nmsg;

  va_start(args, msg);

  raptor_libxml_update_document_locator(sax2, sax2->locator);

  length=prefix_length+strlen(msg)+1;
  nmsg=(char*)RAPTOR_MALLOC(cstring, length);
  if(nmsg) {
    strcpy(nmsg, xml_validation_warning_prefix);
    strcpy(nmsg+prefix_length, msg);
    if(nmsg[length-2]=='\n')
      nmsg[length-2]='\0';
  }

  raptor_libxml_call_handler(sax2->warning_data,
                             sax2->warning_handler, sax2->locator, 
                             nmsg ? nmsg : msg, 
                             args);
  if(nmsg)
    RAPTOR_FREE(cstring,nmsg);
  va_end(args);
}


void
raptor_libxml_init(raptor_sax2* sax2, raptor_uri *base_uri)
{
  xmlSAXHandler *sax=&sax2->sax;

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
  sax->warning=(warningSAXFunc)raptor_libxml_warning;
  sax->error=(errorSAXFunc)raptor_libxml_error;
  sax->fatalError=(fatalErrorSAXFunc)raptor_libxml_fatal_error;

#ifdef RAPTOR_LIBXML_XMLSAXHANDLER_EXTERNALSUBSET
  sax->externalSubset = raptor_libxml_externalSubset;
#endif

#ifdef RAPTOR_LIBXML_XMLSAXHANDLER_INITIALIZED
  sax->initialized = 1;
#endif
}


void
raptor_libxml_init_sax_error_handlers(xmlSAXHandler *sax) {
  sax->warning=(warningSAXFunc)raptor_libxml_warning;
  sax->error=(errorSAXFunc)raptor_libxml_error;
  sax->fatalError=(fatalErrorSAXFunc)raptor_libxml_fatal_error;

#ifdef RAPTOR_LIBXML_XMLSAXHANDLER_INITIALIZED
  sax->initialized = 1;
#endif
}


void
raptor_libxml_init_generic_error_handlers(raptor_parser *rdf_parser)
{
  xmlGenericError = (xmlGenericErrorFunc)raptor_libxml_generic_error;
  xmlGenericErrorContext = rdf_parser;
}


void
raptor_libxml_free(xmlParserCtxtPtr xc) {
  libxml2_endDocument(xc);
  xmlFreeParserCtxt(xc);
}

/* end if RAPTOR_XML_LIBXML */
#endif
