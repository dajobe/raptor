/*
 * Copyright 2008-2011 Digital Bazaar, Inc.
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with librdfa. If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rdfa_utils.h"
#include "rdfa.h"
#include "strtok_r.h"

#define RDFA_WHITESPACE_CHARACTERS " \a\b\t\n\v\f\r"

char* rdfa_join_string(const char* prefix, const char* suffix)
{
   char* rval = NULL;
   size_t prefix_size = strlen(prefix);
   size_t suffix_size = strlen(suffix);
   rval = (char*)malloc(prefix_size + suffix_size + 1);

   memcpy(rval, prefix, prefix_size);
   memcpy(rval+prefix_size, suffix, suffix_size + 1);

   return rval;
}

char* rdfa_n_append_string(
   char* old_string, size_t* string_size,
   const char* suffix, size_t suffix_size)
{
   char* rval = NULL;
   rval = (char*)realloc(old_string, *string_size + suffix_size + 1);
   memcpy(rval + *string_size, suffix, suffix_size + 1);
   *string_size = *string_size + suffix_size;
   return rval;
}

char* rdfa_replace_string(char* old_string, const char* new_string)
{
   char* rval = NULL;

   if(new_string != NULL)
   {
      /* free the memory associated with the old string */
      free(old_string);

      /* copy the new string */
      rval = strdup(new_string);
   }

   return rval;
}

char* rdfa_canonicalize_string(const char* str)
{
   char* rval = (char*)malloc(sizeof(char) * (strlen(str) + 2));
   char* working_string = NULL;
   char* token = NULL;
   char* wptr = NULL;
   char* offset = rval;

   working_string = rdfa_replace_string(working_string, str);

   /* split on any whitespace character that we may find */
   token = strtok_r(working_string, RDFA_WHITESPACE_CHARACTERS, &wptr);
   while(token != NULL)
   {
      size_t token_length = strlen(token);
      memcpy(offset, token, token_length);
      offset += token_length;
      *offset++ = ' ';
      *offset = '\0';

      token = strtok_r(NULL, RDFA_WHITESPACE_CHARACTERS, &wptr);
   }

   if(offset != rval)
   {
      offset--;
      *offset = '\0';
   }

   free(working_string);

   return rval;
}

rdfalist* rdfa_create_list(size_t size)
{
   rdfalist* rval = (rdfalist*)malloc(sizeof(rdfalist));

   rval->max_items = size;
   rval->num_items = 0;
   rval->items = (rdfalistitem**)malloc(
      sizeof(rdfalistitem*) * rval->max_items);

   return rval;
}

rdfalist* rdfa_replace_list(rdfalist* old_list, rdfalist* new_list)
{
   rdfalist* rval = NULL;

   if(new_list != NULL)
   {
      /* free the memory associated with the old list */
      rdfa_free_list(old_list);

      /* copy the new list */
      rval = rdfa_copy_list(new_list);
   }

   return rval;
}

rdfalist* rdfa_copy_list(rdfalist* list)
{
   rdfalist* rval = NULL;

   if(list != NULL)
   {
      unsigned int i;
      rval = rdfa_create_list(list->max_items);

      /* copy the base list variables over */
      rval->num_items = list->num_items;
      rval->user_data = list->user_data;

      /* copy the data of every list member along with all of the flags
       * for each list member. */
      for(i = 0; i < list->max_items; i++)
      {
         if(i < list->num_items)
         {
            rval->items[i] = (rdfalistitem*)malloc(sizeof(rdfalistitem));
            rval->items[i]->data = NULL;
            rval->items[i]->flags = list->items[i]->flags;

            /* copy specific data type */
            if(list->items[i]->flags & RDFALIST_FLAG_TEXT)
            {
               rval->items[i]->data = strdup((char*)list->items[i]->data);
            }
            else if(list->items[i]->flags & RDFALIST_FLAG_TRIPLE)
            {
               rdftriple* t = (rdftriple*)list->items[i]->data;
               rval->items[i]->data =
                  rdfa_create_triple(t->subject, t->predicate, t->object,
                     t->object_type, t->datatype, t->language);
            }
            else if(list->items[i]->flags & RDFALIST_FLAG_CONTEXT)
            {
               /* TODO: Implement the copy for context, if it is needed. */
            }
         }
         else
         {
            rval->items[i] = NULL;
         }
      }
   }

   return rval;
}

