// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 2015 James Haley et al.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//      Management of private wad directories for individual level resources
//
//-----------------------------------------------------------------------------

#include "z_zone.h"
#include "w_levels.h"
#include "w_wad.h"

//
// Map lumps table
//
// For Doom- and Hexen-format maps.
//
static const char *levellumps[] =
{
   "label",    // ML_LABEL,    A separator, name, ExMx or MAPxx
   "THINGS",   // ML_THINGS,   Monsters, items..
   "LINEDEFS", // ML_LINEDEFS, LineDefs, from editing
   "SIDEDEFS", // ML_SIDEDEFS, SideDefs, from editing
   "VERTEXES", // ML_VERTEXES, Vertices, edited and BSP splits generated
   "SEGS",     // ML_SEGS,     LineSegs, from LineDefs split by BSP
   "SSECTORS", // ML_SSECTORS, SubSectors, list of LineSegs
   "NODES",    // ML_NODES,    BSP nodes
   "SECTORS",  // ML_SECTORS,  Sectors, from editing
   "REJECT",   // ML_REJECT,   LUT, sector-sector visibility
   "BLOCKMAP", // ML_BLOCKMAP  LUT, motion clipping, walls/grid element
   "BEHAVIOR"  // ML_BEHAVIOR  haleyjd: ACS bytecode; used to id hexen maps
};

//
// Lumps that are used in console map formats
//
static const char *consolelumps[] =
{
   "LEAFS",
   "LIGHTS",
   "MACROS"
};

//
// haleyjd 12/12/13: Check for supported console map formats
//
static int W_checkConsoleFormat(WadDirectory *dir, int lumpnum)
{
   int          numlumps = dir->getNumLumps();
   lumpinfo_t **lumpinfo = dir->getLumpInfo();

   for(int i = ML_LEAFS; i <= ML_MACROS; i++)
   {
      int ln = lumpnum + i;
      if(ln >= numlumps ||     // past the last lump?
         strncmp(lumpinfo[ln]->name, consolelumps[i - ML_LEAFS], 8))
      {
         if(i == ML_LIGHTS)
            return LEVEL_FORMAT_PSX; // PSX
         else
            return LEVEL_FORMAT_INVALID; // invalid map
      }
   }

   // If we got here, dealing with Doom 64 format. (TODO: Not supported ... yet?)
   return LEVEL_FORMAT_INVALID; //LEVEL_FORMAT_DOOM64;
}

//
// sf 11/9/99: We need to do this now because we no longer have to conform to
// the MAPxy or ExMy standard previously imposed.
//
int W_CheckLevel(WadDirectory *dir, int lumpnum)
{
   int          numlumps = dir->getNumLumps();
   lumpinfo_t **lumpinfo = dir->getLumpInfo();
   
   for(int i = ML_THINGS; i <= ML_BEHAVIOR; i++)
   {
      int ln = lumpnum + i;
      if(ln >= numlumps ||     // past the last lump?
         strncmp(lumpinfo[ln]->name, levellumps[i], 8))
      {
         // If "BEHAVIOR" wasn't found, we assume we are dealing with
         // a DOOM-format map, and it is not an error; any other missing
         // lump means the map is invalid.

         if(i == ML_BEHAVIOR)
         {
            // If the current lump is named LEAFS, it's a console map
            if(ln < numlumps && !strncmp(lumpinfo[ln]->name, "LEAFS", 8))
               return W_checkConsoleFormat(dir, lumpnum);
            else
               return LEVEL_FORMAT_DOOM;
         }
         else
            return LEVEL_FORMAT_INVALID;
      }
   }

   // if we got here, we're dealing with a Hexen-format map
   return LEVEL_FORMAT_HEXEN;
}

//
// Static qsort callback for W_FindAllMapsInLevelWad
//
static int W_sortLevels(const void *first, const void *second)
{
   wadlevel_t *firstLevel  = (wadlevel_t *)first;
   wadlevel_t *secondLevel = (wadlevel_t *)second;

   return strncasecmp(firstLevel->header, secondLevel->header, 9);
}

//
// haleyjd 10/23/10: Finds all valid maps in a wad directory and returns them
// as a sorted set of wadlevel_t's.
//
wadlevel_t *W_FindAllMapsInLevelWad(WadDirectory *dir)
{
   int i, format;
   wadlevel_t *levels = NULL;
   int numlevels;
   int numlevelsalloc;
   int          numlumps = dir->getNumLumps();
   lumpinfo_t **lumpinfo = dir->getLumpInfo();

   // start out with a small set of levels
   numlevels = 0;
   numlevelsalloc = 8;
   levels = estructalloc(wadlevel_t, numlevelsalloc);

   // find all the lumps
   for(i = 0; i < numlumps; i++)
   {
      if((format = W_CheckLevel(dir, i)) != LEVEL_FORMAT_INVALID)
      {
         // grow the array if needed, leaving one at the end
         if(numlevels + 1 >= numlevelsalloc)
         {
            numlevelsalloc *= 2;
            levels = erealloc(wadlevel_t *, levels, numlevelsalloc * sizeof(wadlevel_t));
         }
         memset(&levels[numlevels], 0, sizeof(wadlevel_t));
         levels[numlevels].dir = dir;
         levels[numlevels].lumpnum = i;
         levels[numlevels].fmt = format;
         strncpy(levels[numlevels].header, lumpinfo[i]->name, 9);
         ++numlevels;

         // skip past the level's directory entries
         i += (format == LEVEL_FORMAT_HEXEN ? 11 : 10);
      }
   }

   // sort the levels if necessary
   if(numlevels > 1)
      qsort(levels, numlevels, sizeof(wadlevel_t), W_sortLevels);

   // set the entry at numlevels to all zeroes as a terminator
   memset(&levels[numlevels], 0, sizeof(wadlevel_t));

   return levels;
}

// EOF

