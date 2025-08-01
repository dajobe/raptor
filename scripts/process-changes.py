#!/usr/bin/env python3
#
# Format changes TSV file
#
# USAGE:
#   process-changes.py [OPTIONS] PACKAGE-NAME TSV-FILE
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
import csv
import os
import re
import sys
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, TextIO, Tuple

NBSP = "&#160;"


@dataclass
class Change:
    notes: str


@dataclass
class FunctionChange(Change):
    name: str


@dataclass
class TypeChange(Change):
    name: str


@dataclass
class EnumChange(Change):
    name: str


@dataclass
class NewFunction(FunctionChange):
    return_type: str
    args: str


@dataclass
class DeletedFunction(FunctionChange):
    return_type: str
    args: str


@dataclass
class RenamedFunction(Change):
    old_name: str
    new_name: str


@dataclass
class ChangedFunction(Change):
    old_return_type: str
    old_name: str
    old_args: str
    new_return_type: str
    new_name: str
    new_args: str


@dataclass
class NewType(TypeChange):
    pass


@dataclass
class DeletedType(TypeChange):
    pass


@dataclass
class ChangedType(Change):
    old_name: str
    new_name: str


@dataclass
class NewEnum(EnumChange):
    pass


@dataclass
class DeletedEnum(EnumChange):
    pass


@dataclass
class RenamedEnum(Change):
    old_name: str
    new_name: str


@dataclass
class VersionChanges:
    new_functions: List[NewFunction] = field(default_factory=list)
    deleted_functions: List[DeletedFunction] = field(default_factory=list)
    renamed_functions: List[RenamedFunction] = field(default_factory=list)
    changed_functions: List[ChangedFunction] = field(default_factory=list)
    new_types: List[NewType] = field(default_factory=list)
    deleted_types: List[DeletedType] = field(default_factory=list)
    changed_types: List[ChangedType] = field(default_factory=list)
    new_enums: List[NewEnum] = field(default_factory=list)
    deleted_enums: List[DeletedEnum] = field(default_factory=list)
    renamed_enums: List[RenamedEnum] = field(default_factory=list)


def print_start_chapter_as_docbook_xml(
    fh: TextIO, chapter_id: str, title: str, intro_para: str
):
    fh.write(
        f'<!DOCTYPE refentry PUBLIC "-//OASIS//DTD DocBook XML V4.3//EN"\n'
        f'               "http://www.oasis-open.org/docbook/xml/4.3/docbookx.dtd">\n'
        f'<chapter id="{chapter_id}">\n'
        f"<title>{title}</title>\n\n"
        f"<para>{intro_para}</para>\n"
    )


def print_end_chapter_as_docbook_xml(fh: TextIO):
    fh.write("\n</chapter>\n")


def print_start_section_as_docbook_xml(fh: TextIO, section_id: str, title: str):
    fh.write(f'<section id="{section_id}">\n<title>{title}</title>\n\n')


def print_end_section_as_docbook_xml(fh: TextIO):
    fh.write("\n</section>\n")


def format_function_name_as_docbook_xml(name: str) -> str:
    escaped_name = name.replace("_", "-")
    return f'<link linkend="{escaped_name}"><function>{name}</function></link>'


def format_type_name_as_docbook_xml(name: str) -> str:
    escaped_name = name.replace("_", "-")
    if re.match(r"^[-A-Z0-9]+$", escaped_name):
        escaped_name += ":CAPS"
    return f'<link linkend="{escaped_name}"><type>{name}</type></link>'


def format_enum_name_as_docbook_xml(name: str) -> str:
    escaped_name = name.replace("_", "-")
    if re.match(r"^[-A-Z0-9]+$", escaped_name):
        escaped_name += ":CAPS"
    return f'<link linkend="{escaped_name}"><literal>{name}</literal></link>'


def format_fn_sig(
    format_name: bool, show_sig: bool, fn_return: str, fn_name: str, fn_args: str
) -> str:
    formatted_name = (
        format_function_name_as_docbook_xml(fn_name) if format_name else fn_name
    )
    return f"{fn_return} {formatted_name}{fn_args}" if show_sig else formatted_name


