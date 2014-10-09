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
 * Handles all triple functionality including all incomplete triple
 * functionality.
 *
 * @author Manu Sporny
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rdfa_utils.h"
#include "rdfa.h"

rdftriple* rdfa_create_triple(const char* subject, const char* predicate,
   const char* object, rdfresource_t object_type, const char* datatype,
   const char* language)
{
   rdftriple* rval = (rdftriple*)malloc(sizeof(rdftriple));

   /* clear the memory */
   rval->subject = NULL;
   rval->predicate = NULL;
   rval->object = NULL;
   rval->object_type = object_type;
   rval->datatype = NULL;
   rval->language = NULL;

#if 0
   printf("SUBJECT  : %s\n", subject);
   printf("PREDICATE: %s\n", predicate);
   printf("OBJECT   : %s\n", object);
   printf("DATATYPE : %s\n", datatype);
   printf("LANG     : %s\n", language);
#endif

   /* a triple needs a subject, predicate and object at minimum to be
    * considered a triple. */
   if((subject != NULL) && (predicate != NULL) && (object != NULL))
   {
      rval->subject = rdfa_replace_string(rval->subject, subject);
      rval->predicate = rdfa_replace_string(rval->predicate, predicate);
      rval->object = rdfa_replace_string(rval->object, object);

      /* if the datatype is present, set it */
      if(datatype != NULL)
      {
         rval->datatype = rdfa_replace_string(rval->datatype, datatype);
      }

      /* if the language was specified, set it */
      if(language != NULL)
      {
         rval->language = rdfa_replace_string(rval->language, language);
      }
   }

   return rval;
}

void rdfa_print_triple(rdftriple* triple)
{
   if(triple->object_type == RDF_TYPE_NAMESPACE_PREFIX)
   {
      printf("%s %s: <%s> .\n",
         triple->subject, triple->predicate, triple->object);
   }
   else
   {
      if(triple->subject != NULL)
      {
         if((triple->subject[0] == '_') && (triple->subject[1] == ':'))
         {
            printf("%s\n", triple->subject);
         }
         else
         {
            printf("<%s>\n", triple->subject);
         }
      }
      else
      {
         printf("INCOMPLETE\n");
      }

      if(triple->predicate != NULL)
      {
         printf("   <%s>\n", triple->predicate);
      }
      else
      {
         printf("   INCOMPLETE\n");
      }

      if(triple->object != NULL)
      {
         if(triple->object_type == RDF_TYPE_IRI)
         {
            if((triple->object[0] == '_') && (triple->object[1] == ':'))
            {
               printf("      %s", triple->object);
            }
            else
            {
               printf("      <%s>", triple->object);
            }
         }
         else if(triple->object_type == RDF_TYPE_PLAIN_LITERAL)
         {
            printf("      \"%s\"", triple->object);
            if(triple->language != NULL)
            {
               printf("@%s", triple->language);
            }
         }
         else if(triple->object_type == RDF_TYPE_XML_LITERAL)
         {
            printf("      \"%s\"^^rdf:XMLLiteral", triple->object);
         }
         else if(triple->object_type == RDF_TYPE_TYPED_LITERAL)
         {
            if((triple->datatype != NULL) && (triple->language != NULL))
            {
               printf("      \"%s\"@%s^^<%s>",
                  triple->object, triple->language, triple->datatype);
            }
            else if(triple->datatype != NULL)
            {
               printf("      \"%s\"^^<%s>", triple->object, triple->datatype);
            }
         }
         else
         {
            printf("      <%s> <---- UNKNOWN OBJECT TYPE", triple->object);
         }

         printf(" .\n");
      }
      else
      {
         printf("      INCOMPLETE .");
      }
   }
}

void rdfa_free_triple(rdftriple* triple)
{
   free(triple->subject);
   free(triple->predicate);
   free(triple->object);
   free(triple->datatype);
   free(triple->language);
   free(triple);
}

#ifndef LIBRDFA_IN_RAPTOR
/**
 * Generates a namespace prefix triple for any application that is
 * interested in processing namespace changes.
 *
 * @param context the RDFa context.
 * @param prefix the name of the prefix
 * @param IRI the fully qualified IRI that the prefix maps to.
 */
void rdfa_generate_namespace_triple(
   rdfacontext* context, const char* prefix, const char* iri)
{
   if(context->processor_graph_triple_callback != NULL)
   {
      rdftriple* triple = rdfa_create_triple(
         "@prefix", prefix, iri, RDF_TYPE_NAMESPACE_PREFIX, NULL, NULL);
      context->processor_graph_triple_callback(triple, context->callback_data);
   }
}

