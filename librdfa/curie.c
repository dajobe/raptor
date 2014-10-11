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
 * The CURIE module is used to resolve all forms of CURIEs that
 * XHTML+RDFa accepts.
 *
 * @author Manu Sporny
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include "rdfa_utils.h"
#include "rdfa.h"
#include "strtok_r.h"

/* The base XHTML vocab URL is used to resolve URIs that are reserved
 * words. Any reserved listed above is appended to the URL below to
 * form a complete IRI. */
#define XHTML_VOCAB_URI "http://www.w3.org/1999/xhtml/vocab#"
#define XHTML_VOCAB_URI_SIZE 35

/**
 * Gets the type of CURIE that is passed to it.
 *
 * @param uri the uri to check.
 *
 * @return either CURIE_TYPE_SAFE, CURIE_TYPE_URI or CURIE_TYPE_INVALID.
 */
static curie_t rdfa_get_curie_type(const char* uri)
{
   curie_t rval = CURIE_TYPE_INVALID;
   
   if(uri != NULL)
   {
      size_t uri_length = strlen(uri);

      if((uri[0] == '[') && (uri[uri_length - 1] == ']'))
      {
         /* a safe curie starts with [ and ends with ] */
         rval = CURIE_TYPE_SAFE;
      }
      else if(strstr(uri, ":") != NULL)
      {
         /* at this point, it is unknown whether or not the CURIE is
          * an IRI or an unsafe CURIE */
         rval = CURIE_TYPE_IRI_OR_UNSAFE;
      }
      else
      {
         /* if none of the above match, then the CURIE is probably a
          * relative IRI */
         rval = CURIE_TYPE_IRI_OR_UNSAFE;
      }
   }

   return rval;
}

