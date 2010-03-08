#!/usr/bin/perl -w
#
# Format changes TSV file
#
# USAGE:
#   format-changes.pl --to-docbook.xml raptor-changes.tsv > ...
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

sub format_fn_sig($$$$$) {
  my($format_name, $show_sig, $fn_return, $fn_name, $fn_args)=@_;
  my $formatted_name = $format_name ? format_function_name_as_docbook_xml($fn_name) : $fn_name;

  return $show_sig ? $fn_return . " " . $formatted_name . $fn_args
                   : $formatted_name;
}

sub print_functions_list_as_docbook_xml($$$@) {
  my($title, $format_name, $show_sig, @list)=@_;

  print <<"EOT";
  <itemizedlist>
EOT

  print  "  <caption>$title</caption>\n"
    if defined $title;

  for my $item (@list) {
    my($fn_return, $fn_name, $fn_args) = @$item;
    my $formatted_fn = format_fn_sig($format_name, $show_sig, 
				     $fn_return, $fn_name, $fn_args);
    print "    <listitem><para>$formatted_fn</para></listitem>\n";
  }
  print <<"EOT";
  </itemizedlist>
EOT
}

sub print_renamed_functions_as_docbook_xml($@) {
  my($title, @list)=@_;

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
      <th>Old Function</th>
      <th>New Function</th>
    </tr>
EOT
  for my $item (@list) {
    my($from, $to) = @$item;
    my $formatted_name = format_function_name_as_docbook_xml($to);
    print "    <tr valign='top'>\n      <td>$from</td> <td>$formatted_name</td>\n    </tr>\n";
  }
  print <<"EOT";
  </tbody>
</table>
EOT

}

sub print_changed_functions_as_docbook_xml($@) {
  my($title, @list)=@_;

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
      <th>Old Function</th>
      <th>New Function</th>
    </tr>
EOT
  for my $item (@list) {
    my($old_fn_return, $old_fn_name, $old_fn_args,
       $new_fn_return, $new_fn_name, $new_fn_args) = @$item;

    my $old_formatted_fn = format_fn_sig(0, 1,
					 $old_fn_return, $old_fn_name, $old_fn_args);

    my $new_formatted_fn = format_fn_sig(1, 1,
					 $new_fn_return, $new_fn_name, $new_fn_args);

    print "    <tr valign='top'>\n      <td>$old_formatted_fn</td> <td>$new_formatted_fn</td>\n    </tr>\n";
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



# main

# Arguments
die "USAGE: $program --to-docbook-xml PACKAGE API-TSV-FILE"
  unless @ARGV == 3 && $ARGV[0] eq '--to-docbook-xml';
our($dummy, $package, $file)=@ARGV;


# Read in data

# Example of Format (8 fields):
# OLD VER,OLD RETURN, OLD NAME, OLD ARGS, NEW VER, NEW RETURN, NEW NAME, NEW ARGS
# 1.4.21	void*	raptor_alloc_memory	(size_t size)	1.9.0	void*	raptor_alloc_memory	(size_t size)
our $expected_n_fields = 8;

my(@new_functions);
my(@deleted_functions);
my(@renamed_functions);
my(@changed_functions);

our $old_version;
our $new_version;

open(IN, "<$file") or die "$program: Cannot read $file - $!\n";
while(<IN>) {
  chomp;

  my(@fields)=split(/\t/);
  die "$program: Bad line $.: $_\n"
    unless scalar(@fields) == $expected_n_fields;

  my($old_ver, $old_return, $old_name, $old_args, $new_ver, $new_return, $new_name, $new_args)=@fields;

  $old_version = $old_ver unless defined $old_version;
  $new_version = $new_ver unless defined $new_version;

  if($old_name eq '-') {
    push(@new_functions, [$new_return, $new_name, $new_args]);
  } elsif($new_name eq '-') {
    push(@deleted_functions, [$old_return, $old_name, $old_args]);
  } elsif($old_return eq $new_return && $old_name eq $new_name &&
	  $old_args eq $new_args) {
    # same
  } elsif($old_return eq $new_return && $old_name ne $new_name &&
	  $old_args eq $new_args) {
    # renamed but nothing else changed
    push(@renamed_functions, [$old_name, $new_name]);
  } else {
    # something changed - args and/or return
    push(@changed_functions, [$old_return, $old_name, $old_args, $new_return, $new_name, $new_args]);
  }
}
close(IN);


# Write output

our $intro_title = "Changes between $package $old_version and $new_version";
our $intro_para = <<"EOT";
This chapter describes the changes between $package
$old_version and $new_version.
EOT
print_start_chapter_as_docbook_xml('raptor-changes',
				   $intro_title,
				   $intro_para);

print_start_section_as_docbook_xml('raptor-changes-new',
				   "New functions in $package $new_version");
print_functions_list_as_docbook_xml(undef, 1, 1, @new_functions);
print_end_section_as_docbook_xml();

print_start_section_as_docbook_xml('raptor-changes-deleted',
				   "Deleted functions in $package $new_version");
print_functions_list_as_docbook_xml(undef, 0, 0, @deleted_functions);
print_end_section_as_docbook_xml();

print_start_section_as_docbook_xml('raptor-changes-renamed',
				   "Renamed functions in $package $new_version");
print_renamed_functions_as_docbook_xml(undef, @renamed_functions);
print_end_section_as_docbook_xml();

print_start_section_as_docbook_xml('raptor-changes-changed',
				   "Changed functions in $package between $old_version and $new_version");
print_changed_functions_as_docbook_xml(undef, @changed_functions);
print_end_section_as_docbook_xml();

print_end_chapter_as_docbook_xml();

exit 0;
