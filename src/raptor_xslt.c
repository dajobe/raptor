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


static void raptor_xslt_init(void);
static void raptor_xslt_finish(void);

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



static void
raptor_xslt_init() 
{
  xmlSubstituteEntitiesDefault(1);
  xmlLoadExtDtdDefaultValue = 1;
}


static void
raptor_xslt_finish() 
{
  xsltCleanupGlobals();
  xmlCleanupParser();
}




#ifdef STANDALONE


/* one more prototype */
int main(int argc, char* argv[]);


static void
raptor_xslt_uri_parse_bytes(raptor_www* www,
                            void *userdata, 
                            const void *ptr, size_t size, size_t nmemb)
{
  xmlParserCtxtPtr* ctxt_ptr=(xmlParserCtxtPtr*)userdata;
  int len=size*nmemb;
  int rc=0;
  
  if(!*ctxt_ptr) {
    *ctxt_ptr = xmlCreatePushParserCtxt(NULL, NULL,
                                        ptr, len, 
                                        raptor_uri_as_string(www->uri));
    if(!*ctxt_ptr)
      rc=1;
  } else
    rc=xmlParseChunk(*ctxt_ptr, ptr, len, 0);

  if(rc)
    raptor_www_abort(www, "Parsing failed");
}


/* USAGE
 *
 * Download the source document
 * http://www.w3.org/2003/12/rdf-in-xhtml-xslts/complete-example.html
 * to a local file and run:
 *
 *  ./raptor_xslt_test complete-example.html http://www.w3.org/2003/12/rdf-in-xhtml-xslts/complete-example.html
 */

