/**
 * Copyright 2008 Digital Bazaar, Inc.
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
 *
 * Usage:
 *
 *    rdfacontext* context = rdfa_create_context(BASE_URI);
 *    context->callback_data = your_user_data;
 *    rdfa_set_triple_handler(context, &process_triple);
 *    rdfa_set_buffer_filler(context, &fill_buffer);
 *    rdfa_parse(context);
 *    rdfa_free_context(context);
 *
 * @author Manu Sporny
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rdfa_utils.h"
#include "rdfa.h"

#define READ_BUFFER_SIZE 4096

static void rdfa_init_context(rdfacontext* context)
{   
   // the [parent subject] is set to the [base] value;
   context->parent_subject = NULL;
   if(context->base != NULL)
   {
      context->parent_subject =
         rdfa_replace_string(context->parent_subject, context->base);
   }
   
   // the [parent object] is set to null;
   context->parent_object = NULL;
   
#ifndef LIBRDFA_IN_RAPTOR
   // the [list of URI mappings] is cleared;
   context->uri_mappings = (char**)rdfa_create_mapping(MAX_URI_MAPPINGS);
#endif   

   // the [list of incomplete triples] is cleared;
   context->incomplete_triples = rdfa_create_list(3);
   
   // the [language] is set to null.
   context->language = NULL;
   
   // set the [current object resource] to null;
   context->current_object_resource = NULL;

   // 1. First, the local values are initialized, as follows:
   //
   // * the [recurse] flag is set to 'true';
   context->recurse = 1;

   // * the [skip element] flag is set to 'false';
   context->skip_element = 0;
      
   // * [new subject] is set to null;
   context->new_subject = NULL;

   // * [current object resource] is set to null;
   context->current_object_resource = NULL;
   
   // * the [local list of URI mappings] is set to the list of URI
   //   mappings from the [evaluation context];
   //   NOTE: This step is done in rdfa_create_new_element_context()

   // * the [local list of incomplete triples] is set to null;
   context->local_incomplete_triples = rdfa_create_list(3);

   // * the [current language] value is set to the [language] value
   //   from the [evaluation context].
   //   NOTE: This step is done in rdfa_create_new_element_context()
   
   // The next set of variables are initialized to make the C compiler
   // and valgrind happy - they are not a part of the RDFa spec.
   context->bnode_count = 0;
   context->content = NULL;
   context->datatype = NULL;
   context->property = NULL;
   context->plain_literal = NULL;
   context->xml_literal = NULL;
   context->triples_generated = 0;
   context->complete_incomplete_triples = 0;
   //context->callback_data = NULL;
}

/**
 * Read the head of the XHTML document and determines the base IRI for
 * the document.
 *
 * @param context the current working context.
 * @param working_buffer the current working buffer.
 * @param wb_allocated the number of bytes that have been allocated to
 *                     the working buffer.
 *
 * @return the size of the data available in the working buffer.
 */
static size_t rdfa_init_base(
   rdfacontext* context, char** working_buffer, size_t* working_buffer_size,
   char* temp_buffer, size_t bytes_read)
{
   size_t temp_buffer_size = sizeof(char) * READ_BUFFER_SIZE;
   char* head_end = NULL;
   size_t offset = context->wb_offset;

   // search for the end of <head>, stop if <head> was found
   // extend the working buffer size
   if((offset + bytes_read) > *working_buffer_size)
   {
      *working_buffer_size += temp_buffer_size;
      *working_buffer = (char*)realloc(working_buffer, *working_buffer_size);
   }
   
   // append to the working buffer
   memmove(*working_buffer + offset, temp_buffer, bytes_read);
 
   // search for the end of </head> in 
   head_end = strstr(*working_buffer, "</head>");
   if(head_end == NULL)
      head_end = strstr(*working_buffer, "</HEAD>");
   
   context->wb_offset += bytes_read;

   if(head_end == NULL)
      return bytes_read;
   
   // if </head> was found, search for <base and extract the base URI
   if(head_end != NULL)
   {
      char* base_start = strstr(*working_buffer, "<base ");
      if(base_start == NULL)
         base_start = strstr(*working_buffer, "<BASE ");
      
      if(base_start != NULL)
      {
         char* href_start = strstr(base_start, "href=");
         char* uri_start = href_start + 6;
         char* uri_end = index(uri_start, '"');

         if((uri_start != NULL) && (uri_end != NULL))
         {
            if(*uri_start != '"')
            {
               size_t uri_size = uri_end - uri_start;
               char* temp_uri = malloc(sizeof(char) * uri_size + 1);
               strncpy(temp_uri, uri_start, uri_size);
               temp_uri[uri_size] = '\0';

               // TODO: This isn't in the processing rules, should it
               //       be? Setting current_object_resource will make
               //       sure that the BASE element is inherited by all
               //       subcontexts.
               context->current_object_resource =
                  rdfa_replace_string(context->current_object_resource,
                                      temp_uri);
               context->base =
                  rdfa_replace_string(context->base,
                                      temp_uri);
               
               free(temp_uri);
            }
         }         
      }
   }
   
   return bytes_read;
}

