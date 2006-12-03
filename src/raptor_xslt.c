/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_xslt_parse.c - Raptor GRDDL XSLT Parser implementation
 *
 * Copyright (C) 2005-2006, David Beckett http://purl.org/net/dajobe/
 * Copyright (C) 2005, University of Bristol, UK http://www.bristol.ac.uk/
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

/*
 * W3C Gleaning Resource Descriptions from Dialects of Languages (GRDDL)
 * http://www.w3.org/2004/01/rdxh/spec
 * 
 * See also
 *   http://www.w3.org/2003/g/data-view
 *
 *
 * Looks for indication of GRDDL meaning intended in the XML (XHTML)
 * document source.
 *
 * 1. /html/head[@profile="http://www.w3.org/2003/g/data-view"]
 * 2. /html/head/link[@rel="transformation"] (may be repeated)
 *
 * Indicating that the sheet in the value of @href of #2 transforms
 * the document into RDF/XML and hence RDF triples.
 *
 * In example:
 *
 * <html xmlns="http://www.w3.org/1999/xhtml">
 * <head profile="http://www.w3.org/2003/g/data-view">
 *   ...
 *   <link rel="transformation" href="URI-of-XSLT" />
 *
 * The <link rel> may be repeated.
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

#include <libxml/xpathInternals.h>

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>


/*
 * libxslt API notes
 *
 * Inputs to an XSLT transformation process with libxslt are:
 *   1. A set of (key:value) parameters.
 *   2. An xsltStylesheetPtr for the XSLT sheet
 *     Which could be made from a file or an xmlDoc; and the xmlDoc.
 *     made from a file or memory buffer.
 *   3. An xmlDoc for the XML source
 *     Which could be made from a file or a memory buffer.
 *   
 */



/*
 * XSLT parser object
 */
struct raptor_xslt_parser_context_s {
  xmlSAXHandler sax;

  /* XML document ctxt */
  xmlParserCtxtPtr ctxt;

  /* Create xpath evaluation context */
  xmlXPathContextPtr xpathCtx; 

  /* Evaluate xpath expression */
  xmlXPathObjectPtr xpathObj;

  /* parser for dealing with the result */
  raptor_parser* internal_parser;
  /* ... constructed with this name */
  const char* internal_parser_name;
};


typedef struct raptor_xslt_parser_context_s raptor_xslt_parser_context;


static int
raptor_xslt_parse_init(raptor_parser* rdf_parser, const char *name)
{
  return 0;
}


static void
raptor_xslt_parse_terminate(raptor_parser *rdf_parser)
{
  raptor_xslt_parser_context *xslt_parser=(raptor_xslt_parser_context*)rdf_parser->context;
  if(xslt_parser->ctxt) {
    if(xslt_parser->ctxt->myDoc) {
      xmlFreeDoc(xslt_parser->ctxt->myDoc);
      xslt_parser->ctxt->myDoc=NULL;
    }
    xmlFreeParserCtxt(xslt_parser->ctxt);
  }

  if(xslt_parser->xpathCtx)
    xmlXPathFreeContext(xslt_parser->xpathCtx);

  if(xslt_parser->xpathObj)
    xmlXPathFreeObject(xslt_parser->xpathObj);

  if(xslt_parser->internal_parser)
    raptor_free_parser(xslt_parser->internal_parser);
}


static int
raptor_xslt_parse_start(raptor_parser *rdf_parser) 
{
  raptor_locator *locator=&rdf_parser->locator;
  
  locator->line=1;

  return 0;
}


