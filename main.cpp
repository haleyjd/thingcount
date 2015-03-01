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
#include "i_system.h"
#include "m_argv.h"
#include "m_collection.h"
#include "p_things.h"
#include "p_thingtypes.h"
#include "w_levels.h"
#include "w_wad.h"

// input file name
static const char *inputfile;

// thingtype script
static const char *thingscript = "scripts/doom.cfg";

// maps specified on the command line
static PODCollection<const char *> maps;

static int gametype = -1; // default: run for all game types
static int pclass   = -1; // default: run for all player classes

// input wad directory object
WadDirectory inputDir;

// list of valid levels to scan
wadlevel_t *wadlevels;

//
// Startup Banner
//
static void D_PrintStartupBanner()
{
   puts("thingcount\n"
        "Copyright 2015 James Haley et al.\n"
        "This program is free software distributed under the terms of the GNU General\n"
        "Public License. See the file \"COPYING\" for full details.");
}

static const char *usagestr =
"\n"
"thingcount options:\n"
"\n"
"-file <archive>\n"
"  Required. List a WAD or PKE/PK3 archive to open.\n"
"-script <scriptfile>\n"
"  Change the thingtype definition script.\n"
"  Default is 'scripts/doom.cfg'\n"
"-maps <map> [<map> ...]\n"
"  Specify one or more maps to tabulate by map header name.\n"
"  By default, all maps will be autodetected and tabulated.\n"
"-gametype <single|coop|dm>\n"
"  Restrict output to a single game type.\n"
"-class <fighter|cleric|mage>\n"
"  Restrict output to a single player class for Hexen maps.\n";

//
// D_PrintUsage
//
// Output documentation on all supported command-line parameters and exit.
//
static void D_PrintUsage()
{
   D_PrintStartupBanner();
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
   if(myargc < 3 || M_CheckMultiParm(helpParams, 0))
      D_PrintUsage();

   // check for input file
   if((p = M_CheckParm("-file")) && p < myargc - 1)
      inputfile = myargv[p + 1];
   else
      I_Error("Need an input file\n");

   // check for script specification
   if((p = M_CheckParm("-script")) && p < myargc - 1)
      thingscript = myargv[p + 1];

   // check for -maps param
   if((p = M_CheckParm("-maps")) && p < myargc - 1)
   {
      ++p;
      while(p != myargc && *myargv[p] != '-')
      {
         maps.add(myargv[p]);
         ++p;
      }
   }

   // check for game type
   if((p = M_CheckParm("-gametype")) && p < myargc - 1)
   {
      const char *type = myargv[p + 1];
      if(!strcasecmp(type, "single"))
         gametype = 0;
      else if(!strcasecmp(type, "coop"))
         gametype = 1;
      else if(!strcasecmp(type, "dm"))
         gametype = 2;
   }

   // check for player class
   if((p = M_CheckParm("-class")) && p < myargc - 1)
   {
      const char *tpclass = myargv[p + 1];
      if(!strcasecmp(tpclass, "fighter"))
         pclass = 0;
      else if(!strcasecmp(tpclass, "cleric"))
         pclass = 1;
      else if(!strcasecmp(tpclass, "mage"))
         pclass = 2;
   }
}

//
// Load the input file. If the -maps command-line option was not specified,
// also scan it for levels.
//
static void D_LoadInput()
{
   // open the file
   if(!inputDir.addNewPrivateFile(inputfile))
      I_Error("Could not load input file '%s'\n", inputfile);

   // if maps to open were not specified on the command line, then scan for
   // them in the directory now
   if(!maps.getLength())
   {
      wadlevels = W_FindAllMapsInLevelWad(&inputDir);
   }
   else
   {
      int wadlevelidx = 0;
      wadlevels = estructalloc(wadlevel_t, maps.getLength() + 1);
      
      // check each named map
      for(auto itr = maps.begin(); itr != maps.end(); itr++)
      {
         int fmt, lumpnum;
         if((lumpnum = inputDir.checkNumForName(*itr)) >= 0)
         {
            if((fmt = W_CheckLevel(&inputDir, lumpnum)) != LEVEL_FORMAT_INVALID)
            {
               wadlevels[wadlevelidx].dir     = &inputDir;
               wadlevels[wadlevelidx].lumpnum = lumpnum;
               wadlevels[wadlevelidx].fmt     = fmt;
               strncpy(wadlevels[wadlevelidx].header, *itr, 9);
               ++wadlevelidx;
            }
         }
      }
   }
}

//
// Initialization
//
static void D_Init()
{
   // Zone init
   Z_Init();

   // Check for command line parameters
   D_CheckForParameters();

   // Load input file and create wadlevels array
   D_LoadInput();

   // Load the thing type script
   P_LoadThingTypes(thingscript);
}

//
// The Main Magic (TM)
//
static void D_ProcessLevels()
{
   wadlevel_t *wl = wadlevels;

   while(wl->dir)
   {
      P_LoadThings(*wl);
      P_OutputThingCounts(*wl, gametype, pclass);
      ++wl;
   }
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

   // print out thing information for each level
   D_ProcessLevels();

   return 0;
}

// EOF

