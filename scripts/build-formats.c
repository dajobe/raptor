/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * build-formats.c - Helper to print raptor syntaxes into docbook xml
 *
 * Copyright (C) 2010, David Beckett http://www.dajobe.org/
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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <raptor2.h>

static const char * const program = "build-formats";

static raptor_world* world = NULL;



static void
emit_literal(const char* literal, raptor_iostream* iostr)
{
  raptor_iostream_string_write("<literal>", iostr);
  raptor_iostream_string_write(literal, iostr);
  raptor_iostream_string_write("</literal>", iostr);
}


#if 0
static void
emit_function(const char* name, raptor_iostream* iostr)
{
  int i;
  char c;
  
  raptor_iostream_string_write("<link linkend=\">", iostr);
  for(i = 0; (c = name[i]); i++) {
    if(c == '_')
      c = '-';
    raptor_iostream_write_byte(c, iostr);
  }
  raptor_iostream_string_write("\"><function>", iostr);
  raptor_iostream_string_write(name, iostr);
  raptor_iostream_string_write("()</function></link>", iostr);
}
#endif


static void
emit_header(const char* id, raptor_iostream* iostr)
{
  raptor_iostream_string_write(
"<!DOCTYPE refentry PUBLIC \"-//OASIS//DTD DocBook XML V4.3//EN\" \n"
"               \"http://www.oasis-open.org/docbook/xml/4.3/docbookx.dtd\">\n"
"<chapter id=\"",
    iostr);
  raptor_iostream_string_write(id, iostr);
  raptor_iostream_string_write(
"\">\n"
"<title>Syntax Formats supported in Raptor</title>\n"
"\n"
"<para>This chapter describes the syntax formats supported\n"
"by parsers and serializers in Raptor.\n"
"</para>\n"
"\n",
    iostr);
}


static void
emit_footer(raptor_iostream *iostr) 
{
  raptor_iostream_string_write(
"</chapter>\n"
"\n"
"<!--\n"
"Local variables:\n"
"mode: sgml\n"
"sgml-parent-document: (\"raptor-docs.xml\" \"book\" \"part\")\n"
"End:\n"
"-->\n"
"\n",
    iostr);
}


static void
emit_start_section(const char* id, const char* title, raptor_iostream* iostr)
{
  raptor_iostream_string_write("<section id=\"", iostr);
  raptor_iostream_string_write(id, iostr);
  raptor_iostream_string_write(
"\">\n"
"<title>",
    iostr);
  raptor_xml_escape_string_write((const unsigned char*)title, strlen(title),
                                 '\0', iostr);
  raptor_iostream_string_write("</title>\n", iostr);
}


static void
emit_end_section(raptor_iostream *iostr) 
{
  raptor_iostream_string_write(
"</section>\n"
"\n",
    iostr);
}


static void
emit_start_list(raptor_iostream *iostr) 
{
  raptor_iostream_string_write("  <itemizedlist>\n", iostr);
}


static void
emit_start_list_item(raptor_iostream *iostr) 
{
  raptor_iostream_string_write("    <listitem><para>", iostr);
}


static void
emit_end_list_item(raptor_iostream *iostr) 
{
  raptor_iostream_string_write("</para></listitem>\n", iostr);
}


static void
emit_end_list(raptor_iostream *iostr)
{
  raptor_iostream_string_write("  </itemizedlist>\n", iostr);
}


static void
emit_start_desc_list(const char* title, raptor_iostream *iostr)
{
  raptor_iostream_string_write(
    "  <variablelist>\n",
    iostr);
  if(title) {
    raptor_iostream_string_write(
"  <title>",
    iostr);
    raptor_iostream_string_write(title, iostr);
    raptor_iostream_string_write("</title>\n", iostr);
  }
  raptor_iostream_write_byte('\n', iostr);
}


static void
emit_start_desc_list_term(raptor_iostream *iostr)
{
  raptor_iostream_string_write(
"    <varlistentry><term>",
    iostr);
}


