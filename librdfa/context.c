/**
 * Copyright 2008-2012 Digital Bazaar, Inc.
 *
 * This file is part of librdfa.
 *
 * librdfa is Free Software, and can be licensed under any of the
 * following three licenses:
 *
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any
 *      newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 *
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 *
 * See LICENSE-* at the top of this software distribution for more
 * information regarding the details of each license.
 *
 * The librdfa library is the Fastest RDFa Parser in the Universe. It is
 * a stream parser, meaning that it takes an XML data as input and spits
 * out RDF triples as it comes across them in the stream. Due to this
 * processing approach, librdfa has a very, very small memory footprint.
 * It is also very fast and can operate on hundreds of gigabytes of XML
 * data without breaking a sweat.
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include "rdfa_utils.h"
#include "rdfa.h"

rdfacontext* rdfa_create_context(const char* base)
{
   rdfacontext* rval = NULL;
   size_t base_length = strlen(base);

   /* if the base isn't specified, don't create a context */
   if(base_length > 0)
   {
      char* cleaned_base;

      /* malloc and init whole context to NULL */
      rval = (rdfacontext*)malloc(sizeof(rdfacontext));
      if(!rval)
         return NULL;

      memset(rval, 0, sizeof(rdfacontext));

      /* clean and initialize base */
      cleaned_base = rdfa_iri_get_base(base);
      rval->base = rdfa_replace_string(rval->base, cleaned_base);
      free(cleaned_base);
   }
   else
   {
#ifdef LIBRDFA_IN_RAPTOR
#else
      printf("librdfa error: Failed to create a parsing context, "
         "base IRI was not specified!\n");
#endif
   }

   return rval;
}

void rdfa_init_context(rdfacontext* context)
{
   /* assume the RDFa processing rules are RDFa 1.1 unless otherwise specified */
   context->rdfa_version = RDFA_VERSION_1_1;

   /* assume the default host language is XML1 */
   context->host_language = HOST_LANGUAGE_XML1;

   /* the [parent subject] is set to the [base] value; */
   context->parent_subject = NULL;
   if(context->base != NULL)
   {
      char* cleaned_base = rdfa_iri_get_base(context->base);
      context->parent_subject =
         rdfa_replace_string(context->parent_subject, cleaned_base);
      free(cleaned_base);
   }

   /* the [parent object] is set to null; */
   context->parent_object = NULL;

#ifdef LIBRDFA_IN_RAPTOR
#else
   /* the [list of URI mappings] is cleared; */
   context->uri_mappings = rdfa_create_mapping(MAX_URI_MAPPINGS);
#endif

   /* the [list of incomplete triples] is cleared; */
   context->incomplete_triples = rdfa_create_list(3);

   /* the [language] is set to null. */
   context->language = NULL;

   /* set the [current object resource] to null; */
   context->current_object_resource = NULL;

   /* the list of term mappings is set to null
    * (or a list defined in the initial context of the Host Language). */
   context->term_mappings = rdfa_create_mapping(MAX_TERM_MAPPINGS);

   /* the maximum number of list mappings */
   context->list_mappings = rdfa_create_mapping(MAX_LIST_MAPPINGS);

   /* the maximum number of local list mappings */
   context->local_list_mappings =
      rdfa_create_mapping(MAX_LOCAL_LIST_MAPPINGS);

   /* the default vocabulary is set to null
    * (or a IRI defined in the initial context of the Host Language). */
   context->default_vocabulary = NULL;

   /* whether or not the @inlist attribute is present on the current element */
   context->inlist_present = 0;

   /* whether or not the @rel attribute is present on the current element */
   context->rel_present = 0;

   /* whether or not the @rev attribute is present on the current element */
   context->rev_present = 0;

   /* 1. First, the local values are initialized, as follows:
    *
    * * the [recurse] flag is set to 'true'; */
   context->recurse = 1;

   /* * the [skip element] flag is set to 'false'; */
   context->skip_element = 0;

   /* * [new subject] is set to null; */
   context->new_subject = NULL;

   /* * [current object resource] is set to null; */
   context->current_object_resource = NULL;

   /* * the [local list of URI mappings] is set to the list of URI
    *   mappings from the [evaluation context];
    *   NOTE: This step is done in rdfa_create_new_element_context() */

   /* FIXME: Initialize the term mappings and URI mappings based on Host Language */

   /* * the [local list of incomplete triples] is set to null; */
   context->local_incomplete_triples = rdfa_create_list(3);

   /* * the [current language] value is set to the [language] value
    *   from the [evaluation context].
    *   NOTE: This step is done in rdfa_create_new_element_context() */
}