char* rdfa_resolve_uri(rdfacontext* context, const char* uri)
{
   char* rval = NULL;
   char* path_start = NULL;
   size_t base_length = strlen(context->base);
   
   if(strlen(uri) < 1)
   {
      /* if a blank URI is given, use the base context */
      rval = rdfa_replace_string(rval, context->base);
   }
   else if(strstr(uri, ":") != NULL)
   {
      /* if a IRI is given, don't concatenate */
      rval = rdfa_replace_string(rval, uri);
   }
   else if(uri[0] == '#' || uri[0] == '?')
   {
      /* if a fragment ID or start of a query parameter is given,
       * concatenate it with the base URI */
      rval = rdfa_join_string(context->base, uri);
   }
   else if(uri[0] == '/')
   {
      /* if a relative URI is given, but it starts with a '/', use the
       * host part concatenated to the given URI */
      char* tmp = NULL;
      char* end_index = NULL;

      /* initialize the working-set data */
      tmp = rdfa_replace_string(tmp, context->base);
      end_index = strchr(tmp, '/');


      /* find the final '/' character after the host part of the context base. */
      if(end_index != NULL)
      {
	 end_index = strchr(end_index + 1, '/');
	
	 if(end_index != NULL)
	 {
	    end_index = strchr(end_index + 1, '/');
         }
      }

      /* if the '/' character after the host part was found, copy the host
       * part and append the given URI to the URI, otherwise, append the
       * host part and the URI part as-is, ensuring that a '/' exists at the
       * end of the host part. */
      if(end_index != NULL)
      {
         char* rval_copy;

	 *end_index = '\0';
	 
	 /* if the '/' character after the host part was found, copy the host
	  * part and append the given URI to the URI. */
	 rval_copy = rdfa_replace_string(rval, tmp);
	 rval = rdfa_join_string(rval_copy, uri);
         free(rval_copy);
      }
      else
      {
	 /* append the host part and the URI part as-is, ensuring that a
	  * '/' exists at the end of the host part. */
 	 size_t tlen = strlen(tmp) - 1;
         char* rval_copy;

	 rval_copy = rdfa_replace_string(rval, tmp);

	 if(rval_copy[tlen] == '/')
	 {
	    rval_copy[tlen] = '\0';
	 }
	 rval = rdfa_join_string(rval_copy, uri);
         free(rval_copy);
      }

      free(tmp);
   }
   else
   {
      if((char)context->base[base_length - 1] == '/')
      {
         /* if the base URI already ends in /, concatenate */
         rval = rdfa_join_string(context->base, uri);
      }
      else
      {
         /* if we have a relative URI, chop off the name of the file
          * and replace it with the relative pathname */
         char* end_index = strrchr(context->base, '/');

         if(end_index != NULL)
         {
            char* tmpstr = NULL;
            char* end_index2;

            tmpstr = rdfa_replace_string(tmpstr, context->base);
            end_index2 = strrchr(tmpstr, '/');
            if(end_index2 != NULL) {
              end_index2++;
              *end_index2 = '\0';
            }

            rval = rdfa_join_string(tmpstr, uri);
            free(tmpstr);
         }
      }
   }

   /* It is possible that rval may be NULL here in OOM scenarios */
   if(!rval)
     return NULL;

   /* Find the start of a scheme-based URL path */
   path_start = (char*)strstr(rval, "://");
   if(path_start != NULL)
   {
      if(strstr(path_start, "/.") != NULL)
      {
         path_start += 3;
         path_start = strstr(path_start, "/");
      }
      else
      {
         path_start = NULL;
      }
   }

   /* remove any dot-segments that remain in the URL for URLs w/ schemes */
   if(path_start != NULL)
   {
      size_t rlen = strlen(rval) + 1;
      size_t hlen = path_start - rval;
      char* src = (char*)malloc(rlen + 4);
      char* sptr = src + hlen;
      char* dest = (char*)malloc(rlen + 1);
      char* dptr = dest + hlen;
      char* dfence = dptr;

      memset(src, 0, rlen + 4);
      strcpy(src, rval);
      strncpy(dest, rval, hlen);

      /* Process the path portion of the IRI */
      while(sptr[0] != '?' && sptr[0] != '\0')
      {
         if(sptr[0] == '.' && sptr[1] == '.' && sptr[2] == '/')
         {
            /* A.  If the input buffer begins with a prefix of "../",
             * then remove that prefix from the input buffer; otherwise,
             */
            sptr += 3;
         }
         else if(sptr[0] == '.' && sptr[1] == '/')
         {
            /* A.  If the input buffer begins with a prefix of "./",
             * then remove that prefix from the input buffer; otherwise,
             */
            sptr += 2;
         }
         else if(sptr[0] == '/' && sptr[1] == '.' && sptr[2] == '/')
         {
            /* B.  if the input buffer begins with a prefix of "/./",
             * then replace that prefix with "/" in the input buffer;
             * otherwise,
             */
            sptr += 2;
         }
         else if(sptr[0] == '/' && sptr[1] == '.' && sptr[2] == '\0')
         {
            /* B.  if the input buffer begins with a prefix of "/.",
             * where "." is a complete path segment, then replace that
             * prefix with "/" in the input buffer; otherwise,
             */
            sptr += 1;
            *sptr = '/';
         }
         else if(sptr[0] == '/' && sptr[1] == '.' && sptr[2] == '.' &&
            ((sptr[3] == '/') || (sptr[3] == '\0')))
         {
            /* C.  if the input buffer begins with a prefix of "/../",
             * then replace that prefix with "/" in the input buffer and
             * remove the last segment and its preceding "/" (if any) from
             * the output buffer; otherwise,
             */
            if(sptr[3] == '/')
            {
               sptr += 3;
            }
            else if(sptr[3] == '\0')
            {
               sptr += 2;
               *sptr = '/';
            }

            /* remove the last segment and the preceding '/' */
            if(dptr > dfence)
            {
               dptr--;
               if(dptr[0] == '/')
               {
                  dptr--;
               }
            }
            while(dptr >= dfence && dptr[0] != '/')
            {
               dptr--;
            }
            if(dptr >= dfence)
            {
               dptr[0] = '\0';
            }
            else
            {
               dptr = dfence;
               dptr[0] = '\0';
            }
         }
         else if(sptr[0] == '.' && sptr[1] == '\0')
         {
            /* D. if the input buffer consists only of ".", then remove
             * that from the input buffer; otherwise,
             */
            sptr++;

         }
         else if(sptr[0] == '.' && sptr[1] == '.' && sptr[1] == '\0')
         {
            /* D. if the input buffer consists only of "..", then remove
             * that from the input buffer; otherwise,
             */
            sptr += 2;
         }
         else
         {
            /* Copy the path segment */
            do
            {
               *dptr++ = *sptr++;
               *dptr = '\0';
            } while(sptr[0] != '/' && sptr[0] != '?' && sptr[0] != '\0');
         }
      }

      /* Copy the remaining query parameters */
      if(sptr[0] == '?')
      {
         strcpy(dptr, sptr);
      }
      else
      {
         dptr[0] = '\0';
      }

      free(rval);
      free(src);
      rval = dest;
   }

   return rval;
}

