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
 * This file is used to process RDFa subjects.
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rdfa_utils.h"
#include "rdfa.h"

/**
 * Creates a new bnode given an RDFa context.
 *
 * @param context the RDFa context.
 *
 * @return a newly allocated string containing the bnode name. This
 *         string MUST be memory collected.
 */
char* rdfa_create_bnode(rdfacontext* context)
{
   char* rval = NULL;
   char buffer[64];
   
   /* print and increment the bnode count */
   sprintf(buffer, "_:bnode%i", (int)context->bnode_count++);
   rval = rdfa_replace_string(rval, buffer);

   return rval;
}

/**
 * Establishes a new subject for the given context given the
 * attributes on the current element. The given context's new_subject
 * value is updated if a new subject is found.
 *
 * @param context the RDFa context.
 * @param name the name of the current element that is being processed.
 * @param about the full IRI for about, or NULL if there isn't one.
 * @param src the full IRI for src, or NULL if there isn't one.
 * @param resource the full IRI for resource, or NULL if there isn't one.
 * @param href the full IRI for href, or NULL if there isn't one.
 * @param type_of The list of IRIs for type_of, or NULL if there was
 *                no type_of specified.
 */
void rdfa_establish_new_1_0_subject(
   rdfacontext* context, const char* name, const char* about, const char* src,
   const char* resource, const char* href, const rdfalist* type_of)
{
   /* 4. If the [current element] contains no valid @rel or @rev
    * URI, obtained according to the section on CURIE and URI
    * Processing, then the next step is to establish a value for
    * [new subject]. Any of the attributes that can carry a
    * resource can set [new subject]; */

   if(about != NULL)
   {
      /* * by using the URI from @about, if present, obtained according
       *   to the section on CURIE and URI Processing; */
      context->new_subject =
         rdfa_replace_string(context->new_subject, about);
   }
   else if(src != NULL)
   {   
      /* * otherwise, by using the URI from @src, if present, obtained
       *   according to the section on CURIE and URI Processing. */
      context->new_subject =
         rdfa_replace_string(context->new_subject, src);
   }
   else if(resource != NULL)
   {   
      /* * otherwise, by using the URI from @resource, if present,
       *   obtained according to the section on CURIE and URI
       *   Processing; */
      context->new_subject =
         rdfa_replace_string(context->new_subject, resource);
   }
   else if(href != NULL)
   {
      /* * otherwise, by using the URI from @href, if present, obtained
       *   according to the section on CURIE and URI Processing. */
      context->new_subject =
         rdfa_replace_string(context->new_subject, href);
   }
   else if((type_of != NULL) && (type_of->num_items > 0))
   {
      /* * if @type_of is present, obtained according to the
       * section on CURIE and URI Processing, then [new subject] is
       * set to be a newly created [bnode]; */
      char* bnode = rdfa_create_bnode(context);
      context->new_subject = rdfa_replace_string(context->new_subject, bnode);
      free(bnode);
   }
   else if(context->parent_object != NULL)
   {
      /* * otherwise, if [parent object] is present, [new subject] is
       * set to that and the [skip element] flag is set to 'true'; */
      context->new_subject =
         rdfa_replace_string(context->new_subject, context->parent_object);

      /* TODO: The skip element flag will be set even if there is a
       * @property value, which is a bug, isn't it? */
      /*context->skip_element = 1;*/
   }
}

/**
 * Establishes a new subject for the given context given the
 * attributes on the current element. The given context's new_subject
 * value is updated if a new subject is found.
 *
 * @param context the RDFa context.
 * @param name the name of the current element that is being processed.
 * @param about the full IRI for about, or NULL if there isn't one.
 * @param src the full IRI for src, or NULL if there isn't one.
 * @param resource the full IRI for resource, or NULL if there isn't one.
 * @param href the full IRI for href, or NULL if there isn't one.
 * @param type_of The list of IRIs for type_of, or NULL if there was
 *                no type_of specified.
 * @param property a list of properties that were detected during processing.
 */