static struct {
  const xmlChar* xpath;
  int is_value_list;
  const xmlChar* xslt_sheet_uri;
} match_table[]={
  /* XHTML document where the GRDDL profile is in
   * <link ref='transform' href='url'>  inside the html <head>
   */
  {
    (const xmlChar*)"/html:html/html:head[contains(@profile,\"http://www.w3.org/2003/g/data-view\")]/html:link[@rel=\"transformation\"]/@href",
    0,
    NULL
  }
  ,
  /* XHTML document where the GRDDL profile is in
   * <a rel='transform' href='url'> inside the html <body>
   */
  {
    (const xmlChar*)"/html:html/html:head[contains(@profile,\"http://www.w3.org/2003/g/data-view\")]/../..//html:a[@rel=\"transformation\"]/@href",
    0,
    NULL
  }
  ,
  /* XML document linking to transform via attribute dataview:transformation 
   * Example: http://www.w3.org/2004/01/rdxh/grddl-p3p-example
   **/
  {
    (const xmlChar*)"//@dataview:transformation",
    1, /* list of URIs */
    NULL
  }
  ,
#if 0
  /* FIXME Disabled. This returns the wrong namespaces in
   * Id: dc-extract.xsl,v 1.10 2005/09/07 17:10:06 connolly Exp
   */

  /* Dublin Core in <meta> tags http://dublincore.org/documents/dcq-html/ */
  {
    (const xmlChar*)"/html:html/html:head/html:link[@href=\"http://purl.org/dc/elements/1.1/\"]",
    0,
    (const xmlChar*)"http://www.w3.org/2000/06/dc-extract/dc-extract.xsl"
  }
  ,
#endif
  /* Embedded RDF 
   * <head profile="http://purl.org/NET/erdf/profile"> inside <html>
   */
  { 
    (const xmlChar*)"/html:html/html:head[contains(@profile,\"http://purl.org/NET/erdf/profile\")]",
    0,
    (const xmlChar*)"http://purl.org/NET/erdf/extract-rdf.xsl"
  }
  ,
  /* hCalendar microformat http://microformats.org/wiki/hcalendar */
  {
    (const xmlChar*)"//*[@class=\"vevent\"]",
    0,
    (const xmlChar*)"http://www.w3.org/2002/12/cal/glean-hcal.xsl"
  }
  ,
  { 
    NULL,
    0,
    0
  }
};


/* Run a GRDDL transform using a pre-parsed XSLT stylesheet already
 * formed into a libxml document (with URI)
 */
static int
raptor_xslt_run_grddl_transform_doc(raptor_parser* rdf_parser, 
                                    raptor_uri* xslt_uri, xmlDocPtr xslt_doc,
                                    xmlDocPtr doc)
{
  raptor_xslt_parser_context* xslt_parser=(raptor_xslt_parser_context*)rdf_parser->context;
  int ret=0;
  xsltStylesheetPtr sheet=NULL;
  xmlDocPtr res=NULL;
  xmlChar *doc_txt=NULL;
  int doc_txt_len=0;
  const char* parser_name;

  RAPTOR_DEBUG3("Running GRDDL transform with XSLT URI '%s' on doc URI '%s'\n",
                raptor_uri_as_string(xslt_uri),
                raptor_uri_as_string(rdf_parser->base_uri));
  
  sheet = xsltParseStylesheetDoc(xslt_doc);
  if(!sheet) {
    raptor_parser_error(rdf_parser, "Failed to parse stylesheet in '%s'",
                        raptor_uri_as_string(xslt_uri));
    ret=1;
    goto cleanup_xslt;
  }
  
  res = xsltApplyStylesheet(sheet, doc, NULL); /* no params */
  if(!res) {
    raptor_parser_error(rdf_parser, "Failed to apply stylesheet in '%s'",
                        raptor_uri_as_string(xslt_uri));
    ret=1;
    goto cleanup_xslt;
  }

  if(res->type == XML_HTML_DOCUMENT_NODE) {
    if(sheet->method != NULL)
      xmlFree(sheet->method);
    sheet->method = xmlMalloc(5);
    strncpy((char*)sheet->method, "html", 5);
  }

  /* write the resulting XML to a string */
  xsltSaveResultToString(&doc_txt, &doc_txt_len, res, sheet);
  
  if(!doc_txt || !doc_txt_len) {
    /* FIXME: continue with an empty document? */
    raptor_parser_warning(rdf_parser, "XSLT returned an empty document");
    goto cleanup_xslt;
  }

  RAPTOR_DEBUG4("XSLT returned %d bytes document method %s media type %s\n", 
                doc_txt_len, 
                (sheet->method ? (const char*)sheet->method : "NULL"), 
                (sheet->mediaType ? (const char*)sheet->mediaType : "NULL"));

  /* FIXME: Assumes mime types for XSLT <xsl:output method> */
  if(sheet->mediaType == NULL && sheet->method) {
    if(!(strcmp((const char*)sheet->method, "text"))) {
      sheet->mediaType = xmlMalloc(11);
      strncpy((char*)sheet->mediaType, "text/plain",11);
    } else if(!(strcmp((const char*)sheet->method, "xml"))) {
      sheet->mediaType = xmlMalloc(16);
      strncpy((char*)sheet->mediaType, "application/xml",16);
    } else if(!(strcmp((const char*)sheet->method, "html"))) {
      sheet->mediaType = xmlMalloc(10);
      /* FIXME: use xhtml mime type? */
      strncpy((char*)sheet->mediaType, "text/html",10);
    }
  }

  /* FIXME: Assume all that all media XML is RDF/XML and also that
   * with no information at all we have RDF/XML
   */
  if(!sheet->mediaType ||
     (sheet->mediaType &&
      !strcmp((const char*)sheet->mediaType, "application/xml"))) {
    if(sheet->mediaType)
      xmlFree(sheet->mediaType);
    sheet->mediaType = xmlMalloc(20);
    strncpy((char*)sheet->mediaType, "application/rdf+xml",20);
  }
  
  parser_name=raptor_guess_parser_name(NULL, (const char*)sheet->mediaType, 
                                       doc_txt, doc_txt_len, NULL);
  RAPTOR_DEBUG3("Guessed parser %s from mime type '%s'\n",
                parser_name, sheet->mediaType);

  if(!strcmp((const char*)parser_name, "grddl")) {
    RAPTOR_DEBUG1("Ignoring guess to run grddl parser.");
    goto cleanup_xslt;
  }

  /* construct a parser or use the previous one if it is the same */
  if(!xslt_parser->internal_parser_name ||
     strcmp(xslt_parser->internal_parser_name, parser_name)) {
    if(xslt_parser->internal_parser) {

      RAPTOR_DEBUG2("Freeing internal %s parser.\n",
                    xslt_parser->internal_parser_name);
      
      raptor_free_parser(xslt_parser->internal_parser);
      xslt_parser->internal_parser=NULL;
      xslt_parser->internal_parser_name=NULL;
    }
    
    RAPTOR_DEBUG2("Allocating new internal %s parser.\n", parser_name);
    xslt_parser->internal_parser=raptor_new_parser(parser_name);
    if(!xslt_parser->internal_parser) {
      raptor_parser_error(rdf_parser, "Failed to create %s parser",
                          parser_name);
    } else {
      /* initialise the new parser with the outer state */
      xslt_parser->internal_parser_name=parser_name;
      raptor_parser_copy_user_state(xslt_parser->internal_parser,
                                    rdf_parser);
    }
  }

  if(xslt_parser->internal_parser) {
    /* generate the triples */
    raptor_start_parse(xslt_parser->internal_parser, rdf_parser->base_uri);
    raptor_parse_chunk(xslt_parser->internal_parser, doc_txt, doc_txt_len, 1);
  }
  
  cleanup_xslt:
  if(doc_txt)    
    xmlFree(doc_txt);
  
  if(res)
    xmlFreeDoc(res);
  
  if(sheet)
    xsltFreeStylesheet(sheet);
  
  return ret;
}


