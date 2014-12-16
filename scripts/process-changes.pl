#!/usr/bin/perl -w
#
# Format changes TSV file
#
# USAGE:
#   process-changes.pl [OPTIONS] CHANGES-TSV-FILE
#
# Copyright (C) 2010-2011, David Beckett http://www.dajobe.org/
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
#
#
# Example of Format (9 fields):
# OLD VERSION<tab>type | enum | OLD RETURN<tab>OLD NAME<tab>OLD ARGS<tab>NEW VERSION<tab>type | enum | NEW RETURN<tab>NEW NAME<tab>NEW ARGS<tab>NOTES
#
# Functions
# 0.9.21<tab>void<tab>oldfunctionor-<tab>(args)<tab>0.9.22<tab>void<tab>newfunctionor-<tab>(args)<tab>NOTES
# Types
# 0.9.21<tab>type<tab>oldtypenameor-<tab>-<tab>0.9.22<tab>type<tab>newtypeor-<tab>-<tab>NOTES
# Enums
# 0.9.21<tab>enum<tab>oldenumvalueor-<tab>-<tab>0.9.22<tab>enum<tab>newenumvalueor-<tab>-<tab>NOTES
#

use strict;
use File::Basename;
use IO::File;
use Getopt::Long;
use Pod::Usage;

our $program = basename $0;

our $nbsp = '&#160;';

our $id_prefix = undef;

sub print_start_chapter_as_docbook_xml($$$$) {
  my($fh, $id, $title, $intro_para)=@_;

  print $fh <<"EOT";
<!DOCTYPE refentry PUBLIC "-//OASIS//DTD DocBook XML V4.3//EN" 
               "http://www.oasis-open.org/docbook/xml/4.3/docbookx.dtd">
<chapter id="$id">
<title>$title</title>

<para>$intro_para</para>
EOT

}

sub print_end_chapter_as_docbook_xml($)
{
  my($fh)=@_;
  print $fh <<"EOT";

</chapter>
EOT
}



sub print_docbook_xml($$$@) {
  my($fh, $id, $title, @list)=@_;

  print $fh <<"EOT";
<section id="$id">
<title>$title</title>
EOT

  print $fh <<"EOT";
</section>
EOT
}

sub print_start_section_as_docbook_xml($$$) {
  my($fh, $id, $title)=@_;

  print $fh <<"EOT";
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
  if($escaped_name =~ /^[-A-Z0-9]+$/) {
    $escaped_name .= ":CAPS";
  }
  return qq{<link linkend="$escaped_name"><type>$name</type></link>};
}

sub format_enum_name_as_docbook_xml($) {
  my($name)=@_;
  
  my $escaped_name = $name; $escaped_name =~ s/_/-/g;
  if($escaped_name =~ /^[-A-Z0-9]+$/) {
    $escaped_name .= ":CAPS";
  }
  return qq{<link linkend="$escaped_name"><literal>$name</literal></link>};
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

sub print_functions_list_as_docbook_xml($$$$@) {
  my($fh, $title, $format_name, $show_sig, @list)=@_;

  return if !@list;

  print $fh <<"EOT";
  <itemizedlist>
    <title>Functions</title>
EOT

  print $fh "  <caption>$title</caption>\n"
    if defined $title;

  # Sort by fn_name name
  @list = sort { $a->[1] cmp $b->[1] } @list;

  for my $item (@list) {
    my($fn_return, $fn_name, $fn_args, $notes) = @$item;
    my $formatted_fn = format_fn_sig($format_name, $show_sig, 
				     $fn_return, $fn_name, $fn_args);
    $notes = format_notes(1, $notes);
    print $fh "    <listitem><para>$formatted_fn $notes</para></listitem>\n";
  }
  print $fh <<"EOT";
  </itemizedlist>
EOT
}


sub format_type_sig($$) {
  my($format_name, $type_name)=@_;
  return $format_name ? format_type_name_as_docbook_xml($type_name) : $type_name;
}

sub format_enum_sig($$) {
  my($format_name, $enum_name)=@_;
  return $format_name ? format_enum_name_as_docbook_xml($enum_name) : $enum_name;
}


sub print_types_list_as_docbook_xml($$$$@) {
  my($fh, $title, $format_name, $show_sig, @list)=@_;

  return if !@list;

  print $fh <<"EOT";
  <itemizedlist>
    <title>Types</title>
EOT

  print $fh "  <caption>$title</caption>\n"
    if defined $title;

  # Sort by type name
  @list = sort { $a->[0] cmp $b->[0] } @list;

  for my $item (@list) {
    my($type_name, $notes) = @$item;
    my $formatted_fn = format_type_sig($format_name, $type_name);
    $notes = format_notes(1, $notes);
    print $fh "    <listitem><para>$formatted_fn $notes</para></listitem>\n";
  }
  print $fh <<"EOT";
  </itemizedlist>
EOT
}


sub print_enums_list_as_docbook_xml($$$$@) {
  my($fh, $title, $format_name, $show_sig, @list)=@_;

  return if !@list;

  print $fh <<"EOT";
  <itemizedlist>
    <title>Enums</title>
EOT

  print $fh "  <caption>$title</caption>\n"
    if defined $title;

  # Sort by format name
  @list = sort { $a->[0] cmp $b->[0] } @list;

  for my $item (@list) {
    my($enum_name, $notes) = @$item;
    my $formatted_fn = format_enum_sig($format_name, $enum_name);
    $notes = format_notes(1, $notes);
    print $fh "    <listitem><para>$formatted_fn $notes</para></listitem>\n";
  }
  print $fh <<"EOT";
  </itemizedlist>
EOT
}


sub print_renamed_functions_as_docbook_xml($$$$@) {
  my($fh, $title, $old_function_header, $new_function_header, @list)=@_;

  return if !@list;

  print $fh <<"EOT";
<table border='1'>
EOT

  print $fh  "  <caption>$title</caption>\n"
    if defined $title;

  # Sort by from name
  @list = sort { $a->[0] cmp $b->[0] } @list;

  print $fh <<"EOT";
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
    print $fh "    <tr valign='top'>\n      <td>$from</td> <td>$formatted_name</td> <td>$notes</td>\n   </tr>\n";
  }
  print $fh <<"EOT";
  </tbody>
</table>
EOT

}

