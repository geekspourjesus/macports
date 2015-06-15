#!/usr/bin/perl -w
# $Id: test-french-deconjugator.pl,v 1.2 2012/11/18 19:56:04 sarrazip Exp $
# french-deconjugator-test.pl - Test for the french-deconjugator command
# 
# verbiste - French conjugation system
# Copyright (C) 2003 Pierre Sarrazin <http://sarrazip.com/>
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


my $cmd = "french-deconjugator";
my ($reader, $writer);
my $pid = open2($reader, $writer, $cmd);
if (!defined $pid)
{
    print STDERR "Could not start command: $cmd:\n$!\n";
    exit 1;
}

while (1)
{
    print "\n", "Conjugated verb? ";
    $_ = <STDIN>;
    if (!defined)
    {
	print "\n";
	last;
    }
    print $writer $_;
    while (<$reader>)
    {
	last if /^\n$/;
	print "\t", $_;
    }
}

close $writer;
close $reader;

waitpid $pid, 0;
