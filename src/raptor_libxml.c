/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_libxml.c - Raptor libxml functions
 *
 * $Id$
 *
 * Copyright (C) 2000-2002 David Beckett - http://purl.org/net/dajobe/
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


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
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
static void raptor_libxml_warning(void *context, const char *msg, ...);
static void raptor_libxml_error(void *context, const char *msg, ...);
static void raptor_libxml_fatal_error(void *context, const char *msg, ...);


static const char* xml_warning_prefix="XML parser warning - ";
static const char* xml_error_prefix="XML parser error - ";
static const char* xml_fatal_error_prefix="XML parser fatal error - ";
static const char* xml_validation_error_prefix="XML parser validation error - ";
static const char* xml_validation_warning_prefix="XML parser validation warning - ";


#ifdef HAVE_LIBXML_SAX2_H
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
raptor_libxml_internalSubset(void *ctx, const xmlChar *name,
                             const xmlChar *ExternalID, const xmlChar *SystemID) {
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  libxml2_internalSubset(raptor_get_libxml_context(rdf_parser), name, ExternalID, SystemID);
}


#ifdef RAPTOR_LIBXML_XMLSAXHANDLER_EXTERNALSUBSET
static void
raptor_libxml_externalSubset(void *ctx, const xmlChar *name,
                             const xmlChar *ExternalID, const xmlChar *SystemID)
{
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  libxml2_externalSubset(raptor_get_libxml_context(rdf_parser), name, ExternalID, SystemID);
}
#endif


static int
raptor_libxml_isStandalone (void *ctx) 
{
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  return libxml2_isStandalone(raptor_get_libxml_context(rdf_parser));
}


static int
raptor_libxml_hasInternalSubset (void *ctx) 
{
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  return libxml2_hasInternalSubset(raptor_get_libxml_context(rdf_parser));
}


static int
raptor_libxml_hasExternalSubset (void *ctx) 
{
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  return libxml2_hasExternalSubset(raptor_get_libxml_context(rdf_parser));
}


static xmlParserInputPtr
raptor_libxml_resolveEntity(void *ctx, 
                            const xmlChar *publicId, const xmlChar *systemId) {
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  return libxml2_resolveEntity(raptor_get_libxml_context(rdf_parser), publicId, systemId);
}


static xmlEntityPtr
raptor_libxml_getEntity(void *ctx, const xmlChar *name) {
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  return libxml2_getEntity(raptor_get_libxml_context(rdf_parser), name);
}


static xmlEntityPtr
raptor_libxml_getParameterEntity(void *ctx, const xmlChar *name) {
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  return libxml2_getParameterEntity(raptor_get_libxml_context(rdf_parser), name);
}


static void
raptor_libxml_entityDecl(void *ctx, const xmlChar *name, int type,
                         const xmlChar *publicId, const xmlChar *systemId, 
                         xmlChar *content) {
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  libxml2_entityDecl(raptor_get_libxml_context(rdf_parser), name, type, publicId, systemId, content);
}


static void
raptor_libxml_unparsedEntityDecl(void *ctx, const xmlChar *name,
                                 const xmlChar *publicId, const xmlChar *systemId,
                                 const xmlChar *notationName) {
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  libxml2_unparsedEntityDecl(raptor_get_libxml_context(rdf_parser), name, publicId, systemId, notationName);
}


static void
raptor_libxml_startDocument(void *ctx) {
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  libxml2_startDocument(raptor_get_libxml_context(rdf_parser));
}


static void
raptor_libxml_endDocument(void *ctx) {
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  xmlParserCtxtPtr xc=raptor_get_libxml_context(rdf_parser);

  libxml2_endDocument(raptor_get_libxml_context(rdf_parser));

  if(xc->myDoc) {
    xmlFreeDoc(xc->myDoc);
    xc->myDoc=NULL;
  }
}



static void
raptor_libxml_set_document_locator (void *ctx, xmlSAXLocatorPtr loc) {
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  raptor_set_libxml_document_locator(rdf_parser, loc);
}

