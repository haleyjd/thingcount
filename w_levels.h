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

#ifndef W_LEVELS_H__
#define W_LEVELS_H__

// haleyjd 10/03/05: let P_CheckLevel determine the map format
enum
{
   LEVEL_FORMAT_INVALID,
   LEVEL_FORMAT_DOOM,
   LEVEL_FORMAT_HEXEN,
   LEVEL_FORMAT_PSX,
   LEVEL_FORMAT_DOOM64
};

// Lump order in a map WAD: each map needs a couple of lumps
// to provide a complete scene geometry description.
enum 
{
  ML_LABEL,             // A separator, name, ExMx or MAPxx
  ML_THINGS,            // Monsters, items..
  ML_LINEDEFS,          // LineDefs, from editing
  ML_SIDEDEFS,          // SideDefs, from editing
  ML_VERTEXES,          // Vertices, edited and BSP splits generated
  ML_SEGS,              // LineSegs, from LineDefs split by BSP
  ML_SSECTORS,          // SubSectors, list of LineSegs
  ML_NODES,             // BSP nodes
  ML_SECTORS,           // Sectors, from editing
  ML_REJECT,            // LUT, sector-sector visibility
  ML_BLOCKMAP,          // LUT, motion clipping, walls/grid element
  ML_BEHAVIOR,          // haleyjd 10/03/05: behavior, used to id hexen maps

  // PSX
  ML_LEAFS = ML_BEHAVIOR, // haleyjd 12/12/13: for identifying console map formats

  // Doom 64
  ML_LIGHTS,
  ML_MACROS
};

class WadDirectory;

struct wadlevel_t
{
   char header[9];    // header lump name
   int  lumpnum;      // lump number, relative to directory
   int  fmt;          // level format
   WadDirectory *dir; // parent directory
};

int         W_CheckLevel(WadDirectory *dir, int lumpnum);
wadlevel_t *W_FindAllMapsInLevelWad(WadDirectory *dir);

#endif

// EOF

