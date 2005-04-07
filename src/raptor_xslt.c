/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_xslt_parse.c - Raptor GRDDL XSLT Parser implementation
 *
 * $Id$
 *
 * Copyright (C) 2005, David Beckett http://purl.org/net/dajobe/
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

  /* (RDF/XML) parser for dealing with the result */
  raptor_parser* rdfxml;

};


typedef struct raptor_xslt_parser_context_s raptor_xslt_parser_context;


static int
raptor_xslt_parse_init(raptor_parser* rdf_parser, const char *name)
{
  raptor_xslt_parser_context *xslt_parser=(raptor_xslt_parser_context*)rdf_parser->context;

  xslt_parser->rdfxml=raptor_new_parser("rdfxml");
  if(!xslt_parser->rdfxml) {
    raptor_parser_error(rdf_parser, "Failed to create RDF/XML parser");
    return 1;
  }

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

  if(xslt_parser->rdfxml)
    raptor_free_parser(xslt_parser->rdfxml);
}


static int
raptor_xslt_parse_start(raptor_parser *rdf_parser) 
{
  raptor_xslt_parser_context* xslt_parser=(raptor_xslt_parser_context*)rdf_parser->context;
  raptor_locator *locator=&rdf_parser->locator;
  raptor_parser *p=xslt_parser->rdfxml;
  
  locator->line=1;
  locator->column= -1;
  locator->byte= -1;

  /* copy any user data to the internal parser */
  p->user_data=rdf_parser->user_data;
  p->fatal_error_user_data=rdf_parser->fatal_error_user_data;
  p->error_user_data=rdf_parser->error_user_data;
  p->warning_user_data=rdf_parser->warning_user_data;
  p->fatal_error_handler=rdf_parser->fatal_error_handler;
  p->error_handler=rdf_parser->error_handler;
  p->warning_handler=rdf_parser->warning_handler;
  p->statement_handler=rdf_parser->statement_handler;
  p->generate_id_handler_user_data=rdf_parser->generate_id_handler_user_data;
  p->generate_id_handler=rdf_parser->generate_id_handler;
  p->default_generate_id_handler_base=rdf_parser->default_generate_id_handler_base;
  p->default_generate_id_handler_prefix=rdf_parser->default_generate_id_handler_prefix;
  p->default_generate_id_handler_prefix_length=rdf_parser->default_generate_id_handler_prefix_length;

  return 0;
}


static const xmlChar* xpathExpressions[4]={
  /* XHTML document where the GRDDL profile is in
   * <link ref='transform'>  inside the html <head>
   */
  (const xmlChar*)"/html:html/html:head[contains(@profile,\"http://www.w3.org/2003/g/data-view\")]/html:link[@rel=\"transformation\"]/@href",
  /* XHTML document where the GRDDL profile is in
   * <a rel='transform' href='url'> inside the html <body>
   */
  (const xmlChar*)"/html:html/html:head[contains(@profile,\"http://www.w3.org/2003/g/data-view\")]/../..//html:a[@rel=\"transformation\"]/@href",
  /* XML document linking to transform via attribute dataview:transformation */
  (const xmlChar*)"//@dataview:transformation",
  NULL
};


