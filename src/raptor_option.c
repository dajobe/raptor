/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_option.c - Class options
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


static const struct
{
  raptor_option option;
  raptor_option_area area;
  raptor_option_value_type value_type;
  const char *name;
  const char *label;
} raptor_options_list[RAPTOR_OPTION_LAST + 1] = {
  { RAPTOR_OPTION_SCANNING,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "scanForRDF",
    "RDF/XML parser scans for rdf:RDF in XML content"
  },
  { RAPTOR_OPTION_ALLOW_NON_NS_ATTRIBUTES,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "allowNonNsAttributes",
    "RDF/XML parser allows bare 'name' rather than namespaced 'rdf:name'"
  },
  { RAPTOR_OPTION_ALLOW_OTHER_PARSETYPES,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "allowOtherParsetypes",
    "RDF/XML parser allows user-defined rdf:parseType values"
  },
  { RAPTOR_OPTION_ALLOW_BAGID,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "allowBagID",
    "RDF/XML parser allows rdf:bagID"
  },
  { RAPTOR_OPTION_ALLOW_RDF_TYPE_RDF_LIST,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "allowRDFtypeRDFlist",
    "RDF/XML parser generates the collection rdf:type rdf:List triple"
  },
  { RAPTOR_OPTION_NORMALIZE_LANGUAGE,
    (raptor_option_area)(RAPTOR_OPTION_AREA_PARSER | RAPTOR_OPTION_AREA_SAX2),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "normalizeLanguage",
    "RDF/XML parser normalizes xml:lang values to lowercase"
  },
  { RAPTOR_OPTION_NON_NFC_FATAL,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "nonNFCfatal",
    "RDF/XML parser makes non-NFC literals a fatal error"
  },
  { RAPTOR_OPTION_WARN_OTHER_PARSETYPES,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "warnOtherParseTypes",
    "RDF/XML parser warns about unknown rdf:parseType values"
  },
  { RAPTOR_OPTION_CHECK_RDF_ID,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "checkRdfID",
    "RDF/XML parser checks rdf:ID values for duplicates"
  },
  { RAPTOR_OPTION_RELATIVE_URIS,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "relativeURIs",
    "Serializers write relative URIs wherever possible."
  },
  { RAPTOR_OPTION_WRITER_AUTO_INDENT,
    (raptor_option_area)(RAPTOR_OPTION_AREA_XML_WRITER | RAPTOR_OPTION_AREA_TURTLE_WRITER),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "autoIndent",
    "Turtle and XML Writer automatically indent elements."
  },
  { RAPTOR_OPTION_WRITER_AUTO_EMPTY,
    (raptor_option_area)(RAPTOR_OPTION_AREA_XML_WRITER | RAPTOR_OPTION_AREA_TURTLE_WRITER),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "autoEmpty",
    "Turtle and XML Writer automatically detect and abbreviate empty elements."
  },
  { RAPTOR_OPTION_WRITER_INDENT_WIDTH,
    (raptor_option_area)(RAPTOR_OPTION_AREA_XML_WRITER | RAPTOR_OPTION_AREA_TURTLE_WRITER),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "indentWidth",
    "Turtle and XML Writer use as number of spaces to indent."
  },
  { RAPTOR_OPTION_WRITER_XML_VERSION,
    (raptor_option_area)(RAPTOR_OPTION_AREA_SERIALIZER | RAPTOR_OPTION_AREA_XML_WRITER),
    RAPTOR_OPTION_VALUE_TYPE_INT,
    "xmlVersion",
    "Serializers and XML Writer use as XML version to write."
  },
  { RAPTOR_OPTION_WRITER_XML_DECLARATION,
    (raptor_option_area)(RAPTOR_OPTION_AREA_SERIALIZER | RAPTOR_OPTION_AREA_XML_WRITER),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "xmlDeclaration",
    "Serializers and XML Writer write XML declaration."
  },
  { RAPTOR_OPTION_NO_NET,
    (raptor_option_area)(RAPTOR_OPTION_AREA_PARSER | RAPTOR_OPTION_AREA_SAX2),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "noNet",
    "Parsers and SAX2 XML Parser deny internal network requests."
  },
  { RAPTOR_OPTION_RESOURCE_BORDER,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "resourceBorder",
    "DOT serializer resource border color"
  },
  { RAPTOR_OPTION_LITERAL_BORDER,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "literalBorder",
    "DOT serializer literal border color"
  },
  { RAPTOR_OPTION_BNODE_BORDER,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "bnodeBorder",
    "DOT serializer blank node border color"
  },
  { RAPTOR_OPTION_RESOURCE_FILL,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "resourceFill",
    "DOT serializer resource fill color"
  },
  { RAPTOR_OPTION_LITERAL_FILL,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "literalFill",
    "DOT serializer literal fill color"
  },
  { RAPTOR_OPTION_BNODE_FILL,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "bnodeFill",
    "DOT serializer blank node fill color"
  },
  { RAPTOR_OPTION_HTML_TAG_SOUP,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "htmlTagSoup",
    "GRDDL parser uses a lax HTML parser"
  },
  { RAPTOR_OPTION_MICROFORMATS,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "microformats",
    "GRDDL parser looks for microformats"
  },
  { RAPTOR_OPTION_HTML_LINK,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "htmlLink",
    "GRDDL parser looks for <link type=\"application/rdf+xml\">"
  },
  { RAPTOR_OPTION_WWW_TIMEOUT,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_INT,
    "wwwTimeout",
    "Parser WWW request retrieval timeout"
  },
  { RAPTOR_OPTION_WRITE_BASE_URI,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "writeBaseURI",
    "Serializers write a base URI directive @base / xml:base"
  },
  { RAPTOR_OPTION_WWW_HTTP_CACHE_CONTROL,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "wwwHttpCacheControl",
    "Parser WWW request HTTP Cache-Control: header value"
  },
  { RAPTOR_OPTION_WWW_HTTP_USER_AGENT,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "wwwHttpUserAgent",
    "Parser WWW request HTTP User-Agent: header value"
  },
  { RAPTOR_OPTION_JSON_CALLBACK,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "jsonCallback",
    "JSON serializer callback function name"
  },
  { RAPTOR_OPTION_JSON_EXTRA_DATA,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "jsonExtraData",
    "JSON serializer callback data parameter"
  },
  { RAPTOR_OPTION_RSS_TRIPLES,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "rssTriples",
    "Atom and RSS serializers write extra RDF triples"
  },
  { RAPTOR_OPTION_ATOM_ENTRY_URI,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_URI,
    "atomEntryUri",
    "Atom serializer writes an atom:entry with this URI (otherwise atom:feed)"
  },
  { RAPTOR_OPTION_PREFIX_ELEMENTS,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "prefixElements",
    "Atom and RSS serializers write namespace-prefixed elements"
  },
  { RAPTOR_OPTION_STRICT,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "strict",
    "Operate in strict conformance mode (otherwise lax)"
  },
  { RAPTOR_OPTION_WWW_CERT_FILENAME,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "wwwCertFilename",
    "SSL client certificate filename"
  },
  { RAPTOR_OPTION_WWW_CERT_TYPE,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "wwwCertType",
    "SSL client certificate type"
  },
  { RAPTOR_OPTION_WWW_CERT_PASSPHRASE,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "wwwCertPassphrase",
    "SSL client certificate passphrase"
  },
  { RAPTOR_OPTION_NO_FILE,
    (raptor_option_area)(RAPTOR_OPTION_AREA_PARSER | RAPTOR_OPTION_AREA_SAX2),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "noFile",
    "Parsers and SAX2 deny internal file requests."
  },
  { RAPTOR_OPTION_WWW_SSL_VERIFY_PEER,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_INT,
    "wwwSslVerifyPeer",
    "SSL verify peer certficate"
  },
  { RAPTOR_OPTION_WWW_SSL_VERIFY_HOST,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_INT,
    "wwwSslVerifyHost",
    "SSL verify host matching"
  },
  { RAPTOR_OPTION_LOAD_EXTERNAL_ENTITIES,
    (raptor_option_area)(RAPTOR_OPTION_AREA_PARSER | RAPTOR_OPTION_AREA_SAX2),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "loadExternalEntities",
    "Parsers and SAX2 should load external entities."
  }
};