void rdfa_establish_new_1_1_subject(
   rdfacontext* context, const char* name, const char* about, const char* src,
   const char* resource, const char* href, const rdfalist* type_of,
   const rdfalist* property, const char* content, const char* datatype)
{
   /*
    * If the current element contains the @property attribute, but does not
    * contain either the @content or @datatype attributes, then new subject
    * is set to the resource obtained from the first match from the
    * following rule:
    */
   if(property != NULL && content == NULL && datatype == NULL)
   {
      /* by using the resource from @about, if present, obtained according to
       * the section on CURIE and IRI Processing;
       */
      if(about != NULL)
      {
         /* NOTE: this statement achieves this part of the processing rule
          * as well because @about is set if depth == 1 in RDFa 1.1 in
          * the calling function: otherwise, if the element is the root
          * element of the document, then act as if there is an empty
          * @about present, and process it according to the rule for
          * @about, above;
          */
         context->new_subject =
            rdfa_replace_string(context->new_subject, about);
      }
      else if(context->parent_object != NULL)
      {
         /* otherwise, if parent object is present, new subject is set
          * to the value of parent object.
          */
         context->new_subject =
            rdfa_replace_string(context->new_subject, context->parent_object);
      }

      /* If @typeof is present then typed resource is set to the resource
       * obtained from the first match from the following rules:
       */
      if(type_of != NULL)
      {
         if(about != NULL)
         {
            /* by using the resource from @about, if present, obtained
             * according to the section on CURIE and IRI Processing;
             *
             * NOTE: about is set to the document if this is the root
             * element of the document, so the following rule is also applied
             * in this case:
             *
             * otherwise, if the element is the root element of the
             * document, then act as if there is an empty @about present
             * and process it according to the previous rule;
             */
            context->typed_resource =
               rdfa_replace_string(context->typed_resource, about);
         }
         else
         {
            if(resource != NULL)
            {
               /* by using the resource from @resource, if present, obtained
                * according to the section on CURIE and IRI Processing;
                */
               context->typed_resource =
                  rdfa_replace_string(context->typed_resource, resource);
            }
            else if(href != NULL)
            {
               /* otherwise, by using the IRI from @href, if present, obtained
                * according to the section on CURIE and IRI Processing;
                */
               context->typed_resource =
                  rdfa_replace_string(context->typed_resource, href);
            }
            else if(src != NULL)
            {
               /* otherwise, by using the IRI from @src, if present, obtained
                * according to the section on CURIE and IRI Processing;
                */
               context->typed_resource =
                  rdfa_replace_string(context->typed_resource, src);
            }
            else
            {
               /* otherwise, the value of typed resource is set to a newly
                * created bnode.
                */
               char* bnode = rdfa_create_bnode(context);
               context->typed_resource = rdfa_replace_string(
                  context->typed_resource, bnode);
               free(bnode);
            }

            /* The value of the current object resource is then set to the value
             * of typed resource.
             */
            context->current_object_resource = rdfa_replace_string(
               context->current_object_resource, context->typed_resource);
         }
      }
   }
   else
   {
      /* otherwise:
       * If the element contains an @about, @href, @src, or @resource attribute,
       * new subject is set to the resource obtained as follows:
       */
      if(about != NULL || href != NULL || src != NULL || resource != NULL)
      {
         if(about != NULL)
         {
            /* by using the resource from @about, if present, obtained
             * according to the section on CURIE and IRI Processing;
             */
            context->new_subject =
               rdfa_replace_string(context->new_subject, about);
         }
         else if(resource != NULL)
         {
            /* otherwise, by using the resource from @resource, if present,
             * obtained according to the section on CURIE and IRI Processing;
             */
            context->new_subject =
               rdfa_replace_string(context->new_subject, resource);
         }
         else if(href != NULL)
         {
            /* otherwise, by using the IRI from @href, if present, obtained
             * according to the section on CURIE and IRI Processing;
             */
            context->new_subject =
               rdfa_replace_string(context->new_subject, href);
         }
         else if(src != NULL)
         {
            /* otherwise, by using the IRI from @src, if present, obtained
             * according to the section on CURIE and IRI Processing.
             */
            context->new_subject =
               rdfa_replace_string(context->new_subject, src);
         }
      }
      else
      {
         /* otherwise, if no resource is provided by a resource attribute,
          * then the first match from the following rules will apply:
          */

         /* NOTE: this step is achieved via the parent function call as @about
          * is set if the current element is the root element.
          *
          * if the element is the root element of the document, then act
          * as if there is an empty @about present, and process it according
          * to the rule for @about, above;
          */
         if(type_of != NULL)
         {
            /* otherwise, if @typeof is present, then new subject is set
             * to be a newly created bnode;
             */
            char* bnode = rdfa_create_bnode(context);
            context->new_subject = rdfa_replace_string(context->new_subject,
               bnode);
            free(bnode);
         }
         else if(context->parent_object != NULL)
         {
            /* otherwise, if parent object is present, new subject is set to
             * the value of parent object.
             */
            context->new_subject = rdfa_replace_string(context->new_subject,
               context->parent_object);

            /* Additionally, if @property is not present then the skip
             * element flag is set to 'true'.
             */
            if(property == NULL)
            {
               context->skip_element = 1;
            }
         }
      }

      if(type_of != NULL)
      {
         /* Finally, if @typeof is present, set the typed resource to the value
          * of new subject.
          */
         context->typed_resource = rdfa_replace_string(context->typed_resource,
            context->new_subject);
      }
   }
}

