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



/* Make predicateose error messages for syntax errors */
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


/* Prototypes for testing */
raptor_statement* mkr_spo(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* predicate, raptor_term* object);
raptor_statement* mkr_attribute_po(raptor_parser* rdf_parser, raptor_term* subject, unsigned char* verb, raptor_sequence* poList);
raptor_statement* mkr_blank_attribute(raptor_parser* rdf_parser, raptor_term* blank, raptor_term* verb, raptor_sequence* poList);

/*********************************/
/* mKR parser sentence functions */
/*********************************/
/* raptor_statement* mkr_poList(raptor_parser* rdf_parser, raptor_term* subject, raptor_sequence* poList); */
/* int mkr_statement_print(const raptor_statement* statement, FILE *stream); */
/* raptor_uri* raptor_new_uri_for_mkr_concept(raptor_world* world, const unsigned char *name); */
/* raptor_term_print_as_ntriples(raptor_term* t, stdout);   */
/* raptor_sequence_print(raptor_sequence* s, stdout);       */
/* raptor_uri_print(raptor_uri* u, stdout);                 */

/* raptor_uri* raptor_new_uri_from_rdf_ordinal(raptor_world* world, int ordinal) */


/************************************************************************************/
/************************************************************************************/
/* Turtle style */
/* sentence: subject predicateObjectList; */
raptor_statement*
mkr_poList(raptor_parser* rdf_parser, raptor_term* subject, raptor_sequence* poList)
{
  raptor_statement* triple = NULL;
  int i;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_poList: ");
  if(subject)
    raptor_term_print_as_ntriples(subject, stdout);
  else
    fputs("NULL", stdout);
  if(poList) {
    printf(" (reverse order to syntax) ");
    raptor_sequence_print(poList, stdout);
    printf("\n");
  } else     
    printf(" empty poList\n");
#endif

  if(subject && poList) {
    /* have subject and non-empty property list, handle it  */
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" after substitution predicateObjectList=");
    raptor_sequence_print(poList, stdout);
    printf("\n\n");
#endif

    for(i = 0; i < raptor_sequence_size(poList); i++) {
#if defined(RAPTOR_DEBUG)
      printf("##### DEBUG: mkr_poList: i=%d: \n");
#endif
      triple = (raptor_statement*)raptor_sequence_get_at(poList, i);
      triple->subject = raptor_term_copy(subject);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple);
    }
  }
  return triple;
}


/*****************************************************************************/
/*****************************************************************************/
/* prefixes */

/* sentence: BASE URI_LITERAL SEMICOLON */
/* base: "@base" | "@mkr" */
raptor_statement*
mkr_base(raptor_parser* rdf_parser, unsigned char* var1, raptor_uri* var2)
{
  raptor_statement* triple = NULL;
  unsigned char* basetype = var1;
  raptor_uri *uri = var2;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_base: %s ", var1);
  printf("%s <%s> ;\n", basetype, raptor_uri_as_string(var2));
#endif

  if(rdf_parser->base_uri)
    raptor_free_uri(rdf_parser->base_uri);
  rdf_parser->base_uri = uri;

  return triple;
}

/*****************************************************************************/
/* sentence: prefix IDENTIFIER URI_LITERAL SEMICOLON */
/* prefix: "@prefix" | "@mkb" | "@mke" */
raptor_statement*
mkr_prefix(raptor_parser* rdf_parser, unsigned char* var1, unsigned char* var2, raptor_uri* var3)
{
  raptor_statement* triple = NULL;
  raptor_turtle_parser* mkr_parser = (raptor_turtle_parser*)(rdf_parser->context);
  unsigned char* prefixtype = var1;
  unsigned char *prefix = var2;
  raptor_namespace *ns;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_prefix: ");
  printf("%s %s <%s> ;\n", prefixtype, prefix, raptor_uri_as_string(var3));
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
  } else {
    printf("##### ERROR: mkr_prefix: can't create new namespace for %s\n", (char*)prefix);
  }
  return triple;

}

