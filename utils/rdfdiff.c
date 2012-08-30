/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdfdiff.c - Raptor RDF diff tool
 *
 * Copyright (C) 2000-2008, David Beckett http://www.dajobe.org/
 * Copyright (C) 2000-2005, University of Bristol, UK http://www.bristol.ac.uk/
 * Copyright (C) 2005, Steve Shepard steveshep@gmail.com
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

/* Raptor includes */
#include <raptor2.h>
#include <raptor_internal.h>

/* for access() and R_OK */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef WIN32
#include <io.h>
#endif

/* many places for getopt */
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include <raptor_getopt.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if 0
#undef RAPTOR_DEBUG
#define RAPTOR_DEBUG 2
#endif

#ifndef RAPTOR_INTERNAL
#define RAPTOR_MALLOC(type, size)		(type)malloc(size)
#define RAPTOR_CALLOC(type, nmemb, size)	(type)calloc(nmemb, size)
#define RAPTOR_FREE(type, ptr)			free((void*)ptr)
#endif

#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif

#define MAX_ASCII_INT_SIZE 13
#define RDF_NAMESPACE_URI_LEN 43
#define ORDINAL_STRING_LEN (RDF_NAMESPACE_URI_LEN + MAX_ASCII_INT_SIZE + 1)

#define GETOPT_STRING "bhf:t:u:"

#ifdef HAVE_GETOPT_LONG
static const struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"brief"       , 0, 0, 'b'},
  {"help"        , 0, 0, 'h'},
  {"from-format" , 1, 0, 'f'},
  {"to-format"   , 1, 0, 't'},
  {"base-uri"    , 1, 0, 'u'},
  {NULL          , 0, 0, 0}
};
#endif

#ifdef HAVE_GETOPT_LONG
#define HELP_TEXT(short, long, description) "  -" short ", --" long "  " description
#define HELP_ARG(short, long) "--" #long
#define HELP_PAD "\n                          "
#else
#define HELP_TEXT(short, long, description) "  -" short "  " description
#define HELP_ARG(short, long) "-" #short
#define HELP_PAD "\n      "
#endif

typedef struct rdfdiff_link_s {
  struct rdfdiff_link_s *next;
  raptor_statement *statement;
} rdfdiff_link;

typedef struct rdfdiff_blank_s {
  struct rdfdiff_blank_s *next;
  raptor_world *world;
  char *blank_id;
  raptor_statement *owner;
  rdfdiff_link *first;
  rdfdiff_link *last;
  int matched;
} rdfdiff_blank;

typedef struct {
  raptor_world *world;
  char *name;
  raptor_parser *parser;
  rdfdiff_link *first;
  rdfdiff_link *last;
  rdfdiff_blank *first_blank;
  rdfdiff_blank *last_blank;
  int statement_count;
  int error_count;
  int warning_count;
  int difference_count;
} rdfdiff_file;

static int brief = 0;
static char *program = NULL;
static const char * const title_format_string="Raptor RDF diff utility %s\n";
static int ignore_errors = 0;
static int ignore_warnings = 0;
static int emit_from_header = 1;
static int emit_to_header = 1;

static rdfdiff_file* from_file = NULL;
static rdfdiff_file*to_file = NULL;

static rdfdiff_file* rdfdiff_new_file(raptor_world* world, const unsigned char *name, const char *syntax);
static void rdfdiff_free_file(rdfdiff_file* file);

static rdfdiff_blank *rdfdiff_find_blank(rdfdiff_blank *first, char *blank_id);
static rdfdiff_blank *rdfdiff_new_blank(raptor_world *world, char *blank_id);
static void rdfdiff_free_blank(rdfdiff_blank *blank);

static int  rdfdiff_blank_equals(const rdfdiff_blank *b1, const rdfdiff_blank *b2,
                                 rdfdiff_file*b1_file, rdfdiff_file*b2_file);

static void rdfdiff_log_handler(void *data, raptor_log_message *message);

static void rdfdiff_collect_statements(void *user_data, raptor_statement *statement);

int main(int argc, char *argv[]);


