/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * mkrtype.h - Redland Parser Toolkit for RDF (Raptor) - public API
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


/**
 * mkr_type
  * @MKR_UNKNOWN:
  * @MKR_CHAR:
  * @MKR_STRING:
  * @MKR_INTEGER:
  * @MKR_REAL:
  * @MKR_BOOLEAN:
  * @MKR_GROUP:
    * @MKR_EXGROUP:     exclusive
      * @MKR_ALT:         ordered
      * @MKR_ENUM:        unordered
    * @MKR_INGROUP:     inclusive
      * @MKR_LIST:        ordered
      * @MKR_SET:         unordered
      * @MKR_MULTISET:    unordered, counted
      * @MKR_ARRAY:     key,value pairs
      * @MKR_HIERARCHY: isa,isu,iss relation
      * @MKR_RELATION:  list of values


   * @MKR_RDFLIST:     ordered (list with first,rest,nil)
   * @MKR_RDFSET:      unordered (set with first,rest,nil)
   * @MKR_RESULTSET:   rs:ResultSet

   * @MKR_RDFCOLLECTION:
   * @MKR_RDFALT:
   * @MKR_RDFBAG:
   * @MKR_RDFSEQ:


  * @MKR_NAME:
  * @MKR_VARIABLE:
    * @MKR_BVARIABLE:
    * @MKR_DVARIABLE:
    * @MKR_QVARIABLE:
  * @MKR_VALUE:
    * @MKR_NIL:            ( )
    * @MKR_EMPTYSET:       [ ]
    * @MKR_OBJECT:         object
    * @MKR_RDFLIST:        first,rest,nil
    * @MKR_VALUELIST:      [ valueList ]
    * @MKR_OBJECTLIST:     [ objectList ]
    * @MKR_SENTENCELIST:   { sentenceList }
    * @MKR_VALUESET:       ( valueList )
    * @MKR_OBJECTSET:      ( objectList )
    * @MKR_SENTENCESET:    { sentenceSet }

  * @MKR_WORD:
    * @MKR_PRONOUN:
      * @MKR_I:
      * @MKR_ME:
      * @MKR_YOU:
      * @MKR_HE:
      * @MKR_SHE:
      * @MKR_IT:
      * @MKR_HERE:
      * @MKR_NOW:
    * @MKR_PREPOSITION:
      * @MKR_AT:
      * @MKR_OF:
      * @MKR_WITH:
      * @MKR_OD:
      * @MKR_FROM:
      * @MKR_TO:
      * @MKR_IN:
      * @MKR_OUT:
      * @MKR_WHERE:
    * @MKR_QUANTIFIER:
      * @MKR_OPTIONAL:
      * @MKR_NO:
      * @MKR_ANY:
      * @MKR_SOME:
      * @MKR_MANY:
      * @MKR_ALL:
    * @MKR_CONJUNCTION:
      * @MKR_AND:
      * @MKR_OR:
      * @MKR_XOR:
      * @MKR_CAUSES:
      * @MKR_BECAUSE:
      * @MKR_IF
      * @MKR_THEN
      * @MKR_ELSE
      * @MKR_UNKNOWN
      * @MKR_FI
      * @MKR_IFF:
      * @MKR_IMPLIES:
    * @MKR_ITERATOR:
      * @MKR_EVERY:
      * @MKR_WHILE:
      * @MKR_UNTIL:
      * @MKR_WHEN:
      * @MKR_FORSOME:
      * @MKR_FORALL:
  * @MKR_PHRASE:
    * @MKR_NV:
    * @MKR_PP:
      * @MKR_PPOBJ: preposition objectList
      * @MKR_PPNV:  preposition nvList
  * @MKR_SUBJECT:
  * @MKR_VERB:
    * @MKR_ISVERB
      * @MKR_IS:
    * @MKR_HOVERB
      * @MKR_ISU:
      * @MKR_ISS:
      * @MKR_ISA:
      * @MKR_ISASTAR:
      * @MKR_ISP:
      * @MKR_ISG:
      * @MKR_ISC:
      * @MKR_ISCSTAR:
    * @MKR_HASVERB
      * @MKR_HAS:
      * @MKR_HASPART:
      * @MKR_ISAPART:
    * @MKR_DOVERB
    * @MKR_DOVERB
      * @MKR_DO:
      * @MKR_HDO:
      * @MKR_BANG:
  * @MKR_PREDICATE:
  * @MKR_OBJECT:

  * @MKR_CONCEPT:
  * @MKR_UNIT: bottom hierarchy concepts
  * @MKR_UNIVERSE: top hierarchy concept
    * @MKR_ENTITY:
    * @MKR_CHARACTERISTIC:
    * @MKR_PROPERTY:
      * @MKR_ATTRIBUTE:
      * @MKR_PART:
      * @MKR_ACTION:
        * @MKR_METHOD:
        * @MKR_AGENT: subject of action
    * MKR_SENTENCE:
      * @MKR_CONTEXT:
        * @MKR_SPACE:
        * @MKR_TIME:
        * @MKR_VIEW:
        * @MKR_GRAPH:
      * @MKR_STATEMENT:
        * @MKR_DEFINITION:
          * @MKR_GENUS:
          * @MKR_DIFFERENTIA:
      * @MKR_QUESTION:
      * @MKR_COMMAND:
      * @MKR_ASSIGNMENT:
      * @MKR_CONDITIONAL:
      * @MKR_ITERATION:
      * @MKR_BEGIN:
      * @MKR_END:
      * @MKR_HO: hierarchy unit
      * @MKR_REL: relation unit

  * @MKR_SUBJECTLIST:
  * @MKR_PHRASELIST:
  * @MKR_NVLIST:
  * @MKR_PPLIST:
  * @MKR_RAPTOR_TERM:
  * @MKR_RAPTOR_SEQUENCE:


 */