/*****************************************************************************/
/*****************************************************************************/
/* sentence: viewOption nameOption sentence SEMICOLON */
/* sentence: view SEMICOLON */
raptor_statement*
mkr_sentence(raptor_parser* rdf_parser, mkr_type type, MKR_nv* contextOption, raptor_term* nameOption, raptor_statement* sentence)
{
#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_sentence: mkrtype = %s\n", MKR_TYPE_NAME(type));
  printf("in ");
  mkr_nv_print(contextOption, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(nameOption, stdout);
  printf(" :: ");
  raptor_term_print_as_ntriples(sentence, stdout);
  printf(" ;\n");
#endif

  printf("##### WARNING: mkr_sentence: not implemented\n");

  return sentence;
}


/*****************************************************************************/
/* doVerb predicate [= event] ; */
/* doVerb predicate [= event] ppList ; */
raptor_statement*
mkr_command(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* verb, raptor_term* predicate, raptor_sequence* pplist)
{
  raptor_statement* triple = NULL;
  raptor_term* command = predicate;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_command: ");
  raptor_term_print_as_ntriples(verb, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(predicate, stdout);
  printf(" ");
  raptor_sequence_print(pplist, stdout);
  printf(";\n");
#endif

  verb = raptor_term_copy(verb);
  command = raptor_term_copy(command);

  triple = mkr_pplist(rdf_parser, MKR_COMMAND, subject, verb, command, pplist);

  return triple;
}

/*****************************************************************************/
/* begin groupType groupName; */
/* end   groupType groupName; */
raptor_statement*
mkr_group(raptor_parser* rdf_parser, mkr_type type, raptor_term* gtype, raptor_term* gname)
{
  raptor_world* world = rdf_parser->world;
  raptor_turtle_parser* mkr_parser = (raptor_turtle_parser*)rdf_parser->context;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
  raptor_term* t4 = NULL;
  raptor_statement* triple = NULL;
  const char* event = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_group: %s ", MKR_TYPE_NAME(type));
  raptor_term_print_as_ntriples(gtype, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(gname, stdout);
  printf(";\n");
#endif

  mkr_parser->groupType = raptor_term_copy(gtype);
  mkr_parser->groupName = raptor_term_copy(gname);
  switch(type) {
    case MKR_BEGIN:
      event = "begin";
      t1 = raptor_term_copy(mkr_parser->groupName);
      t2 = raptor_term_copy(RAPTOR_RDF_type_term(world));
      t3 = raptor_term_copy(mkr_parser->groupType);
      triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple);
      mkr_parser->groupList = raptor_new_sequence((raptor_data_free_handler)raptor_free_term,
                                                  (raptor_data_print_handler)raptor_term_print_as_ntriples);
      break;

    case MKR_END:
      event = "end";
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
      printf("groupList = (");
      raptor_sequence_print(mkr_parser->groupList, stdout);  /* core dump ? */
      printf(")\n");
#endif

      t1 = raptor_term_copy(mkr_parser->groupName);
      if(raptor_term_equals(mkr_parser->groupType, mkr_new_variable(rdf_parser, MKR_VARIABLE, (unsigned char*)"hierarchy")))
        t2 = raptor_term_copy(mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"ho"));
      else if(raptor_term_equals(mkr_parser->groupType, mkr_new_variable(rdf_parser, MKR_VARIABLE, (unsigned char*)"relation")))
        t2 = raptor_term_copy(mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"rel"));
      else
        t2 = raptor_term_copy(mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"groupList"));
      t4 = raptor_term_copy(mkr_new_rdflist(rdf_parser, mkr_parser->groupList));
      triple = raptor_new_statement_from_nodes(world, t1, t2, t4, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple);

      if(mkr_parser->groupType) {
        raptor_free_term(mkr_parser->groupType);
        mkr_parser->groupType = NULL;
      }
      if(mkr_parser->groupName) {
        raptor_free_term(mkr_parser->groupName);
        mkr_parser->groupName = NULL;
      }
      if(mkr_parser->groupList) {
        /* raptor_free_sequence(mkr_parser->groupList);  core dump */
        mkr_parser->groupList = NULL;
      }
      break;
  }
