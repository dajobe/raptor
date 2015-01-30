/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * mkr_sentence.c - Raptor mKR parser sentence functions
 *
 * Copyright (C) 2015, David Beckett http://www.dajobe.org/
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
 * mKR is defined in http://mkrmke.org/parser/
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

#include "raptor2.h"
#include "raptor_internal.h"

#include <turtle_parser.h>

#define YY_NO_UNISTD_H 1
#include <turtle_lexer.h>

#include <turtle_common.h>



/* Make verbose error messages for syntax errors */
#define YYERROR_VERBOSE 1

/* Fail with an debug error message if RAPTOR_DEBUG > 1 */
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
#define YYERROR_MSG(msg) do { fputs("** YYERROR ", RAPTOR_DEBUG_FH); fputs(msg, RAPTOR_DEBUG_FH); fputc('\n', RAPTOR_DEBUG_FH); YYERROR; } while(0)
#else
#define YYERROR_MSG(ignore) YYERROR
#endif
#define YYERR_MSG_GOTO(label,msg) do { errmsg = msg; goto label; } while(0)

/* Slow down the grammar operation and watch it work */
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 2
#undef YYDEBUG
#define YYDEBUG 1
#endif


/* the lexer does not seem to track this */
#undef RAPTOR_TURTLE_USE_ERROR_COLUMNS


/* global variables */
extern raptor_term* groupType = NULL;
extern raptor_term* groupName = NULL;
extern raptor_sequence* groupList = NULL;

/* Prototypes for testing */
void mkr_spo(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* predicate, raptor_term* object);
void mkr_attribute_po(raptor_parser* rdf_parser, raptor_term* var1, unsigned char* var2, raptor_sequence* var3);
/* void mkr_poList(raptor_parser* rdf_parser, raptor_term* var1, raptor_sequence* var2); */
void mkr_blank_attribute(raptor_parser* rdf_parser, raptor_term* var1, raptor_term* var2, raptor_sequence* var3);

/*********************************/
/* mKR parser sentence functions */
/*********************************/
/* int mkr_statement_print(const raptor_statement* statement, FILE *stream); */
/* raptor_uri* raptor_new_uri_for_mkr_concept(raptor_world* world, const unsigned char *name); */
/* raptor_term_print_as_ntriples(raptor_term* t, stdout);   */
/* raptor_sequence_print(raptor_sequence* s, stdout);       */
/* raptor_uri_print(raptor_uri* u, stdout);                 */

/************************************************************************************/
/************************************************************************************/
/* Turtle style */
/* subject predicateObjectList */
void
mkr_poList(raptor_parser* rdf_parser, raptor_term* var1, raptor_sequence* var2)
/* triples: subject predicateObjectList */
{
  int i;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_poList: ");
  if(var1)
    raptor_term_print_as_ntriples(var1, stdout);
  else
    fputs("NULL", stdout);
  if(var2) {
    printf(" (reverse order to syntax) ");
    raptor_sequence_print(var2, stdout);
    printf("\n");
  } else     
    printf(" empty poList\n");
#endif

  if(var1 && var2) {
    /* have subject and non-empty property list, handle it  */
    for(i = 0; i < raptor_sequence_size(var2); i++) {
      raptor_statement* triple = (raptor_statement*)raptor_sequence_get_at(var2, i);
      triple->subject = raptor_term_copy(var1);
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" after substitution predicateObjectList=");
    raptor_sequence_print(var2, stdout);
    printf("\n\n");
#endif
    for(i = 0; i < raptor_sequence_size(var2); i++) {
      raptor_statement* triple = (raptor_statement*)raptor_sequence_get_at(var2, i);
      raptor_mkr_generate_statement(rdf_parser, triple);
    }
  }
/*
  if(var2)
    raptor_free_sequence(var2);
  if(var1)
    raptor_free_term(var1);
*/
}


/*****************************************************************************/
/*****************************************************************************/
/* prefixes */

/* sentence: BASE URI_LITERAL SEMICOLON */
void
mkr_base(raptor_parser* rdf_parser, raptor_uri* var2)
{
  raptor_uri *uri = var2;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_base: @base ");
  printf("<%s> ;\n", raptor_uri_as_string(var2));
#endif

  if(rdf_parser->base_uri)
    raptor_free_uri(rdf_parser->base_uri);
  rdf_parser->base_uri = uri;
}

/*****************************************************************************/
/* sentence: prefix IDENTIFIER URI_LITERAL SEMICOLON */
/* prefix: "@prefix" | "@mkb" | "@mke" */
void
mkr_prefix(raptor_parser* rdf_parser, unsigned char* var1, unsigned char* var2, raptor_uri* var3)
{
  unsigned char *prefix = var2;
  raptor_turtle_parser* mkr_parser = (raptor_turtle_parser*)(rdf_parser->context);
  raptor_namespace *ns;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_prefix: ");
  printf("%s %s <%s> ;\n", var1, var2, raptor_uri_as_string(var3));
#endif

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("directive %s %s %s\n", var1, (var2 ? (char*)var2 : "(default)"), raptor_uri_as_string(var3));
#endif

  if(prefix) {
    size_t len = strlen((const char*)prefix);
    if(prefix[len-1] == ':') {
      if(len == 1)
         /* declaring default namespace prefix PREFIX : ... */
        prefix = NULL;
      else
        prefix[len-1]='\0';
    }
  }

  ns = raptor_new_namespace_from_uri(&mkr_parser->namespaces, prefix, var3, 0);
  if(ns) {
    raptor_namespaces_start_namespace(&mkr_parser->namespaces, ns);
    raptor_parser_start_namespace(rdf_parser, ns);
  } else
    printf("##### ERROR: mkr_prefix: can't create new namespace for %s\n", (char*)prefix);

}