/**
 * Creates a new context for the current element by cloning certain
 * parts of the old context on the top of the given stack.
 *
 * @param context_stack the context stack that is associated with this
 *                      processing run.
 */
static rdfacontext* rdfa_create_new_element_context(rdfalist* context_stack)
{
   rdfacontext* parent_context = (rdfacontext*)
      context_stack->items[context_stack->num_items - 1]->data;
   rdfacontext* rval = rdfa_create_context(parent_context->base);

   // 10. If the [recurse] flag is 'true', all elements that are
   // children of the [current element] are processed using the rules
   // described here, using a new [evaluation context], initialized as follows:
   //
   // If the [skip element] flag is 'true' then the new
   // [evaluation context] is a copy of the current context that was
   // passed in to this level of processing, with the [language] and
   // [list of URI mappings] values replaced with the local values;
   //
   // TODO: Don't know if this is quite correct... should probably
   // check [skip element] later on in this function.
   
   // initialize the context
   rval->base = rdfa_replace_string(rval->base, parent_context->base);
   rdfa_init_context(rval);

   // copy the URI mappings
#ifndef LIBRDFA_IN_RAPTOR
   if(rval->uri_mappings != NULL)
   {
      rdfa_free_mapping(rval->uri_mappings);
   }
   rval->uri_mappings = rdfa_copy_mapping(parent_context->uri_mappings);
#endif

   // inherit the parent context's language
   if(parent_context->language != NULL)
   {
      rval->language =
         rdfa_replace_string(rval->language, parent_context->language);
   }

   // set the triple callback
   rval->triple_callback = parent_context->triple_callback;
   rval->buffer_filler_callback = parent_context->buffer_filler_callback;

   // set the bnode count and inherit the recurse flag
   rval->bnode_count = parent_context->bnode_count;
   rval->recurse = parent_context->recurse;
   rval->skip_element = 0;
   rval->callback_data = parent_context->callback_data;
   
   // inherit the parent context's new_subject
   // TODO: This is not anywhere in the syntax processing document
   //if(parent_context->new_subject != NULL)
   //{
   //   rval->new_subject = rdfa_replace_string(
   //      rval->new_subject, parent_context->new_subject);
   //}
   
   if(parent_context->skip_element == 0)
   {
      // the [parent subject] is set to the value of [new subject], if
      // non-null, or the value of the [parent subject] of the current
      // [evaluation context];
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
      
      // the [parent object] is set to value of
      // [current object resource], if non-null, or the value of
      // [new subject], if non-null, or the value of the
      // [parent subject] of the current [evaluation context];
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
      
      // copy the incomplete triples
      if(rval->incomplete_triples != NULL)
      {
         rdfa_free_list(rval->incomplete_triples);
      }
   
      rval->incomplete_triples =
         rdfa_copy_list(parent_context->local_incomplete_triples);
   }
   else
   {
      rval->parent_subject = rdfa_replace_string(
         rval->parent_subject, parent_context->parent_subject);
      rval->parent_object = rdfa_replace_string(
         rval->parent_object, parent_context->parent_object);

      // copy the incomplete triples
      if(rval->incomplete_triples != NULL)
      {
         rdfa_free_list(rval->incomplete_triples);
      }
   
      rval->incomplete_triples =
         rdfa_copy_list(parent_context->incomplete_triples);

      // copy the local list of incomplete triples
      if(rval->local_incomplete_triples != NULL)
      {
         rdfa_free_list(rval->local_incomplete_triples);
      }
   
      rval->local_incomplete_triples =
         rdfa_copy_list(parent_context->local_incomplete_triples);
   }

#ifdef LIBRDFA_IN_RAPTOR
   rval->base_uri = parent_context->base_uri;
   rval->sax2     = parent_context->sax2;
   rval->namespace_handler = parent_context->namespace_handler;
   rval->namespace_handler_user_data = parent_context->namespace_handler_user_data;
#endif
   
   return rval;
}