sub print_changed_functions_as_docbook_xml($$$$@) {
  my($fh, $title, $old_function_header, $new_function_header, @list)=@_;

  return if !@list;

  print $fh <<"EOT";
<table border='1'>
EOT

  print $fh "  <caption>$title</caption>\n"
    if defined $title;

  print $fh <<"EOT";
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
    print $fh "    <tr valign='top'>\n      <td>$old_formatted_fn</td> <td>$new_formatted_fn</td> <td>$notes</td>\n    </tr>\n";
  }
  print $fh <<"EOT";
  </tbody>
</table>
EOT

}

sub print_end_section_as_docbook_xml($)
{
  my($fh)=@_;
  print $fh <<"EOT";

</section>
EOT
}


sub print_changed_types_as_docbook_xml($$$$@) {
  my($fh, $title, $old_type_header, $new_type_header, @list)=@_;

  return if !@list;

  print $fh <<"EOT";
<table border='1'>
EOT

  print $fh "  <caption>$title</caption>\n"
    if defined $title;

  # Sort by old type name
  @list = sort { $a->[0] cmp $b->[0] } @list;

  print $fh <<"EOT";
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
    print $fh "    <tr valign='top'>\n      <td>$old_formatted_type</td> <td>$new_formatted_type</td> <td>$notes</td>\n    </tr>\n";
  }
  print $fh <<"EOT";
  </tbody>
</table>
EOT

}


sub print_renamed_enums_as_docbook_xml($$$$@) {
  my($fh, $title, $old_enum_header, $new_enum_header, @list)=@_;

  return if !@list;

  print $fh <<"EOT";
<table border='1'>
EOT

  print $fh "  <caption>$title</caption>\n"
    if defined $title;

  # Sort by from name
  @list = sort { $a->[0] cmp $b->[0] } @list;

  print $fh <<"EOT";
  <thead>
  </thead>
  <tbody>
    <tr>
      <th>$old_enum_header</th>
      <th>$new_enum_header</th>
      <th>Notes</th>
    </tr>
EOT
  for my $item (@list) {
    my($from, $to,  $notes) = @$item;
    my $formatted_name = format_enum_name_as_docbook_xml($to);

    $notes = format_notes(0, $notes);
    print $fh "    <tr valign='top'>\n      <td>$from</td> <td>$formatted_name</td> <td>$notes</td>\n   </tr>\n";
  }
  print $fh <<"EOT";
  </tbody>
</table>
EOT

}


sub print_deletes_as_perl_script($$@) {
  my($out_fh, $title, @names) = @_;

  print $out_fh "\n# $title\n";

  for my $entry (@names) {
    my($name,$note)=@$entry;
    $note ||= '';
    print $out_fh qq{s|^(.*$name.*)\$|/\\* WARNING: $name - deleted. $note \\*/ \$1|g;\n};
  }
}


