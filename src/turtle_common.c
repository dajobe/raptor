/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * turtle_common.c - Raptor Turtle common code
 *
 * Copyright (C) 2003-2007, David Beckett http://www.dajobe.org/
 * Copyright (C) 2003-2005, University of Bristol, UK http://www.bristol.ac.uk/
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
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"

#include <turtle_parser.h>
#include <turtle_lexer.h>
#include <turtle_common.h>

/**
 * raptor_stringbuffer_append_turtle_string:
 * @stringbuffer: String buffer to add to
 * @text: turtle string to decode
 * @len: length of string
 * @delim: terminating delimiter for string - only ', " or &gt; are allowed
 * @error_handler: error handling function
 * @error_data: error handler data
 *
 * Append to a stringbuffer a Turtle-escaped string.
 *
 * The passed in string is handled according to the Turtle string
 * escape rules giving a UTF-8 encoded output of the Unicode codepoints.
 *
 * The Turtle escapes are \b \f \n \r \t \\
 * \uXXXX \UXXXXXXXX where X is [A-F0-9]
 *
 * Turtle 2013 allows \ with -_~.!$&\'()*+,;=/?#@%
 *
 * URIs may not have \t \b \n \r \f or raw ' ' or \u0020 or \u003C or \u003E
 *
 * Return value: non-0 on failure
 **/
int
raptor_stringbuffer_append_turtle_string(raptor_stringbuffer* stringbuffer,
                                         const unsigned char *text,
                                         size_t len, int delim,
                                         raptor_simple_message_handler error_handler, 
                                         void *error_data,
                                         int is_uri)
{
  size_t i;
  const unsigned char *s;
  unsigned char *d;
  unsigned char *string = RAPTOR_MALLOC(unsigned char*, len + 1);
  const char* label = (is_uri ? "URI" : "string");

  if(!string)
    return -1;

  for(s = text, d = string, i = 0; i < len; s++, i++) {
    unsigned char c=*s;

    if(c == ' ' &&  is_uri) {
      error_handler(error_data,
                    "Turtle %s error - character '%c'", label, c);
      RAPTOR_FREE(char*, string);
      return 1;
    }

    if(c == '\\' ) {
      s++; i++;
      c = *s;
      if(c == 'n' || c == 'r' || c == 't' || c == 'b' || c == 'f') {
        if(is_uri) {
          error_handler(error_data,
                        "Turtle %s error - illegal URI escape '\\%c'", label, c);
          RAPTOR_FREE(char*, string);
          return 1;
        }
        if(c == 'n')
          *d++ = '\n';
        else if(c == 'r')
          *d++ = '\r';
        else if(c == 't')
          *d++ = '\t';
        else if(c == 'b')
          *d++ = '\b';
        else /* 'f' */
          *d++ = '\f';
      } else if(c == '\\' || c == delim ||
              c == '-' || c == '_' || c == '~' || c == '.' || c == '!' ||
              c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' ||
              c == '*' || c == '+' || c == ',' || c == ';' ||c == '=' ||
              c == '/' || c == '?' || c == '#' || c == '@' ||c == '%')
        *d++ = c;
      else if(c == 'u' || c == 'U') {
        size_t ulen = (c == 'u') ? 4 : 8;
        unsigned long unichar = 0;
        int n;
        int unichar_width;
        size_t ii;

        s++; i++;
        if(i+ulen > len) {
          error_handler(error_data,
                        "Turtle %s error - \\%c over end of line", label, c);
          RAPTOR_FREE(char*, string);
          return 1;
        }

        for(ii = 0; ii < ulen; ii++) {
          char cc = s[ii];
          if(!isxdigit(RAPTOR_GOOD_CAST(char, cc))) {
            error_handler(error_data,
                          "Turtle %s error - illegal hex digit %c in Unicode escape '%c%s...'",
                          label, cc, c, s);
            RAPTOR_FREE(char*, string);
            return 1;
          }
        }

        n = sscanf((const char*)s, ((ulen == 4) ? "%04lx" : "%08lx"), &unichar);
        if(n != 1) {
          error_handler(error_data,
                        "Turtle %s error - illegal Unicode escape '%c%s...'",
                        label, c, s);
          RAPTOR_FREE(char*, string);
          return 1;
        }

        s+= ulen-1;
        i+= ulen-1;
        
        if(is_uri && (unichar == 0x0020 || unichar == 0x003C || unichar == 0x003E)) {
          error_handler(error_data,
                        "Turtle %s error - illegal Unicode escape \\u%04lX in URI.", label, unichar);
          break;
        }

        if(unichar > raptor_unicode_max_codepoint) {
          error_handler(error_data,
                        "Turtle %s error - illegal Unicode character with code point #x%lX (max #x%lX).", 
                        label, unichar, raptor_unicode_max_codepoint);
          RAPTOR_FREE(char*, string);
          return 1;
        }
          
        unichar_width = raptor_unicode_utf8_string_put_char(unichar, d, 
                                                            len-(d-string));
        if(unichar_width < 0) {
          error_handler(error_data,
                        "Turtle %s error - illegal Unicode character with code point #x%lX.", 
                        label, unichar);
          RAPTOR_FREE(char*, string);
          return 1;
        }
        d += (size_t)unichar_width;

      } else {
        /* don't handle \x where x isn't one of: \t \n \r \\ (delim) */
        error_handler(error_data,
                      "Turtle %s error - illegal escape \\%c (#x%02X) in \"%s\"", 
                      label, c, c, text);
      }
    } else
      *d++=c;
  }
  *d='\0';

  /* calculate output string size */
  len = d-string;
  
  /* string gets owned by the stringbuffer after this */
  return raptor_stringbuffer_append_counted_string(stringbuffer, 
                                                   string, len, 0);

}


