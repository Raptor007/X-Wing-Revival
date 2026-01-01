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
			RESET_DEFAULTS = 'Anew',
			GAMETYPE = 'Mode',
			TIME_REMAINING = 'Time',
			TIME_TO_RESPAWN = 'Wait',
			EXPLOSION = 'Boom',
			SHOT_HIT_SHIP = 'HitS',
			SHOT_HIT_TURRET = 'HitT',
			SHOT_HIT_HAZARD = 'HitH',
			MISC_HIT_SHIP = '?Hit',
			REPAIR = 'Fix ',
			REARM = 'Ammo',
			JUMP_OUT = 'Jump',
			CHANGE_SEAT = 'Seat',
			TOGGLE_COPILOT = 'CoPi',
			ENGINE_SOUND = 'Engi',
			CHECKPOINT = 'GoGo',
			ACHIEVEMENT = 'Wow!',
			MISSION_COMPLETE = 'Win!',
			UPLOAD_MISSION = 'Misn'
		};
	}
	
	namespace Object
	{
		enum
		{
			SHIP = 'Ship',
			SHIP_CLASS = 'ShpC',
			SHOT = 'Shot',
			SHOT_MISSILE = 'Misl',
			ASTEROID = 'Rock',
			TURRET = 'Turr',
			DEATH_STAR = 'Moon',
			DEATH_STAR_BOX = 'aBox',
			DOCKING_BAY = 'Dock',
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
			CTF = 'CTF ',
			TEAM_RACE = 'Kesl',
			FFA_RACE = 'Race',
			BATTLE_OF_YAVIN = 'Yavn',
			BATTLE_OF_ENDOR = 'Endr',
			CAPITAL_SHIP_HUNT = 'Hunt',
			FLEET_BATTLE = 'Flet'
		};
	}
	
	namespace ServerFeature
	{
		enum
		{
			ZERO_LAG = '0Lag'  // FIXME: Needed for v0.4.x but ignored in v0.5+ which always supports ZeroLag shots.
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
			TARGET_STORE,
			TARGET_NOTHING,
			TARGET_CROSSHAIR,
			TARGET_NEAREST_ENEMY,
			TARGET_NEAREST_ATTACKER,
			TARGET_NEWEST_INCOMING,
			TARGET_TARGET_ATTACKER,
			TARGET_OBJECTIVE,
			TARGET_DOCKABLE,
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
			TARGET1,
			TARGET2,
			TARGET3,
			TARGET4,
			TARGET_PREV,
			TARGET_NEXT,
			TARGET_PREV_ENEMY,  // SPECTATE_PREV (Xbox)
			TARGET_NEXT_ENEMY,  // SPECTATE_NEXT (Xbox)
			TARGET_PREV_FRIENDLY,
			TARGET_NEXT_FRIENDLY,
			TARGET_NEXT_PLAYER,
			TARGET_PREV_PLAYER,
			TARGET_NEXT_SUBSYSTEM,
			TARGET_PREV_SUBSYSTEM,
			SEAT_COCKPIT,
			SEAT_GUNNER1,
			SEAT_GUNNER2,
			CHEWIE_TAKE_THE_WHEEL,
			VIEW_COCKPIT,
			VIEW_CROSSHAIR,
			VIEW_CHASE,
			VIEW_PADLOCK,
			VIEW_STATIONARY,
			VIEW_CINEMA,
			VIEW_FIXED,
			VIEW_SELFIE,
			VIEW_GUNNER,
			VIEW_CYCLE,
			VIEW_INSTRUMENTS,
			CHAT,
			CHAT_TEAM,
			VOICE_ALL,
			VOICE_TEAM,
			MENU,
			PREFS,
			PAUSE,
			
			COUNT
		};
	}
	
	namespace View
	{
		enum
		{
			AUTO = 0,
			COCKPIT,
			GUNNER,
			SELFIE,
			CHASE,
			CINEMA,
			FIXED,
			STATIONARY,
			INSTRUMENTS,
			CYCLE
		};
	}
}