int
main(int argc, char* argv[]) 
{
  const char *program=raptor_basename(argv[0]);
#define MAX_PARAMS 16
  const char *params[MAX_PARAMS + 1];
  int param_count;
  int i;
  int rc=0;
  FILE *fh=NULL;
/*  const char *xslt_filename=NULL;*/
  const char *xml_filename=NULL;
  raptor_uri *base_uri=NULL;
  int base_uri_alloced=0;
  
  raptor_init();

  raptor_xslt_init();
  
  if (argc <= 1) {
    fprintf(stderr, "%s: [--param key value...] doc-file [stylesheet-file]\n", 
            program);
    return 1;
  }
  

  param_count=0;
  
  for (i=1; i < argc; i++) {
    if(argv[i][0] != '-')
      break;

    if(!strcmp(argv[i], "--param")) {
      i++;
      if(param_count < MAX_PARAMS) {
        params[param_count++] = argv[i];
        params[param_count++] = argv[i+1];
      } else
        fprintf(stderr, "%s: Too many parameters\n", program);
      i+= 2;
      
    } else
      fprintf(stderr, "%s: Unknown option %s\n", program, argv[i]);

  }

  params[param_count] = NULL;  

  
  if(i+1 > argc) {
    fprintf(stderr, "%s: Missing doc-file\n", program);
    rc=1;
    goto cleanup;
  }
  

  xml_filename=argv[i++];

  if(argv[i] != NULL)
    base_uri=raptor_new_uri((unsigned char*)argv[i]);
  else {
    unsigned char *uri_string=raptor_uri_filename_to_uri_string(xml_filename);
    base_uri=raptor_new_uri(uri_string);
    raptor_free_memory(uri_string);
    base_uri_alloced=1;
  }
  
  fprintf(stderr, "%s: Handling XML file '%s' with base URI %s\n",
          program, xml_filename, raptor_uri_as_string(base_uri));
  

  xmlParserCtxtPtr ctxt=NULL; /* an XML parser context */

  #define RAPTOR_XML_BUFFER_SIZE 2048
  char xml_buffer[RAPTOR_XML_BUFFER_SIZE];


  fh=fopen(xml_filename, "r");
  if(!fh) {
    fprintf(stderr, "%s: XML file '%s' open failed - %s",
            program, xml_filename, strerror(errno));
    goto cleanup;
  }

  while(!feof(fh)) {
    int len=fread(xml_buffer, 1, RAPTOR_XML_BUFFER_SIZE, fh);
    int is_end=(len < RAPTOR_XML_BUFFER_SIZE);

    if(!ctxt) {
      /* first time, so init context with first read bytes */
      ctxt = xmlCreatePushParserCtxt(NULL, NULL,
                                     xml_buffer, len, xml_filename);
      if(!ctxt) {
        fprintf(stderr, "%s: Failed to create XML parser\n",
                program);
        rc=1;
        goto cleanup;
      }
      continue;
    }

    rc=xmlParseChunk(ctxt, xml_buffer, len, is_end);
    if(rc || is_end)
      break;
  }
  fclose(fh);
  fh=NULL;


  xmlDocPtr doc=ctxt->myDoc;

  const xmlChar* xpathExpr=(const xmlChar*)"//html:html/html:head[@profile=\"http://www.w3.org/2003/g/data-view\"]/html:link[@rel=\"transformation\"]/@href";
  
  /* Create xpath evaluation context */
  xmlXPathContextPtr xpathCtx=NULL; 

  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    fprintf(stderr, "%s: Unable to create new XPath context\n", 
            program);
    goto cleanup;
  }
  
  xmlXPathRegisterNs(xpathCtx, "html", "http://www.w3.org/1999/xhtml");
  
  /* Evaluate xpath expression */
  xmlXPathObjectPtr xpathObj;
  
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(!xpathObj) {
    fprintf(stderr, "%s: Unable to evaluate XPath expression \"%s\"\n", 
            program, xpathExpr);
    goto cleanup;
   }
  
  xmlNodeSetPtr nodes=xpathObj->nodesetval;
  if(!nodes || xmlXPathNodeSetIsEmpty(nodes)) {
    fprintf(stderr, "%s: No nodes returned from XPath expression \"%s\"\n", 
            program, xpathExpr);
    goto cleanup;
  }
  
  fprintf(stderr, "%s: Found GRDDL in document\n", program);
  
  for(i=0; i < xmlXPathNodeSetGetLength(nodes); i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    
    if(node->type != XML_ATTRIBUTE_NODE) {
      fprintf(stderr, "Got unexpected node type %d\n", node->type);
      continue;
    }

    const unsigned char* uri_string=(const unsigned char*)node->children->content;
    
    raptor_uri* uri=raptor_new_uri_relative_to_base(base_uri, uri_string);
    
    fprintf(stderr, "%s: Running GRDDL transform with URI '%s'\n",
            program, raptor_uri_as_string(uri));
    
    
    /* make an xsltStylesheetPtr */
    xsltStylesheetPtr sheet=NULL;
    xmlDocPtr sheet_doc=NULL;
    xmlDocPtr res=NULL;
    
    xmlParserCtxtPtr xslt_ctxt=NULL;
    
    raptor_www *www;
    www=raptor_www_new();
    raptor_www_set_write_bytes_handler(www,
                                       raptor_xslt_uri_parse_bytes, 
                                       &xslt_ctxt);
    
    if(raptor_www_fetch(www, uri))
      rc=1;
    raptor_www_free(www);
    if(rc)
      goto cleanup;
    
    xmlParseChunk(xslt_ctxt, NULL, 0, 1);
    
    sheet_doc = xslt_ctxt->myDoc;
    sheet = xsltParseStylesheetDoc(sheet_doc);
    
    res = xsltApplyStylesheet(sheet, doc, params);
    
    /* write the resulting XML to a file */
    /* xsltSaveResultToFile(stdout, res, sheet); */

    /* write the resulting XML to a string */
    xmlChar *doc_txt_ptr=NULL;
    int doc_txt_len;
    xsltSaveResultToString(&doc_txt_ptr, &doc_txt_len, 
                           res, sheet);
 
    fprintf(stderr, "%s: XSLT gave %d bytes XML result\n", 
            program, doc_txt_len);
    
    if(doc_txt_ptr)    
      xmlFree(doc_txt_ptr);

    xmlFreeDoc(res);

    xsltFreeStylesheet(sheet);

    xmlFreeParserCtxt(xslt_ctxt); 

    raptor_free_uri(uri);
  }
  

  xmlXPathFreeObject(xpathObj);

  xmlXPathFreeContext(xpathCtx); 
  xpathCtx=NULL;
  

  goto cleanup;
  
  xmlFreeParserCtxt(ctxt);

  if(rc) {
    fprintf(stderr, "transform failed - %d\n", rc);
    goto cleanup;
  }

  rc=0;

  cleanup:
  if(fh)
    fclose(fh);
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx); 
  if(base_uri_alloced)
    raptor_free_uri(base_uri);

  raptor_xslt_finish();
  
  raptor_finish();

  return rc;
}

#endif