static void
emit_start_desc_list_defn(raptor_iostream *iostr) 
{
  raptor_iostream_string_write(
"</term>\n"
"      <listitem>",
    iostr);
}


static void
emit_end_desc_list_item(raptor_iostream *iostr) 
{
  raptor_iostream_string_write(
"      </listitem>\n"
"    </varlistentry>\n"
"\n",
    iostr);
}


static void
emit_end_desc_list(raptor_iostream *iostr) 
{
  raptor_iostream_string_write("  </variablelist>\n", iostr);
}


static void
emit_mime_type_name(const char *name, raptor_iostream* iostr)
{
  emit_literal(name, iostr);
}

static void
emit_mime_type_q(unsigned char q, raptor_iostream* iostr)
{
  if(q < 10) {
    raptor_iostream_string_write("q 0.", iostr);
    raptor_iostream_decimal_write((int)q, iostr);
  } else
    raptor_iostream_string_write("q 1.0", iostr);
}

static void
emit_mime_type(const raptor_type_q* mt, raptor_iostream* iostr)
{
  emit_mime_type_name(mt->mime_type, iostr);
  raptor_iostream_string_write(" with ", iostr);
  emit_mime_type_q(mt->q, iostr);
}


static void
emit_format_description_name(const char* type_name,
                             const raptor_syntax_description* sd,
                             raptor_iostream* iostr)
{
  raptor_xml_escape_string_write((const unsigned char*)sd->label,
                                 strlen(sd->label),
                                 '\0', iostr);
  if(type_name) {
    raptor_iostream_write_byte(' ', iostr);
    raptor_iostream_string_write(type_name, iostr);
  }
  raptor_iostream_string_write(" (", iostr);
  emit_literal(sd->names[0], iostr);
  raptor_iostream_write_byte(')', iostr);
}


static void
emit_format_description(const char* type_name,
                        const raptor_syntax_description* sd,
                        raptor_iostream* iostr)
{
  unsigned int i;
  
  if(!sd->mime_types_count)
    return;

  /* term */
  emit_start_desc_list_term(iostr);
  emit_format_description_name(type_name, sd, iostr);

  /* definition */
  emit_start_desc_list_defn(iostr);
  raptor_iostream_string_write("\n    ", iostr);

  emit_start_list(iostr);
  for(i = 0; i < sd->mime_types_count; i++) {
    const raptor_type_q* mime_type = &sd->mime_types[i];
    if(!sd)
      break;
    raptor_iostream_string_write("    ", iostr);
    emit_start_list_item(iostr);
    emit_mime_type(mime_type, iostr);
    emit_end_list_item(iostr);
  }
  raptor_iostream_string_write("    ", iostr);
  emit_end_list(iostr);

  emit_end_desc_list_item(iostr);
}


static int
sort_sd_by_name(const void *a, const void *b)
{
  raptor_syntax_description* sd_a = *(raptor_syntax_description**)a;
  raptor_syntax_description* sd_b = *(raptor_syntax_description**)b;

  return strcmp(sd_a->label, sd_b->label);
}


typedef struct 
{
  const char *mime_type;
  unsigned char q;
  raptor_syntax_description* parser_sd;
  raptor_syntax_description* serializer_sd;
} type_syntax;
  

static int
sort_type_syntax_by_mime_type(const void *a, const void *b)
{
  int rc;
  
  const char* mime_type_a = ((type_syntax*)a)->mime_type;
  const char* mime_type_b = ((type_syntax*)b)->mime_type;

  if(!mime_type_a || !mime_type_b) {
    if(!mime_type_a && !mime_type_b)
      return (int)(mime_type_b - mime_type_a);
    return (mime_type_a) ? 1 : -1;
  }
  
  rc = strcmp(mime_type_a, mime_type_b);
  if(rc)
    return rc;
  return ((type_syntax*)b)->q - ((type_syntax*)a)->q;
}



