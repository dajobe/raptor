/* -*- Mode: c; c-basic-offset: 2 -*-
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
 * raptor_xml_escape_string - return an XML-escaped version a string
 * @string: string to XML escape (UTF-8)
 * @len: length of string
 * @buffer: the buffer to use for new string (UTF-8)
 * @length: buffer size
 * @quote: optional quote character to escape for attribute content, or 0
 * 
 * Follows Canonical XML rules on Text Nodes and Attribute Nodes
 *   http://www.w3.org/TR/xml-c14n#ProcessingModel
 *
 * Both:
 *   Replaces & and < with &amp; and &lt; respectively, preserving other
 *   characters.
 * 
 * Text Nodes:
 *   > is turned into &gt; #xD is turned into &#xD;
 *
 * Attribute Nodes:
 *   > is generated not &gt.  #x9, #xA and #xD are turned into
 *   &#x9; &#xA; and &#xD; entities.
 *
 * If quote is given it can be either of '\'' or '\"'
 * which will be turned into &apos; or &quot; respectively.
 * ASCII NUL ('\0') or any other character will not be escaped.
 * 
 * If buffer is NULL, no work is done but the size of buffer
 * required is returned.  The output in buffer remains in UTF-8.
 *
 * Return value: the number of bytes required / used or 0 on failure.
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
      new_len+= 5;
    else if(unichar == '<' || (!quote && unichar == '>'))
      /* &lt; or &gt; */
      new_len+= 4;
    else if (quote && unichar == quote)
      /* &apos; or &quot; */
      new_len+= 6;
    else if (unichar == 0x0d ||
             (quote && (unichar == 0x09 || unichar == 0x0a)))
      /* &#xD; or &#x9; or &xA; */
      new_len+= 5;
    else
      new_len+= unichar_len;

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
    } else if (!quote && unichar == '>') {
      strncpy(q, "&gt;", 4);
      q+= 4;
    } else if (quote && unichar == quote) {
      if(quote == '\'')  
        strncpy(q, "&apos;", 6);
      else
        strncpy(q, "&quot;", 6);
      q+= 6;
    } else if (unichar == 0x0d ||
               (quote && (unichar == 0x09 || unichar == 0x0a))) {
      /* &#xX; */
      *q++='&';
      *q++='#';
      *q++='x';
      if(unichar == 0x09)
        *q++ = '9';
      else
        *q++ = 'A'+unichar-0x0a;
      *q++= ';';
    } else {
      strncpy(q, p, unichar_len);
      q+= unichar_len;
    }

    unichar_len--; /* since loop does len-- */
    p += unichar_len; l -= unichar_len;
  }

  /* Terminate new string */
  *q = '\0';

  return new_len;
}





#ifdef STANDALONE

/* static prototypes */
void raptor_bad_string_print(const unsigned char *input, FILE *stream);
int main(int argc, char *argv[]);

void
raptor_bad_string_print(const unsigned char *input, FILE *stream)
{
  while(*input) {
    char c=*input;
    if(isprint(c))
      fputc(c, stream);
    else
      fprintf(stream, "\\x%02X", (c & 0xff));
    input++;
  }
}


int
main(int argc, char *argv[]) 
{
  char *program=argv[0];
  struct tv {
    const unsigned char *string;
    const char quote;
    const unsigned char *result;
  };
  struct tv *t;
  struct tv test_values[]={
    {"&", 0, "&amp;"},
    {"<", 0, "&lt;"},
    {">", 0, "&gt;"},
    {"\x09", 0, "\x09"},
    {"\x0a", 0, "\x0a"},
    {"\x0d", 0, "&#xD;"},

    {"'&'",  '\'', "&apos;&amp;&apos;"},
    {"'<'",  '\'', "&apos;&lt;&apos;"},
    {"'>'",  '\'', "&apos;>&apos;"},
    {"\x09", '\'', "&#x9;"},
    {"\x0a", '\'', "&#xA;"},
    {"\x0d", '\'', "&#xD;"},

    {"\"&\"", '\"', "&quot;&amp;&quot;"},
    {"\"<\"", '\"', "&quot;&lt;&quot;"},
    {"\">\"", '\"', "&quot;>&quot;"},
    {"\x09",  '\"', "&#x9;"},
    {"\x0a",  '\"', "&#xA;"},
    {"\x0d",  '\"', "&#xD;"},

    {"&amp;", 0, "&amp;amp;"},
    {"<foo>", 0, "&lt;foo&gt;"},
#if 0
    {"\x1f", 0, "&#x1F;"},
    {"\xc2\x80", 0, "&#x80;"},
    {"\xe0\xa0\x80", 0, "&#x0800;"},
    {"\xf0\x90\x80\x80", 0, "&#x10000;"},

    {"\x7f", 0, "&#x7F;"},
    {"\xdf\xbf", 0, "&#x07FF;"},
    {"\xef\xbf\xbd", 0, "&#xFFFD;"},
    {"\xf4\x8f\xbf\xbf", 0, "&#x10FFFF;"},

    {"\xc3\xbf", 0, "&#xFF;"},
    {"\xf0\x8f\xbf\xbf", 0, "&#xFFFF;"},
#endif
    {NULL, 0, 0}
  };
  int i;
  int failures=0;

  for(i=0; (t=&test_values[i]) && t->string; i++) {
    const unsigned char *utf8_string=t->string;
    int quote=t->quote;
    size_t utf8_string_len=strlen(utf8_string);
    unsigned char *xml_string;
    int xml_string_len=0;

    xml_string_len=raptor_xml_escape_string(NULL, utf8_string, utf8_string_len,
                                            NULL, 0, quote);
    xml_string=(char*)RAPTOR_MALLOC(cstring, xml_string_len+1);
    
    xml_string_len=raptor_xml_escape_string(NULL, utf8_string, utf8_string_len,
                                            xml_string, xml_string_len, quote);
    if(!xml_string) {
      fprintf(stderr, "%s: raptor_xml_escape_string FAILED to escape string '",
              program);
      raptor_bad_string_print(utf8_string, stderr);
      fputs("'\n", stderr);
      failures++;
      continue;
    }
    if(strcmp(xml_string, t->result)) {
      fprintf(stderr, "%s: raptor_xml_escape_string FAILED to escape string '",
              program);
      raptor_bad_string_print(utf8_string, stderr);
      fprintf(stderr, "', expected '%s', result was '%s'\n",
              t->result, xml_string);
      failures++;
      continue;
    }
    
    fprintf(stderr, "%s: raptor_xml_escape_string escaped string to '%s' ok\n",
            program, xml_string);
    RAPTOR_FREE(cstring, xml_string);
  }

  if(!failures)
    fprintf(stderr, "%s: raptor_xml_escape_string all tests OK\n", program);

  return failures;
}

#endif