/**
 * Establishes a new subject for the given context when @rel or @rev
 * is present. The given context's new_subject and
 * current_object_resource values are updated if a new subject is found.
 *
 * @param context the RDFa context.
 * @param about the full IRI for about, or NULL if there isn't one.
 * @param src the full IRI for src, or NULL if there isn't one.
 * @param resource the full IRI for resource, or NULL if there isn't one.
 * @param href the full IRI for href, or NULL if there isn't one.
 * @param type_of the list of IRIs for type_of, or NULL if type_of
 *                wasn't specified on the current element.
 */
void rdfa_establish_new_1_0_subject_with_relrev(
   rdfacontext* context, const char* name, const char* about, const char* src,
   const char* resource, const char* href, const rdfalist* type_of)
{
   /* 5. If the [current element] does contain a valid @rel or @rev
    * URI, obtained according to the section on CURIE and URI
    * Processing, then the next step is to establish both a value
    * for [new subject] and a value for [current object resource]:
    *
    * [new subject] is set to the URI obtained from the first match
    * from the following rules: */
   
   if(about != NULL)
   {
      /* * by using the URI from @about, if present, obtained
       * according to the section on CURIE and URI Processing; */
      context->new_subject =
         rdfa_replace_string(context->new_subject, about);
   }
   else if(context->rdfa_version == RDFA_VERSION_1_0 &&  src != NULL)
   {
      /* * otherwise, by using the URI from @src, if present, obtained
       * according to the section on CURIE and URI Processing. */
      context->new_subject =
         rdfa_replace_string(context->new_subject, src);
   }
   else if((type_of != NULL) && (type_of->num_items > 0))
   {
      /* * if @type_of is present, obtained according to the
       * section on CURIE and URI Processing, then [new subject] is
       * set to be a newly created [bnode]; */
      char* bnode = rdfa_create_bnode(context);
      context->new_subject = rdfa_replace_string(context->new_subject, bnode);
      free(bnode);
   }
   else if(context->parent_object != NULL)
   {
      /* * otherwise, if [parent object] is present, [new subject] is
       * set to that; */
      context->new_subject =
         rdfa_replace_string(context->new_subject, context->parent_object);
   }

   /* Then the [current object resource] is set to the URI obtained
    * from the first match from the following rules: */
   if(resource != NULL)
   {
      /* * by using the URI from @resource, if present, obtained
       *   according to the section on CURIE and URI Processing; */
      context->current_object_resource =
         rdfa_replace_string(context->current_object_resource, resource);
   }
   else if(href != NULL)
   {
      /* * otherwise, by using the URI from @href, if present,
       *   obtained according to the section on CURIE and URI Processing. */
      context->current_object_resource =
         rdfa_replace_string(context->current_object_resource, href);
   }
   else
   {
      /* * otherwise, null. */
      context->current_object_resource = NULL;
   }

   /* Note that final value of the [current object resource] will
    * either be null, or a full URI. */
}

