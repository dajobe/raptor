/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_log.c - Raptor log handling
 *
 * Copyright (C) 2000-2010, David Beckett http://www.dajobe.org/
 * Copyright (C) 2000-2005, University of Bristol, UK http://www.bristol.ac.uk/
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


static const char* const raptor_log_level_labels[RAPTOR_LOG_LEVEL_LAST + 1] = {
  "none",
  "trace",
  "debug",
  "info",
  "warning",
  "error",
  "fatal error"
};


/**
 * raptor_log_level_get_label:
 * @level: log message level
 *
 * Get label for a log message level
 *
 * Return value: label string or NULL if level is not valid
 */
const char*
raptor_log_level_get_label(raptor_log_level level)
{
  return (level <= RAPTOR_LOG_LEVEL_LAST) ? raptor_log_level_labels[level] : NULL;
}


void
raptor_log_error_varargs(raptor_world* world, raptor_log_level level,
                         raptor_locator* locator,
                         const char* message, va_list arguments)
{
  char *buffer = NULL;
  size_t length;
  
  if(level == RAPTOR_LOG_LEVEL_NONE)
    return;

  if(world && world->internal_ignore_errors)
    return;

  length = raptor_vasprintf(&buffer, message, arguments);
  if(!buffer) {
    if(locator) {
      raptor_locator_print(locator, stderr);
      fputc(' ', stderr);
    }
    fputs("raptor ", stderr);
    fputs(raptor_log_level_labels[level], stderr);
    fputs(" - ", stderr);
    vfprintf(stderr, message, arguments);
    fputc('\n', stderr);
    return;
  }

  if(length >= 1 && buffer[length-1] == '\n')
    buffer[length-1]='\0';
  
  raptor_log_error(world, level, locator, buffer);

  RAPTOR_FREE(char*, buffer);
}


void
raptor_log_error_formatted(raptor_world* world, raptor_log_level level,
                           raptor_locator* locator,
                           const char* message, ...)
{
  va_list arguments;

  va_start(arguments, message);
  raptor_log_error_varargs(world, level, locator, message, arguments);
  va_end(arguments);
}


/* internal */
void
raptor_log_error(raptor_world* world, raptor_log_level level,
                 raptor_locator* locator, const char* text)
{
  raptor_log_handler handler;

  if(level == RAPTOR_LOG_LEVEL_NONE)
    return;

  if(world) {
    if(world->internal_ignore_errors)
      return;

    memset(&world->message, '\0', sizeof(world->message));
    world->message.code = -1;
    world->message.domain = RAPTOR_DOMAIN_NONE;
    world->message.level = level;
    world->message.locator = locator;
    world->message.text = text;
  
    handler = world->message_handler;
    if(handler) {
      /* This is the place in raptor that ALL of the user error handler
       * functions are called.
       */
      handler(world->message_handler_user_data, &world->message);
      return;
    }
  }

  /* default - print it to stderr */
  if(locator) {
    raptor_locator_print(locator, stderr);
    fputc(' ', stderr);
  }
  fputs("raptor ", stderr);
  fputs(raptor_log_level_labels[level], stderr);
  fputs(" - ", stderr);
  fputs(text, stderr);
  fputc('\n', stderr);
}
