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
//      thingcount THINGS lump processing code
//
//-----------------------------------------------------------------------------

#include "z_zone.h"
#include "z_auto.h"
#include "e_hash.h"
#include "e_hashkeys.h"
#include "m_binary.h"
#include "p_thingtypes.h"
#include "w_levels.h"
#include "w_wad.h"

// DOOM mapthing flags
#define MTF_EASY        1
#define MTF_NORMAL      2
#define MTF_HARD        4
#define MTF_AMBUSH      8
#define MTF_NOTSINGLE  16
#define MTF_NOTDM      32
#define MTF_NOTCOOP    64
#define MTF_FRIEND    128
#define MTF_RESERVED  256

// PSX flags
#define MTF_PSX_GHOST      32         // 50% transparent monster
#define MTF_PSX_ADDITIVE  (32|64)     // 100% additive monster
#define MTF_PSX_NIGHTMARE (32|128)    // subtractive w/2x spawn health
#define MTF_PSX_SPECTRE   (32|64|128) // 25% additive monster

// Hexen mapthing flags
#define MTF_HX_EASY           1
#define MTF_HX_NORMAL         2
#define MTF_HX_HARD           4
#define MTF_HX_AMBUSH         8
#define MTF_HX_DORMANT       16
#define MTF_HX_FIGHTER       32
#define MTF_HX_CLERIC        64
#define MTF_HX_MAGE         128
#define MTF_HX_GSINGLE      256
#define MTF_HX_GCOOP        512
#define MTF_HX_GDEATHMATCH 1024

struct mapthing_t
{
   int16_t tid;     // scripting id
   int16_t x;       // x coord
   int16_t y;       // y coord
   int16_t height;  // z height relative to floor
   int16_t angle;   // angle in wad format
   int16_t type;    // doomednum
   int16_t options; // bitflags
   int     special; // scripting special
   int     args[5]; // arguments for special
};

static mapthing_t *things;
static int         numthings;
static int         levelformat;

//
// Load DOOM things
//
static void P_loadDoomThings(wadlevel_t &wl)
{
   int lumpnum = wl.lumpnum + ML_THINGS;
   ZAutoBuffer thinglump;
   wl.dir->cacheLumpAuto(lumpnum, thinglump);

   numthings = wl.dir->lumpLength(lumpnum) / 10;
   things    = estructalloc(mapthing_t, numthings);

   byte *rover = thinglump.getAs<byte *>();
   for(int i = 0; i < numthings; i++)
   {
      mapthing_t *ft = &things[i];

      ft->x       = GetBinaryWord(&rover);
      ft->y       = GetBinaryWord(&rover);
      ft->angle   = GetBinaryWord(&rover);
      ft->type    = GetBinaryWord(&rover);
      ft->options = GetBinaryWord(&rover);

      // remove extended BOOM flags if MTF_RESERVED is set, due to
      // Hellmaker levels
      if(ft->options & MTF_RESERVED)
         ft->options &= ~(MTF_NOTDM|MTF_NOTCOOP|MTF_FRIEND);
   }
}

//
// Load Hexen things
//
static void P_loadHexenThings(wadlevel_t &wl)
{
   int lumpnum = wl.lumpnum + ML_THINGS;
   ZAutoBuffer thinglump;
   wl.dir->cacheLumpAuto(lumpnum, thinglump);

   numthings = wl.dir->lumpLength(lumpnum) / 20;
   things    = estructalloc(mapthing_t, numthings);

   byte *rover = thinglump.getAs<byte *>();
   for(int i = 0; i < numthings; i++)
   {
      mapthing_t *ft = &things[i];

      ft->tid     = GetBinaryWord(&rover);
      ft->x       = GetBinaryWord(&rover);
      ft->y       = GetBinaryWord(&rover);
      ft->height  = GetBinaryWord(&rover);
      ft->angle   = GetBinaryWord(&rover);
      ft->type    = GetBinaryWord(&rover);
      ft->options = GetBinaryWord(&rover);
      ft->special = *rover++;
      memcpy(ft->args, rover, 5);
      rover += 5;
   }
}