def format_notes(is_inline: bool, notes: str) -> str:
    if not notes:
        return "" if is_inline else NBSP

    notes = re.sub(
        r"#((?:raptor|librdf|rasqal)\w+)",
        lambda m: format_type_name_as_docbook_xml(m.group(1)),
        notes,
    )
    notes = re.sub(
        r"#?((?:RAPTOR|LIBRDF|RASQAL)_\w+)",
        lambda m: format_enum_name_as_docbook_xml(m.group(1)),
        notes,
    )
    notes = re.sub(
        r"((?:raptor|librdf|rasqal)_\w+?)\(",
        lambda m: format_function_name_as_docbook_xml(m.group(1)) + "(",
        notes,
    )

    return f"- {notes}" if is_inline else notes


def print_functions_list_as_docbook_xml(
    fh: TextIO, title: str, format_name: bool, show_sig: bool, items: List[NewFunction]
):
    if not items:
        return

    fh.write('  <itemizedlist>\n    <title>Functions</title>\n')
    if title:
        fh.write(f"  <caption>{title}</caption>\n")

    items.sort(key=lambda x: x.name)

    for item in items:
        formatted_fn = format_fn_sig(
            format_name, show_sig, item.return_type, item.name, item.args
        )
        notes_str = format_notes(True, item.notes)
        fh.write(f"    <listitem><para>{formatted_fn} {notes_str}</para></listitem>\n")

    fh.write("  </itemizedlist>\n")


def format_type_sig(format_name: bool, type_name: str) -> str:
    return format_type_name_as_docbook_xml(type_name) if format_name else type_name


def format_enum_sig(format_name: bool, enum_name: str) -> str:
    return format_enum_name_as_docbook_xml(enum_name) if format_name else enum_name


def print_types_list_as_docbook_xml(
    fh: TextIO, title: str, format_name: bool, items: List[NewType]
):
    if not items:
        return

    fh.write('  <itemizedlist>\n    <title>Types</title>\n')
    if title:
        fh.write(f"  <caption>{title}</caption>\n")

    items.sort(key=lambda x: x.name)

    for item in items:
        formatted_type = format_type_sig(format_name, item.name)
        notes_str = format_notes(True, item.notes)
        fh.write(
            f"    <listitem><para>{formatted_type} {notes_str}</para></listitem>\n"
        )

    fh.write("  </itemizedlist>\n")


def print_enums_list_as_docbook_xml(
    fh: TextIO, title: str, format_name: bool, items: List[NewEnum]
):
    if not items:
        return

    fh.write('  <itemizedlist>\n    <title>Enums</title>\n')
    if title:
        fh.write(f"  <caption>{title}</caption>\n")

    items.sort(key=lambda x: x.name)

    for item in items:
        formatted_enum = format_enum_sig(format_name, item.name)
        notes_str = format_notes(True, item.notes)
        fh.write(
            f"    <listitem><para>{formatted_enum} {notes_str}</para></listitem>\n"
        )

    fh.write("  </itemizedlist>\n")


def print_renamed_functions_as_docbook_xml(
    fh: TextIO, title: str, old_header: str, new_header: str, items: List[RenamedFunction]
):
    if not items:
        return

    fh.write('<table border=\'1\'>\n')
    if title:
        fh.write(f"  <caption>{title}</caption>\n")

    items.sort(key=lambda x: x.old_name)

    fh.write(
        f"  <thead>\n  </thead>\n  <tbody>\n    <tr>\n"
        f"      <th>{old_header}</th>\n      <th>{new_header}</th>\n      <th>Notes</th>\n    </tr>\n"
    )
    for item in items:
        formatted_name = format_function_name_as_docbook_xml(item.new_name)
        notes_str = format_notes(False, item.notes)
        fh.write(
            f'    <tr valign=\'top\'>\n      <td>{item.old_name}</td> <td>{formatted_name}</td> <td>{notes_str}</td>\n   </tr>\n'
        )
    fh.write("  </tbody>\n</table>\n")