static const char * const raptor_option_uri_prefix = "http://feature.librdf.org/raptor-";
/* NOTE: this is strlen(raptor_option_uri_prefix) */
static const int raptor_option_uri_prefix_len = 33;


static raptor_option_area
raptor_option_get_option_area_for_domain(raptor_domain domain)
{
  raptor_option_area area = RAPTOR_OPTION_AREA_NONE;

  if(domain == RAPTOR_DOMAIN_PARSER) 
    area = RAPTOR_OPTION_AREA_PARSER;
  else if(domain == RAPTOR_DOMAIN_SERIALIZER)
    area = RAPTOR_OPTION_AREA_SERIALIZER;
  else if(domain == RAPTOR_DOMAIN_SAX2)
    area = RAPTOR_OPTION_AREA_SAX2;
  else if(domain == RAPTOR_DOMAIN_XML_WRITER)
    area = RAPTOR_OPTION_AREA_XML_WRITER;
  else if(domain == RAPTOR_DOMAIN_TURTLE_WRITER)
    area = RAPTOR_OPTION_AREA_TURTLE_WRITER;

  return area;
}


/**
 * raptor_free_option_description:
 * @option_description: option description
 * 
 * Destructor - free an option description object.
 */
void
raptor_free_option_description(raptor_option_description* option_description)
{
  if(!option_description)
    return;

  /* these are shared strings pointing to static data in raptor_options_list[] */
  /* RAPTOR_FREE(char*, option_description->name); */
  /* RAPTOR_FREE(char*, option_description->label); */

  if(option_description->uri)
    raptor_free_uri(option_description->uri);

  RAPTOR_FREE(raptor_option_description, option_description);
}