/* Version of strcmp that can take NULL parameters. Assume that
 * Non-NULL strings are lexically greater than NULL strings 
 */
static int
safe_strcmp(const char *s1, const char *s2)
{
  if(s1 == NULL && s2 == NULL) {
    return 0;
  } else if(s1 == NULL && s2 != NULL) {
    return -1;
  } else if(s1 != NULL && s2 == NULL) {
    return 1;
  } else {
    return strcmp(s1, s2);
  }
  
}


static rdfdiff_file*
rdfdiff_new_file(raptor_world *world, const unsigned char *name, const char *syntax)
{
  rdfdiff_file* file = RAPTOR_CALLOC(rdfdiff_file*, 1, sizeof(*file));
  if(file) {
    file->world = world;
    file->name = RAPTOR_MALLOC(char*, strlen((const char*)name) + 1);
    strcpy((char*)file->name, (const char*)name);
    
    file->parser = raptor_new_parser(world, syntax);
    if(file->parser) {
      raptor_world_set_log_handler(world, file, rdfdiff_log_handler);
    } else {      
      fprintf(stderr, "%s: Failed to create raptor parser type %s for %s\n",
              program, syntax, name);
      rdfdiff_free_file(file);
      return(0);
    }


  }
  
  return file;  
}


static void
rdfdiff_free_file(rdfdiff_file* file) 
{
  rdfdiff_link *cur, *next;
  rdfdiff_blank *cur1, *next1;
  
  if(file->name)
    RAPTOR_FREE(char*, file->name);

  if(file->parser)
    raptor_free_parser(file->parser);
  
  for(cur = file->first; cur; cur = next) {
    next = cur->next;

    raptor_free_statement(cur->statement);
    RAPTOR_FREE(rdfdiff_link, cur);
  }

  for(cur1 = file->first_blank; cur1; cur1 = next1) {
    next1 = cur1->next;

    rdfdiff_free_blank(cur1);
  }
  
  RAPTOR_FREE(rdfdiff_file, file);  
  
}


static rdfdiff_blank *
rdfdiff_new_blank(raptor_world* world, char *blank_id) 
{
  rdfdiff_blank *blank = RAPTOR_CALLOC(rdfdiff_blank*, 1, sizeof(*blank));

  if(blank) {
    blank->world = world;
    blank->blank_id = RAPTOR_MALLOC(char*, strlen(blank_id) + 1);
    strcpy((char*)blank->blank_id, (const char*)blank_id);
  }
  
  return blank;
}


static void
rdfdiff_free_blank(rdfdiff_blank *blank) 
{
  rdfdiff_link *cur, *next;

  if(blank->blank_id)
    RAPTOR_FREE(char*, blank->blank_id);

  if(blank->owner)
    raptor_free_statement(blank->owner);
  
  for(cur = blank->first; cur; cur = next) {
    next = cur->next;

    raptor_free_statement(cur->statement);
    RAPTOR_FREE(rdfdiff_link, cur);
  }

  RAPTOR_FREE(rdfdiff_blank, blank);
  
}


