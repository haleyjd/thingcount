/*
  Thing Count

  A command-line utility for printing out information on thing types inside
  DOOM-engine levels.

  Copyright (C) 2015 James Haley [haleyjd@hotmail.com]

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "z_zone.h"
#include "m_argv.h"

//
// Startup Banner
//
static void D_PrintStartupBanner()
{
   puts("thingcount\n"
        "Copyright 2015 James Haley et al.\n"
        "This program is free software distributed under the terms of the GNU General\n"
        "Public License. See the file \"COPYING\" for full details.\n");
}

static const char *usagestr =
"\n"
"thingcount options:\n"
"\n";

//
// D_PrintUsage
//
// Output documentation on all supported command-line parameters and exit.
//
static void D_PrintUsage()
{
   puts(usagestr);
   exit(0);
}

//
// D_CheckForParameters
//
// Check for command line parameters
//
static void D_CheckForParameters()
{
   int p;
   static const char *helpParams[] = { "-help", "-?", nullptr };

   // check for help
   if(myargc < 2 || M_CheckMultiParm(helpParams, 0))
      D_PrintUsage();
}

//
// Initialization
//
static void D_Init()
{
   D_PrintStartupBanner();

   // Zone init
   Z_Init();

   // Check for command line parameters
   D_CheckForParameters();

   // Load PSX wad files from input directory
   //printf("D_LoadInputFiles: Loading PSX wad files...\n");
   //D_LoadInputFiles(baseinputdir);
}

//
// Main Program
//
int main(int argc, char **argv)
{
   // setup m_argv
   myargc = argc;
   myargv = argv;

   // perform initialization
   D_Init();

   return 0;
}

// EOF