char* rdfa_resolve_curie(
   rdfacontext* context, const char* uri, curieparse_t mode)
{
   char* rval = NULL;
   curie_t ctype = rdfa_get_curie_type(uri);

   if(!uri)
      return NULL;

   if(ctype == CURIE_TYPE_INVALID)
   {
      rval = NULL;
   }
   else if((ctype == CURIE_TYPE_IRI_OR_UNSAFE) &&
           ((mode == CURIE_PARSE_HREF_SRC) ||
            (context->rdfa_version == RDFA_VERSION_1_0 &&
               mode == CURIE_PARSE_ABOUT_RESOURCE)))
   {
      /* If we are parsing something that can take either a CURIE or a
       * URI, and the type is either IRI or UNSAFE, assume that it is
       * an IRI */
      rval = rdfa_resolve_uri(context, uri);
   }

   /*
    * Check to see if the value is a term.
    */
   if(ctype == CURIE_TYPE_IRI_OR_UNSAFE && mode == CURIE_PARSE_PROPERTY)
   {
      const char* term_iri;
      term_iri = (const char*)rdfa_get_mapping(context->term_mappings, uri);
      if(term_iri != NULL)
      {
         rval = strdup(term_iri);
      }
      else if(context->default_vocabulary == NULL && strstr(uri, ":") == NULL)
      {
         /* Generate the processor warning if this is a missing term */
#define FORMAT_1 "The use of the '%s' term was unrecognized by the RDFa processor because it is not a valid term for the current Host Language."

#ifdef LIBRDFA_IN_RAPTOR
         raptor_parser_warning((raptor_parser*)context->callback_data, 
                               FORMAT_1, uri);
#else
         char msg[1024];
         snprintf(msg, 1024, FORMAT_1, uri);

         rdfa_processor_triples(context, RDFA_PROCESSOR_WARNING, msg);
#endif
      }
   }

   /* if we are processing a safe CURIE OR
    * if we are parsing an unsafe CURIE that is an @type_of,
    * @datatype, @property, @rel, or @rev attribute, treat the curie
    * as not an IRI, but an unsafe CURIE */
   if(rval == NULL && ((ctype == CURIE_TYPE_SAFE) ||
         ((ctype == CURIE_TYPE_IRI_OR_UNSAFE) &&
          ((mode == CURIE_PARSE_INSTANCEOF_DATATYPE) ||
           (mode == CURIE_PARSE_PROPERTY) ||
           (mode == CURIE_PARSE_RELREV) ||
           (context->rdfa_version == RDFA_VERSION_1_1 &&
              mode == CURIE_PARSE_ABOUT_RESOURCE)))))
   {
      char* working_copy = NULL;
      char* wcptr = NULL;
      char* prefix = NULL;
      char* curie_reference = NULL;
      const char* expanded_prefix = NULL;

      working_copy = (char*)malloc(strlen(uri) + 1);
      strcpy(working_copy, uri);/*rdfa_replace_string(working_copy, uri);*/

      /* if this is a safe CURIE, chop off the beginning and the end */
      if(ctype == CURIE_TYPE_SAFE)
      {
         prefix = strtok_r(working_copy, "[:]", &wcptr);
         if(wcptr)
            curie_reference = strtok_r(NULL, "[]", &wcptr);
      }
      else if(ctype == CURIE_TYPE_IRI_OR_UNSAFE)
      {
         prefix = strtok_r(working_copy, ":", &wcptr);
         if(wcptr)
            curie_reference = strtok_r(NULL, "", &wcptr);
      }

      /* fully resolve the prefix and get its length */

      /* if a colon was found, but no prefix, use the XHTML vocabulary URI
       * as the expanded prefix */
      if((uri[0] == ':') || (strcmp(uri, "[:]") == 0))
      {
         expanded_prefix = XHTML_VOCAB_URI;
         curie_reference = prefix;
         prefix = NULL;
      }
      else if(uri[0] == ':')
      {
         /* FIXME: This looks like a bug - don't know why this code is
          * in here. I think it's for the case where ":next" is
          * specified, but the code's not checking that -- manu */
         expanded_prefix = context->base;
         curie_reference = prefix;
         prefix = NULL;
      }
      else if(prefix != NULL)
      {
         if((mode != CURIE_PARSE_PROPERTY) &&
            (mode != CURIE_PARSE_RELREV) &&
            strcmp(prefix, "_") == 0)
         {
            /* if the prefix specifies this as a blank node, then we
             * use the blank node prefix */
            expanded_prefix = "_";
         }
         else
         {
            /* if the prefix was defined, get it from the set of URI mappings. */
#ifdef LIBRDFA_IN_RAPTOR
            if(!strcmp(prefix, "xml"))
            {
               expanded_prefix = RAPTOR_GOOD_CAST(const char*, raptor_xml_namespace_uri);
            }
            else
            {
               raptor_namespace *nspace;
               raptor_uri* ns_uri;
               nspace = raptor_namespaces_find_namespace(&context->sax2->namespaces,
                                                         (const unsigned char*)prefix,
                                                         (int)strlen(prefix));
               if(nspace) {
                  ns_uri = raptor_namespace_get_uri(nspace);
                  if(ns_uri)
                     expanded_prefix = (const char*)raptor_uri_as_string(ns_uri);
               }
            }
#else
            expanded_prefix =
               rdfa_get_mapping(context->uri_mappings, prefix);

            /* Generate the processor warning if the prefix was not found */
            if(expanded_prefix == NULL && strstr(uri, ":") != NULL &&
               strstr(uri, "://") == NULL)
            {
#define FORMAT_2 "The '%s' prefix was not found. You may want to check that it is declared before it is used, or that it is a valid prefix string."
#ifdef LIBRDFA_IN_RAPTOR
              raptor_parser_warning((raptor_parser*)context->callback_data, 
                                    FORMAT_2, prefix);
#else
              char msg[1024];
              snprintf(msg, 1024, FORMAT_2, prefix);

               rdfa_processor_triples(context, RDFA_PROCESSOR_WARNING, msg);
#endif
            }
#endif
         }
      }

      if((expanded_prefix != NULL) && (curie_reference != NULL))
      {
         /* if the expanded prefix and the reference exist, generate the
          * full IRI. */
         if(strcmp(expanded_prefix, "_") == 0)
         {
            rval = rdfa_join_string("_:", curie_reference);
         }
         else
         {
            rval = rdfa_join_string(expanded_prefix, curie_reference);
         }
      }
      else if((expanded_prefix != NULL) && (expanded_prefix[0] != '_') && 
         (curie_reference == NULL))
      {
         /* if the expanded prefix exists, but the reference is null,
          * generate the CURIE because a reference-less CURIE is still
          * valid */
 	 rval = rdfa_join_string(expanded_prefix, "");
      }

      free(working_copy);
   }

   if(rval == NULL)
   {
      /* if we're NULL at this point, the CURIE might be the special
       * unnamed bnode specified by _: */
      if((strcmp(uri, "[_:]") == 0) || (strcmp(uri, "_:") == 0))
      {
         if(context->underscore_colon_bnode_name == NULL)
         {
            context->underscore_colon_bnode_name = rdfa_create_bnode(context);
         }
         rval = rdfa_replace_string(rval, context->underscore_colon_bnode_name);
      }
      /* if we're NULL at this point and the IRI isn't [], then this might be
       * an IRI */
      else if(context->rdfa_version == RDFA_VERSION_1_1 &&
         (strcmp(uri, "[]") != 0))
      {
         if((context->default_vocabulary != NULL) &&
            ((mode == CURIE_PARSE_PROPERTY) || (mode == CURIE_PARSE_RELREV) ||
               (mode == CURIE_PARSE_INSTANCEOF_DATATYPE)) &&
            (strstr(uri, ":") == NULL))
         {
            rval = rdfa_join_string(context->default_vocabulary, uri);
         }
         else if(((mode == CURIE_PARSE_PROPERTY) ||
            (mode == CURIE_PARSE_ABOUT_RESOURCE) ||
            (mode == CURIE_PARSE_INSTANCEOF_DATATYPE)) &&
            (strstr(uri, "_:") == NULL) && (strstr(uri, "[_:") == NULL))
         {
            rval = rdfa_resolve_uri(context, uri);
         }
      }
   }
   
   /* even though a reference-only CURIE is valid, it does not
    * generate a triple in XHTML+RDFa. If we're NULL at this point,
    * the given value wasn't valid in XHTML+RDFa. */
   
   return rval;
}

