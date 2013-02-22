/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_name.c - Raptor name checking utilities
 *
 * Copyright (C) 2013, David Beckett http://www.dajobe.org/
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

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


/**
 * raptor_sparql_name_check_bitflags:
 * @RAPTOR_SPARQL_NAME_CHECK_ALLOW_UL_09_FIRST: '_' '0'-'9' is allowed at start
 * @RAPTOR_SPARQL_NAME_CHECK_ALLOW_DOT_LAST: '.' is allowed at end
 * @RAPTOR_SPARQL_NAME_CHECK_ALLOW_COLON_HEX_BS: allow ':', '%'[A-Fa-f0-9][A-Fa-f0-9] and \+certain chars everywhere
 * @RAPTOR_SPARQL_NAME_CHECK_ALLOW_MINUS_REST: '-' is allowed in middle/last
 *
 * INTERNAL - things to allow in a name
 *
 */
typedef enum {
  RAPTOR_SPARQL_NAME_CHECK_ALLOW_UL_09_FIRST  = 1,
  RAPTOR_SPARQL_NAME_CHECK_ALLOW_DOT_LAST     = 2,
  RAPTOR_SPARQL_NAME_CHECK_ALLOW_COLON_HEX_BS = 4,
  RAPTOR_SPARQL_NAME_CHECK_ALLOW_MINUS_REST   = 8
} raptor_sparql_name_check_bitflags;


/**
 * raptor_sparql_name_check:
 * @string: name string
 * @length: name length
 * @check_type: check to make
 *
 * Check a UTF-8 encoded name string to match SPARQL / Turtle name constraints
 *
 * The input is a sequence of characters taken from the set:
 *   "A-Z0-9a-z-._:%\\\x80-\xff"
 * where \ can be followed by one of the characters in the set:
 *   "_~.-!$&'()*+,;=/?#@%"
 * and % must be followed by two characters in the set:
 *   "A-Fa-f0-9"
 *
 * See also raptor_xml_name_check(),
 * raptor_unicode_is_xml11_namestartchar() and
 * raptor_unicode_is_xml11_namechar() which check what XML 1.1 allows
 * in names.
 *
 * Return value: <0 on error (such as invalid @check_type or empty string), 0 if invalid, >0 if valid
 */
int
raptor_sparql_name_check(unsigned char *string, size_t length,
                         raptor_sparql_name_check_type check_type)
{
  unsigned char *p = string;
  int rc = 0;
  raptor_sparql_name_check_bitflags check_bits;
  int pos;
  raptor_unichar unichar = 0;
  int unichar_len = 0;
  int escaping = 0;
  int hexing = 0; /* 0: not in hex. 1, 2: at hex digit */

  if(check_type == RAPTOR_SPARQL_NAME_CHECK_VARNAME)
    check_bits = RAPTOR_SPARQL_NAME_CHECK_ALLOW_UL_09_FIRST;
  else if(check_type == RAPTOR_SPARQL_NAME_CHECK_PREFIXED_NAME)
    check_bits = RAPTOR_SPARQL_NAME_CHECK_ALLOW_MINUS_REST;
  else if(check_type == RAPTOR_SPARQL_NAME_CHECK_LOCAL_NAME)
    check_bits = RAPTOR_SPARQL_NAME_CHECK_ALLOW_UL_09_FIRST |
                 RAPTOR_SPARQL_NAME_CHECK_ALLOW_MINUS_REST |
                 RAPTOR_SPARQL_NAME_CHECK_ALLOW_COLON_HEX_BS;
  else if(check_type == RAPTOR_SPARQL_NAME_CHECK_BLANK)
    check_bits = RAPTOR_SPARQL_NAME_CHECK_ALLOW_UL_09_FIRST |
                 RAPTOR_SPARQL_NAME_CHECK_ALLOW_MINUS_REST;
  else
    return -1;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 2
  RAPTOR_DEBUG1("Checking name '");
  if(length)
     fwrite(string, length, sizeof(unsigned char), stderr);
  fprintf(stderr, "' (length %d), flags %d\n",
          RAPTOR_BAD_CAST(int, length), RAPTOR_GOOD_CAST(int, check_flags));
#endif

  if(!length)
    return -1;

  for(pos = 0, p = string;
      length > 0;
      pos++, p += unichar_len, length -= unichar_len) {

    unichar_len = raptor_unicode_utf8_string_get_char(p, length, &unichar);
    if(unichar_len < 0 || RAPTOR_GOOD_CAST(size_t, unichar_len) > length)
      goto done;

    if(unichar > raptor_unicode_max_codepoint)
      goto done;

    /* checks for anywhere in name */

    if(hexing) {
      if(!(unichar >= '0' && unichar <= '9') ||
          (unichar >= 'a' && unichar <= 'f') ||
          (unichar >= 'F' && unichar <= 'F')) {
        goto done;
      }
      if(hexing++ == 3)
        hexing = 0;
      continue;
    }
    
    /* This is NOT allowed by XML */
    if(escaping) {
      if(!(check_bits & RAPTOR_SPARQL_NAME_CHECK_ALLOW_COLON_HEX_BS))
        goto done;

      escaping = 0;
      if(unichar == '_' || unichar == '~' || unichar == '.' ||
         unichar == '-' || unichar == '!' || unichar == '$' ||
         unichar == '&' || unichar == '\'' || unichar == '(' ||
         unichar == ')' || unichar == '*' || unichar == '+' ||
         unichar == ',' || unichar == ';' || unichar == '=' ||
         unichar == '/' || unichar == '?' || unichar == '#' ||
         unichar == '@' || unichar == '%')
        continue;
      goto done;
    }
    escaping = (unichar == '\\');
    if(escaping)
      continue;

    /* This is NOT allowed by XML */
    if(unichar == ':') {
      if(check_bits & RAPTOR_SPARQL_NAME_CHECK_ALLOW_COLON_HEX_BS)
        continue;
      goto done;
    }

    /* This is NOT allowed by XML */
    if(unichar == '%') {
      if(check_bits & RAPTOR_SPARQL_NAME_CHECK_ALLOW_COLON_HEX_BS) {
        hexing = 1;
        continue;
      }
      goto done;
    }

    if(!pos) {
      /* start of name */

      /* This is NOT allowed by XML */
      if(unichar >= '0' && unichar <= '9') {
        if(check_bits & RAPTOR_SPARQL_NAME_CHECK_ALLOW_UL_09_FIRST)
          continue;
        goto done;
      }

      /* This IS allowed by XML */
      if(unichar == '_') {
        if(check_bits & RAPTOR_SPARQL_NAME_CHECK_ALLOW_UL_09_FIRST)
          continue;
        goto done;
      }

      if(!raptor_unicode_is_xml11_namestartchar(unichar)) {
        goto done;
      }
    } else {
      /* middle / end of name  */

      /* :, hex, \-escapes is tested above */

      if(unichar == '-') {
        if(check_bits & RAPTOR_SPARQL_NAME_CHECK_ALLOW_MINUS_REST)
          continue;
        goto done;
      }

      if(!raptor_unicode_is_xml11_namechar(unichar)) {
        goto done;
      }
    }
  }

  /* Check final character */
  /* This IS allowed by XML */
  if(unichar == '.') {
    if(!(check_bits & RAPTOR_SPARQL_NAME_CHECK_ALLOW_DOT_LAST))
      goto done;
  }

  /* It is good */
  rc = 1;

  done:

  return rc;
}