/**
 * Handles the start_element call
 */
static void XMLCALL
   start_element(void* user_data, const char* name, const char** attributes)
{
   rdfalist* context_stack = user_data;
   rdfacontext* context = rdfa_create_new_element_context(context_stack);
   const char** aptr = attributes;
   const char* xml_lang = NULL;
   const char* about_curie = NULL;
   char* about = NULL;
   const char* src_curie = NULL;
   char* src = NULL;
   const char* type_of_curie = NULL;
   rdfalist* type_of = NULL;
   const char* rel_curie = NULL;
   rdfalist* rel = NULL;
   const char* rev_curie = NULL;
   rdfalist* rev = NULL;
   const char* property_curie = NULL;
   rdfalist* property = NULL;
   const char* resource_curie = NULL;
   char* resource = NULL;
   const char* href_curie = NULL;
   char* href = NULL;
   const char* content = NULL;
   const char* datatype_curie = NULL;
   char* datatype = NULL;

   rdfa_push_item(context_stack, context, RDFALIST_FLAG_CONTEXT);

   if(DEBUG)
   {
      printf("DEBUG: ------- START - %s -------\n", name);
   }
   
   // start the XML Literal text
   if(context->xml_literal == NULL)
   {
      context->xml_literal = rdfa_replace_string(context->xml_literal, "<");
   }
   else
   {
      context->xml_literal = rdfa_append_string(context->xml_literal, "<");
   }
   context->xml_literal = rdfa_append_string(context->xml_literal, name);

   // prepare all of the RDFa-specific attributes we are looking for.

   // scan all of the attributes for the RDFa-specific attributes
   if(aptr != NULL)
   {
      while(*aptr != NULL)
      {
         const char* attr;
         const char* value;
         char* literal_text;

         attr = *aptr++;
         value = *aptr++;

         // append the attribute-value pair to the XML literal
         literal_text = malloc(strlen(attr) + strlen(value) + 5);
         sprintf(literal_text, " %s=\"%s\"", attr, value);
         context->xml_literal =
            rdfa_append_string(context->xml_literal, literal_text);
         free(literal_text);
         
         if(strcmp(attr, "about") == 0)
         {
            about_curie = value;
            about = rdfa_resolve_curie(
               context, about_curie, CURIE_PARSE_ABOUT_RESOURCE);
         }
         else if(strcmp(attr, "src") == 0)
         {
            src_curie = value;
            src = rdfa_resolve_curie(context, src_curie, CURIE_PARSE_HREF_SRC);
         }
         else if(strcmp(attr, "typeof") == 0)
         {
            type_of_curie = value;
            type_of = rdfa_resolve_curie_list(
               context, type_of_curie,
               CURIE_PARSE_INSTANCEOF_DATATYPE);
         }
         else if(strcmp(attr, "rel") == 0)
         {
            rel_curie = value;
            rel = rdfa_resolve_curie_list(
               context, rel_curie, CURIE_PARSE_RELREV);
         }
         else if(strcmp(attr, "rev") == 0)
         {
            rev_curie = value;
            rev = rdfa_resolve_curie_list(
               context, rev_curie, CURIE_PARSE_RELREV);
         }
         else if(strcmp(attr, "property") == 0)
         {
            property_curie = value;
            property =
               rdfa_resolve_curie_list(
                  context, property_curie, CURIE_PARSE_PROPERTY);
         }
         else if(strcmp(attr, "resource") == 0)
         {
            resource_curie = value;
            resource = rdfa_resolve_curie(
               context, resource_curie, CURIE_PARSE_ABOUT_RESOURCE);
         }
         else if(strcmp(attr, "href") == 0)
         {
            href_curie = value;
            href =
               rdfa_resolve_curie(context, href_curie, CURIE_PARSE_HREF_SRC);
         }
         else if(strcmp(attr, "content") == 0)
         {
            content = value;
         }
         else if(strcmp(attr, "datatype") == 0)
         {
            datatype_curie = value;
            datatype = rdfa_resolve_curie(context, datatype_curie,
               CURIE_PARSE_INSTANCEOF_DATATYPE);
         }
         else if(strcmp(attr, "xml:lang") == 0)
         {
            xml_lang = value;
         }
#ifdef LIBRDFA_IN_RAPTOR
         else if(strstr(attr, "xmlns") != NULL)
         {
            // 2. Next the [current element] is parsed for
            //    [URI mapping]s and these are added to the
            //    [local list of URI mappings]. Note that a
            //    [URI mapping] will simply overwrite any current
            //    mapping in the list that has the same name;
            rdfa_update_uri_mappings(context, attr, value);
         }
#endif
      }
   }

   // close the XML Literal value
   context->xml_literal = rdfa_append_string(context->xml_literal, ">");
   
   // 3. The [current element] is also parsed for any language
   //    information, and [language] is set in the [current
   //    evaluation context];
#ifdef LIBRDFA_IN_RAPTOR
   if(context->sax2)
      rdfa_update_language(context, (const char*)raptor_sax2_inscope_xml_language(context->sax2));
#else
   rdfa_update_language(context, xml_lang);
#endif

   /***************** FOR DEBUGGING PURPOSES ONLY ******************/
   if(DEBUG)
   {
      if(about != NULL)
      {
         printf("DEBUG: @about = %s\n", about);
      }
      if(src != NULL)
      {
         printf("DEBUG: @src = %s\n", src);
      }
      if(type_of != NULL)
      {
         printf("DEBUG: @type_of = ");
         rdfa_print_list(type_of);
      }
      if(rel != NULL)
      {
         printf("DEBUG: @rel = ");
         rdfa_print_list(rel);
      }
      if(rev != NULL)
      {
         printf("DEBUG: @rev = ");
         rdfa_print_list(rev);
      }
      if(property != NULL)
      {
         printf("DEBUG: @property = ");
         rdfa_print_list(property);
      }
      if(resource != NULL)
      {
         printf("DEBUG: @resource = %s\n", resource);
      }
      if(href != NULL)
      {
         printf("DEBUG: @href = %s\n", href);
      }
      if(content != NULL)
      {
         printf("DEBUG: @content = %s\n", content);
      }
      if(datatype != NULL)
      {
         printf("DEBUG: @datatype = %s\n", datatype);
      }
   }

   // TODO: This isn't part of the processing model, it needs to be
   // included and is a correction for the last item in step #4.
   if((about == NULL) && (src == NULL) && (type_of == NULL) &&
      (rel == NULL) && (rev == NULL) && (property == NULL) &&
      (resource == NULL) && (href == NULL))
   {
      context->skip_element = 1;
   }
   
   if((rel == NULL) && (rev == NULL))
   {
      // 4. If the [current element] contains no valid @rel or @rev
      // URI, obtained according to the section on CURIE and URI
      // Processing, then the next step is to establish a value for
      // [new subject]. Any of the attributes that can carry a
      // resource can set [new subject];
      rdfa_establish_new_subject(
         context, name, about, src, resource, href, type_of);
   }
   else
   {
      // 5. If the [current element] does contain a valid @rel or @rev
      // URI, obtained according to the section on CURIE and URI
      // Processing, then the next step is to establish both a value
      // for [new subject] and a value for [current object resource]:
      rdfa_establish_new_subject_with_relrev(
         context, name, about, src, resource, href, type_of);
   }

   if(context->new_subject != NULL)
   {
      if(DEBUG)
      {
         printf("DEBUG: new_subject = %s\n", context->new_subject);
      }
      
      // 6. If in any of the previous steps a [new subject] was set to
      // a non-null value,
      
      // it is now used to provide a subject for type values;
      if(type_of != NULL)
      {
         rdfa_complete_type_triples(context, type_of);
      }
      
      // Note that none of this block is executed if there is no
      // [new subject] value, i.e., [new subject] remains null.
   }
   
   if(context->current_object_resource != NULL)
   {
      // 7. If in any of the previous steps a [current object  resource]
      // was set to a non-null value, it is now used to generate triples
      rdfa_complete_relrev_triples(context, rel, rev);
   }
   else if((rel != NULL) || (rev != NULL))
   {
      // 8. If however [current object resource] was set to null, but
      // there are predicates present, then they must be stored as
      // [incomplete triple]s, pending the discovery of a subject that
      // can be used as the object. Also, [current object resource]
      // should be set to a newly created [bnode]
      rdfa_save_incomplete_triples(context, rel, rev);
   }

   // save these for processing steps #9 and #10
   context->property = property;
   context->content = rdfa_replace_string(context->datatype, content);
   context->datatype = rdfa_replace_string(context->datatype, datatype);
   
   // free the resolved CURIEs
   free(about);
   free(src);
   rdfa_free_list(type_of);
   rdfa_free_list(rel);
   rdfa_free_list(rev);
   free(resource);
   free(href);
   free(datatype);
}