static void
emit_format_to_syntax_list(raptor_iostream* iostr,
                           type_syntax* type_syntaxes,
                           const char* mime_type,
                           int start, int end) 
{
  int i;
  int parser_seen = 0;
  int serializer_seen = 0;
  
  /* term */
  emit_start_desc_list_term(iostr);
  emit_mime_type_name(mime_type, iostr);
  
  /* definition */
  emit_start_desc_list_defn(iostr);
  raptor_iostream_string_write("\n    ", iostr);
  
  emit_start_list(iostr);
  for(i = start; i <= end; i++) {
    raptor_iostream_string_write("    ", iostr);
    emit_start_list_item(iostr);
    if(type_syntaxes[i].parser_sd) {
      emit_format_description_name("Parser",
                                   type_syntaxes[i].parser_sd,
                                   iostr);
      parser_seen++;
    } else {
      emit_format_description_name("Serializer",
                                   type_syntaxes[i].serializer_sd,
                                   iostr);
      serializer_seen++;
    }
    raptor_iostream_string_write(" with ", iostr);
    emit_mime_type_q(type_syntaxes[i].q, iostr);
    emit_end_list_item(iostr);
  }
  if(!parser_seen || !serializer_seen) {
    emit_start_list_item(iostr);
    if(!parser_seen)
      raptor_iostream_string_write("No parser.", iostr);
    else
      raptor_iostream_string_write("No serializer.", iostr);
    emit_end_list_item(iostr);
  }
  raptor_iostream_string_write("    ", iostr);
  emit_end_list(iostr);
  
  emit_end_desc_list_item(iostr);
}