/**
 * Generates a set of triples that describe the location of a warning or
 * error in a document.
 *
 * @param context the currently active context.
 * @param subject the name of the subject that is associated with the triples.
 */
#if 1 /* remove when the prototype is in the header */
void rdfa_processor_location_triples(rdfacontext* context, const char* subject);
#endif
void rdfa_processor_location_triples(rdfacontext* context, const char* subject)
{
}

/**
 * Generates a set of triples in the processor graph including the processor's
 * position in the byte stream.
 *
 * @param context the current active context.
 * @param type the type of the message, which may be any of the RDF Classes
 *             defined in the RDFa Core specification:
 *             http://www.w3.org/TR/rdfa-core/#processor-graph-reporting
 * @param msg the message associated with the processor warning.
 */
void rdfa_processor_triples(
   rdfacontext* context, const char* type, const char* msg)
{
   if(context->processor_graph_triple_callback != NULL)
   {
      char buffer[32];
      char* subject = rdfa_create_bnode(context);
      char* context_subject = rdfa_create_bnode(context);

      /* generate the RDFa Processing Graph warning type triple */
      rdftriple* triple = rdfa_create_triple(
         subject, "http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
         type, RDF_TYPE_IRI, NULL, NULL);
      context->processor_graph_triple_callback(triple, context->callback_data);

      /* generate the description */
      triple = rdfa_create_triple(
         subject, "http://purl.org/dc/terms/description", msg,
         RDF_TYPE_PLAIN_LITERAL, NULL, "en");
      context->processor_graph_triple_callback(triple, context->callback_data);

      /* generate the context triple for the error */
      triple = rdfa_create_triple(
         subject, "http://www.w3.org/ns/rdfa#context",
         context_subject, RDF_TYPE_IRI, NULL, NULL);
      context->processor_graph_triple_callback(triple, context->callback_data);

      /* generate the type for the context triple */
      triple = rdfa_create_triple(
         context_subject, "http://www.w3.org/1999/02/22-rdf-syntax-ns#type",
         "http://www.w3.org/2009/pointers#LineCharPointer",
         RDF_TYPE_IRI, NULL, NULL);
      context->processor_graph_triple_callback(triple, context->callback_data);

      /* generate the line number */
      snprintf(buffer, sizeof(buffer) - 1, "%d",
         (int)xmlSAX2GetLineNumber(context->parser));
      triple = rdfa_create_triple(
         context_subject, "http://www.w3.org/2009/pointers#lineNumber",
         buffer, RDF_TYPE_TYPED_LITERAL,
         "http://www.w3.org/2001/XMLSchema#positiveInteger", NULL);
      context->processor_graph_triple_callback(triple, context->callback_data);

      free(context_subject);
      free(subject);
   }
}
#endif

/**
 * Completes all incomplete triples that are part of the current
 * context by matching the new_subject with the list of incomplete
 * triple predicates.
 *
 * @param context the RDFa context.
 */
