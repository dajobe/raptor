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
emit_literal(const char* literal, FILE *stream)
{
  fputs("<literal>", stream);
  fputs(literal, stream);
  fputs("</literal>", stream);
}


#if 0
static void
emit_function(const char* name, FILE *stream)
{
  int i;
  char c;
  
  fputs("<link linkend=\">", stream);
  for(i = 0; (c = name[i]); i++) {
    if(c == '_')
      c = '-';
    fputc(c, stream);
  }
  fputs("\"><function>", stream);
  fputs(name, stream);
  fputs("()</function></link>", stream);
}
#endif


static void
emit_header(const char* id, FILE *stream)
{
  fprintf(stream,
"<!DOCTYPE refentry PUBLIC \"-//OASIS//DTD DocBook XML V4.3//EN\" \n"
"               \"http://www.oasis-open.org/docbook/xml/4.3/docbookx.dtd\">\n"
"<chapter id=\"%s\">\n"
"<title>Syntax Formats supported in Raptor %s</title>\n"
"\n"
"<para>This chapter describes the syntax formats supported\n"
"by parsers and serializers in raptor %s\n"
"</para>\n"
"\n",
          id, raptor_version_string, raptor_version_string);
}


static void
emit_footer(FILE *stream) 
{
  fputs(
"</chapter>\n"
"\n"
"<!--\n"
"Local variables:\n"
"mode: sgml\n"
"sgml-parent-document: (\"raptor-docs.xml\" \"book\" \"part\")\n"
"End:\n"
"-->\n"
"\n",
  stream);
}

static void
emit_escaped_string(const char* string, FILE *stream)
{
  size_t len = strlen(string);
  unsigned char *buffer = NULL;
  int needed;
  
  needed = raptor_xml_escape_string(world, (const unsigned char*)string,
                                     len, NULL, 0, '\0');
  if(needed < 0)
    return;
  
  buffer = (unsigned char*)malloc(needed + 1);
  if(!buffer)
    return;
  
  (void)raptor_xml_escape_string(world, (const unsigned char*)string,
                                 len, buffer, (size_t)needed, '\0');

  fputs((const char*)buffer, stream);
  free(buffer);
}



static void
emit_start_section(const char* id, const char* title, FILE *stream)
{
  fprintf(stream,
"<section id=\"%s\">\n"
"<title>",
          id);
  emit_escaped_string(title, stream);
  fputs("</title>\n", stream);
}


static void
emit_end_section(FILE *stream) 
{
  fputs(
"</section>\n"
"\n",
  stream);
}


static void
emit_start_list(FILE *stream) 
{
  fputs("  <itemizedlist>\n", stream);
}


static void
emit_start_list_item(FILE *stream) 
{
  fputs("    <listitem><para>", stream);
}


static void
emit_end_list_item(FILE *stream) 
{
  fputs("</para></listitem>\n",  stream);
}


static void
emit_end_list(FILE *stream) 
{
  fputs("  </itemizedlist>\n",  stream);
}


static void
emit_start_desc_list(FILE *stream, const char* title)
{
  fprintf(stream,
"  <variablelist>\n"
"  <title>%s</title>\n",
          title);
}


static void
emit_start_desc_list_term(FILE *stream)
{
  fputs(
"    <varlistentry><term>",
stream);
}


static void
emit_start_desc_list_defn(FILE *stream) 
{
  fputs(
"</term>\n"
"      <listitem>",
          stream);
}


static void
emit_end_desc_list_item(FILE *stream) 
{
  fputs(
"      </listitem>\n"
"    </varlistentry>\n"
"\n",  stream);
}


static void
emit_end_desc_list(FILE *stream) 
{
  fputs("  </variablelist>\n",  stream);
}


static void
emit_mime_type(const raptor_type_q* mt, FILE* stream)
{
  fputs("type ", stream);
  emit_literal(mt->mime_type, stream);
  fputs(" with ", stream);
  if(mt->q < 10)
    fprintf(stream, "q 0.%d", (int)mt->q);
  else
    fputs("q 1.0", stream);
}


static void
emit_format_description(const char* type_name, 
                        const raptor_syntax_description* sd,
                        FILE* stream) 
{
  unsigned int i;
  
  if(!sd->mime_types_count)
    return;

  /* term */
  emit_start_desc_list_term(stream);
  emit_escaped_string(sd->label, stream);
  fputc(' ', stream);
  fputs(type_name, stream);
  fputs(" (name ", stream);
  emit_literal(sd->names[0], stream);
  fputc(')', stream);

  /* definition */
  emit_start_desc_list_defn(stream);
  fputs("\n    ", stream);
  emit_start_list(stream);
  
  for(i = 0; i < sd->mime_types_count; i++) {
    const raptor_type_q* mime_type = &sd->mime_types[i];
    if(!sd)
      break;
    fputs("    ", stream);
    emit_start_list_item(stream);
    emit_mime_type(mime_type, stream);
    emit_end_list_item(stream);
  }

  fputs("    ", stream);
  emit_end_list(stream);

  emit_end_desc_list_item(stream);
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
  

  emit_header("raptor-formats", stdout);

  emit_start_section("raptor-formats-types-by-parser",
                     "MIME Types by Parser",
                     stdout);
  emit_start_desc_list(stdout, "MIME Types by Parser");
  for(i = 0; i < parsers_count; i++) {
    emit_format_description("Parser", parsers[i],
                            stdout);
  }
  emit_end_desc_list(stdout);
  emit_end_section(stdout);


  emit_start_section("raptor-formats-types-by-serializer", 
                     "MIME Types by Serializer",
                     stdout);
  emit_start_desc_list(stdout, "MIME Types by Serializer");
  for(i = 0; i < serializers_count; i++) {
    emit_format_description("Serializer", serializers[i],
                            stdout);
  }
  emit_end_desc_list(stdout);
  emit_end_section(stdout);

  emit_footer(stdout);
            
  /* success */
  rc = 0;

  tidy:
  if(parsers)
    free(parsers);
  if(serializers)
    free(serializers);
  
  if(world)
    raptor_free_world(world);
  
  return rc;
}