def print_changed_functions_as_docbook_xml(
    fh: TextIO, title: str, old_header: str, new_header: str, items: List[ChangedFunction]
):
    if not items:
        return

    fh.write('<table border=\'1\'>\n')
    if title:
        fh.write(f"  <caption>{title}</caption>\n")

    fh.write(
        f"  <thead>\n  </thead>\n  <tbody>\n    <tr>\n"
        f"      <th>{old_header}</th>\n      <th>{new_header}</th>\n      <th>Notes</th>\n    </tr>\n"
    )
    for item in items:
        old_formatted = format_fn_sig(
            False, True, item.old_return_type, item.old_name, item.old_args
        )
        new_formatted = format_fn_sig(
            True, True, item.new_return_type, item.new_name, item.new_args
        )
        notes_str = format_notes(False, item.notes)
        fh.write(
            f'    <tr valign=\'top\'>\n      <td>{old_formatted}</td> <td>{new_formatted}</td> <td>{notes_str}</td>\n    </tr>\n'
        )
    fh.write("  </tbody>\n</table>\n")


def print_changed_types_as_docbook_xml(
    fh: TextIO, title: str, old_header: str, new_header: str, items: List[ChangedType]
):
    if not items:
        return

    fh.write('<table border=\'1\'>\n')
    if title:
        fh.write(f"  <caption>{title}</caption>\n")

    items.sort(key=lambda x: x.old_name)

    fh.write(
        f"  <thead>\n  </thead>\n  <tbody>\n    <tr>\n"
        f"      <th>{old_header}</th>\n      <th>{new_header}</th>\n      <th>Notes</th>\n    </tr>\n"
    )
    for item in items:
        old_formatted = format_type_sig(False, item.old_name)
        new_formatted = format_type_sig(True, item.new_name)
        notes_str = format_notes(False, item.notes)
        fh.write(
            f'    <tr valign=\'top\'>\n      <td>{old_formatted}</td> <td>{new_formatted}</td> <td>{notes_str}</td>\n    </tr>\n'
        )
    fh.write("  </tbody>\n</table>\n")


def print_renamed_enums_as_docbook_xml(
    fh: TextIO, title: str, old_header: str, new_header: str, items: List[RenamedEnum]
):
    if not items:
        return

    fh.write('<table border=\'1\'>\n')
    if title:
        fh.write(f"  <caption>{title}</caption>\n")

    items.sort(key=lambda x: x.old_name)

    fh.write(
        f"  <thead>\n  </thead>\n  <tbody>\n    <tr>\n"
        f"      <th>{old_header}</th>\n      <th>{new_header}</th>\n      <th>Notes</th>\n    </tr>\n"
    )
    for item in items:
        formatted_name = format_enum_name_as_docbook_xml(item.new_name)
        notes_str = format_notes(False, item.notes)
        fh.write(
            f'    <tr valign=\'top\'>\n      <td>{item.old_name}</td> <td>{formatted_name}</td> <td>{notes_str}</td>\n   </tr>\n'
        )
    fh.write("  </tbody>\n</table>\n")


def to_id(text: str) -> str:
    id_str = re.sub(r"\W", "-", text)
    id_str = re.sub(r"-+", "-", id_str)
    id_str = re.sub(r"^\-", "", id_str)
    id_str = re.sub(r"\-$", "", id_str)
    return id_str


def version_for_sort(version_string: str) -> List[str]:
    return [f"{int(p):02d}" for p in version_string.split(".")]


