/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * ntriples_parse.c - Raptor N-Triples Parser implementation
 *
 * N-Triples
 * http://www.w3.org/TR/rdf-testcases/#ntriples
 *
 * Copyright (C) 2001-2010, David Beckett http://www.dajobe.org/
 * Copyright (C) 2001-2005, University of Bristol, UK http://www.bristol.ac.uk/
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

/* Set RAPTOR_DEBUG to > 1 to get lots of buffer related debugging */
/*
#undef RAPTOR_DEBUG
#define RAPTOR_DEBUG 2
*/


/* Prototypes for local functions */
static void raptor_ntriples_generate_statement(raptor_parser* parser, raptor_term* subject_term, raptor_term* predicate_term, raptor_term* object_term, raptor_term* graph_term);

/*
 * NTriples parser object
 */
struct raptor_ntriples_parser_context_s {
  /* current line */
  unsigned char *line;
  /* current line length */
  size_t line_length;
  /* current char in line buffer */
  size_t offset;

  char last_char;
  
  /* static statement for use in passing to user code */
  raptor_statement statement;

  /* Non-0 if N-Quads */
  int is_nquads;

  int literal_graph_warning;
};


typedef struct raptor_ntriples_parser_context_s raptor_ntriples_parser_context;



/**
 * raptor_ntriples_parse_init:
 *
 * Initialise the Raptor NTriples parser.
 *
 * Return value: non 0 on failure
 **/

static int
raptor_ntriples_parse_init(raptor_parser* rdf_parser, const char *name)
{
  raptor_ntriples_parser_context *ntriples_parser;
  ntriples_parser = (raptor_ntriples_parser_context*)rdf_parser->context;

  raptor_statement_init(&ntriples_parser->statement, rdf_parser->world);

  if(!strcmp(name, "nquads"))
    ntriples_parser->is_nquads = 1;
  
  return 0;
}


/* PUBLIC FUNCTIONS */


/*
 * raptor_ntriples_parse_terminate - Free the Raptor NTriples parser
 * @rdf_parser: parser object
 * 
 **/
static void
raptor_ntriples_parse_terminate(raptor_parser* rdf_parser)
{
  raptor_ntriples_parser_context *ntriples_parser;
  ntriples_parser = (raptor_ntriples_parser_context*)rdf_parser->context;
  if(ntriples_parser->line_length)
    RAPTOR_FREE(cdata, ntriples_parser->line);
}


static void
raptor_ntriples_generate_statement(raptor_parser* parser, 
                                   raptor_term *subject,
                                   raptor_term *predicate,
                                   raptor_term *object,
                                   raptor_term *graph)
{
  /* raptor_ntriples_parser_context *ntriples_parser = (raptor_ntriples_parser_context*)parser->context; */
  raptor_statement *statement = &parser->statement;

  if(!parser->emitted_default_graph) {
    raptor_parser_start_graph(parser, NULL, 0);
    parser->emitted_default_graph++;
  }

  statement->subject = subject;
  statement->predicate = predicate;
  statement->object = object;
  statement->graph = graph;

  /* Do not generate a partial triple - but do clean up */
  if(!subject || !predicate || !object)
    goto cleanup;

  /* If there is no statement handler - there is nothing else to do */
  if(!parser->statement_handler)
    goto cleanup;

  /* Generate the statement */
  (*parser->statement_handler)(parser->user_data, statement);

  cleanup:
  raptor_free_statement(statement);
}



#define MAX_NTRIPLES_TERMS 4