//
// Load the THINGS lump for a Doom engine map
//
void P_LoadThings(wadlevel_t &wl)
{
   if(things)
   {
      efree(things);
      things = nullptr;
   }

   levelformat = wl.fmt;

   switch(wl.fmt)
   {
   case LEVEL_FORMAT_DOOM:
   case LEVEL_FORMAT_PSX:
      P_loadDoomThings(wl);
      break;
   case LEVEL_FORMAT_HEXEN:
      P_loadHexenThings(wl);
      break;
   default:
      return; // not supported
   }
}

enum
{
   GAME_TYPE_SINGLE,
   GAME_TYPE_COOP,
   GAME_TYPE_DM,
   NUM_GAME_TYPES
};

// For Hexen, the flag MUST be present
static unsigned int HxFlagForGameType[NUM_GAME_TYPES] =
{
   MTF_HX_GSINGLE,
   MTF_HX_GCOOP,
   MTF_HX_GDEATHMATCH
};

// For Doom (with BOOM extensions enabled, the flag must NOT be present
static unsigned int DoomFlagForGameType[NUM_GAME_TYPES] =
{
   MTF_NOTSINGLE,
   MTF_NOTCOOP,
   MTF_NOTDM
};

static const char *PrettyGameType[NUM_GAME_TYPES] =
{
   "Single Player",
   "Cooperative",
   "Deathmatch"
};

//
// Check if the thing type is present for the given game type
//
static bool P_isInGameType(mapthing_t *thing, int gametype)
{
   switch(levelformat)
   {
   case LEVEL_FORMAT_HEXEN:
      return ((thing->options & HxFlagForGameType[gametype]) == HxFlagForGameType[gametype]);
   default:
      return ((thing->options & DoomFlagForGameType[gametype]) == 0);
   }
}

enum
{
   SKILL_EASY,
   SKILL_NORMAL,
   SKILL_HARD,
   NUM_SKILLS
};

// NB: Same between Doom and Hexen formats
static unsigned int FlagsForSkill[NUM_SKILLS] =
{
   MTF_EASY,
   MTF_NORMAL,
   MTF_HARD
};

//
// Check if the thing type is present for the given skill level
//
static bool P_isInSkill(mapthing_t *thing, int skill)
{
   return ((thing->options & FlagsForSkill[skill]) == FlagsForSkill[skill]);
}

enum
{
   CLASS_FIGHTER,
   CLASS_CLERIC,
   CLASS_MAGE,
   NUM_CLASSES
};

static unsigned int FlagsForClass[NUM_CLASSES] =
{
   MTF_HX_FIGHTER,
   MTF_HX_CLERIC,
   MTF_HX_MAGE
};

static const char *PrettyPClasses[NUM_CLASSES] =
{
   "Fighter",
   "Cleric",
   "Mage"
};

//
// Check if the thing type is present for the given player class
//
static bool P_isInClassGame(mapthing_t *thing, int pclass)
{
   return ((thing->options & FlagsForClass[pclass]) == FlagsForClass[pclass]);
}

struct thingtally_t
{
   DLListItem<thingtally_t> links;
   thingtype_t *type;
   int          doomednum;
   int          counts[NUM_SKILLS];
};

static const char *PrettyClassNames[CLASS_MAX] =
{
   "Unknown",
   "Monsters",    // Monster (attacks/moves/respawns/etc)
   "NPCs",        // Strife NPC or similar interactive but non-hostile
   "Health",      // Health item
   "Armor",       // Armor item
   "Ammo",        // Ammunition
   "Weapons",     // Weapon
   "Keys",        // Key item
   "Artifacts",   // Artifact (Heretic/Hexen/Strife)
   "Powerups",    // Power-up item
   "Quest items", // Quest item (Strife or Hexen)
   "Decor",       // Decorative
   "Hazards",     // Hazardous, but not an enemy
   "Ambience",    // Ambient sound object
   "Technical",   // Items like player spawns, teleman, deathmatch starts, etc.
};