/*****************************************************************************/
/*****************************************************************************/
/* sentence: viewOption nameOption sentence SEMICOLON */
void
mkr_sentence(raptor_parser* rdf_parser, raptor_term* var1, raptor_term* var2, raptor_term* var3)
{
#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_sentence: ");
  printf("in ");
  raptor_term_print_as_ntriples(var1, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var2, stdout);
  printf(" :: ");
  raptor_term_print_as_ntriples(var3, stdout);
  printf(" ;\n");
#endif

  printf("##### WARNING: mkr_sentence: not implemented\n");
}

/*****************************************************************************/
/* sentence: LEFT_CURLY sentenceList LEFT_CURLY SEMICOLON */
void
mkr_view(raptor_parser* rdf_parser, raptor_sequence* var2)
{
#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_view: ");
  printf("{ ");
  raptor_sequence_print(var2, stdout);
  printf(" } ;\n");

  printf("##### WARNING: mkr_view: not implemented\n");
#endif
}


/*****************************************************************************/
/* doVerb predicate [= event] ; */
/* doVerb predicate [= event] ppList ; */
void
mkr_command(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* var1, raptor_term* var2, raptor_sequence* var3)
{
  raptor_term* verb = NULL;
  raptor_term* command = NULL;
  raptor_sequence* pplist = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_command: ");
  raptor_term_print_as_ntriples(var1, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var2, stdout);
  printf(" ");
  raptor_sequence_print(var3, stdout);
  printf(";\n");
#endif

  verb = raptor_term_copy(var1);
  command = raptor_term_copy(var2);
  pplist = var3;
  mkr_pplist(rdf_parser, MKR_COMMAND, subject, verb, command, pplist);
}

/*****************************************************************************/
/* begin groupType groupName; */
/* end   groupType groupName; */
void
mkr_group(raptor_parser* rdf_parser, mkr_type type, raptor_term* var2, raptor_term* var3)
{
  raptor_world* world = rdf_parser->world;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
  raptor_term* t4 = NULL;
  raptor_statement* triple = NULL;
#if defined(RAPTOR_DEBUG)
  const char* event = NULL;

  switch(type) {
    case MKR_BEGIN: event = "begin"; break;
    case MKR_END:   event = "end";   break;
  }
  printf("##### DEBUG: mkr_group: ");
  printf("%s ", event);
  raptor_term_print_as_ntriples(var2, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var3, stdout);
  printf(";\n");
#endif

  groupType = raptor_term_copy(var2);
  groupName = raptor_term_copy(var3);
  switch(type) {
    case MKR_BEGIN:
      t1 = raptor_term_copy(groupName);
      t2 = raptor_term_copy(RAPTOR_RDF_type_term(world));
      t3 = raptor_term_copy(groupType);
      triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple);
      groupList = raptor_new_sequence((raptor_data_free_handler)raptor_free_term,
                                     (raptor_data_print_handler)raptor_term_print_as_ntriples);
      break;

    case MKR_END:
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
      printf("groupList = (");
      raptor_sequence_print(groupList, stdout);  /* core dump ? */
      printf(")\n");
#endif

      t1 = raptor_term_copy(groupName);
      if(raptor_term_equals(groupType, mkr_new_variable(rdf_parser, MKR_VARIABLE, (unsigned char*)"hierarchy")))
        t2 = raptor_term_copy(mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"ho"));
      else if(raptor_term_equals(groupType, mkr_new_variable(rdf_parser, MKR_VARIABLE, (unsigned char*)"relation")))
        t2 = raptor_term_copy(mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"rel"));
      else
        t2 = raptor_term_copy(mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"groupList"));
      t4 = raptor_term_copy(mkr_new_rdflist(rdf_parser, groupList));
      triple = raptor_new_statement_from_nodes(world, t1, t2, t4, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple);

      if(groupType) {
        raptor_free_term(groupType);
        groupType = NULL;
      }
      if(groupName) {
        raptor_free_term(groupName);
        groupName = NULL;
      }
      if(groupList) {
        /* raptor_free_sequence(groupList);  core dump */
        groupList = NULL;
      }
      break;
  }
#if defined(RAPTOR_DEBUG)
  printf("exit mkr_group\n");
#endif

}


/*****************************************************************************/
/* ho objectList; */
void
mkr_ho(raptor_parser* rdf_parser, raptor_sequence* var2)
{
  raptor_term* hoList = NULL;
  raptor_term* t4 = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_ho: ho ");
  raptor_sequence_print(var2, stdout);
  printf(";\n");
#endif
  
  hoList = mkr_new_rdflist(rdf_parser, var2);
  t4 = raptor_term_copy(hoList);
  raptor_sequence_push(groupList, t4);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_ho: exit\n");
#endif
}


/* rel objectList; */
void
mkr_rel(raptor_parser* rdf_parser, raptor_sequence* var2)
{
  raptor_term* relList = NULL;
  raptor_term* t4 = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_rel: rel ");
  raptor_sequence_print(var2, stdout);
  printf(";\n");
#endif

  relList = mkr_new_rdflist(rdf_parser, var2);
  t4 = raptor_term_copy(relList);
  raptor_sequence_push(groupList, t4);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: exit mkr_rel\n");
#endif
}

