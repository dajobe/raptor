#!/usr/bin/perl -w
#
# manifest.pl - Run Raptor against RDF Core tests Manifest.rdf
#
# $Id$
#
# Copyright (C) 2002 David Beckett - http://purl.org/net/dajobe/
# Institute for Learning and Research Technology - http://www.ilrt.org/
# University of Bristol - http://www.bristol.ac.uk/
# 
# This package is Free Software or Open Source available under the
# following licenses (these are alternatives):
#   1. GNU Lesser General Public License (LGPL)
#   2. GNU General Public License (GPL)
#   3. Mozilla Public License (MPL)
# 
# See LICENSE.html or LICENSE.txt at the top of this package for the
# full license terms.
#
# USAGE: ./manifest.pl
#  to run all known tests from $manifest_URL
#
#   ./manifest.pl TEST-URL ...
# to run particular tests (slightly more verbose).
#

use strict;

use File::Basename;
use LWP::Simple;

my $progname = basename $0;

my $offline=0;

my $global_verbose=0;

my $manifest_URL='http://www.w3.org/2000/10/rdf-tests/rdfcore/Manifest.rdf';
my $local_tests_url='http://www.w3.org/2000/10/rdf-tests/rdfcore/';

# CHANGE THIS for your system
my $local_tests_area="./rdfcore/";
my $local_manifest_file="./rdfcore/Manifest.rdf";

my $format="%-6s %5d %7.2f%%\n";


sub get_test_content($$) {
  my($url,$out_file)=@_;
  if ($url =~ m%^$local_tests_url(.*)$%) {
    my $in_file=$local_tests_area.$1;

    return $in_file if -r $in_file;

    open(IN, "<$in_file") or die "Cannot read $in_file ($url) - $!\n";
    open(OUT, ">$out_file") or die "Cannot write to $out_file - $!\n";
    print OUT join('', <IN>);
    close(IN);
    close(OUT);
  } else {
    return if $offline;
    mirror($url, $out_file);
  }

  return $out_file;
}


sub run_test($$$$$$) {
  my($test_url, $test_status,
     $is_positive, $is_verbose, $rdfxml_url, $ntriples_url)=@_;

  my $rdfxml_file=get_test_content($rdfxml_url, 'test.rdf');
  my $ntriples_file=$is_positive ? get_test_content($ntriples_url, 'test.nt') : undef;
 
  if(!-r $rdfxml_file || ($is_positive && ! -r $ntriples_file)) {
    return undef;
  }

  my $plabel=($is_positive) ? 'Positive' : 'Negative';

  if($is_verbose) {
    print "$progname: $plabel Test $test_url ($test_status)\n";
    print "  Input RDF/XML $rdfxml_url - $rdfxml_file \n";
    print "  Output N-Triples $ntriples_url - $ntriples_file\n"
      if $ntriples_url;
  }

  my $cmd="./rapper -q --m strict -o ntriples $rdfxml_file '$rdfxml_url'";
  
  unlink 'test.err', 'parser.nt', 'expected.nt', 'test.ntc';
  my $status=system("$cmd 2>test.err >parser.nt");
  $status = ($status & 0xff00) >> 8;

  if($status > 128) {
    return ('SYSTEM', "$cmd failed - returned status $status\n");
  }

  # Nothing to compare to, it must be a negative test
  if(!$ntriples_url) { # && !$is_positive
    if($status) {
      return ('BAD', "$cmd failed\n");
    }
    return ('OK', "$cmd succeeded");
  }

  open(NT,"<$ntriples_file") or die "Cannot read $ntriples_file - $!\n";
  open(NT2,">expected.nt") or die "Cannot create expected.nt - $!\n";
  while(<NT>) {
    next if /^\s*\#/;
    next if /^\s*$/;
    print NT2;
  }
  close(NT);
  close(NT2);

  $cmd="./ntc parser.nt expected.nt > test.ntc";
  $status=system("$cmd 2>&1 >/dev/null");
  $status = ($status & 0xff00) >> 8;

  if($status > 2) {
    return ('SYSTEM', "$cmd failed - returned status $status\n");
  }

  if($status == 1) {
    my $msg=`diff -u expected.nt parser.nt | tail +3`;
    return ('BAD', "N-Triples match failed\n$msg");
  }

  if($status == 2) {
    return ('PRINTING', "N-Triples matched with printings\n");
  }

  return ('OK', "N-Triples matched");
}