typedef struct 
{
  raptor_parser* rdf_parser;
  xmlParserCtxtPtr xc;
} raptor_xslt_parse_bytes_context;
  

static void
raptor_xslt_uri_parse_bytes(raptor_www* www,
                            void *userdata, 
                            const void *ptr, size_t size, size_t nmemb)
{
  raptor_xslt_parse_bytes_context* pbc=(raptor_xslt_parse_bytes_context*)userdata;
  int len=size*nmemb;
  int rc=0;
  
  if(!pbc->xc) {
    xmlParserCtxtPtr xc;
    
    xc = xmlCreatePushParserCtxt(NULL, NULL,
                                 (const char*)ptr, len, 
                                 (const char*)raptor_uri_as_string(www->uri));
    if(!xc)
      rc=1;
    else {
      int libxml_options = 0;

#ifdef XML_PARSE_NONET
      if(pbc->rdf_parser->features[RAPTOR_FEATURE_NO_NET])
        libxml_options |= XML_PARSE_NONET;
#endif
#ifdef HAVE_XMLCTXTUSEOPTIONS
      xmlCtxtUseOptions(xc, libxml_options);
#endif

      xc->replaceEntities = 1;
      xc->loadsubset = 1;
    }
    pbc->xc=xc;
  } else
    rc=xmlParseChunk(pbc->xc, (const char*)ptr, len, 0);

  if(rc)
    raptor_parser_error(pbc->rdf_parser, "Parsing failed");
}