#if defined(RAPTOR_DEBUG)
  printf("exit mkr_group\n");
#endif

  return triple;
}

/*****************************************************************************/
/* contextOption: IN gtype gname | %empty */
/* push contextStack */

MKR_nv*
mkr_context(raptor_parser* rdf_parser, mkr_type type, raptor_term* gtype, raptor_term* gname)
{
  raptor_world* world = rdf_parser->world;
  raptor_turtle_parser* mkr_parser = (raptor_turtle_parser*)rdf_parser->context;
  MKR_nv* nv = NULL;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
  raptor_term* t4 = NULL;
  raptor_statement* triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_context: ");
  mkr_nv_print(nv, stdout);
  printf(";\n");
#endif

  nv = mkr_new_nv(rdf_parser, MKR_NV, gtype, gname);
  mkr_parser->groupType = raptor_term_copy(gtype);
  mkr_parser->groupName = raptor_term_copy(gname);
  switch(type) {
    case MKR_BEGIN:
      /* push contextStack */
      t1 = raptor_term_copy(mkr_parser->groupName);
      t2 = raptor_term_copy(RAPTOR_RDF_type_term(world));
      t3 = raptor_term_copy(mkr_parser->groupType);
      triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple);
      mkr_parser->groupList = raptor_new_sequence((raptor_data_free_handler)mkr_free_nv,
                                                  (raptor_data_print_handler)mkr_nv_print);
      nv = mkr_new_nv(rdf_parser, MKR_NV, gtype, gname);
      raptor_sequence_push(mkr_parser->groupList, nv);
      break;

    case MKR_END:
      /* pop contextStack */
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
      printf("groupList = (");
      raptor_sequence_print(mkr_parser->groupList, stdout);  /* core dump ? */
      printf(")\n");
#endif

      t1 = raptor_term_copy(mkr_parser->groupName);
      if(raptor_term_equals(mkr_parser->groupType, mkr_new_variable(rdf_parser, MKR_VARIABLE, (unsigned char*)"hierarchy")))
        t2 = raptor_term_copy(mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"ho"));
      else if(raptor_term_equals(mkr_parser->groupType, mkr_new_variable(rdf_parser, MKR_VARIABLE, (unsigned char*)"relation")))
        t2 = raptor_term_copy(mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"rel"));
      else
        t2 = raptor_term_copy(mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"groupList"));
      t4 = raptor_term_copy(mkr_new_rdflist(rdf_parser, mkr_parser->groupList));
      triple = raptor_new_statement_from_nodes(world, t1, t2, t4, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple);

      if(mkr_parser->groupType) {
        raptor_free_term(mkr_parser->groupType);
        mkr_parser->groupType = NULL;
      }
      if(mkr_parser->groupName) {
        raptor_free_term(mkr_parser->groupName);
        mkr_parser->groupName = NULL;
      }
      if(mkr_parser->groupList) {
        /* raptor_free_sequence(mkr_parser->groupList);  core dump */
        mkr_parser->groupList = NULL;
      }
      break;
  }
#if defined(RAPTOR_DEBUG)
  printf("exit mkr_group\n");
#endif

  return nv;
}