def parse_tsv_file(
    tsv_file: Path, no_warn_version: str = None
) -> Tuple[List[Tuple[str, str]], Dict[str, VersionChanges]]:
    version_pairs = []
    version_pairs_seen = set()
    changes: Dict[str, VersionChanges] = defaultdict(VersionChanges)
    expected_n_fields = 9

    try:
        with tsv_file.open("r", newline="") as f:
            reader = csv.reader(f, delimiter="\t")
            for i, fields in enumerate(reader):
                if not fields or fields[0].startswith("#"):
                    continue

                if len(fields) != expected_n_fields:
                    sys.exit(
                        f"{os.path.basename(sys.argv[0])}: Bad line has {len(fields)} fields, "
                        f"expected {expected_n_fields} at line {i + 1}: {fields}"
                    )

                version_pair = f"{fields[0]}-{fields[4]}"
                if version_pair not in version_pairs_seen:
                    version_pairs.append((fields[0], fields[4]))
                    version_pairs_seen.add(version_pair)

                notes = "" if fields[8] == "-" else fields[8]
                change_set = changes[version_pair]

                if fields[1] == "type":
                    old_name, new_name = fields[2], fields[6]
                    if old_name == "-":
                        change_set.new_types.append(NewType(name=new_name, notes=notes))
                    elif new_name == "-":
                        change_set.deleted_types.append(
                            DeletedType(name=old_name, notes=notes)
                        )
                    elif old_name != new_name or notes:
                        change_set.changed_types.append(
                            ChangedType(old_name=old_name, new_name=new_name, notes=notes)
                        )
                elif fields[1] == "enum":
                    old_name, new_name = fields[2], fields[6]
                    if old_name == "-":
                        change_set.new_enums.append(NewEnum(name=new_name, notes=notes))
                    elif new_name == "-":
                        change_set.deleted_enums.append(
                            DeletedEnum(name=old_name, notes=notes)
                        )
                    elif old_name != new_name or notes:
                        change_set.renamed_enums.append(
                            RenamedEnum(old_name=old_name, new_name=new_name, notes=notes)
                        )
                else:  # Function
                    (
                        _,
                        old_return,
                        old_name,
                        old_args,
                        _,
                        new_return,
                        new_name,
                        new_args,
                        _,
                    ) = fields
                    if old_name == "-":
                        change_set.new_functions.append(
                            NewFunction(
                                name=new_name,
                                return_type=new_return,
                                args=new_args,
                                notes=notes,
                            )
                        )
                    elif new_name == "-":
                        change_set.deleted_functions.append(
                            DeletedFunction(
                                name=old_name,
                                return_type=old_return,
                                args=old_args,
                                notes=notes,
                            )
                        )
                    elif (
                        old_return == new_return
                        and old_name != new_name
                        and old_args == new_args
                    ):
                        change_set.renamed_functions.append(
                            RenamedFunction(
                                old_name=old_name, new_name=new_name, notes=notes
                            )
                        )
                    elif (
                        old_return == new_return
                        and old_name == new_name
                        and old_args == new_args
                    ):
                        if not no_warn_version or fields[0] != no_warn_version:
                            print(
                                f"Warning: Line records no function change: {fields}",
                                file=sys.stderr,
                            )
                    else:
                        change_set.changed_functions.append(
                            ChangedFunction(
                                old_name=old_name,
                                old_return_type=old_return,
                                old_args=old_args,
                                new_name=new_name,
                                new_return_type=new_return,
                                new_args=new_args,
                                notes=notes,
                            )
                        )
    except FileNotFoundError:
        sys.exit(f"{sys.argv[0]}: Cannot read {tsv_file} - No such file or directory")

    return version_pairs, changes


