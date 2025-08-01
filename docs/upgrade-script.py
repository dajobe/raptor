#!/usr/bin/env python3
#
# Python script to upgrade raptor code between versions
#
# USAGE:
#   upgrade-script.py [FILES...]
#
# This script applies transformation rules to upgrade raptor code
# from older versions to newer versions.
#
# Copyright (C) 2025, David Beckett https://www.dajobe.org/
#
# This package is Free Software and part of Redland https://librdf.org/
#
# It is licensed under the following three licenses as alternatives:
#   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
#   2. GNU General Public License (GPL) V2 or any newer version
#   3. Apache License, V2.0 or any newer version
#
# You may not use this file except in compliance with at least one of
# the above three licenses.
#
# See LICENSE.html or LICENSE.txt at the top of this package for the
# complete terms and further detail along with the license texts for
# the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
#

import argparse
import re
import shutil
import sys
from pathlib import Path
from typing import List, Tuple


def apply_transformations(content: str) -> str:
    """Apply all transformation rules to the content."""
    
    # Replace statement fields with term fields (order matters!)
    transformations = [
        # Statement field transformations - specific ones first
        (r'->object_literal_datatype', r'->object.value.literal.datatype'),
        (r'->object_literal_language', r'->object.value.literal.language'),
        (r'->object_type', r'->object.type'),
        (r'->subject_type', r'->subject.type'),
        (r'->predicate_type', r'->predicate.type'),
        # General field replacements - these must come after the specific ones
        (r'->subject(?![\.\w])', r'->subject.value.uri or subject.value.blank.string /* WARNING: must choose one */'),
        (r'->predicate(?![\.\w])', r'->predicate.value.uri'),
        (r'->object(?![\.\w])', r'->object.value.uri or object.value.literal.string or object.value.blank.string /* WARNING: must choose one */'),
    ]
    
    # Deleted functions - comment out with warnings
    deleted_functions = [
        ('raptor_compare_strings', 'Trivial utility function removed.'),
        ('raptor_copy_identifier', 'Use raptor_term_copy() with #raptor_term objects.'),
        ('raptor_error_handlers_init', 'Replaced by generic raptor log mechanism.  See raptor_world_set_log_handler()'),
        ('raptor_error_handlers_init_v2', 'Replaced by generic raptor log mechanism.  See raptor_world_set_log_handler()'),
        ('raptor_feature_value_type', 'Use raptor_world_get_option_description() for the option and access the value_type field.'),
        ('raptor_finish', 'Use raptor_free_world() with #raptor_world object.'),
        ('raptor_free_identifier', 'Use raptor_free_term() with #raptor_term objects.'),
        ('raptor_init', 'Use raptor_new_world() to create a new #raptor_world object.'),
        ('raptor_iostream_get_bytes_written_count', 'Deprecated for raptor_iostream_tell().'),
        ('raptor_iostream_write_string_turtle', 'Deprecated for raptor_string_python_write().'),
        ('raptor_new_identifier', 'Replaced by raptor_new_term_from_blank(), raptor_new_term_from_literal() or raptor_new_term_from_blank() and #raptor_term class.'),
        ('raptor_new_identifier_v2', 'Replaced by raptor_new_term_from_blank(), raptor_new_term_from_literal() or raptor_new_term_from_blank() and #raptor_term class.'),
        ('raptor_ntriples_string_as_utf8_string', 'Deprecated internal debug function.'),
        ('raptor_ntriples_term_as_string', 'Deprecated internal debug function.'),
        ('raptor_print_ntriples_string', 'Use raptor_string_ntriples_write() with a #raptor_iostream'),
        ('raptor_print_statement_detailed', 'Deprecated internal function.'),
        ('raptor_sequence_print_string', 'Trivial utility function removed.'),
        ('raptor_sequence_print_uri', 'Deprecated for raptor_uri_print()'),
        ('raptor_sequence_set_print_handler', 'Use the argument in the raptor_new_sequence() constructor instead.'),
        ('raptor_sequence_set_print_handler_v2', 'Use the argument in the raptor_new_sequence() constructor instead.'),
        ('raptor_serializer_set_error_handler', 'Replaced by raptor_world_set_log_handler() on the #raptor_world object.'),
        ('raptor_serializer_set_warning_handler', 'Replaced by raptor_world_set_log_handler() on the #raptor_world object.'),
        ('raptor_set_error_handler', 'Replaced by raptor_world_set_log_handler() on the #raptor_world object.'),
        ('raptor_set_fatal_error_handler', 'Replaced by raptor_world_set_log_handler() on the #raptor_world object.'),
        ('raptor_set_parser_strict', 'Replaced by raptor_parser_set_option() with option RAPTOR_OPTION_STRICT'),
        ('raptor_set_warning_handler', 'Replaced by raptor_world_set_log_handler() on the #raptor_world object.'),
        ('raptor_statement_part_as_counted_string', 'Better done via methods of #raptor_term class such as raptor_term_to_counted_string().'),
        ('raptor_statement_part_as_counted_string_v2', 'Better done via methods of #raptor_term class such as raptor_term_to_counted_string().'),
        ('raptor_statement_part_as_string', 'Better done via methods of #raptor_term class such as raptor_term_to_string().'),
        ('raptor_statement_part_as_string_v2', 'Better done via methods of #raptor_term class such as raptor_term_to_string().'),
        ('raptor_uri_get_handler', 'Entire URI implementation is internal and not replaceable.'),
        ('raptor_uri_get_handler_v2', 'Entire URI implementation is internal and not replaceable.'),
        ('raptor_uri_set_handler', 'Entire URI implementation is internal and not replaceable.'),
        ('raptor_uri_set_handler_v2', 'Entire URI implementation is internal and not replaceable.'),
        ('raptor_www_finish', 'No need for this to be public.'),
        ('raptor_www_finish_v2', 'No need for this to be public.'),
        ('raptor_www_init', 'No need for this to be public.'),
        ('raptor_www_init_v2', 'No need for this to be public.'),
        ('raptor_www_no_www_library_init_finish', 'Deprecated for raptor_world_set_flag().'),
        ('raptor_www_no_www_library_init_finish_v2', 'Deprecated for raptor_world_set_flag().'),
        ('raptor_www_set_error_handler', 'Replaced by raptor_world_set_log_handler() on the #raptor_world object.'),
        ('raptor_www_set_user_agent', 'Deprecated for raptor_www_set_user_agent2'),
        ('raptor_www_set_proxy', 'Deprecated for raptor_www_set_proxy2'),
        ('raptor_www_set_http_accept', 'Deprecated for raptor_www_set_http_accept2'),
    ]
    
    # Add function renaming rules
    function_renames = [
        ('raptor_format_locator\\(', 'raptor_locator_format('),
        ('raptor_get_feature_count\\(', 'raptor_option_get_count('),
        ('raptor_get_locator\\(', 'raptor_parser_get_locator('),
        ('raptor_get_name\\(', 'raptor_parser_get_name('),
        ('raptor_guess_parser_name_v2\\(', 'raptor_world_guess_parser_name('),
        ('raptor_namespace_copy\\(', 'raptor_namespace_stack_start_namespace('),
        ('raptor_namespaces_format\\(', 'raptor_namespace_format_as_xml('),
        ('raptor_namespaces_qname_from_uri\\(', 'raptor_new_qname_from_namespace_uri('),
        ('raptor_new_namespace_parts_from_string\\(', 'raptor_xml_namespace_string_parse('),
        ('raptor_new_parser_for_content_v2\\(', 'raptor_new_parser_for_content('),
        ('raptor_new_parser_v2\\(', 'raptor_new_parser('),
        ('raptor_new_qname_from_namespace_local_name_v2\\(', 'raptor_new_qname_from_namespace_local_name('),
        ('raptor_new_serializer_v2\\(', 'raptor_new_serializer('),
        ('raptor_new_uri_from_id_v2\\(', 'raptor_new_uri_from_id('),
        ('raptor_new_uri_from_uri_local_name_v2\\(', 'raptor_new_uri_from_uri_local_name('),
        ('raptor_new_uri_relative_to_base_v2\\(', 'raptor_new_uri_relative_to_base('),
        ('raptor_new_uri_v2\\(', 'raptor_new_uri('),
        ('raptor_parse_abort\\(', 'raptor_parser_parse_abort('),
        ('raptor_parse_chunk\\(', 'raptor_parser_parse_chunk('),
        ('raptor_parse_file\\(', 'raptor_parser_parse_file('),
        ('raptor_parse_file_stream\\(', 'raptor_parser_parse_file_stream('),
        ('raptor_parse_uri\\(', 'raptor_parser_parse_uri('),
        ('raptor_parse_uri_with_connection\\(', 'raptor_parser_parse_uri_with_connection('),
        ('raptor_serialize_end\\(', 'raptor_serializer_serialize_end('),
        ('raptor_serialize_set_namespace\\(', 'raptor_serializer_set_namespace('),
        ('raptor_serialize_set_namespace_from_namespace\\(', 'raptor_serializer_set_namespace_from_namespace('),
        ('raptor_serialize_start\\(', 'raptor_serializer_start_to_iostream('),
        ('raptor_serialize_start_to_file_handle\\(', 'raptor_serializer_start_to_file_handle('),
        ('raptor_serialize_start_to_filename\\(', 'raptor_serializer_start_to_filename('),
        ('raptor_serialize_start_to_iostream\\(', 'raptor_serializer_start_to_iostream('),
        ('raptor_serialize_start_to_string\\(', 'raptor_serializer_start_to_string('),
        ('raptor_serializer_syntax_name_check_v2\\(', 'raptor_world_is_serializer_name('),
        ('raptor_set_namespace_handler\\(', 'raptor_parser_set_namespace_handler('),
        ('raptor_set_statement_handler\\(', 'raptor_parser_set_statement_handler('),
        ('raptor_start_parse\\(', 'raptor_parser_parse_start('),
        ('raptor_uri_is_file_uri\\(', 'raptor_uri_uri_string_is_file_uri('),
        ('raptor_utf8_check\\(', 'raptor_unicode_check_utf8_string('),
        ('raptor_www_free\\(', 'raptor_free_www('),
        ('raptor_www_new_v2\\(', 'raptor_new_www('),
        ('raptor_www_new_with_connection_v2\\(', 'raptor_new_www_with_connection('),
    ]
    
    # Apply basic transformations
    for old, new in transformations:
        content = re.sub(old, new, content)
    
    # Apply function deletions
    for func_name, note in deleted_functions:
        pattern = rf'^(.*)({re.escape(func_name)}.*)$'
        replacement = rf'/* WARNING: {func_name} - deleted. {note} */ \1\2'
        content = re.sub(pattern, replacement, content, flags=re.MULTILINE)
    
    # Apply function renames
    for old_pattern, new_name in function_renames:
        content = re.sub(old_pattern, new_name, content)
    
    # Apply enum/constant renames
    enum_renames = [
        ('RAPTOR_IDENTIFIER_TYPE_UNKNOWN', 'RAPTOR_TERM_TYPE_UNKNOWN'),
        ('RAPTOR_IDENTIFIER_TYPE_RESOURCE', 'RAPTOR_TERM_TYPE_URI'),
        ('RAPTOR_IDENTIFIER_TYPE_ANONYMOUS', 'RAPTOR_TERM_TYPE_BLANK'),
        ('RAPTOR_IDENTIFIER_TYPE_PREDICATE', 'RAPTOR_TERM_TYPE_URI'),
        ('RAPTOR_IDENTIFIER_TYPE_LITERAL', 'RAPTOR_TERM_TYPE_LITERAL'),
        ('RAPTOR_IDENTIFIER_TYPE_XML_LITERAL', 'RAPTOR_TERM_TYPE_LITERAL'),
        ('RAPTOR_NTRIPLES_TERM_TYPE_URI_REF', 'RAPTOR_TERM_TYPE_URI'),
        ('RAPTOR_NTRIPLES_TERM_TYPE_BLANK_NODE', 'RAPTOR_TERM_TYPE_BLANK'),
        ('RAPTOR_NTRIPLES_TERM_TYPE_LITERAL', 'RAPTOR_TERM_TYPE_LITERAL'),
        ('RAPTOR_FEATURE_SCANNING', 'RAPTOR_OPTION_SCANNING'),
        ('RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES', 'RAPTOR_OPTION_ALLOW_NON_NS_ATTRIBUTES'),
        ('RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES', 'RAPTOR_OPTION_ALLOW_OTHER_PARSETYPES'),
        ('RAPTOR_FEATURE_ALLOW_BAGID', 'RAPTOR_OPTION_ALLOW_BAGID'),
        ('RAPTOR_FEATURE_ALLOW_RDF_TYPE_RDF_LIST', 'RAPTOR_OPTION_ALLOW_RDF_TYPE_RDF_LIST'),
        ('RAPTOR_FEATURE_NORMALIZE_LANGUAGE', 'RAPTOR_OPTION_NORMALIZE_LANGUAGE'),
        ('RAPTOR_FEATURE_NON_NFC_FATAL', 'RAPTOR_OPTION_NON_NFC_FATAL'),
        ('RAPTOR_FEATURE_WARN_OTHER_PARSETYPES', 'RAPTOR_OPTION_WARN_OTHER_PARSETYPES'),
        ('RAPTOR_FEATURE_CHECK_RDF_ID', 'RAPTOR_OPTION_CHECK_RDF_ID'),
        ('RAPTOR_FEATURE_RELATIVE_URIS', 'RAPTOR_OPTION_RELATIVE_URIS'),
        ('RAPTOR_FEATURE_WRITER_AUTO_INDENT', 'RAPTOR_OPTION_WRITER_AUTO_INDENT'),
        ('RAPTOR_FEATURE_WRITER_AUTO_EMPTY', 'RAPTOR_OPTION_WRITER_AUTO_EMPTY'),
        ('RAPTOR_FEATURE_WRITER_INDENT_WIDTH', 'RAPTOR_OPTION_WRITER_INDENT_WIDTH'),
        ('RAPTOR_FEATURE_WRITER_XML_VERSION', 'RAPTOR_OPTION_WRITER_XML_VERSION'),
        ('RAPTOR_FEATURE_WRITER_XML_DECLARATION', 'RAPTOR_OPTION_WRITER_XML_DECLARATION'),
        ('RAPTOR_FEATURE_NO_NET', 'RAPTOR_OPTION_NO_NET'),
        ('RAPTOR_FEATURE_RESOURCE_BORDER', 'RAPTOR_OPTION_RESOURCE_BORDER'),
        ('RAPTOR_FEATURE_LITERAL_BORDER', 'RAPTOR_OPTION_LITERAL_BORDER'),
        ('RAPTOR_FEATURE_BNODE_BORDER', 'RAPTOR_OPTION_BNODE_BORDER'),
        ('RAPTOR_FEATURE_RESOURCE_FILL', 'RAPTOR_OPTION_RESOURCE_FILL'),
        ('RAPTOR_FEATURE_LITERAL_FILL', 'RAPTOR_OPTION_LITERAL_FILL'),
        ('RAPTOR_FEATURE_BNODE_FILL', 'RAPTOR_OPTION_BNODE_FILL'),
        ('RAPTOR_FEATURE_HTML_TAG_SOUP', 'RAPTOR_OPTION_HTML_TAG_SOUP'),
        ('RAPTOR_FEATURE_MICROFORMATS', 'RAPTOR_OPTION_MICROFORMATS'),
        ('RAPTOR_FEATURE_HTML_LINK', 'RAPTOR_OPTION_HTML_LINK'),
        ('RAPTOR_FEATURE_WWW_TIMEOUT', 'RAPTOR_OPTION_WWW_TIMEOUT'),
        ('RAPTOR_FEATURE_WRITE_BASE_URI', 'RAPTOR_OPTION_WRITE_BASE_URI'),
        ('RAPTOR_FEATURE_WWW_HTTP_CACHE_CONTROL', 'RAPTOR_OPTION_WWW_HTTP_CACHE_CONTROL'),
        ('RAPTOR_FEATURE_WWW_HTTP_USER_AGENT', 'RAPTOR_OPTION_WWW_HTTP_USER_AGENT'),
        ('RAPTOR_FEATURE_JSON_CALLBACK', 'RAPTOR_OPTION_JSON_CALLBACK'),
        ('RAPTOR_FEATURE_JSON_EXTRA_DATA', 'RAPTOR_OPTION_JSON_EXTRA_DATA'),
        ('RAPTOR_FEATURE_RSS_TRIPLES', 'RAPTOR_OPTION_RSS_TRIPLES'),
        ('RAPTOR_FEATURE_ATOM_ENTRY_URI', 'RAPTOR_OPTION_ATOM_ENTRY_URI'),
        ('RAPTOR_FEATURE_PREFIX_ELEMENTS', 'RAPTOR_OPTION_PREFIX_ELEMENTS'),
        ('RAPTOR_FEATURE_LAST', 'RAPTOR_OPTION_LAST'),
        ('RAPTOR_LOG_LEVEL_WARNING', 'RAPTOR_LOG_LEVEL_WARN'),
        ('RAPTOR_LIBXML_FLAGS_GENERIC_ERROR_SAVE', 'RAPTOR_WORLD_FLAG_LIBXML_GENERIC_ERROR_SAVE'),
        ('RAPTOR_LIBXML_FLAGS_STRUCTURED_ERROR_SAVE', 'RAPTOR_WORLD_FLAG_LIBXML_STRUCTURED_ERROR_SAVE'),
    ]
    
    for old_enum, new_enum in enum_renames:
        content = re.sub(re.escape(old_enum), new_enum, content)
    
    return content