/**
 * raptor_turtle_expand_qname_escapes:
 * @name: turtle qname string to decode
 * @len: length of name
 * @error_handler: error handling function
 * @error_data: error handler data
 *
 * Expands Turtle escapes for the given turtle qname string
 *
 * The passed in string is handled according to the Turtle string
 * escape rules giving a UTF-8 encoded output of the Unicode codepoints.
 *
 * The Turtle escapes are \b \f \n \r \t \\
 * \uXXXX \UXXXXXXXX where X is [A-F0-9]
 *
 * Turtle 2013 allows \ with -_~.!$&\'()*+,;=/?#@%
 *
 * Return value: new length or 0 on failure
 **/
size_t
raptor_turtle_expand_qname_escapes(unsigned char *name,
                                   size_t len,
                                   raptor_simple_message_handler error_handler, 
                                   void *error_data)
{
  size_t i;
  const unsigned char *s;
  unsigned char *d;
  
  if(!name)
    return -1;

  for(s = name, d = name, i = 0; i < len; s++, i++) {
    unsigned char c=*s;

    if(c == '\\' ) {
      s++; i++;
      c = *s;
      if(c == 'n')
        *d++ = '\n';
      else if(c == 'r')
        *d++ = '\r';
      else if(c == 't')
        *d++ = '\t';
      else if(c == 'b')
        *d++ = '\b';
      else if(c == 'f')
        *d++ = '\f';
      else if(c == '\\' ||
              c == '-' || c == '_' || c == '~' || c == '.' || c == '!' ||
              c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' ||
              c == '*' || c == '+' || c == ',' || c == ';' ||c == '=' ||
              c == '/' || c == '?' || c == '#' || c == '@' ||c == '%')
        *d++ = c;
      else if(c == 'u' || c == 'U') {
        size_t ulen = (c == 'u') ? 4 : 8;
        unsigned long unichar = 0;
        int n;
        int unichar_width;
        size_t ii;

        s++; i++;
        if(i+ulen > len) {
          error_handler(error_data,
                        "Turtle name error - \\%c over end of line", c);
          return 1;
        }
        
        for(ii = 0; ii < ulen; ii++) {
          char cc = s[ii];
          if(!isxdigit(RAPTOR_GOOD_CAST(char, cc))) {
            error_handler(error_data,
                          "Turtle name error - illegal hex digit %c in Unicode escape '%c%s...'",
                          cc, c, s);
            return 1;
          }
        }

        n = sscanf((const char*)s, ((ulen == 4) ? "%04lx" : "%08lx"), &unichar);
        if(n != 1) {
          error_handler(error_data,
                        "Turtle name error - illegal Uncode escape '%c%s...'",
                        c, s);
          return 1;
        }

        s+= ulen-1;
        i+= ulen-1;
        
        if(unichar > raptor_unicode_max_codepoint) {
          error_handler(error_data,
                        "Turtle name error - illegal Unicode character with code point #x%lX (max #x%lX).", 
                        unichar, raptor_unicode_max_codepoint);
          return 1;
        }
          
        unichar_width = raptor_unicode_utf8_string_put_char(unichar, d, 
                                                            len - (d-name));
        if(unichar_width < 0) {
          error_handler(error_data,
                        "Turtle name error - illegal Unicode character with code point #x%lX.", 
                        unichar);
          return 1;
        }
        d += (size_t)unichar_width;

      } else {
        /* don't handle \x where x isn't one of: \t \n \r \\ (delim) */
        error_handler(error_data,
                      "Turtle name error - illegal escape \\%c (#x%02X) in \"%s\"", 
                      c, c, name);
      }
    } else
      *d++ = c;
  }
  *d='\0';

  /* calculate output string size */
  len = d - name;
  
  /* string gets owned by the stringbuffer after this */
  return len;
}