/**
 * raptor_world_get_option_description:
 * @world: raptor world object
 * @domain: domain
 * @option: option enumeration (0+)
 * 
 * Get a description of an option for a domain.
 *
 * The returned description must be freed with
 * raptor_free_option_description().
 *
 * Return value: option description or NULL on failure or if option is unknown
 **/
raptor_option_description*
raptor_world_get_option_description(raptor_world* world,
                                    const raptor_domain domain,
                                    const raptor_option option)
{
  raptor_option_area area;
  raptor_option_description *option_description = NULL;
  raptor_uri *base_uri = NULL;
  int i;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);
  
  area = raptor_option_get_option_area_for_domain(domain);
  if(area == RAPTOR_OPTION_AREA_NONE)
    return NULL;
  
  for(i = 0; i <= RAPTOR_OPTION_LAST; i++) {
    if(raptor_options_list[i].option == option &&
       (raptor_options_list[i].area & area))
      break;
  }

  if(i > RAPTOR_OPTION_LAST)
    return NULL;
  
  option_description = RAPTOR_CALLOC(raptor_option_description*, 1,
                                     sizeof(*option_description));
  if(!option_description)
    return NULL;

  option_description->domain = domain;
  option_description->option = option;
  option_description->value_type = raptor_options_list[i].value_type;
  option_description->name = raptor_options_list[i].name;
  option_description->name_len = strlen(option_description->name);
  option_description->label = raptor_options_list[i].label;

  base_uri = raptor_new_uri_from_counted_string(world,
                                                (const unsigned char*)raptor_option_uri_prefix,
                                                raptor_option_uri_prefix_len);
  if(!base_uri) {
    raptor_free_option_description(option_description);
    return NULL;
  }
  
  option_description->uri = raptor_new_uri_from_uri_local_name(world,
                                                               base_uri,
                                                               (const unsigned char*)raptor_options_list[i].name);
  raptor_free_uri(base_uri);
  if(!option_description->uri) {
    raptor_free_option_description(option_description);
    return NULL;
  }
  
  return option_description;
}