static int
rdfdiff_statement_equals(raptor_world *world, const raptor_statement *s1, const raptor_statement *s2)
{
  int rv = 0;
  
  if(!s1 || !s2)
    return 0;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  fprintf(stderr, "(rdfdiff_statement_equals) Comparing ");
  raptor_statement_print_as_ntriples(s1, stderr);
  fprintf(stderr, " to ");
  raptor_statement_print_as_ntriples(s2, stderr);
#endif

  /* normal comparison */
  if(s1->subject->type != s2->subject->type) {
    rv = 0;
    goto done;
  }

  if(s1->subject->type == RAPTOR_TERM_TYPE_BLANK) {
    /* Here for completeness. Anonymous nodes are taken care of
     * elsewhere */
    /*if(strcmp((const char *)s1->subject, (const char *)s2->subject->value) != 0)
      return 0;*/
  } else {
    if(!raptor_uri_equals(s1->subject->value.uri,
                          s2->subject->value.uri)) {
      rv = 0;
      goto done;
    }
  }

  if(s1->predicate->type != s2->predicate->type) {
    rv = 0;
    goto done;
  }
  
  if(!raptor_uri_equals(s1->predicate->value.uri,
                        s2->predicate->value.uri)) {
    rv = 0;
    goto done;
  }
  
  if(s1->object->type != s2->object->type) {
    rv = 0;
    goto done;
  }
  
  if(s1->object->type == RAPTOR_TERM_TYPE_LITERAL) {
    int equal;
    
    equal= !safe_strcmp((char *)s1->object->value.literal.string,
                        (char *)s2->object->value.literal.string);

    if(equal) {
      if(s1->object->value.literal.language && s2->object->value.literal.language)
        equal = !strcmp((char *)s1->object->value.literal.language,
                        (char *)s2->object->value.literal.language);
      else if(s1->object->value.literal.language || s2->object->value.literal.language)
        equal = 0;
      else
        equal = 1;

      if(equal)
        equal = raptor_uri_equals(s1->object->value.literal.datatype, 
                                  s2->object->value.literal.datatype);
    }

    rv = equal;
    goto done;
  } else if(s1->object->type == RAPTOR_TERM_TYPE_BLANK) {
    /* Here for completeness. Anonymous nodes are taken care of
     * elsewhere */
    /* if(strcmp((const char *)s1->object, (const char *)s2->object->value) != 0)
       return 0; */
  } else {
    if(!raptor_uri_equals(s1->object->value.uri,
                          s2->object->value.uri)) {
      rv = 0;
      goto done;
    }
  }

  rv = 1;
  done:

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  fprintf(stderr, " : %s\n", (rv ? "equal" : "not equal"));
#endif
  return rv;
}


static int
rdfdiff_blank_equals(const rdfdiff_blank *b1, const rdfdiff_blank *b2,
                     rdfdiff_file *b1_file, rdfdiff_file *b2_file) 
{
  /* first compare "owners". Owners are subject/predicate or arcs
   * in. */
  int equal = 0;

  if(b1->owner == NULL && b2->owner == NULL) {
    /* Both are "top-level" anonymous objects. I.E. Neither is the
     * object of a statement. Fall through and compare based on their
     * contents. */
    equal = 1;
  } else if(b1->owner == NULL || b2->owner == NULL) {
    equal = 0;
  } else if(b1->owner->subject->type != RAPTOR_TERM_TYPE_BLANK &&
            b2->owner->subject->type != RAPTOR_TERM_TYPE_BLANK) {
    /* Neither are anonymous. Normal comparison. This will return
     * false if both the subject and the predicates don't match. We
     * know the objects are blank nodes. */
    equal = rdfdiff_statement_equals(b1->world, b1->owner, b2->owner);
    
  } else if(b1->owner->subject->type == RAPTOR_TERM_TYPE_BLANK &&
            b2->owner->subject->type == RAPTOR_TERM_TYPE_BLANK) {
    rdfdiff_blank *p1;
    rdfdiff_blank *p2;

    /* Both are anonymous.  Need further testing. Check that the
     * containing anononymous nodes are eaual. */
#if 0
    fprintf(stderr, "b1->owner: ");    
    raptor_statement_print_as_ntriples(b1->owner, stderr);
    fprintf(stderr, "\n");

    fprintf(stderr, "b2->owner: ");    
    raptor_statement_print_as_ntriples(b2->owner, stderr);
    fprintf(stderr, "\n");
#endif    
    p1 = rdfdiff_find_blank(b1_file->first_blank, 
                            (char *)b1->owner->subject->value.blank.string);
    p2 = rdfdiff_find_blank(b2_file->first_blank,
                            (char *)b2->owner->subject->value.blank.string);
    equal = rdfdiff_blank_equals(p1, p2, b1_file, b2_file);
  } else {
    equal = 0;
  }

  /* Now compare the contents. This accounts for the case where a
   * subject has several properties (of the same predicate value) with
   * different blank nodes as values. */
  if(equal) {
    rdfdiff_link *s1 = b1->first;
    while(s1) {

      rdfdiff_link *s2 = b2->first;
      while(s2) {

        if(rdfdiff_statement_equals(b1->world, s1->statement, s2->statement))
          break;
          
        s2 = s2->next;

      }

      if(s2 == 0) {
        equal = 0;
        break;
      }

      s1 = s1->next;

    }
    
  }
  
  return equal;
}


