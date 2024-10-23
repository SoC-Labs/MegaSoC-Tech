#!/usr/bin/env perl
#######################################################################################################################
#
# The confidential and proprietary information contained in this file may
# only be used by a person authorised under and to the extent permitted
# by a subsisting licensing agreement from Arm Limited or its affiliates.
#
#                (C) COPYRIGHT 2011-2021 Arm Limited or its affiliates.
#                     ALL RIGHTS RESERVED
#
# This entire notice must be reproduced on all copies of this file
# and copies of this file may only be made by a person if such person is
# permitted to do so under the terms of a subsisting license agreement
# from Arm Limited or its affiliates.
#
#######################################################################################################################

#use strict;
#use warnings;
use Getopt::Long;

my $input_file  = "test_s.hex";
my $output_file = "test_s.hex";
my $add_offset  = "0";
my $sub_offset  = "10000000";

    # Parse our command-line arguments
    GetOptions ("in=s"          => \$input_file,
                "out=s"         => \$output_file,
                "add-offset=s"  => \$add_offset,
                "sub-offset=s"  => \$sub_offset)
    or die("Error in command line arguments\n");
    print "Input: $input_file, Output: $output_file\n";

my $input_lines;
my $output_lines;

    # Slurp our file
    open(INFILE, $input_file) or die "Error opening input file $input_file: $!\n";
    {local $/; undef $/; $input_lines = <INFILE>;}
    close INFILE;
    
    # Generate 32 bits/line and re-align addresses
    $output_lines = $input_lines;
undef $input_lines;
    $output_lines =~ s|(\S\S) (\S\S) (\S\S) (\S\S)\s*|$4$3$2$1\n|g;
    $output_lines =~ s|\n+|\n|g;
    $output_lines =~ s|\@(\S+)|"\@".sprintf("%X", (hex($1)-hex($sub_offset)+hex($add_offset))/4)|ge;
    
    # Write contents to target file
    open(my $OUTFILE, '>', $output_file) or die "Error opening output file $output_file: $!\n";
    print $OUTFILE $output_lines;
    close $OUTFILE;

__END__  

