/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_nfc_test.c - Raptor Unicode NFC validation check
 *
 * Copyright (C) 2004-2008, David Beckett http://www.dajobe.org/
 * Copyright (C) 2004-2004, University of Bristol, UK http://www.bristol.ac.uk/
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
 * It operates over the Unicode NormalizationTest.txt
 * which tests normalization the process, NOT normalization checking.
 * It says:
 * " CONFORMANCE:
 *   1. The following invariants must be true for all conformant implementations
 *      NFC
 *        c2 ==  NFC(c1) ==  NFC(c2) ==  NFC(c3)
 *        c4 ==  NFC(c4) ==  NFC(c5)
 * "
 *
 * It does NOT require that c1, c3 and c5 are NFC.
 */


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_raptor_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h> /* for isprint() */
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"
#include "raptor_nfc.h"


#undef RAPTOR_NFC_DECODE_DEBUG


/*
 * decode_to_utf8:
 * @utf8_string: destination utf8 buffer (FIXME big enough!)
 * @unicode_string: first char of string
 * @end: last char of unicode_string
 */
static int
decode_to_utf8(unsigned char *utf8_string, size_t utf8_string_length,
               const char *unicode_string, const char *end)
{
  unsigned char *u=utf8_string;
  const char *p=unicode_string;
  
#ifdef RAPTOR_NFC_DECODE_DEBUG
  fputs("decode_to_utf8: string '", stderr);
  (void)fwrite(unicode_string, sizeof(char), (end-unicode_string)+1, stderr);
  fputs("' converts to:\n  ", stderr);
#endif

  while(p < end) {
    unsigned long c=0;
    char *endptr;

    if(*p == ' ') {
      p++;
      continue;
    }
    
    c=(unsigned long)strtol(p, &endptr, 16);

#ifdef RAPTOR_NFC_DECODE_DEBUG
    fprintf(stderr, "U+%04lX ", c);
#endif

    p=(const char*)endptr;
    
    u+= raptor_unicode_char_to_utf8(c, u);
    
    if((u-utf8_string) > (int)utf8_string_length) {
      fprintf(stderr,
              "decode_to_utf8 overwote utf8_string buffer at byte %d\n",
              (u-utf8_string));
      abort();
    }
  }

#ifdef RAPTOR_NFC_DECODE_DEBUG
  fputs("\n", stderr);
#endif

  return u-utf8_string;
}



static void
utf8_print(const unsigned char *input, int length, FILE *stream)
{
  int i=0;
  
  while(i<length && *input) {
    unsigned long c;
    int size=raptor_utf8_to_unicode_char(&c, input, length-i);
    if(size <= 0)
      return;
    if(i)
      fputc(' ', stream);
    fprintf(stream, "U+%04X", (int)c);
    input += size;
    i += size;
  }
}


int
main (int argc, char *argv[]) 
{
  const char *program=raptor_basename(argv[0]);
  const char *filename;
  FILE *fh;
  int rc=0;
  unsigned int line=1;
  size_t max_c2_len=0;
  size_t max_c4_len=0;
  int passes=0;
  int fails=0;

  if(argc != 2) {
    fprintf(stderr,
            "USAGE %s [path to NormalizationTest.txt]\n"
            "Get it at http://unicode.org/Public/UNIDATA/NormalizationTest.txt\n",
            program);
    return 1;
  }
  
  filename=argv[1];
  fh=fopen(filename, "r");
  if(!fh) {
    fprintf(stderr, "%s: file '%s' open failed - %s\n",
            program, filename, strerror(errno));
    return 1;
  }

#define LINE_BUFFER_SIZE 1024

/* FIXME big enough for Unicode 4 (c2 max 16; c4 max 33) */
#define UNISTR_SIZE 40

  for(;!feof(fh); line++) {
    char buffer[LINE_BUFFER_SIZE];
    char *p, *start;
    unsigned char column2[UNISTR_SIZE];
    unsigned char column4[UNISTR_SIZE];
    size_t column2_len, column4_len;
    int nfc_rc;
    int error;
    
    p=fgets(buffer, LINE_BUFFER_SIZE, fh);
    if(!p) {
      if(ferror(fh)) {
        fprintf(stderr, "%s: file '%s' read failed - %s\n",
                program, filename, strerror(errno));
        rc=1;
        break;
      }
      /* assume feof */
      break;
    };

#if 0
    fprintf(stderr, "%s:%d: line '%s'\n", program, line, buffer);
#endif

    /* skip lines */
    if(*p == '@' || *p == '#')
      continue;

    if(line != 56)
      continue;
    

    /* skip column 1 */
    while(*p++ != ';')
      ;

    /* read column 2 into column2, column2_len */
    start=p;
    /* find end column 2 */
    while(*p++ != ';')
      ;

    column2_len=decode_to_utf8(column2, UNISTR_SIZE, start, p-1);
    if(column2_len > max_c2_len)
      max_c2_len=column2_len;
    
    /* skip column 3 */
    while(*p++ != ';')
      ;

    /* read column 4 into column4, column4_len */
    start=p;
    /* find end column 4 */
    while(*p++ != ';')
      ;

    column4_len=decode_to_utf8(column4, UNISTR_SIZE, start, p-1);
    if(column4_len > max_c4_len)
      max_c4_len=column4_len;

    if(!raptor_utf8_check(column2, column2_len)) {
      fprintf(stderr, "%s:%d: UTF8 column 2 failed on: '", filename, line);
      utf8_print(column2, column2_len, stderr);
      fputs("'\n", stderr);
      fails++;
    } else
      passes++;

    /* Column 2 must be NFC */
    nfc_rc=raptor_nfc_check(column2, column2_len, &error);
    if(!nfc_rc) {
      fprintf(stderr, "%s:%d: NFC column 2 failed on: '", filename, line);
      utf8_print(column2, column2_len, stderr);
      fprintf(stderr, "' at byte %d of %d\n", error, (int)column2_len);
      fails++;
    } else
      passes++;

    if(column2_len == column4_len && !memcmp(column2, column4, column2_len))
      continue;

    if(!raptor_utf8_check(column4, column4_len)) {
      fprintf(stderr, "%s:%d: UTF8 column 4 failed on: '", filename, line);
      utf8_print(column4, column4_len, stderr);
      fputs("'\n", stderr);
      fails++;
    } else
      passes++;

    /* Column 4 must be in NFC */
    nfc_rc=raptor_nfc_check(column4, column4_len, &error);
    if(!nfc_rc) {
      fprintf(stderr, "%s:%d: NFC column 4 failed on: '", filename, line);
      utf8_print(column4, column4_len, stderr);
      fprintf(stderr, "' at byte %d of %d\n", error, (int)column4_len);
      fails++;
    } else
      passes++;
  }

  fclose(fh);

  fprintf(stderr, "%s: max column 2 len: %d,  max column 4 len: %d\n", program,
          (int)max_c2_len, (int)max_c4_len);
  fprintf(stderr, "%s: passes: %d fails: %d\n", program,
          passes, fails);

  return rc;
}