/*****************************************************************************/
/***************************************************************/
/* type: MKR_DEFINITION | MKR_ACTION | MKR_COMMAND             */
/* sentence: subject isVerb genus   [= event] pplist SEMICOLON */
/*         | subject doVerb action  [= event] ppList SEMICOLON */
/*         |         doVerb command [= event] ppList SEMICOLON */
/************************** predicate **************************/
void
mkr_pplist(raptor_parser* rdf_parser, mkr_type type, raptor_term* subject, raptor_term* verb, raptor_term* predicate, raptor_sequence* pplist)
{
  raptor_world* world = rdf_parser->world;
  raptor_term* sub = NULL;
  raptor_term* event = NULL;
  raptor_term* nvevent = NULL;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
  raptor_sequence* seq = NULL;
  raptor_term* rdflist = NULL;
  raptor_statement* triple = NULL;
  int i = 0;
  int j = 0;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_pplist: %s: ", MKR_TYPE_NAME(type));
  raptor_term_print_as_ntriples(subject, stdout); printf(" ");
  raptor_term_print_as_ntriples(verb, stdout); printf(" ");
  raptor_term_print_as_ntriples(predicate, stdout); printf(" ");
  raptor_sequence_print(pplist, stdout); printf(";\n");
#endif

  if(subject)
    sub = raptor_term_copy(subject);
  else switch(verb->mkrtype) {
    case MKR_DO:
    case MKR_HDO:
      sub = mkr_new_variable(rdf_parser, MKR_SUBJECT, (unsigned char*)"ke");
      break;

    case MKR_BANG:
      sub = mkr_new_variable(rdf_parser, MKR_SUBJECT, (unsigned char*)"sh");
      break;

    default:
      printf("##### ERROR: mkr_pplist: unexpected verb mkrtype: %s\n", MKR_TYPE_NAME(verb->mkrtype));
      return;
      break;
  }
  if(!sub) {
    printf("##### ERROR: mkr_pplist: NULL subject\n");
    return;
  }
  sub = raptor_term_copy(sub);

  event = mkr_new_blankTerm(rdf_parser);
  event->mkrtype = type;

#if defined(RAPTOR_DEBUG)
  printf("sub = "); raptor_term_print_as_ntriples(sub, stdout); printf("\n");
  printf("event = "); raptor_term_print_as_ntriples(event, stdout); printf("\n");
#endif


  switch(type) {
    case MKR_DEFINITION:
      /* sub definition event .  */
      /* event genus predicate . */
      /* event pplist .          */
      break;

    case MKR_ACTION:
    case MKR_COMMAND:
      /* sub predicate event . */
      /* event agent sub .     */
      /* event pplist .        */

      /* predicate iss action */
      t1 = raptor_term_copy(predicate);
      t2 = raptor_term_copy( mkr_new_variable(rdf_parser, MKR_HOVERB, (unsigned char*)"iss") );
      t3 = raptor_term_copy( mkr_new_variable(rdf_parser, MKR_ACTION, (unsigned char*)"action") );
      triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple); triple = NULL;
      break;

    default:
      printf("##### ERROR: mkr_pplist: unexpected sentence type: %s\n", MKR_TYPE_NAME(type));
      return;
      break;
  }

/******************************************************************************/

  switch(type) {
    case MKR_DEFINITION:
      /* sub definition event .   */
      t1 = raptor_term_copy(sub);
      t2 = raptor_term_copy( mkr_new_variable(rdf_parser, MKR_DEFINITION, (unsigned char*)"definition") );
      if(!t2) {
        printf("##### ERROR: mkr_pplist: NULL definition term\n");
        return;
      }
      t3 = raptor_term_copy(event);
      triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple); triple = NULL;
      break;

    case MKR_ACTION:
    case MKR_COMMAND:
      /* sub predicate event . */
      t1 = raptor_term_copy(sub);
      t2 = raptor_term_copy(predicate);
      t3 = raptor_term_copy(event);
      triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple); triple = NULL;
      break;
  }

/**********************************************************/

  switch(type) {
    case MKR_DEFINITION:
      /* event genus predicate . */
      t1 = raptor_term_copy(event);
      t2 = raptor_term_copy( mkr_new_variable(rdf_parser, MKR_GENUS, (unsigned char*)"genus") );
      t3 = raptor_term_copy(predicate);
      triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple); triple = NULL;
      break;

    case MKR_ACTION:
    case MKR_COMMAND:
      /* event agent sub .     */
      t1 = raptor_term_copy(event);
      t2 = raptor_term_copy( mkr_new_variable(rdf_parser, MKR_AGENT, (unsigned char*)"agent") );
      t3 = raptor_term_copy(sub);
      triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple); triple = NULL;
      break;
  }

