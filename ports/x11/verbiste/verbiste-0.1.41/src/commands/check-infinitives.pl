#!/usr/bin/perl -w
# $Id: check-infinitives.pl,v 1.8 2012/11/18 19:56:04 sarrazip Exp $
# check-infinitives.pl - Test of french-conjugator
#
# Checks that for each infinitive known to Verbiste, french-conjugator
# returns the same word in the infinitive field of the conjugation.
#
# verbiste - French conjugation system
# Copyright (C) 2003-2010 Pierre Sarrazin <http://sarrazip.com/>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

use strict;
use IPC::Open2;
use XML::Parser;


$| = 1;  # no buffering on STDOUT

my $numErrors = 0;
my ($reader, $writer);  # pipes
my $acc;  # accumulator used by XML parser handlers


sub errmsg
{
    my ($msg) = @_;

    print "$0: ERROR: $msg\n";
    $numErrors++;
}

sub isException
{
    my ($obtained, $expected) = @_;

    if ($expected eq "asseoir" || $expected eq "assoir")
    {
        return $obtained eq "asseoir, assoir";
    }
    if ($expected eq "rasseoir" || $expected eq "rassoir")
    {
        return $obtained eq "rasseoir, rassoir";
    }
    return 0;
}

sub processVerb
{
    my ($verb) = @_;

    #print "processVerb: start: verb=[$verb]\n";

    print $writer "$verb\n";
    my $state = 0;
    my $numLinesRead = 0;
    while (my $line = <$reader>)
    {
        $numLinesRead++;
        if ($state == 0 && $line =~ /^- infinitive present:/)
        {
            $state = 1;
            next;
        }
        if ($state == 1)
        {
            #print "\tINF: $line";
            $line =~ s/\s+$//s;
            if ($line ne $verb && !isException($line, $verb))
            {
                errmsg("wrong infinitive \"$line\" given for verb $verb");
            }
            $state = 2;
            next;
        }
        if ($line =~ /^-\s*\n$/i)  # if end of conjugation
        {
            $state = 0;
            last;
        }
        #print "\t| $line";
    }
    #print "processVerb: end: verb=[$verb], $numLinesRead line(s) read\n";

    if ($numLinesRead == 0)
    {
        errmsg("french-conjugator wrote no lines for verb $verb (possible crash)");
        exit 1;
    }
}

sub handleStart
{
    my ($expat, $el) = @_;

    if ($el eq "i")
    {
        $acc = "";  # start accumulating
    }
}

sub handleChar
{
    my ($expat, $str) = @_;

    if (defined $acc)
    {
        $acc .= $str;
    }
}

sub handleEnd
{
    my ($expat, $el) = @_;

    if (defined $acc)
    {
        processVerb($acc);
        $acc = undef;
    }
}


my $xmlVerbListFilename = shift or die "Missing argument";

# Open french-conjugator and connect to it with 2 pipes.
#
my $cmd = "./french-conjugator";
if (! -e $cmd)
{
    print STDERR "$0: command $cmd not found\n";
    exit 1;
}
if (! -x $cmd)
{
    print STDERR "$0: file $cmd is not executable\n";
    exit 1;
}
my $pid = open2($reader, $writer, $cmd);
if (!defined $pid)
{
    print STDERR "$0: could not start command: $cmd:\n$!\n";
    exit 1;
}

# Open the list of verbs.
my $parser = new XML::Parser(Handlers => {
                Start => \&handleStart,
                Char  => \&handleChar,
                End   => \&handleEnd
                });
$parser->parsefile($xmlVerbListFilename);

close $writer;
close $reader;

waitpid $pid, 0;

exit($numErrors > 0);