/*****************************************************************************/
/* ho objectList; */
raptor_statement*
mkr_ho(raptor_parser* rdf_parser, raptor_term* hoterm, raptor_sequence* objectList)
{
  raptor_turtle_parser* mkr_parser = (raptor_turtle_parser*)rdf_parser->context;
  raptor_world* world = rdf_parser->world;
  raptor_term* hoList = NULL;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
  raptor_statement* triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_ho: ho ");
  raptor_sequence_print(objectList, stdout);
  printf(";\n");
#endif
  
  t2 = raptor_term_copy(hoterm);
  hoList = mkr_new_rdflist(rdf_parser, objectList);
  t3 = raptor_term_copy(hoList);
  raptor_sequence_push(mkr_parser->groupList, t3);

  triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("exit mkr_ho\n");
#endif
  return triple;
}


/* rel objectList; */
raptor_statement*
mkr_rel(raptor_parser* rdf_parser, raptor_term* relterm, raptor_sequence* objectList)
{
  raptor_turtle_parser* mkr_parser = (raptor_turtle_parser*)rdf_parser->context;
  raptor_world* world = rdf_parser->world;
  raptor_term* relList = NULL;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
  raptor_statement* triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_rel: rel ");
  raptor_sequence_print(objectList, stdout);
  printf(";\n");
#endif

  t2 = raptor_term_copy(relterm);
  relList = mkr_new_rdflist(rdf_parser, objectList);
  t3 = raptor_term_copy(relList);
  raptor_sequence_push(mkr_parser->groupList, t3);


  triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("exit mkr_rel\n");
#endif
  return triple;
}



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* Note: mKR option to specify event is not implemented        */
/***************************************************************/
/* type: MKR_DEFINITION | MKR_ACTION | MKR_COMMAND             */
/* sentence: subject isVerb genus   [= event] pplist SEMICOLON */
/*         | subject doVerb action  [= event] ppList SEMICOLON */
/*         |         doVerb command [= event] ppList SEMICOLON */
/************************** predicate **************************/
raptor_statement*
mkr_pplist(raptor_parser* rdf_parser, mkr_type type, raptor_term* subject, raptor_term* verb, raptor_term* predicate, raptor_sequence* pplist)
{
  raptor_world* world = rdf_parser->world;
  raptor_term* sub = NULL;
  raptor_term* event = NULL;
  raptor_term* nvevent = NULL;
  raptor_term* rdflist = NULL;
  int ppsize = 0;
  MKR_pp* pp = NULL;
  raptor_term* preposition = NULL;
  raptor_sequence* ppSequence = NULL;
  raptor_sequence* nvList = NULL;
  MKR_nv* nv = NULL;
  raptor_term* name = NULL;
  raptor_term* value = NULL;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
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
      printf("##### ERROR: mkr_pplist: unexpected verb mkrtype: %s\n", MKR_TYPE_NAME(predicate->mkrtype));
      return NULL;
      break;
  }
  if(!sub) {
    printf("##### ERROR: mkr_pplist: NULL subject\n");
    return NULL;
  }
  sub = raptor_term_copy(sub);

  event = mkr_new_blank_term(rdf_parser);
  event->mkrtype = type;

#if defined(RAPTOR_DEBUG)
  printf("sub = "); raptor_term_print_as_ntriples(sub, stdout); printf("\n");
  printf("event = "); raptor_term_print_as_ntriples(event, stdout); printf("\n");
#endif


  switch(type) {
    case MKR_DEFINITION:
      /* sub definition event .      */
      /* event genus predicate .     */
      /* event differentia nvevent . */
      /* nvevent name value .        */
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
      return NULL;
      break;
  }