static void XMLCALL character_data(void *user_data, const char *s, int len)
{
   rdfalist* context_stack = user_data;
   rdfacontext* context = (rdfacontext*)
      context_stack->items[context_stack->num_items - 1]->data;
   
   char *buffer = malloc(len + 1);
   memset(buffer, 0, len + 1);
   memcpy(buffer, s, len);   

   // append the text to the current context's plain literal
   if(context->plain_literal == NULL)
   {
      context->plain_literal =
         rdfa_replace_string(context->plain_literal, buffer);
   }
   else
   {
      context->plain_literal =
         rdfa_append_string(context->plain_literal, buffer);
   }

   // append the text to the current context's XML literal
   if(context->xml_literal == NULL)
   {
      context->xml_literal =
         rdfa_replace_string(context->xml_literal, buffer);
   }
   else
   {
      context->xml_literal =
         rdfa_append_string(context->xml_literal, buffer);
   }

   //printf("plain_literal: %s\n", context->plain_literal);
   //printf("xml_literal: %s\n", context->xml_literal);
   
   free(buffer);
}

static void XMLCALL
   end_element(void *user_data, const char *name)
{
   rdfalist* context_stack = user_data;
   rdfacontext* context = (rdfacontext*)rdfa_pop_item(context_stack);
   rdfacontext* parent_context = (rdfacontext*)
      context_stack->items[context_stack->num_items - 1]->data;
   
   // append the text to the current context's XML literal
   char* buffer = malloc(strlen(name) + 4);
   
   if(DEBUG)
   {
      printf("DEBUG: </%s>\n", name);
   }
   
   sprintf(buffer, "</%s>", name);
   if(context->xml_literal == NULL)
   {
      context->xml_literal =
         rdfa_replace_string(context->xml_literal, buffer);
   }
   else
   {
      context->xml_literal =
         rdfa_append_string(context->xml_literal, buffer);
   }
   free(buffer);
   
   // 9. The next step of the iteration is to establish any
   // [current object literal];

   // generate the complete object literal triples
   if(context->property != NULL)
   {
      char* saved_xml_literal = context->xml_literal;
      char* working_xml_literal = NULL;

      // ensure to mark only the inner-content of the XML node for
      // processing the object literal.
      buffer = NULL;

      if(context->xml_literal != NULL)
      {
         char* content_start = NULL;
         char* content_end = NULL;
      
         saved_xml_literal = context->xml_literal;
         context->xml_literal = NULL;

         // mark the beginning and end of the enclosing XML element
         context->xml_literal =
            rdfa_replace_string(context->xml_literal, saved_xml_literal);
         content_start = index(context->xml_literal, '>');
         content_end = rindex(context->xml_literal, '<');

         if((content_start != NULL) && (content_end != NULL))
         {
            working_xml_literal = context->xml_literal;
            context->xml_literal = ++content_start;
            *content_end = '\0';
         }
      }

      rdfa_complete_object_literal_triples(context);

      if(saved_xml_literal != NULL)
      {
         free(working_xml_literal);
         context->xml_literal = saved_xml_literal;
      }
   }

   //printf(context->plain_literal);

   // append the XML literal and plain text literals to the parent
   // literals
   if(context->xml_literal != NULL)
   {
      if(parent_context->xml_literal == NULL)
      {
         parent_context->xml_literal =
            rdfa_replace_string(
               parent_context->xml_literal, context->xml_literal);
      }
      else
      {
         parent_context->xml_literal =
            rdfa_append_string(
               parent_context->xml_literal, context->xml_literal);
      }      

      // if there is an XML literal, there is probably a plain literal
      if(context->plain_literal != NULL)
      {
         if(parent_context->plain_literal == NULL)
         {
            parent_context->plain_literal =
               rdfa_replace_string(
                  parent_context->plain_literal, context->plain_literal);
         }
         else
         {
            parent_context->plain_literal =
               rdfa_append_string(
                  parent_context->plain_literal, context->plain_literal);
         }
      }
   }


   
   // 11. If the [skip element] flag is 'false', and either: the
   // previous step resulted in a 'true' flag, or [new subject] was
   // set to a non-null value, then any [incomplete triple]s within
   // the current context should be completed:
   if((context->skip_element == 0) &&
      (context->complete_incomplete_triples || (context->new_subject != NULL)))
   {
      rdfa_complete_incomplete_triples(context);
   }

   // 12.  If any triples were created during the current level of
   // processing, or [new subject] was set to a non-null and non-bnode
   // value, or a value of 'true' was returned from the recursion step
   // (step 10, above), then a value of 'true' should be returned from
   // this level of processing. Otherwise a value of false should be
   // returned. This returned value is forwarded to step 10 and
   // determines whether to complete any [incomplete triple]s after
   // having recursed into the processing of descendants.
   if(context->triples_generated ||
      ((context->new_subject != NULL) && (context->new_subject[0] != '_') &&
       (context->new_subject[1] != ':')) ||
      context->complete_incomplete_triples == 1)
   {
      parent_context->complete_incomplete_triples = 1;
   }
   
   // free the context
   rdfa_free_context(context);
}