/**
 * Establishes a new subject for the given context when @rel or @rev
 * is present. The given context's new_subject and
 * current_object_resource values are updated if a new subject is found.
 *
 * @param context the RDFa context.
 * @param about the full IRI for about, or NULL if there isn't one.
 * @param src the full IRI for src, or NULL if there isn't one.
 * @param resource the full IRI for resource, or NULL if there isn't one.
 * @param href the full IRI for href, or NULL if there isn't one.
 * @param type_of the list of IRIs for type_of, or NULL if type_of
 *                wasn't specified on the current element.
 */
void rdfa_establish_new_1_1_subject_with_relrev(
   rdfacontext* context, const char* name, const char* about, const char* src,
   const char* resource, const char* href, const rdfalist* type_of)
{
   /* If the current element does contain a @rel or @rev attribute, then
    * the next step is to establish both a value for new subject and a
    * value for current object resource:
    */

   /* new subject is set to the resource obtained from the first match from
    * the following rules:
    */

   if(about != NULL)
   {
      /* by using the resource from @about, if present, obtained according
       * to the section on CURIE and IRI Processing;
       *
       * NOTE: This will also catch the following rule due to @about being
       * set in the calling function:
       *
       * if the element is the root element of the document then act as if
       * there is an empty @about present, and process it according to the
       * rule for @about, above;
       */
      context->new_subject =
         rdfa_replace_string(context->new_subject, about);
   }

   if(type_of != NULL)
   {
      /* if the @typeof attribute is present, set typed resource to
       * new subject.
       */
      context->typed_resource =
         rdfa_replace_string(context->typed_resource, context->new_subject);
   }

   /* If no resource is provided then the first match from the following rules
    * will apply:
    *
    */
   if(context->new_subject == NULL && context->parent_object != NULL)
   {
      /* otherwise, if parent object is present, new subject is set to that.
       */
      context->new_subject = rdfa_replace_string(
         context->new_subject, context->parent_object);
   }

   /* Then the current object resource is set to the resource obtained from
    * the first match from the following rules:
    */

   if(resource != NULL)
   {
      /* by using the resource from @resource, if present, obtained according
       * to the section on CURIE and IRI Processing;
       */
      context->current_object_resource = rdfa_replace_string(
         context->current_object_resource, resource);
   }
   else if(href != NULL)
   {
      /* otherwise, by using the IRI from @href, if present, obtained
       * according to the section on CURIE and IRI Processing;
       */
      context->current_object_resource = rdfa_replace_string(
         context->current_object_resource, href);
   }
   else if(src != NULL)
   {
      /* otherwise, by using the IRI from @src, if present, obtained
       * according to the section on CURIE and IRI Processing;
       */
      context->current_object_resource = rdfa_replace_string(
         context->current_object_resource, src);
   }
   else if(type_of != NULL && about == NULL)
   {
      /* otherwise, if @typeof is present and @about is not, use a
       * newly created bnode.
       */
      char* bnode = rdfa_create_bnode(context);
      context->current_object_resource = rdfa_replace_string(
         context->current_object_resource, bnode);
      free(bnode);
   }

   if(type_of != NULL && about == NULL)
   {
      /* If @typeof is present and @about is not, set typed resource to current
       * object resource.
       */
      context->typed_resource = rdfa_replace_string(
         context->typed_resource, context->current_object_resource);
   }

   /* Note that final value of the current object resource will either be
    * null (from initialization) or a full IRI or bnode.
    */
}