/**********************************************************/

  switch(type) {
    case MKR_DEFINITION:
    case MKR_ACTION:
    case MKR_COMMAND:
      /* event pplist .        */

  /* event pplist */
  for(i = 0; i < raptor_sequence_size(pplist); i++) {
    MKR_pp *pp = (MKR_pp*)raptor_sequence_get_at(pplist, i);
#if defined(RAPTOR_DEBUG)
    printf("pp = "); mkr_pp_print(pp, stdout); printf("\n");
#endif
    raptor_term* preposition = pp->ppPreposition;
    raptor_sequence* ppSequence = pp->ppSequence;
    switch(pp->ppType) {
      case MKR_PPOBJ:
        /* event preposition rdflist */
        rdflist = mkr_new_rdflist(rdf_parser, ppSequence);
        t1 = raptor_term_copy(event);
        t2 = raptor_term_copy(preposition);
        t3 = raptor_term_copy(rdflist);
        triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
        raptor_mkr_generate_statement(rdf_parser, triple);
        raptor_free_statement(triple); triple = NULL;
        break;

      case MKR_PPNV:
        /* event preposition nvevent . */
        nvevent = mkr_new_blankTerm(rdf_parser);
        nvevent->mkrtype = type;

#if defined(RAPTOR_DEBUG)
        printf("nvevent = "); raptor_term_print_as_ntriples(nvevent, stdout); printf("\n");
#endif
        t1 = raptor_term_copy(event);
        t2 = raptor_term_copy(preposition);
        t3 = raptor_term_copy(nvevent);
        triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
        raptor_mkr_generate_statement(rdf_parser, triple);
        raptor_free_statement(triple); triple = NULL;

        for(j = 0; j < raptor_sequence_size(ppSequence); j++) {
          /* nvevent name value . */
          MKR_nv* nv = (MKR_nv*)raptor_sequence_get_at(ppSequence, j);
#if defined(RAPTOR_DEBUG)
          printf("nv = "); mkr_nv_print(nv, stdout); printf("\n");
#endif
          raptor_term* name = nv->nvName;
          MKR_value* value = nv->nvValue;
          t1 = raptor_term_copy(nvevent);
          t2 = raptor_term_copy(name);
          switch(value->mkrtype) {
            case MKR_NIL:
            case MKR_OBJECT:
            case MKR_RDFLIST:
            case MKR_RESULTSET:
            case MKR_SENTENCE:
            case MKR_RAPTOR_TERM:
              t3 = (raptor_term*)value->mkrvalue;
              break;

            case MKR_OBJECTSET:
            case MKR_OBJECTLIST:
            case MKR_SENTENCELIST:
            case MKR_RAPTOR_SEQUENCE:
              seq = (raptor_sequence*)value->mkrvalue;
              rdflist = mkr_new_rdflist(rdf_parser, seq);
#if defined(RAPTOR_DEBUG)
              printf("seq = "); raptor_sequence_print(seq, stdout); printf("\n");
              printf("rdflist = "); mkr_rdflist_print(rdflist, stdout); printf("\n");
#endif
              t3 = raptor_term_copy(rdflist);
              break;

            default:
              printf("##### WARNING: mkr_pplist: unexpected pp value type: %s\n", MKR_TYPE_NAME(value->mkrtype));
              t3 = raptor_term_copy(raptor_new_term_from_uri(world, RAPTOR_RDF_nil_URI(world)));
              break;
          }
          triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
          raptor_mkr_generate_statement(rdf_parser, triple);
          raptor_free_statement(triple); triple = NULL;


/*
          if(nv)
            mkr_free_nv(nv); nv = NULL;
*/
        }
        if(nvevent)
          raptor_free_term(nvevent); nvevent = NULL;
        break;

      default:
        printf("##### ERROR: mkr_pplist: not ppList type: %s\n", MKR_TYPE_NAME(pp->ppType));
        break;
    }
/*
    if(pp)
      mkr_free_pp(pp); pp = NULL;
*/
  }
      break;
  }

/**********************************************************/

/*
  if(sub)
    raptor_free_term(sub); sub = NULL;
  if(blank)
    raptor_free_term(blank); blank = NULL;
*/

#if defined(RAPTOR_DEBUG)
  printf("exit mkr_pplist: \n");
#endif
}


/*******************************************************************/
/* variable := value; */
void
mkr_assignment(raptor_parser* rdf_parser, raptor_term* var1, MKR_value* var3)
{
  raptor_world* world = rdf_parser->world;
  mkr_type type = MKR_UNKNOWN;
  raptor_term *t1 = NULL;
  raptor_term *t2 = NULL;
  raptor_term *t3 = NULL;
  raptor_statement* triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_assignment: ");
  raptor_term_print_as_ntriples(var1, stdout);
  printf(" := ");
  mkr_value_print(var3, stdout);
  printf(";\n");
#endif

  switch(type = var3->mkrtype) {
    case MKR_OBJECT:
      t1 = raptor_term_copy(var1);
      t2 = mkr_local_to_raptor_term(rdf_parser, (unsigned char*)":=");
      t3 = raptor_term_copy((raptor_term*)var3->mkrvalue);
      triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple);
      break;

    case MKR_RDFLIST:
    case MKR_SENTENCE:
    case MKR_OBJECTLIST:
    case MKR_SENTENCELIST:
      printf("##### WARNING: mkr_assignment: not implemented: value type: %s: value = ", MKR_TYPE_NAME(type));
      mkr_value_print(var3, stdout);
      printf("\n");
      break;

    case MKR_UNKNOWN:
    default:
      printf("WARNING: mkr_assignment: unexpected value type: %s value = ", MKR_TYPE_NAME(type));
      mkr_value_print(var3, stdout);
      printf("\n");
      break;
  }

#if defined(RAPTOR_DEBUG)
  printf("exit mkr_asssignment: \n");
#endif
}

/*****************************************************************************/
/* subject is object; */
raptor_statement*
mkr_alias(raptor_parser* rdf_parser, raptor_term* var1, raptor_term* var2, raptor_term* var3)
{
  raptor_world* world = rdf_parser->world;
  raptor_statement* triple = NULL;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_alias: ");
  raptor_term_print_as_ntriples(var1, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var2, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var3, stdout);
  printf(";\n");
#endif

  t1 = raptor_term_copy(var1);
  t2 = raptor_term_copy(var2);
  t3 = raptor_term_copy(var3);
  triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);

  return triple;
}

/*****************************************************************************/
/* subject is object ppList; */
void
mkr_definition(raptor_parser* rdf_parser, raptor_term* var1, raptor_term* var2, raptor_term* var3, raptor_sequence* var4)
{
  raptor_term* subject = NULL;
  raptor_term* verb = NULL;
  raptor_term* genus = NULL;
  raptor_sequence* pplist = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_definition: ");
  raptor_term_print_as_ntriples(var1, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var2, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var3, stdout);
  printf(" ");
  raptor_sequence_print(var4, stdout);
  printf(";\n");
#endif

  subject = raptor_term_copy(var1);
  verb = raptor_term_copy(var2);
  genus = raptor_term_copy(var3);
  pplist = var4;

  mkr_pplist(rdf_parser, MKR_DEFINITION, subject, verb, genus, pplist);
}