void rdfa_complete_incomplete_triples(rdfacontext* context)
{
   /* 10. If the [ skip element ] flag is 'false', and [ new subject ]
    *     was set to a non-null value, then any [ incomplete triple ]s
    *     within the current context should be completed:
    *
    * The [ list of incomplete triples ] from the current [ evaluation
    * context ] ( not the [ local list of incomplete triples ]) will
    * contain zero or more predicate URIs. This list is iterated, and
    * each of the predicates is used with [ parent subject ] and
    * [ new subject ] to generate a triple. Note that at each level
    * there are two , lists of [ incomplete triple ]s; one for the
    * current processing level (which is passed to each child element
    * in the previous step), and one that was received as part of the
    * [ evaluation context ]. It is the latter that is used in
    * processing during this step. */
   unsigned int i;
   for(i = 0; i < context->incomplete_triples->num_items; i++)
   {
      rdfalist* incomplete_triples = context->incomplete_triples;
      rdfalistitem* incomplete_triple = incomplete_triples->items[i];

      if(incomplete_triple->flags & RDFALIST_FLAG_DIR_NONE)
      {
         /* If direction is 'none', the new subject is added to the list
          * from the iterated incomplete triple. */
         const char* predicate = (const char*)incomplete_triple->data;
         rdftriple* triple = rdfa_create_triple(context->parent_subject,
            predicate, context->new_subject, RDF_TYPE_IRI, NULL, NULL);

         /* ensure the list mapping exists */
         rdfa_create_list_mapping(
            context, context->local_list_mappings,
            context->parent_subject, predicate);

         /* add the predicate to the list mapping */
         rdfa_append_to_list_mapping(context->local_list_mappings,
            context->parent_subject, predicate, (void*)triple);
      }
      else if(incomplete_triple->flags & RDFALIST_FLAG_DIR_FORWARD)
      {
         /* If [direction] is 'forward' then the following triple is generated:
          *
          * subject
          *    [parent subject]
          * predicate
          *    the predicate from the iterated incomplete triple
          * object
          *    [new subject] */
         rdftriple* triple =
            rdfa_create_triple(context->parent_subject,
               (const char*)incomplete_triple->data, context->new_subject,
               RDF_TYPE_IRI, NULL, NULL);
         context->default_graph_triple_callback(triple, context->callback_data);
      }
      else
      {
         /* If [direction] is not 'forward' then this is the triple generated:
          *
          * subject
          *    [new subject]
          * predicate
          *    the predicate from the iterated incomplete triple
          * object
          *    [parent subject] */
         rdftriple* triple =
            rdfa_create_triple(context->new_subject,
               (const char*)incomplete_triple->data, context->parent_subject,
               RDF_TYPE_IRI, NULL, NULL);
         context->default_graph_triple_callback(triple, context->callback_data);
      }
      free(incomplete_triple->data);
      free(incomplete_triple);
   }
   context->incomplete_triples->num_items = 0;
}

void rdfa_complete_type_triples(
   rdfacontext* context, const rdfalist* type_of)
{
   unsigned int i;
   rdfalistitem** iptr = type_of->items;
   const char* subject;
   const char* type;

   if(context->rdfa_version == RDFA_VERSION_1_0)
   {
      /* RDFa 1.0: 6.1 One or more 'types' for the [new subject] can be set by
       * using @type_of. If present, the attribute must contain one or
       * more URIs, obtained according to the section on URI and CURIE
       * Processing, each of which is used to generate a triple as follows:
       *
       * subject
       *    [new subject]
       * predicate
       *    http://www.w3.org/1999/02/22-rdf-syntax-ns#type
       * object
       *    full URI of 'type'
       */
      subject = context->new_subject;
   }
   else
   {
      /* RDFa 1.1: 7. One or more 'types' for the typed resource can be set by
       * using @typeof. If present, the attribute may contain one or more IRIs,
       * obtained according to the section on CURIE and IRI Processing, each of
       * which is used to generate a triple as follows:
       *
       * subject
       *    typed resource
       * predicate
       *    http://www.w3.org/1999/02/22-rdf-syntax-ns#type
       * object
       *    current full IRI of 'type' from typed resource
       */
      subject = context->typed_resource;
   }

   for(i = 0; i < type_of->num_items; i++)
   {
      rdfalistitem* iri = *iptr;
      rdftriple* triple;
      type = (const char*)iri->data;

      triple = rdfa_create_triple(subject,
         "http://www.w3.org/1999/02/22-rdf-syntax-ns#type", type, RDF_TYPE_IRI,
         NULL, NULL);

      context->default_graph_triple_callback(triple, context->callback_data);
      iptr++;
   }
}

void rdfa_complete_relrev_triples(
   rdfacontext* context, const rdfalist* rel, const rdfalist* rev)
{
   /* 7. If in any of the previous steps a [current object  resource]
    * was set to a non-null value, it is now used to generate triples */
   unsigned int i;

   /* Predicates for the [current object resource] can be set by using
    * one or both of the @rel and @rev attributes. */

   /* If present, @rel will contain one or more URIs, obtained
    * according to the section on CURIE and URI Processing each of
    * which is used to generate a triple as follows:
    *
    * subject
    *    [new subject]
    * predicate
    *    full URI
    * object
    *    [current object resource] */
   if(rel != NULL)
   {
      rdfalistitem** relptr = rel->items;
      for(i = 0; i < rel->num_items; i++)
      {
         rdfalistitem* curie = *relptr;

         rdftriple* triple = rdfa_create_triple(context->new_subject,
            (const char*)curie->data, context->current_object_resource,
            RDF_TYPE_IRI, NULL, NULL);

         context->default_graph_triple_callback(triple, context->callback_data);
         relptr++;
      }
   }

   /* If present, @rev will contain one or more URIs, obtained
    * according to the section on CURIE and URI Processing each of which
    * is used to generate a triple as follows:
    *
    * subject
    *    [current object resource]
    * predicate
    *    full URI
    * object
    *    [new subject] */
   if(rev != NULL)
   {
      rdfalistitem** revptr = rev->items;
      for(i = 0; i < rev->num_items; i++)
      {
         rdfalistitem* curie = *revptr;

         rdftriple* triple = rdfa_create_triple(
            context->current_object_resource, (const char*)curie->data,
            context->new_subject, RDF_TYPE_IRI, NULL, NULL);

         context->default_graph_triple_callback(triple, context->callback_data);
         revptr++;
      }
   }
}