typedef EHashTable<thingtally_t, EIntHashKey,
                   &thingtally_t::doomednum, &thingtally_t::links> tallyhash_t;

//
// Tabulate things on the level:
// * By game mode (single/coop/DM)
// * By skill level
// * By player class (if Hexen)
// * By type
// 
static void P_TabulateThings(int mode, int classtype)
{
   if(!things)
      return;

   tallyhash_t hash;

   // gather up the things present in the given combination of game
   // properties
   for(int i = 0; i < numthings; i++)
   {
      mapthing_t *mt = &things[i];

      if(!P_isInGameType(mt, mode)) // check if in game type
         continue;

      // if Hexen format, check if in class game
      if(levelformat == LEVEL_FORMAT_HEXEN && !P_isInClassGame(mt, classtype))
         continue;

      // find or create a new tally object for this type
      thingtally_t *tt;

      if(!(tt = hash.objectForKey(mt->type)))
      {
         tt = estructalloc(thingtally_t, 1);
         tt->doomednum = mt->type;
         tt->type      = P_ThingTypeForDEN(mt->type); // may return null
         hash.addObject(tt);
      }
      // determine skill levels
      if(P_isInSkill(mt, SKILL_EASY))
         tt->counts[SKILL_EASY]++;
      if(P_isInSkill(mt, SKILL_NORMAL))
         tt->counts[SKILL_NORMAL]++;
      if(P_isInSkill(mt, SKILL_HARD))
         tt->counts[SKILL_HARD]++;
   }

   printf("Game mode: %s\n", PrettyGameType[mode]);
   if(levelformat == LEVEL_FORMAT_HEXEN)
      printf("Player class: %s\n", PrettyPClasses[classtype]);

   // print all tallied objects by class type
   for(int i = 0; i < CLASS_MAX; i++)
   {
      thingtally_t *tt = nullptr;
      bool printedHeader = false;

      while((tt = hash.tableIterator(tt)))
      {
         const char *name;
         if(i != CLASS_NONE && !tt->type) // Unknown go into class NONE
            continue;
         if(tt->type && tt->type->classtype != i)
            continue;

         // don't print the header for this class of objects until one is
         // actually found in the level.
         if(!printedHeader)
         {
            printf("%s:\n", PrettyClassNames[i]);
            printf("  DEN Type                      Easy Norm.  Hard\n");
            printf("================================================\n");
            printedHeader = true;
         }

         name = tt->type ? tt->type->name : "Unknown";
         printf("%5d %-24.24s %5d %5d %5d\n", tt->doomednum, name, 
                tt->counts[SKILL_EASY], tt->counts[SKILL_NORMAL],
                tt->counts[SKILL_HARD]);
      }
      if(printedHeader)
         printf("\n");
   }
   printf("\n");

   // destroy the tally objects
   thingtally_t *tt = nullptr;
   while((tt = hash.tableIterator(tt)))
   {
      hash.removeObject(tt);
      efree(tt);
      tt = nullptr;
   }
}

//
// Output the tables of thing counts for the indicated game types and/or
// player classes. If the counts are passed as -1, all count tables will
// be generated.
//
void P_OutputThingCounts(wadlevel_t &wl, int theType, int theClass)
{
   int starttype;
   int maxtype;
   int startclass;
   int maxclass;

   if(theType == -1) // run for all game types?
   {
      starttype = 0;
      maxtype   = NUM_GAME_TYPES;
   }
   else
   {
      starttype = theType;
      maxtype   = theType + 1;
   }
   if(theClass == -1) // run for all classes?
   {
      startclass = 0;
      maxclass   = NUM_CLASSES;
   }
   else
   {
      startclass = theClass;
      maxclass   = theClass + 1;
   }

   printf("======================%.8s======================\n\n", wl.header);

   for(int type = starttype; type < maxtype; type++)
   {
      if(levelformat == LEVEL_FORMAT_HEXEN)
      {
         for(int pclass = startclass; pclass < maxclass; pclass++)
            P_TabulateThings(type, pclass);
      }
      else
         P_TabulateThings(type, 0);
   }
}

// EOF

