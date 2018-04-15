#!/usr/bin/perl -w
# --------------------------------------------------------------------------- #
#       Copyright (C), AirM2M Comm. Co., Ltd. All rights reserved.            #
# --------------------------------------------------------------------------- #

# --------------------------------------------------------------------------- #
# This document contains proprietary information belonging to AirM2m.         #
# Passing on and copying of this document, use and communication of its       #
# contents is not permitted without prior written authorisation.              #
# --------------------------------------------------------------------------- #
#
# when       who     what, where, why
# YY.MM.DD   ---     ----------------
# --------   ---     --------------------------------------------------------
# 12.01.19   Lifei   Create
#---------------------------------------------------------------------------- #

# ------------------------------------------------------------------------
# 功能:
#     根据USER_DATA_BASE自动生成相应的map_cfg文件
# ------------------------------------------------------------------------

use strict;
use Config;
use File::Find;
use Getopt::Long;
use File::Basename;

my $g_dbg_level = "5";
   #0 No debug info
   #1 Main
   #2 Subroutines
   #3 Loops in Main
   #4 Loops in Subroutines
   #5 all

my $platform_map_cfg_file;
my $output_map_cfg_file;
my $user_data_base;
my $flsh_model;



#/*********************************************************
#  Function: dbg_out
#  Description: 输出调试信息
#  Input:
#    1. 输出信息的等级
#    2. 需要输出的信息
#  Output:
#  Return: 
#  Others:
#     根据当前全局变量g_dbg_level来决定是否需要输出信息
#*********************************************************/
sub dbg_out
{
    my $dbg_level = $_[0];
    my $dbg_info = $_[1];

    if ( $g_dbg_level >= $dbg_level )
    {
       printf "** $dbg_info\n";
    }
}

sub map_cfg_gen_process
{
    my $line;
    my $block_count = int hex($user_data_base) / (64 * 1024);
    my $sector_count= @_;
    my $OUTPUT;

		if($flsh_model eq  "flsh_spi32m")
		{
			$sector_count =   (32 / 8 * 1024 * 1024 -  $block_count *(64 * 1024))/ (4 * 1024);
		}
		else 
		{
			if($flsh_model eq "flsh_spi64m")
			{
				$sector_count = (64 / 8 * 1024 * 1024 -  $block_count *(64 * 1024))/ (4 * 1024);
			}
			else
			{
				printf("333\r\n");
			}
		}
		
    open( INPUT, "<$platform_map_cfg_file" ) or die "Cannot open input platform map cfg file: $platform_map_cfg_file\n";
    open( $OUTPUT, ">$output_map_cfg_file" ) or die "Cannot open output map cfg lod file: $output_map_cfg_file\n";
    while( defined( $line = <INPUT> ) )
    {
			if($line =~  /^[\s].*$flsh_model:[\s].+load[\s]?$/)
			{
	        printf("get!");
			    print $OUTPUT "$line";
			    print $OUTPUT "    {\r\n";
			    print $OUTPUT "            filename \"flash.lod\"\r\n";
			    print $OUTPUT "            sectormap ($block_count x 64k, $sector_count x 4K)\r\n";
			    print $OUTPUT "            fillvalue 0xffffffff\r\n";
			    print $OUTPUT "            fill on\r\n";
			    print $OUTPUT "            saveheader on	\r\n";
			    print $OUTPUT "    }\r\n";
			    
			    while( defined( $line = <INPUT> ))
			    {
			    	if($line =~  /^[\s].*}[\s]?$/)
			    	{
			    	    last;	
			    	}
			    }
			}
			else
			{
			    print $OUTPUT "$line";
			}			
    }
}

sub map_cfg_gen
{
    my $i = 0;
    
    #获取参数
    while( $_[$i] )
    {
        my $key = $_[$i];

        dbg_out("1", "key: $key");
        # -l input lod file
        if ($key eq "-l")
        {
            $i++;
            $platform_map_cfg_file = $_[$i];
            dbg_out("1", "platform_map_cfg_file=$platform_map_cfg_file");
            $i++;
        }

        # -o output combined lod file
        elsif ($key eq "-o" )
        {
            $i++;
            $output_map_cfg_file = $_[$i];
            dbg_out("1", "output_map_cfg_file=$output_map_cfg_file");
            $i++;
        }
        elsif ($key eq "-i" )
        {
            $i++;
            $user_data_base = $_[$i];
            dbg_out("1", "user_data_base=$user_data_base");
            $i++;
        }        
        elsif ($key eq "-f" )
        {
            $i++;
            $flsh_model = $_[$i];
            dbg_out("1", "flsh_model=$flsh_model");
            $i++;
        }        # not valid parameter
        else
        {
            dbg_out("1", "not valid parameter");
        }
    }
    
    if($platform_map_cfg_file eq "" || $output_map_cfg_file eq "")
    {
        print "Parameter error!\n\n";
    }
    
    dbg_out("1", "parameter ok");

    map_cfg_gen_process();
    #platform_lod_checksum();
}

#**********************************************************
# lua
#**********************************************************

#**********************************************************
# openat
#**********************************************************

# ------------------------------------------------------------------------
# Main
# ------------------------------------------------------------------------
if( not defined $ARGV[0])
{
    dbg_out("1", "no parameters");
}
else
{
   map_cfg_gen(@ARGV);
}

exit 0;