void rdfa_save_incomplete_triples(
   rdfacontext* context, const rdfalist* rel, const rdfalist* rev)
{
   unsigned int i;
   /* 8. If however [current object resource] was set to null, but
    * there are predicates present, then they must be stored as
    * [incomplete triple]s, pending the discovery of a subject that
    * can be used as the object. Also, [current object resource]
    * should be set to a newly created [bnode] */
   if(context->current_object_resource == NULL)
   {
      context->current_object_resource = rdfa_create_bnode(context);
   }

   /* If present, @rel must contain one or more URIs, obtained
    * according to the section on CURIE and URI Processing each of
    * which is added to the [local local list of incomplete triples]
    * as follows:
    *
    * predicate
    *    full URI
    * direction
    *    forward */
   if(rel != NULL)
   {
      rdfalistitem** relptr = rel->items;
      for(i = 0; i < rel->num_items; i++)
      {
         rdfalistitem* curie = *relptr;

         rdfa_add_item(
            context->local_incomplete_triples, curie->data,
               (liflag_t)(RDFALIST_FLAG_DIR_FORWARD | RDFALIST_FLAG_TEXT));

         relptr++;
      }
   }

   /* If present, @rev must contain one or more URIs, obtained
    * according to the section on CURIE and URI Processing, each of
    * which is added to the [local list of incomplete triples] as follows:
    *
    * predicate
    *    full URI
    * direction
    *    reverse */
   if(rev != NULL)
   {
      rdfalistitem** revptr = rev->items;
      for(i = 0; i < rev->num_items; i++)
      {
         rdfalistitem* curie = *revptr;

         rdfa_add_item(
            context->local_incomplete_triples, curie->data,
               (liflag_t)(RDFALIST_FLAG_DIR_REVERSE | RDFALIST_FLAG_TEXT));

         revptr++;
      }
   }
}