static void
rdfdiff_log_handler(void *data, raptor_log_message *message)
{
  rdfdiff_file* file = (rdfdiff_file*)data;
  
   switch(message->level) {
    case RAPTOR_LOG_LEVEL_FATAL:
    case RAPTOR_LOG_LEVEL_ERROR:
      if(!ignore_errors) {
        fprintf(stderr, "%s: Error - ", program);
        raptor_locator_print(message->locator, stderr);
        fprintf(stderr, " - %s\n", message->text);
        
        raptor_parser_parse_abort(file->parser);
      }
      
      file->error_count++;
      break;
      
    case RAPTOR_LOG_LEVEL_WARN:
      if(!ignore_warnings) {
        fprintf(stderr, "%s: Warning - ", program);
        raptor_locator_print(message->locator, stderr);
        fprintf(stderr, " - %s\n", message->text);
      }
      
      file->warning_count++;
      break;
      
    case RAPTOR_LOG_LEVEL_NONE:
    case RAPTOR_LOG_LEVEL_TRACE:
    case RAPTOR_LOG_LEVEL_DEBUG:
    case RAPTOR_LOG_LEVEL_INFO:

      fprintf(stderr, "%s: Unexpected %s message - ", program,
              raptor_log_level_get_label(message->level));
      raptor_locator_print(message->locator, stderr);
      fprintf(stderr, " - %s\n", message->text);
      break;
  }
  
}



static rdfdiff_blank *
rdfdiff_find_blank(rdfdiff_blank *first, char *blank_id) 
{
  rdfdiff_blank *rv_blank = 0;
  rdfdiff_blank *cur = first;

  while(cur) {

    if(strcmp(cur->blank_id, blank_id) == 0) {
      rv_blank = cur;
      break;
    }

    cur = cur->next;
    
  }
  
  return rv_blank;
  
}


static rdfdiff_blank *
rdfdiff_lookup_blank(rdfdiff_file* file, char *blank_id) 
{
  rdfdiff_blank *rv_blank = rdfdiff_find_blank(file->first_blank, blank_id);
  
  if(!rv_blank) {
    rv_blank = rdfdiff_new_blank(file->world, blank_id);
    if(rv_blank) {

      if(!file->first_blank) {
        file->first_blank = rv_blank;
        file->last_blank = rv_blank;
      } else {
        file->last_blank->next = rv_blank;
        file->last_blank = rv_blank;
      }
    }    
  }

  return rv_blank;
  
}


static int
rdfdiff_add_blank_statement(rdfdiff_file* file,
                            raptor_statement *statement)
{
  rdfdiff_blank *blank;
  rdfdiff_link *dlink;

  blank = rdfdiff_lookup_blank(file, (char *)statement->subject->value.blank.string);
  if(!blank)
    goto failed;

  dlink = RAPTOR_MALLOC(rdfdiff_link*, sizeof(*dlink));
  if(!dlink)
    goto failed;
  
  dlink->statement = raptor_statement_copy(statement);
  if(!dlink->statement) {
    RAPTOR_FREE(rdfdiff_link, dlink);
    goto failed;
  }
  
  dlink->next = NULL;
  if(!blank->first) {
    blank->first = dlink;
    blank->last = dlink;
  } else {
    blank->last->next = dlink;
    blank->last = dlink;
  }
  
  return 0;

failed:
  fprintf(stderr, "%s: Internal Error\n", program);
  return 1;
}


static int
rdfdiff_add_blank_statement_owner(rdfdiff_file* file,
                                  raptor_statement *statement)
{
  rdfdiff_blank *blank;

  blank = rdfdiff_lookup_blank(file,
                               (char*)statement->object->value.blank.string);
  if(!blank)
    goto failed;

  if(blank->owner)
    raptor_free_statement(blank->owner);

  blank->owner = raptor_statement_copy(statement);
  if(!blank->owner)
    goto failed;

  return 0;

failed:
  fprintf(stderr, "%s: Internal Error\n", program);
  return 1;
}