/******************************************************************************/

  switch(type) {
    case MKR_DEFINITION:
      /* sub definition event .   */
      t1 = raptor_term_copy(sub);
      t2 = raptor_term_copy( mkr_new_variable(rdf_parser, MKR_DEFINITION, (unsigned char*)"definition") );
      if(!t2) {
        printf("##### ERROR: mkr_pplist: NULL 'definition' term\n");
        return NULL;
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

  nvevent = mkr_new_blank_term(rdf_parser);
  nvevent->mkrtype = type;

  switch(type) {
  case MKR_DEFINITION:
    /* event differentia nvevent . */
    t1 = raptor_term_copy(event);
    t2 = raptor_term_copy(mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"differentia"));
    t3 = raptor_term_copy(nvevent);
    triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
    raptor_mkr_generate_statement(rdf_parser, triple);
    raptor_free_statement(triple);
  
    /* nvevent name value . */
    ppsize = raptor_sequence_size(pplist);
    if(ppsize != 1)
      printf("##### WARNING: unexpected pplist size: %d\n", ppsize);
    pp = (MKR_pp*)raptor_sequence_get_at(pplist, 0);
    preposition = pp->ppPreposition;
    nvList = pp->ppSequence;
    for(j = 0; j < raptor_sequence_size(nvList); j++) {
      /* nvevent name value . */
      nv = (MKR_nv*)raptor_sequence_get_at(nvList, j);
#if defined(RAPTOR_DEBUG)
      printf("nv = "); mkr_nv_print(nv, stdout); printf("\n");
#endif
      name = nv->nvName;
      value = nv->nvValue;
      t1 = raptor_term_copy(nvevent);
      t2 = raptor_term_copy(name);
      t3 = raptor_term_copy(value);
      triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
      raptor_mkr_generate_statement(rdf_parser, triple);
      raptor_free_statement(triple); triple = NULL;
  
      if(nv)
        mkr_free_nv(nv); nv = NULL;
    }
/* need this for serializer
    if(nvevent)
      raptor_free_term(nvevent); nvevent = NULL;
*/
    break;
  
  case MKR_ACTION:
  case MKR_COMMAND:
    /* event pplist */
    for(i = 0; i < raptor_sequence_size(pplist); i++) {
      pp = (MKR_pp*)raptor_sequence_get_at(pplist, i);
#if defined(RAPTOR_DEBUG)
      printf("pp = "); mkr_pp_print(pp, stdout); printf("\n");
#endif
      preposition = pp->ppPreposition;
      ppSequence = pp->ppSequence;
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
            nv = (MKR_nv*)raptor_sequence_get_at(ppSequence, j);
#if defined(RAPTOR_DEBUG)
            printf("nv = "); mkr_nv_print(nv, stdout); printf("\n");
#endif
            name = nv->nvName;
            value = nv->nvValue;
            t1 = raptor_term_copy(nvevent);
            t2 = raptor_term_copy(name);
            t3 = raptor_term_copy(value);
            triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
            raptor_mkr_generate_statement(rdf_parser, triple);
            raptor_free_statement(triple); triple = NULL;
  
            if(nv)
              mkr_free_nv(nv); nv = NULL;
          }
/* need this for serializer
          if(nvevent)
            raptor_free_term(nvevent); nvevent = NULL;
*/
          break;
  
        default:
          printf("##### ERROR: mkr_pplist: not ppList type: %s\n", MKR_TYPE_NAME(pp->ppType));
          break;
      }
  
      if(pp)
        mkr_free_pp(pp); pp = NULL;
    }
    break;
  }

/**********************************************************/

/*
  if(sub)
    raptor_free_term(sub); sub = NULL;
  if(event)
    raptor_free_term(event); event = NULL;
  if(nvevent)
    raptor_free_term(nvevent); nvevent = NULL;
  if(genus)
    raptor_free_term(genus); genus = NULL;
  if(differentia)
    raptor_free_term(differentia); differentia = NULL;
*/

#if defined(RAPTOR_DEBUG)
  printf("exit mkr_pplist: \n");
#endif
  return triple;
}
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/



