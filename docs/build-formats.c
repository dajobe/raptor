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

#include <raptor.h>

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
"<title>Syntax Formats supported in Raptor ",
    iostr);
  raptor_iostream_string_write(raptor_version_string, iostr);
  raptor_iostream_string_write(
"</title>\n"
"\n"
"<para>This chapter describes the syntax formats supported\n"
"by parsers and serializers in raptor ",
    iostr);
  raptor_iostream_string_write(raptor_version_string, iostr);
  raptor_iostream_string_write(
"\n"
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
emit_mime_type(const raptor_type_q* mt, raptor_iostream* iostr)
{
  raptor_iostream_string_write("type ", iostr);
  emit_literal(mt->mime_type, iostr);
  raptor_iostream_string_write(" with ", iostr);
  if(mt->q < 10) {
    raptor_iostream_string_write("q 0.", iostr);
    raptor_iostream_decimal_write((int)mt->q, iostr);
  } else
    raptor_iostream_string_write("q 1.0", iostr);
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
  raptor_xml_escape_string_write((const unsigned char*)sd->label,
                                 strlen(sd->label),
                                 '\0', iostr);
  raptor_iostream_write_byte(' ', iostr);
  raptor_iostream_string_write(type_name, iostr);
  raptor_iostream_string_write(" (name ", iostr);
  emit_literal(sd->names[0], iostr);
  raptor_iostream_write_byte(')', iostr);

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


int
main(int argc, char *argv[]) 
{
  int rc = 1;
  int i;
  int parsers_count;
  int serializers_count;
  raptor_syntax_description** parsers = NULL;
  raptor_syntax_description** serializers = NULL;
  raptor_iostream* iostr = NULL;
  
  if(argc != 1) {
    fprintf(stderr, "%s: USAGE: %s\n", program, program);
    return 1;
  }

  world = raptor_new_world();
  if(!world)
    goto tidy;


  for(i = 0; 1; i++) {
    if(!raptor_world_get_parser_description(world, i))
      break;
    parsers_count++;
  }
  for(i = 0; 1; i++) {
    if(!raptor_world_get_serializer_description(world, i))
      break;
    serializers_count++;
  }

  parsers = (raptor_syntax_description**)calloc(parsers_count,
    sizeof(raptor_syntax_description*));
  if(!parsers)
    goto tidy;

  serializers = (raptor_syntax_description**)calloc(serializers_count,
    sizeof(raptor_syntax_description*));
  if(!serializers)
    goto tidy;

  for(i = 0; 1; i++) {
    raptor_syntax_description* sd;
    sd = (raptor_syntax_description*)raptor_world_get_parser_description(world, i);
    if(!sd)
      break;
    parsers[i] = sd;
  }

  qsort(parsers, parsers_count, sizeof(raptor_syntax_description*),
        sort_sd_by_name);
  
  for(i = 0; 1; i++) {
    raptor_syntax_description* sd;
    sd = (raptor_syntax_description*)raptor_world_get_serializer_description(world, i);
    if(!sd)
      break;
    serializers[i] = sd;
  }

  qsort(serializers, serializers_count, sizeof(raptor_syntax_description*),
        sort_sd_by_name);
  

  iostr = raptor_new_iostream_to_file_handle(world, stdout);
  if(!iostr)
    goto tidy;
  

  emit_header("raptor-formats", iostr);

  emit_start_section("raptor-formats-types-by-parser",
                     "MIME Types by Parser",
                     iostr);
  emit_start_desc_list("MIME Types by Parser", iostr);
  for(i = 0; i < parsers_count; i++) {
    emit_format_description("Parser", parsers[i],
                            iostr);
  }
  emit_end_desc_list(iostr);
  emit_end_section(iostr);


  emit_start_section("raptor-formats-types-by-serializer", 
                     "MIME Types by Serializer",
                     iostr);
  emit_start_desc_list("MIME Types by Serializer", iostr);
  for(i = 0; i < serializers_count; i++) {
    emit_format_description("Serializer", serializers[i],
                            iostr);
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
  
  if(world)
    raptor_free_world(world);
  
  return rc;
}
