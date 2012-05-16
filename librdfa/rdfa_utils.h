/**
 * Copyright 2008 Digital Bazaar, Inc.
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
 * This file contains functions used for common rdfa utility functions.
 */
#ifndef _RDFA_UTILS_H_
#define _RDFA_UTILS_H_
#include "rdfa.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * A CURIE type can be safe, unsafe, and Internationalized Resource
 * Identifier, reference-only or invalid.
 */
typedef enum
{
   CURIE_TYPE_SAFE,
   CURIE_TYPE_IRI_OR_UNSAFE,
   CURIE_TYPE_LINK_TYPE,
   CURIE_TYPE_INVALID
}  curie_t;

/**
 * A CURIE parse type lets the CURIE processor know what type of CURIE
 * is being parsed so that the proper namespace resolution may occur.
 */
typedef enum
{
   CURIE_PARSE_ABOUT_RESOURCE,
   CURIE_PARSE_PROPERTY,
   CURIE_PARSE_INSTANCEOF_DATATYPE,
   CURIE_PARSE_HREF_SRC,
   CURIE_PARSE_RELREV   
} curieparse_t;

/**
 * The list member flag type is used to attach attribute information
 * to list member data.
 */
typedef enum
{
   RDFALIST_FLAG_NONE = 0,
   RDFALIST_FLAG_DIR_NONE = (1 << 1),
   RDFALIST_FLAG_DIR_FORWARD  = (1 << 2),
   RDFALIST_FLAG_DIR_REVERSE = (1 << 3),
   RDFALIST_FLAG_TEXT = (1 << 4),
   RDFALIST_FLAG_CONTEXT = (1 << 5),
   RDFALIST_FLAG_TRIPLE = (1 << 6),
   RDFALIST_FLAG_LAST = (1 << 7)
} liflag_t;

/*
 *  RDFa processor graph reporting types
 */
#define RDFA_PROCESSOR_INFO "http://www.w3.org/ns/rdfa#Info"
#define RDFA_PROCESSOR_WARNING "http://www.w3.org/ns/rdfa#Warning"
#define RDFA_PROCESSOR_ERROR "http://www.w3.org/ns/rdfa#Error"

/* key establishing a deleted mapping entry */
#define RDFA_MAPPING_DELETED_KEY "<DELETED-KEY>"

/**
 * A function pointer that will be used to copy mapping values.
 */
typedef void* (*copy_mapping_value_fp)(void*, void*);

/**
 * A function pointer that will be used to update mapping values.
 */
typedef void* (*update_mapping_value_fp)(const void*, const void*);

/**
 * A function pointer that will be used to print mapping values.
 */
typedef void (*print_mapping_value_fp)(void*);

/**
 * A function pointer that will be used to free memory associated with values.
 */
typedef void (*free_mapping_value_fp)(void*);

/**
 * Initializes a mapping given the number of elements the mapping is
 * expected to hold.
 *
 * @param elements the maximum number of elements the mapping is
 *                 supposed to hold.
 *
 * @return an initialized void**, with all of the elements set to NULL.
 */
void** rdfa_create_mapping(size_t elements);

/**
 * Adds a list to a mapping given a key to create. The result will be a
 * zero-item list associated with the given key in the mapping.
 *
 * @param context the current active context.
 * @param mapping the mapping to modify.
 * @param subject the current active subject.
 * @param key the key to add to the mapping.
 * @param user_data the user-defined data to store with the list information.
 */
void rdfa_create_list_mapping(
   rdfacontext* context, void** mapping, const char* subject, const char* key);

/**
 * Adds an item to the end of the list that is associated with the given
 * key in the mapping.
 *
 * @param mapping the mapping to modify.
 * @param subject the current active subject.
 * @param key the key to use when looking up the list value.
 * @param value the value to append to the end of the list.
 */
void rdfa_append_to_list_mapping(
   void** mapping, const char* subject, const char* key, void* value);

/**
 * Gets the value for a given list mapping when presented with a subject
 * and a key. If the subject-key combo doesn't exist in the mapping,
 * NULL is returned.
 *
 * @param mapping the mapping to search.
 * @param subject the current active subject.
 * @param key the key.
 *
 * @return value the value in the mapping for the given key.
 */
const void* rdfa_get_list_mapping(
   void** mapping, const char* subject, const char* key);

/**
 * Copies the entire contents of a mapping verbatim and returns a
 * pointer to the copied mapping.
 *
 * @param mapping the mapping to copy
 *
 * @return the copied mapping, with all of the memory newly
 *         allocated. You MUST free the returned mapping when you are
 *         done with it.
 */
void** rdfa_copy_mapping(
   void** mapping, copy_mapping_value_fp copy_mapping_value);

/**
 * Updates the given mapping when presented with a key and a value. If
 * the key doesn't exist in the mapping, it is created.
 *
 * @param mapping the mapping to update.
 * @param key the key.
 * @param value the value.
 * @param replace_mapping_value a pointer to a function that will replace the
 *    old
 */
