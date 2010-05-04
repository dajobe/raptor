#!/usr/bin/perl -w
#
# Format changes TSV file
#
# USAGE:
#   format-changes.pl --to-docbook-xml raptor-changes.tsv > ...
#
# Copyright (C) 2010, David Beckett http://www.dajobe.org/
#
# This package is Free Software and part of Redland http://librdf.org/
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


use strict;
use File::Basename;

our $program = basename $0;

our $nbsp = '&#160;';

sub print_start_chapter_as_docbook_xml($$$) {
  my($id, $title, $intro_para)=@_;

  print <<"EOT";
<!DOCTYPE refentry PUBLIC "-//OASIS//DTD DocBook XML V4.3//EN" 
               "http://www.oasis-open.org/docbook/xml/4.3/docbookx.dtd">
<chapter id="$id">
<title>$title</title>

<para>$intro_para</para>
EOT

}

sub print_end_chapter_as_docbook_xml()
{
  print <<"EOT";

</chapter>
EOT
}



sub print_docbook_xml($$@) {
  my($id, $title, @list)=@_;

  print <<"EOT";
<section id="$id">
<title>$title</title>
EOT

  print <<"EOT";
</section>
EOT
}

sub print_start_section_as_docbook_xml($$) {
  my($id, $title)=@_;

  print <<"EOT";
<section id="$id">
<title>$title</title>

EOT
}


sub format_function_name_as_docbook_xml($) {
  my($name)=@_;
  
  my $escaped_name = $name; $escaped_name =~ s/_/-/g;
  return qq{<link linkend="$escaped_name"><function>$name</function></link>};
}

sub format_type_name_as_docbook_xml($) {
  my($name)=@_;
  
  my $escaped_name = $name; $escaped_name =~ s/_/-/g;
  return qq{<link linkend="$escaped_name"><type>$name</type></link>};
}


sub format_enum_name_as_docbook_xml($) {
  my($name)=@_;
  
  my $escaped_name = $name; $escaped_name =~ s/_/-/g;
  return qq{<link linkend="$escaped_name:CAPS"><literal>$name</literal></link>};
}


sub format_fn_sig($$$$$) {
  my($format_name, $show_sig, $fn_return, $fn_name, $fn_args)=@_;
  my $formatted_name = $format_name ? format_function_name_as_docbook_xml($fn_name) : $fn_name;

  return $show_sig ? $fn_return . " " . $formatted_name . $fn_args
                   : $formatted_name;
}