void rdfa_complete_object_literal_triples(rdfacontext* context)
{
   /* 9. The next step of the iteration is to establish any
    * [current object literal];
    *
    * Predicates for the [current object literal] can be set by using
    * @property. If present, a URI is obtained according to the
    * section on CURIE and URI Processing, and then the actual literal
    * value is obtained as follows: */
   const char* current_object_literal = NULL;
   rdfresource_t type = RDF_TYPE_UNKNOWN;

   unsigned int i;
   rdfalistitem** pptr;

   /* * as a [plain literal] if:
    *   o @content is present;
    *   o or all children of the [current element] are text nodes;
    *   o or there are no child nodes; TODO: Is this needed?
    *   o or the body of the [current element] does have non-text
    *     child nodes but @datatype is present, with an empty value.
    *
    * Additionally, if there is a value for [current language] then
    * the value of the [plain literal] should include this language
    * information, as described in [RDF-CONCEPTS]. The actual literal
    * is either the value of @content (if present) or a string created
    * by concatenating the text content of each of the descendant
    * elements of the [current element] in document order. */
   if((context->content != NULL))
   {
      current_object_literal = context->content;
      type = RDF_TYPE_PLAIN_LITERAL;
   }
   else if(context->xml_literal && strchr(context->xml_literal, '<') == NULL)
   {
      current_object_literal = context->plain_literal;
      type = RDF_TYPE_PLAIN_LITERAL;
   }
   else if(strlen(context->plain_literal) == 0)
   {
      current_object_literal = (const char*)"";
      type = RDF_TYPE_PLAIN_LITERAL;
   }
   else if((context->xml_literal != NULL) &&
           (context->datatype != NULL) &&
           (strlen(context->xml_literal) > 0) &&
           (strcmp(context->datatype, "") == 0))
   {
      current_object_literal = context->plain_literal;
      type = RDF_TYPE_PLAIN_LITERAL;
   }


   /* * as an [XML literal] if:
    *    o the [current element] has any child nodes that are not
    *      simply text nodes, and @datatype is not present, or is
    *      present, but is set to rdf:XMLLiteral.
    *
    * The value of the [XML literal] is a string created by
    * serializing to text, all nodes that are descendants of the
    * [current element], i.e., not including the element itself, and
    * giving it a datatype of rdf:XMLLiteral. */
   if((context->xml_literal != NULL) &&
      (current_object_literal == NULL) &&
      (strchr(context->xml_literal, '<') != NULL) &&
      ((context->datatype == NULL) ||
       (strcmp(context->datatype,
               "http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral") == 0)))
   {
      current_object_literal = context->xml_literal;
      type = RDF_TYPE_XML_LITERAL;
   }

   /* * as a [typed literal] if:
    *    o @datatype is present, and does not have an empty
    *      value.
    *
    * The actual literal is either the value of @content (if present)
    * or a string created by concatenating the value of all descendant
    * text nodes, of the [current element] in turn. The final string
    * includes the datatype URI, as described in [RDF-CONCEPTS], which
    * will have been obtained according to the section on CURIE and
    * URI Processing. */
   if((context->datatype != NULL) && (strlen(context->datatype) > 0))
   {
      if(context->content != NULL)
      {
         /* Static code analyzer clang says next line is not needed;
          * "Assigned value is always the same as the existing value"
          */
         /* current_object_literal = context->content; */
         type = RDF_TYPE_TYPED_LITERAL;
      }
      else if(strcmp(context->datatype,
               "http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral") != 0)
      {
         current_object_literal = context->plain_literal;
         type = RDF_TYPE_TYPED_LITERAL;
      }
   }

   /* TODO: Setting the current object literal to the plain literal in
    *       the case of xsd:string isn't mentioned in the syntax
    *       processing document. */
   if((current_object_literal == NULL) && (context->datatype != NULL) &&
      (strcmp(
         context->datatype, "http://www.w3.org/2001/XMLSchema#string") == 0))
   {
      current_object_literal = context->plain_literal;
      type = RDF_TYPE_TYPED_LITERAL;
   }

   /* The [current object literal] is then used with each predicate to
    * generate a triple as follows:
    *
    * subject
    *    [new subject]
    * predicate
    *    full URI
    * object
    *    [current object literal] */
   pptr = context->property->items;
   for(i = 0; i < context->property->num_items; i++)
   {

      rdfalistitem* curie = *pptr;
      rdftriple* triple = NULL;

      triple = rdfa_create_triple(context->new_subject,
         (const char*)curie->data, current_object_literal, type,
         context->datatype, context->language);

      context->default_graph_triple_callback(triple, context->callback_data);
      pptr++;
   }

   /* TODO: Implement recurse flag being set to false
    *
    * Once the triple has been created, if the [datatype] of the
    * [current object literal] is rdf:XMLLiteral, then the [recurse]
    * flag is set to false */
   context->recurse = 0;
}