static void
raptor_xslt_uri_parse_bytes(raptor_www* www,
                            void *userdata, 
                            const void *ptr, size_t size, size_t nmemb)
{
  xmlParserCtxtPtr* ctxt_ptr=(xmlParserCtxtPtr*)userdata;
  int len=size*nmemb;
  int rc=0;
  
  if(!*ctxt_ptr) {
    xmlParserCtxtPtr xc;
    
    xc = xmlCreatePushParserCtxt(NULL, NULL,
                                 ptr, len, 
                                 raptor_uri_as_string(www->uri));
    if(!xc)
      rc=1;
    else {
      xc->replaceEntities = 1;
      xc->loadsubset = 1;
    }
    *ctxt_ptr=xc;
  } else
    rc=xmlParseChunk(*ctxt_ptr, ptr, len, 0);

  if(rc)
    raptor_www_abort(www, "Parsing failed");
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

  if(!xslt_parser->ctxt) {
    uri_string=raptor_uri_as_string(rdf_parser->base_uri);

    /* first time, so init context with first read bytes */
    xslt_parser->ctxt = xmlCreatePushParserCtxt(NULL, NULL,
                                                s, len, uri_string);
    if(!xslt_parser->ctxt) {
      raptor_parser_error(rdf_parser, "Failed to create XML parser");
      return 1;
    }
    
    raptor_libxml_init_sax_error_handlers(&xslt_parser->sax);
    raptor_libxml_init_generic_error_handlers(rdf_parser);
    
    xslt_parser->ctxt->replaceEntities = 1;
    xslt_parser->ctxt->loadsubset = 1;

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
                     "html", "http://www.w3.org/1999/xhtml");
  xmlXPathRegisterNs(xslt_parser->xpathCtx,
                     "dataview", "http://www.w3.org/2003/g/data-view#");

  /* Try all XPaths */
  for(expri=0; xpathExpressions[expri]; expri++) {
    const xmlChar* xpathExpr=xpathExpressions[expri];

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
      RAPTOR_DEBUG3("No GRDDL found with XPath expression \"%s\" over '%s'\n", 
                    xpathExpr, raptor_uri_as_string(rdf_parser->base_uri));
      continue;
    }


    for(i=0; i < xmlXPathNodeSetGetLength(nodes); i++) {
      xmlNodePtr node=nodes->nodeTab[i];
      xsltStylesheetPtr sheet=NULL;
      xmlDocPtr res=NULL;
      xmlParserCtxtPtr xslt_ctxt;
      raptor_www *www;
      xmlChar *doc_txt=NULL;
      int doc_txt_len=0;
      xmlChar *base_uri_string;
      raptor_uri* base_uri=NULL;

      if(node->type != XML_ATTRIBUTE_NODE) {
        raptor_parser_error(rdf_parser, "Got unexpected node type %d", 
                            node->type);
        continue;
      }


      /* returns base URI or NULL - must be freed with xmlFree() */
      base_uri_string=xmlNodeGetBase(doc, node);
      if(base_uri_string) {
        base_uri=raptor_new_uri(base_uri_string);
        xmlFree(base_uri_string);
        RAPTOR_DEBUG2("Got XML base URI '%s'\n", raptor_uri_as_string(base_uri));
      } else if(rdf_parser->base_uri)
        base_uri=raptor_uri_copy(rdf_parser->base_uri);

      uri_string=(const unsigned char*)node->children->content;
      uri=raptor_new_uri_relative_to_base(base_uri, uri_string);
      if(base_uri)
        raptor_free_uri(base_uri);

      RAPTOR_DEBUG2("Running GRDDL transform with URI '%s'\n",
                    raptor_uri_as_string(uri));


      /* make an xsltStylesheetPtr via the raptor_xslt_uri_parse_bytes 
       * callback as bytes are returned
       */
      xslt_ctxt=NULL;

      www=raptor_www_new();
      raptor_www_set_write_bytes_handler(www,
                                         raptor_xslt_uri_parse_bytes, 
                                         &xslt_ctxt);

      if(raptor_www_fetch(www, uri)) {
        ret=1;
        goto cleanup_xslt;
      }

      xmlParseChunk(xslt_ctxt, NULL, 0, 1);

      sheet = xsltParseStylesheetDoc(xslt_ctxt->myDoc);
      if(!sheet) {
        raptor_parser_error(rdf_parser, "Failed to parse stylesheet in '%s'",
                            raptor_uri_as_string(uri));
        ret=1;
        goto cleanup_xslt;
      }

      res = xsltApplyStylesheet(sheet, doc, NULL); /* no params */
      if(!res) {
        raptor_parser_error(rdf_parser, "Failed to apply stylesheet in '%s'",
                            raptor_uri_as_string(uri));
        ret=1;
        goto cleanup_xslt;
      }


      /* write the resulting XML to a string */
      xsltSaveResultToString(&doc_txt, &doc_txt_len, res, sheet);

      if(!doc_txt || !doc_txt_len) {
        /* empty document - continue? FIXME */
        raptor_parser_warning(rdf_parser, 
                              "Stylesheet returned an empty document");
      } else {
        RAPTOR_DEBUG2("XSLT gave %d bytes XML result\n", doc_txt_len);

        /* generate the triples */
        raptor_start_parse(xslt_parser->rdfxml, rdf_parser->base_uri);
        raptor_parse_chunk(xslt_parser->rdfxml, doc_txt, doc_txt_len, 1);
      }

    cleanup_xslt:
      if(doc_txt)    
        xmlFree(doc_txt);

      if(res)
        xmlFreeDoc(res);

      if(sheet)
        xsltFreeStylesheet(sheet);

      if(xslt_ctxt)
        xmlFreeParserCtxt(xslt_ctxt); 

      if(uri)
        raptor_free_uri(uri);

      if(www)
        raptor_www_free(www);

    } /* end node loop */

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
  
  factory->init      = raptor_xslt_parse_init;
  factory->terminate = raptor_xslt_parse_terminate;
  factory->start     = raptor_xslt_parse_start;
  factory->chunk     = raptor_xslt_parse_chunk;
  factory->recognise_syntax = raptor_xslt_parse_recognise_syntax;
}


void
raptor_init_parser_grddl(void)
{
  raptor_parser_register_factory("grddl",  "GRDDL over XHTML/XML using XSLT",
                                 NULL, NULL,
                                 NULL,
                                 &raptor_xslt_parser_register_factory);
}