static int
rdfdiff_add_statement(rdfdiff_file* file, raptor_statement *statement) 
{
  int rv = 0;
  
  rdfdiff_link *dlink = RAPTOR_MALLOC(rdfdiff_link*, sizeof(*dlink));

  if(dlink) {

    dlink->statement = raptor_statement_copy(statement);

    if(dlink->statement) {
      
      dlink->next = NULL;

      if(!file->first) {
        file->first = dlink;
        file->last = dlink;
      } else {
        file->last->next = dlink;
        file->last = dlink;
      }
      
    } else {
      RAPTOR_FREE(rdfdiff_link, dlink);
      rv = 1;
    }
    
  } else {
    rv = 1;
  }

  if(rv != 0)
    fprintf(stderr, "%s: Internal Error\n", program);

  return rv;

}


static rdfdiff_link*
rdfdiff_statement_find(rdfdiff_file* file, const raptor_statement *statement,
                       rdfdiff_link** prev_p)
{
  rdfdiff_link* prev = NULL;
  rdfdiff_link* cur = file->first;
  
  while(cur) {
    if(rdfdiff_statement_equals(file->world, cur->statement, statement)) {
      if(prev_p)
        *prev_p=prev;
      return cur;
    }
    prev = cur;
    cur = cur->next;
  }

  return NULL;
}


static int
rdfdiff_statement_exists(rdfdiff_file* file, const raptor_statement *statement)
{
  rdfdiff_link* node;
  rdfdiff_link* prev = NULL;
  node = rdfdiff_statement_find(file, statement, &prev);
  return (node != NULL);
}


/*
 * rdfdiff_collect_statements - Called when parsing "from" file to build a
 * list of statements for comparison with those in the "to" file.
 */
static void
rdfdiff_collect_statements(void *user_data, raptor_statement *statement)
{
  int rv = 0;
  rdfdiff_file* file = (rdfdiff_file*)user_data;

  if(rdfdiff_statement_exists(file, statement))
    return;
  
  file->statement_count++;

  if(statement->subject->type == RAPTOR_TERM_TYPE_BLANK ||
     statement->object->type  == RAPTOR_TERM_TYPE_BLANK) {

    if(statement->subject->type == RAPTOR_TERM_TYPE_BLANK)
      rv = rdfdiff_add_blank_statement(file, statement);

    if(rv == 0 && statement->object->type == RAPTOR_TERM_TYPE_BLANK)
      rv = rdfdiff_add_blank_statement_owner(file, statement);

  } else {
    rv = rdfdiff_add_statement(file, statement);
  }

  if(rv != 0) {
    raptor_parser_parse_abort(file->parser);
  }
  
}