/**
 * Resolves a given uri depending on whether or not it is a fully
 * qualified IRI, a CURIE, or a short-form XHTML reserved word for
 * @rel or @rev as defined in the XHTML+RDFa Syntax Document.
 *
 * @param context the current processing context.
 * @param uri the URI part to process.
 *
 * @return the fully qualified IRI, or NULL if the conversion failed
 *         due to the given URI not being a short-form XHTML reserved
 *         word. The memory returned from this function MUST be freed.
 */
char* rdfa_resolve_relrev_curie(rdfacontext* context, const char* uri)
{
   char* rval = NULL;
   const char* resource = uri;

   /* check to make sure the URI doesn't have an empty prefix */
   if(uri[0] == ':')
   {
      resource++;
   }

   /* override reserved words if there is a default vocab defined
    * NOTE: Don't have to check for RDFa 1.1 mode because vocab is only defined
    * in RDFa 1.1 */
   if(context->default_vocabulary != NULL)
   {
      rval = rdfa_resolve_curie(context, uri, CURIE_PARSE_RELREV);
   }
   else if(context->host_language == HOST_LANGUAGE_XHTML1)
   {
      /* search all of the XHTML @rel/@rev reserved words for a
       * case-insensitive match against the given URI */
      char* term = strdup(resource);
      char* ptr = NULL;

      for(ptr = term; *ptr; ptr++)
      {
         *ptr = tolower(*ptr);
      }

      rval = (char*)rdfa_get_mapping(context->term_mappings, term);
      if(rval != NULL)
      {
         rval = strdup(rval);
      }
      free(term);
   }
   else
   {
      /* Search the term mappings for a match */
      rval = (char*)rdfa_get_mapping(context->term_mappings, resource);
      if(rval != NULL)
      {
         rval = strdup(rval);
      }
   }

   /* if a search against the registered terms failed,
    * attempt to resolve the value as a standard CURIE */
   if(rval == NULL)
   {
      rval = rdfa_resolve_curie(context, uri, CURIE_PARSE_RELREV);
   }

   /* if a CURIE wasn't found, attempt to resolve the value as an IRI */
   if(rval == NULL && (context->rdfa_version == RDFA_VERSION_1_1))
   {
      rval = rdfa_resolve_uri(context, uri);
   }
   
   return rval;
}

