/**
 * Copyright 2008-2010 Digital Bazaar, Inc.
 *
 * This file is part of librdfa.
 * 
 * librdfa is Free Software, and can be licensed under any of the
 * following three licenses:
 * 
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any 
 *      newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 * 
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 * 
 * See LICENSE-* at the top of this software distribution for more
 * information regarding the details of each license.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with librdfa. If not, see <http://www.gnu.org/licenses/>.
 *
 * The librdfa library is the Fastest RDFa Parser in the Universe. It is
 * a stream parser, meaning that it takes an XML data as input and spits
 * out RDF triples as it comes across them in the stream. Due to this
 * processing approach, librdfa has a very, very small memory footprint.
 * It is also very fast and can operate on hundreds of gigabytes of XML
 * data without breaking a sweat.
 *
 * Usage:
 *
 * rdfacontext* context = rdfa_create_context(base_uri);
 * context->callback_data = your_user_data;
 * rdfa_set_triple_handler(context, triple_function);
 * rdfa_set_buffer_filler(context, buffer_filler_function);
 * rdfa_parse(context);
 * rdfa_free_context(context);
 *
 * If you would like to get warnings/error triples from the processor graph:
 *
 * rdfa_set_issue_handler(context, triple_function);
 *
 * Usage if you need more control over when to fill rdfa's buffer:
 *
 * rdfacontext* context = rdfa_create_context(base_uri);
 * context->callback_data = your_user_data;
 * rdfa_set_triple_handler(context, triple_function);
 * int rval = rdfa_parse_start(context);
 * if(rval == RDFA_PARSE_SUCCESS)
 * {
 *    FILE* myfile = fopen("myfilename");
 *    size_t buf_len = 0;
 *    size_t read = 0;
 *    do
 *    {
 *       char* buf = rdfa_get_buffer(context, &buf_len);
 *       if(buf_len > 0)
 *       {
 *          // fill buffer here up to buf_len bytes from your input stream
 *          read = fread(buf, sizeof(char), buf_len, myfile);
 *       }
 *
 *       // parse the read data
 *       rdfa_parse_buffer(context, read);
 *    }
 *    while(read > 0);
 *    fclose(myfile);
 *
 *    rdfa_parse_end(context);
 * }
 * rdfa_free_context(context);
 *
 */
#ifndef _LIBRDFA_RDFA_H_
#define _LIBRDFA_RDFA_H_
#include <stdlib.h>
#include <libxml/SAX2.h>

/* Activate the stupid Windows DLL exporting mechanism if we're building for Windows */
#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#ifdef LIBRDFA_IN_RAPTOR
#include "raptor2.h"
#include "raptor_internal.h"
#endif /* LIBRDFA_IN_RAPTOR */