int
raptor_option_is_valid_for_area(const raptor_option option,
                                raptor_option_area area)
{
  if(option > RAPTOR_OPTION_LAST)
    return 0;
  return (raptor_options_list[option].area & area) != 0;
}


int
raptor_option_value_is_numeric(const raptor_option option)
{
  raptor_option_value_type t = raptor_options_list[option].value_type;
  
  return t == RAPTOR_OPTION_VALUE_TYPE_BOOL ||
         t == RAPTOR_OPTION_VALUE_TYPE_INT;
}


/**
 * raptor_world_get_option_from_uri:
 * @world: raptor_world instance
 * @uri: option URI
 *
 * Get an option ID from a URI
 * 
 * Option URIs are the concatenation of the string
 * "http://feature.librdf.org/raptor-" plus the short name.
 *
 * They are automatically returned for any option described with
 * raptor_world_get_option_description().
 *
 * Return value: < 0 if the option is unknown or on error
 **/
raptor_option
raptor_world_get_option_from_uri(raptor_world* world, raptor_uri *uri)
{
  unsigned char *uri_string;
  int i;
  raptor_option option = (raptor_option)-1;
  
  if(!uri)
    return option;
  
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, (raptor_option)-1);

  raptor_world_open(world);
  
  uri_string = raptor_uri_as_string(uri);
  if(strncmp((const char*)uri_string, raptor_option_uri_prefix,
             raptor_option_uri_prefix_len))
    return option;

  uri_string += raptor_option_uri_prefix_len;

  for(i = 0; i <= RAPTOR_OPTION_LAST; i++)
    if(!strcmp(raptor_options_list[i].name, (const char*)uri_string)) {
      option = (raptor_option)i;
      break;
    }

  return option;
}


/**
 * raptor_option_get_count:
 *
 * Get the count of options defined.
 *
 * This is prefered to the compile time-only symbol #RAPTOR_OPTION_LAST
 * and returns a count of the number of options which is
 * #RAPTOR_OPTION_LAST + 1.
 *
 * Return value: count of options in the #raptor_option enumeration
 **/
unsigned int
raptor_option_get_count(void)
{
  return RAPTOR_OPTION_LAST + 1;
}


const char* const
raptor_option_value_type_labels[RAPTOR_OPTION_VALUE_TYPE_URI + 1] = {
  "boolean",
  "integer",
  "string",
  "uri"
};


/**
 * raptor_option_get_value_type_label:
 * @type: value type
 *
 * Get a label for a value type
 *
 * Return value: label for type or NULL for invalid type
 */
const char*
raptor_option_get_value_type_label(const raptor_option_value_type type)
{
  if(type > RAPTOR_OPTION_VALUE_TYPE_LAST)
    return NULL;
  return raptor_option_value_type_labels[type];
}


int
raptor_object_options_copy_state(raptor_object_options* to,
                                 raptor_object_options* from)
{
  int rc = 0;
  int i;
  
  to->area = from->area;
  for(i = 0; !rc && i <= RAPTOR_OPTION_LAST; i++) {
    if(raptor_option_value_is_numeric((raptor_option)i))
      to->options[i].integer = from->options[i].integer;
    else {
      /* non-numeric values may need allocations */
      char* string = from->options[i].string;
      if(string) {
        size_t len = strlen(string);
        to->options[i].string = RAPTOR_MALLOC(char*, len + 1);
        if(to->options[i].string)
          memcpy(to->options[i].string, string, len + 1);
        else
          rc = 1;
      }
    }
  }
  
  return rc;
}