/*******************************************************************/
/* variable := value; */
raptor_statement*
mkr_assignment(raptor_parser* rdf_parser, raptor_term* variable, raptor_term* value)
{
  raptor_world* world = rdf_parser->world;
  raptor_term *t1 = NULL;
  raptor_term *t2 = NULL;
  raptor_term *t3 = NULL;
  raptor_statement* triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_assignment: ");
  raptor_term_print_as_ntriples(variable, stdout);
  printf(" := ");
  raptor_term_print_as_ntriples(value, stdout);
  printf(";\n");
#endif

  t1 = raptor_term_copy(variable);
  t2 = mkr_local_to_raptor_term(rdf_parser, (unsigned char*)":=");
  t3 = raptor_term_copy(value);
  triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
  raptor_mkr_generate_statement(rdf_parser, triple);
  raptor_free_statement(triple);

#if defined(RAPTOR_DEBUG)
  printf("exit mkr_asssignment: \n");
#endif

  return triple;
}

/*****************************************************************************/
/* subject is object; */
raptor_statement*
mkr_alias(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* verb, raptor_term* object)
{
  raptor_world* world = rdf_parser->world;
  raptor_statement* triple = NULL;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_alias: ");
  raptor_term_print_as_ntriples(subject, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(verb, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(object, stdout);
  printf(";\n");
#endif

  t1 = raptor_term_copy(subject);
  t2 = raptor_term_copy(verb);
  t3 = raptor_term_copy(object);
  triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);

  return triple;
}

/*****************************************************************************/
/* subject is object ppList; */
raptor_statement*
mkr_definition(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* verb, raptor_term* genus, raptor_sequence* pplist)
{
  raptor_statement* triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_definition: ");
  raptor_term_print_as_ntriples(subject, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(verb, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(genus, stdout);
  printf(" ");
  raptor_sequence_print(pplist, stdout);
  printf(";\n");
#endif

  subject = raptor_term_copy(subject);
  verb = raptor_term_copy(verb);
  genus = raptor_term_copy(genus);

  triple = mkr_pplist(rdf_parser, MKR_DEFINITION, subject, verb, genus, pplist);

  return triple;
}


/*****************************************************************************/
/* subject hoVerb object; */
raptor_statement*
mkr_hierarchy(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* predicate, raptor_term* object)
{
  raptor_world* world = rdf_parser->world;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
  raptor_statement* triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_hierarchy: ");
  raptor_term_print_as_ntriples(subject, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(predicate, stdout);
  printf(" ");
  raptor_term_print_as_ntriples((raptor_term*)object, stdout);
  printf(";\n");
#endif

  t1 = raptor_term_copy(subject);
  t2 = raptor_term_copy(predicate);
  t3 = raptor_term_copy(object);
  triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);

  return triple;
}


/*****************************************************************************/
/* sentence: subject hasVerb nv SEMICOLON */
/* hasVerb: HAS | HASPART | ISAPART */
raptor_statement*
mkr_attribute(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* verb, MKR_nv* nv)
{
  raptor_world* world = rdf_parser->world;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
  raptor_statement* triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_attribute: ");
  raptor_term_print_as_ntriples(subject, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(verb, stdout);
  printf(" ");
  mkr_nv_print(nv, stdout);
  printf(";\n");
#endif

  { /* nvList */
    raptor_term* predicate = nv->nvName;
    raptor_term* object = nv->nvValue;

    t1 = raptor_term_copy(subject);
    t2 = raptor_term_copy(predicate);
    t3 = raptor_term_copy(object);
    triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);

    if(nv)
      mkr_free_nv(nv);
  } /* nvList */

#if defined(RAPTOR_DEBUG)
  mkr_statement_print(triple, stdout);
#endif
  return triple;
}

/*****************************************************************************/
/* subject partVerb nv; */
raptor_statement*
mkr_part(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* verb, MKR_nv* nv)
{
  raptor_world* world = rdf_parser->world;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
  raptor_statement* triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_part: ");
  raptor_term_print_as_ntriples(subject, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(verb, stdout);
  printf(" ");
  mkr_nv_print(nv, stdout);
  printf(";\n");
#endif

  { /* nvList */
    raptor_term* predicate = nv->nvName;
    raptor_term* object = nv->nvValue;

    t1 = raptor_term_copy(subject);
    t2 = raptor_term_copy(predicate);
    t3 = raptor_term_copy(object);
    triple = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);

    if(nv)
      mkr_free_nv(nv);
  } /* nvList */

#if defined(RAPTOR_DEBUG)
  mkr_statement_print(triple, stdout);
#endif
  return triple;
}

/*****************************************************************************/
/* subject doVerb predicate [= event] ppList; */
raptor_statement*
mkr_action(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* verb, raptor_term* predicate, raptor_sequence* pplist)
{
  raptor_term* action = predicate;
  raptor_statement* triple = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_action: ");
  raptor_term_print_as_ntriples(subject, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(verb, stdout);
  printf(" ");
  raptor_term_print_as_ntriples(predicate, stdout);
  printf(" ");
  raptor_sequence_print(pplist, stdout);
  printf(";\n");
#endif

  subject = raptor_term_copy(subject);
  verb = raptor_term_copy(verb);
  action = raptor_term_copy(action);

  triple = mkr_pplist(rdf_parser, MKR_ACTION, subject, verb, action, pplist);

  return triple;
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

  fputs("))", stream);

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


    case MKR_RDFCOLLECTION:	return "MKR_RDFCOLLECTION";		break;
    case MKR_RDFALT:		return "MKR_RDFALT";			break;
    case MKR_RDFBAG:		return "MKR_RDFBAG";			break;
    case MKR_RDFSEQ:		return "MKR_RDFSEQ";			break;

    case MKR_RDFLIST:		return "MKR_RDFLIST";			break;
    case MKR_RDFSET:		return "MKR_RDFSET";			break;
    case MKR_RESULTSET:		return "MKR_RESULTSET";			break;


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
    case MKR_FI:		return "MKR_FI";			break;
    case MKR_AND:		return "MKR_AND";			break;
    case MKR_OR:		return "MKR_OR";			break;
    case MKR_XOR:		return "MKR_XOR";			break;
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
    case MKR_GRAPH:		return "MKR_GRAPH";			break;

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
    case MKR_EMPTYSET:		return "MKR_EMPTYSET";			break;
    case MKR_OBJECT:		return "MKR_OBJECT";			break;
    case MKR_OBJECTLIST:	return "MKR_OBJECTLIST";		break;
    case MKR_VALUELIST:		return "MKR_VALUELIST";			break;
    case MKR_SENTENCELIST:	return "MKR_SENTENCELIST";		break;
    case MKR_OBJECTSET:		return "MKR_OBJECTSET";			break;
    case MKR_VALUESET:		return "MKR_VALUESET";			break;
    case MKR_SENTENCESET:	return "MKR_SENTENCESET";		break;
  }
}



/************************************************************************************/
/************************************************************************************/
/************************************************************************************/

/* testing functions */



/*****************************************************************************/
/*****************************************************************************/
/* subject HAS predicateObjectList; */
raptor_statement*
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
raptor_statement*
mkr_blank_attribute(raptor_parser* rdf_parser, raptor_term* var1, raptor_term* var2, raptor_sequence* var3)
/* triples: blankNodePropertyList predicateObjectListOpt */
{
  raptor_statement* triple = NULL;
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
      triple = (raptor_statement*)raptor_sequence_get_at(var3, i);
      triple->subject = raptor_term_copy(var2);
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" after substitution predicateObjectListOpt=");
    raptor_sequence_print(var3, stdout);
    printf("\n\n");
#endif
    for(i = 0; i < raptor_sequence_size(var3); i++) {
      triple = (raptor_statement*)raptor_sequence_get_at(var3, i);
      raptor_mkr_generate_statement(rdf_parser, triple);
    }
  }

  return triple;
}

/*****************************************************************************/
/* subject predicate object; */
raptor_statement*
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

  return triple;
}