void rdfa_print_list(rdfalist* list)
{
   unsigned int i;

   printf("[ ");

   for(i = 0; i < list->num_items; i++)
   {
      if(i != 0)
      {
         printf(", ");
      }

      puts((const char*)list->items[i]->data);
   }

   printf(" ]\n");
}

void rdfa_print_triple_list(rdfalist* list)
{
   unsigned int i;

   if(list != NULL)
   {
      printf("[ ");

      for(i = 0; i < list->num_items; i++)
      {
         if(i != 0)
         {
            printf(", ");
         }

         rdfa_print_triple((rdftriple*)list->items[i]->data);
      }

      printf(" ]\n");
   }
   else
   {
      printf("NULL\n");
   }
}

void rdfa_free_list(rdfalist* list)
{
   if(list != NULL)
   {
      unsigned int i;
      for(i = 0; i < list->num_items; i++)
      {
         if(list->items[i]->flags & RDFALIST_FLAG_TEXT)
         {
            free(list->items[i]->data);
         }
         else if(list->items[i]->flags & RDFALIST_FLAG_TRIPLE)
         {
            rdftriple* t = (rdftriple*)list->items[i]->data;
            rdfa_free_triple(t);
         }

         free(list->items[i]);
      }

      free(list->items);
      free(list);
   }
}

void rdfa_push_item(rdfalist* stack, void* data, liflag_t flags)
{
   rdfa_add_item(stack, data, flags);
}

void* rdfa_pop_item(rdfalist* stack)
{
   void* rval = NULL;

   if(stack->num_items > 0)
   {
      --stack->num_items;
      rval = stack->items[stack->num_items]->data;
      free(stack->items[stack->num_items]);
      stack->items[stack->num_items] = NULL;
   }

   return rval;
}

void rdfa_add_item(rdfalist* list, void* data, liflag_t flags)
{
   rdfalistitem* item;

   if(!list)
      return;

   item = (rdfalistitem*)malloc(sizeof(rdfalistitem));

   item->data = NULL;

   if((flags & RDFALIST_FLAG_CONTEXT) || (flags & RDFALIST_FLAG_TRIPLE))
   {
      item->data = data;
   }
   else
   {
      item->data = (char*)rdfa_replace_string(
         (char*)item->data, (const char*)data);
   }

   item->flags = flags;

   if(list->num_items == list->max_items)
   {
      list->max_items = 1 + (list->max_items * 2);
      list->items = (rdfalistitem**)realloc(
         list->items, sizeof(rdfalistitem*) * list->max_items);
   }

   list->items[list->num_items] = item;
   ++list->num_items;
}

void** rdfa_create_mapping(size_t elements)
{
   size_t mapping_size = sizeof(void*) * MAX_URI_MAPPINGS * 2;
   void** mapping = (void**)malloc(mapping_size);

   /* only initialize the mapping if it is not null. */
   if(mapping != NULL)
   {
      memset(mapping, 0, mapping_size);
   }

   return mapping;
}

void rdfa_create_list_mapping(
   rdfacontext* context, void** mapping,
   const char* subject, const char* key)
{
   char* realkey = NULL;
   size_t str_size;
   rdfalist* value = NULL;
   char* list_bnode;
   rdftriple* triple;

   /* Attempt to find the list mapping */
   value = (rdfalist*)rdfa_get_list_mapping(mapping, subject, key);

   if(value == NULL)
   {
      /* create the mapping */
      value = rdfa_create_list(MAX_LIST_ITEMS);
      value->user_data = context->depth;

      /* build the real key to use when updating the mapping */
      str_size = strlen(subject);
      realkey = strdup(subject);
      realkey = rdfa_n_append_string(realkey, &str_size, " ", 1);
      realkey = rdfa_n_append_string(realkey, &str_size, key, strlen(key));
      rdfa_update_mapping(mapping, realkey, value,
         (update_mapping_value_fp)rdfa_replace_list);
      free(realkey);
      rdfa_free_list(value);

      /* add the first item in the list as the bnode for the list */
      list_bnode = rdfa_create_bnode(context);
      triple = rdfa_create_triple(
         list_bnode, list_bnode, list_bnode, RDF_TYPE_IRI, NULL, NULL);
      rdfa_append_to_list_mapping(mapping, subject, key, (void*)triple);
      free(list_bnode);
   }
}