static int
raptor_ntriples_parse_line(raptor_parser* rdf_parser,
                           unsigned char *buffer, size_t len,
                           int max_terms)
{
  raptor_ntriples_parser_context *ntriples_parser = (raptor_ntriples_parser_context*)rdf_parser->context;
  int i;
  unsigned char *p;
  raptor_term* terms[MAX_NTRIPLES_TERMS+1] = {NULL, NULL, NULL, NULL, NULL};
  int rc = 0;
  
  /* ASSERTION:
   * p always points to first char we are considering
   * p[len-1] always points to last char
   */
  
  /* Handle empty  lines */
  if(!len)
    return 0;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG3("handling line '%s' (%d bytes)\n", buffer, (unsigned int)len);
#endif
  
  p = buffer;

  while(len > 0 && isspace((int)*p)) {
    p++;
    rdf_parser->locator.column++;
    rdf_parser->locator.byte++;
    len--;
  }

  /* Handle empty - all whitespace lines */
  if(!len)
    return 0;
  
  /* Handle comment lines */
  if(*p == '#')
    return 0;
  
  /* Remove trailing spaces */
  while(len > 0 && isspace((int)p[len-1])) {
    p[len-1] = '\0';
    len--;
  }

  /* can't be empty now - that would have been caught above */
  
  /* Must be triple/quad */

  for(i = 0; i < MAX_NTRIPLES_TERMS + 1; i++) {
    if(!len) {
      if(ntriples_parser->is_nquads) {
        /* context is optional in nquads */
        if(i == 3 || i ==4)
          break;
      } else {
        if(i == 3)
          break;
      }
      raptor_parser_error(rdf_parser, "Unexpected end of line");
      goto cleanup;
    }
    

    if(i == 3) {
      /* graph term (3): blank node or <URI> */
      if(*p != '<' && *p != '_') {
        raptor_parser_error(rdf_parser, "Saw '%c', expected Graph term <URIref>, _:bnodeID", *p);
        goto cleanup;
      }
    } else if(i == 2) {
      /* object term (2): expect either <URI> or _:name or literal */
      if(*p != '<' && *p != '_' && *p != '"') {
        raptor_parser_error(rdf_parser, "Saw '%c', expected object term <URIref>, _:bnodeID or \"literal\"", *p);
        goto cleanup;
      }
    } else if(i == 1) {
      /* predicate term (1): expect URI only */
      if(*p != '<') {
        raptor_parser_error(rdf_parser, "Saw '%c', expected predict term <URIref>", *p);
        goto cleanup;
      }
    } else {
      /* subject (0) or graph (3) terms: expect <URI> or _:name */
      if(*p != '<' && *p != '_') {
        raptor_parser_error(rdf_parser, "Saw '%c', expected subject term <URIref> or _:bnodeID", *p);
        goto cleanup;
      }
    }


    rc = raptor_ntriples_parse_term(rdf_parser->world, &rdf_parser->locator,
                                    p, &len, &terms[i], 0);
    
    if(!rc) {
      rc = 1;
      goto cleanup;
    }
    p += rc;
    rc = 0;

    if(terms[i] && terms[i]->type == RAPTOR_TERM_TYPE_URI) {
      unsigned const char* uri_string;

      /* Check for absolute URI */
      uri_string = raptor_uri_as_string(terms[i]->value.uri);
      if(!raptor_uri_uri_string_is_absolute(uri_string)) {
        raptor_parser_error(rdf_parser, "URI %s is not absolute", uri_string);
        goto cleanup;
      }
    }

    /* Skip whitespace after terms */
    while(len > 0 && isspace((int)*p)) {
      p++;
      len--;
      rdf_parser->locator.column++;
      rdf_parser->locator.byte++;
    }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    if(terms[i]) {
      unsigned char* c = raptor_term_to_string(terms[i]);
      fprintf(stderr, "item %d: term '%s' type %d\n",
              i, c, terms[i]->type);
      raptor_free_memory(c);
    } else
      fprintf(stderr, "item %d: NULL term\n", i);
#endif

    /* Look for terminating '.' after 3rd (ntriples) or 3rd/4th (nquads) term */
    if(i == (ntriples_parser->is_nquads ? 4 : 3) && *p != '.') {
      raptor_parser_error(rdf_parser, "Missing terminating \".\"");
      return 0;
    }

    /* Still may be optional so check again */
    if(*p == '.') {
      p++;
      len--;
      rdf_parser->locator.column++;
      rdf_parser->locator.byte++;

      /* Skip whitespace after '.' */
      while(len > 0 && isspace((int)*p)) {
        p++;
        len--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;
      }

      /* Only a comment is allowed here */
      if(*p && *p != '#') {
        raptor_parser_error(rdf_parser, "Junk after terminating \".\"");
        return 0;
      }

      p += len; len = 0;
    }
  }


  if(ntriples_parser->is_nquads) {
    /* Check N-Quads has 3 or 4 terms */
    if(terms[4]) {
      raptor_free_term(terms[4]);
      terms[4] = NULL;
      raptor_parser_error(rdf_parser, "N-Quads only allows 3 or 4 terms");
      goto cleanup;
    }
  } else {
    /* Check N-Triples has only 3 terms */
    if(terms[3] || terms[4]) {
      if(terms[4]) {
        raptor_free_term(terms[4]);
        terms[4] = NULL;
      }
      if(terms[3]) {
        raptor_free_term(terms[3]);
        terms[3] = NULL;
      }
      raptor_parser_error(rdf_parser, "N-Triples only allows 3 terms");
      goto cleanup;
    }
  }

  if(terms[3] && terms[3]->type == RAPTOR_TERM_TYPE_LITERAL) {
    if(!ntriples_parser->literal_graph_warning++)
      raptor_parser_warning(rdf_parser, "Ignoring N-Quad literal contexts");

    raptor_free_term(terms[3]);
    terms[3] = NULL;
  }

  raptor_ntriples_generate_statement(rdf_parser, 
                                     terms[0], terms[1], terms[2], terms[3]);

  rdf_parser->locator.byte += RAPTOR_BAD_CAST(int, len);

 cleanup:

  return rc;
}