void
raptor_libxml_update_document_locator (raptor_parser *rdf_parser) {
  /* for storing error info */
  raptor_locator *locator=raptor_get_locator(rdf_parser);
  xmlSAXLocatorPtr loc=raptor_get_libxml_document_locator(rdf_parser);
  xmlParserCtxtPtr xc=raptor_get_libxml_context(rdf_parser);

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
  

static void
raptor_libxml_warning(void *ctx, const char *msg, ...) 
{
  raptor_parser* rdf_parser;
  va_list args;
  int prefix_length=strlen(xml_warning_prefix);
  int length;
  char *nmsg;

  /* Work around libxml2 bug - sometimes the sax2->error
   * returns a ctx, sometimes the userdata
   */
  if(((raptor_parser*)ctx)->magic == RAPTOR_LIBXML_MAGIC)
    rdf_parser=(raptor_parser*)ctx;
  else
    /* ctx is not userData */
    rdf_parser=(raptor_parser*)((xmlParserCtxtPtr)ctx)->userData;

  va_start(args, msg);

  raptor_libxml_update_document_locator(rdf_parser);
  
  length=prefix_length+strlen(msg)+1;
  nmsg=(char*)RAPTOR_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    raptor_parser_warning_varargs(rdf_parser, msg, args);
  } else {
    strcpy(nmsg, xml_warning_prefix);
    strcpy(nmsg+prefix_length, msg);
    if(nmsg[length-2]=='\n')
      nmsg[length-2]='\0';
    raptor_parser_warning_varargs(rdf_parser, nmsg, args);
    RAPTOR_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
raptor_libxml_error(void *ctx, const char *msg, ...) 
{
  raptor_parser* rdf_parser;
  va_list args;
  int prefix_length=strlen(xml_error_prefix);
  int length;
  char *nmsg;

  /* Work around libxml2 bug - sometimes the sax2->error
   * returns a ctx, sometimes the userdata
   */
  if(((raptor_parser*)ctx)->magic == RAPTOR_LIBXML_MAGIC)
    rdf_parser=(raptor_parser*)ctx;
  else
    /* ctx is not userData */
    rdf_parser=(raptor_parser*)((xmlParserCtxtPtr)ctx)->userData;

  va_start(args, msg);

  raptor_libxml_update_document_locator(rdf_parser);

  length=prefix_length+strlen(msg)+1;
  nmsg=(char*)RAPTOR_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    raptor_parser_error_varargs(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_error_prefix);
    strcpy(nmsg+prefix_length, msg);
    if(nmsg[length-2]=='\n')
      nmsg[length-2]='\0';
    raptor_parser_error_varargs(rdf_parser, nmsg, args);
    RAPTOR_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
raptor_libxml_fatal_error(void *ctx, const char *msg, ...) 
{
  va_list args;
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  int prefix_length=strlen(xml_fatal_error_prefix);
  int length;
  char *nmsg;

  va_start(args, msg);

  raptor_libxml_update_document_locator(rdf_parser);

  length=prefix_length+strlen(msg)+1;
  nmsg=(char*)RAPTOR_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    raptor_parser_fatal_error_varargs(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_error_prefix);
    strcpy(nmsg+prefix_length, msg);
    if(nmsg[length-2]=='\n')
      nmsg[length-2]='\0';
    raptor_parser_fatal_error_varargs(rdf_parser, nmsg, args);
    RAPTOR_FREE(cstring,nmsg);
  }
  va_end(args);
}


void
raptor_libxml_validation_error(void *ctx, const char *msg, ...) 
{
  va_list args;
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  int prefix_length=strlen(xml_validation_error_prefix);
  int length;
  char *nmsg;

  va_start(args, msg);

  raptor_libxml_update_document_locator(rdf_parser);

  length=prefix_length+strlen(msg)+1;
  nmsg=(char*)RAPTOR_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    raptor_parser_fatal_error_varargs(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_validation_error_prefix);
    strcpy(nmsg+prefix_length, msg);
    if(nmsg[length-2]=='\n')
      nmsg[length-2]='\0';
    raptor_parser_fatal_error_varargs(rdf_parser, nmsg, args);
    RAPTOR_FREE(cstring,nmsg);
  }
  va_end(args);
}


void
raptor_libxml_validation_warning(void *ctx, const char *msg, ...) 
{
  va_list args;
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  int prefix_length=strlen(xml_validation_warning_prefix);
  int length;
  char *nmsg;

  va_start(args, msg);

  raptor_libxml_update_document_locator(rdf_parser);

  length=prefix_length+strlen(msg)+1;
  nmsg=(char*)RAPTOR_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    raptor_parser_warning_varargs(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_validation_warning_prefix);
    strcpy(nmsg+prefix_length, msg);
    if(nmsg[length-2]=='\n')
      nmsg[length-2]='\0';
    raptor_parser_fatal_error_varargs(rdf_parser, nmsg, args);
    RAPTOR_FREE(cstring,nmsg);
  }
  va_end(args);
}


#ifdef RAPTOR_LIBXML_MY_ENTITIES
/*
 * raptor_free_xml_entity : free an entity record.
 */
static void
raptor_libxml_free_entity(raptor_libxml_entity *ent) {
  if (!ent)
    return;
  
  if (ent->entity.name)
    RAPTOR_FREE(cstring,  (char*)ent->entity.name);
  if (ent->entity.ExternalID)
    RAPTOR_FREE(cstring,  (char*)ent->entity.ExternalID);
  if (ent->entity.SystemID)
    RAPTOR_FREE(cstring,  (char*)ent->entity.SystemID);
  if (ent->entity.content)
    RAPTOR_FREE(cstring,  ent->entity.content);

  RAPTOR_FREE(raptor_libxml_entity, ent);
}


