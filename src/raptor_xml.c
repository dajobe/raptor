f/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_xml.c - Raptor XML routines
 *
 * $Id$
 *
 * Copyright (C) 2003 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.org/
 * University of Bristol - http://www.bristol.ac.uk/
 * 
 * This package is Free Software or Open Source available under the
 * following licenses (these are alternatives):
 *   1. GNU Lesser General Public License (LGPL)
 *   2. GNU General Public License (GPL)
 *   3. Mozilla Public License (MPL)
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * full license terms.
 * 
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
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

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/**
 * raptor_validate_xml_ID - check the string matches the xml:ID value constraints
 * @rdf_parser: RDF parser
 * @string: The string to check.
 *
 * This checks the syntax part of the xml:ID validity constraint,
 * that it matches [ VC: Name Token ] as amended by XML Namespaces:
 *
 *   http://www.w3.org/TR/REC-xml-names/#NT-NCName
 * 
 * Returns non-zero if the ID string is valid
 **/
int
raptor_valid_xml_ID(raptor_parser *rdf_parser, const unsigned char *string)
{
  unsigned char c;
  int len=strlen((const char*)string);
  int unichar_len;
  long unichar;
  int pos;
  
  for(pos=0; (c=*string); string++, len--, pos++) {

    /* It is unicode */
    
    unichar_len=raptor_utf8_to_unicode_char(NULL, (const unsigned char *)string, len);
    if(unichar_len < 0 || unichar_len > len) {
      raptor_parser_error(rdf_parser, "Bad UTF-8 encoding missing.");
      return 0;
    }
  
    unichar_len=raptor_utf8_to_unicode_char(&unichar,
                                            (const unsigned char *)string, len);
    if(!pos) {
      /* start of xml:ID name */
      if(!raptor_unicode_is_namestartchar(unichar))
        return 0;
    } else {
      /* rest of xml:ID name */
      if(!raptor_unicode_is_namechar(unichar))
        return 0;
    }

    unichar_len--; /* since loop does len-- */
    string += unichar_len; len -= unichar_len;
  }
  return 1;
}



/**
 * raptor_xml_escape_string - return an XML-escaped version of the current string
 * @string: string to XML escape
 * @len: length of string
 * @buffer: the buffer to use
 * @length: buffer size
 * @quote: optional quote character to escape, or 0
 * 
 * Replaces each &, < and > with &amp; &lt; &gt; respectively
 * and turns characters 0x7e to 0xff inclusive into the 
 * escaped &#nnn form.
 *
 * If quote is given it can be either of '\'' or '\"'
 * which will be turned into &apos; or &quot; respectively.
 * ASCII NUL ('\0') or any other character will not be escaped.
 * 
 * If buffer is NULL, no work is done but the size of buffer
 * required is returned.
 *
 * Return value: the number of bytes written or 0 on failure.
 **/
size_t
raptor_xml_escape_string(raptor_parser *rdf_parser, 
                         const unsigned char *string, size_t len,
                         unsigned char *buffer, size_t length,
                         char quote)
{
  int l;
  int new_len=0;
  const unsigned char *p;
  char *q;
  int unichar_len;
  unsigned long unichar;

  if(quote != '\"' && quote != '\'')
    quote='\0';

  for(l=len, p=string; l; p++, l--) {
    if(*p > 0x7f) {
      unichar_len=raptor_utf8_to_unicode_char(&unichar, p, l);
      if(unichar_len < 0 || unichar_len > l) {
        raptor_parser_error(rdf_parser, "Bad UTF-8 encoding missing.");
        return 0;
      }
    } else {
      unichar=*p;
      unichar_len=1;
    }
  
    if(unichar == '&')
      /* &amp; */
      new_len+=5;
    else if(unichar == '<' || unichar == '>')
      /* &lt; or &gt; */
      new_len+=4;
    else if (quote && unichar == quote)
      /* &apos; or &quot; */
      new_len+= 6;
    else if (unichar == 0x09 || unichar == 0x0d || unichar == 0x0a)
      /* XML whitespace excluding 0x20, since next condition skips it */
      new_len++;
    else if (unichar < 0x20 || unichar > 0x7e) {
      /* &#xXX; (0/0x00 to 255/0xff) */
      new_len += 6;
      if(unichar > 0xff)
        /* &#xXXXX; */
        new_len+=2;
      if(unichar > 0xffff)
        /* &#xXXXXX; */
        new_len++;
      if(unichar > 0xfffff)
        /* &#xXXXXXX; */
        new_len++;
    } else
      new_len++;

    unichar_len--; /* since loop does len-- */
    p += unichar_len; l -= unichar_len;
  }

  if(length && new_len > length)
    return 0;

  if(!buffer)
    return new_len;
  
  for(l=len, p=string, q=buffer; l; p++, l--) {
    if(*p > 0x7f) {
      unichar_len=raptor_utf8_to_unicode_char(&unichar, p, l);
    } else {
      unichar=*p;
      unichar_len=1;
    }

    if(unichar == '&') {
      strncpy(q, "&amp;", 5);
      q+= 5;
    } else if (unichar == '<') {
      strncpy(q, "&lt;", 4);
      q+= 4;
    } else if (unichar == '>') {
      strncpy(q, "&gt;", 4);
      q+= 4;
    } else if (quote && unichar == quote) {
      if(quote == '\'')  
        strncpy(q, "&apos;", 6);
      else
        strncpy(q, "&quot;", 6);
      q+= 6;
    } else if (unichar == 0x09 || unichar == 0x0d || unichar == 0x0a)
      /* XML whitespace excluding 0x20, since next condition skips it */
      *q++ = unichar;
    else if (unichar < 0x20 || unichar > 0x7e) {
      if(unichar < 0x100) {
        /* &#xXX; */
        sprintf(q, "&#x%02lX;", unichar);
        q+= 6;
      } else if (unichar < 0x10000) {
        /* &#xXXXX; */
        sprintf(q, "&#x%04lX;", unichar);
        q+= 8;
      } else if (unichar < 0x100000) {
        /* &#xXXXXX; */
        sprintf(q, "&#x%05lX;", unichar);
        q+= 9;
      } else {
        /* &#xXXXXXX; */
        sprintf(q, "&#x%06lX;", unichar);
        q+= 10;
      }
    } else
      *q++ = unichar;

    unichar_len--; /* since loop does len-- */
    p += unichar_len; l -= unichar_len;
  }

  /* Terminate new string */
  *q = '\0';

  return new_len;
}
