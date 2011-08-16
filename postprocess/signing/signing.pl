:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

use strict;
use Getopt::Long;

my $debug = 0;
my $max_files = 20; 		  # sign $max_files with one command line

#### globals #####
my $myname 		= "";
my $opt_dir 	= "";
my $opt_exclude = "";         # file with a list of not signable dll and exe files
my $opt_verbose = 0;
my $opt_help	= 0;
my $opt_log		= "";		  # for logging
my $opt_pass	= "";         # password for signing
my $opt_pfxfile = "";		  # Personal Information Exchange file
my $opt_timestamp_url = "";   # timestamp url
my %exclude_files = ();  	  # list of not signable dll and exe files
my $signtool    = "signtool.exe sign";
my @args		= ();
my @files_to_sign = ();

#### main #####
$myname = script_id();
if ( $#ARGV < 2 ) {
	usage();
    exit(1);
}
@args = parse_options();
get_exclude_files();
@files_to_sign = get_files(\@args);
if ( $opt_log ) {               # logging
	open(LOG,">$opt_log") || die "Can't open log file $opt_log\n";
}
sign_files(\@files_to_sign);
close LOG if ($opt_log);        # logging
exit 0;


#### subroutines ####

sub script_id
{
    ( my $script_name = $0 ) =~ s/^.*[\\\/]([\w\.]+)$/$1/;

    my $script_rev;
    my $id_str = ' $Revision$ ';
    $id_str =~ /Revision:\s+(\S+)\s+\$/
      ? ($script_rev = $1) : ($script_rev = "-");
#    print "\n$script_name -- version: $script_rev\n";
    return $script_name;
}

############################################################################
sub parse_options		#09.07.2007 08:13
############################################################################
{
	# e exclude list file
	# v verbose
	my $success = GetOptions('h' => \$opt_help,
         'd=s' => \$opt_dir, 'e=s'=>\$opt_exclude, 'f=s'=>\$opt_pfxfile, 'l=s'=>\$opt_log,
		 'p=s'=>\$opt_pass,'v'=>\$opt_verbose, 't=s'=>\$opt_timestamp_url);
    if ( !$success || $opt_help ) {
        usage();
        exit(1);
    }
	if ( !$opt_exclude || !$opt_pfxfile || !$opt_pass || !$opt_timestamp_url) {
		print "ERROR: Parameter missing!\n!";
        usage();
        exit(1);
	}
	return @ARGV;
}	##parse_options

############################################################################
sub get_exclude_files		#09.07.2007 10:12
############################################################################
{
	if ( -e $opt_exclude ) {
            # get data from cache file
            open( IN, "<$opt_exclude") || die "Can't open exclude file $opt_exclude\n";
            while ( my $line = <IN> ) {
			chomp($line);
			$exclude_files{$line} = 1;			# fill hash
			print "$line - $exclude_files{$line}\n" if ($debug);
            }
        } else
        {
			print_error("Can't open $opt_exclude file!\n");
		}
}	##get_exclude_files

############################################################################
sub get_files		#10.07.2007 10:19
############################################################################
 {
	use File::Basename;
    my $target = shift;
	my $file_pattern;
	my $file;
	my @files = ();
	print "\n";
	foreach $file_pattern ( @$target )
	{
		print "Files: $file_pattern\n";
        foreach $file ( glob( $file_pattern ) )
		{
            my $lib = File::Basename::basename $file;
			if ( ! $exclude_files{$lib} ) {
				push @files,$file;
			}
			else
			{
				print "exclude=$lib\n" if ($opt_verbose);
			}
		}
	}
	print "\n";
	return @files;
}	##get_files