int
main(int argc, char *argv[]) 
{
  int rc = 1;
  int i;
  int parsers_count = 0;
  int serializers_count = 0;
  int mime_types_count = 0;
  raptor_syntax_description** parsers = NULL;
  raptor_syntax_description** serializers = NULL;
  raptor_iostream* iostr = NULL;
  type_syntax* type_syntaxes = NULL;
  int type_syntaxes_count = 0;
  
  if(argc != 1) {
    fprintf(stderr, "%s: USAGE: %s\n", program, program);
    return 1;
  }

  world = raptor_new_world();
  if(!world)
    goto tidy;


  for(i = 0; 1; i++) {
    raptor_syntax_description* sd;
    sd = (raptor_syntax_description*)raptor_world_get_parser_description(world, i);
    if(!sd)
      break;
    parsers_count++;
    mime_types_count += sd->mime_types_count;
  }
  for(i = 0; 1; i++) {
    raptor_syntax_description* sd;
    sd = (raptor_syntax_description*)raptor_world_get_serializer_description(world, i);
    if(!sd)
      break;
    serializers_count++;
    mime_types_count += sd->mime_types_count;
  }

  parsers = (raptor_syntax_description**)calloc(parsers_count,
    sizeof(raptor_syntax_description*));
  if(!parsers)
    goto tidy;

  serializers = (raptor_syntax_description**)calloc(serializers_count,
    sizeof(raptor_syntax_description*));
  if(!serializers)
    goto tidy;

  type_syntaxes = (type_syntax*)calloc(mime_types_count,
    sizeof(type_syntax));
  if(!type_syntaxes)
    goto tidy;

  type_syntaxes_count = 0;

  for(i = 0; 1; i++) {
    raptor_syntax_description* sd;
    unsigned int m;
    
    sd = (raptor_syntax_description*)raptor_world_get_parser_description(world, i);
    if(!sd)
      break;
    parsers[i] = sd;

    for(m = 0; m < sd->mime_types_count; m++) {
      type_syntaxes[type_syntaxes_count].mime_type = sd->mime_types[m].mime_type;
      type_syntaxes[type_syntaxes_count].q = sd->mime_types[m].q;
      type_syntaxes[type_syntaxes_count].parser_sd = sd;
      type_syntaxes_count++;
    }
  }

  qsort(parsers, parsers_count, sizeof(raptor_syntax_description*),
        sort_sd_by_name);
  
  for(i = 0; 1; i++) {
    raptor_syntax_description* sd;
    unsigned int m;
    
    sd = (raptor_syntax_description*)raptor_world_get_serializer_description(world, i);
    if(!sd)
      break;
    serializers[i] = sd;

    for(m = 0; m < sd->mime_types_count; m++) {
      type_syntaxes[type_syntaxes_count].mime_type = sd->mime_types[m].mime_type;
      type_syntaxes[type_syntaxes_count].q = sd->mime_types[m].q;
      type_syntaxes[type_syntaxes_count].serializer_sd = sd;
      type_syntaxes_count++;
    }
  }

  qsort(serializers, serializers_count, sizeof(raptor_syntax_description*),
        sort_sd_by_name);
  

  iostr = raptor_new_iostream_to_file_handle(world, stdout);
  if(!iostr)
    goto tidy;
  

  /* MIME Types by parser */
  emit_header("raptor-formats", iostr);

  emit_start_section("raptor-formats-intro",
                     "Introduction",
                     iostr);
  raptor_iostream_string_write(
"<para>\n"
"The parsers and serializers in raptor can handle different MIME Types with different levels of quality (Q).  A Q of 1.0 indicates that the parser or serializer will be able to read or write the full format with high quality, and it should be the prefered parser or serializer for that mime type.  Lower Q values indicate either additional mime type support (for parsing) or less-preferred mime types (for serializing).  A serializer typically has just 1 mime type of Q 1.0; the preferred type."
"</para>\n"
,
    iostr);
  emit_end_section(iostr);

  emit_start_section("raptor-formats-types-by-parser",
                     "MIME Types by Parser",
                     iostr);
  emit_start_desc_list(NULL, iostr);
  for(i = 0; i < parsers_count; i++) {
    emit_format_description(NULL, parsers[i],
                            iostr);
  }
  emit_end_desc_list(iostr);
  emit_end_section(iostr);


  /* MIME Types by serializer */
  emit_start_section("raptor-formats-types-by-serializer", 
                     "MIME Types by Serializer",
                     iostr);
  emit_start_desc_list(NULL, iostr);
  for(i = 0; i < serializers_count; i++) {
    emit_format_description(NULL, serializers[i],
                            iostr);
  }
  emit_end_desc_list(iostr);
  emit_end_section(iostr);


  /* MIME Types index */
  qsort(type_syntaxes, type_syntaxes_count, sizeof(type_syntax),
        sort_type_syntax_by_mime_type);

  emit_start_section("raptor-formats-types-index", 
                     "MIME Types Index",
                     iostr);
  emit_start_desc_list(NULL, iostr);
  if(1) {
    const char* last_mime_type = NULL;
    int last_start_index = -1;
    for(i = 0; i < type_syntaxes_count; i++) {
      const char *this_mime_type = type_syntaxes[i].mime_type;
      
      if(last_start_index < 0) {
        last_mime_type = this_mime_type;
        last_start_index = i;
        continue;
      }
      /* continue if same mime type */
      if(!strcmp(last_mime_type, this_mime_type))
        continue;

      emit_format_to_syntax_list(iostr, type_syntaxes,
                                 last_mime_type, last_start_index, i-1);

      last_mime_type = type_syntaxes[i].mime_type;
      last_start_index = i;
    }

    emit_format_to_syntax_list(iostr, type_syntaxes,
                               last_mime_type, last_start_index, i-1);
  }
  emit_end_desc_list(iostr);
  emit_end_section(iostr);



  emit_footer(iostr);

  raptor_free_iostream(iostr);
  iostr = NULL;
            
  /* success */
  rc = 0;

  tidy:
  if(iostr)
    raptor_free_iostream(iostr);
  
  if(parsers)
    free(parsers);

  if(serializers)
    free(serializers);
  
  if(type_syntaxes)
    free(type_syntaxes);
  
  if(world)
    raptor_free_world(world);
  
  return rc;
}