sub read_err($) {
  my($err)=@_;
  open(ERR,"<$err") or die "Cannot read $err - $!\n";
  $err=join('', grep(!/(?:raptor_|^\s*attributes|^\s*$)/, <ERR>));
  close(ERR);
  return $err;
}

sub run_tests($$$$$@) {
  my($tests,$test_statuses,$is_verbose,$results,$totals,@test_urls)=@_;

  for my $test (@test_urls) {

    if(!$tests->{$test}) {
      print "$progname: No such test $test, skipping\n";
      next;
    }
       
    my $is_positive=$tests->{$test}->{positive};
    my $plabel=($is_positive) ? 'Positive' : 'Negative';
    my $inputs=$tests->{$test}->{'test:inputDocument'};
    my $outputs=$tests->{$test}->{'test:outputDocument'};

    if(!defined $inputs || !$inputs->{'test:RDF-XML-Document'}
       ||scalar(@{$inputs->{'test:RDF-XML-Document'}}) !=1) {
      die "$plabel Test $test has not got exactly 1 input RDF/XML file\n";
    }
    
    if($is_positive) {
      if(!defined $outputs || !$outputs->{'test:NT-Document'}
	 ||scalar(@{$outputs->{'test:NT-Document'}}) !=1) {
	die "$plabel Test $test has not got exactly 1 output N-Triple file\n";
      }
    }

    my $input_rdfxml=$inputs->{'test:RDF-XML-Document'}->[0];
    my $output_ntriples=$is_positive ? $outputs->{'test:NT-Document'}->[0] : undef;

    my($result,$msg)=run_test($test, $test_statuses->{$test}, 
			      $is_positive, $is_verbose, 
			      $input_rdfxml, $output_ntriples);

    if($result eq 'SYSTEM') {
      my $err=read_err('test.err');
      $msg .= "\n".$err;
      print "Test $test SYSTEM ERROR: $msg\n";
      push(@{$totals->{SYSTEM}}, $test);
    } elsif($result eq 'OK') {
      if($is_positive) {
	push(@{$totals->{OK}}, $test);
      } else {
	open(TOUT,"<parser.nt") or die "Cannot read parser.nt - $!\n";
	my $out=join("\n  ", <TOUT>);
	close(TOUT);
	$msg= "  ".$out;
	print "$plabel Test $test SUCCEEDED, should have failed - returned: \n$msg\n";
	push(@{$totals->{BAD}}, $test);
      }
    } else {
      if($is_positive) {
	my $err=read_err('test.err');
	$msg .= "\n".$err;
	print "$plabel Test $test FAILED - $msg\n";
	push(@{$totals->{BAD}}, $test);
      } else {
	push(@{$totals->{OK}}, $test);
      }
    }

    $results->{$test}=$result;
  }

}