void rdfa_update_mapping(void** mapping, const char* key, const void* value,
   update_mapping_value_fp update_mapping_value);

/**
 * Gets the value for a given mapping when presented with a key. If
 * the key doesn't exist in the mapping, NULL is returned.
 *
 * @param mapping the mapping to search.
 * @param key the key.
 *
 * @return value the value in the mapping for the given key.
 */
const void* rdfa_get_mapping(void** mapping, const char* key);

/**
 * Gets the current mapping for the given mapping and increments the
 * mapping to the next value in the chain. 
 *
 * @param mapping the mapping to use and increment.
 * @param key the key that will be retrieved, NULL if the mapping is
 *            blank or you are at the end of the mapping.
 * @param value the value that is associated with the key. NULL if the
 *              mapping is blank or you are at the end of the mapping.
 */
void rdfa_next_mapping(void** mapping, char** key, void** value);

/**
 * Prints the mapping to the screen in a human-readable way.
 *
 * @param mapping the mapping to print to the screen.
 * @param print_value the function pointer to use to print the mapping values.
 */
void rdfa_print_mapping(void** mapping, print_mapping_value_fp print_value);

/**
 * Frees all memory associated with a mapping.
 *
 * @param mapping the mapping to free.
 * @param free_value the function to free mapping values.
 */
void rdfa_free_mapping(void** mapping, free_mapping_value_fp free_value);

/**
 * Creates a list and initializes it to the given size.
 *
 * @param size the starting size of the list.
 */
rdfalist* rdfa_create_list(size_t size);

/**
 * Copies the given list.
 *
 * @param list the list to copy.
 *
 * @return the copied list. You MUST free the memory associated with
 *         the returned list once you are done with it.
 */
rdfalist* rdfa_copy_list(rdfalist* list);

/**
 * Replaced the old_list by free'ing the memory associated with it. A
 * copy is made of the new list and then returned.
 *
 * @param old_list the list to replace. The memory associated with this list
 *                 is freed.
 * @param new_list the new list to copy in replacement of the old list. A
 *                 deep copy is performed on the new list.
 *
 * @return the copied list. You MUST free the memory associated with
 *         the returned list once you are done with it.
 */
rdfalist* rdfa_replace_list(rdfalist* old_list, rdfalist* new_list);

/**
 * Adds an item to the end of the list.
 *
 * @param list the list to add the item to.
 * @param data the data to add to the list.
 * @param flags the flags to attach to the item.
 */
void rdfa_add_item(rdfalist* list, void* data, liflag_t flags);

/**
 * Pushes an item onto the top of a stack. This function uses a list
 * for the underlying implementation.
 *
 * @param stack the stack to add the item to.
 * @param data the data to add to the stack.
 * @param flags the flags to attach to the item.
 */
void rdfa_push_item(rdfalist* stack, void* data, liflag_t flags);

/**
 * Pops an item off of the top of a stack. This function uses a list
 * for the underlying implementation 
 *
 * @param stack the stack to pop the item off of.
 *
 * @return the item that was just popped off of the top of the
 *         stack. You MUST free the memory associated with the return
 *         value.
 */
void* rdfa_pop_item(rdfalist* stack);

/**
 * Prints the list to the screen in a human-readable way.
 *
 * @param list the list to print to the screen.
 */
void rdfa_print_list(rdfalist* list);

/**
 * Frees all memory associated with the given list.
 *
 * @param list the list to free.
 */
void rdfa_free_list(rdfalist* list);

/**
 * Replaces an old string with a new string, freeing the old memory
 * and allocating new memory for the new string.
 *
 * @param old_string the old string to free and replace.
 * @param new_string the new string to copy to the old_string's
 *                   location.
 *
 * @return a pointer to the newly allocated string.
 */
char* rdfa_replace_string(char* old_string, const char* new_string);

/**
 * Appends a new string to the old string, expanding the old string's
 * memory area if needed. The old string's size must be provided and
 * will be updated to the new length.
 * 
 * @param old_string the old string to reallocate if needed.
 * @param string_size the old string's length, to be updated.
 * @param suffix the string to append to the old_string.
 * @param suffix_size the size of the suffix string.
 *
 * @return a pointer to the newly re-allocated string.
 */
char* rdfa_n_append_string(
   char* old_string, size_t* string_size,
   const char* suffix, size_t suffix_size);

/**
 * Joins two strings together and returns a newly allocated string
 * with both strings joined.
 *
 * @param prefix the beginning part of the string.
 * @param suffix the ending part of the string.
 *
 * @return a pointer to the newly allocated string that has both
 *         prefix and suffix in it.
 */
char* rdfa_join_string(const char* prefix, const char* suffix);

/**
 * Prints a string to stdout. This function is used by the rdfa_print_mapping
 * function.
 *
 * @param str the string to print to stdout.
 */
void rdfa_print_string(const char* str);

/**
 * Canonicalizes a given string by condensing all whitespace to single
 * spaces and stripping leading and trailing whitespace.
 *
 * @param str the string to canonicalize.
 *
 * @return a pointer to a newly allocated string that contains the
 *         canonicalized text.
 */