/*****************************************************************************/
/* subject hoVerb object; */
raptor_statement*
mkr_hierarchy(raptor_parser* rdf_parser, raptor_term* var1, raptor_term* var2, raptor_term* var3)
{
  raptor_world* world = rdf_parser->world;
  raptor_statement* triple = NULL;
  int i;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_hierarchy: ");
  raptor_term_print_as_ntriples(var1, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var2, stdout);
  printf(" ");
  raptor_sequence_print(var3, stdout);
  printf(";\n");
#endif

  { /* objectList */
    raptor_term* t1 = raptor_term_copy(var1);
    raptor_term* t2 = raptor_term_copy(var2);
    raptor_term* t3 = raptor_term_copy(var3);
    triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
  } /* objectList */

  return triple;
}


/*****************************************************************************/
/* sentence: subject hasVerb nv SEMICOLON */
/* hasVerb: HAS | HASPART | ISAPART */
raptor_statement*
mkr_attribute(raptor_parser* rdf_parser, raptor_term* var1, raptor_term* var2, MKR_nv* var3)
{
  raptor_world* world = rdf_parser->world;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
  raptor_term* subject = NULL;
  raptor_statement* triple = NULL;
  mkr_type type = MKR_UNKNOWN;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_attribute: ");
  raptor_term_print_as_ntriples(var1, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var2, stdout);
  printf(" ");
  mkr_nv_print(var3, stdout);
  printf(";\n");
#endif

  subject = raptor_term_copy(var1);
  { /* nvList */
    MKR_nv* nv = (MKR_nv*)var3;
    raptor_term* predicate = nv->nvName;
    MKR_value* value = nv->nvValue;
    raptor_term* object = NULL;
#if defined(RAPTOR_DEBUG)
    mkr_nv_print(nv, stdout);
    printf("\n");
#endif
    switch(type = value->mkrtype) {
      case MKR_NIL:
      case MKR_OBJECT:
      case MKR_RDFLIST:
      case MKR_RDFSET:
      case MKR_OBJECTSET:
      case MKR_SENTENCE:
        object = (raptor_term*)value->mkrvalue;
        t1 = raptor_term_copy(subject);
        t2 = raptor_term_copy(predicate);
        t3 = raptor_term_copy(object);
        triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
        break;

      case MKR_SENTENCELIST:
        printf("##### WARNING: mkr_attribute: not implemented mkrtype: %s\n", MKR_TYPE_NAME(type));
        break;

      case MKR_VALUELIST:
      case MKR_VALUESET:
      default:
        printf("##### ERROR: mkr_attribute: unexpected mkrtype: %s\n", MKR_TYPE_NAME(type));
        break;
    }
/*
    if(nv)
      mkr_free_nv(nv);
    if(predicate)
      raptor_free_term(predicate);
    if(object)
      raptor_free_term(object);
*/
  } /* nvList */
/*
  if(subject)
    raptor_free_term(subject);
*/

#if defined(RAPTOR_DEBUG)
  mkr_statement_print(triple, stdout);
#endif
  return triple;
}

/*****************************************************************************/
/* subject partVerb nv; */
raptor_statement*
mkr_part(raptor_parser* rdf_parser, raptor_term* var1, raptor_term* var2, MKR_nv* var3)
{
  raptor_world* world = rdf_parser->world;
  mkr_type type = MKR_UNKNOWN;
  MKR_value* value = NULL;
  raptor_statement* triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_part: ");
  raptor_term_print_as_ntriples(var1, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var2, stdout);
  printf(" ");
  raptor_sequence_print(var3, stdout);
  printf(";\n");
#endif

  { /* nvList */
     MKR_nv* nv = (MKR_nv*)var3;
     raptor_term* t1 = raptor_term_copy(var1);
     raptor_term* t2 = nv->nvName;
     raptor_term* t3 = nv->nvValue;
     value = nv->nvValue;
     switch(type = value->mkrtype) {
       case MKR_RAPTOR_TERM:
         t3 = (raptor_term*)value->mkrvalue;
         break;

       default:
         printf("##### ERROR: mkr_part: unexpected value type: %s\n", MKR_TYPE_NAME(type));
         goto part_error;
         break;
     }
     triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
  } /* nvList */

  part_error:

  return triple;
}