void
raptor_object_options_init(raptor_object_options* options,
                           raptor_option_area area)
{
  int i;
  
  options->area = area;

  for(i = 0; i <= RAPTOR_OPTION_LAST; i++) {
    if(raptor_option_value_is_numeric((raptor_option)i))
      options->options[i].integer = 0;
    else
      options->options[i].string = NULL;
  }

  /* Initialise default options that are not 0 or NULL */

  /* Emit @base directive or equivalent */
  options->options[RAPTOR_OPTION_WRITE_BASE_URI].integer = 1;
  
  /* Emit relative URIs where possible */
  options->options[RAPTOR_OPTION_RELATIVE_URIS].integer = 1;

  /* XML 1.0 output */
  options->options[RAPTOR_OPTION_WRITER_XML_VERSION].integer = 10;

  /* Write XML declaration */
  options->options[RAPTOR_OPTION_WRITER_XML_DECLARATION].integer = 1;

  /* Indent 2 spaces */
  options->options[RAPTOR_OPTION_WRITER_INDENT_WIDTH].integer = 2;

  /* lax (no strict) parsing */
  options->options[RAPTOR_OPTION_STRICT].integer = 0;

  /* SSL verify peers */
  options->options[RAPTOR_OPTION_WWW_SSL_VERIFY_PEER].integer = 1;

  /* SSL fully verify hosts */
  options->options[RAPTOR_OPTION_WWW_SSL_VERIFY_HOST].integer = 2;

}


void
raptor_object_options_clear(raptor_object_options* options)
{
  int i;
  
  for(i = 0; i <= RAPTOR_OPTION_LAST; i++) {
    if(raptor_option_value_is_numeric((raptor_option)i))
      continue;

    if(options->options[i].string)
      RAPTOR_FREE(char*, options->options[i].string);
  }
}


/*
 * raptor_object_options_get_option:
 * @options: options object
 * @option: option to get value
 * @string_p: pointer to where to store string value
 * @integer_p: pointer to where to store integer value
 *
 * INTERNAL - get option value
 * 
 * Any string value returned in *@string_p is shared and must be
 * copied by the caller.
 *
 * The allowed options vary by the area field of @options.
 *
 * Return value: option value or < 0 for an illegal option
 **/
int
raptor_object_options_get_option(raptor_object_options* options,
                                 raptor_option option,
                                 char** string_p, int* integer_p)
{
  if(!raptor_option_is_valid_for_area(option, options->area))
    return 1;
  
  if(raptor_option_value_is_numeric(option)) {
    /* numeric options */
    int value = options->options[(int)option].integer;
    if(integer_p)
      *integer_p = value;
  } else {
    /* non-numeric options */
    char* string = options->options[(int)option].string;
    if(string_p)
      *string_p = string;
  }
  
  return 0;
}


/*
 * raptor_object_options_set_option:
 * @options: options object
 * @option: option to set
 * @string: string option value (or NULL)
 * @integer: integer option value
 *
 * INTERNAL - set option
 * 
 * If @string is not NULL and the option type is numeric, the string
 * value is converted to an integer and used in preference to @integer.
 *
 * If @string is NULL and the option type is not numeric, an error is
 * returned.
 *
 * The @string values used are copied.
 *
 * The allowed options vary by the area field of @options.
 *
 * Return value: non 0 on failure or if the option is unknown
 **/
int
raptor_object_options_set_option(raptor_object_options *options,
                                 raptor_option option,
                                 const char* string, int integer)
{
  if(!raptor_option_is_valid_for_area(option, options->area))
    return 1;
  
  if(raptor_option_value_is_numeric(option)) {
    /* numeric options */
    if(string)
      integer = atoi((const char*)string);

    options->options[(int)option].integer = integer;
    return 0;
  } else {
    /* non-numeric options */
    char *string_copy;
    size_t len = 0;
    
    if(string)
      len = strlen((const char*)string);
    string_copy = RAPTOR_MALLOC(char*, len + 1);
    if(!string_copy)
      return 1;
  
    if(len)
      memcpy(string_copy, string, len);
    string_copy[len] = '\0';
    
    options->options[(int)option].string = string_copy;
  }
  
  return 0;
}
