/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_feature.c - Parser and Serializer features
 *
 * Copyright (C) 2004-2010, David Beckett http://www.dajobe.org/
 * Copyright (C) 2004-2005, University of Bristol, UK http://www.bristol.ac.uk/
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
 * 
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

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


static const struct
{
  raptor_feature feature;
  raptor_feature_area area;
  raptor_feature_value_type value_type;
  const char *name;
  const char *label;
} raptor_features_list[RAPTOR_FEATURE_LAST + 1] = {
  { RAPTOR_FEATURE_SCANNING                , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "scanForRDF", "Scan for rdf:RDF in XML content" },
  { RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "allowNonNsAttributes", "Allow bare 'name' rather than namespaced 'rdf:name'" },
  { RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES  , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "allowOtherParsetypes", "Allow user-defined rdf:parseType values" },
  { RAPTOR_FEATURE_ALLOW_BAGID             , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "allowBagID", "Allow rdf:bagID" },
  { RAPTOR_FEATURE_ALLOW_RDF_TYPE_RDF_LIST , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "allowRDFtypeRDFlist", "Generate the collection rdf:type rdf:List triple" },
  { RAPTOR_FEATURE_NORMALIZE_LANGUAGE      , RAPTOR_FEATURE_AREA_PARSER | RAPTOR_FEATURE_AREA_SAX2, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "normalizeLanguage", "Normalize xml:lang values to lowercase" },
  { RAPTOR_FEATURE_NON_NFC_FATAL           , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "nonNFCfatal", "Make non-NFC literals cause a fatal error" },
  { RAPTOR_FEATURE_WARN_OTHER_PARSETYPES   , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "warnOtherParseTypes", "Warn about unknown rdf:parseType values" },
  { RAPTOR_FEATURE_CHECK_RDF_ID            , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "checkRdfID", "Check rdf:ID values for duplicates" },
  { RAPTOR_FEATURE_RELATIVE_URIS           , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "relativeURIs", "Write relative URIs wherever possible in serializing." },
  { RAPTOR_FEATURE_START_URI               , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_URI,  "startURI", "Start URI for serializing to use." },
  { RAPTOR_FEATURE_WRITER_AUTO_INDENT      , RAPTOR_FEATURE_AREA_XML_WRITER | RAPTOR_FEATURE_AREA_TURTLE_WRITER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "autoIndent", "Automatically indent elements." },
  { RAPTOR_FEATURE_WRITER_AUTO_EMPTY       , RAPTOR_FEATURE_AREA_XML_WRITER | RAPTOR_FEATURE_AREA_TURTLE_WRITER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "autoEmpty", "Automatically detect and abbreviate empty elements." },
  { RAPTOR_FEATURE_WRITER_INDENT_WIDTH     , RAPTOR_FEATURE_AREA_XML_WRITER | RAPTOR_FEATURE_AREA_TURTLE_WRITER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "indentWidth", "Number of spaces to indent." },
  { RAPTOR_FEATURE_WRITER_XML_VERSION      , RAPTOR_FEATURE_AREA_SERIALIZER | RAPTOR_FEATURE_AREA_XML_WRITER, RAPTOR_FEATURE_VALUE_TYPE_INT, "xmlVersion", "XML version to write." },
  { RAPTOR_FEATURE_WRITER_XML_DECLARATION  , RAPTOR_FEATURE_AREA_SERIALIZER | RAPTOR_FEATURE_AREA_XML_WRITER, RAPTOR_FEATURE_VALUE_TYPE_BOOL, "xmlDeclaration", "Write XML declaration." },
  { RAPTOR_FEATURE_NO_NET                  , RAPTOR_FEATURE_AREA_PARSER | RAPTOR_FEATURE_AREA_SAX2, RAPTOR_FEATURE_VALUE_TYPE_BOOL,  "noNet", "Deny network requests." },
  { RAPTOR_FEATURE_RESOURCE_BORDER   , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_STRING, "resourceBorder", "DOT serializer resource border color" },
  { RAPTOR_FEATURE_LITERAL_BORDER    , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_STRING, "literalBorder", "DOT serializer literal border color" },
  { RAPTOR_FEATURE_BNODE_BORDER      , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_STRING, "bnodeBorder", "DOT serializer blank node border color" },
  { RAPTOR_FEATURE_RESOURCE_FILL     , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_STRING, "resourceFill", "DOT serializer resource fill color" },
  { RAPTOR_FEATURE_LITERAL_FILL      , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_STRING, "literalFill", "DOT serializer literal fill color" },
  { RAPTOR_FEATURE_BNODE_FILL        , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_STRING, "bnodeFill", "DOT serializer blank node fill color" },
  { RAPTOR_FEATURE_HTML_TAG_SOUP     , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_BOOL,  "htmlTagSoup", "HTML parsing uses a lax HTML parser" },
  { RAPTOR_FEATURE_MICROFORMATS      , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_BOOL,  "microformats", "GRDDL parsing looks for microformats" },
  { RAPTOR_FEATURE_HTML_LINK         , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_BOOL,  "htmlLink", "GRDDL parsing looks for <link type=\"application/rdf+xml\">" },
  { RAPTOR_FEATURE_WWW_TIMEOUT       , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_INT,  "wwwTimeout", "Set internal WWW URI retrieval timeout" },
  { RAPTOR_FEATURE_WRITE_BASE_URI    , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_BOOL,  "writeBaseURI", "Write @base / xml:base directive in serializer output" },
  { RAPTOR_FEATURE_WWW_HTTP_CACHE_CONTROL, RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_STRING, "wwwHttpCacheControl", "Set HTTP Cache-Control: header value" },
  { RAPTOR_FEATURE_WWW_HTTP_USER_AGENT , RAPTOR_FEATURE_AREA_PARSER, RAPTOR_FEATURE_VALUE_TYPE_STRING, "wwwHttpUserAgent", "Set HTTP User-Agent: header value" },
  { RAPTOR_FEATURE_JSON_CALLBACK     , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_STRING, "jsonCallback", "JSON serializer callback" },
  { RAPTOR_FEATURE_JSON_EXTRA_DATA   , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_STRING, "jsonExtraData", "JSON serializer extra data" },
  { RAPTOR_FEATURE_RSS_TRIPLES       , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_STRING, "rssTriples", "Atom/RSS serializer writes extra RDF triples" },
  { RAPTOR_FEATURE_ATOM_ENTRY_URI    , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_URI, "atomEntryUri", "Atom serializer Entry URI" },
  { RAPTOR_FEATURE_PREFIX_ELEMENTS   , RAPTOR_FEATURE_AREA_SERIALIZER, RAPTOR_FEATURE_VALUE_TYPE_BOOL,  "prefixElements", "Atom/RSS serializers write namespace-prefixed elements" }
};