/*****************************************************************************/
/* subject doVerb predicate [= event] ppList; */
/* subject doVerb predicate = event ppList;  not implemented */
void
mkr_action(raptor_parser* rdf_parser, raptor_term* var1, raptor_term* var2, raptor_term* var3, raptor_sequence* var4)
{
  raptor_term* subject = NULL;
  raptor_term* verb = NULL;
  raptor_term* action = NULL;
  raptor_sequence* pplist = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_action: ");
  raptor_term_print_as_ntriples(var1, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var2, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(var3, stdout);
  printf(" ");
  raptor_sequence_print(var4, stdout);
  printf(";\n");
#endif

  subject = raptor_term_copy(var1);
  verb = raptor_term_copy(var2);
  action = raptor_term_copy(var3);
  pplist = var4;

  mkr_pplist(rdf_parser, MKR_ACTION, subject, verb, action, pplist);
}

/************************************************************************************/
/************************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/**
 * mkr_statement_print:
 * @statement: #raptor_statement object to print
 * @stream: FILE* stream
 *
 * Print a mkr_statement to a stream.
 *
 * Return value: non-0 on failure
 **/
int
mkr_statement_print(const raptor_statement * statement, FILE *stream) 
{
  raptor_term_type type = RAPTOR_TERM_TYPE_UNKNOWN;
  int rc = 0;
 
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(statement, raptor_statement, 1);

  fputc('(', stream);

  if(!statement->subject) {
    fputs("NULL", stream);
  } else {
    printf("%s(", RAPTOR_TERM_TYPE_NAME(statement->subject->type));
    switch(type = statement->subject->type) {
      case RAPTOR_TERM_TYPE_URI:
        raptor_uri_print(statement->subject->value.uri, stream);
        break;

      case RAPTOR_TERM_TYPE_BLANK:
        fputs((const char*)statement->subject->value.blank.string, stream);
        break;

      default:
      case RAPTOR_TERM_TYPE_LITERAL:
      case RAPTOR_TERM_TYPE_UNKNOWN:
        printf("(subject term type: %s)", RAPTOR_TERM_TYPE_NAME(type));
        break;
    }
  }

  fputs("), ", stream);

  if(!statement->predicate) {
    fputs("NULL", stream);
  } else {
    printf("%s(", RAPTOR_TERM_TYPE_NAME(statement->predicate->type));
    switch(type = statement->predicate->type) {
      case RAPTOR_TERM_TYPE_URI:
        raptor_uri_print(statement->predicate->value.uri, stream);
        break;

      case RAPTOR_TERM_TYPE_BLANK:
        fputs((const char*)statement->predicate->value.blank.string, stream);
        break;

      default:
      case RAPTOR_TERM_TYPE_LITERAL:
      case RAPTOR_TERM_TYPE_UNKNOWN:
        printf("(predicate term type: %s)", RAPTOR_TERM_TYPE_NAME(type));
        break;
    }
  }
  
  fputs("), ", stream);

  if(!statement->object) {
    fputs("NULL", stream);
  } else {
    printf("%s(", RAPTOR_TERM_TYPE_NAME(statement->object->type));
    switch(type = statement->object->type) {
      case RAPTOR_TERM_TYPE_URI:
        raptor_uri_print(statement->object->value.uri, stream);
        break;

      case RAPTOR_TERM_TYPE_BLANK:
        fputs((const char*)statement->object->value.blank.string, stream);
        break;

      case RAPTOR_TERM_TYPE_LITERAL:
        if(statement->object->value.literal.datatype) {
          raptor_uri* dt_uri = statement->object->value.literal.datatype;
          fputc('<', stream);
          fputs((const char*)raptor_uri_as_string(dt_uri), stream);
          fputc('>', stream);
        }
        fputc('"', stream);
        fputs((const char*)statement->object->value.literal.string, stream);
        fputc('"', stream);
        if(statement->object->type == RAPTOR_TERM_TYPE_BLANK) {
          fputs("_:", stream);
          fputs((const char*)statement->object->value.blank.string, stream);
        } else {
          raptor_uri_print(statement->object->value.uri, stream);
        }
        break;

      default:
      case RAPTOR_TERM_TYPE_UNKNOWN:
        printf("(object term type: %s)", RAPTOR_TERM_TYPE_NAME(type));
        break;
    }
  }

  if(statement->graph) {
    switch(type = statement->graph->type) {
      case RAPTOR_TERM_TYPE_URI:
        if(statement->graph->value.uri) {
          fputs("), ", stream);
          raptor_uri_print(statement->graph->value.uri, stream);
        }
        break;

      case RAPTOR_TERM_TYPE_BLANK:
        if(statement->graph->value.blank.string) {
          fputs("), ", stream);
          fputs((const char*)statement->graph->value.blank.string, stream);
        }
        break;

      default:
      case RAPTOR_TERM_TYPE_LITERAL:
      case RAPTOR_TERM_TYPE_UNKNOWN:
        printf("%s(", RAPTOR_TERM_TYPE_NAME(type));
        break;
    }
  }

  fputc('))', stream);

  return rc;
}



/*****************************************************************************/
const char*
RAPTOR_TERM_TYPE_NAME(raptor_term_type n)
{
  switch(n) {
    default:
      printf("(not raptor_term_type: %u)",n);
      return NULL;
      break;
    case RAPTOR_TERM_TYPE_UNKNOWN: 		return "RAPTOR_TERM_TYPE_UNKNOWN";		break;
    case RAPTOR_TERM_TYPE_URI:     		return "RAPTOR_TERM_TYPE_URI";			break;
    case RAPTOR_TERM_TYPE_LITERAL: 		return "RAPTOR_TERM_TYPE_LITERAL";		break;
    /* unused type 3 */
    case RAPTOR_TERM_TYPE_BLANK:   		return "RAPTOR_TERM_TYPE_BLANK";		break;
  }
}

const char*
MKR_TYPE_NAME(mkr_type n)
{
  switch(n) {
    default:
      printf("(not mkr_type: %u)",n);
      return NULL;
      break;
    case MKR_UNKNOWN:		return "MKR_UNKNOWN";			break;
    case MKR_CHAR:		return "MKR_CHAR";			break;
    case MKR_STRING:		return "MKR_STRING";			break;
    case MKR_INTEGER:		return "MKR_INTEGER";			break;
    case MKR_REAL:		return "MKR_REAL";			break;
    case MKR_BOOLEAN:		return "MKR_BOOLEAN";			break;
    case MKR_GROUP:		return "MKR_GROUP";			break;
    case MKR_EXGROUP:		return "MKR_EXGROUP";			break;
    case MKR_ALT:		return "MKR_ALT";			break;
    case MKR_ENUM:		return "MKR_ENUM";			break;
    case MKR_INGROUP:		return "MKR_INGROUP";			break;
    case MKR_LIST:		return "MKR_LIST";			break;
    case MKR_SET:		return "MKR_SET";			break;
    case MKR_MULTISET:		return "MKR_MULTISET";			break;
    case MKR_ARRAY:		return "MKR_ARRAY";			break;
    case MKR_HIERARCHY:		return "MKR_HIERARCHY";			break;
    case MKR_HO:		return "MKR_HO";			break;
    case MKR_RELATION:		return "MKR_RELATION";			break;
    case MKR_REL:		return "MKR_REL";			break;
    case MKR_BEGIN:		return "MKR_BEGIN";			break;
    case MKR_END:		return "MKR_END";			break;
    case MKR_NAME:		return "MKR_NAME";			break;
    case MKR_METHOD:		return "MKR_METHOD";			break;

    case MKR_WORD:		return "MKR_WORD";			break;
    case MKR_PRONOUN:		return "MKR_PRONOUN";			break;
    case MKR_I:			return "MKR_I";				break;
    case MKR_ME:		return "MKR_ME";			break;
    case MKR_YOU:		return "MKR_YOU";			break;
    case MKR_HE:		return "MKR_HE";			break;
    case MKR_SHE:		return "MKR_SHE";			break;
    case MKR_IT:		return "MKR_IT";			break;
    case MKR_HERE:		return "MKR_HERE";			break;
    case MKR_NOW:		return "MKR_NOW";			break;
    case MKR_PREPOSITION:	return "MKR_PREPOSITION";		break;
    case MKR_AT:		return "MKR_AT";			break;
    case MKR_OF:		return "MKR_OF";			break;
    case MKR_WITH:		return "MKR_WITH";			break;
    case MKR_OD:		return "MKR_OD";			break;
    case MKR_FROM:		return "MKR_FROM";			break;
    case MKR_TO:		return "MKR_TO";			break;
    case MKR_IN:		return "MKR_IN";			break;
    case MKR_OUT:		return "MKR_OUT";			break;
    case MKR_WHERE:		return "MKR_WHERE";			break;
    case MKR_QUANTIFIER:	return "MKR_QUANTIFIER";		break;
    case MKR_OPTIONAL:		return "MKR_OPTIONAL";			break;
    case MKR_NO:		return "MKR_NO";			break;
    case MKR_ANY:		return "MKR_ANY";			break;
    case MKR_SOME:		return "MKR_SOME";			break;
    case MKR_MANY:		return "MKR_MANY";			break;
    case MKR_ALL:		return "MKR_ALL";			break;
    case MKR_CONJUNCTION:	return "MKR_CONJUNCTION";		break;
    case MKR_IF:		return "MKR_IF";			break;
    case MKR_THEN:		return "MKR_THEN";			break;
    case MKR_ELSE:		return "MKR_ELSE";			break;
    case MKR_FI:		return "MKR_CONJUNCTION";		break;
    case MKR_IFF:		return "MKR_IFF";			break;
    case MKR_IMPLIES:		return "MKR_IMPLIES";			break;
    case MKR_CAUSES:		return "MKR_CAUSES";			break;
    case MKR_BECAUSE:		return "MKR_BECAUSE";			break;
    case MKR_ITERATOR:		return "MKR_ITERATOR";			break;
    case MKR_EVERY:		return "MKR_EVERY";			break;
    case MKR_WHILE:		return "MKR_WHILE";			break;
    case MKR_UNTIL:		return "MKR_UNTIL";			break;
    case MKR_WHEN:		return "MKR_WHEN";			break;
    case MKR_FORSOME:		return "MKR_FORSOME";			break;
    case MKR_FORALL:		return "MKR_FORALL";			break;
    case MKR_PHRASE:		return "MKR_PHRASE";			break;
    case MKR_NV:		return "MKR_NV";			break;
    case MKR_PP:		return "MKR_PP";			break;
    case MKR_PPOBJ:		return "MKR_PPOBJ";			break;
    case MKR_PPNV:		return "MKR_PPNV";			break;
    case MKR_SUBJECT:		return "MKR_SUBJECT";			break;
    case MKR_VERB:		return "MKR_VERB";			break;
    case MKR_ISVERB:		return "MKR_ISVERB";			break;
    case MKR_IS:		return "MKR_IS";			break;
    case MKR_HOVERB:		return "MKR_HOVERB";			break;
    case MKR_ISU:		return "MKR_ISU";			break;
    case MKR_ISS:		return "MKR_ISS";			break;
    case MKR_ISA:		return "MKR_ISA";			break;
    case MKR_ISASTAR:		return "MKR_ISASTAR";			break;
    case MKR_ISP:		return "MKR_ISP";			break;
    case MKR_ISG:		return "MKR_ISS";			break;
    case MKR_ISC:		return "MKR_ISC";			break;
    case MKR_ISCSTAR:		return "MKR_ISCSTAR";			break;
    case MKR_HASVERB:		return "MKR_HASVERB";			break;
    case MKR_HAS:		return "MKR_HAS";			break;
    case MKR_HASPART:		return "MKR_HASPART";			break;
    case MKR_ISAPART:		return "MKR_ISAPART";			break;
    case MKR_DOVERB:		return "MKR_DOVERB";			break;
    case MKR_DO:		return "MKR_DO";			break;
    case MKR_HDO:		return "MKR_HDO";			break;
    case MKR_BANG:		return "MKR_BANG";			break;
    case MKR_PREDICATE:		return "MKR_PREDICATE";			break;

    case MKR_SENTENCE:		return "MKR_SENTENCE";			break;
    case MKR_CONTEXT:		return "MKR_CONTEXT";			break;
    case MKR_STATEMENT:		return "MKR_STATEMENT";			break;
    case MKR_DEFINITION:	return "MKR_DEFINITION";		break;
    case MKR_GENUS:		return "MKR_GENUS";			break;
    case MKR_DIFFERENTIA:	return "MKR_DIFFERENTIA";		break;
    case MKR_QUESTION:		return "MKR_QUESTION";			break;
    case MKR_COMMAND:		return "MKR_COMMAND";			break;
    case MKR_ASSIGNMENT:	return "MKR_ASSIGNMENT";		break;
    case MKR_CONDITIONAL:	return "MKR_CONDITIONAL";		break;
    case MKR_ITERATION:		return "MKR_ITERATION";			break;

    case MKR_CONCEPT:		return "MKR_CONCEPT";			break;
    case MKR_UNIT:		return "MKR_UNIT";			break;
    case MKR_UNIVERSE:		return "MKR_UNIVERSE";			break;
    case MKR_ENTITY:		return "MKR_ENTITY";			break;
    case MKR_CHARACTERISTIC:	return "MKR_CHARACTERISTIC";		break;
    case MKR_PROPERTY:		return "MKR_PROPERTY";			break;
    case MKR_ATTRIBUTE:		return "MKR_ATTRIBUTE";			break;
    case MKR_PART:		return "MKR_PART";			break;
    case MKR_ACTION:		return "MKR_ACTION";			break;
    case MKR_AGENT:		return "MKR_AGENT";			break;
    case MKR_SPACE:		return "MKR_SPACE";			break;
    case MKR_TIME:		return "MKR_TIME";			break;
    case MKR_VIEW:		return "MKR_VIEW";			break;

    case MKR_SUBJECTLIST:	return "MKR_SUBJECTLIST";		break;
    case MKR_PHRASELIST:	return "MKR_PHRASELIST";		break;
    case MKR_NVLIST:		return "MKR_NVLIST";			break;
    case MKR_PPLIST:		return "MKR_PPLIST";			break;

    case MKR_RAPTOR_TERM:	return "MKR_RAPTOR_TERM";		break;
    case MKR_RAPTOR_SEQUENCE:	return "MKR_RAPTOR_SEQUENCE";		break;

    case MKR_VARIABLE:		return "MKR_VARIABLE";			break;
    case MKR_BVARIABLE:		return "MKR_BVARIABLE";			break;
    case MKR_DVARIABLE:		return "MKR_DVARIABLE";			break;
    case MKR_QVARIABLE:		return "MKR_QVARIABLE";			break;
    case MKR_VALUE:		return "MKR_VALUE";			break;
    case MKR_NIL:		return "MKR_NIL";			break;
    case MKR_OBJECT:		return "MKR_OBJECT";			break;
    case MKR_OBJECTLIST:	return "MKR_OBJECTLIST";		break;
    case MKR_VALUELIST:		return "MKR_VALUELIST";			break;
    case MKR_SENTENCELIST:	return "MKR_SENTENCELIST";		break;
    case MKR_RDFLIST:		return "MKR_RDFLIST";			break;
    case MKR_OBJECTSET:		return "MKR_OBJECTSET";			break;
    case MKR_VALUESET:		return "MKR_VALUESET";			break;
    case MKR_SENTENCESET:	return "MKR_SENTENCESET";		break;
    case MKR_RDFSET:		return "MKR_RDFSET";			break;
    case MKR_RESULTSET:		return "MKR_RESULTSET";			break;
  }
}



/************************************************************************************/
/************************************************************************************/
/************************************************************************************/

/* testing functions */



/*****************************************************************************/
/*****************************************************************************/
/* subject HAS predicateObjectList; */
void
mkr_attribute_po(raptor_parser* rdf_parser, raptor_term* var1, unsigned char* var2, raptor_sequence* var3)
{
  int i;

/* #if defined(RAPTOR_DEBUG) */
  printf("##### DEBUG: mkr_attribute_po: ");
printf("before print"); exit(0);
  raptor_term_print_as_ntriples(var1, stdout);
  printf(" has ");
  raptor_sequence_print(var3, stdout);
  printf(";\n");
/* #endif */

  for(i = 0; i < raptor_sequence_size(var3); i++) {
    raptor_statement* triple = (raptor_statement*)raptor_sequence_get_at(var3, i);
    triple->subject = raptor_term_copy(var1);
    raptor_mkr_generate_statement(rdf_parser, triple);
    if(triple)
      raptor_free_statement(triple);
  }

}

/************************************************************************************/
/* { blankNode  blankNodePropertyList predicateObjectListOpt SEMICOLON } */
void
mkr_blank_attribute(raptor_parser* rdf_parser, raptor_term* var1, raptor_term* var2, raptor_sequence* var3)
/* triples: blankNodePropertyList predicateObjectListOpt */
{
  int i;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("triples 2\n blankNodePropertyList=");
  if(var2)
    raptor_term_print_as_ntriples(var2, stdout);
  else
    fputs("NULL", stdout);
  if(var3) {
    printf("\n predicateObjectListOpt (reverse order to syntax)=");
    raptor_sequence_print(var3, stdout);
    printf("\n");
  } else     
    printf("\n and empty predicateObjectListOpt\n");
#endif

  if(var2 && var3) {
    /* have subject and non-empty predicate object list, handle it  */
    for(i = 0; i < raptor_sequence_size(var3); i++) {
      raptor_statement* t2 = (raptor_statement*)raptor_sequence_get_at(var3, i);
      t2->subject = raptor_term_copy(var2);
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" after substitution predicateObjectListOpt=");
    raptor_sequence_print(var3, stdout);
    printf("\n\n");
#endif
    for(i = 0; i < raptor_sequence_size(var3); i++) {
      raptor_statement* t2 = (raptor_statement*)raptor_sequence_get_at(var3, i);
      raptor_mkr_generate_statement(rdf_parser, t2);
    }
  }

  if(var3)
    raptor_free_sequence(var3);
  if(var2)
    raptor_free_term(var2);
}

/*****************************************************************************/
/* subject predicate object; */
void
mkr_spo(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* predicate, raptor_term* object)
{
  raptor_world* world = rdf_parser->world;
  raptor_term* t1 = subject;
  raptor_term* t3 = object;
  raptor_statement *triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_spo: ");
  raptor_term_print_as_ntriples(t1, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(predicate, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(t3, stdout);
  printf(" ;\n");
#endif

  triple = raptor_new_statement_from_nodes(world, t1, predicate, t3, NULL);
  raptor_mkr_generate_statement(rdf_parser, triple);
  raptor_free_statement(triple);

}