#ifdef LIBRDFA_IN_RAPTOR
#define DECLARE_URI_MAPPING(context, prefix, value)                     \
do {                                                                    \
    raptor_namespace_stack* nstack = &context->sax2->namespaces;        \
    raptor_namespace* ns = raptor_new_namespace(nstack,                 \
      (const unsigned char *)prefix, (const unsigned char*)value, 0);   \
    raptor_namespaces_start_namespace(nstack, ns);                      \
    } while(0)
#else
#define DECLARE_URI_MAPPING(context, prefix, value)                     \
  rdfa_update_mapping(context->uri_mappings, prefix, value,             \
                      (update_mapping_value_fp)rdfa_replace_string)
#endif

void rdfa_setup_initial_context(rdfacontext* context)
{
#ifdef LIBRDFA_IN_RAPTOR
#else
   char* key = NULL;
   void* value = NULL;
   void** mptr = context->uri_mappings;
#endif

   /* Setup the base RDFa 1.1 prefix and term mappings */
   if(context->rdfa_version == RDFA_VERSION_1_1)
   {
      /* Setup the base RDFa 1.1 prefix mappings */
      DECLARE_URI_MAPPING(context,
         "grddl", "http://www.w3.org/2003/g/data-view#");
      DECLARE_URI_MAPPING(context,
         "ma", "http://www.w3.org/ns/ma-ont#");
      DECLARE_URI_MAPPING(context,
         "owl", "http://www.w3.org/2002/07/owl#");
      DECLARE_URI_MAPPING(context,
         "rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
      DECLARE_URI_MAPPING(context,
         "rdfa", "http://www.w3.org/ns/rdfa#");
      DECLARE_URI_MAPPING(context,
         "rdfs", "http://www.w3.org/2000/01/rdf-schema#");
      DECLARE_URI_MAPPING(context,
         "rif", "http://www.w3.org/2007/rif#");
      DECLARE_URI_MAPPING(context,
         "skos", "http://www.w3.org/2004/02/skos/core#");
      DECLARE_URI_MAPPING(context,
         "skosxl", "http://www.w3.org/2008/05/skos-xl#");
      DECLARE_URI_MAPPING(context,
         "wdr", "http://www.w3.org/2007/05/powder#");
      DECLARE_URI_MAPPING(context,
         "void", "http://rdfs.org/ns/void#");
      DECLARE_URI_MAPPING(context,
         "wdrs", "http://www.w3.org/2007/05/powder-s#");
      DECLARE_URI_MAPPING(context,
         "xhv", "http://www.w3.org/1999/xhtml/vocab#");
      DECLARE_URI_MAPPING(context,
         "xml", "http://www.w3.org/XML/1998/namespace");
      DECLARE_URI_MAPPING(context,
         "xsd", "http://www.w3.org/2001/XMLSchema#");
      DECLARE_URI_MAPPING(context,
         "cc", "http://creativecommons.org/ns#");
      DECLARE_URI_MAPPING(context,
         "ctag", "http://commontag.org/ns#");
      DECLARE_URI_MAPPING(context,
         "dc", "http://purl.org/dc/terms/");
      DECLARE_URI_MAPPING(context,
         "dcterms", "http://purl.org/dc/terms/");
      DECLARE_URI_MAPPING(context,
         "foaf", "http://xmlns.com/foaf/0.1/");
      DECLARE_URI_MAPPING(context,
         "gr", "http://purl.org/goodrelations/v1#");
      DECLARE_URI_MAPPING(context,
         "ical", "http://www.w3.org/2002/12/cal/icaltzd#");
      DECLARE_URI_MAPPING(context,
         "og", "http://ogp.me/ns#");
      DECLARE_URI_MAPPING(context,
         "rev", "http://purl.org/stuff/rev#");
      DECLARE_URI_MAPPING(context,
         "sioc", "http://rdfs.org/sioc/ns#");
      DECLARE_URI_MAPPING(context,
         "v", "http://rdf.data-vocabulary.org/#");
      DECLARE_URI_MAPPING(context,
         "vcard", "http://www.w3.org/2006/vcard/ns#");
      DECLARE_URI_MAPPING(context,
         "schema", "http://schema.org/");

      /* Setup the base RDFa 1.1 term mappings */
      rdfa_update_mapping(context->term_mappings,
         "describedby", "http://www.w3.org/2007/05/powder-s#describedby",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "license", "http://www.w3.org/1999/xhtml/vocab#license",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "role", "http://www.w3.org/1999/xhtml/vocab#role",
         (update_mapping_value_fp)rdfa_replace_string);
   }

   /* Setup the term mappings for XHTML1 */
   if(context->host_language == HOST_LANGUAGE_XHTML1)
   {
      rdfa_update_mapping(context->term_mappings,
         "alternate", "http://www.w3.org/1999/xhtml/vocab#alternate",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "appendix", "http://www.w3.org/1999/xhtml/vocab#appendix",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "cite", "http://www.w3.org/1999/xhtml/vocab#cite",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "bookmark", "http://www.w3.org/1999/xhtml/vocab#bookmark",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "contents", "http://www.w3.org/1999/xhtml/vocab#contents",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "chapter", "http://www.w3.org/1999/xhtml/vocab#chapter",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "copyright", "http://www.w3.org/1999/xhtml/vocab#copyright",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "first", "http://www.w3.org/1999/xhtml/vocab#first",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "glossary", "http://www.w3.org/1999/xhtml/vocab#glossary",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "help", "http://www.w3.org/1999/xhtml/vocab#help",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "icon", "http://www.w3.org/1999/xhtml/vocab#icon",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "index", "http://www.w3.org/1999/xhtml/vocab#index",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "last", "http://www.w3.org/1999/xhtml/vocab#last",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "license", "http://www.w3.org/1999/xhtml/vocab#license",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "meta", "http://www.w3.org/1999/xhtml/vocab#meta",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "next", "http://www.w3.org/1999/xhtml/vocab#next",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "prev", "http://www.w3.org/1999/xhtml/vocab#prev",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "previous", "http://www.w3.org/1999/xhtml/vocab#previous",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "section", "http://www.w3.org/1999/xhtml/vocab#section",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "start", "http://www.w3.org/1999/xhtml/vocab#start",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "stylesheet", "http://www.w3.org/1999/xhtml/vocab#stylesheet",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "subsection", "http://www.w3.org/1999/xhtml/vocab#subsection",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "top", "http://www.w3.org/1999/xhtml/vocab#top",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "up", "http://www.w3.org/1999/xhtml/vocab#up",
         (update_mapping_value_fp)rdfa_replace_string);
      rdfa_update_mapping(context->term_mappings,
         "p3pv1", "http://www.w3.org/1999/xhtml/vocab#p3pv1",
         (update_mapping_value_fp)rdfa_replace_string);

      /* From the role attribute module */
      rdfa_update_mapping(context->term_mappings,
         "role", "http://www.w3.org/1999/xhtml/vocab#role",
         (update_mapping_value_fp)rdfa_replace_string);
   }

   /* Setup the prefix and term mappings for HTML4 and HTML5 */
   if(context->host_language == HOST_LANGUAGE_HTML)
   {
      /* No term or prefix mappings as of 2012-04-04 */
   }

#ifdef LIBRDFA_IN_RAPTOR
   /* Raptor does this elsewhere */
#else
   /* Generate namespace triples for all values in the uri_mapping */
   while(*mptr != NULL)
   {
      rdfa_next_mapping(mptr++, &key, &value);
      mptr++;
      rdfa_generate_namespace_triple(context, key, value);
   }
#endif
}

/**
 * Creates a new context for the current element by cloning certain
 * parts of the old context on the top of the given stack.
 *
 * @param context_stack the context stack that is associated with this
 *                      processing run.
 */
rdfacontext* rdfa_create_new_element_context(rdfalist* context_stack)
{
   rdfacontext* parent_context = (rdfacontext*)
      context_stack->items[context_stack->num_items - 1]->data;
   rdfacontext* rval = rdfa_create_context(parent_context->base);

   if(!rval)
      return NULL;

   /* * Otherwise, the values are: */

   /* * the [ base ] is set to the [ base ] value of the current
    *   [ evaluation context ]; */
   rval->base = rdfa_replace_string(rval->base, parent_context->base);
   rdfa_init_context(rval);

   /* Set the processing depth as parent + 1 */
   rval->depth = parent_context->depth + 1;

   /* copy the URI mappings */
#ifdef LIBRDFA_IN_RAPTOR
   /* Raptor does this automatically for URIs */
#else
   rdfa_free_mapping(rval->uri_mappings, (free_mapping_value_fp)free);
#endif
   rdfa_free_mapping(rval->term_mappings, (free_mapping_value_fp)free);
   rdfa_free_mapping(rval->list_mappings, (free_mapping_value_fp)rdfa_free_list);
   rdfa_free_mapping(rval->local_list_mappings, (free_mapping_value_fp)rdfa_free_list);
#ifdef LIBRDFA_IN_RAPTOR
   /* Raptor does this automatically for URIs */
#else
   rval->uri_mappings =
      rdfa_copy_mapping((void**)parent_context->uri_mappings,
         (copy_mapping_value_fp)rdfa_replace_string);
#endif
   rval->term_mappings =
      rdfa_copy_mapping((void**)parent_context->term_mappings,
         (copy_mapping_value_fp)rdfa_replace_string);
   rval->list_mappings =
      rdfa_copy_mapping((void**)parent_context->local_list_mappings,
         (copy_mapping_value_fp)rdfa_replace_list);
   rval->local_list_mappings =
      rdfa_copy_mapping((void**)parent_context->local_list_mappings,
         (copy_mapping_value_fp)rdfa_replace_list);

   /* inherit the parent context's host language and RDFa processor mode */
   rval->host_language = parent_context->host_language;
   rval->rdfa_version = parent_context->rdfa_version;

   /* inherit the parent context's language */
   if(parent_context->language != NULL)
   {
      rval->language =
         rdfa_replace_string(rval->language, parent_context->language);
   }

   /* inherit the parent context's default vocabulary */
   if(parent_context->default_vocabulary != NULL)
   {
      rval->default_vocabulary = rdfa_replace_string(
         rval->default_vocabulary, parent_context->default_vocabulary);
   }

   /* set the callbacks callback */
   rval->default_graph_triple_callback =
      parent_context->default_graph_triple_callback;
   rval->processor_graph_triple_callback =
      parent_context->processor_graph_triple_callback;
   rval->buffer_filler_callback = parent_context->buffer_filler_callback;

   /* inherit the bnode count, _: bnode name, recurse flag, and state
    * of the xml_literal_namespace_insertion */
   rval->bnode_count = parent_context->bnode_count;
   rval->underscore_colon_bnode_name =
      rdfa_replace_string(rval->underscore_colon_bnode_name,
                          parent_context->underscore_colon_bnode_name);
   rval->recurse = parent_context->recurse;
   rval->skip_element = 0;
   rval->callback_data = parent_context->callback_data;
   rval->xml_literal_namespaces_defined =
      parent_context->xml_literal_namespaces_defined;
   rval->xml_literal_xml_lang_defined =
      parent_context->xml_literal_xml_lang_defined;

#if 0
   /* inherit the parent context's new_subject
    * TODO: This is not anywhere in the syntax processing document */
   if(parent_context->new_subject != NULL)
   {
      rval->new_subject = rdfa_replace_string(
         rval->new_subject, parent_context->new_subject);
   }
#endif

   if(parent_context->skip_element == 0)
   {
      /* o the [ parent subject ] is set to the value of [ new subject ],
       *   if non-null, or the value of the [ parent subject ] of the
       *   current [ evaluation context ]; */
      if(parent_context->new_subject != NULL)
      {
         rval->parent_subject = rdfa_replace_string(
            rval->parent_subject, parent_context->new_subject);
      }
      else
      {
         rval->parent_subject = rdfa_replace_string(
            rval->parent_subject, parent_context->parent_subject);
      }

      /* o the [ parent object ] is set to value of [ current object
       *   resource ], if non-null, or the value of [ new subject ], if
       *   non-null, or the value of the [ parent subject ] of the
       *   current [ evaluation context ]; */
      if(parent_context->current_object_resource != NULL)
      {
         rval->parent_object =
            rdfa_replace_string(
               rval->parent_object, parent_context->current_object_resource);
      }
      else if(parent_context->new_subject != NULL)
      {
         rval->parent_object =
            rdfa_replace_string(
               rval->parent_object, parent_context->new_subject);
      }
      else
      {
         rval->parent_object =
            rdfa_replace_string(
               rval->parent_object, parent_context->parent_subject);
      }

      /* o the [ list of incomplete triples ] is set to the [ local list
       *   of incomplete triples ]; */
      rval->incomplete_triples = rdfa_replace_list(
         rval->incomplete_triples, parent_context->local_incomplete_triples);
   }
   else
   {
      rval->parent_subject = rdfa_replace_string(
         rval->parent_subject, parent_context->parent_subject);
      rval->parent_object = rdfa_replace_string(
         rval->parent_object, parent_context->parent_object);

      /* copy the incomplete triples */
      rval->incomplete_triples = rdfa_replace_list(
         rval->incomplete_triples, parent_context->incomplete_triples);

      /* copy the local list of incomplete triples */
      rval->local_incomplete_triples = rdfa_replace_list(
         rval->local_incomplete_triples,
         parent_context->local_incomplete_triples);
   }

#ifdef LIBRDFA_IN_RAPTOR
   rval->base_uri = parent_context->base_uri;
   rval->sax2     = parent_context->sax2;
   rval->namespace_handler = parent_context->namespace_handler;
   rval->namespace_handler_user_data = parent_context->namespace_handler_user_data;
#endif

   return rval;
}

void rdfa_free_context_stack(rdfacontext* context)
{
   /* this field is not NULL only on the rdfacontext* at the top of the stack */
   if(context->context_stack != NULL)
   {
      void* rval;
      /* free the stack ensuring that we do not delete this context if
       * it is in the list (which it may be, if parsing ended on error) */
      do
      {
         rval = rdfa_pop_item(context->context_stack);
         if(rval && rval != context)
         {
            rdfa_free_context((rdfacontext*)rval);
         }
      }
      while(rval);
      free(context->context_stack->items);
      free(context->context_stack);
      context->context_stack = NULL;
   }
}

void rdfa_free_context(rdfacontext* context)
{
   free(context->base);
   free(context->default_vocabulary);
   free(context->parent_subject);
   free(context->parent_object);

#ifdef LIBRDFA_IN_RAPTOR
#else
   rdfa_free_mapping(context->uri_mappings, (free_mapping_value_fp)free);
#endif

   rdfa_free_mapping(context->term_mappings, (free_mapping_value_fp)free);
   rdfa_free_list(context->incomplete_triples);
   rdfa_free_mapping(context->list_mappings,
      (free_mapping_value_fp)rdfa_free_list);
   rdfa_free_mapping(context->local_list_mappings,
      (free_mapping_value_fp)rdfa_free_list);
   free(context->language);
   free(context->underscore_colon_bnode_name);
   free(context->new_subject);
   free(context->current_object_resource);
   free(context->about);
   free(context->typed_resource);
   free(context->resource);
   free(context->href);
   free(context->src);
   free(context->content);
   free(context->datatype);
   rdfa_free_list(context->property);
   free(context->plain_literal);
   free(context->xml_literal);

   /* TODO: These should be moved into their own data structure */
   rdfa_free_list(context->local_incomplete_triples);

   rdfa_free_context_stack(context);
   free(context->working_buffer);
   free(context);
}
