/**
 * Copyright 2012 Digital Bazaar, Inc.
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

void rdfa_establish_new_inlist_triples(rdfacontext* context,
   rdfalist* predicates, const char* object, rdfresource_t object_type)
{
   int i = 0;
   for(i = 0; i < (int)predicates->num_items; i++)
   {
      const char* predicate = (const char*)predicates->items[i]->data;
      char* resolved_predicate = rdfa_resolve_relrev_curie(context, predicate);
      rdftriple* triple;
      /* ensure the list mapping exists */
      rdfa_create_list_mapping(
         context, context->local_list_mappings,
         context->new_subject, resolved_predicate);

      /* add an incomplete triple for each list mapping */
      triple = rdfa_create_triple(context->new_subject, resolved_predicate,
         object, object_type, context->datatype, context->language);
      rdfa_append_to_list_mapping(context->local_list_mappings,
         context->new_subject, resolved_predicate, triple);

      free(resolved_predicate);
   }

#if defined(DEBUG) && DEBUG > 0
   printf("LOCAL LIST MAPPINGS: ");
   rdfa_print_mapping(context->local_list_mappings,
         (print_mapping_value_fp)rdfa_print_triple_list);
#endif
}

void rdfa_save_incomplete_list_triples(
   rdfacontext* context, const rdfalist* rel)
{
   unsigned int i;
   for(i = 0; i < rel->num_items; i++)
   {
      const char* curie = (const char*)rel->items[i]->data;
      char* resolved_curie = rdfa_resolve_relrev_curie(context, curie);

      /* ensure the list mapping exists */
      rdfa_create_list_mapping(
         context, context->local_list_mappings,
         context->new_subject, resolved_curie);

      /* get the list name */
      rdfa_add_item(
         context->local_incomplete_triples, resolved_curie,
         (liflag_t)(RDFALIST_FLAG_DIR_NONE | RDFALIST_FLAG_TEXT));

      free(resolved_curie);
   }

#if defined(DEBUG) && DEBUG > 0
   printf("LOCAL INCOMPLETE TRIPLES: ");
   rdfa_print_list(context->local_incomplete_triples);
#endif
}

void rdfa_complete_list_triples(rdfacontext* context)
{
   /* For each IRI in the local list mapping, if the equivalent list does
    * not exist in the evaluation context, indicating that the list was
    * originally instantiated on the current element, use the list as follows: */
   int i;
   rdfalist* list;
   rdftriple* triple;
   void** mptr = context->local_list_mappings;
   char* key = NULL;
   void** kptr = NULL;
   void* value = NULL;
   unsigned int list_depth = 0;

#if defined(DEBUG) && DEBUG > 0
   printf("local_list_mappings: ");
   rdfa_print_mapping(context->local_list_mappings,
         (print_mapping_value_fp)rdfa_print_triple_list);
#endif

   while(*mptr != NULL)
   {
      kptr = mptr;
      rdfa_next_mapping(mptr++, &key, &value);
      list = (rdfalist*)value;
      list_depth = list->user_data;
      mptr++;
#if defined(DEBUG) && DEBUG > 0
      printf("LIST TRIPLES for key (%u/%u): KEY(%s)\n",
             context->depth, list_depth, key);
#endif

      if((context->depth < (int)list_depth) &&
         (rdfa_get_list_mapping(
            context->list_mappings, context->new_subject, key) == NULL) &&
         (strcmp(key, RDFA_MAPPING_DELETED_KEY) != 0))
      {
         char* predicate = strstr(key, " ") + 1;
         triple = (rdftriple*)list->items[0]->data;
         if(list->num_items == 1)
         {
            /* Free unused list triple */
            rdfa_free_triple(triple);

            /* the list is empty, generate an empty list triple */
            triple = rdfa_create_triple(context->new_subject, predicate,
               "http://www.w3.org/1999/02/22-rdf-syntax-ns#nil",
               RDF_TYPE_IRI, NULL, NULL);
            context->default_graph_triple_callback(
               triple, context->callback_data);
         }
         else
         {
            char* bnode = NULL;
            char* subject;
            char* tmp = NULL;
            bnode = rdfa_replace_string(bnode, triple->subject);
            for(i = 1; i < (int)list->num_items; i++)
            {
               char* next = NULL;
               triple = (rdftriple*)list->items[i]->data;
               /* Create a new 'bnode' array containing newly created bnodes,
                * one for each item in the list
                * For each bnode-(IRI or literal) pair from the list the
                * following triple is generated:
                *
                * subject
                *   bnode
                * predicate
                *   http://www.w3.org/1999/02/22-rdf-syntax-ns#first
                * object
                *   full IRI or literal */
               triple->subject =
                  rdfa_replace_string(triple->subject, bnode);
               triple->predicate =
                  rdfa_replace_string(triple->predicate,
                     "http://www.w3.org/1999/02/22-rdf-syntax-ns#first");
               context->default_graph_triple_callback(
                  triple, context->callback_data);

               /* Free the list item */
               free(list->items[i]);
               list->items[i] = NULL;

               /* For each item in the 'bnode' array the following triple is
                * generated:
                *
                * subject
                *   bnode
                * predicate
                *   http://www.w3.org/1999/02/22-rdf-syntax-ns#rest
                * object
                *   next item in the 'bnode' array or, if that does not exist,
                *   http://www.w3.org/1999/02/22-rdf-syntax-ns#nil */
               if(i < (int)list->num_items - 1)
               {
                  next = rdfa_create_bnode(context);
               }
               else
               {
                  next = strdup((char*)"http://www.w3.org/1999/02/22-rdf-syntax-ns#nil");
               }

               triple = rdfa_create_triple(bnode,
                  "http://www.w3.org/1999/02/22-rdf-syntax-ns#rest",
                  next, RDF_TYPE_IRI, NULL, NULL);
               context->default_graph_triple_callback(
                  triple, context->callback_data);

               /* Free the bnode, setting 'next' appropriately */
               free(bnode);
               bnode = next;
            }

            /* A single additional triple is generated:
             * subject
             *   current subject
             * predicate
             *   full IRI of the local list mapping associated with this list
             * object
             *   first item of the 'bnode' array */
            subject = strdup(key);
            if(subject)
              tmp = strstr(subject, " ");

            if(tmp) {
              tmp[0] = '\0';
              triple = (rdftriple*)list->items[0]->data;
              triple->subject =
                rdfa_replace_string(triple->subject, subject);
              triple->predicate =
                rdfa_replace_string(triple->predicate, predicate);
              context->default_graph_triple_callback(
                triple, context->callback_data);
            }
            if(subject)
              free(subject);
            if(bnode)
              free(bnode);
         }

         /* Free the first list item and empty the list */
         free(list->items[0]);
         list->items[0] = NULL;
         list->num_items = 0;

         /* clear the entry from the mapping */
         *kptr = rdfa_replace_string((char*)*kptr, RDFA_MAPPING_DELETED_KEY);
      }
   }
}
