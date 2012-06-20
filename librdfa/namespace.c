/**
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
 * This file implements mapping data structure memory management as
 * well as updating URI mappings.
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
 * Attempts to update the uri mappings in the given context using the
 * given attribute/value pair.
 *
 * @param attribute the attribute, which must start with xmlns.
 * @param value the value of the attribute
 */
void rdfa_update_uri_mappings(
   rdfacontext* context, const char* attr, const char* value)
{
#ifdef LIBRDFA_IN_RAPTOR
  raptor_namespace_stack* nstack;
  nstack = &context->sax2->namespaces;
#endif

   /* * the [current element] is parsed for [URI mappings] and these
    * are added to the [list of URI mappings]. Note that a [URI
    * mapping] will simply overwrite any current mapping in the list
    * that has the same name; */

   /* Mappings are provided by @xmlns. The value to be mapped is set
    * by the XML namespace prefix, and the value to map is the value
    * of the attribute -- a URI. Note that the URI is not processed
    * in any way; in particular if it is a relative path it is not
    * resolved against the [current base]. Authors are advised to
    * follow best practice for using namespaces, which includes not
    * using relative paths. */

   if(attr == NULL)
   {
#ifdef LIBRDFA_IN_RAPTOR
      raptor_namespaces_start_namespace_full(nstack,
                                             NULL,
                                             (const unsigned char*)value,
                                             0);
#else
      rdfa_update_mapping(
         context->uri_mappings, XMLNS_DEFAULT_MAPPING, value,
         (update_mapping_value_fp)rdfa_replace_string);
#endif
   }
   else if(strcmp(attr, "_") == 0)
   {
#define FORMAT_1 "The underscore character must not be declared as a prefix " \
         "because it conflicts with the prefix for blank node identifiers. " \
         "The occurrence of this prefix declaration is being ignored."
#ifdef LIBRDFA_IN_RAPTOR
      raptor_parser_warning((raptor_parser*)context->callback_data, 
                            FORMAT_1);
#else
      rdfa_processor_triples(context,
         RDFA_PROCESSOR_WARNING,
         FORMAT_1);
#endif
   }
   else if(attr[0] == ':' || attr[0] == '_' ||
      (attr[0] >= 'A' && attr[0] <= 'Z') ||
      (attr[0] >= 'a' && attr[0] <= 'z') ||
      ((unsigned char)attr[0] >= 0xc0 && (unsigned char)attr[0] <= 0xd6) ||
      ((unsigned char)attr[0] >= 0xd8 && (unsigned char)attr[0] <= 0xf6) || (unsigned char)attr[0] >= 0xf8)
   {
#ifdef LIBRDFA_IN_RAPTOR
     raptor_namespaces_start_namespace_full(nstack,
                                            (const unsigned char*)attr,
                                            (const unsigned char*)value,
                                            0);
#else
      rdfa_generate_namespace_triple(context, attr, value);
      rdfa_update_mapping(context->uri_mappings, attr, value,
         (update_mapping_value_fp)rdfa_replace_string);
#endif
   }
   else
   {
      /* allowable characters for CURIEs:
       * ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] |
       * [#x370-#x37D] | [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] |
       * [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD]
       * | [#x10000-#xEFFFF]
       */

      /* Generate the processor warning if this is an invalid prefix */
#define FORMAT_2 "The declaration of the '%s' prefix is invalid " \
         "because it starts with an invalid character. Please see " \
         "http://www.w3.org/TR/REC-xml/#NT-NameStartChar for a " \
         "full explanation of valid first characters for declaring " \
         "prefixes."
#ifdef LIBRDFA_IN_RAPTOR
      raptor_parser_warning((raptor_parser*)context->callback_data, 
                            FORMAT_2, attr);
#else
      char msg[1024];
      snprintf(msg, 1024, FORMAT_1);
      rdfa_processor_triples(context, RDFA_PROCESSOR_WARNING, msg);
#endif
   }

#ifdef LIBRDFA_IN_RAPTOR
#else
   /* print the current mapping */
   if(DEBUG)
   {
      printf("DEBUG: PREFIX MAPPINGS:");
      rdfa_print_mapping(context->uri_mappings,
         (print_mapping_value_fp)rdfa_print_string);
   }
#endif
}