int
main(int argc, char *argv[]) 
{
  raptor_world *world = NULL;
  unsigned char *from_string = NULL;
  unsigned char *to_string = NULL;
  raptor_uri *from_uri = NULL;
  raptor_uri *to_uri = NULL;
  raptor_uri *base_uri = NULL;
  const char *from_syntax = "rdfxml";
  const char *to_syntax = "rdfxml";
  int free_from_string = 0;
  int free_to_string = 0;
  int usage = 0;
  int help = 0;
  char *p;
  int rv = 0;
  rdfdiff_blank *b1;
  rdfdiff_link *cur;
  
  program = argv[0];
  if((p = strrchr(program, '/')))
    program = p+1;
  else if((p = strrchr(program, '\\')))
    program = p+1;
  argv[0] = program;

  world = raptor_new_world();
  if(!world)
    exit(1);
  rv = raptor_world_open(world);
  if(rv)
    exit(1);

  while(!usage && !help)
  {
    int c;
#ifdef HAVE_GETOPT_LONG
    int option_index = 0;

    c = getopt_long (argc, argv, GETOPT_STRING, long_options, &option_index);
#else
    c = getopt (argc, argv, GETOPT_STRING);
#endif
    if(c == -1)
      break;

    switch (c) {
      case 0:
      case '?': /* getopt() - unknown option */
        usage = 1;
        break;
        
      case 'b':
        brief = 1;
        break;

      case 'h':
        help = 1;
        break;

      case 'f':
        if(optarg)
          from_syntax = optarg;
        break;

      case 't':
        if(optarg)
          to_syntax = optarg;
        break;

      case 'u':
        if(optarg)
          base_uri = raptor_new_uri(world, (const unsigned char*)optarg);
        break;

    }
    
  }

  if(optind != argc-2)
    help = 1;
  
  if(usage) {
    if(usage > 1) {
      fprintf(stderr, title_format_string, raptor_version_string);
      fputs(raptor_short_copyright_string, stderr);
      fputc('\n', stderr);
    }
    fprintf(stderr, "Try `%s " HELP_ARG(h, help) "' for more information.\n",
                    program);
    rv = 1;
    goto exit;
  }

  if(help) {
    printf("Usage: %s [OPTIONS] <from URI> <to URI>\n", program);
    printf(title_format_string, raptor_version_string);
    puts(raptor_short_copyright_string);
    puts("Find differences between two RDF files.");
    puts("\nOPTIONS:");
    puts(HELP_TEXT("h", "help                      ", "Print this help, then exit"));
    puts(HELP_TEXT("b", "brief                     ", "Report only whether files differ"));
    puts(HELP_TEXT("u BASE-URI", "base-uri BASE-URI  ", "Set the base URI for the files"));
    puts(HELP_TEXT("f FORMAT",   "from-format FORMAT ", "Format of <from URI> (default is rdfxml)"));
    puts(HELP_TEXT("t FORMAT",   "to-format FORMAT   ", "Format of <to URI> (default is rdfxml)"));
    rv = 1;
    goto exit;
  }

  from_string = (unsigned char *)argv[optind++];
  to_string = (unsigned char *)argv[optind];
  
  if(!access((const char *)from_string, R_OK)) {
    char *filename = (char *)from_string;
    from_string = raptor_uri_filename_to_uri_string(filename);
    if(!from_string) {
      fprintf(stderr, "%s: Failed to create URI for file %s.\n", program, filename);
      rv = 2;
      goto exit;
    }
    free_from_string = 1;
  }
  
  if(!access((const char *)to_string, R_OK)) {
    char *filename = (char *)to_string;
    to_string = raptor_uri_filename_to_uri_string(filename);
    if(!to_string) {
      fprintf(stderr, "%s: Failed to create URI for file %s.\n", program, filename);
      rv = 2;
      goto exit;
    }
    free_to_string = 1;
  }
  
  if(from_string) {
    from_uri = raptor_new_uri(world, from_string);
    if(!from_uri) {
      fprintf(stderr, "%s: Failed to create URI for %s\n", program, from_string);
      rv = 2;
      goto exit;
    }
  }
  
  if(to_string) {
    to_uri = raptor_new_uri(world, to_string);
    if(!to_uri) {
      fprintf(stderr, "%s: Failed to create URI for %s\n", program, from_string);
      rv = 2;
      goto exit;
    }
  }

  /* create and init "from" data structures */
  from_file = rdfdiff_new_file(world, from_string, from_syntax);
  if(!from_file) {
    rv = 2;
    goto exit;
  }
  
  /* create and init "to" data structures */
  to_file = rdfdiff_new_file(world, to_string, to_syntax);
  if(!to_file) {
    rv = 2;
    goto exit;
  }

  /* parse the files */
  raptor_parser_set_statement_handler(from_file->parser, from_file,
                               rdfdiff_collect_statements);
  
  if(raptor_parser_parse_uri(from_file->parser, from_uri, base_uri)) {
    fprintf(stderr, "%s: Failed to parse URI %s as %s content\n", program, 
            from_string, from_syntax);
    rv = 1;
    goto exit;
  } else {

    /* Note intentional from_uri as base_uri */
    raptor_parser_set_statement_handler(to_file->parser, to_file,
                                 rdfdiff_collect_statements);
    if(raptor_parser_parse_uri(to_file->parser, to_uri, base_uri ? base_uri: from_uri)) {
      fprintf(stderr, "%s: Failed to parse URI %s as %s content\n", program, 
              to_string, to_syntax);
      rv = 1;
      goto exit;
    }
  }


  /* Compare triples with no blank nodes */
  cur = to_file->first;
  while(cur) {
    rdfdiff_link* node;
    rdfdiff_link* prev;
    node = rdfdiff_statement_find(from_file, cur->statement, &prev);
    if(node) {
      /* exists in from file - remove it from the list */
      if(from_file->first == node) {
        from_file->first = node->next;
      } else {
        prev->next = node->next;
      }
      raptor_free_statement(node->statement);
      RAPTOR_FREE(rdfdiff_link, node);
    } else {
      if(!brief) {
        if(emit_from_header) {
          fprintf(stderr, "Statements in %s but not in %s\n",
                  to_file->name, from_file->name);
          emit_from_header = 0;
        }
        
        fprintf(stderr, "<    ");
        raptor_statement_print_as_ntriples(cur->statement, stderr);
        fprintf(stderr, "\n");
      }
      
      to_file->difference_count++;
    }
    cur = cur->next;
  }

  
  /* Now compare the blank nodes */
  b1 = to_file->first_blank;
  while(b1) {

    rdfdiff_blank *b2 = from_file->first_blank;

    while(b2) {

      if(!b2->matched && rdfdiff_blank_equals(b1, b2, to_file, from_file)) {
        b1->matched = 1;
        b2->matched = 1;
        break;
      }
      
      b2 = b2->next;
      
    }

    if(b2 == 0) {
      if(!brief) {        
#if 0
        fprintf(stderr, "<    ");
        raptor_statement_print_as_ntriples(b1->owner, stderr);
        fprintf(stderr, "\n");
#else
        if(emit_from_header) {
          fprintf(stderr, "Statements in %s but not in %s\n",  to_file->name, from_file->name);
          emit_from_header = 0;
        }

        fprintf(stderr, "<    anonymous node %s\n", b1->blank_id);
#endif        
      }
      
      to_file->difference_count++;
    }
    
    b1 = b1->next;

  }
  
  if(from_file->first) {
    /* The entrys left in from_file have not been found in to_file. */
    if(!brief) {

      if(emit_to_header) {
        fprintf(stderr, "Statements in %s but not in %s\n",  from_file->name,
                to_file->name);
        emit_to_header = 0;
      }
      
      cur = from_file->first;
      while(cur) {
        if(!brief) {
          fprintf(stderr, ">    ");
          raptor_statement_print_as_ntriples(cur->statement, stderr);
          fprintf(stderr, "\n");
        }
      
        cur = cur->next;
        from_file->difference_count++;
      }
    }

  }

  if(from_file->first_blank) {
    rdfdiff_blank *blank = from_file->first_blank;
    while(blank) {

      if(!blank->matched) {
        if(!brief) {
#if 0          
          fprintf(stderr, ">    ");
          raptor_statement_print_as_ntriples(blank->owner, stderr);
          fprintf(stderr, "\n");
#else
          if(emit_to_header) {
            fprintf(stderr, "Statements in %s but not in %s\n",  from_file->name, to_file->name);
            emit_to_header = 0;
          }
          fprintf(stderr, ">    anonymous node %s\n", blank->blank_id);
#endif          
        }
        from_file->difference_count++;
      }
    
      blank = blank->next;
      
    }

  }
  
  if(!(from_file->difference_count == 0 &&
        to_file->difference_count == 0)) {

    if(brief)
      fprintf(stderr, "Files differ\n");

    rv = 1;
  }

exit:

  if(base_uri)
    raptor_free_uri(base_uri);
  
  if(from_file)
    rdfdiff_free_file(from_file);

  if(to_file)
    rdfdiff_free_file(to_file);
  
  if(free_from_string)
    raptor_free_memory(from_string);
    
  if(free_to_string)
    raptor_free_memory(to_string);

  if(from_uri)
    raptor_free_uri(from_uri);

  if(to_uri)
    raptor_free_uri(to_uri);

  raptor_free_world(world);

  return rv;
  
}

