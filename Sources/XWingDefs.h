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
			CHANGE_SEAT = 'Seat',
			TOGGLE_COPILOT = 'CoPi',
			ENGINE_SOUND = 'Engi',
			CHECKPOINT = 'GoGo',
			ACHIEVEMENT = 'Wow!'
		};
	}
	
	namespace Object
	{
		enum
		{
			SHIP = 'Ship',
			SHIP_CLASS = 'ShpC',
			SHOT = 'Shot',
			ASTEROID = 'Rock',
			TURRET = 'Turr',
			DEATH_STAR = 'Moon',
			DEATH_STAR_BOX = 'aBox',
			CHECKPOINT = 'GoGo'
		};
	}
	
	namespace Team
	{
		enum
		{
			NONE = 0,
			REBEL,
			EMPIRE
		};
	}
	
	namespace GameType
	{
		enum
		{
			UNDEFINED = 0,
			TEAM_ELIMINATION = 'Team',
			FFA_ELIMINATION = 'FFA ',
			TEAM_DEATHMATCH = 'TDM ',
			FFA_DEATHMATCH = 'DM  ',
			TEAM_RACE = 'Kesl',
			FFA_RACE = 'Race',
			BATTLE_OF_YAVIN = 'Yavn',
			CAPITAL_SHIP_HUNT = 'Hunt',
			FLEET_BATTLE = 'Flet'
		};
	}
	
	namespace Control
	{
		enum
		{
			// Analog Axes
			ROLL = 1, ROLL_INVERTED,
			PITCH,    PITCH_INVERTED,
			YAW,      YAW_INVERTED,
			THROTTLE, THROTTLE_INVERTED,
			LOOK_X,   LOOK_X_INVERTED,
			LOOK_Y,   LOOK_Y_INVERTED,
			
			// Held Digital Buttons
			ROLL_LEFT,
			ROLL_RIGHT,
			PITCH_UP,
			PITCH_DOWN,
			YAW_LEFT,
			YAW_RIGHT,
			THROTTLE_UP,
			THROTTLE_DOWN,
			THROTTLE_0,
			THROTTLE_33,  // SPECTATE_PREV (keyboard)
			THROTTLE_66,  // SPECTATE_NEXT (keyboard)
			THROTTLE_100,
			FIRE,
			TARGET_NOTHING,
			TARGET_CROSSHAIR,
			TARGET_NEAREST_ENEMY,
			TARGET_NEAREST_ATTACKER,
			TARGET_NEWEST_INCOMING,
			TARGET_OBJECTIVE,
			TARGET_NEWEST,
			TARGET_GROUPMATE,
			TARGET_SYNC,
			EJECT,
			LOOK_CENTER,
			LOOK_UP,
			LOOK_DOWN,
			LOOK_LEFT,
			LOOK_RIGHT,
			LOOK_UP_LEFT,
			LOOK_UP_RIGHT,
			LOOK_DOWN_LEFT,
			LOOK_DOWN_RIGHT,
			GLANCE_UP,
			GLANCE_BACK,
			GLANCE_LEFT,
			GLANCE_RIGHT,
			GLANCE_UP_LEFT,
			GLANCE_UP_RIGHT,
			GLANCE_BACK_LEFT,
			GLANCE_BACK_RIGHT,
			SCORES,
			
			// Pressed Digital Buttons
			WEAPON,
			MODE,
			SHIELD_DIR,
			TARGET_PREV,
			TARGET_NEXT,
			TARGET_PREV_ENEMY,  // SPECTATE_PREV (Xbox)
			TARGET_NEXT_ENEMY,  // SPECTATE_NEXT (Xbox)
			TARGET_PREV_FRIENDLY,
			TARGET_NEXT_FRIENDLY,
			TARGET_NEXT_SUBSYSTEM,
			TARGET_PREV_SUBSYSTEM,
			SEAT_COCKPIT,
			SEAT_GUNNER1,
			SEAT_GUNNER2,
			CHEWIE_TAKE_THE_WHEEL,
			CHAT,
			MENU,
			PREFS,
			PAUSE,
			
			COUNT
		};
	}
}
