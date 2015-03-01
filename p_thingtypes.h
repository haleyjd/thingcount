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
//      thingcount info tables
//
//-----------------------------------------------------------------------------

#ifndef P_THINGTYPES_H__
#define P_THINGTYPES_H__

#include "m_dllist.h"

// thingtype classes for output purposes
enum
{
   CLASS_NONE,
   CLASS_MONSTER,   // Monster (attacks/moves/respawns/etc)
   CLASS_NPC,       // Strife NPC or similar interactive but non-hostile
   CLASS_HEALTH,    // Health item
   CLASS_ARMOR,     // Armor item
   CLASS_AMMO,      // Ammunition
   CLASS_WEAPON,    // Weapon
   CLASS_KEY,       // Key item
   CLASS_ARTIFACT,  // Artifact (Heretic/Hexen/Strife)
   CLASS_POWERUP,   // Power-up item
   CLASS_QUEST,     // Quest item (Strife or Hexen)
   CLASS_DECOR,     // Decorative
   CLASS_HAZARD,    // Hazardous, but not an enemy
   CLASS_AMBIENT,   // Ambient sound object
   CLASS_TECHNICAL, // Items like player spawns, teleman, deathmatch starts, etc.
   CLASS_MAX
};

struct thingtype_t
{
   DLListItem<thingtype_t> links;
   int doomednum;
   int classtype;
   const char *name;
};

void P_LoadThingTypes(const char *filename);
thingtype_t *P_ThingTypeForDEN(int doomednum);

#endif

// EOF