char* rdfa_canonicalize_string(const char* str);

/**
 * Creates a triple given the subject, predicate, object, datatype and
 * language for the triple.
 *
 * @param subject the subject for the triple.
 * @param predicate the predicate for the triple.
 * @param object the object for the triple.
 * @param object_type the type of the object, which must be an rdfresource_t.
 * @param datatype the datatype of the triple.
 * @param language the language for the triple.
 *
 * @return a newly allocated triple with all of the given
 *         information. This triple MUST be free()'d when you are done
 *         with it.
 */
rdftriple* rdfa_create_triple(const char* subject, const char* predicate,
   const char* object, rdfresource_t object_type, const char* datatype,
   const char* language);

/**
 * Prints a triple in a human-readable fashion.
 *
 * @triple the triple to display.
 */
void rdfa_print_triple(rdftriple* triple);

/**
 * Prints a list of triples in a human readable form.
 *
 * @triple the triple to display.
 */
void rdfa_print_triple_list(rdfalist* list);

/**
 * Frees the memory associated with a triple.
 */
void rdfa_free_triple(rdftriple* triple);

/**
 * Resolves a given uri by appending it to the context's base parameter.
 *
 * @param context the current processing context.
 * @param uri the URI part to process.
 *
 * @return the fully qualified IRI. The memory returned from this
 *         function MUST be freed.
 */
char* rdfa_resolve_uri(rdfacontext* context, const char* uri);

/**
 * Resolves a given uri depending on whether or not it is a fully
 * qualified IRI or a CURIE.
 *
 * @param context the current processing context.
 * @param uri the URI part to process.
 * @param mode the CURIE processing mode to use when parsing the CURIE.
 *
 * @return the fully qualified IRI. The memory returned from this
 *         function MUST be freed.
 */
char* rdfa_resolve_curie(
   rdfacontext* context, const char* uri, curieparse_t mode);

/**
 * Resolves one or more CURIEs into fully qualified IRIs.
 *
 * @param rdfa_context the current processing context.
 * @param uris a list of URIs.
 * @param mode the CURIE parsing mode to use, one of
 *             CURIE_PARSE_INSTANCEOF, CURIE_PARSE_RELREV, or
 *             CURIE_PARSE_PROPERTY.
 *
 * @return an RDFa list if one or more IRIs were generated, NULL if not.
 */
rdfalist* rdfa_resolve_curie_list(
   rdfacontext* rdfa_context, const char* uris, curieparse_t mode);

char* rdfa_resolve_relrev_curie(rdfacontext* context, const char* uri);

char* rdfa_resolve_property_curie(rdfacontext* context, const char* uri);

void rdfa_update_language(rdfacontext* context, const char* lang);

char* rdfa_create_bnode(rdfacontext* context);

/* All functions that rdfa.c needs. */
void rdfa_update_uri_mappings(rdfacontext* context, const char* attr, const char* value);
void rdfa_establish_new_1_0_subject(
   rdfacontext* context, const char* name, const char* about, const char* src,
   const char* resource, const char* href, const rdfalist* type_of);
void rdfa_establish_new_1_1_subject(
   rdfacontext* context, const char* name, const char* about, const char* src,
   const char* resource, const char* href, const rdfalist* type_of,
   const rdfalist* property, const char* content, const char* datatype);
void rdfa_establish_new_1_0_subject_with_relrev(
   rdfacontext* context, const char* name, const char* about, const char* src,
   const char* resource, const char* href, const rdfalist* type_of);
void rdfa_establish_new_1_1_subject_with_relrev(
   rdfacontext* context, const char* name, const char* about, const char* src,
   const char* resource, const char* href, const rdfalist* type_of);
void rdfa_complete_incomplete_triples(rdfacontext* context);
void rdfa_save_incomplete_list_triples(
   rdfacontext* context, const rdfalist* rel);
void rdfa_complete_type_triples(rdfacontext* context, const rdfalist* type_of);
void rdfa_complete_relrev_triples(
   rdfacontext* context, const rdfalist* rel, const rdfalist* rev);
void rdfa_save_incomplete_triples(
   rdfacontext* context, const rdfalist* rel, const rdfalist* rev);
void rdfa_complete_object_literal_triples(rdfacontext* context);
void rdfa_complete_current_property_value_triples(rdfacontext* context);

/* Declarations needed by namespace.c */
void rdfa_generate_namespace_triple(
   rdfacontext* context, const char* prefix, const char* iri);
void rdfa_processor_triples(
   rdfacontext* context, const char* type, const char* msg);

/* Declarations needed by rdfa.c */
void rdfa_setup_initial_context(rdfacontext* context);
void rdfa_establish_new_inlist_triples(
   rdfacontext* context, rdfalist* predicates, const char* object,
   rdfresource_t object_type);
void rdfa_complete_list_triples(rdfacontext* context);
rdfacontext* rdfa_create_new_element_context(rdfalist* context_stack);
void rdfa_free_context_stack(rdfacontext* context);

#ifdef __cplusplus
}
#endif

#endif