static int
raptor_ntriples_parse_chunk(raptor_parser* rdf_parser, 
                            const unsigned char *s, size_t len,
                            int is_end)
{
  unsigned char *buffer;
  unsigned char *ptr;
  unsigned char *start;
  raptor_ntriples_parser_context *ntriples_parser = (raptor_ntriples_parser_context*)rdf_parser->context;
  int max_terms = ntriples_parser->is_nquads ? 4 : 3;
  unsigned char* end_ptr;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2("adding %d bytes to buffer\n", (unsigned int)len);
#endif

  if(len) {
    buffer = RAPTOR_MALLOC(unsigned char*, ntriples_parser->line_length + len + 1);
    if(!buffer) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      return 1;
    }

    if(ntriples_parser->line_length) {
      memcpy(buffer, ntriples_parser->line, ntriples_parser->line_length);
      RAPTOR_FREE(char*, ntriples_parser->line);
    }

    ntriples_parser->line = buffer;

    /* move pointer to end of cdata buffer */
    ptr = buffer + ntriples_parser->line_length;

    /* adjust stored length */
    ntriples_parser->line_length += len;

    /* now write new stuff at end of cdata buffer */
    memcpy(ptr, s, len);
    ptr += len;
    *ptr = '\0';
  } else
    buffer = ntriples_parser->line;


#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2("buffer now %ld bytes\n", ntriples_parser->line_length);
#endif

  if(!ntriples_parser->line_length)
    return 0;

  ptr = buffer + ntriples_parser->offset;
  end_ptr = buffer + ntriples_parser->line_length;
  while((start = ptr) < end_ptr) {
    unsigned char *line_start = ptr;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG3("line buffer now '%s' (offset %ld)\n", ptr, ptr-(buffer+ntriples_parser->offset));
#endif

    /* skip \n when just seen \r - i.e. \r\n or CR LF */
    if(ntriples_parser->last_char == '\r' && *ptr == '\n') {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
      RAPTOR_DEBUG1("skipping a \\n\n");
#endif
      ptr++;
      rdf_parser->locator.byte++;
      rdf_parser->locator.column = 0;
      start = line_start = ptr;
    }

    if(1) {
      int quote = '\0';
      int in_uri = '\0';
      int bq = 0;
      while(ptr < end_ptr) {
        if(!bq) {
          if(*ptr == '\\') {
            bq = 1;
            ptr++;
            continue;
          }

          if(*ptr == '<')
            in_uri = 1;
          else if (in_uri && *ptr == '>')
            in_uri = 0;

          if(!quote) {
            if((!in_uri && *ptr == '\'') || *ptr == '"')
              quote = *ptr;
            if(*ptr == '\n' || *ptr == '\r')
              break;
          } else {
            if(*ptr == quote)
              quote = 0;
          }
        }
        ptr++;
        bq = 0;
      }
    }

    if(ptr == end_ptr) {
      if(!is_end)
        /* middle of line */
        break;
    } else {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
      RAPTOR_DEBUG3("found newline \\x%02x at offset %ld\n", *ptr,
                    ptr-line_start);
#endif
      ntriples_parser->last_char = *ptr;
    }
    
    len = ptr - line_start;
    rdf_parser->locator.column = 0;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    RAPTOR_DEBUG2("line (%ld) : >>>", len);
    fwrite(line_start, sizeof(char), len, stderr);
    fputs("<<<\n", stderr);
#endif
    *ptr = '\0';
    if(raptor_ntriples_parse_line(rdf_parser, line_start, len, max_terms))
      return 1;
    
    rdf_parser->locator.line++;

    /* go past newline */
    if(ptr < end_ptr) {
      ptr++;
      rdf_parser->locator.byte++;
    }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    /* Do not peek if too far */
    if(RAPTOR_BAD_CAST(size_t, ptr - buffer) < ntriples_parser->line_length)
      RAPTOR_DEBUG2("next char is \\x%02x\n", *ptr);
    else
      RAPTOR_DEBUG1("next char unknown - end of buffer\n");
#endif
  }

  ntriples_parser->offset = start - buffer;

  len = ntriples_parser->line_length - ntriples_parser->offset;
    
  if(len && ntriples_parser->line_length != len) {
    /* collapse buffer */

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    RAPTOR_DEBUG3("collapsing buffer from %ld to %ld bytes\n", ntriples_parser->line_length, len);
#endif
    buffer = RAPTOR_MALLOC(unsigned char*, len + 1);
    if(!buffer) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      return 1;
    }

    memcpy(buffer, 
           ntriples_parser->line + ntriples_parser->line_length - len,
           len);
    buffer[len] = '\0';

    RAPTOR_FREE(char*, ntriples_parser->line);

    ntriples_parser->line = buffer;
    ntriples_parser->line_length -= ntriples_parser->offset;
    ntriples_parser->offset = 0;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    RAPTOR_DEBUG3("buffer now '%s' (%ld bytes)\n", ntriples_parser->line, ntriples_parser->line_length);