typedef enum {
  MKR_UNKNOWN,
  MKR_CHAR,
  MKR_STRING,
  MKR_INTEGER,
  MKR_REAL,
  MKR_BOOLEAN,
  MKR_GROUP,
    MKR_EXGROUP,
      MKR_ALT,
      MKR_ENUM,
    MKR_INGROUP,
      MKR_LIST,
      MKR_SET,
      MKR_MULTISET,
      MKR_ARRAY,
      MKR_HIERARCHY,
      MKR_RELATION,


  MKR_RDFALT,
  MKR_RDFBAG,
  MKR_RDFSEQ,
  MKR_RDFCOLLECTION,

  MKR_RDFLIST,
  MKR_RDFSET,
  MKR_RESULTSET,


  MKR_NAME,
  MKR_VARIABLE,
    MKR_BVARIABLE,
    MKR_DVARIABLE,
    MKR_QVARIABLE,
  MKR_VALUE,
    MKR_NIL,
    MKR_EMPTYSET,
    MKR_OBJECT,
    MKR_RDFLIST,
    MKR_VALUELIST,
    MKR_OBJECTLIST,
    MKR_RDFSET,
    MKR_VALUESET,
    MKR_OBJECTSET,
    MKR_RESULTSET,
    MKR_SENTENCELIST,
    MKR_SENTENCESET,
  MKR_WORD,
    MKR_PRONOUN,
      MKR_I,
      MKR_ME,
      MKR_YOU,
      MKR_HE,
      MKR_SHE,
      MKR_IT,
      MKR_HERE,
      MKR_NOW,
    MKR_PREPOSITION,
      MKR_AT,
      MKR_OF,
      MKR_WITH,
      MKR_OD,
      MKR_FROM,
      MKR_TO,
      MKR_IN,
      MKR_OUT,
      MKR_WHERE,
    MKR_QUANTIFIER,
      MKR_OPTIONAL,
      MKR_NO,
      MKR_ANY,
      MKR_SOME,
      MKR_MANY,
      MKR_ALL,
    MKR_CONJUNCTION,
      MKR_AND,
      MKR_OR,
      MKR_XOR,
      MKR_IF,
      MKR_THEN,
      MKR_ELSE,
      MKR_FI,
      MKR_IFF,
      MKR_IMPLIES,
      MKR_CAUSES,
      MKR_BECAUSE,
    MKR_ITERATOR,
      MKR_EVERY,
      MKR_WHILE,
      MKR_UNTIL,
      MKR_WHEN,
      MKR_FORSOME,
      MKR_FORALL,
  MKR_PHRASE,
    MKR_NV,
    MKR_PP,
      MKR_PPOBJ,
      MKR_PPNV,
  MKR_SUBJECT,
  MKR_VERB,
    MKR_ISVERB,
      MKR_IS,
    MKR_HOVERB,
      MKR_ISU,
      MKR_ISS,
      MKR_ISA,
      MKR_ISASTAR,
      MKR_ISP,
      MKR_ISG,
      MKR_ISC,
      MKR_ISCSTAR,
    MKR_HASVERB,
      MKR_HAS,
      MKR_HASPART,
      MKR_ISAPART,
    MKR_DOVERB,
      MKR_DO,
      MKR_HDO,
      MKR_BANG,
  MKR_PREDICATE,

  MKR_CONCEPT,
  MKR_UNIT,
  MKR_UNIVERSE,
    MKR_ENTITY,
    MKR_CHARACTERISTIC,
    MKR_PROPERTY,
      MKR_ATTRIBUTE,
      MKR_PART,
      MKR_ACTION,
        MKR_METHOD,
        MKR_AGENT,
    MKR_SENTENCE,
      MKR_CONTEXT,
        MKR_SPACE,
        MKR_TIME,
        MKR_VIEW,
        MKR_GRAPH,
      MKR_STATEMENT,
        MKR_DEFINITION,
          MKR_GENUS,
          MKR_DIFFERENTIA,
      MKR_QUESTION,
      MKR_COMMAND,
      MKR_ASSIGNMENT,
      MKR_CONDITIONAL,
      MKR_ITERATION,
      MKR_BEGIN,
      MKR_END,
      MKR_HO,
      MKR_REL,

  MKR_SUBJECTLIST,
  MKR_PHRASELIST,
  MKR_NVLIST,
  MKR_PPLIST,

  MKR_RAPTOR_TERM,
  MKR_RAPTOR_SEQUENCE

} mkr_type;

const char *MKR_TYPE_NAME(mkr_type n);

const char *RAPTOR_TERM_TYPE_NAME(raptor_term_type n);

typedef void *CONCEPT;