static const char * const raptor_feature_uri_prefix="http://feature.librdf.org/raptor-";
/* NOTE: this is strlen(raptor_feature_uri_prefix) */
#define RAPTOR_FEATURE_URI_PREFIX_LEN 33


/*
 * raptor_features_enumerate_common:
 * @world: raptor_world object
 * @feature: feature enumeration (0+)
 * @name: pointer to store feature short name (or NULL)
 * @uri: pointer to store feature URI (or NULL)
 * @label: pointer to feature label (or NULL)
 * @area: area to match.
 * 
 * Internal: Get list of syntax features.
 *
 * If @uri is not NULL, a pointer toa new raptor_uri is returned
 * that must be freed by the caller with raptor_free_uri().
 *
 * Return value: 0 on success, <0 on failure, >0 if feature is unknown
 **/
int
raptor_features_enumerate_common(raptor_world* world,
                                 const raptor_feature feature,
                                 const char **name, 
                                 raptor_uri **uri, const char **label,
                                 raptor_feature_area area)
{
  int i;

  for(i = 0; i <= RAPTOR_FEATURE_LAST; i++)
    if(raptor_features_list[i].feature == feature &&
       (raptor_features_list[i].area & area)) {
      if(name)
        *name = raptor_features_list[i].name;
      
      if(uri) {
        raptor_uri *base_uri;
        base_uri = raptor_new_uri_from_counted_string(world,
                                                      (const unsigned char*)raptor_feature_uri_prefix,
                                                      RAPTOR_FEATURE_URI_PREFIX_LEN);
        if(!base_uri)
          return -1;
        
        *uri = raptor_new_uri_from_uri_local_name(world,
                                                  base_uri,
                                                  (const unsigned char*)raptor_features_list[i].name);
        raptor_free_uri(base_uri);
      }
      if(label)
        *label = raptor_features_list[i].label;
      return 0;
    }

  return 1;
}



/**
 * raptor_feature_get_value_type
 * @feature: raptor serializer or parser feature
 *
 * Get the type of a features.
 *
 * Most features are boolean or integer values and use
 * raptor_parser_set_feature() and raptor_parser_get_feature()
 * (raptor_serializer_set_feature() and raptor_serializer_get_feature() )
 *
 * String and URI value features use raptor_parser_set_feature_string() and
 * raptor_parser_get_feature_string()
 * ( raptor_serializer_set_feature_string()
 * and raptor_serializer_get_feature_string() )
 *
 * Return value: the type of the feature or < 0 if @feature is unknown
 */
raptor_feature_value_type
raptor_feature_get_value_type(const raptor_feature feature)
{
  if(feature > RAPTOR_FEATURE_LAST)
    return -1;
  return raptor_features_list[feature].value_type;
}


int
raptor_feature_value_is_numeric(const raptor_feature feature)
{
  raptor_feature_value_type t = raptor_features_list[feature].value_type;
  
  return t == RAPTOR_FEATURE_VALUE_TYPE_BOOL ||
         t == RAPTOR_FEATURE_VALUE_TYPE_INT;
}


/**
 * raptor_world_get_feature_from_uri:
 * @world: raptor_world instance
 * @uri: feature URI
 *
 * Get a feature ID from a URI
 * 
 * The allowed feature URIs are available via raptor_features_enumerate().
 *
 * Return value: < 0 if the feature is unknown
 **/
raptor_feature
raptor_world_get_feature_from_uri(raptor_world* world, raptor_uri *uri)
{
  unsigned char *uri_string;
  int i;
  raptor_feature feature= (raptor_feature)-1;
  
  if(!uri)
    return feature;
  
  uri_string = raptor_uri_as_string(uri);
  if(strncmp((const char*)uri_string, raptor_feature_uri_prefix,
             RAPTOR_FEATURE_URI_PREFIX_LEN))
    return feature;

  uri_string += RAPTOR_FEATURE_URI_PREFIX_LEN;

  for(i = 0; i <= RAPTOR_FEATURE_LAST; i++)
    if(!strcmp(raptor_features_list[i].name, (const char*)uri_string)) {
      feature = (raptor_feature)i;
      break;
    }

  return feature;
}


/**
 * raptor_parser_get_feature_count:
 *
 * Get the count of features defined.
 *
 * This is prefered to the compile time-only symbol #RAPTOR_FEATURE_LAST
 * and returns a count of the number of features which is
 * #RAPTOR_FEATURE_LAST+1.
 *
 * Return value: count of features in the #raptor_feature enumeration
 **/
unsigned int
raptor_parser_get_feature_count(void) {
  return RAPTOR_FEATURE_LAST+1;
}