void rdfa_append_to_list_mapping(
   void** mapping, const char* subject, const char* key, void* value)
{
   rdfalist* list = (rdfalist*)rdfa_get_list_mapping(mapping, subject, key);
   rdfa_add_item(list, value, RDFALIST_FLAG_TRIPLE);
}

void** rdfa_copy_mapping(
   void** mapping, copy_mapping_value_fp copy_mapping_value)
{
   void** rval = (void**)calloc(MAX_URI_MAPPINGS * 2, sizeof(void*));
   void** mptr = mapping;
   void** rptr = rval;

   /* copy each element of the old mapping to the new mapping. */
   while(*mptr != NULL)
   {
      /* copy the key */
      *rptr = rdfa_replace_string((char*)*rptr, (const char*)*mptr);
      rptr++;
      mptr++;

      /* copy the value */
      *rptr = copy_mapping_value(*rptr, *mptr);
      rptr++;
      mptr++;
   }

   return rval;
}

void rdfa_update_mapping(void** mapping, const char* key, const void* value,
   update_mapping_value_fp update_mapping_value)
{
   int found = 0;
   void** mptr = mapping;

   /* search the current mapping to see if the key exists in the mapping */
   while(!found && (*mptr != NULL))
   {
      if(strcmp((char*)*mptr, key) == 0)
      {
         mptr++;
         *mptr = update_mapping_value(*mptr, value);
         found = 1;
      }
      else
      {
         mptr++;
      }
      mptr++;
   }

   /* if we made it through the entire URI mapping and the key was not
    * found, create a new key-value pair. */
   if(!found)
   {
     *mptr = rdfa_replace_string((char*)*mptr, key);
      mptr++;
      *mptr = update_mapping_value(*mptr, value);
   }
}

const void* rdfa_get_mapping(void** mapping, const char* key)
{
   const void* rval = NULL;
   char** mptr = (char**)mapping;

   /* search the current mapping to see if the key exists in the mapping. */
   while(*mptr != NULL)
   {
      if(strcmp(*mptr, key) == 0)
      {
         mptr++;
         rval = *mptr;
      }
      else
      {
         mptr++;
      }
      mptr++;
   }

   return rval;
}

const void* rdfa_get_list_mapping(
   void** mapping, const char* subject, const char* key)
{
   void* rval;
   char* realkey = NULL;
   size_t str_size = strlen(subject);

   /* generate the real list mapping key and retrieve it from the mapping */
   realkey = strdup(subject);
   realkey = rdfa_n_append_string(realkey, &str_size, " ", 1);
   realkey = rdfa_n_append_string(realkey, &str_size, key, strlen(key));
   rval = (void*)rdfa_get_mapping(mapping, realkey);
   free(realkey);

   return (const void*)rval;
}

void rdfa_next_mapping(void** mapping, char** key, void** value)
{
   *key = NULL;
   *value = NULL;

   if(*mapping != NULL)
   {
      *key = *(char**)mapping++;
      *value = *mapping++;
   }
}

void rdfa_print_mapping(void** mapping, print_mapping_value_fp print_value)
{
   void** mptr = mapping;
   printf("{\n");
   while(*mptr != NULL)
   {
      char* key;
      void* value;
      key = (char*)*mptr++;
      value = *mptr++;

      printf("   %s : ", key);
      print_value(value);

      if(*mptr != NULL)
      {
         printf(",\n");
      }
      else
      {
         printf("\n");
      }
   }
   printf("}\n");
}

void rdfa_print_string(const char* str)
{
   printf("%s", str);
}

void rdfa_free_mapping(void** mapping, free_mapping_value_fp free_value)
{
   void** mptr = mapping;

   if(mapping != NULL)
   {
      /* free all of the memory in the mapping */
      while(*mptr != NULL)
      {
         free(*mptr);
         mptr++;
         free_value(*mptr);
         mptr++;
      }

      free(mapping);
   }
}

