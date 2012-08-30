/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_nfc_icu.c - Raptor Unicode NFC checking via ICU library
 *
 * Copyright (C) 2012, David Beckett http://www.dajobe.org/
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"

#include <unicode/unorm.h>

/**
 * raptor_nfc_icu_check:
 * @input: UTF-8 string
 * @length: length of string
 * @errorp: pointer to store offset of character in error (or NULL)
 *
 * Unicode Normal Form C (NFC) check function.
 *
 * If errorp is not NULL, it is set to the offset of the character
 * in error in the buffer, or <0 if there is no error.
 * 
 * Return value: Non 0 if the string is NFC
 **/
int
raptor_nfc_icu_check(const unsigned char* string, size_t len, int *error)
{
   UNormalizationCheckResult res;
   UErrorCode error_code = U_ZERO_ERROR;
   
   res = unorm_quickCheck((const UChar *)string, (int32_t)len,
                          UNORM_NFC, &error_code);
   if(!U_SUCCESS(error_code)) {
     if(error)
       *error = 1;
     return 0;
   }
   
   return (res == UNORM_YES);
}
