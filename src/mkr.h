/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * mkr.h - Redland Parser Toolkit for RDF (Raptor) - public API
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
 */


/* mkr_type  is defined in raptor2.h.in */
/* mkr_value is defined in raptor2.h.in */


/**
 * MKR_nv
 * @nvType: MKR_NV
 * @nvName: raptor_term*
 * @nvSeparator: string ("=")
 * @nvValue: raptor_term*
 * 
 * name value phrase
 */
typedef struct MKR_nv_s {
  mkr_type             nvType;
  raptor_term         *nvName;
  const unsigned char *nvSeparator;
  raptor_term         *nvValue;
} MKR_nv;

/**
 * MKR_pp
 * @ppType: MKR_PP
 * @ppPreposition: raptor_term*
 * @ppSequence: raptor_sequence*
 * 
 * preposition phrase
 */
typedef struct MKR_pp_s {
  mkr_type  ppType;
  raptor_term     *ppPreposition;
  raptor_sequence *ppSequence;
} MKR_pp;

/**
 * MKR_rdflist
 * @listType: MKR_RDFLIST
 * @unitType: mkr_type
 * @listHead: raptor_term*
 * 
 * list using first, rest, nil
 */
typedef struct MKR_rdflist_s {
  mkr_type     listType;
  mkr_type     unitType;
  raptor_term *listHead;
} MKR_rdflist;


/**
 * MKR_subject
 # @subjectType: MKR_SUB or MKR_SUBPP
 * @sub: raptor_term*
 * @pplist: raptor_sequence*
 *
 */
typedef struct MKR_subject_s {
  mkr_type         subjectType;
  raptor_term     *sub;
  raptor_sequence *pplist;
} MKR_subject;

/**
 * MKR_verb
 # @verbType: MKR_VERB or MKR_PREDICATE
 * @verbName: raptor_term*
 *
 */
typedef struct MKR_verb_s {
  mkr_type     verbType;
  raptor_term *verbName;
} MKR_verb;

/**
 * MKR_object
 # @objectType: MKR_OBJ or MKR_OBJNV or MKR_OBJPP
 * @obj: raptor_term*
 * @pplist: raptor_sequence*
 * @separator: string ("=")
 * @value: raptor_term*
 *
 */
typedef struct MKR_object_s {
  mkr_type         objectType;
  raptor_term      obj;
  raptor_sequence *pplist;
  unsigned char   *separator;
  raptor_term     *value;
} MKR_object;

/**
 * MKR_sentence
 # @sentenceType:
 # @subject:
 # @verb:
 # @object:
 *
 */
typedef struct MKR_sentence_s {
  mkr_type sentenceType;
  raptor_term     context;
  raptor_term    *name;
  MKR_subject    *subject;
  MKR_verb       *verb;
  MKR_object     *object;
} MKR_sentence;



typedef void* CONCEPT;

/**
 * MKR_union
  * @MKR_char:
  * @MKR_string:
  * @MKR_integer:
  * @MKR_real:
  * @MKR_boolean:
  * @MKR_group:
  * @MKR_alt:
  * @MKR_enum:
  * @MKR_list:
  * @MKR_set:
  * @MKR_multiset:
  * @MKR_array:
  * @MKR_hierarchy:
  * @MKR_relation:

  * @MKR_name:
  * @MKR_method:

  * @MKR_word:
    * @MKR_preposition:
    * @MKR_quantifier:
    * @MKR_conjunction:
    * @MKR_iterator:
  * @MKR_phrase:
    * @MKR_nv:
    * @MKR_pp:
  * @MKR_subbject:
  * @MKR_verb:
  * @MKR_predicate:
  * @MKR_object:
  * @MKR_sentence:
    * @MKR_context:
    * @MKR_statement:
    * @MKR_question:
    * @MKR_command:
    * @MKR_assignment:
    * @MKR_conditional:
    * @MKR_iteration:

  * @MKR_entity:
  * @MKR_attribute:
  * @MKR_part:
  * @MKR_action:
  * @MKR_space:
  * @MKR_time:
  * @MKR_view:

  * @MKR_subjectList:
  * @MKR_objectList:
  * @MKR_nvList:
  * @MKR_ppList:
  * @MKR_sentenceList:

  * @MKR_raptor_term:
  * @MKR_raptor_sequence:
 *
 */