#endif    
  }

  /* exit now, no more input */
  if(is_end) {
    if(ntriples_parser->offset != ntriples_parser->line_length) {
       raptor_parser_error(rdf_parser, "Junk at end of input.");
       return 1;
    }
    
    if(rdf_parser->emitted_default_graph) {
      raptor_parser_end_graph(rdf_parser, NULL, 0);
      rdf_parser->emitted_default_graph--;
    }

    return 0;
  }
    
  return 0;
}


static int
raptor_ntriples_parse_start(raptor_parser* rdf_parser) 
{
  raptor_locator *locator = &rdf_parser->locator;
  raptor_ntriples_parser_context *ntriples_parser = (raptor_ntriples_parser_context*)rdf_parser->context;

  locator->line = 1;
  locator->column = 0;
  locator->byte = 0;

  ntriples_parser->last_char = '\0';

  return 0;
}


#if defined RAPTOR_PARSER_NTRIPLES || defined RAPTOR_PARSER_NQUADS
static int
raptor_ntriples_parse_recognise_syntax(raptor_parser_factory* factory, 
                                       const unsigned char *buffer, size_t len,
                                       const unsigned char *identifier, 
                                       const unsigned char *suffix, 
                                       const char *mime_type)
{
  int score = 0;
  
  if(suffix) {
    if(!strcmp((const char*)suffix, "nt"))
      score = 8;

    /* Explicitly refuse to do anything with Turtle or N3 named content */
    if(!strcmp((const char*)suffix, "ttl") ||
       !strcmp((const char*)suffix, "n3")) {
      return 0;
    }
  }
  
  if(mime_type) {
    if(strstr((const char*)mime_type, "ntriples"))
      score += 6;
  }
  
  if(buffer && len) {
    int has_ntriples_3;

    /* recognizing N-Triples is tricky but rely that it is line based
     * and that all URLs are absolute, and there are a lot of http:
     * URLs
     */
#define  HAS_AT_PREFIX (raptor_memstr((const char*)buffer, len, "@prefix ") != NULL)

#define  HAS_NTRIPLES_START_1_LEN 8
#define  HAS_NTRIPLES_START_1 (!memcmp((const char*)buffer, "<http://", HAS_NTRIPLES_START_1_LEN))
#define  HAS_NTRIPLES_START_2_LEN 2
#define  HAS_NTRIPLES_START_2 (!memcmp((const char*)buffer, "_:", HAS_NTRIPLES_START_2_LEN))

#define  HAS_NTRIPLES_1 (raptor_memstr((const char*)buffer, len, "\n<http://") != NULL)
#define  HAS_NTRIPLES_2 (raptor_memstr((const char*)buffer, len, "\r<http://") != NULL)
#define  HAS_NTRIPLES_3 (raptor_memstr((const char*)buffer, len, "> <http://") != NULL)
#define  HAS_NTRIPLES_4 (raptor_memstr((const char*)buffer, len, "> <") != NULL)
#define  HAS_NTRIPLES_5 (raptor_memstr((const char*)buffer, len, "> \"") != NULL)
    if(HAS_AT_PREFIX)
      /* Turtle */
      return 0;

    has_ntriples_3 = HAS_NTRIPLES_3;

    /* Bonus if the first bytes look N-Triples-like */
    if(len >= HAS_NTRIPLES_START_1_LEN && HAS_NTRIPLES_START_1)
      score++;
    if(len >= HAS_NTRIPLES_START_2_LEN && HAS_NTRIPLES_START_2)
      score++;

    if(HAS_NTRIPLES_1 || HAS_NTRIPLES_2) {
      /* N-Triples file with newlines and HTTP subjects */
      score += 6;
      if(has_ntriples_3)
        score++;
    } else if(has_ntriples_3) {
      /* an HTTP URL predicate or object but no HTTP subject */
      score += 3;
    } else if(HAS_NTRIPLES_4) {
      /* non HTTP urls - weak check */
      score += 2;
      if(HAS_NTRIPLES_5)
        /* bonus for a literal object */
        score++;
    }
  }

  return score;
}


