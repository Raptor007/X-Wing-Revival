/*
 *  XWingDefs.h
 */

#pragma once

#include "PlatformSpecific.h"
#include "RaptorDefs.h"


namespace XWing
{
	namespace State
	{
		enum
		{
			LOBBY = Raptor::State::GAME_SPECIFIC,
			COUNTDOWN,
			FLYING,
			ROUND_WILL_END,
			ROUND_ENDED
		};
	}
	
	namespace Packet
	{
		enum
		{
			LOBBY = 'Loby',
			FLY = 'Fly ',
			ROUND_ENDED = 'Fin ',
			TIME_REMAINING = 'Time',
			EXPLOSION = 'Boom',
			SHOT_HIT_SHIP = 'HitS',
			SHOT_HIT_TURRET = 'HitT',
			SHOT_HIT_HAZARD = 'HitH',
			MISC_HIT_SHIP = '?Hit',
			AMMO_UPDATE = 'Ammo'
		};
	}
	
	namespace Object
	{
		enum
		{
			SHIP = 'Ship',
			SHOT = 'Shot',
			ASTEROID = 'Rock',
			TURRET = 'Turr',
			DEATH_STAR = 'Moon',
			DEATH_STAR_BOX = 'aBox'
		};
	}
	
	namespace Team
	{
		enum
		{
			NONE = 0,
			AUTO = 'Auto',
			REBEL = 'Rebl',
			EMPIRE = 'Empr',
			SPECTATOR = 'Spec'
		};
	}
	
	namespace GameType
	{
		enum
		{
			TEAM_ELIMINATION = 'Team',
			FFA_ELIMINATION = 'FFA ',
			TEAM_DEATHMATCH = 'TDM ',
			FFA_DEATHMATCH = 'DM  ',
			BATTLE_OF_YAVIN = 'Yavn',
			CAPITAL_SHIP_HUNT = 'Hunt',
			DEFEND_DESTROY = 'DeDe'
		};
	}
}