sub format_notes($$) {
  my($is_inline,$notes)=@_;

  if ($notes eq '') {
    return $is_inline ? '' : $nbsp;
  }

  $notes =~ s{#((?:raptor|librdf|rasqal)\w+)}{format_type_name_as_docbook_xml($1)}ge;
  $notes =~ s{#?((?:RAPTOR|LIBRDF|RASQAL)_\w+)}{format_enum_name_as_docbook_xml($1)}ge;
  $notes =~ s{((?:raptor|librdf|rasqal)_\w+?)\(}{format_function_name_as_docbook_xml($1)."("}ge;

  return $is_inline ? "- " . $notes : $notes;
}

sub print_functions_list_as_docbook_xml($$$@) {
  my($title, $format_name, $show_sig, @list)=@_;

  print <<"EOT";
  <itemizedlist>
    <title>Functions</title>
EOT

  print  "  <caption>$title</caption>\n"
    if defined $title;

  for my $item (@list) {
    my($fn_return, $fn_name, $fn_args, $notes) = @$item;
    my $formatted_fn = format_fn_sig($format_name, $show_sig, 
				     $fn_return, $fn_name, $fn_args);
    $notes = format_notes(1, $notes);
    print "    <listitem><para>$formatted_fn $notes</para></listitem>\n";
  }
  print <<"EOT";
  </itemizedlist>
EOT
}


sub format_type_sig($$) {
  my($format_name, $type_name)=@_;
  return $format_name ? format_type_name_as_docbook_xml($type_name) : $type_name;
}


sub print_types_list_as_docbook_xml($$$@) {
  my($title, $format_name, $show_sig, @list)=@_;

  print <<"EOT";
  <itemizedlist>
    <title>Types</title>
EOT

  print  "  <caption>$title</caption>\n"
    if defined $title;

  for my $item (@list) {
    my($type_name, $notes) = @$item;
    my $formatted_fn = format_type_sig($format_name, $type_name);
    $notes = format_notes(1, $notes);
    print "    <listitem><para>$formatted_fn $notes</para></listitem>\n";
  }
  print <<"EOT";
  </itemizedlist>
EOT
}


sub print_renamed_functions_as_docbook_xml($$$@) {
  my($title, $old_function_header, $new_function_header, @list)=@_;

  print <<"EOT";
<table border='1'>
EOT

  print  "  <caption>$title</caption>\n"
    if defined $title;

  print <<"EOT";
  <thead>
  </thead>
  <tbody>
    <tr>
      <th>$old_function_header</th>
      <th>$new_function_header</th>
      <th>Notes</th>
    </tr>
EOT
  for my $item (@list) {
    my($from, $to, $notes) = @$item;
    my $formatted_name = format_function_name_as_docbook_xml($to);

    $notes = format_notes(0, $notes);
    print "    <tr valign='top'>\n      <td>$from</td> <td>$formatted_name</td> <td>$notes</td>\n   </tr>\n";
  }
  print <<"EOT";
  </tbody>
</table>
EOT

}

sub print_changed_functions_as_docbook_xml($$$@) {
  my($title, $old_function_header, $new_function_header, @list)=@_;

  print <<"EOT";
<table border='1'>
EOT

  print  "  <caption>$title</caption>\n"
    if defined $title;

  print <<"EOT";
  <thead>
  </thead>
  <tbody>
    <tr>
      <th>$old_function_header</th>
      <th>$new_function_header</th>
      <th>Notes</th>
    </tr>
EOT
  for my $item (@list) {
    my($old_fn_return, $old_fn_name, $old_fn_args,
       $new_fn_return, $new_fn_name, $new_fn_args, $notes) = @$item;

    my $old_formatted_fn = format_fn_sig(0, 1,
					 $old_fn_return, $old_fn_name, $old_fn_args);

    my $new_formatted_fn = format_fn_sig(1, 1,
					 $new_fn_return, $new_fn_name, $new_fn_args);

    $notes = format_notes(0, $notes);
    print "    <tr valign='top'>\n      <td>$old_formatted_fn</td> <td>$new_formatted_fn</td> <td>$notes</td>\n    </tr>\n";
  }
  print <<"EOT";
  </tbody>
</table>
EOT

}

sub print_end_section_as_docbook_xml()
{
  print <<"EOT";

</section>
EOT
}


sub print_changed_types_as_docbook_xml($$$@) {
  my($title, $old_type_header, $new_type_header, @list)=@_;

  print <<"EOT";
<table border='1'>
EOT

  print  "  <caption>$title</caption>\n"
    if defined $title;

  print <<"EOT";
  <thead>
  </thead>
  <tbody>
    <tr>
      <th>$old_type_header</th>
      <th>$new_type_header</th>
      <th>Notes</th>
    </tr>
EOT
  for my $item (@list) {
    my($old_type_name, $new_type_name,  $notes) = @$item;

    my $old_formatted_type = format_type_sig(0, $old_type_name);
    my $new_formatted_type = format_type_sig(1, $new_type_name);

    $notes = format_notes(0, $notes);
    print "    <tr valign='top'>\n      <td>$old_formatted_type</td> <td>$new_formatted_type</td> <td>$notes</td>\n    </tr>\n";
  }
  print <<"EOT";
  </tbody>
</table>
EOT

}



# main

# Arguments
die "USAGE: $program --to-docbook-xml PACKAGE API-TSV-FILE"
  unless @ARGV == 3 && $ARGV[0] eq '--to-docbook-xml';
our($dummy, $package, $file)=@ARGV;


# Read in data

# Example of Format (9 fields):
# OLD VERSION<tab>OLD RETURN<tab>OLD NAME<tab>OLD ARGS<tab>NEW VERSION<tab>NEW RETURN<tab>NEW NAME<tab>NEW ARGS<tab>NOTES
# 1.4.21	void*	raptor_alloc_memory	(size_t size)	1.9.0	void*	raptor_alloc_memory	(size_t size)	-
our $expected_n_fields = 9;

my(@new_functions);
my(@deleted_functions);
my(@renamed_functions);
my(@changed_functions);

my(@new_types);
my(@deleted_types);
my(@changed_types);

our $old_version;
our $new_version;

open(IN, "<$file") or die "$program: Cannot read $file - $!\n";
while(<IN>) {
  chomp;

  next if /^#/;

  my(@fields)=split(/\t/);
  die "$program: Bad line $.: $_\n"
    unless scalar(@fields) == $expected_n_fields;

  if($fields[1] eq 'type') {
    # Do not handle types yet.

    my($old_ver, $dummy1, $old_name, $old_args, $new_ver, $dummy2, $new_name, $new_args,$notes)=@fields;

    $old_version = $old_ver unless defined $old_version;
    $new_version = $new_ver unless defined $new_version;

    $notes = '' if $notes eq '-';

    if($old_name eq '-') {
      push(@new_types, [$new_name, $notes]);
    } elsif($new_name eq '-') {
      push(@deleted_types, [$old_name, $notes]);
    } elsif($old_name eq $new_name) {
      # same
    } else {
      # renamed and maybe something else changed - in the notes
      push(@changed_types, [$old_name, $new_name, $notes]);
    }

  } else {
    my($old_ver, $old_return, $old_name, $old_args, $new_ver, $new_return, $new_name, $new_args,$notes)=@fields;

    $old_version = $old_ver unless defined $old_version;
    $new_version = $new_ver unless defined $new_version;

    $notes = '' if $notes eq '-';

    if($old_name eq '-') {
      push(@new_functions, [$new_return, $new_name, $new_args, $notes]);
    } elsif($new_name eq '-') {
      push(@deleted_functions, [$old_return, $old_name, $old_args, $notes]);
    } elsif($old_return eq $new_return && $old_name eq $new_name &&
	    $old_args eq $new_args) {
      # same
    } elsif($old_return eq $new_return && $old_name ne $new_name &&
	    $old_args eq $new_args) {
      # renamed but nothing else changed
      push(@renamed_functions, [$old_name, $new_name, $notes]);
    } else {
      # something changed - args and/or return
      push(@changed_functions, [$old_return, $old_name, $old_args, $new_return, $new_name, $new_args, $notes]);
    }
  }
}
close(IN);


# Write output

our $intro_title = "API Changes";
our $intro_para = <<"EOT";
This chapter describes the API changes between $package
$old_version and $new_version.
EOT
print_start_chapter_as_docbook_xml('raptor-changes',
				   $intro_title,
				   $intro_para);

print_start_section_as_docbook_xml('raptor-changes-intro',
				   "Introduction");
print <<'EOT';
<para>
The following sections describe the function changes in the API:
additions, deletions, renames (retaining the same number of
parameters, types and return value type) and functions with more
complex changes.  Changes to typedefs and structs are not described here.
</para>
EOT

print_end_section_as_docbook_xml();


print_start_section_as_docbook_xml('raptor-changes-new',
				   "New functions and types in $package $new_version");
print_functions_list_as_docbook_xml(undef, 1, 1, @new_functions);
print_types_list_as_docbook_xml(undef, 1, 1, @new_types);
print_end_section_as_docbook_xml();

print_start_section_as_docbook_xml('raptor-changes-deleted',
				   "Deleted functions and types in $package $new_version");
print_functions_list_as_docbook_xml(undef, 0, 0, @deleted_functions);
print_types_list_as_docbook_xml(undef, 1, 1, @deleted_types);
print_end_section_as_docbook_xml();

print_start_section_as_docbook_xml('raptor-changes-renamed',
				   "Renamed functions in $package $new_version");
print_renamed_functions_as_docbook_xml(undef,
				       "$old_version function",
				       "$new_version function",
				       @renamed_functions);
print_end_section_as_docbook_xml();

print_start_section_as_docbook_xml('raptor-changes-changed',
				   "Changed functions and types in $package $new_version");
print_changed_functions_as_docbook_xml('Functions', 
				       "$old_version function",
				       "$new_version function",
				       @changed_functions);
print_changed_types_as_docbook_xml('Types', 
				   "$old_version type",
				   "$new_version type",
				   @changed_types);
print_end_section_as_docbook_xml();

print_end_chapter_as_docbook_xml();

exit 0;