#ifdef LIBRDFA_IN_RAPTOR
static void raptor_rdfa_start_element(void *user_data,
                                      raptor_xml_element *xml_element) 
{
  raptor_qname* qname=raptor_xml_element_get_name(xml_element);
  int attr_count=raptor_xml_element_get_attributes_count(xml_element);
  raptor_qname** attrs=raptor_xml_element_get_attributes(xml_element);
  const char* name=(const char*)raptor_qname_get_local_name(qname);
  char** attr=NULL;
  int i;

  if(attr_count > 0) {
    attr=(char**)malloc(sizeof(char*) * (1+(attr_count*2)));
    for(i=0; i<attr_count; i++) {
      attr[2*i]=(char*)raptor_qname_get_local_name(attrs[i]);
      attr[1+(2*i)]=(char*)raptor_qname_get_value(attrs[i]);
    }
    attr[2*i]=NULL;
  }
  start_element(user_data, name, (const char**)attr);
  if(attr)
    free(attr);
}

static void raptor_rdfa_end_element(void *user_data,
                                    raptor_xml_element* xml_element) 
{
  raptor_qname* qname=raptor_xml_element_get_name(xml_element);
  const char* name=(const char*)raptor_qname_get_local_name(qname);

  end_element(user_data, name);
}

static void raptor_rdfa_character_data(void *user_data, 
                                       raptor_xml_element* xml_element,
                                       const unsigned char *s, int len) 
{
  character_data(user_data, (const char *)s, len);
}