/* Run a GRDDL transform using a XSLT stylesheet at a given URI */
static int
raptor_xslt_run_grddl_transform_uri(raptor_parser* rdf_parser, 
                                    raptor_uri* xslt_uri, xmlDocPtr doc)
{
  raptor_www *www=NULL;
  xmlParserCtxtPtr xslt_ctxt=NULL;
  raptor_xslt_parse_bytes_context pbc;
  int ret=0;
  
  RAPTOR_DEBUG2("Running GRDDL transform with XSLT URI '%s'\n",
                raptor_uri_as_string(xslt_uri));
  
  /* make an xsltStylesheetPtr via the raptor_xslt_uri_parse_bytes 
   * callback as bytes are returned
   */
  pbc.xc=NULL;
  pbc.rdf_parser=rdf_parser;
  
  www=raptor_www_new();

  if(rdf_parser->uri_filter)
    raptor_www_set_uri_filter(www, rdf_parser->uri_filter,
                              rdf_parser->uri_filter_user_data);
  else if(rdf_parser->features[RAPTOR_FEATURE_NO_NET])
    raptor_www_set_uri_filter(www, raptor_parse_uri_no_net_filter, rdf_parser);

  raptor_www_set_write_bytes_handler(www, raptor_xslt_uri_parse_bytes, &pbc);
  
  if(raptor_www_fetch(www, xslt_uri)) {
    ret=1;
    goto cleanup_xslt;
  }
  
  xslt_ctxt=pbc.xc;
  xmlParseChunk(pbc.xc, NULL, 0, 1);
  
  ret=raptor_xslt_run_grddl_transform_doc(rdf_parser, 
                                          xslt_uri, xslt_ctxt->myDoc, 
                                          doc);
  
  cleanup_xslt:
  if(xslt_ctxt)
    xmlFreeParserCtxt(xslt_ctxt); 
  
  if(www)
    raptor_www_free(www);

  return ret;
}


static int
raptor_xslt_parse_chunk(raptor_parser* rdf_parser, 
                        const unsigned char *s, size_t len,
                        int is_end)
{
  raptor_xslt_parser_context* xslt_parser=(raptor_xslt_parser_context*)rdf_parser->context;
  int i;
  int ret=0;
  const unsigned char* uri_string;
  raptor_uri* uri;
  /* XML document DOM */
  xmlDocPtr doc;
  xmlNodeSetPtr nodes;
  int expri;
  xmlChar *base_uri_string;
  raptor_uri* base_uri=NULL;

  if(!xslt_parser->ctxt) {
    uri_string=raptor_uri_as_string(rdf_parser->base_uri);

    /* first time, so init context with first read bytes */
    xslt_parser->ctxt = xmlCreatePushParserCtxt(NULL, NULL,
                                                (const char*)s, len,
                                                (const char*)uri_string);
    if(!xslt_parser->ctxt) {
      raptor_parser_error(rdf_parser, "Failed to create XML parser");
      return 1;
    }
    
    raptor_libxml_init_sax_error_handlers(&xslt_parser->sax);
    raptor_libxml_init_generic_error_handlers(rdf_parser);
    
    xslt_parser->ctxt->replaceEntities = 1;
    xslt_parser->ctxt->loadsubset = 1;

    if(is_end)
      xmlParseChunk(xslt_parser->ctxt, (const char*)s, 0, is_end);
  } else if((s && len) || is_end)
    xmlParseChunk(xslt_parser->ctxt, (const char*)s, len, is_end);

  if(!is_end)
    return 0;


  doc=xslt_parser->ctxt->myDoc;
  if(!doc) {
    raptor_parser_error(rdf_parser, "Failed to create XML DOM for document");
    return 1;
  }
  

  /* Create the XPath evaluation context */
  xslt_parser->xpathCtx=NULL; 

  xslt_parser->xpathCtx = xmlXPathNewContext(doc);
  if(!xslt_parser->xpathCtx) {
    raptor_parser_error(rdf_parser, "Failed to create XPath context for document");
    return 1;
  }

  xmlXPathRegisterNs(xslt_parser->xpathCtx,
                     (const xmlChar*)"html", 
                     (const xmlChar*)"http://www.w3.org/1999/xhtml");
  xmlXPathRegisterNs(xslt_parser->xpathCtx,
                     (const xmlChar*)"dataview",
                     (const xmlChar*)"http://www.w3.org/2003/g/data-view#");

  base_uri=NULL;
  
  /* Try all XPaths */
  for(expri=0; match_table[expri].xpath; expri++) {
    const xmlChar* xpathExpr=match_table[expri].xpath;

    /* Evaluate xpath expression */
    xslt_parser->xpathObj = xmlXPathEvalExpression(xpathExpr, 
                                                   xslt_parser->xpathCtx);
    if(!xslt_parser->xpathObj) {
      raptor_parser_error(rdf_parser, 
                          "Unable to evaluate XPath expression \"%s\"",
                          xpathExpr);
      return 1;
     }

    nodes=xslt_parser->xpathObj->nodesetval;
    if(!nodes || xmlXPathNodeSetIsEmpty(nodes)) {
      RAPTOR_DEBUG3("No match found with XPath expression \"%s\" over '%s'\n", 
                    xpathExpr, raptor_uri_as_string(rdf_parser->base_uri));
      continue;
    }

    RAPTOR_DEBUG3("Found match with XPath expression \"%s\" over '%s'\n", 
                  xpathExpr, raptor_uri_as_string(rdf_parser->base_uri));

    if(match_table[expri].xslt_sheet_uri) {
      /* Ignore what matched, use a hardcoded XSLT URI */
      uri_string=match_table[expri].xslt_sheet_uri;
      base_uri=raptor_uri_copy(rdf_parser->base_uri);

      RAPTOR_DEBUG2("Using hard-coded XSLT URI '%s'\n",  uri_string);

      uri=raptor_new_uri_relative_to_base(base_uri, uri_string);
      ret=raptor_xslt_run_grddl_transform_uri(rdf_parser, uri, doc);
      raptor_free_uri(uri);

      if(base_uri)
        raptor_free_uri(base_uri);
    } else {
      for(i=0; i < xmlXPathNodeSetGetLength(nodes); i++) {
        xmlNodePtr node=nodes->nodeTab[i];

        if(node->type != XML_ATTRIBUTE_NODE) {
          raptor_parser_error(rdf_parser, "Got unexpected node type %d", 
                              node->type);
          continue;
        }

        /* returns base URI or NULL - must be freed with xmlFree() */
        base_uri_string=xmlNodeGetBase(doc, node);

        uri_string=(const unsigned char*)node->children->content;

        if(base_uri_string) {
          base_uri=raptor_new_uri(base_uri_string);
          xmlFree(base_uri_string);
          RAPTOR_DEBUG2("Got XML base URI '%s'\n", raptor_uri_as_string(base_uri));
        } else if(rdf_parser->base_uri)
          base_uri=raptor_uri_copy(rdf_parser->base_uri);
        else
          base_uri=NULL;

        if(match_table[expri].is_value_list) {
          char *start;
          char *end;
          char* buffer;
          size_t list_len=strlen((const char*)uri_string);

          buffer=(char*)RAPTOR_MALLOC(cstring, list_len+1);
          strncpy(buffer, (const char*)uri_string, list_len+1);
          
          for(start=end=buffer; end; start=end+1) {
            end=strchr(start, ' ');
            if(end)
              *end='\0';

            if(start == end)
              continue;
            
            RAPTOR_DEBUG2("Got list URI '%s'\n", start);

            uri=raptor_new_uri_relative_to_base(base_uri, (const unsigned char*)start);
            ret=raptor_xslt_run_grddl_transform_uri(rdf_parser, uri, doc);
            raptor_free_uri(uri);
          }
          RAPTOR_FREE(cstring, buffer);
        } else {
          uri=raptor_new_uri_relative_to_base(base_uri, uri_string);
          ret=raptor_xslt_run_grddl_transform_uri(rdf_parser, uri, doc);
          raptor_free_uri(uri);
        }
        
        if(base_uri)
          raptor_free_uri(base_uri);

      }
    }
    
    if(rdf_parser->failed || ret != 0)
      break;

  } /* end XPath expression loop */
  
  if(rdf_parser->failed)
    return 1;

  return (ret != 0);
}