sub summarize_results($$$$;$) {
  my($title, $results, $totals, $total, $verbose)=@_;

  print "Results for $title\n";
  for my $type (sort keys %$totals) {
    my(@rt)=@{$totals->{$type}};
    my(@short_rt)=map {s/^$local_tests_url//; $_} @rt;
    print sprintf($format,$type, scalar(@rt), (int(scalar(@rt)/$total*10000))/100);
    if($verbose) {
      print "  ",join("\n  ",@rt),"\n";
    }
  }
  print sprintf($format, 'TOTAL', $total, '100');
}


my(%tests);
my(%test_statuses);

my(@positive_test_urls);
my(@negative_test_urls);
my(@approved_positive_test_urls);
my(@approved_negative_test_urls);
my(@unapproved_positive_test_urls);
my(@unapproved_negative_test_urls);

if($offline) {
  print "$progname: OFFLINE - using stored manifest URL $manifest_URL\n";
} else {
  print "$progname: Checking mirrored manifest URL $manifest_URL\n";
  if(mirror($manifest_URL, $local_manifest_file ) == RC_NOT_MODIFIED) {
    print "$progname: OK, not modified\n";
  } else {
    print "$progname: Unknown error\n";
  }
}

# Content from the file
my $content='';
open(IN, "<$local_manifest_file") or die "Cannot read $local_manifest_file - $!\n";
$content .= join('',<IN>);
close($content);

# Remove comments
1 while $content =~ s/<!--.+?-->//gms;

# Unify blanks
$content =~ s/\s+/ /gs;
$content =~ s/\s*$//s;

# Remove everything but tests
$content =~ s%^<\?xml version="1.0"\?> <rdf:RDF[^>]+>%%;
$content =~ s%</rdf:RDF>$%%s;

# Find the tests
while(length $content) {
  $content =~ s/^\s+//;
  last if !length $content;
  if($content =~ s%^<test:(Positive|Negative)ParserTest rdf:about="([^"]+)">(.+?)</test:(Positive|Negative)ParserTest>%%) { # "
    my($type,$url,$test_content)=($1,$2,$3);
    while(length $test_content) {
      $test_content =~ s/^\s+//;
      last if !length $test_content;
      if($test_content =~ s%^<(\S+) rdf:resource="([^"]+)"\s*/>%%) { #"
        $tests{$url}->{$1}=$2;
      } elsif ($test_content =~ s%^<(test:\w+)>\s*<(test:[-\w]+) rdf:about="([^"]+)"\s*/>\s*</(test:\w+)>%%) { #"
        push(@{$tests{$url}->{$1}->{$2}}, $3);
      } elsif ($test_content =~ s%^<(test:\w+)>([^<]+)</test:\w+>%%) {
        $tests{$url}->{$1}=$2;
      } elsif ($test_content =~ s%^<test:\w+>\s*<test:[-\w]+\s*/>\s*</test:\w+>\s*%%) {
      } else {
	die "I'm stumped 1 at test content >>$test_content<<\n";
      }

    }

    my $test_status=$tests{$url}->{'test:status'} || '';

    $test_statuses{$url}=$test_status;

    if ($test_status =~ /^OBSOLETE/) {
      #print "$progname: Ignoring Obsolete Test URL $url\n";
      next;
    }
 
    if ($type eq 'Positive') {
      push(@positive_test_urls, $url);
      $tests{$url}->{positive}=1;
    } else {
      push(@negative_test_urls, $url);
    }

    if ($test_status eq 'APPROVED') {
      if ($type eq 'Positive') {
	push(@approved_positive_test_urls, $url);
      } else {
	push(@approved_negative_test_urls, $url);
      }
    } elsif ($test_status eq 'NOT_APPROVED' || $test_status eq 'PENDING') {
      if ($type eq 'Positive') {
	push(@unapproved_positive_test_urls, $url);
      } else {
	push(@unapproved_negative_test_urls, $url);
      }
    } else { 
      print "$progname: Ignoring test with unknown test:status $test_status\n";
      next;
    }

  } elsif($content =~ s%^<test:(Positive|Negative)EntailmentTest rdf:about="([^"]+)">(.+?)</test:(Positive|Negative)EntailmentTest>%%) { # "
    my($type,$url,$test_content)=($1,$2,$3);
    while(length $test_content) {
      $test_content =~ s/^\s+//;
      last if !length $test_content;
      if($test_content =~ s%^<(\S+) rdf:resource="([^"]+)"\s*/>%%) { #"
        $tests{$url}->{$1}=$2;
      } elsif ($test_content =~ s%^<(test:\w+)>\s*<(test:[-\w]+) rdf:about="([^"]+)"\s*/>\s*</(test:\w+)>%%) { #"
        push(@{$tests{$url}->{$1}->{$2}}, $3);
      } elsif ($test_content =~ s%^<(test:\w+)>([^<]+)</test:\w+>%%) {
        $tests{$url}->{$1}=$2;
      } elsif ($test_content =~ s%^<test:\w+>\s*<test:[-\w]+\s*/>\s*</test:\w+>\s*%%) {
      } else {
  	die "I'm stumped 2 at test content >>$test_content<<\n";
      }
    }

    my $test_status=$tests{$url}->{'test:status'} || 'not APPROVED';
    #print "$progname: Ignoring $type Entailment Test URL $url ($test_status)\n";
  } elsif($content =~ s%^<test:MiscellaneousTest rdf:about="([^"]+)">(.+?)</test:MiscellaneousTest>%%) { # "
    my($url,$test_content)=($1,$2);
    while(length $test_content) {
      $test_content =~ s/^\s+//;
      last if !length $test_content;
      if($test_content =~ s%^<(\S+) rdf:resource="([^"]+)"\s*/>%%) { #"
        $tests{$url}->{$1}=$2;
      } elsif ($test_content =~ s%^<(test:\w+)>\s*<(test:[-\w]+) rdf:about="([^"]+)"\s*/>\s*</(test:\w+)>%%) { #"
        push(@{$tests{$url}->{$1}->{$2}}, $3);
      } elsif ($test_content =~ s%^<(test:\w+)>([^<]+)</test:\w+>%%) {
        $tests{$url}->{$1}=$2;
      } elsif ($test_content =~ s%^<test:\w+>\s*<test:[-\w]+\s*/>\s*</test:\w+>\s*%%) {
      } else {
  	die "I'm stumped 3 at test content >>$test_content<<\n";
      }
    }
    my $test_status=$tests{$url}->{'test:status'} || 'not APPROVED';
    #print "$progname: Ignoring Miscellaneous Test URL $url ($test_status)\n";
  } else {
    die "I'm stumped 4 at content >>$content<<\n";
  }
}