static void raptor_rdfa_namespace_handler(void *user_data,
                                          raptor_namespace* nspace)
{
  rdfalist* context_stack = user_data;
  rdfacontext* context = (rdfacontext*)
    context_stack->items[context_stack->num_items - 1]->data;

  if(context->namespace_handler)
    (*context->namespace_handler)(context->namespace_handler_user_data, 
                                  nspace);
}



#endif


rdfacontext* rdfa_create_context(const char* base)
{
   rdfacontext* rval = NULL;
   size_t base_length = strlen(base);

   // if the base isn't specified, don't create a context
   if(base_length > 0)
   {
      rval = malloc(sizeof(rdfacontext));
      rval->base = NULL;
      rval->base = rdfa_replace_string(rval->base, base);

      /* parse state */
      rval->wb_allocated = 0;
      rval->working_buffer = NULL;
      rval->wb_offset = 0;
#ifdef LIBRDFA_IN_RAPTOR
      rval->base_uri = NULL;
      rval->sax2 = NULL;
      rval->namespace_handler = NULL;
      rval->namespace_handler_user_data = NULL;
#else
      rval->parser = NULL;
#endif
      rval->done = 0;
      rval->context_stack = NULL;
      rval->wb_preread = 0;
      rval->preread = 0;
   }
   else
   {
      printf("OMG!\n");
   }
   
   return rval;
}