static raptor_libxml_entity*
raptor_libxml_new_entity(raptor_parser* rdf_parser,
                         const xmlChar *name, int type,
                         const xmlChar *ExternalID,
                         const xmlChar *SystemID, const xmlChar *content)
{
  raptor_libxml_entity *ent;
  
  ent=(raptor_libxml_entity*)RAPTOR_CALLOC(raptor_libxml_entity, 
                                        sizeof(raptor_libxml_entity), 1);
  if(!ent) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return NULL;
  }
  
  RAPTOR_ENTITY_NAME_LENGTH(ent)=strlen(name);
  ent->entity.name = RAPTOR_MALLOC(cstring, RAPTOR_ENTITY_NAME_LENGTH(ent)+1);
  if(!ent->entity.name) {
    raptor_libxml_free_entity(ent);
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return NULL;
  }
  
  strncpy((char*)ent->entity.name, name, RAPTOR_ENTITY_NAME_LENGTH(ent) +1); /* +1 for \0 */
  
#ifdef RAPTOR_LIBXML_ENTITY_ETYPE
  ent->entity.type = XML_ENTITY_DECL;
  ent->entity.etype = type;
#else
  ent->entity.type = type;
#endif
  
  if (ExternalID) {
    ent->entity.ExternalID = RAPTOR_MALLOC(cstring, strlen(ExternalID)+1);
    if(!ent->entity.ExternalID) {
      raptor_libxml_free_entity(ent);
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      return NULL;
    }
    strcpy((char*)ent->entity.ExternalID, ExternalID);
  } else
    ent->entity.ExternalID = NULL;
  
  if (SystemID) {
    ent->entity.SystemID = RAPTOR_MALLOC(cstring, strlen(SystemID)+1);
    if(!ent->entity.SystemID) {
      raptor_libxml_free_entity(ent);
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      return NULL;
    }
    strcpy((char*)ent->entity.SystemID, SystemID);
  } else
    ent->entity.SystemID = NULL;
  
  if (content) {
    ent->entity.length = strlen(content);
    ent->entity.content = RAPTOR_MALLOC(cstring, ent->entity.length+1);
    if(!ent->entity.content) {
      raptor_libxml_free_entity(ent);
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      return NULL;
    }
    strncpy(ent->entity.content, content, ent->entity.length+1);
  } else {
    ent->entity.length = 0;
    ent->entity.content = NULL;
  }
  
  return ent;
}


void
raptor_libxml_libxml_free_entities(raptor_parser *rdf_parser) 
{
  raptor_libxml_entity *cur, *next;
  cur=raptor_libxml_get_entities(rdf_parser);
  while(cur) {
    next=cur->next;
    raptor_libxml_free_entity(cur);
    cur=next;
  }
}


static void
raptor_libxml_entity_decl(void *ctx, 
                          const xmlChar *name, int type,
                          const xmlChar *publicId, const xmlChar *systemId, 
                          xmlChar *content)
{
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  raptor_libxml_entity *ent;

  ent=raptor_libxml_new_entity(rdf_parser, 
                            name, type, publicId, systemId, content);
  if(!ent)
    return;
  
  ent->next=raptor_libxml_get_entities(rdf_parser);
  raptor_set_libxml_entities(rdf_parser,ent);
}


static xmlEntityPtr
raptor_libxml_get_entity(void *ctx, const xmlChar *name) {
  raptor_parser* rdf_parser=(raptor_parser*)ctx;

  raptor_libxml_entity *ent;
  ent=raptor_libxml_get_entities(rdf_parser);
  while(ent) {
    if (!xmlStrcmp(ent->entity.name, name)) 
      return &ent->entity;
    ent=ent->next;
  }

  return xmlGetPredefinedEntity(name);
}
#endif


void
raptor_libxml_init(xmlSAXHandler *sax) {
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
  sax->setDocumentLocator=raptor_libxml_set_document_locator;
  sax->startDocument = raptor_libxml_startDocument;
  sax->endDocument = raptor_libxml_endDocument;
  sax->startElement=raptor_xml_start_element_handler;
  sax->endElement=raptor_xml_end_element_handler;
  sax->reference = NULL;     /* reference */
  sax->characters=raptor_xml_characters_handler;
  sax->cdataBlock = raptor_xml_cdata_handler; /* like <![CDATA[...]> */
  sax->ignorableWhitespace=raptor_xml_cdata_handler;
  sax->processingInstruction = NULL; /* processingInstruction */
  sax->comment = raptor_xml_comment_handler;      /* comment */
  sax->warning=raptor_libxml_warning;
  sax->error=raptor_libxml_error;
  sax->fatalError=raptor_libxml_fatal_error;

#ifdef RAPTOR_LIBXML_XMLSAXHANDLER_EXTERNALSUBSET
  sax->externalSubset = raptor_libxml_externalSubset;
#endif

#ifdef RAPTOR_LIBXML_XMLSAXHANDLER_INITIALIZED
  sax->initialized = 1;
#endif

#ifdef RAPTOR_LIBXML_MY_ENTITIES
  sax->getEntity=raptor_libxml_get_entity;
  sax->entityDecl=raptor_libxml_entity_decl;
#endif
}


/* end if RAPTOR_XML_LIBXML */
#endif