def process_file(file_path: Path, backup: bool = True) -> bool:
    """Process a single file with transformations."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            original_content = f.read()
        
        transformed_content = apply_transformations(original_content)
        
        # Only write if content changed
        if transformed_content != original_content:
            if backup:
                backup_path = file_path.with_suffix(file_path.suffix + '~')
                shutil.copy2(file_path, backup_path)
                print(f"Created backup: {backup_path}")
            
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(transformed_content)
            
            print(f"Transformed: {file_path}")
            return True
        else:
            print(f"No changes: {file_path}")
            return False
            
    except Exception as e:
        print(f"Error processing {file_path}: {e}", file=sys.stderr)
        return False


def main():
    parser = argparse.ArgumentParser(
        description="Upgrade raptor code between versions by applying transformation rules.",
        epilog="This script applies transformation rules to upgrade raptor code from older versions to newer versions. "
               "By default, backup files are created with a '~' suffix."
    )
    parser.add_argument(
        "files", nargs='*', help="Files to process (if none specified, reads from stdin)"
    )
    parser.add_argument(
        "--no-backup", action="store_true", 
        help="Don't create backup files"
    )
    parser.add_argument(
        "--dry-run", action="store_true",
        help="Show what would be changed without modifying files"
    )
    
    args = parser.parse_args()
    
    if not args.files:
        # Process stdin
        content = sys.stdin.read()
        transformed = apply_transformations(content)
        sys.stdout.write(transformed)
        return 0
    
    success_count = 0
    total_count = len(args.files)
    
    for file_arg in args.files:
        file_path = Path(file_arg)
        if not file_path.exists():
            print(f"Error: File not found: {file_path}", file=sys.stderr)
            continue
        
        if args.dry_run:
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    original = f.read()
                transformed = apply_transformations(original)
                if transformed != original:
                    print(f"Would transform: {file_path}")
                else:
                    print(f"No changes needed: {file_path}")
                success_count += 1
            except Exception as e:
                print(f"Error reading {file_path}: {e}", file=sys.stderr)
        else:
            if process_file(file_path, backup=not args.no_backup):
                success_count += 1
    
    if args.files:
        print(f"Processed {success_count}/{total_count} files successfully")
    
    return 0 if success_count == total_count else 1


if __name__ == "__main__":
    sys.exit(main())