void rdfa_free_context(rdfacontext* context)
{
   if(context->base)
   {
      free(context->base);
   }
   
   if(context->parent_subject != NULL)
   {
      free(context->parent_subject);
   }

   if(context->parent_object != NULL)
   {
      free(context->parent_object);
   }

#ifndef LIBRDFA_IN_RAPTOR
   if(context->uri_mappings != NULL)
   {
      rdfa_free_mapping(context->uri_mappings);
   }
#endif

   if(context->incomplete_triples != NULL)
   {
      rdfa_free_list(context->incomplete_triples);
   }
   
   if(context->language != NULL)
   {
      free(context->language);
   }

   if(context->new_subject != NULL)
   {
      free(context->new_subject);
   }

   if(context->current_object_resource != NULL)
   {
      free(context->current_object_resource);
   }

   if(context->content != NULL)
   {
      free(context->content);
   }
   
   if(context->datatype != NULL)
   {
      free(context->datatype);
   }

   if(context->property != NULL)
   {
      rdfa_free_list(context->property);
   }

   if(context->plain_literal != NULL)
   {
      free(context->plain_literal);
   }

   if(context->xml_literal != NULL)
   {
      free(context->xml_literal);
   }

   // TODO: These should be moved into their own data structure
   if(context->local_incomplete_triples != NULL)
   {
      rdfa_free_list(context->local_incomplete_triples);
   }

   if(context->context_stack != NULL)
   {
      // TODO: This leaks memory! Must be fixed!
      //rdfa_free_list(context->context_stack);
   }

   if(context->working_buffer != NULL)
   {
      free(context->working_buffer);
   }
   
   free(context);
}

void rdfa_set_triple_handler(rdfacontext* context, triple_handler_fp th)
{
   context->triple_callback = th;
}

void rdfa_set_buffer_filler(rdfacontext* context, buffer_filler_fp bf)
{
   context->buffer_filler_callback = bf;
}

int rdfa_parse_start(rdfacontext* context)
{
   // create the buffers and expat parser
   int rval = RDFA_PARSE_SUCCESS;
   
   context->wb_allocated = sizeof(char) * READ_BUFFER_SIZE;
   context->working_buffer = calloc(context->wb_allocated, sizeof(char));

#ifdef LIBRDFA_IN_RAPTOR
   raptor_error_handlers_init(&context->error_handlers);
#else
   context->parser = XML_ParserCreate(NULL);
#endif
   context->done = 0;
   context->context_stack = rdfa_create_list(32);

   // initialize the context stack
   rdfa_push_item(context->context_stack, context, RDFALIST_FLAG_CONTEXT);
   
#ifdef LIBRDFA_IN_RAPTOR
   context->sax2 = raptor_new_sax2(context->context_stack,
                                   &context->error_handlers);
#else
#endif

   // set up the context stack
#ifdef LIBRDFA_IN_RAPTOR
   raptor_sax2_set_start_element_handler(context->sax2,
                                         raptor_rdfa_start_element);
   raptor_sax2_set_end_element_handler(context->sax2,
                                       raptor_rdfa_end_element);
   raptor_sax2_set_characters_handler(context->sax2,
                                      raptor_rdfa_character_data);
   raptor_sax2_set_namespace_handler(context->sax2,
                                     raptor_rdfa_namespace_handler);
#else
   XML_SetUserData(context->parser, context->context_stack);
   XML_SetElementHandler(context->parser, start_element, end_element);
   XML_SetCharacterDataHandler(context->parser, character_data);
#endif

   rdfa_init_context(context);

#ifdef LIBRDFA_IN_RAPTOR
   context->base_uri=raptor_new_uri((const unsigned char*)context->base);
   raptor_sax2_parse_start(context->sax2, context->base_uri);
#endif

   return rval;
}