def generate_docbook_xml(
    output_file: Path,
    package: str,
    id_prefix: str,
    version_pairs: List[Tuple[str, str]],
    changes: Dict[str, VersionChanges],
):
    with output_file.open("w") as fh:
        intro_title = "API Changes"
        intro_para = f"This chapter describes the API changes for {package}.\n"
        print_start_chapter_as_docbook_xml(
            fh, f"{id_prefix}-changes", intro_title, intro_para
        )

        print_start_section_as_docbook_xml(
            fh, f"{id_prefix}-changes-intro", "Introduction"
        )
        fh.write(
            "<para>\nThe following sections describe the changes in the API between\n"
            "versions including additions, deletions, renames (retaining the same\n"
            "number of parameters, types and return value type) and more complex\n"
            "changes to functions, types and enums.\n</para>\n"
        )
        print_end_section_as_docbook_xml(fh)

        version_pairs.sort(key=lambda x: version_for_sort(x[1]))

        for old_version, new_version in version_pairs:
            version_pair_str = f"{old_version}-{new_version}"
            id_part = f"{to_id(old_version)}-to-{to_id(new_version)}"
            change_set = changes.get(version_pair_str)

            if not change_set:
                continue

            print_start_section_as_docbook_xml(
                fh,
                f"{id_prefix}-changes-{id_part}",
                f"Changes between {package} {old_version} and {new_version}",
            )

            # New
            if (
                change_set.new_functions
                or change_set.new_types
                or change_set.new_enums
            ):
                print_start_section_as_docbook_xml(
                    fh,
                    f"{id_prefix}-changes-new-{id_part}",
                    "New functions, types and enums",
                )
                print_functions_list_as_docbook_xml(
                    fh, None, True, True, change_set.new_functions
                )
                print_types_list_as_docbook_xml(
                    fh, None, True, change_set.new_types
                )
                print_enums_list_as_docbook_xml(
                    fh, None, True, change_set.new_enums
                )
                print_end_section_as_docbook_xml(fh)

            # Deleted
            if (
                change_set.deleted_functions
                or change_set.deleted_types
                or change_set.deleted_enums
            ):
                print_start_section_as_docbook_xml(
                    fh,
                    f"{id_prefix}-changes-deleted-{id_part}",
                    "Deleted functions, types and enums",
                )
                print_functions_list_as_docbook_xml(
                    fh, None, False, False, change_set.deleted_functions
                )
                print_types_list_as_docbook_xml(
                    fh, None, False, change_set.deleted_types
                )
                print_enums_list_as_docbook_xml(
                    fh, None, False, change_set.deleted_enums
                )
                print_end_section_as_docbook_xml(fh)

            # Renamed
            if change_set.renamed_functions or change_set.renamed_enums:
                print_start_section_as_docbook_xml(
                    fh,
                    f"{id_prefix}-changes-renamed-{id_part}",
                    "Renamed function and enums",
                )
                print_renamed_functions_as_docbook_xml(
                    fh,
                    None,
                    f"{old_version} function",
                    f"{new_version} function",
                    change_set.renamed_functions,
                )
                print_renamed_enums_as_docbook_xml(
                    fh,
                    None,
                    f"{old_version} enum",
                    f"{new_version} enum",
                    change_set.renamed_enums,
                )
                print_end_section_as_docbook_xml(fh)

            # Changed
            if change_set.changed_functions or change_set.changed_types:
                print_start_section_as_docbook_xml(
                    fh,
                    f"{id_prefix}-changes-changed-{id_part}",
                    "Changed functions and types",
                )
                print_changed_functions_as_docbook_xml(
                    fh,
                    None,
                    f"{old_version} function",
                    f"{new_version} function",
                    change_set.changed_functions,
                )
                print_changed_types_as_docbook_xml(
                    fh,
                    None,
                    f"{old_version} type",
                    f"{new_version} type",
                    change_set.changed_types,
                )
                print_end_section_as_docbook_xml(fh)

            print_end_section_as_docbook_xml(fh)

        print_end_chapter_as_docbook_xml(fh)


def main():
    parser = argparse.ArgumentParser(
        description="Turn a package's changes TSV into files.",
        epilog="Turn a package's changes TSV file into docbook XML.",
    )
    parser.add_argument(
        "--docbook-xml",
        metavar="DOCBOOK_XML",
        type=Path,
        help="Set the output docbook XML file",
    )
    parser.add_argument(
        "--package", help="Set the package name (used as a prefix in IDs)"
    )
    parser.add_argument(
        "--no-warn-version", 
        help="Don't warn about unchanged functions for this old version (e.g., 1.4.21 for raptor)"
    )
    parser.add_argument(
        "package_name", metavar="PACKAGE-NAME", help="The name of the package"
    )
    parser.add_argument(
        "tsv_file",
        metavar="TSV-FILE",
        type=Path,
        help="The input TSV file with API changes",
    )

    args = parser.parse_args()

    package = args.package_name
    id_prefix = args.package or package

    version_pairs, changes = parse_tsv_file(args.tsv_file, args.no_warn_version)

    if args.docbook_xml:
        generate_docbook_xml(
            args.docbook_xml, package, id_prefix, version_pairs, changes
        )


if __name__ == "__main__":
    main()