sub print_renames_as_perl_script($$$@) {
  my($out_fh, $title, $is_function, @names) = @_;

  print $out_fh "\n# $title\n";

  for my $entry (@names) {
    my($from, $to, $note)=@$entry;
    $note ||= '';
    my $suffix = ($is_function ? '\\(' : '');
    print $out_fh qq{s|$from$suffix|$to$suffix|g;\n};
  }
}


sub print_changes_as_perl_script($$@) {
  my($out_fh, $title, @names) = @_;

  print $out_fh "\n# $title\n";

  for my $entry (@names) {
    my($from, $to, $note)=@$entry;
    $note ||= '';
    print $out_fh qq{s|^(.*)($from)(.*)\$|/\\* WARNING: $from. $note \\*/ \$\{1\}$to\$\{3\}|g;\n};
  }
}


sub print_statement_field_renames_as_perl_script($) {
  my($out_fh)=@_;
  # These are tricky / tedious to deal with entirely by hand but
  # the replacement for subject and object can only be determined by a person
  my(%statement_field_maps) = (
    'subject' => 'subject.value.uri or subject.value.blank.string /* WARNING: must choose one */',
    'subject_type' => 'subject.type',
    'predicate' => 'predicate.value.uri',
    'predicate_type' => 'predicate.type',
    'object' => 'object.value.uri or object.value.literal.string or object.value.blank.string /* WARNING: must choose one */',
    'object_type' => 'object.type',
    'object_literal_datatype' => 'object.value.literal.datatype',
    'object_literal_language' => 'object.value.literal.language'
  );
  print $out_fh "\n# Replace statement fields with term fields.\n";
  while(my($old,$new) = each %statement_field_maps) {
    print $out_fh qq{s|->$old|->$new|g;\n};
  }
  print $out_fh "\n";
}


sub to_id($) {
  my $id=shift;

  $id =~ s/\W/-/g;
  $id =~ s/\-+/-/g;
  $id =~ s/^\-//;
  $id =~ s/\-$//;

  return $id;
}


# main

my $docbook_xml_file = undef;
my $upgrade_script_file = undef;
my $usage = undef;

GetOptions(
  'docbook-xml=s'    => \$docbook_xml_file,
  'upgrade-script=s' => \$upgrade_script_file,
  'package=s'        => \$id_prefix,
  'help|h|?'         => \$usage
) || pod2usage(2);

pod2usage(-verbose => 2) 
  if $usage;

# Arguments
our($package, $file) = @ARGV;

$id_prefix ||= $package;

# Read in data

our $expected_n_fields = 9;

# "$old-$new" versions in order
our(@version_pairs);
# and seen
our(%version_pairs_seen);

# Hashes keyed by $version_pair.  Value is array of descriptive
# arrays specific to each type
my(%new_functions);
my(%deleted_functions);
my(%renamed_functions);
my(%changed_functions);

my(%new_types);
my(%deleted_types);
my(%changed_types);

my(%new_enums);
my(%deleted_enums);
my(%renamed_enums);