typedef union MKR_union_s {
  char			MKR_char;
  char*			MKR_string;
  int			MKR_integer;
  float			MKR_real;
  int			MKR_boolean;
  CONCEPT*		MKR_group;
  CONCEPT*		MKR_alt;
  int			MKR_enum;
  CONCEPT*		MKR_list;
  struct MKR_rdflist_s	MKR_rdflist;
  CONCEPT*		MKR_set;
  CONCEPT*		MKR_multiset;
  CONCEPT*		MKR_array;
  CONCEPT*		MKR_hierarchy;
  CONCEPT*		MKR_relation;

  char*			MKR_name;

  char*			MKR_word;
    char*		  MKR_preposition;
    char*		  MKR_quantifier;
    char*		  MKR_conjunction;
    char*		  MKR_iterator;
  CONCEPT*		MKR_phrase;
    CONCEPT*		  MKR_wordphrase;
    struct MKR_nv_s       MKR_nv;
    struct MKR_pp_s       MKR_pp;
  struct MKR_subject_s	MKR_subject;
  struct MKR_verb_s	MKR_verb;
  raptor_term		MKR_predicate;
  struct MKR_object_s	MKR_object;

  CONCEPT*		MKR_existent;
    CONCEPT*		MKR_entity;
    CONCEPT*		MKR_chararcteristic;
      CONCEPT*		  MKR_attribute;
      CONCEPT*		  MKR_part;
      CONCEPT*		  MKR_action;
      CONCEPT*		    MKR_method;
    CONCEPT*		MKR_sentence;
      CONCEPT*		  MKR_context;
        CONCEPT*		    MKR_space;
        CONCEPT*		    MKR_time;
        CONCEPT*		    MKR_view;
      CONCEPT*		  MKR_statement;
      CONCEPT*		  MKR_question;
      CONCEPT*		  MKR_command;
      CONCEPT*		  MKR_assignment;
      CONCEPT*		  MKR_conditional;
      CONCEPT*		  MKR_iteration;

  CONCEPT*		MKR_subjectList;
  CONCEPT*		MKR_objectList;
  CONCEPT*		MKR_nvList;
  CONCEPT*		MKR_ppList;
  CONCEPT*		MKR_sentenceList;

  raptor_term		MKR_raptor_term;
  CONCEPT*		MKR_raptor_sequence;

} MKR_union;



/* Prototypes for mKR classes */

raptor_term* mkr_new_list(raptor_parser *parser, raptor_sequence* objectList);
raptor_term* mkr_new_set(raptor_parser *parser, raptor_sequence* objectList);
raptor_term* mkr_new_view(raptor_parser *parser, raptor_sequence* sentenceList);
raptor_term* mkr_new_graph(raptor_parser *parser, raptor_sequence* sentenceList);

raptor_term* mkr_new_variable(raptor_parser *parser, mkr_type type, unsigned char* name);
raptor_term* mkr_new_preposition(raptor_parser *parser, mkr_type type, unsigned char* name);
raptor_term* mkr_new_verb(raptor_parser *parser, mkr_type type, unsigned char* name);
raptor_term* mkr_new_pronoun(raptor_parser *parser, mkr_type type, unsigned char* name);
raptor_term* mkr_new_conjunction(raptor_parser *parser, mkr_type type, unsigned char* name);
raptor_term* mkr_new_quantifier(raptor_parser *parser, mkr_type type, unsigned char* name);
raptor_term* mkr_new_iterator(raptor_parser *parser, mkr_type type, unsigned char* name);

MKR_nv* mkr_new_nv(raptor_parser* rdf_parser, mkr_type type, raptor_term* predicate, raptor_term* object);
void mkr_free_nv(MKR_nv* nv);
void mkr_nv_print(MKR_nv* value, FILE* fh);

MKR_pp* mkr_new_pp(raptor_parser* rdf_parser, mkr_type type, raptor_term* preposition, raptor_sequence* value);
void mkr_free_pp(MKR_pp* pp);
void mkr_pp_print(MKR_pp* value, FILE* fh);