static const char* const ntriples_names[2] = { "ntriples", NULL };

static const char* const ntriples_uri_strings[3] = {
  "http://www.w3.org/ns/formats/N-Triples",
  "http://www.w3.org/TR/rdf-testcases/#ntriples",
  NULL
};
  
#define NTRIPLES_TYPES_COUNT 2
static const raptor_type_q ntriples_types[NTRIPLES_TYPES_COUNT + 1] = {
  { "application/n-triples", 21, 10},
  { "text/plain", 10, 1},
  { NULL, 0, 0}
};

static int
raptor_ntriples_parser_register_factory(raptor_parser_factory *factory) 
{
  int rc = 0;

  factory->desc.names = ntriples_names;

  factory->desc.mime_types = ntriples_types;

  factory->desc.label = "N-Triples";
  factory->desc.uri_strings = ntriples_uri_strings;

  factory->desc.flags = 0;
  
  factory->context_length     = sizeof(raptor_ntriples_parser_context);

  factory->init      = raptor_ntriples_parse_init;
  factory->terminate = raptor_ntriples_parse_terminate;
  factory->start     = raptor_ntriples_parse_start;
  factory->chunk     = raptor_ntriples_parse_chunk;
  factory->recognise_syntax = raptor_ntriples_parse_recognise_syntax;

  return rc;
}


int
raptor_init_parser_ntriples(raptor_world* world)
{
  return !raptor_world_register_parser_factory(world,
                                               &raptor_ntriples_parser_register_factory);
}

#endif


#ifdef RAPTOR_PARSER_NQUADS
static int
raptor_nquads_parse_recognise_syntax(raptor_parser_factory* factory, 
                                     const unsigned char *buffer, size_t len,
                                     const unsigned char *identifier, 
                                     const unsigned char *suffix, 
                                     const char *mime_type)
{
  int score = 0;
  int ntriples_score;
  
  if(suffix) {
    if(!strcmp((const char*)suffix, "nq"))
      score = 2;

    /* Explicitly refuse to do anything with N-Triples, Turtle or N3
     * named content 
     */
    if(!strcmp((const char*)suffix, "nt") ||
       !strcmp((const char*)suffix, "ttl") ||
       !strcmp((const char*)suffix, "n3")) {
      return 0;
    }
  }
  
  if(mime_type) {
    if(strstr((const char*)mime_type, "nquads"))
      score += 2;
  }
  
  /* ntriples is a subset of nquads, score higher than ntriples */
  ntriples_score = raptor_ntriples_parse_recognise_syntax(factory, buffer, len, identifier, suffix, mime_type);
  if(ntriples_score > 0) {
    score += ntriples_score + 1;
  }

  return score;
}


static const char* const nquads_names[2] = { "nquads", NULL };

static const char* const nquads_uri_strings[2] = {
  "http://sw.deri.org/2008/07/n-quads/",
  NULL
};
  
#define NQUADS_TYPES_COUNT 1
static const raptor_type_q nquads_types[NQUADS_TYPES_COUNT + 1] = {
  { "text/x-nquads", 13, 10},
  { NULL, 0, 0}
};

static int
raptor_nquads_parser_register_factory(raptor_parser_factory *factory) 
{
  int rc = 0;

  factory->desc.names = nquads_names;

  factory->desc.mime_types = nquads_types;

  factory->desc.label = "N-Quads";
  factory->desc.uri_strings = nquads_uri_strings;

  factory->desc.flags = 0;
  
  factory->context_length     = sizeof(raptor_ntriples_parser_context);

  factory->init      = raptor_ntriples_parse_init;
  factory->terminate = raptor_ntriples_parse_terminate;
  factory->start     = raptor_ntriples_parse_start;
  factory->chunk     = raptor_ntriples_parse_chunk;
  factory->recognise_syntax = raptor_nquads_parse_recognise_syntax;

  return rc;
}


int
raptor_init_parser_nquads(raptor_world* world)
{
  return !raptor_world_register_parser_factory(world,
                                               &raptor_nquads_parser_register_factory);
}
#endif