open(IN, "<$file") or die "$program: Cannot read $file - $!\n";
while(<IN>) {
  chomp;

  next if /^#/;

  my(@fields)=split(/\t/);
  my $actual_n_fields=scalar(@fields);
  die "$program: Bad line has $actual_n_fields fields expected $expected_n_fields $.: $_\n"
    unless $actual_n_fields == $expected_n_fields;

  if($fields[1] eq 'type') {
    my($old_ver, $dummy1, $old_name, $old_args, $new_ver, $dummy2, $new_name, $new_args,$notes)=@fields;

    my $version_pair = $old_ver."-".$new_ver;
    if(!$version_pairs_seen{$version_pair}) {
      push(@version_pairs, [$old_ver, $new_ver]);
      $version_pairs_seen{$version_pair} = 1;
    }

    $notes = '' if $notes eq '-';

    if($old_name eq '-') {
      push(@{$new_types{$version_pair}}, [$new_name, $notes]);
    } elsif($new_name eq '-') {
      push(@{$deleted_types{$version_pair}}, [$old_name, $notes]);
    } elsif(($old_name eq $new_name) && $notes eq '') {
      # same
    } else {
      # renamed and maybe something else changed - in the notes
      push(@{$changed_types{$version_pair}}, [$old_name, $new_name, $notes]);
    }

  } elsif($fields[1] eq 'enum') {
    my($old_ver, $dummy1, $old_name, $old_args, $new_ver, $dummy2, $new_name, $new_args,$notes)=@fields;

    my $version_pair = $old_ver."-".$new_ver;
    if(!$version_pairs_seen{$version_pair}) {
      push(@version_pairs, [$old_ver, $new_ver]);
      $version_pairs_seen{$version_pair} = 1;
    }

    $notes = '' if $notes eq '-';

    if($old_name eq '-') {
      push(@{$new_enums{$version_pair}}, [$new_name, $notes]);
    } elsif($new_name eq '-') {
      push(@{$deleted_enums{$version_pair}}, [$old_name, $notes]);
    } elsif(($old_name eq $new_name) && $notes eq '') {
      # same
    } else {
      push(@{$renamed_enums{$version_pair}}, [$old_name, $new_name, $notes]);
    }

  } else {
    my($old_ver, $old_return, $old_name, $old_args, $new_ver, $new_return, $new_name, $new_args,$notes)=@fields;

    my $version_pair = $old_ver."-".$new_ver;
    if(!$version_pairs_seen{$version_pair}) {
      push(@version_pairs, [$old_ver, $new_ver]);
      $version_pairs_seen{$version_pair} = 1;
    }

    $notes = '' if $notes eq '-';

    if($old_name eq '-') {
      push(@{$new_functions{$version_pair}}, [$new_return, $new_name, $new_args, $notes]);
    } elsif($new_name eq '-') {
      push(@{$deleted_functions{$version_pair}}, [$old_return, $old_name, $old_args, $notes]);
    } elsif($old_return eq $new_return && $old_name eq $new_name &&
	    $old_args eq $new_args) {
      # same
      warn "$program: Line records no function change old: $old_return $old_name $old_args to new: $new_return $new_name $new_args\n$.: $_\n";
    } elsif($old_return eq $new_return && $old_name ne $new_name &&
	    $old_args eq $new_args) {
      # renamed but nothing else changed
      push(@{$renamed_functions{$version_pair}}, [$old_name, $new_name, $notes]);
    } else {
      # something changed - args and/or return
      push(@{$changed_functions{$version_pair}}, [$old_return, $old_name, $old_args, $new_return, $new_name, $new_args, $notes]);
    }
  }
}
close(IN);



sub version_for_sort($) {
  map { sprintf("%02d", $_) } split(/\./, $_[0]);
}

# Write Docbook XML output