raptor_term* mkr_new_rdflist(raptor_parser* rdf_parser, raptor_sequence* seq);
void mkr_free_rdflist(raptor_term* rdflist);
void mkr_rdflist_print(raptor_term* rdflist, FILE* fh);

raptor_term* mkr_new_rdfset(raptor_parser* rdf_parser, raptor_sequence* seq);
void mkr_free_rdfset(raptor_term* rdfset);
void mkr_rdfset_print(raptor_term* rdfset, FILE* fh);

/*
raptor_sequence* mkr_new_sequence_from_rdflist(raptor_serializer* serializer,
                                               raptor_abbrev_subject* subject,
                                               int depth);
*/

raptor_term* mkr_new_sentence(raptor_parser* rdf_parser, raptor_statement* triple);



/* Prototypes for mKR sentence functions - see mkr_sentence.c */

raptor_statement* mkr_sentence(raptor_parser* rdf_parser, mkr_type type, MKR_nv* contextOption, raptor_term* nameOption, raptor_term* sentence);
raptor_statement* mkr_base(raptor_parser* rdf_parser, raptor_uri* var2);
raptor_statement* mkr_prefix(raptor_parser* rdf_parser, unsigned char* var1, unsigned char* prefix, raptor_uri* uri);
raptor_statement* mkr_group(raptor_parser* rdf_parser, mkr_type type, raptor_term* gtype, raptor_term* gname);
MKR_nv* mkr_context(raptor_parser* rdf_parser, mkr_type type, raptor_term* gtype, raptor_term* gname);
raptor_sequence* mkr_ho(raptor_parser* rdf_parser, raptor_sequence* objectList);
raptor_sequence* mkr_rel(raptor_parser* rdf_parser, raptor_sequence* objectList);
raptor_statement* mkr_pplist(raptor_parser* rdf_parser, mkr_type type, raptor_term* subject, raptor_term* verb, raptor_term* predicate, raptor_sequence* pplist);
raptor_statement* mkr_command(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* verb, raptor_term* command, raptor_sequence* pplist);
raptor_statement* mkr_assignment(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* object);

raptor_statement* mkr_poList(raptor_parser* rdf_parser, raptor_term* var1, raptor_sequence* var2);
raptor_statement* mkr_attribute(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* verb, MKR_nv* nv);
raptor_statement* mkr_part(raptor_parser* rdf_parser,      raptor_term* subject, raptor_term* verb, MKR_nv* nv);
raptor_statement* mkr_hierarchy(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* verb, raptor_term* object);
raptor_statement* mkr_alias(raptor_parser* rdf_parser,     raptor_term* subject, raptor_term* verb, raptor_term* object);

raptor_statement* mkr_action(raptor_parser* rdf_parser,     raptor_term* subject, raptor_term* verb, raptor_term* action, raptor_sequence* pplist);
raptor_statement* mkr_definition(raptor_parser* rdf_parser, raptor_term* subject, raptor_term* verb, raptor_term* genus, raptor_sequence* pplist);
void mkr_generate_type_statement(raptor_parser* rdf_parser, raptor_term* concept, raptor_term* genus);

void raptor_mkr_generate_statement(raptor_parser *parser, raptor_statement *triple);
void raptor_mkr_clone_statement(raptor_parser *parser, raptor_statement *triple);
void raptor_mkr_handle_statement(raptor_parser *parser, raptor_statement *triple);
void raptor_mkr_defer_statement(raptor_parser *parser, raptor_statement *triple);

int mkr_statement_print(const raptor_statement* statement, FILE *stream);
raptor_term* mkr_local_to_raptor_term(raptor_parser* rdf_parser, unsigned char* local);
raptor_term* rdf_local_to_raptor_term(raptor_parser* rdf_parser, unsigned char* local);
raptor_term* mkr_new_blank_term(raptor_parser* rdf_parser);
raptor_term* mkr_new_integer_term(raptor_world* world, int ordinal);
raptor_term* mkr_new_index_term(raptor_world* world, int ordinal);
raptor_uri* raptor_mkr_get_graph(raptor_parser* rdf_parser);
int raptorstring_to_csvstring(unsigned char *string, unsigned char *csvstr);