void rdfa_complete_current_property_value_triples(rdfacontext* context)
{
   /* 11. The next step of the iteration is to establish any current property
    *     value;
    * Predicates for the current property value can be set by using @property.
    * If present, one or more resources are obtained according to the section
    * on CURIE and IRI Processing, and then the actual literal value is
    * obtained as follows: */
   char* current_property_value = NULL;
   rdfresource_t type = RDF_TYPE_UNKNOWN;

   unsigned int i;
   rdfalistitem** pptr;

   /* as a typed literal if @datatype is present, does not have an empty
    * value according to the section on CURIE and IRI Processing, and is not
    * set to XMLLiteral in the vocabulary
    * http://www.w3.org/1999/02/22-rdf-syntax-ns#. */
   if((context->datatype != NULL) && (strcmp(context->datatype,
      "http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral") != 0))
   {
      /* The actual literal is either the value of @content (if present) or a
       * string created by concatenating the value of all descendant text nodes,
       * of the current element in turn. */
      if(context->content != NULL)
      {
         current_property_value = context->content;
      }
      else
      {
         current_property_value = context->plain_literal;
      }

      /* The final string includes the datatype
       * IRI, as described in [RDF-CONCEPTS], which will have been obtained
       * according to the section on CURIE and IRI Processing.
       * otherwise, as a plain literal if @datatype is present but has an
       * empty value according to the section on CURIE and IRI Processing. */
      if(strlen(context->datatype) > 0)
      {
         type = RDF_TYPE_TYPED_LITERAL;
      }
      else
      {
         type = RDF_TYPE_PLAIN_LITERAL;
      }
   }
   else if((context->datatype != NULL) && (strcmp(context->datatype,
      "http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral") == 0))
   {
      /* otherwise, as an XML literal if @datatype is present and is set to
       * XMLLiteral in the vocabulary
       * http://www.w3.org/1999/02/22-rdf-syntax-ns#.
       * The value of the XML literal is a string created by serializing to
       * text, all nodes that are descendants of the current element, i.e., not
       * including the element itself, and giving it a datatype of XMLLiteral
       * in the vocabulary http://www.w3.org/1999/02/22-rdf-syntax-ns#. The
       * format of the resulting serialized content is as defined in Exclusive
       * XML Canonicalization Version [XML-EXC-C14N].
       * In order to maintain maximum portability of this literal, any children
       * of the current node that are elements must have the current XML
       * namespace declarations (if any) declared on the serialized element.
       * Since the child element node could also declare new XML namespaces,
       * the RDFa Processor must be careful to merge these together when
       * generating the serialized element definition. For avoidance of doubt,
       * any re-declarations on the child node must take precedence over
       * declarations that were active on the current node. */
      current_property_value = context->xml_literal;
      type = RDF_TYPE_XML_LITERAL;
   }
   else if(context->content != NULL)
   {
      /* otherwise, as an plain literal using the value of @content if
       * @content is present. */
      current_property_value = context->content;
      type = RDF_TYPE_PLAIN_LITERAL;
   }
   else if((context->rel_present == 0) && (context->rev_present == 0) &&
      (context->content == NULL))
   {
      /* otherwise, if the @rel, @rev, and @content attributes are not present,
       * as a resource obtained from one of the following: */
      if(context->resource != NULL)
      {
         /* by using the resource from @resource, if present, obtained
          * according to the section on CURIE and IRI Processing; */
         current_property_value = context->resource;
         type = RDF_TYPE_IRI;
      }
      else if(context->href != NULL)
      {
         /* otherwise, by using the IRI from @href, if present, obtained
          * according to the section on CURIE and IRI Processing; */
         current_property_value = context->href;
         type = RDF_TYPE_IRI;
      }
      else if(context->src != NULL)
      {
         /* otherwise, by using the IRI from @src, if present, obtained
          * according to the section on CURIE and IRI Processing. */
         current_property_value = context->src;
         type = RDF_TYPE_IRI;
      }
      else if((context->about == NULL) && (context->typed_resource != NULL))
      {
         /* otherwise, if @typeof is present and @about is not, the value of
          * typed resource. */
         current_property_value = context->typed_resource;
         type = RDF_TYPE_IRI;
      }
      else
      {
         /* otherwise as a plain literal. */
         current_property_value = context->plain_literal;
         type = RDF_TYPE_PLAIN_LITERAL;
      }
   }
   else
   {
      /* otherwise as a plain literal. */
      current_property_value = context->plain_literal;
      type = RDF_TYPE_PLAIN_LITERAL;
   }

   /* Additionally, if there is a value for current language then the value
    * of the plain literal should include this language information, as
    * described in [RDF-CONCEPTS]. The actual literal is either the value
    * of @content (if present) or a string created by concatenating the text
    * content of each of the descendant elements of the current element in
    * document order.
    *
    * NOTE: This happens automatically due to the way the code is setup. */

   if(context->inlist_present)
   {
      /* The current property value is then used with each predicate as
       * follows:
       * If the element also includes the @inlist attribute, the current
       * property value is added to the local list mapping as follows:
       * if the local list mapping does not contain a list associated with
       * the predicate IRI, instantiate a new list and add to local list
       * mappings add the current property value to the list associated
       * with the predicate IRI in the local list mapping */
      rdfa_establish_new_inlist_triples(
         context, context->property, current_property_value, type);
   }
   else
   {
      pptr = context->property->items;
      for(i = 0; i < context->property->num_items; i++)
      {
         /* Otherwise the current property value is used to generate a triple
          * as follows:
          * subject
          *   new subject
          * predicate
          *   full IRI
          * object
          *   current property value */
         rdfalistitem* curie = *pptr;
         rdftriple* triple = rdfa_create_triple(context->new_subject,
            (const char*)curie->data, current_property_value, type,
            context->datatype, context->language);

         context->default_graph_triple_callback(triple, context->callback_data);

         pptr++;
      }
   }
}
