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
#include "e_hash.h"
#include "e_hashkeys.h"
#include "m_misc.h"
#include "p_thingtypes.h"
#include "xl_scripts.h"

//=============================================================================
//
// Data store
//

typedef EHashTable<thingtype_t, EIntHashKey, 
                   &thingtype_t::doomednum, &thingtype_t::links> thingtable_t;

static thingtable_t thingtable;

//=============================================================================
//
// Script parser
//

class XLThingScript : public XLParser
{
protected:
   // state table declaration
   static bool (XLThingScript::*States[])(XLTokenizer &);

   // parser state enumeration
   enum
   {
      STATE_EXPECTDEN,   // expect doomednum
      STATE_EXPECTCLASS, // expect class
      STATE_EXPECTNAME   // expect name
   };

   // state handlers
   bool doStateExpectDEN(XLTokenizer &);
   bool doStateExpectClass(XLTokenizer &);
   bool doStateExpectName(XLTokenizer &);

   // parser state data
   int state;
   int den;
   int classtype;

   virtual bool doToken(XLTokenizer &token);
   virtual void startLump();
   virtual void initTokenizer(XLTokenizer &token);
   virtual void onEOF(bool early);

public:
   XLThingScript()
      : XLParser(""), state(STATE_EXPECTDEN), den(0), classtype(CLASS_NONE)
   {
   }
};

// State table
bool (XLThingScript::* XLThingScript::States[])(XLTokenizer &) =
{
   &XLThingScript::doStateExpectDEN,
   &XLThingScript::doStateExpectClass,
   &XLThingScript::doStateExpectName
};

// Dispatch token to appropriate state handler
bool XLThingScript::doToken(XLTokenizer &token)
{
   return (this->*States[state])(token);
}

// Reinitialize parser at beginning of processing
void XLThingScript::startLump()
{
   state     = STATE_EXPECTDEN;
   den       = 0;
   classtype = CLASS_NONE;
}

// Setup tokenizer before parsing begins
void XLThingScript::initTokenizer(XLTokenizer &token)
{
   token.setTokenFlags(XLTokenizer::TF_SLASHCOMMENTS);
}

// Handle EOF
void XLThingScript::onEOF(bool early)
{
   // TODO: ?
}

// Expecting doomednum
bool XLThingScript::doStateExpectDEN(XLTokenizer &token)
{
   den = token.getToken().toInt();
   state = STATE_EXPECTCLASS;
   return true;
}

static const char *classnames[CLASS_MAX] =
{
   "NONE",
   "MONSTER",  // Monster (attacks/moves/respawns/etc)
   "NPC",      // Strife NPC or similar interactive but usually non-hostile
   "HEALTH",   // Health item
   "ARMOR",    // Armor item
   "AMMO",     // Ammunition
   "WEAPON",   // Weapon
   "KEY",      // Key item
   "ARTIFACT", // Artifact (Heretic/Hexen/Strife)
   "POWERUP",  // Power-up item
   "QUEST",    // Quest item (Strife or Hexen)
   "DECOR",    // Decorative
   "HAZARD",   // Hazardous, but not an enemy
   "AMBIENT",  // Ambient sound object
   "TECHNICAL" // Items like player spawns, teleman, deathmatch starts, etc.
};

// Expecting class
bool XLThingScript::doStateExpectClass(XLTokenizer &token)
{
   classtype = M_StrToNumLinear(classnames, CLASS_MAX, token.getToken().constPtr());
   if(classtype == CLASS_MAX)
      classtype = CLASS_NONE;
   state = STATE_EXPECTNAME;
   return true;
}

// Expecting name
bool XLThingScript::doStateExpectName(XLTokenizer &token)
{
   thingtype_t *tt = estructalloc(thingtype_t, 1);
   tt->classtype = classtype;
   tt->doomednum = den;
   tt->name      = token.getToken().duplicate();
   thingtable.addObject(tt);
   state = STATE_EXPECTDEN;
   return true;
}

//=============================================================================
//
// External interface
//

//
// Load the specified thingtype script
//
void P_LoadThingTypes(const char *filename)
{
   XLThingScript().parseFile(filename);
}

//
// Find a thingtype definition for the given DoomEd number. Returns null if not
// defined by the loaded game configuration.
//
thingtype_t *P_ThingTypeForDEN(int doomednum)
{
   return thingtable.objectForKey(doomednum);
}

// EOF