#ifdef __cplusplus
extern "C"
{
#endif

#define DEBUG 0

/* RDFa version numbers */
#define RDFA_VERSION_1_0 1
#define RDFA_VERSION_1_1 2

/* parse process return types */
#define RDFA_PARSE_WARNING -2
#define RDFA_PARSE_FAILED -1
#define RDFA_PARSE_UNKNOWN 0
#define RDFA_PARSE_SUCCESS 1

/* maximum list lengths */
#define MAX_LOCAL_LIST_MAPPINGS 32
#define MAX_LIST_MAPPINGS 48
#define MAX_LIST_ITEMS 16
#define MAX_TERM_MAPPINGS 64
#define MAX_URI_MAPPINGS 128
#define MAX_INCOMPLETE_TRIPLES 128

/* host language definitions */
#define HOST_LANGUAGE_NONE 0
#define HOST_LANGUAGE_XML1 1
#define HOST_LANGUAGE_XHTML1 2
#define HOST_LANGUAGE_HTML 3

/* default mapping key for xmlns */
#define XMLNS_DEFAULT_MAPPING "XMLNS_DEFAULT"

/* whitespace characters for RDFa Core 1.1 */
#define RDFA_WHITESPACE " \t\n\v\f\r"

/**
 * An RDF resource type is used to denote the content of a triple's
 * object value.
 */
typedef enum
{
   RDF_TYPE_NAMESPACE_PREFIX,
   RDF_TYPE_IRI,
   RDF_TYPE_PLAIN_LITERAL,
   RDF_TYPE_XML_LITERAL,
   RDF_TYPE_TYPED_LITERAL,
   RDF_TYPE_UNKNOWN
} rdfresource_t;

/**
 * An RDF triple is the result of an RDFa statement that contains, at
 * the very least, a subject, a predicate and an object. It is the
 * smallest, complete statement one can make in RDF.
 */
typedef struct rdftriple
{
   char* subject;
   char* predicate;
   char* object;
   rdfresource_t object_type;
   char* datatype;
   char* language;
} rdftriple;

/**
 * The specification for a callback that is capable of handling
 * triples. Produces a triple that must be freed once the application
 * is done with the object.
 */
typedef void (*triple_handler_fp)(rdftriple*, void*);

/**
 * The specification for a callback that is used to fill the input buffer
 * with data to parse.
 */
typedef size_t (*buffer_filler_fp)(char*, size_t, void*);

/**
 * An RDFA list item is used to hold each datum in an rdfa list. It
 * contains a list of flags as well as the data for the list member.
 */
typedef struct rdfalistitem
{
   unsigned char flags;
   void* data;
} rdfalistitem;

/**
 * An RDFa list is used to store multiple text strings that have a set
 * of attributes associated with them. These can be lists of CURIEs,
 * or lists of incomplete triples. The structure grows with use, but
 * cannot be shrunk.
 */
typedef struct rdfalist
{
   rdfalistitem** items;
   size_t num_items;
   size_t max_items;
   unsigned int user_data;
} rdfalist;

/**
 * The RDFa Parser structure is responsible for keeping track of the state of
 * the current RDFa parser. Things such as the default namespace,
 * CURIE mappings, and other context-specific
 */
typedef struct rdfacontext
{
   unsigned char rdfa_version;
   char* base;
   char* parent_subject;
   char* parent_object;
   char* default_vocabulary;
#ifndef LIBRDFA_IN_RAPTOR
   void** uri_mappings;
#endif
   void** term_mappings;
   void** list_mappings;
   void** local_list_mappings;
   rdfalist* incomplete_triples;
   rdfalist* local_incomplete_triples;
   char* language;
   unsigned char host_language;

   triple_handler_fp default_graph_triple_callback;
   buffer_filler_fp buffer_filler_callback;
   triple_handler_fp processor_graph_triple_callback;

   unsigned char recurse;
   unsigned char skip_element;
   char* new_subject;
   char* current_object_resource;

   char* about;
   char* typed_resource;
   char* resource;
   char* href;
   char* src;
   char* content;
   char* datatype;
   rdfalist* property;
   unsigned char inlist_present;
   unsigned char rel_present;
   unsigned char rev_present;
   char* plain_literal;
   size_t plain_literal_size;
   char* xml_literal;
   size_t xml_literal_size;

   void* callback_data;

   /* parse state */
   size_t bnode_count;
   char* underscore_colon_bnode_name;
   unsigned char xml_literal_namespaces_defined;
   unsigned char xml_literal_xml_lang_defined;
   size_t wb_allocated;
   char* working_buffer;
   size_t wb_position;
#ifdef LIBRDFA_IN_RAPTOR
   raptor_world *world;
   raptor_locator *locator;
   /* a pointer (in every context) to the error_handlers structure
    * held in the raptor_parser object */
   raptor_uri* base_uri;
   raptor_sax2* sax2;
   raptor_namespace_handler namespace_handler;
   void* namespace_handler_user_data;
   int raptor_rdfa_version; /* 10 or 11 or otherwise default */
#else
   xmlParserCtxtPtr parser;
#endif
   int done;
   rdfalist* context_stack;
   size_t wb_preread;
   int preread;
   int depth;
} rdfacontext;

/**
 * Creates an initial context for RDFa.
 *
 * @param base The base URI that should be used for the parser.
 *
 * @return a pointer to the base RDFa context, or NULL if memory
 *         allocation failed.
 */
DLLEXPORT rdfacontext* rdfa_create_context(const char* base);

/**
 * Sets the default graph triple handler for the application.
 *
 * @param context the base rdfa context for the application.
 * @param th the triple handler function.
 */
DLLEXPORT void rdfa_set_default_graph_triple_handler(
   rdfacontext* context, triple_handler_fp th);

/**
 * Sets the processor graph triple handler for the application.
 *
 * @param context the base rdfa context for the application.
 * @param th the triple handler function.
 */
DLLEXPORT void rdfa_set_processor_graph_triple_handler(
   rdfacontext* context, triple_handler_fp th);

/**
 * Sets the buffer filler for the application.
 *
 * @param context the base rdfa context for the application.
 * @param bf the buffer filler function.
 */
DLLEXPORT void rdfa_set_buffer_filler(
   rdfacontext* context, buffer_filler_fp bf);

/**
 * Starts processing given the base rdfa context.
 *
 * @param context the base rdfa context.
 *
 * @return RDFA_PARSE_SUCCESS if everything went well. RDFA_PARSE_FAILED
 *         if there was a fatal error and RDFA_PARSE_WARNING if there
 *         was a non-fatal error.
 */
DLLEXPORT int rdfa_parse(rdfacontext* context);

DLLEXPORT int rdfa_parse_start(rdfacontext* context);

DLLEXPORT int rdfa_parse_chunk(
   rdfacontext* context, char* data, size_t wblen, int done);

/**
 * Gets the input buffer for the given context so it can be filled with data.
 * A pointer to the buffer will be returned and the maximum number of bytes
 * that can be written to that buffer will be set to the blen parameter. Once
 * data has been written to the buffer, rdfa_parse_buffer() should be called.
 *
 * @param context the base rdfa context.
 * @param blen the variable to set to the buffer length.
 *
 * @return a pointer to the context's input buffer.
 */
DLLEXPORT char* rdfa_get_buffer(rdfacontext* context, size_t* blen);

/**
 * Informs the parser to attempt to parse more of the given context's input
 * buffer. To fill the input buffer with data, call rdfa_get_buffer().
 *
 * If any of the input buffer can be parsed, it will be. It is possible
 * that none of the data will be parsed, in which case this function will
 * still return RDFA_PARSE_SUCCESS. More data should be written to the input
 * buffer using rdfa_get_buffer() as it is made available to the application.
 * Once there is no more data to write, rdfa_parse_end() should be called.
 *
 * @param context the base rdfa context.
 * @param bytes the number of bytes written to the input buffer via the last
 *           call to rdfa_get_buffer(), a value of 0 will indicate that there
 *           is no more data to parse.
 *
 * @return RDFA_PARSE_SUCCESS if everything went well. RDFA_PARSE_FAILED
 *         if there was a fatal error and RDFA_PARSE_WARNING if there
 *         was a non-fatal error.
 */
DLLEXPORT int rdfa_parse_buffer(rdfacontext* context, size_t bytes);

DLLEXPORT void rdfa_parse_end(rdfacontext* context);

DLLEXPORT void rdfa_init_context(rdfacontext* context);

DLLEXPORT char* rdfa_iri_get_base(const char* iri);

/**
 * Destroys the given rdfa context by freeing all memory associated
 * with the context.
 *
 * @param context the rdfa context.
 */
DLLEXPORT void rdfa_free_context(rdfacontext* context);

#ifdef __cplusplus
}
#endif

#endif