int rdfa_parse_chunk(rdfacontext* context, char* data, size_t wblen, int done)
{
   // it is an error to call this before rdfa_parse_start()
   if(context->done)
   {
      return RDFA_PARSE_FAILED;
   }
   
   if(!context->preread)
   {
      // search for the <base> tag and use the href contained therein to
      // set the parsing context.
      context->wb_preread = rdfa_init_base(context,
         &context->working_buffer, &context->wb_allocated, data, wblen);
      
      // continue looking if in first 131072 bytes of data
      if(!context->base && context->wb_preread < (1<<17))
         return RDFA_PARSE_SUCCESS;

#ifdef LIBRDFA_IN_RAPTOR

      if(raptor_sax2_parse_chunk(context->sax2,
                                 (const unsigned char*)context->working_buffer,
                                 context->wb_offset, done))
      {
         return RDFA_PARSE_FAILED;
      }
#else
      if(XML_Parse(context->parser, context->working_buffer,
         context->wb_offset, 0) == XML_STATUS_ERROR)
      {
         fprintf(stderr,
                 "%s at line %d, column %d\n",
                 XML_ErrorString(XML_GetErrorCode(context->parser)),
                 XML_GetCurrentLineNumber(context->parser),
                 XML_GetCurrentColumnNumber(context->parser));
         return RDFA_PARSE_FAILED;
      }
#endif
      
      context->preread = 1;
      
      return RDFA_PARSE_SUCCESS;
   }

   // otherwise just parse the block passed in
#ifdef LIBRDFA_IN_RAPTOR
   if(raptor_sax2_parse_chunk(context->sax2, (const unsigned char*)data, wblen, done))
   {
      return RDFA_PARSE_FAILED;
   }
#else
   if(XML_Parse(context->parser, data, wblen, done) == XML_STATUS_ERROR)
   {
      fprintf(stderr,
              "%s at line %d, column %d.\n",
              XML_ErrorString(XML_GetErrorCode(context->parser)),
              XML_GetCurrentLineNumber(context->parser),
              XML_GetCurrentColumnNumber(context->parser));
      return RDFA_PARSE_FAILED;
   }
#endif   

   return RDFA_PARSE_SUCCESS;
}

void rdfa_parse_end(rdfacontext* context)
{
   // Free the expat parser and the like
#ifdef LIBRDFA_IN_RAPTOR
   if(context->base_uri)
      raptor_free_uri(context->base_uri);
   raptor_free_sax2(context->sax2);
   context->sax2=NULL;
#else
   XML_ParserFree(context->parser);
#endif
}

int rdfa_parse(rdfacontext* context)
{
  int rval;

  rval = rdfa_parse_start(context);
  if(rval != RDFA_PARSE_SUCCESS)
  {
    context->done = 1;
    return rval;
  }

  do
  {
     size_t wblen;
     int done;
     
     wblen = context->buffer_filler_callback(
        context->working_buffer, context->wb_allocated,
        context->callback_data);
     done = (wblen == 0);
     
     rval = rdfa_parse_chunk(
        context, context->working_buffer, wblen, done);
     context->done=done;
  }
  while(!context->done && rval == RDFA_PARSE_SUCCESS);

  rdfa_parse_end(context);
  
  return rval;
}