if(defined $docbook_xml_file) {
  my $out_fh = new IO::File;
  $out_fh->open(">$docbook_xml_file");

  our $intro_title = "API Changes";
  our $intro_para = <<"EOT";
This chapter describes the API changes for $package.
EOT

  print_start_chapter_as_docbook_xml($out_fh,
				     $id_prefix.'-changes',
				     $intro_title,
				     $intro_para);
  

    print_start_section_as_docbook_xml($out_fh,
				       $id_prefix.'-changes-intro',
				       "Introduction");
    print $out_fh <<"EOT";
<para>
The following sections describe the changes in the API between
versions including additions, deletions, renames (retaining the same
number of parameters, types and return value type) and more complex
changes to functions, types and enums.
</para>
EOT

    print_end_section_as_docbook_xml($out_fh);

  # Sort by new version, newest first
  for my $vp (sort { version_for_sort($b->[1]) cmp version_for_sort($a->[1]) } @version_pairs) {
    my($old_version, $new_version)= @$vp;
    my $id = to_id($old_version) . "-to-" . to_id($new_version);

    my $version_pair = $old_version."-".$new_version;

    print_start_section_as_docbook_xml($out_fh,
				       $id_prefix.'-changes-'.$id,
				       "Changes between $package $old_version and $new_version");

    my(@f, @t, @e);
    @f = @{$new_functions{$version_pair} || []};
    @t = @{$new_types{$version_pair} || []};
    @e = @{$new_enums{$version_pair} || []};

    if(@f || @t || @e) {
      print_start_section_as_docbook_xml($out_fh,
					 $id_prefix.'-changes-new-'.$id,
					 "New functions, types and enums");
      print_functions_list_as_docbook_xml($out_fh,
					  undef, 1, 1, @f);
      print_types_list_as_docbook_xml($out_fh,
				      undef, 1, 1, @t);
      print_enums_list_as_docbook_xml($out_fh,
				      undef, 1, 1, @e);
      print_end_section_as_docbook_xml($out_fh);
    }

    @f = @{$deleted_functions{$version_pair} || []};
    @t = @{$deleted_types{$version_pair} || []};
    @e = @{$deleted_enums{$version_pair} || []};
    if(@f || @t || @e) {
      print_start_section_as_docbook_xml($out_fh,
					 $id_prefix.'-changes-deleted-'.$id,
					 "Deleted functions, types and enums");
      print_functions_list_as_docbook_xml($out_fh,
					  undef, 0, 0, @f);
      print_types_list_as_docbook_xml($out_fh,
				      undef, 0, 1, @t);
      print_enums_list_as_docbook_xml($out_fh,
				      undef, 0, 1, @e);
      print_end_section_as_docbook_xml($out_fh);
    }
    

    @f = @{$renamed_functions{$version_pair} || []};
    @e = @{$renamed_enums{$version_pair} || []};
    if(@f || @e) {
      print_start_section_as_docbook_xml($out_fh,
					 $id_prefix.'-changes-renamed-'.$id,
					 "Renamed function and enums");
      print_renamed_functions_as_docbook_xml($out_fh,
					     undef,
					     "$old_version function",
					     "$new_version function",
					     @f);
      print_renamed_enums_as_docbook_xml($out_fh,
					 undef,
					 "$old_version enum",
					 "$new_version enum",
					 @e);
      print_end_section_as_docbook_xml($out_fh);
    }
    
    @f = @{$changed_functions{$version_pair} || []};
    @t = @{$changed_types{$version_pair} || []};
    if(@f || @t) {
      print_start_section_as_docbook_xml($out_fh,
					 $id_prefix.'-changes-changed-'.$id,
					 "Changed functions and types");
      print_changed_functions_as_docbook_xml($out_fh,
					     undef,
					     "$old_version function",
					     "$new_version function",
					     @f);
      print_changed_types_as_docbook_xml($out_fh,
					 undef,
					 "$old_version type",
					 "$new_version type",
					 @t);
      print_end_section_as_docbook_xml($out_fh);
   }

   print_end_section_as_docbook_xml($out_fh);

  } # end pair of old/new versions

  print_end_chapter_as_docbook_xml($out_fh);

  $out_fh->close;
}


# Write Upgrade script output

if(defined $upgrade_script_file) {
  my $out_fh = new IO::File;
  $out_fh->open(">$upgrade_script_file");

  print $out_fh "#!/usr/bin/perl -pi~\n";

  for my $vp (@version_pairs) {
    my($old_version, $new_version)= @$vp;

    my $version_pair = $old_version."-".$new_version;

    print $out_fh "# Perl script to upgrade $package $old_version to $new_version\n\n";

    print_statement_field_renames_as_perl_script($out_fh);

    my(@f, @t, @e);

    @f = @{$deleted_functions{$version_pair} || []};
    @t = @{$deleted_types{$version_pair} || []};
    @e = @{$deleted_enums{$version_pair} || []};

    print_deletes_as_perl_script($out_fh, 'Deleted functions',
				 (map { [ $_->[1], $_->[3] ] } @f));

    print_deletes_as_perl_script($out_fh, 'Deleted types',
				 @t);

    print_deletes_as_perl_script($out_fh, 'Deleted enums',
				 @e);

    @f = @{$renamed_functions{$version_pair} || []};
    @e = @{$renamed_enums{$version_pair} || []};
    print_renames_as_perl_script($out_fh, 'Renamed functions', 1,
				 @f);

    print_renames_as_perl_script($out_fh, 'Renamed enums', 0,
				 @e);

    @f = @{$changed_functions{$version_pair} || []};
    @t = @{$changed_types{$version_pair} || []};
    print_changes_as_perl_script($out_fh, 'Changed functions',
				 (map { [ $_->[1], $_->[4], $_->[6] ] } @f));

    print_changes_as_perl_script($out_fh, 'Changed types',
				 @t);

  } # end of version pair loop

  $out_fh->close;
}


exit 0;


__END__

=head1 NAME

process-changes - turn changes TSV into files

=head1 SYNOPSIS

process-changes [options] PACKAGE-NAME TSV-FILE

=head1 OPTIONS

=over 8

=item B<--help>

Give command help summary.

=item B<--docbook-xml> DOCBOOK-XML

Set the output docbook XML file

=item B<--upgrade-script> UPGRADE-SCRIPT-PL

Set the output perl script to upgrade the function and type names
where possible.

=back

=head1 DESCRIPTION

Turn a package's changes TSV file into docbook XML.

=cut