static int
raptor_xslt_parse_recognise_syntax(raptor_parser_factory* factory, 
                                   const unsigned char *buffer, size_t len,
                                   const unsigned char *identifier, 
                                   const unsigned char *suffix, 
                                   const char *mime_type)
{
  int score= 0;
  
  if(suffix) {
    if(!strcmp((const char*)suffix, "xhtml"))
      score=7;
    if(!strcmp((const char*)suffix, "html"))
      score=2;
  }
  
  if(identifier) {
    if(strstr((const char*)identifier, "xhtml"))
      score+=5;
  }
  
  return score;
}


static void
raptor_xslt_parser_register_factory(raptor_parser_factory *factory) 
{
  factory->context_length     = sizeof(raptor_xslt_parser_context);
  
  factory->need_base_uri = 1;
  
  factory->init      = raptor_xslt_parse_init;
  factory->terminate = raptor_xslt_parse_terminate;
  factory->start     = raptor_xslt_parse_start;
  factory->chunk     = raptor_xslt_parse_chunk;
  factory->recognise_syntax = raptor_xslt_parse_recognise_syntax;

  raptor_parser_factory_add_mime_type(factory, "text/html", 2);
  raptor_parser_factory_add_mime_type(factory, "application/html+xml", 2);
}


void
raptor_init_parser_grddl(void)
{
  raptor_parser_register_factory("grddl",  "GRDDL over XHTML/XML using XSLT",
                                 &raptor_xslt_parser_register_factory);
}
