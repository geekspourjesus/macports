#!/usr/bin/perl -w
#
# This script prints the list of all conjugated forms known to Verbiste.
#
# Pass --inf or --infinitives to make it print only the infinitive forms.
#
# Some forms can appear twice.  Piping the output of this script to the
# 'sort -uf' command eliminates the duplicates while sorting the result.
#
# The french-conjugator command must be in the PATH.
#
# Takes about 4 seconds to execute when output is redirected to a file,
# as of 2013.

use strict;
use XML::Parser;
use IPC::Open2;


my @stack = ();
my $acc;
my @infinitives;  # filled by End()
my $printOnlyInfinitives = 0;


sub Start
{
    my ($expat, $el, %att) = @_;

    push @stack, $el;
    $acc = "";
}

sub Char
{
    my ($expat, $str) = @_;

    $acc .= $str if defined $acc;
}

sub End
{
    my ($expat, $el) = @_;

    die unless $el eq pop @stack;

    if ($el eq "i")
    {
	die unless defined $acc;
	die unless length($acc) > 0;

        print "$acc\n" if $printOnlyInfinitives;

        push @infinitives, $acc;
    }

    $acc = undef;
}


$printOnlyInfinitives = (@ARGV > 0 && $ARGV[0] =~ /^--inf(initives?)?$/);

binmode STDOUT, ":utf8" if $printOnlyInfinitives;

my $parser = new XML::Parser(Handlers => {
	Start => \&Start,
	Char => \&Char,
	End => \&End,
	});

$parser->parsefile("../data/verbs-fr.xml");

exit 0 if $printOnlyInfinitives;


my ($fromChild, $toChild);
my $pid = open2($fromChild, $toChild, "french-conjugator") or die;
binmode $toChild, ":utf8";

for my $inf (@infinitives)
{
    print $toChild "$inf\n";

    my $numLinesRead = 0;
    while (<$fromChild>)
    {
        ++$numLinesRead;
        s/\R$//;  # remove line ending sequence (\n, \r\n, etc)
        last if $_ eq "-";  # a lone dash finishes a conjugation
        next if /^$/ || /^-/;  # skip undefined forms and tense headers
        my @words = split /, /, $_;  # alternative conjugations are separated by commas
        print "$_\n" foreach (@words);  # print each alternative on its own line
    }

    die "inf=$inf" unless $numLinesRead == 63;  # check that each infinitive is recognized
}

close($toChild);
close($fromChild);

waitpid($pid, 0);
my $status = $? >> 8;
exit($status);