print "$progname: Parser tests found:\n";
print "$progname:   Positive: ",scalar(@positive_test_urls),"\n";
print "$progname:   Negative: ",scalar(@negative_test_urls),"\n";

print "$progname: APPROVED parser tests found:\n";
print "$progname:   Positive: ",scalar(@approved_positive_test_urls),"\n";
print "$progname:   Negative: ",scalar(@approved_negative_test_urls),"\n";

print "$progname: not APPROVED parser tests found:\n";
print "$progname:   Positive: ",scalar(@unapproved_positive_test_urls),"\n";
print "$progname:   Negative: ",scalar(@unapproved_negative_test_urls),"\n";

print "\n\n";


my(%results);

if(@ARGV) {
  my(%totals);
  print "$progname: Running user parser tests:\n";
  run_tests(\%tests, \%test_statuses, 1, \%results, \%totals, @ARGV);

  summarize_results("User Parser Tests", \%results, \%totals, scalar(@ARGV));
  exit 0;
}


my(%approved_positive_totals)=();
run_tests(\%tests, \%test_statuses, $global_verbose, \%results, \%approved_positive_totals, @approved_positive_test_urls);
summarize_results("Positive Approved Parser Tests", \%results, \%approved_positive_totals, scalar(@approved_positive_test_urls));

print "\n\n";

my(%approved_negative_totals)=();
run_tests(\%tests, \%test_statuses, $global_verbose, \%results, \%approved_negative_totals, @approved_negative_test_urls);
summarize_results("Negative Approved Parser Tests", \%results, \%approved_negative_totals, scalar(@approved_negative_test_urls));

print "\n\n";

exit 0;

my(%unapproved_positive_totals)=();
run_tests(\%tests, \%test_statuses, $global_verbose, \%results, \%unapproved_positive_totals, @unapproved_positive_test_urls);
summarize_results("Not APPROVED Positive Parser Tests", \%results, \%unapproved_positive_totals, scalar(@unapproved_positive_test_urls),1);

print "\n\n";

my(%unapproved_negative_totals)=();
run_tests(\%tests, \%test_statuses, $global_verbose, \%results, \%unapproved_negative_totals, @unapproved_negative_test_urls);
summarize_results("Not APPROVED Negative Parser Tests", \%results, \%unapproved_negative_totals, scalar(@unapproved_negative_test_urls),1);
