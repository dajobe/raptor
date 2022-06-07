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

#include <unicode/ustring.h>
#if ICU_UC_MAJOR_VERSION >= 56
#include <unicode/unorm2.h>
#else
#include <unicode/unorm.h>
#endif


/*
 * raptor_nfc_icu_check:
 * @input: UTF-8 string
 * @length: length of string
 * @error: pointer to error flag (or NULL)
 *
 * INTERNAL - Unicode Normal Form C (NFC) check function via ICU
 *
 * If errorp is not NULL, it is set to non-0 on error
 * 
 * Return value: <0 on error, 0 if is not NFC, >0 if is NFC
 **/
int
raptor_nfc_icu_check(const unsigned char* string, size_t len)
{
#if ICU_UC_MAJOR_VERSION >= 56
  /* norm2 is be a singleton - do not attempt to free it */
  const UNormalizer2 *norm2;
#endif
  UErrorCode error_code = U_ZERO_ERROR;
  UNormalizationCheckResult res;
  UChar *dest; /* UTF-16 */
  int32_t dest_capacity = len << 1;
  int32_t dest_length;
  int rc = 0;

  /* ICU functions take a UTF-16 string so convert */
  dest = RAPTOR_MALLOC(UChar*, dest_capacity + 1);
  if(!dest)
    goto error;

  (void)u_strFromUTF8(dest, dest_capacity, &dest_length,
                      (const char *)string, (int32_t)len, &error_code);
  if(!U_SUCCESS(error_code))
    goto error;

  /* unorm_quickCheck was deprecated in ICU UC V56 */
#if ICU_UC_MAJOR_VERSION >= 56
  norm2 = unorm2_getNFCInstance(&error_code);
  if(!U_SUCCESS(error_code))
    goto error;

  res = unorm2_quickCheck(norm2, dest, dest_length, &error_code);
#else
  res = unorm_quickCheck(dest, dest_length, UNORM_NFC, &error_code);
#endif
  if(!U_SUCCESS(error_code))
    goto error;

  /* success */
  rc = (res == UNORM_YES);
  goto cleanup;

error:
  rc = -1;

cleanup:
  if(dest)
    RAPTOR_FREE(UChar*, dest);

  return rc;
}