rdfalist* rdfa_resolve_curie_list(
   rdfacontext* rdfa_context, const char* uris, curieparse_t mode)
{
   rdfalist* rval = rdfa_create_list(3);
   char* working_uris = NULL;
   char* uptr = NULL;
   char* ctoken = NULL;
   working_uris = rdfa_replace_string(working_uris, uris);

   /* go through each item in the list of CURIEs and resolve each */
   ctoken = strtok_r(working_uris, RDFA_WHITESPACE, &uptr);
   
   while(ctoken != NULL)
   {
      char* resolved_curie = NULL;

      if((mode == CURIE_PARSE_INSTANCEOF_DATATYPE) ||
         (mode == CURIE_PARSE_ABOUT_RESOURCE) ||
         (mode == CURIE_PARSE_PROPERTY))
      {
         resolved_curie =
            rdfa_resolve_curie(rdfa_context, ctoken, mode);
      }
      else if(mode == CURIE_PARSE_RELREV)
      {
         resolved_curie =
            rdfa_resolve_relrev_curie(rdfa_context, ctoken);
      }

      /* add the CURIE if it was a valid one */
      if(resolved_curie != NULL)
      {
         rdfa_add_item(rval, resolved_curie, RDFALIST_FLAG_TEXT);
         free(resolved_curie);
      }
      
      ctoken = strtok_r(NULL, RDFA_WHITESPACE, &uptr);
   }
   
   free(working_uris);

   return rval;
}