############################################################################
sub sign_files		#09.07.2007 10:36
############################################################################
{
	my $files_to_sign = shift;
	my $commandline_base = ""; # contains whole stuff without the file name
	my $file = "";
	my $result = "";

	print_error("Can't open PFX file: $opt_pfxfile\n") if ( ! -e $opt_pfxfile );
	print_error("Password is empty\n") if ( !$opt_pass );
	if ( $opt_pass =~ /\.exe$/ ) {
		# get password by tool
		open(PIPE, "$opt_pass 2>&1 |") || die "Can't open PIPE!\n";
		my $pass = <PIPE>;
		close PIPE;
		print_error("Can't get password!\n") if ( !$pass ); # exit here
		$opt_pass = $pass;
	}
	$signtool .= " -v" if ($opt_verbose);
	$commandline_base = $signtool . " " . "-f $opt_pfxfile -p $opt_pass -t $opt_timestamp_url";

	# Here switch between:
	# one command line for muliple files (all doesn't work, too much) / for each file one command line
	if ( $max_files > 1 ) {
		exec_multi_sign($files_to_sign, $commandline_base);
	} else
	{
		exec_single_sign($files_to_sign, $commandline_base);
	}
}	##sign_files

############################################################################
sub exec_single_sign		#11.07.2007 09:05
############################################################################
{
	my $files_to_sign    = shift;
	my $commandline_base = shift; 				  # contains whole stuff without the file name
	my $file = "";
	my $commandline = "";

	foreach $file (@$files_to_sign)
	{
		$commandline = $commandline_base . " $file";
		print "$commandline\n" if ($debug);
		execute($commandline);
	} #foreach
}	##exec_single_sign

############################################################################
sub exec_multi_sign		#11.07.2007 08:56
############################################################################
 {
	# sign multiple file with one command line
	my $files_to_sign    = shift;
	my $commandline_base = shift; 				  # contains whole stuff without the file name
	my $commandline = $commandline_base;	      # contains stuff which will be executed
	my $file = "";
	my $counter = 0;

	foreach $file (@$files_to_sign)
	{
		$commandline .= " $file";
		++$counter;
		if ( $counter >= $max_files ) {
			execute($commandline);
			$counter = 0;						 # reset counter
			$commandline = $commandline_base;    # reset command line
		}
	}
	execute($commandline) if ($counter > 0);
}	##exec_multi_sign

############################################################################
sub execute		#11.07.2007 10:02
############################################################################
{
	my $commandline = shift;
	my $result = "";

  	print "$commandline\n" if ($debug);
  	open(PIPE, "$commandline 2>&1 |") || die "Error: Cant open pipe!\n";
  	while ( $result = <PIPE> ) {
  		print LOG "$result" if ($opt_log);        # logging
  		if ( $result =~ /SignTool Error\:/ ) {
			close PIPE;
  			print_error( "$result\n" );
  		} # if error
  	} # while
  	close PIPE;
}	##execute

############################################################################
sub print_error		#09.07.2007 11:21
############################################################################
 {
	my $text = shift;
	print "ERROR: $text\n";
	print LOG "ERROR: $text\n" if ($opt_log);        # logging
	close LOG if ($opt_log);        				 # logging
	exit(1);
}	##print_error

############################################################################
sub usage		#09.07.2007 08:39
############################################################################
 {
	print "Usage:\t $myname <-e filename> <-f filename> <-p password> <-t timestamp> [-l filename] [-v] <file[list]> \n";
    print "Options:\n";
	print "\t -e filename\t\t\tFile which contains a list of files which don't have to be signed.\n";
    print                            "Mandatory.\n";
    print "\t -f pfx_filename\t\t\"Personal Information Exchange\" file. ";
    print                            "Mandatory.\n";
    print "\t -p password\t\t\tPassword for \"Personal Information Exchange\" file. Mandatory.\n";
    print "\t -t timestamp\t\t\tTimestamp URL e.g. \"http://timestamp.verisign.com/scripts/timstamp.dll\"\n";
    print "\t\t\t\t\tMandatory.\n";
	print "\t -l log_filename\t\tFile for logging.\n";
    print "\t -v\t\t\t\tVerbose.\n";
}	##usage




