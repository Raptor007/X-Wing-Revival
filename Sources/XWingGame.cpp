/*
 *  XWingGame.cpp
 */

#include "XWingGame.h"

#include <cstddef>
#include <cmath>
#include <dirent.h>
#include <algorithm>
#include "XWingServer.h"
#include "FirstLoadScreen.h"
#include "MainMenu.h"
#include "Screensaver.h"
#include "WaitScreen.h"
#include "LobbyMenu.h"
#include "RenderLayer.h"
#include "Rand.h"
#include "Str.h"
#include "Num.h"
#include "Math3D.h"
#include "Ship.h"
#include "Shot.h"
#include "Asteroid.h"
#include "Turret.h"
#include "DeathStar.h"
#include "DeathStarBox.h"
#include "ShipClass.h"
#include "Checkpoint.h"
#include "SaitekX52Pro.h"


XWingGame::XWingGame( std::string version ) : RaptorGame( "X-Wing Revival", version )
{
	ChatSeparator = ":  ";
	GameType = XWing::GameType::UNDEFINED;
	CampaignTeam = XWing::Team::NONE;
	ObservedShipID = 0;
	memset( StoredTargets, 0, sizeof(StoredTargets) );
	View = XWing::View::AUTO;
	LookYaw = 0.;
	LookPitch = 0.;
	ThumbstickLook = true;
	TurretFiringMode = 1;
	AsteroidLOD = 1.;
	OverlayScroll = 0.;
	BlastPoints = 0;
	Victor = XWing::Team::NONE;
	
	// Analog Axes
	Input.DefineControl( XWing::Control::ROLL,                    "Roll",      "Pedal",   -1.,  1. );
	Input.DefineControl( XWing::Control::ROLL_INVERTED,           "Roll-",     "Pedal",    1., -1. );
	Input.DefineControl( XWing::Control::PITCH,                   "Pitch",     "",        -1.,  1. );
	Input.DefineControl( XWing::Control::PITCH_INVERTED,          "Pitch-",    "",         1., -1. );
	Input.DefineControl( XWing::Control::YAW,                     "Yaw",       "Pedal",   -1.,  1. );
	Input.DefineControl( XWing::Control::YAW_INVERTED,            "Yaw-",      "Pedal",    1., -1. );
	Input.DefineControl( XWing::Control::THROTTLE,                "Throttle",  "Throttle", 1.,  0. );
	Input.DefineControl( XWing::Control::THROTTLE_INVERTED,       "Throttle-", "Throttle" );
	Input.DefineControl( XWing::Control::LOOK_X,                  "LookX",     "",        -1.,  1. );
	Input.DefineControl( XWing::Control::LOOK_X_INVERTED,         "LookX-",    "",         1., -1. );
	Input.DefineControl( XWing::Control::LOOK_Y,                  "LookY",     "",        -1.,  1. );
	Input.DefineControl( XWing::Control::LOOK_Y_INVERTED,         "LookY-",    "",         1., -1. );
	
	// Analog or Digital Controls
	Input.DefineControl( XWing::Control::ROLL_LEFT,               "RollLeft",  "Pedal" );
	Input.DefineControl( XWing::Control::ROLL_RIGHT,              "RollRight", "Pedal" );
	Input.DefineControl( XWing::Control::PITCH_UP,                "PitchUp" );
	Input.DefineControl( XWing::Control::PITCH_DOWN,              "PitchDown" );
	Input.DefineControl( XWing::Control::YAW_LEFT,                "YawLeft",   "Pedal" );
	Input.DefineControl( XWing::Control::YAW_RIGHT,               "YawRight",  "Pedal" );
	
	// Held Digital Buttons
	Input.DefineControl( XWing::Control::THROTTLE_UP,             "ThrottleUp" );
	Input.DefineControl( XWing::Control::THROTTLE_DOWN,           "ThrottleDown" );
	Input.DefineControl( XWing::Control::THROTTLE_0,              "Throttle0" );
	Input.DefineControl( XWing::Control::THROTTLE_33,             "Throttle33" );
	Input.DefineControl( XWing::Control::THROTTLE_66,             "Throttle66" );
	Input.DefineControl( XWing::Control::THROTTLE_100,            "Throttle100" );
	Input.DefineControl( XWing::Control::FIRE,                    "Fire" );
	Input.DefineControl( XWing::Control::TARGET_STORE,            "TargetStore" );
	Input.DefineControl( XWing::Control::TARGET_NOTHING,          "TargetClear" );
	Input.DefineControl( XWing::Control::TARGET_CROSSHAIR,        "TargetCrosshair" );
	Input.DefineControl( XWing::Control::TARGET_NEAREST_ENEMY,    "TargetEnemy" );
	Input.DefineControl( XWing::Control::TARGET_NEAREST_ATTACKER, "TargetAttacker" );
	Input.DefineControl( XWing::Control::TARGET_NEWEST_INCOMING,  "TargetIncoming" );
	Input.DefineControl( XWing::Control::TARGET_TARGET_ATTACKER,  "TargetTheirAttacker" );
	Input.DefineControl( XWing::Control::TARGET_OBJECTIVE,        "TargetObjective" );
	Input.DefineControl( XWing::Control::TARGET_DOCKABLE,         "TargetDockable" );
	Input.DefineControl( XWing::Control::TARGET_NEWEST,           "TargetNewest" );
	Input.DefineControl( XWing::Control::TARGET_GROUPMATE,        "TargetGroupmate" );
	Input.DefineControl( XWing::Control::TARGET_SYNC,             "TargetDataLink" );
	Input.DefineControl( XWing::Control::EJECT,                   "Eject" );
	Input.DefineControl( XWing::Control::LOOK_CENTER,             "LookCenter" );
	Input.DefineControl( XWing::Control::LOOK_UP,                 "LookUp" );
	Input.DefineControl( XWing::Control::LOOK_DOWN,               "LookDown" );
	Input.DefineControl( XWing::Control::LOOK_LEFT,               "LookLeft" );
	Input.DefineControl( XWing::Control::LOOK_RIGHT,              "LookRight" );
	Input.DefineControl( XWing::Control::LOOK_UP_LEFT,            "LookUpLeft" );
	Input.DefineControl( XWing::Control::LOOK_UP_RIGHT,           "LookUpRight" );
	Input.DefineControl( XWing::Control::LOOK_DOWN_LEFT,          "LookDownLeft" );
	Input.DefineControl( XWing::Control::LOOK_DOWN_RIGHT,         "LookDownRight" );
	Input.DefineControl( XWing::Control::GLANCE_UP,               "GlanceUp" );
	Input.DefineControl( XWing::Control::GLANCE_BACK,             "GlanceBack" );
	Input.DefineControl( XWing::Control::GLANCE_LEFT,             "GlanceLeft" );
	Input.DefineControl( XWing::Control::GLANCE_RIGHT,            "GlanceRight" );
	Input.DefineControl( XWing::Control::GLANCE_UP_LEFT,          "GlanceUpLeft" );
	Input.DefineControl( XWing::Control::GLANCE_UP_RIGHT,         "GlanceUpRight" );
	Input.DefineControl( XWing::Control::GLANCE_BACK_LEFT,        "GlanceBackLeft" );
	Input.DefineControl( XWing::Control::GLANCE_BACK_RIGHT,       "GlanceBackRight" );
	Input.DefineControl( XWing::Control::SCORES,                  "Scores" );
	
	// Pressed Digital Buttons
	Input.DefineControl( XWing::Control::WEAPON,                  "WeaponNext" );
	Input.DefineControl( XWing::Control::MODE,                    "WeaponMode" );
	Input.DefineControl( XWing::Control::SHIELD_DIR,              "AngleDeflector" );
	Input.DefineControl( XWing::Control::TARGET1,                 "Target1" );
	Input.DefineControl( XWing::Control::TARGET2,                 "Target2" );
	Input.DefineControl( XWing::Control::TARGET3,                 "Target3" );
	Input.DefineControl( XWing::Control::TARGET4,                 "Target4" );
	Input.DefineControl( XWing::Control::TARGET_PREV,             "TargetPrev" );
	Input.DefineControl( XWing::Control::TARGET_NEXT,             "TargetNext" );
	Input.DefineControl( XWing::Control::TARGET_PREV_ENEMY,       "TargetPrevEnemy" );
	Input.DefineControl( XWing::Control::TARGET_NEXT_ENEMY,       "TargetNextEnemy" );
	Input.DefineControl( XWing::Control::TARGET_PREV_FRIENDLY,    "TargetPrevFriendly" );
	Input.DefineControl( XWing::Control::TARGET_NEXT_FRIENDLY,    "TargetNextFriendly" );
	Input.DefineControl( XWing::Control::TARGET_PREV_PLAYER,      "TargetPrevPlayer" );
	Input.DefineControl( XWing::Control::TARGET_NEXT_PLAYER,      "TargetNextPlayer" );
	Input.DefineControl( XWing::Control::TARGET_PREV_SUBSYSTEM,   "TargetPrevSystem" );
	Input.DefineControl( XWing::Control::TARGET_NEXT_SUBSYSTEM,   "TargetNextSystem" );
	Input.DefineControl( XWing::Control::SEAT_COCKPIT,            "Cockpit" );
	Input.DefineControl( XWing::Control::SEAT_GUNNER1,            "Gunner1" );
	Input.DefineControl( XWing::Control::SEAT_GUNNER2,            "Gunner2" );
	Input.DefineControl( XWing::Control::CHEWIE_TAKE_THE_WHEEL,   "Chewie" );
	Input.DefineControl( XWing::Control::VIEW_COCKPIT,            "ViewCockpit" );
	Input.DefineControl( XWing::Control::VIEW_CROSSHAIR,          "ViewCrosshair" );
	Input.DefineControl( XWing::Control::VIEW_CHASE,              "ViewChase" );
	Input.DefineControl( XWing::Control::VIEW_PADLOCK,            "ViewPadlock" );
	Input.DefineControl( XWing::Control::VIEW_STATIONARY,         "ViewDropCam" );
	Input.DefineControl( XWing::Control::VIEW_CINEMA,             "ViewCinema" );
	Input.DefineControl( XWing::Control::VIEW_FIXED,              "ViewFixed" );
	Input.DefineControl( XWing::Control::VIEW_SELFIE,             "ViewPilot" );
	Input.DefineControl( XWing::Control::VIEW_GUNNER,             "ViewGunner" );
	Input.DefineControl( XWing::Control::VIEW_CYCLE,              "ViewCycle" );
	Input.DefineControl( XWing::Control::VIEW_INSTRUMENTS,        "ViewInstruments" );
	Input.DefineControl( XWing::Control::CHAT,                    "Chat" );
	Input.DefineControl( XWing::Control::CHAT_TEAM,               "ChatTeam" );
	Input.DefineControl( XWing::Control::VOICE_TEAM,              "VoiceTeam" );
	Input.DefineControl( XWing::Control::VOICE_ALL,               "VoiceAll" );
	Input.DefineControl( XWing::Control::MENU,                    "MainMenu" );
	Input.DefineControl( XWing::Control::PREFS,                   "PrefsMenu" );
	Input.DefineControl( XWing::Control::PAUSE,                   "Pause" );
}


XWingGame::~XWingGame()
{
}


void XWingGame::SetDefaultJoyTypes( void )
{
	RaptorGame::SetDefaultJoyTypes();
	
	// Give these joysticks their own binds even if their axis mappings are the same.
	Input.DeviceTypes.insert( "X52" );
	Input.DeviceTypes.insert( "SideWinder" );
	Input.DeviceTypes.insert( "Gunfighter" );
	
#ifndef WIN32
	// Linux does not replace the device string for Xbox compatible controllers, and some do not contain "Xbox".
	Input.DeviceTypes.insert( "ProEX" );
#endif
}


void XWingGame::SetDefaultControls( void )
{
	RaptorGame::SetDefaultControls();
	
	Cfg.KeyBinds[ SDLK_UP           ] = XWing::Control::PITCH_DOWN;
	Cfg.KeyBinds[ SDLK_DOWN         ] = XWing::Control::PITCH_UP;
	Cfg.KeyBinds[ SDLK_LEFT         ] = XWing::Control::YAW_LEFT;
	Cfg.KeyBinds[ SDLK_RIGHT        ] = XWing::Control::YAW_RIGHT;
	Cfg.KeyBinds[ SDLK_BACKSLASH    ] = XWing::Control::THROTTLE_0;
	Cfg.KeyBinds[ SDLK_LEFTBRACKET  ] = XWing::Control::THROTTLE_33;
	Cfg.KeyBinds[ SDLK_RIGHTBRACKET ] = XWing::Control::THROTTLE_66;
	Cfg.KeyBinds[ SDLK_BACKSPACE    ] = XWing::Control::THROTTLE_100;
	Cfg.KeyBinds[ SDLK_EQUALS       ] = XWing::Control::THROTTLE_UP;
	Cfg.KeyBinds[ SDLK_MINUS        ] = XWing::Control::THROTTLE_DOWN;
	Cfg.KeyBinds[ SDLK_SPACE        ] = XWing::Control::FIRE;
	Cfg.KeyBinds[ SDLK_LSHIFT       ] = XWing::Control::TARGET_STORE;
	Cfg.KeyBinds[ SDLK_RSHIFT       ] = XWing::Control::TARGET_STORE;
	Cfg.KeyBinds[ SDLK_LCTRL        ] = XWing::Control::TARGET_CROSSHAIR;
	Cfg.KeyBinds[ SDLK_q            ] = XWing::Control::TARGET_NOTHING;
	Cfg.KeyBinds[ SDLK_w            ] = XWing::Control::WEAPON;
	Cfg.KeyBinds[ SDLK_e            ] = XWing::Control::TARGET_NEAREST_ATTACKER;
	Cfg.KeyBinds[ SDLK_r            ] = XWing::Control::TARGET_NEAREST_ENEMY;
	Cfg.KeyBinds[ SDLK_t            ] = XWing::Control::TARGET_NEXT;
	Cfg.KeyBinds[ SDLK_y            ] = XWing::Control::TARGET_PREV;
	Cfg.KeyBinds[ SDLK_u            ] = XWing::Control::TARGET_NEWEST;
	Cfg.KeyBinds[ SDLK_i            ] = XWing::Control::TARGET_NEWEST_INCOMING;
	Cfg.KeyBinds[ SDLK_o            ] = XWing::Control::TARGET_OBJECTIVE;
	Cfg.KeyBinds[ SDLK_p            ] = XWing::Control::TARGET_NEXT_PLAYER;
	Cfg.KeyBinds[ SDLK_a            ] = XWing::Control::TARGET_TARGET_ATTACKER;
	Cfg.KeyBinds[ SDLK_s            ] = XWing::Control::SHIELD_DIR;
	Cfg.KeyBinds[ SDLK_d            ] = XWing::Control::ROLL_LEFT;
	Cfg.KeyBinds[ SDLK_f            ] = XWing::Control::ROLL_RIGHT;
	Cfg.KeyBinds[ SDLK_g            ] = XWing::Control::TARGET_GROUPMATE;
	Cfg.KeyBinds[ SDLK_h            ] = XWing::Control::TARGET_DOCKABLE;
	Cfg.KeyBinds[ SDLK_k            ] = XWing::Control::EJECT;
	Cfg.KeyBinds[ SDLK_x            ] = XWing::Control::MODE;
	Cfg.KeyBinds[ SDLK_v            ] = XWing::Control::TARGET_SYNC;
	Cfg.KeyBinds[ SDLK_CAPSLOCK     ] = XWing::Control::TARGET_SYNC;
	Cfg.KeyBinds[ SDLK_COMMA        ] = XWing::Control::TARGET_PREV_SUBSYSTEM;
	Cfg.KeyBinds[ SDLK_PERIOD       ] = XWing::Control::TARGET_NEXT_SUBSYSTEM;
	Cfg.KeyBinds[ SDLK_SLASH        ] = XWing::Control::VIEW_SELFIE;
	Cfg.KeyBinds[ SDLK_KP7          ] = XWing::Control::LOOK_UP_LEFT;
	Cfg.KeyBinds[ SDLK_KP8          ] = XWing::Control::LOOK_UP;
	Cfg.KeyBinds[ SDLK_KP9          ] = XWing::Control::LOOK_UP_RIGHT;
	Cfg.KeyBinds[ SDLK_KP4          ] = XWing::Control::LOOK_LEFT;
	Cfg.KeyBinds[ SDLK_KP5          ] = XWing::Control::LOOK_CENTER;
	Cfg.KeyBinds[ SDLK_KP6          ] = XWing::Control::LOOK_RIGHT;
	Cfg.KeyBinds[ SDLK_KP1          ] = XWing::Control::LOOK_DOWN_LEFT;
	Cfg.KeyBinds[ SDLK_KP2          ] = XWing::Control::LOOK_DOWN;
	Cfg.KeyBinds[ SDLK_KP3          ] = XWing::Control::LOOK_DOWN_RIGHT;
	Cfg.KeyBinds[ SDLK_INSERT       ] = XWing::Control::GLANCE_UP_LEFT;
	Cfg.KeyBinds[ SDLK_HOME         ] = XWing::Control::GLANCE_UP;
	Cfg.KeyBinds[ SDLK_PAGEUP       ] = XWing::Control::GLANCE_UP_RIGHT;
	Cfg.KeyBinds[ SDLK_DELETE       ] = XWing::Control::GLANCE_LEFT;
	Cfg.KeyBinds[ SDLK_END          ] = XWing::Control::GLANCE_BACK;
	Cfg.KeyBinds[ SDLK_PAGEDOWN     ] = XWing::Control::GLANCE_RIGHT;
	Cfg.KeyBinds[ SDLK_TAB          ] = XWing::Control::SCORES;
	Cfg.KeyBinds[ SDLK_F1           ] = XWing::Control::SEAT_COCKPIT;
	Cfg.KeyBinds[ SDLK_F2           ] = XWing::Control::SEAT_GUNNER1;
	Cfg.KeyBinds[ SDLK_F3           ] = XWing::Control::SEAT_GUNNER2;
	Cfg.KeyBinds[ SDLK_F4           ] = XWing::Control::CHEWIE_TAKE_THE_WHEEL;
	Cfg.KeyBinds[ SDLK_F5           ] = XWing::Control::TARGET1;
	Cfg.KeyBinds[ SDLK_F6           ] = XWing::Control::TARGET2;
	Cfg.KeyBinds[ SDLK_F7           ] = XWing::Control::TARGET3;
	Cfg.KeyBinds[ SDLK_F8           ] = XWing::Control::TARGET4;
	Cfg.KeyBinds[ SDLK_F15          ] = XWing::Control::PAUSE;
	Cfg.KeyBinds[ SDLK_1            ] = XWing::Control::VIEW_COCKPIT;
	Cfg.KeyBinds[ SDLK_2            ] = XWing::Control::VIEW_CROSSHAIR;
	Cfg.KeyBinds[ SDLK_3            ] = XWing::Control::VIEW_CHASE;
	Cfg.KeyBinds[ SDLK_4            ] = XWing::Control::VIEW_PADLOCK;
	Cfg.KeyBinds[ SDLK_5            ] = XWing::Control::VIEW_STATIONARY;
	Cfg.KeyBinds[ SDLK_6            ] = XWing::Control::VIEW_CINEMA;
	Cfg.KeyBinds[ SDLK_7            ] = XWing::Control::VIEW_FIXED;
	Cfg.KeyBinds[ SDLK_8            ] = XWing::Control::VIEW_GUNNER;
	Cfg.KeyBinds[ SDLK_9            ] = XWing::Control::VIEW_CYCLE;
	Cfg.KeyBinds[ SDLK_0            ] = XWing::Control::VIEW_INSTRUMENTS;
	Cfg.KeyBinds[ SDLK_RETURN       ] = XWing::Control::CHAT;
	Cfg.KeyBinds[ SDLK_KP_ENTER     ] = XWing::Control::CHAT_TEAM;
	Cfg.KeyBinds[ SDLK_RALT         ] = XWing::Control::VOICE_ALL;
	Cfg.KeyBinds[ SDLK_RCTRL        ] = XWing::Control::VOICE_TEAM;
	Cfg.KeyBinds[ SDLK_ESCAPE       ] = XWing::Control::MENU;
	Cfg.KeyBinds[ SDLK_F9           ] = XWing::Control::MENU;
	Cfg.KeyBinds[ SDLK_F10          ] = XWing::Control::PREFS;
	Cfg.KeyBinds[ SDLK_PAUSE        ] = XWing::Control::PAUSE;
	
	Cfg.MouseBinds[ SDL_BUTTON_LEFT      ] = XWing::Control::FIRE;
	Cfg.MouseBinds[ SDL_BUTTON_RIGHT     ] = XWing::Control::TARGET_CROSSHAIR;
	Cfg.MouseBinds[ SDL_BUTTON_MIDDLE    ] = XWing::Control::WEAPON;
	Cfg.MouseBinds[ SDL_BUTTON_X1        ] = XWing::Control::VOICE_TEAM;
	Cfg.MouseBinds[ SDL_BUTTON_X2        ] = XWing::Control::VOICE_ALL;
	Cfg.MouseBinds[ SDL_BUTTON_WHEELUP   ] = XWing::Control::TARGET_PREV;
	Cfg.MouseBinds[ SDL_BUTTON_WHEELDOWN ] = XWing::Control::TARGET_NEXT;
	
	Cfg.JoyAxisBinds[ "Joy" ][ 0 ] = XWing::Control::ROLL;      // Stick X
	Cfg.JoyAxisBinds[ "Joy" ][ 1 ] = XWing::Control::PITCH;     // Stick Y
	Cfg.JoyAxisBinds[ "Joy" ][ 2 ] = XWing::Control::THROTTLE;  // Throttle
	Cfg.JoyAxisBinds[ "Joy" ][ 3 ] = XWing::Control::YAW;       // Twist
	
	Cfg.JoyButtonBinds[ "Joy" ][  0 ] = XWing::Control::FIRE;                     // Trigger
	Cfg.JoyButtonBinds[ "Joy" ][  1 ] = XWing::Control::WEAPON;                   // Fire
	Cfg.JoyButtonBinds[ "Joy" ][  2 ] = XWing::Control::TARGET_CROSSHAIR;         // A
	Cfg.JoyButtonBinds[ "Joy" ][  3 ] = XWing::Control::TARGET_NEAREST_ENEMY;     // B
	Cfg.JoyButtonBinds[ "Joy" ][  4 ] = XWing::Control::MODE;                     // C
	Cfg.JoyButtonBinds[ "Joy" ][  5 ] = XWing::Control::LOOK_CENTER;              // Pinkie
	Cfg.JoyButtonBinds[ "Joy" ][  6 ] = XWing::Control::SHIELD_DIR;               // D
	Cfg.JoyButtonBinds[ "Joy" ][  7 ] = XWing::Control::TARGET_NEAREST_ATTACKER;  // E
	Cfg.JoyButtonBinds[ "Joy" ][  8 ] = XWing::Control::TARGET_NEWEST_INCOMING;   // T1
	Cfg.JoyButtonBinds[ "Joy" ][  9 ] = XWing::Control::TARGET_SYNC;              // T2
	Cfg.JoyButtonBinds[ "Joy" ][ 10 ] = XWing::Control::SEAT_COCKPIT;             // T3
	Cfg.JoyButtonBinds[ "Joy" ][ 11 ] = XWing::Control::CHEWIE_TAKE_THE_WHEEL;    // T4
	Cfg.JoyButtonBinds[ "Joy" ][ 12 ] = XWing::Control::SEAT_GUNNER1;             // T5
	Cfg.JoyButtonBinds[ "Joy" ][ 13 ] = XWing::Control::SEAT_GUNNER2;             // T6
	Cfg.JoyButtonBinds[ "Joy" ][ 19 ] = XWing::Control::TARGET_OBJECTIVE;         // Hat 2 Up
	Cfg.JoyButtonBinds[ "Joy" ][ 20 ] = XWing::Control::TARGET3;                  // Hat 2 Right
	Cfg.JoyButtonBinds[ "Joy" ][ 21 ] = XWing::Control::TARGET2;                  // Hat 2 Down
	Cfg.JoyButtonBinds[ "Joy" ][ 22 ] = XWing::Control::TARGET1;                  // Hat 2 Left
	Cfg.JoyButtonBinds[ "Joy" ][ 23 ] = XWing::Control::TARGET_STORE;             // Hat 3 Up
	Cfg.JoyButtonBinds[ "Joy" ][ 24 ] = XWing::Control::TARGET_NEXT;              // Hat 3 Right
	Cfg.JoyButtonBinds[ "Joy" ][ 25 ] = XWing::Control::SCORES;                   // Hat 3 Down
	Cfg.JoyButtonBinds[ "Joy" ][ 26 ] = XWing::Control::TARGET_PREV;              // Hat 3 Left
	Cfg.JoyButtonBinds[ "Joy" ][ 30 ] = XWing::Control::TARGET_NEWEST_INCOMING;   // Clutch
	
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_UP    ] = XWing::Control::GLANCE_UP;     // Hat 1 Up
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_RIGHT ] = XWing::Control::GLANCE_RIGHT;  // Hat 1 Right
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_DOWN  ] = XWing::Control::GLANCE_BACK;   // Hat 1 Down
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_LEFT  ] = XWing::Control::GLANCE_LEFT;   // Hat 1 Left
	
	if( Input.DeviceTypes.find("X52") != Input.DeviceTypes.end() )
	{
		#if SDL_VERSION_ATLEAST(2,0,0)
			Cfg.JoyAxisBinds[ "X52" ][ 0 ] = XWing::Control::ROLL;      // Stick X
			Cfg.JoyAxisBinds[ "X52" ][ 1 ] = XWing::Control::PITCH;     // Stick Y
			Cfg.JoyAxisBinds[ "X52" ][ 2 ] = XWing::Control::THROTTLE;  // Throttle
			Cfg.JoyAxisBinds[ "X52" ][ 5 ] = XWing::Control::YAW;       // Twist
		#else
			Cfg.JoyAxisBinds[ "X52" ] = Cfg.JoyAxisBinds[ "Joy" ];
		#endif
		
		Cfg.JoyButtonBinds[ "X52" ] = Cfg.JoyButtonBinds[ "Joy" ];
		Cfg.JoyHatBinds   [ "X52" ] = Cfg.JoyHatBinds   [ "Joy" ];
		
		// If X52 has its own binds, other joysticks default to good settings for Cyborg Evo instead.
		Cfg.JoyButtonBinds[ "Joy" ].clear();
		Cfg.JoyButtonBinds[ "Joy" ][  0 ] = XWing::Control::FIRE;                     // Trigger
		Cfg.JoyButtonBinds[ "Joy" ][  1 ] = XWing::Control::WEAPON;                   // 2
		Cfg.JoyButtonBinds[ "Joy" ][  2 ] = XWing::Control::MODE;                     // 3
		Cfg.JoyButtonBinds[ "Joy" ][  3 ] = XWing::Control::TARGET_NEAREST_ENEMY;     // 4
		Cfg.JoyButtonBinds[ "Joy" ][  4 ] = XWing::Control::LOOK_CENTER;              // 5
		Cfg.JoyButtonBinds[ "Joy" ][  5 ] = XWing::Control::TARGET_CROSSHAIR;         // 6
		Cfg.JoyButtonBinds[ "Joy" ][  6 ] = XWing::Control::TARGET_NEAREST_ATTACKER;  // F1
		Cfg.JoyButtonBinds[ "Joy" ][  7 ] = XWing::Control::TARGET_NEWEST_INCOMING;   // F2
		Cfg.JoyButtonBinds[ "Joy" ][  8 ] = XWing::Control::TARGET1;                  // F3
		Cfg.JoyButtonBinds[ "Joy" ][  9 ] = XWing::Control::TARGET_OBJECTIVE;         // F4
		Cfg.JoyButtonBinds[ "Joy" ][ 10 ] = XWing::Control::SHIELD_DIR;               // >
		Cfg.JoyButtonBinds[ "Joy" ][ 11 ] = XWing::Control::SCORES;                   // <
	}
	
	if( Input.DeviceTypes.find("SideWinder") != Input.DeviceTypes.end() )
	{
		#if SDL_VERSION_ATLEAST(2,0,0)
			Cfg.JoyAxisBinds[ "SideWinder" ][ 0 ] = XWing::Control::ROLL;      // Stick X
			Cfg.JoyAxisBinds[ "SideWinder" ][ 1 ] = XWing::Control::PITCH;     // Stick Y
			Cfg.JoyAxisBinds[ "SideWinder" ][ 2 ] = XWing::Control::YAW;       // Twist
			Cfg.JoyAxisBinds[ "SideWinder" ][ 3 ] = XWing::Control::THROTTLE;  // Throttle
		#else
			Cfg.JoyAxisBinds[ "SideWinder" ] = Cfg.JoyAxisBinds[ "Joy" ];
		#endif
		
		Cfg.JoyButtonBinds[ "SideWinder" ][ 0 ] = XWing::Control::FIRE;                     // Trigger
		Cfg.JoyButtonBinds[ "SideWinder" ][ 1 ] = XWing::Control::MODE;                     // Stick Big Button
		Cfg.JoyButtonBinds[ "SideWinder" ][ 2 ] = XWing::Control::TARGET_CROSSHAIR;         // Stick Top Button
		Cfg.JoyButtonBinds[ "SideWinder" ][ 3 ] = XWing::Control::WEAPON;                   // Stick Bottom Button
		Cfg.JoyButtonBinds[ "SideWinder" ][ 4 ] = XWing::Control::TARGET_NEAREST_ENEMY;     // A
		Cfg.JoyButtonBinds[ "SideWinder" ][ 5 ] = XWing::Control::TARGET_NEAREST_ATTACKER;  // B
		Cfg.JoyButtonBinds[ "SideWinder" ][ 6 ] = XWing::Control::TARGET_NEWEST_INCOMING;   // C
		Cfg.JoyButtonBinds[ "SideWinder" ][ 7 ] = XWing::Control::SHIELD_DIR;               // D
		Cfg.JoyButtonBinds[ "SideWinder" ][ 8 ] = XWing::Control::LOOK_CENTER;              // Shift
		
		Cfg.JoyHatBinds[ "SideWinder" ] = Cfg.JoyHatBinds[ "Joy" ];
	}
	
	if( Input.DeviceTypes.find("Gunfighter") != Input.DeviceTypes.end() )
	{
		#if SDL_VERSION_ATLEAST(2,0,0)
			Cfg.JoyAxisBinds[ "Gunfighter" ][ 0 ] = XWing::Control::ROLL;      // Stick X
			Cfg.JoyAxisBinds[ "Gunfighter" ][ 1 ] = XWing::Control::PITCH;     // Stick Y
			Cfg.JoyAxisBinds[ "Gunfighter" ][ 6 ] = XWing::Control::YAW;       // Twist
		#else
			Cfg.JoyAxisBinds[ "Gunfighter" ] = Cfg.JoyAxisBinds[ "Joy" ];
		#endif
		
		Cfg.JoyButtonBinds[ "Gunfighter" ][  0 ] = XWing::Control::FIRE;                     // Trigger
		Cfg.JoyButtonBinds[ "Gunfighter" ][  2 ] = XWing::Control::WEAPON;                   // Red Button
		Cfg.JoyButtonBinds[ "Gunfighter" ][  3 ] = XWing::Control::MODE;                     // Grey Button
		Cfg.JoyButtonBinds[ "Gunfighter" ][  4 ] = XWing::Control::LOOK_CENTER;              // Pinkie
		Cfg.JoyButtonBinds[ "Gunfighter" ][  5 ] = XWing::Control::TARGET_CROSSHAIR;         // Hat 2 Up
		Cfg.JoyButtonBinds[ "Gunfighter" ][  6 ] = XWing::Control::TARGET_NEAREST_ENEMY;     // Hat 2 Right
		Cfg.JoyButtonBinds[ "Gunfighter" ][  7 ] = XWing::Control::TARGET_NEAREST_ATTACKER;  // Hat 2 Down
		Cfg.JoyButtonBinds[ "Gunfighter" ][  8 ] = XWing::Control::TARGET_NEWEST_INCOMING;   // Hat 2 Left
		Cfg.JoyButtonBinds[ "Gunfighter" ][ 10 ] = XWing::Control::TARGET_OBJECTIVE;         // Hat 3 Up
		Cfg.JoyButtonBinds[ "Gunfighter" ][ 11 ] = XWing::Control::TARGET3;                  // Hat 3 Right
		Cfg.JoyButtonBinds[ "Gunfighter" ][ 12 ] = XWing::Control::TARGET2;                  // Hat 3 Down
		Cfg.JoyButtonBinds[ "Gunfighter" ][ 13 ] = XWing::Control::TARGET1;                  // Hat 3 Left
		Cfg.JoyButtonBinds[ "Gunfighter" ][ 15 ] = XWing::Control::VOICE_ALL;                // Hat 4 Up
		Cfg.JoyButtonBinds[ "Gunfighter" ][ 16 ] = XWing::Control::SHIELD_DIR;               // Hat 4 Right
		Cfg.JoyButtonBinds[ "Gunfighter" ][ 17 ] = XWing::Control::VOICE_TEAM;               // Hat 4 Down
		Cfg.JoyButtonBinds[ "Gunfighter" ][ 18 ] = XWing::Control::SCORES;                   // Hat 4 Left
		Cfg.JoyButtonBinds[ "Gunfighter" ][ 20 ] = XWing::Control::TARGET_NEXT;              // Trigger 2 Back
		Cfg.JoyButtonBinds[ "Gunfighter" ][ 21 ] = XWing::Control::TARGET_PREV;              // Trigger 2 Forward
		
		Cfg.JoyHatBinds[ "Gunfighter" ] = Cfg.JoyHatBinds[ "Joy" ];
	}
	
	Cfg.JoyAxisBinds[ "Pedal" ][ 0 ] = XWing::Control::YAW_LEFT;      // Left Pedal
	Cfg.JoyAxisBinds[ "Pedal" ][ 1 ] = XWing::Control::YAW_RIGHT;     // Right Pedal
	Cfg.JoyAxisBinds[ "Pedal" ][ 2 ] = XWing::Control::YAW;           // Combined Slider
	
	Cfg.JoyAxisBinds[ "Throttle" ][ 0 ] = XWing::Control::THROTTLE;
	
	Cfg.JoyButtonBinds[ "Throttle" ][  0 ] = XWing::Control::SHIELD_DIR;               // E
	Cfg.JoyButtonBinds[ "Throttle" ][  1 ] = XWing::Control::TARGET_NEAREST_ATTACKER;  // F
	Cfg.JoyButtonBinds[ "Throttle" ][  2 ] = XWing::Control::TARGET_NEAREST_ENEMY;     // G
	Cfg.JoyButtonBinds[ "Throttle" ][  3 ] = XWing::Control::TARGET_NEWEST_INCOMING;   // H
	Cfg.JoyButtonBinds[ "Throttle" ][  4 ] = XWing::Control::TARGET_STORE;             // I
	Cfg.JoyButtonBinds[ "Throttle" ][  5 ] = XWing::Control::TARGET_PREV_ENEMY;        // SW 1
	Cfg.JoyButtonBinds[ "Throttle" ][  6 ] = XWing::Control::TARGET_NEXT_ENEMY;        // SW 2
	Cfg.JoyButtonBinds[ "Throttle" ][  7 ] = XWing::Control::TARGET_PREV_FRIENDLY;     // SW 3
	Cfg.JoyButtonBinds[ "Throttle" ][  8 ] = XWing::Control::TARGET_NEXT_FRIENDLY;     // SW 4
	Cfg.JoyButtonBinds[ "Throttle" ][  9 ] = XWing::Control::TARGET_PREV_PLAYER;       // SW 5
	Cfg.JoyButtonBinds[ "Throttle" ][ 10 ] = XWing::Control::TARGET_NEXT_PLAYER;       // SW 6
	Cfg.JoyButtonBinds[ "Throttle" ][ 11 ] = XWing::Control::TARGET_SYNC;              // TGL 1 Up
	Cfg.JoyButtonBinds[ "Throttle" ][ 12 ] = XWing::Control::TARGET_GROUPMATE;         // TGL 1 Down
	Cfg.JoyButtonBinds[ "Throttle" ][ 13 ] = XWing::Control::TARGET_OBJECTIVE;         // TGL 2 Up
	Cfg.JoyButtonBinds[ "Throttle" ][ 14 ] = XWing::Control::TARGET_DOCKABLE;          // TGL 2 Down
	Cfg.JoyButtonBinds[ "Throttle" ][ 15 ] = XWing::Control::MODE;                     // TGL 3 Up
	Cfg.JoyButtonBinds[ "Throttle" ][ 16 ] = XWing::Control::WEAPON;                   // TGL 3 Down
	Cfg.JoyButtonBinds[ "Throttle" ][ 17 ] = XWing::Control::SCORES;                   // TGL 4 Up
	Cfg.JoyButtonBinds[ "Throttle" ][ 18 ] = XWing::Control::LOOK_CENTER;              // TGL 4 Down
	Cfg.JoyButtonBinds[ "Throttle" ][ 19 ] = XWing::Control::TARGET4;                  // H3 Up
	Cfg.JoyButtonBinds[ "Throttle" ][ 20 ] = XWing::Control::TARGET3;                  // H3 Right
	Cfg.JoyButtonBinds[ "Throttle" ][ 21 ] = XWing::Control::TARGET2;                  // H3 Down
	Cfg.JoyButtonBinds[ "Throttle" ][ 22 ] = XWing::Control::TARGET1;                  // H3 Left
	Cfg.JoyButtonBinds[ "Throttle" ][ 23 ] = XWing::Control::SEAT_GUNNER1;             // H4 Up
	Cfg.JoyButtonBinds[ "Throttle" ][ 24 ] = XWing::Control::SEAT_COCKPIT;             // H4 Right
	Cfg.JoyButtonBinds[ "Throttle" ][ 25 ] = XWing::Control::SEAT_GUNNER2;             // H4 Down
	Cfg.JoyButtonBinds[ "Throttle" ][ 26 ] = XWing::Control::CHEWIE_TAKE_THE_WHEEL;    // H4 Left
	Cfg.JoyButtonBinds[ "Throttle" ][ 27 ] = XWing::Control::VOICE_TEAM;               // K1 Up
	Cfg.JoyButtonBinds[ "Throttle" ][ 28 ] = XWing::Control::VOICE_ALL;                // K1 Down
	Cfg.JoyButtonBinds[ "Throttle" ][ 29 ] = XWing::Control::TARGET_NEXT;              // Knob Up
	Cfg.JoyButtonBinds[ "Throttle" ][ 30 ] = XWing::Control::TARGET_PREV;              // Knob Down
	Cfg.JoyButtonBinds[ "Throttle" ][ 31 ] = XWing::Control::TARGET_NEWEST_INCOMING;   // Clutch
	
	Cfg.JoyAxisBinds[ "Xbox" ][ 0 ] = XWing::Control::YAW;            // Left Thumbstick X
	Cfg.JoyAxisBinds[ "Xbox" ][ 1 ] = XWing::Control::PITCH;          // Left Thumbstick Y
#ifndef WIN32
	Cfg.JoyAxisBinds[ "Xbox" ][ 2 ] = XWing::Control::ROLL_LEFT;      // Left Trigger
	Cfg.JoyAxisBinds[ "Xbox" ][ 3 ] = XWing::Control::LOOK_X;         // Right Thumbstick X
	Cfg.JoyAxisBinds[ "Xbox" ][ 4 ] = XWing::Control::LOOK_Y;         // Right Thumbstick Y
	Cfg.JoyAxisBinds[ "Xbox" ][ 5 ] = XWing::Control::ROLL_RIGHT;     // Right Trigger
#elif SDL_VERSION_ATLEAST(2,0,0)
	Cfg.JoyAxisBinds[ "Xbox" ][ 2 ] = XWing::Control::LOOK_X;         // Right Thumbstick X
	Cfg.JoyAxisBinds[ "Xbox" ][ 3 ] = XWing::Control::LOOK_Y;         // Right Thumbstick Y
	Cfg.JoyAxisBinds[ "Xbox" ][ 4 ] = XWing::Control::ROLL_LEFT;      // Left Trigger
	Cfg.JoyAxisBinds[ "Xbox" ][ 5 ] = XWing::Control::ROLL_RIGHT;     // Right Trigger
#else
	Cfg.JoyAxisBinds[ "Xbox" ][ 2 ] = XWing::Control::ROLL_INVERTED;  // Triggers
	Cfg.JoyAxisBinds[ "Xbox" ][ 3 ] = XWing::Control::LOOK_Y;         // Right Thumbstick Y
	Cfg.JoyAxisBinds[ "Xbox" ][ 4 ] = XWing::Control::LOOK_X;         // Right Thumbstick X
#endif
	
	Cfg.JoyButtonBinds[ "Xbox" ][ 0 ] = XWing::Control::THROTTLE_DOWN;     // A
	Cfg.JoyButtonBinds[ "Xbox" ][ 1 ] = XWing::Control::MODE;              // B
	Cfg.JoyButtonBinds[ "Xbox" ][ 2 ] = XWing::Control::THROTTLE_UP;       // X
	Cfg.JoyButtonBinds[ "Xbox" ][ 3 ] = XWing::Control::WEAPON;            // Y
	Cfg.JoyButtonBinds[ "Xbox" ][ 4 ] = XWing::Control::TARGET_CROSSHAIR;  // LB
	Cfg.JoyButtonBinds[ "Xbox" ][ 5 ] = XWing::Control::FIRE;              // RB
	Cfg.JoyButtonBinds[ "Xbox" ][ 6 ] = XWing::Control::SCORES;            // Back
	Cfg.JoyButtonBinds[ "Xbox" ][ 7 ] = XWing::Control::SHIELD_DIR;        // Start
	Cfg.JoyButtonBinds[ "Xbox" ][ 8 ] = XWing::Control::VOICE_TEAM;        // Left Thumbstick Click
	Cfg.JoyButtonBinds[ "Xbox" ][ 9 ] = XWing::Control::LOOK_CENTER;       // Right Thumbstick Click
	
	Cfg.JoyHatBinds[ "Xbox" ][ 0 ][ SDL_HAT_UP    ] = XWing::Control::TARGET_NEAREST_ENEMY;     // D-Pad Up
	Cfg.JoyHatBinds[ "Xbox" ][ 0 ][ SDL_HAT_DOWN  ] = XWing::Control::TARGET_NEAREST_ATTACKER;  // D-Pad Down
	Cfg.JoyHatBinds[ "Xbox" ][ 0 ][ SDL_HAT_LEFT  ] = XWing::Control::TARGET_PREV_ENEMY;        // D-Pad Left
	Cfg.JoyHatBinds[ "Xbox" ][ 0 ][ SDL_HAT_RIGHT ] = XWing::Control::TARGET_NEXT_ENEMY;        // D-Pad Right
	
	if( Input.DeviceTypes.find("Pad") != Input.DeviceTypes.end() )
	{
		Cfg.JoyAxisBinds  [ "Pad" ] = Cfg.JoyAxisBinds  [ "Xbox" ];
		Cfg.JoyButtonBinds[ "Pad" ] = Cfg.JoyButtonBinds[ "Xbox" ];
		Cfg.JoyHatBinds   [ "Pad" ] = Cfg.JoyHatBinds   [ "Xbox" ];
	}
	
	if( Input.DeviceTypes.find("ProEX") != Input.DeviceTypes.end() )
	{
		Cfg.JoyAxisBinds  [ "ProEX" ] = Cfg.JoyAxisBinds  [ "Xbox" ];
		Cfg.JoyButtonBinds[ "ProEX" ] = Cfg.JoyButtonBinds[ "Xbox" ];
		Cfg.JoyHatBinds   [ "ProEX" ] = Cfg.JoyHatBinds   [ "Xbox" ];
		
		// Unbind downward-drifting right thumbstick axis on PowerA controllers.
#ifndef WIN32
		Cfg.JoyAxisBinds[ "ProEX" ][ 4 ] = 0;  // Right Thumbstick Y
#elif SDL_VERSION_ATLEAST(2,0,0)
		Cfg.JoyAxisBinds[ "ProEX" ][ 3 ] = 0;  // Right Thumbstick Y
#else
		Cfg.JoyAxisBinds[ "ProEX" ][ 3 ] = 0;  // Right Thumbstick Y
#endif
		Cfg.JoyButtonBinds[ "ProEX" ][ 9 ] = XWing::Control::GLANCE_BACK;  // Right Thumbstick Click
	}
	
	if( Input.DeviceTypes.find("MFD") != Input.DeviceTypes.end() )
	{
		Cfg.JoyButtonBinds[ "MFD" ][  0 ] = XWing::Control::TARGET_SYNC;              // Top #1
		Cfg.JoyButtonBinds[ "MFD" ][  1 ] = XWing::Control::TARGET_GROUPMATE;         // Top #2
		Cfg.JoyButtonBinds[ "MFD" ][  2 ] = XWing::Control::LOOK_CENTER;              // Top #3
		Cfg.JoyButtonBinds[ "MFD" ][  3 ] = XWing::Control::TARGET_PREV;              // Top #4
		Cfg.JoyButtonBinds[ "MFD" ][  4 ] = XWing::Control::TARGET_NEXT;              // Top #5
		Cfg.JoyButtonBinds[ "MFD" ][  5 ] = XWing::Control::TARGET_NEWEST;            // Right #1
		Cfg.JoyButtonBinds[ "MFD" ][  6 ] = XWing::Control::TARGET_PREV_FRIENDLY;     // Right #2
		Cfg.JoyButtonBinds[ "MFD" ][  7 ] = XWing::Control::TARGET_NEXT_FRIENDLY;     // Right #3
		Cfg.JoyButtonBinds[ "MFD" ][  8 ] = XWing::Control::TARGET_PREV_ENEMY;        // Right #4
		Cfg.JoyButtonBinds[ "MFD" ][  9 ] = XWing::Control::TARGET_NEXT_ENEMY;        // Right #5
		Cfg.JoyButtonBinds[ "MFD" ][ 10 ] = XWing::Control::TARGET_NEWEST_INCOMING;   // Bottom #5
		Cfg.JoyButtonBinds[ "MFD" ][ 11 ] = XWing::Control::TARGET_NEAREST_ENEMY;     // Bottom #4
		Cfg.JoyButtonBinds[ "MFD" ][ 12 ] = XWing::Control::TARGET_NEAREST_ATTACKER;  // Bottom #3
		Cfg.JoyButtonBinds[ "MFD" ][ 13 ] = XWing::Control::TARGET_NEWEST_INCOMING;   // Bottom #2
		Cfg.JoyButtonBinds[ "MFD" ][ 14 ] = XWing::Control::SHIELD_DIR;               // Bottom #1
		Cfg.JoyButtonBinds[ "MFD" ][ 15 ] = XWing::Control::TARGET_NOTHING;           // Left #5
		Cfg.JoyButtonBinds[ "MFD" ][ 16 ] = XWing::Control::WEAPON;                   // Left #4
		Cfg.JoyButtonBinds[ "MFD" ][ 17 ] = XWing::Control::MODE;                     // Left #3
		Cfg.JoyButtonBinds[ "MFD" ][ 18 ] = XWing::Control::TARGET_OBJECTIVE;         // Left #2
		Cfg.JoyButtonBinds[ "MFD" ][ 19 ] = XWing::Control::SCORES;                   // Left #1
	}
	
	Cfg.JoyAxisBinds[ "Wheel" ][ 0 ] = XWing::Control::ROLL;            // Steering Wheel
#if SDL_VERSION_ATLEAST(2,0,0)
	Cfg.JoyAxisBinds[ "Wheel" ][ 1 ] = XWing::Control::THROTTLE;        // Accelerator Pedal
	Cfg.JoyAxisBinds[ "Wheel" ][ 2 ] = XWing::Control::PITCH_UP;        // Brake Pedal
#else
	Cfg.JoyAxisBinds[ "Wheel" ][ 2 ] = XWing::Control::THROTTLE;        // Accelerator Pedal
	Cfg.JoyAxisBinds[ "Wheel" ][ 3 ] = XWing::Control::PITCH_UP;        // Brake Pedal
#endif
	Cfg.JoyAxisBinds[ "Wheel" ][ 4 ] = XWing::Control::PITCH_DOWN;      // Clutch Pedal
	
	Cfg.JoyButtonBinds[ "Wheel" ][  0 ] = XWing::Control::TARGET_NEAREST_ENEMY;     // Shift Red Button #1 (Left)
	Cfg.JoyButtonBinds[ "Wheel" ][  1 ] = XWing::Control::TARGET_NEAREST_ATTACKER;  // Shift Red Button #2
	Cfg.JoyButtonBinds[ "Wheel" ][  2 ] = XWing::Control::TARGET_PREV;              // Shift Red Button #3
	Cfg.JoyButtonBinds[ "Wheel" ][  3 ] = XWing::Control::TARGET_NEXT;              // Shift Red Button #4 (Right)
	Cfg.JoyButtonBinds[ "Wheel" ][  4 ] = XWing::Control::YAW_RIGHT;                // Wheel Right Paddle
	Cfg.JoyButtonBinds[ "Wheel" ][  5 ] = XWing::Control::YAW_LEFT;                 // Wheel Left Paddle
	Cfg.JoyButtonBinds[ "Wheel" ][  6 ] = XWing::Control::FIRE;                     // Wheel Top Right Button
	Cfg.JoyButtonBinds[ "Wheel" ][  7 ] = XWing::Control::TARGET_CROSSHAIR;         // Wheel Top Left Button
	Cfg.JoyButtonBinds[ "Wheel" ][  8 ] = XWing::Control::ROLL_RIGHT;               // 1st Gear
	Cfg.JoyButtonBinds[ "Wheel" ][  9 ] = XWing::Control::ROLL_LEFT;                // 2nd Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 10 ] = XWing::Control::PITCH_DOWN;               // 3rd Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 11 ] = XWing::Control::PITCH_UP;                 // 4th Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 12 ] = XWing::Control::ROLL_LEFT;                // 5th Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 13 ] = XWing::Control::ROLL_RIGHT;               // 6th Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 14 ] = XWing::Control::TARGET_SYNC;              // Reverse Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 15 ] = XWing::Control::SEAT_COCKPIT;             // Shift Top Black Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 16 ] = XWing::Control::SEAT_GUNNER1;             // Shift Left Black Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 17 ] = XWing::Control::LOOK_CENTER;              // Shift Bottom Black Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 18 ] = XWing::Control::SEAT_GUNNER2;             // Shift Right Black Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 19 ] = XWing::Control::MODE;                     // Wheel Middle Right Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 20 ] = XWing::Control::SHIELD_DIR;               // Wheel Middle Left Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 21 ] = XWing::Control::WEAPON;                   // Wheel Bottom Right Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 22 ] = XWing::Control::LOOK_CENTER;              // Wheel Bottom Left Button
	
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_UP    ] = XWing::Control::GLANCE_UP;     // Shift Hat Up
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_RIGHT ] = XWing::Control::GLANCE_RIGHT;  // Shift Hat Right
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_DOWN  ] = XWing::Control::GLANCE_BACK;   // Shift Hat Down
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_LEFT  ] = XWing::Control::GLANCE_LEFT;   // Shift Hat Left
}


void XWingGame::SetDefaults( void )
{
	std::string rebel_mission     = Cfg.SettingAsString( "rebel_mission",  "rebel0"  );
	std::string empire_mission    = Cfg.SettingAsString( "empire_mission", "empire0" );
	std::string rebel_difficulty  = Cfg.SettingAsString( "rebel_difficulty"  );
	std::string empire_difficulty = Cfg.SettingAsString( "empire_difficulty" );
	
	RaptorGame::SetDefaults();
	
	if( Cfg.Settings[ "name" ] == "Name" )
		Cfg.Settings[ "name" ] = "Rookie One";
	
	Cfg.Settings[ "host_address" ] = "www.raptor007.com";
	
	Cfg.Settings[ "view" ] = "auto";
	Cfg.Settings[ "spectator_view" ] = "auto";
	Cfg.Settings[ "spectator_sync" ] = "false";
	
	Cfg.Settings[ "g_fsaa" ] = (Raptor::Game->Gfx.DesktopW >= 3200) ? "2" : ( ((Raptor::Game->Gfx.DesktopW >= 1600) || (Raptor::Game->Gfx.DesktopW <= 0)) ? "3" : "4" );
	Cfg.Settings[ "g_bg" ] = "true";
	Cfg.Settings[ "g_stars" ] = "0";
	Cfg.Settings[ "g_debris" ] = "500";
	Cfg.Settings[ "g_debris_quality" ] = "3";
	Cfg.Settings[ "g_engine_glow" ] = "true";
	Cfg.Settings[ "g_effects" ] = "1";
	Cfg.Settings[ "g_sway" ] = "1";
	Cfg.Settings[ "g_asteroid_lod" ] = "1";
	Cfg.Settings[ "g_deathstar_trench" ] = "4";
	Cfg.Settings[ "g_deathstar_surface" ] = "4";
	Cfg.Settings[ "g_deathstar_bumpmap" ] = "true";
	Cfg.Settings[ "g_crosshair_thickness" ] = "1.5";
	Cfg.Settings[ "g_shader_blastpoints" ] = "20";
	Cfg.Settings[ "g_shader_blastpoint_quality" ] = "2";
	
	Cfg.Settings[ "ui_scale" ] = (Raptor::Game->Gfx.DesktopH >= 2000) ? "2" : ((Raptor::Game->Gfx.DesktopH >= 1400) ? "1.5" : ((Raptor::Game->Gfx.DesktopH >= 1000) ? "1.25" : "1")); // FIXME: Only Windows knows DesktopH during Initialize.
	Cfg.Settings[ "ui_box_color" ] = "1,1,1,1";
	Cfg.Settings[ "ui_box_color2" ] = "0,1,0,1";
	Cfg.Settings[ "ui_box_line" ] = (Raptor::Game->Gfx.DesktopH >= 2000) ? "2.5" : "1.5";
	Cfg.Settings[ "ui_box_style" ] = "1";
	Cfg.Settings[ "ui_classic" ] = "false";
	Cfg.Settings[ "ui_ship_preview" ] = "true";
	Cfg.Settings[ "ui_ship_rotate" ] = "20";
	Cfg.Settings[ "ui_pause" ] = "true";
	
	Cfg.Settings[ "s_engine_volume" ] = "0.9";
	Cfg.Settings[ "s_shield_alarm_radius" ] = "1";
	Cfg.Settings[ "s_menu_music" ] = "true";
	Cfg.Settings[ "s_game_music" ] = "true";
	Cfg.Settings[ "s_imuse" ] = "false";
	
	Cfg.Settings[ "vr_always" ] = "false";
	Cfg.Settings[ "vr_sway" ] = "0";
	Cfg.Settings[ "vr_messages" ] = "false";
	
	Cfg.Settings[ "joy_enable" ] = "true";
	Cfg.Settings[ "joy_hide_binds" ] = "true";
	
	Cfg.Settings[ "mouse_mode" ] = "disabled";
	Cfg.Settings[ "mouse_invert" ] = "true";
	Cfg.Settings[ "mouse_smooth" ] = "0.25";
	
	Cfg.Settings[ "swap_yaw_roll" ] = "false";
	Cfg.Settings[ "turret_invert" ] = "false";
	
	Cfg.Settings[ "net_zerolag" ] = "2";
	Cfg.Settings[ "sv_threads" ] = "0";
	
	Cfg.Settings[ "rebel_mission"  ] = rebel_mission;
	Cfg.Settings[ "empire_mission" ] = empire_mission;
	if( ! rebel_difficulty.empty() )
		Cfg.Settings[ "rebel_difficulty"  ] = rebel_difficulty;
	if( ! empire_difficulty.empty() )
		Cfg.Settings[ "empire_difficulty" ] = empire_difficulty;
	
	#ifndef __APPLE__
		Cfg.Settings[ "g_shader_version" ] = "130";  // Allow slightly more efficient worldspace blastpoint translation (flat in/out vs varying).
	#endif
	
	#ifdef APPLE_POWERPC
		Cfg.Settings[ "g_dynamic_lights" ] = "1";
		Cfg.Settings[ "g_debris" ] = "200";
		Cfg.Settings[ "g_deathstar_detail" ] = "2";
	#endif
}


void XWingGame::Setup( int argc, char **argv )
{
	Res.SearchPath.push_back( "Missions/Sounds" );
	
	bool screensaver = Cfg.SettingAsBool("screensaver");
	bool vr = Cfg.SettingAsBool("vr_always");
	bool safemode = false;
	for( int i = 1; i < argc; i ++ )
	{
		if( strcasecmp( argv[ i ], "-safe" ) == 0 )
		{
			safemode = true;
			vr = false;  // This still allows "-safe -vr" to enable both.
		}
		else if( strcasecmp( argv[ i ], "-vr" ) == 0 )
			vr = true;
	}
	
	// Campaign progress is stored separately so settings.cfg can be safely wiped.
	Cfg.Settings[ "rebel_mission"  ] = "rebel0";
	Cfg.Settings[ "empire_mission" ] = "empire0";
	Cfg.Load( "campaign.cfg" );
	
	// VR at startup is enabled by "vr_always" setting or "-vr" parameter.
	Cfg.Settings[ "vr_enable" ] = vr ? "true" : "false";
	
	if( screensaver )
	{
		Cfg.Settings[ "g_vsync" ] = Cfg.SettingAsBool( "screensaver_vsync", true );
		Cfg.Settings[ "s_menu_music" ] = "false";
		Cfg.Settings[ "s_game_music" ] = "false";
		Cfg.Settings[ "saitek_enable" ] = "false";
		Cfg.Settings[ "view" ] = Cfg.SettingAsString( "screensaver_view", "cycle" );
		Cfg.Settings[ "spectator_view" ] = "auto";
		Cfg.Settings[ "sv_netrate" ] = Cfg.Settings[ "sv_maxfps" ] = "30";
	}
	else
		Cfg.Settings[ "view" ] = "auto";
	
	if( safemode )
	{
		Cfg.Settings[ "precache" ] = "false";
		Cfg.Settings[ "g_group_skins" ] = "false";
	}
	
	// Set music to shuffle.
	Snd.ShuffleMusic = true;
	
	// Show a loading screen while precaching resources.
	Gfx.Clear( 0.f, 0.f, 0.f );
	FirstLoadScreen *load_screen = new FirstLoadScreen( screensaver ? "bg_menu_small.ani" : "bg_menu.ani" );
	Layers.Add( load_screen );
	Layers.Draw();
	Gfx.SwapBuffers();
	Layers.Remove( load_screen );
	load_screen = NULL;
	SDL_Delay( 1 );
	
	// If enabled, play the menu music.
	if( Cfg.SettingAsBool("s_menu_music") )
		Snd.PlayMusicSubdir( "Menu" );
	else
		Snd.StopMusic();
	
	// Start playing music while we load other resources.
	Snd.Update();
	
	int precache = Cfg.SettingAsInt( "precache", 2 );
	
	if( ! screensaver )
	{
		// Add the main menu to the now-empty Layers stack.
		MainMenu *main_menu = new MainMenu();
		Layers.Add( main_menu );
		
		// Precache mode 2 (new default) is done by the main menu, to make sure the menu music has a chance to start first.
		if( precache == 2 )
			main_menu->NeedPrecache = true;
	}
	else
	{
		// Add an input handler to quit the screensaver when necessary.
		AddScreensaverLayer();
		
		// Screensaver never shows the main menu, so any precache must be done now.
		if( precache == 2 )
			precache = 1;
	}
	
	if( ! screensaver )
		Res.GetShader("deathstar");
	
	if( Cfg.SettingAsInt("g_shader_blastpoints") )
		Res.GetShader("asteroid");
	
	Res.GetShader("asteroid_far");
	Res.GetShader("model_hud");
	
	// Load and select the model shader, but don't activate it yet.
	ShaderMgr.Select( Res.GetShader("model") );
	
	// Generate all framebuffers for render-to-texture.
	Res.GetFramebuffer( "health", 384, 512 );
	Res.GetFramebuffer( "target", 384, 512 );
	Res.GetFramebuffer( "intercept", 384, 256 );
	Res.GetFramebuffer( "throttle", 32, 256 );
	
	// Precache mode 1 (old way) loads resources while the loading screen is still being shown.
	if( precache == 1 )
		Precache();
}


void XWingGame::Precache( void )
{
	Res.GetAnimation("explosion.ani");
	Res.GetAnimation("laser_red.ani");
	Res.GetAnimation("laser_green.ani");
	Res.GetAnimation("ion_cannon.ani");
	Res.GetAnimation("torpedo.ani");
	Res.GetAnimation("missile.ani");
	
	Res.GetModel("torpedo.obj");
	Res.GetModel("missile.obj");
	Res.GetModel("turret_body.obj");
	Res.GetModel("turret_gun.obj");
	
	// Technically these are played at zero volume in the screensaver.
	Res.GetSound("laser_red.wav");
	Res.GetSound("laser_red_2.wav");
	Res.GetSound("laser_green.wav");
	Res.GetSound("laser_green_2.wav");
	Res.GetSound("torpedo.wav");
	Res.GetSound("torpedo_fly.wav");
	Res.GetSound("missile.wav");
	Res.GetSound("missile_fly.wav");
	Res.GetSound("turbolaser_green.wav");
	Res.GetSound("turbolaser_green_2.wav");
	Res.GetSound("turbolaser_red.wav");
	Res.GetSound("turbolaser_red_2.wav");
	Res.GetSound("laser_turret.wav");
	Res.GetSound("laser_turret_2.wav");
	Res.GetSound("ion_cannon.wav");
	Res.GetSound("ion_cannon_2.wav");
	Res.GetSound("explosion.wav");
	Res.GetSound("explosion_2.wav");
	Res.GetSound("explosion_3.wav");
	Res.GetSound("damage_hull.wav");
	Res.GetSound("damage_shield.wav");
	Res.GetSound("hit_hull.wav");
	Res.GetSound("hit_shield.wav");
	Res.GetSound("beep_aim.wav");
	Res.GetSound("jump_in.wav");
	Res.GetSound("jump_in_2.wav");
	
	bool screensaver = Cfg.SettingAsBool("screensaver");
	if( screensaver )
	{
		Res.GetAnimation( Cfg.SettingAsString( "screensaver_bg", "nebula2" ) + std::string(".ani") );
		if( Cfg.SettingAsBool( "screensaver_asteroids", true ) )
			Res.GetModel("asteroid.obj");
	}
	else
	{
		Res.GetAnimation("nebula.ani");
		Res.GetAnimation("nebula2.ani");
		Res.GetAnimation("stars.ani");
		Res.GetAnimation("bg_lobby.ani");
		
		Res.GetAnimation("special/health.ani");
		Res.GetAnimation("special/target.ani");
		Res.GetAnimation("special/intercept.ani");
		Res.GetAnimation("special/throttle.ani");
		
		Res.GetAnimation("shields_c.ani");
		Res.GetAnimation("shields_f.ani");
		Res.GetAnimation("shields_r.ani");
		Res.GetAnimation("mic.ani");
		
		Res.GetAnimation("superlaser.ani");
		Res.GetAnimation("deathstar.ani");
		
		Res.GetModel("asteroid.obj");
		Res.GetModel("deathstar_detail.obj");
		Res.GetModel("deathstar_detail_bottom.obj");
		Res.GetModel("deathstar_box.obj");
		Res.GetModel("deathstar_exhaust_port.obj");
		
		Res.GetSound("beep.wav");
		Res.GetSound("beep_error.wav");
		Res.GetSound("beep_store.wav");
		Res.GetSound("beep_sc.wav");
		Res.GetSound("beep_sf.wav");
		Res.GetSound("beep_sr.wav");
		Res.GetSound("beep_laser1.wav");
		Res.GetSound("beep_laser2.wav");
		Res.GetSound("beep_laser3.wav");
		Res.GetSound("beep_ion1.wav");
		Res.GetSound("beep_ion2.wav");
		Res.GetSound("beep_ion3.wav");
		Res.GetSound("beep_torpedo1.wav");
		Res.GetSound("beep_torpedo2.wav");
		Res.GetSound("beep_respawn.wav");
		Res.GetSound("locking.wav");
		Res.GetSound("locked.wav");
		Res.GetSound("lock_warn.wav");
		Res.GetSound("incoming.wav");
		Res.GetSound("shield_alarm.wav");
		Res.GetSound("checkpoint.wav");
		Res.GetSound("chat.wav");
		Res.GetSound("eject.wav");
		
		Res.GetSound("damage_hull_2.wav");
		Res.GetSound("damage_crit.wav");
		Res.GetSound("torpedo_enter.wav");
		Res.GetSound("jump_in_cockpit.wav");
		Res.GetSound("jump_out.wav");
		Res.GetSound("jump_out_cockpit.wav");
		Res.GetSound("repair.wav");
		Res.GetSound("rearm.wav");
		
		Res.GetMusic("rebel.dat");
		Res.GetMusic("empire.dat");
		Res.GetMusic("victory.dat");
		Res.GetMusic("defeat.dat");
		
		Res.GetSound("deathstar_30min.wav");
		Res.GetSound("deathstar_15min.wav");
		Res.GetSound("deathstar_7min.wav");
		Res.GetSound("deathstar_5min.wav");
		Res.GetSound("deathstar_3min.wav");
		Res.GetSound("deathstar_1min.wav");
		Res.GetSound("deathstar_30sec.wav");
		Res.GetSound("deathstar_0sec.wav");
		Res.GetSound("superlaser.wav");
		
		Res.GetSound("great_shot.wav");
		Res.GetSound("impressive.wav");
		Res.GetSound("powerful.wav");
		Res.GetSound("failed.wav");
	}
	
	if( DIR *dir_p = opendir("Ships") )
	{
		std::set<std::string> screensaver_ships;
		if( screensaver )
		{
			std::string gametype = Cfg.SettingAsString( "screensaver_gametype", "fleet" );
			if( gametype == "yavin" )
			{
				screensaver_ships.insert( Cfg.SettingAsString( "screensaver_rebel_fighter",  "X/W" ) );
				screensaver_ships.insert( Cfg.SettingAsString( "screensaver_rebel_bomber",   "Y/W" ) );
				screensaver_ships.insert( Cfg.SettingAsString( "screensaver_empire_fighter", "T/F" ) );
			}
			else
			{
				screensaver_ships.insert( Cfg.SettingAsString( "screensaver_rebel_fighter",  "X/W" ) );
				screensaver_ships.insert( Cfg.SettingAsString( "screensaver_empire_fighter", "T/I" ) );
				if( gametype == "fleet" )
				{
					screensaver_ships.insert( Cfg.SettingAsString( "screensaver_rebel_bomber",  "A/W" ) );
					screensaver_ships.insert( Cfg.SettingAsString( "screensaver_empire_bomber", "T/F" ) );
				}
			}
			
			std::string spawn = Cfg.SettingAsString( "screensaver_spawn", "GUN,YT1300,T/A" );
			if( ! spawn.empty() )
			{
				std::list<std::string> spawn_list = Str::SplitToList( spawn, "," );
				for( std::list<std::string>::iterator spawn_iter = spawn_list.begin(); spawn_iter != spawn_list.end(); spawn_iter ++ )
					screensaver_ships.insert( *spawn_iter );
			}
		}
		
		while( struct dirent *dir_entry_p = readdir(dir_p) )
		{
			if( dir_entry_p->d_name[ 0 ] == '.' )
				continue;
			if( ! CStr::EndsWith( dir_entry_p->d_name, ".def" ) )
				continue;
			
			ShipClass sc;
			if( sc.Load( std::string("Ships/") + std::string(dir_entry_p->d_name) ) )
			{
				if( sc.Secret )
					continue;
				if( screensaver && ! screensaver_ships.count(sc.ShortName) )
					continue;
				
				if( sc.ExternalModel.length() )
					Res.GetModel( sc.ExternalModel );
				if( sc.CockpitModel.length() )
					Res.GetModel( sc.CockpitModel );
				if( sc.CockpitModelVR.length() )
					Res.GetModel( sc.CockpitModelVR );
				if( sc.TurretBody.length() )
					Res.GetModel( sc.TurretBody );
				if( sc.TurretGun.length() )
					Res.GetModel( sc.TurretGun );
				
				if( Cfg.SettingAsBool("g_group_skins",true) && ! screensaver )
				{
					for( std::map<uint8_t,std::string>::const_iterator skin_iter = sc.GroupSkins.begin(); skin_iter != sc.GroupSkins.end(); skin_iter ++ )
						Res.GetModel( skin_iter->second );
					for( std::map<uint8_t,std::string>::const_iterator skin_iter = sc.GroupCockpits.begin(); skin_iter != sc.GroupCockpits.end(); skin_iter ++ )
						Res.GetModel( skin_iter->second );
				}
				
				if( Cfg.SettingAsBool("g_engine_glow",true) )
				{
					for( std::vector<ShipClassEngine>::const_iterator engine_iter = sc.Engines.begin(); engine_iter != sc.Engines.end(); engine_iter ++ )
						Res.GetAnimation( engine_iter->Texture );
				}
				
				for( std::map< double, std::string >::const_iterator flyby_iter = sc.FlybySounds.begin(); flyby_iter != sc.FlybySounds.end(); flyby_iter ++ )
					Res.GetSound( flyby_iter->second );
			}
		}
		closedir( dir_p );
	}
}


void XWingGame::AddScreensaverLayer( void )
{
	Screensaver *screensaver_layer = new Screensaver();
	screensaver_layer->IgnoreKeys.insert( SDLK_F12 );
	screensaver_layer->IgnoreKeys.insert( SDLK_PRINT );
	for( std::map<SDLKey,uint8_t>::const_iterator bind_iter = Cfg.KeyBinds.begin(); bind_iter != Cfg.KeyBinds.end(); bind_iter ++ )
	{
		if( (bind_iter->second == XWing::Control::THROTTLE_33       ) // Spectate Prev
		||  (bind_iter->second == XWing::Control::THROTTLE_66       ) // Spectate Next
		||  (bind_iter->second == XWing::Control::TARGET_NEXT_PLAYER)
		||  (bind_iter->second == XWing::Control::TARGET_PREV_PLAYER)
		||  (bind_iter->second == XWing::Control::LOOK_CENTER       )
		||  (bind_iter->second == XWing::Control::LOOK_UP           )
		||  (bind_iter->second == XWing::Control::LOOK_DOWN         )
		||  (bind_iter->second == XWing::Control::LOOK_LEFT         )
		||  (bind_iter->second == XWing::Control::LOOK_RIGHT        )
		||  (bind_iter->second == XWing::Control::LOOK_UP_LEFT      )
		||  (bind_iter->second == XWing::Control::LOOK_UP_RIGHT     )
		||  (bind_iter->second == XWing::Control::LOOK_DOWN_LEFT    )
		||  (bind_iter->second == XWing::Control::LOOK_DOWN_RIGHT   )
		||  (bind_iter->second == XWing::Control::GLANCE_UP         )
		||  (bind_iter->second == XWing::Control::GLANCE_BACK       )
		||  (bind_iter->second == XWing::Control::GLANCE_LEFT       )
		||  (bind_iter->second == XWing::Control::GLANCE_RIGHT      )
		||  (bind_iter->second == XWing::Control::GLANCE_UP_LEFT    )
		||  (bind_iter->second == XWing::Control::GLANCE_UP_RIGHT   )
		||  (bind_iter->second == XWing::Control::GLANCE_BACK_LEFT  )
		||  (bind_iter->second == XWing::Control::GLANCE_BACK_RIGHT )
		||  (bind_iter->second == XWing::Control::PAUSE             )
		||  (bind_iter->second == XWing::Control::SCORES            )
		|| ((bind_iter->second >= XWing::Control::VIEW_COCKPIT) && (bind_iter->second <= XWing::Control::VIEW_INSTRUMENTS)) )
			screensaver_layer->IgnoreKeys.insert( bind_iter->first );
	}
	Layers.Add( screensaver_layer );
}


void XWingGame::Update( double dt )
{
	// Apply time_scale to various timers.
	
	dt *= Data.TimeScale;
	
	RoundTimer.SetTimeScale( Data.TimeScale );
	RespawnTimer.SetTimeScale( Data.TimeScale );
	
	for( std::list<Effect>::iterator effect_iter = Data.Effects.begin(); effect_iter != Data.Effects.end(); effect_iter ++ )
	{
		effect_iter->Lifetime.SetTimeScale( Data.TimeScale );
		effect_iter->Anim.Timer.SetTimeScale( Data.TimeScale );
	}
	
	double recent_pan_time_scale = (Data.TimeScale >= 1.) ? 1. : Data.TimeScale;
	for( std::map<uint32_t,Clock>::iterator sound_iter = Snd.RecentPans.begin(); sound_iter != Snd.RecentPans.end(); sound_iter ++ )
		sound_iter->second.SetTimeScale( recent_pan_time_scale );
	
	
	BlastPoints = Cfg.SettingAsInt("g_shader_blastpoints");
	int zerolag = Cfg.SettingAsInt("net_zerolag",2);
	
	
	// Update ship's motion changes based on client's controls.
	
	double roll = 0.;
	double pitch = 0.;
	double yaw = 0.;
	static double throttle = 1.;
	bool firing = false;
	static bool error_beep_allowed = true;
	double glance_up_fwd = (View == XWing::View::COCKPIT) ? 70. : 0., glance_up_back = 0.;
	
	bool target_crosshair = false;
	bool target_nearest_enemy = false;
	bool target_nearest_attacker = false;
	bool target_target_attacker = false;
	bool target_newest = false;
	bool target_newest_incoming = false;
	bool target_objective = false;
	bool target_dockable = false;
	bool target_nothing = false;
	bool target_groupmate = false;
	bool target_sync = false;
	
	// Build a list of all ships because we'll refer to it often, and find the player's ship and/or turret.
	std::vector<Ship*> ships;
	std::vector<Shot*> missiles;
	Ship *my_ship = NULL;
	Turret *my_turret = NULL;
//#define DEATHSTAR_GRAVITY (1.625) // Approximate gravity of a small moon.
#ifdef DEATHSTAR_GRAVITY
	DeathStar *deathstar = NULL;
#endif
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->Type() == XWing::Object::SHIP )
		{
			Ship *ship = (Ship*) obj_iter->second;
			ships.push_back( ship );
			
			if( ship->PlayerID == PlayerID )
				my_ship = ship;
		}
		else if( obj_iter->second->Type() == XWing::Object::TURRET )
		{
			Turret *turret = (Turret*) obj_iter->second;
			if( turret->PlayerID == PlayerID )
				my_turret = turret;
		}
		else if( obj_iter->second->Type() == XWing::Object::SHOT )
		{
			Shot *shot = (Shot*) obj_iter->second;
			if( shot->CollisionType() == XWing::Object::SHOT_MISSILE )
				missiles.push_back( shot );
		}
#ifdef DEATHSTAR_GRAVITY
		else if( obj_iter->second->Type() == XWing::Object::DEATH_STAR )
			deathstar = (DeathStar*) obj_iter->second;
#endif
	}
	
	// Clear respawn timer when we're piloting or gunning.
	if( (my_ship && (my_ship->Health > 0.)) || my_turret )
		RespawnTimer.CountUpToSecs = 0.;
	
	Ship *observed_ship = (Ship*) Data.GetObject( ObservedShipID );
	if( ! observed_ship )
		observed_ship = my_ship;
	
	Mic.FromObject = my_turret ? my_turret->ID : (my_ship ? my_ship->ID : ObservedShipID);
	
	if( observed_ship && observed_ship->Class && (View == XWing::View::COCKPIT) )
	{
		glance_up_fwd  = observed_ship->Class->GlanceUpFwd;
		glance_up_back = observed_ship->Class->GlanceUpBack;
	}
	
	// Mouse
	std::string mouse_mode = Cfg.SettingAsString("mouse_mode");
	bool mouse_fly2 = (mouse_mode == "fly2");
	if( ReadMouse )
	{
		double mouse_x_percent = Mouse.X * 2. / Gfx.W - 1.;
		double mouse_y_percent = Mouse.Y * 2. / Gfx.H - 1.;
		if( Cfg.SettingAsBool("mouse_correction") )
		{
			double min_dim = (Gfx.H < Gfx.W) ? Gfx.H : Gfx.W;
			mouse_x_percent *= Gfx.W / min_dim;
			mouse_y_percent *= Gfx.H / min_dim;
		}
		
		if( mouse_fly2 || (mouse_mode == "fly") || (my_turret && (mouse_mode == "gunner")) )
		{
			// Read mouse position for yaw/pitch (or roll/pitch).
			if( mouse_fly2 )
				roll = (fabs(mouse_x_percent) <= 1.) ? mouse_x_percent : Num::Sign(mouse_x_percent);
			else // mouse_fly
				yaw = (fabs(mouse_x_percent) <= 1.) ? mouse_x_percent : Num::Sign(mouse_x_percent);
			
			pitch = (fabs(mouse_y_percent) <= 1.) ? mouse_y_percent : Num::Sign(mouse_y_percent);
			
			// If mouse_invert=false, reverse the pitch.
			if( ! Cfg.SettingAsBool("mouse_invert",true) )
				pitch *= -1.;
			
			double smooth = Cfg.SettingAsDouble("mouse_smooth");
			yaw = fabs(pow( fabs(yaw), smooth + 1. )) * Num::Sign(yaw);
			pitch = fabs(pow( fabs(pitch), smooth + 1. )) * Num::Sign(pitch);
		}
		else if( (Cfg.SettingAsString("mouse_mode") == "look") && (View != XWing::View::STATIONARY) && ! Head.VR )
		{
			LookYaw = ( (fabs(mouse_x_percent) <= 1.) ? mouse_x_percent : Num::Sign(mouse_x_percent) ) * 180.;
			LookPitch = ( (fabs(mouse_y_percent) <= 1.) ? mouse_y_percent : Num::Sign(mouse_y_percent) ) * -90.;
			ThumbstickLook = false;
		}
	}
	
	// Analog Axes
	roll  = Num::Clamp( roll  + Input.ControlTotal( XWing::Control::ROLL  ) + Input.ControlTotal( XWing::Control::ROLL_INVERTED  ) + Input.ControlTotal( XWing::Control::ROLL_RIGHT ) - Input.ControlTotal( XWing::Control::ROLL_LEFT  ), -1., 1. );
	pitch = Num::Clamp( pitch + Input.ControlTotal( XWing::Control::PITCH ) + Input.ControlTotal( XWing::Control::PITCH_INVERTED ) + Input.ControlTotal( XWing::Control::PITCH_UP   ) - Input.ControlTotal( XWing::Control::PITCH_DOWN ), -1., 1. );
	yaw   = Num::Clamp( yaw   + Input.ControlTotal( XWing::Control::YAW   ) + Input.ControlTotal( XWing::Control::YAW_INVERTED   ) + Input.ControlTotal( XWing::Control::YAW_RIGHT  ) - Input.ControlTotal( XWing::Control::YAW_LEFT   ), -1., 1. );
	double analog_throttle = throttle;
	if( Input.HasControlAxis( XWing::Control::THROTTLE ) || Input.HasControlAxis( XWing::Control::THROTTLE_INVERTED ) )
		analog_throttle = Num::Clamp( Input.ControlTotal( XWing::Control::THROTTLE ) + Input.ControlTotal( XWing::Control::THROTTLE_INVERTED ), 0., 1. );
	double look_x = Num::Clamp( Input.ControlValue( XWing::Control::LOOK_X ) + Input.ControlValue( XWing::Control::LOOK_X_INVERTED ), -1., 1. );
	double look_y = Num::Clamp( Input.ControlValue( XWing::Control::LOOK_Y ) + Input.ControlValue( XWing::Control::LOOK_Y_INVERTED ), -1., 1. );
	if( look_x || look_y )
		ThumbstickLook = true;
	if( ThumbstickLook )
	{
		LookYaw   = 120. * look_x;
		LookPitch = -90. * look_y;
		double pitch_rear = std::min<double>( 0., fabs(LookYaw) * 0.5 - 60. );
		if( LookPitch < pitch_rear )
		{
			// Thumbstick back means look behind, not down.
			double behind = LookPitch / -90.;
			LookPitch = glance_up_back * behind;
			LookYaw = ((LookYaw >= 0.) ? 180. : -180.) * behind + LookYaw * (1. - behind);
		}
		if( View == XWing::View::STATIONARY )
		{
			LookYaw   =  90. * look_x * FrameTime;
			LookPitch = -90. * look_y * FrameTime;
		}
	}
	
	// Held Digital Buttons
	double glance_x = Num::Clamp( Input.ControlTotal( XWing::Control::GLANCE_RIGHT ) + Input.ControlTotal( XWing::Control::GLANCE_UP_RIGHT  ) + Input.ControlTotal( XWing::Control::GLANCE_BACK_RIGHT ) - Input.ControlTotal( XWing::Control::GLANCE_LEFT ) - Input.ControlTotal( XWing::Control::GLANCE_UP_LEFT ) - Input.ControlTotal( XWing::Control::GLANCE_BACK_LEFT ), -1., 1. );
	double glance_y = Num::Clamp( Input.ControlTotal( XWing::Control::GLANCE_BACK  ) + Input.ControlTotal( XWing::Control::GLANCE_BACK_LEFT ) + Input.ControlTotal( XWing::Control::GLANCE_BACK_RIGHT ) - Input.ControlTotal( XWing::Control::GLANCE_UP   ) - Input.ControlTotal( XWing::Control::GLANCE_UP_LEFT ) - Input.ControlTotal( XWing::Control::GLANCE_UP_RIGHT  ), -1., 1. );
	if( View == XWing::View::STATIONARY )
	{
		LookYaw   +=  90. * glance_x * FrameTime;
		LookPitch += -90. * glance_y * FrameTime;
	}
	else if( glance_x || glance_y )
	{
		if( ! ThumbstickLook )
		{
			LookYaw = LookPitch = 0.;
			ThumbstickLook = true;
		}
		if( glance_x && glance_y )
		{
			LookYaw += ((glance_y < 0.) ? 45. : 135.) * glance_x;
			LookPitch += glance_up_back * 0.5 * fabs(glance_y);
		}
		else
		{
			LookYaw += 90. * glance_x;
			if( glance_y < 0. )
				LookPitch -= glance_up_fwd * glance_y;
			else if( glance_y )
			{
				LookYaw += 180.;
				LookPitch += glance_up_back * glance_y;
			}
		}
	}
	if( Input.ControlPressed( XWing::Control::ROLL_RIGHT ) || Input.ControlPressed( XWing::Control::ROLL_LEFT ) )
	{
		double digital_roll = Num::Clamp( Input.ControlTotal( XWing::Control::ROLL_RIGHT ) - Input.ControlTotal( XWing::Control::ROLL_LEFT ), -1., 1. );
		if( digital_roll )
			roll = digital_roll;
	}
	if( Input.ControlPressed( XWing::Control::PITCH_UP ) || Input.ControlPressed( XWing::Control::PITCH_DOWN ) )
	{
		double digital_pitch = Num::Clamp( Input.ControlTotal( XWing::Control::PITCH_UP ) - Input.ControlTotal( XWing::Control::PITCH_DOWN ), -1., 1. );
		if( digital_pitch )
			pitch = digital_pitch;
	}
	if( Input.ControlPressed( XWing::Control::YAW_RIGHT ) || Input.ControlPressed( XWing::Control::YAW_LEFT ) )
	{
		double digital_yaw = Num::Clamp( Input.ControlTotal( XWing::Control::YAW_RIGHT ) - Input.ControlTotal( XWing::Control::YAW_LEFT ), -1., 1. );
		if( digital_yaw )
			yaw = digital_yaw;
	}
	if( Input.ControlPressed( XWing::Control::THROTTLE_100 ) )
		throttle = 1.;
	else if( Input.ControlPressed( XWing::Control::THROTTLE_66 ) )
		throttle = 0.6667;
	else if( Input.ControlPressed( XWing::Control::THROTTLE_33 ) )
		throttle = 0.3333;
	else if( Input.ControlPressed( XWing::Control::THROTTLE_0 ) )
		throttle = 0.;
	else if( Input.ControlPressed( XWing::Control::THROTTLE_UP ) || Input.ControlPressed( XWing::Control::THROTTLE_DOWN ) )
	{
		double throttle_change = Num::Clamp( Input.ControlTotal( XWing::Control::THROTTLE_UP ) - Input.ControlTotal( XWing::Control::THROTTLE_DOWN ), -1., 1. );
		if( ! my_ship )
			;
		else if( throttle_change > 0. )
			throttle = std::max<double>( throttle, my_ship->GetThrottle() );
		else if( throttle_change < 0. )
			throttle = std::min<double>( throttle, my_ship->GetThrottle() );
		throttle += throttle_change * FrameTime / 2.;
	}
	else
		throttle = analog_throttle;
	if( Input.ControlPressed( XWing::Control::FIRE ) )
		firing = true;
	if( Input.ControlPressed( XWing::Control::TARGET_NOTHING ) )
		target_nothing = true;
	if( Input.ControlPressed( XWing::Control::TARGET_CROSSHAIR ) )
		target_crosshair = true;
	if( Input.ControlPressed( XWing::Control::TARGET_NEAREST_ENEMY ) )
		target_nearest_enemy = true;
	if( Input.ControlPressed( XWing::Control::TARGET_NEAREST_ATTACKER ) )
		target_nearest_attacker = true;
	if( Input.ControlPressed( XWing::Control::TARGET_TARGET_ATTACKER ) )
		target_target_attacker = true;
	if( Input.ControlPressed( XWing::Control::TARGET_NEWEST_INCOMING ) )
		target_newest_incoming = true;
	if( Input.ControlPressed( XWing::Control::TARGET_OBJECTIVE ) )
		target_objective = true;
	if( Input.ControlPressed( XWing::Control::TARGET_DOCKABLE ) )
		target_dockable = true;
	if( Input.ControlPressed( XWing::Control::TARGET_NEWEST ) )
		target_newest = true;
	if( Input.ControlPressed( XWing::Control::TARGET_GROUPMATE ) )
		target_groupmate = true;
	if( Input.ControlPressed( XWing::Control::TARGET_SYNC ) )
		target_sync = true;
	if( Input.ControlPressed( XWing::Control::LOOK_UP ) || Input.ControlPressed( XWing::Control::LOOK_UP_LEFT ) || Input.ControlPressed( XWing::Control::LOOK_UP_RIGHT ) )
	{
		LookPitch += 90. * FrameTime;
		ThumbstickLook = false;
	}
	if( Input.ControlPressed( XWing::Control::LOOK_DOWN ) || Input.ControlPressed( XWing::Control::LOOK_DOWN_LEFT ) || Input.ControlPressed( XWing::Control::LOOK_DOWN_RIGHT ) )
	{
		LookPitch -= 90. * FrameTime;
		ThumbstickLook = false;
	}
	if( Input.ControlPressed( XWing::Control::LOOK_LEFT ) || Input.ControlPressed( XWing::Control::LOOK_UP_LEFT ) || Input.ControlPressed( XWing::Control::LOOK_DOWN_LEFT ) )
	{
		LookYaw -= 90. * FrameTime;
		ThumbstickLook = false;
	}
	if( Input.ControlPressed( XWing::Control::LOOK_RIGHT ) || Input.ControlPressed( XWing::Control::LOOK_UP_RIGHT ) || Input.ControlPressed( XWing::Control::LOOK_DOWN_RIGHT ) )
	{
		LookYaw += 90. * FrameTime;
		ThumbstickLook = false;
	}
	if( Input.ControlPressed( XWing::Control::LOOK_CENTER ) )
	{
		LookPitch = 0.;
		LookYaw   = 0.;
		ThumbstickLook = false;
		Head.Recenter();
	}
	
	static bool voice_transmit = false;
	static uint8_t voice_channel = Raptor::VoiceChannel::TEAM;
	if( Input.ControlPressed( XWing::Control::VOICE_TEAM ) )
	{
		if( (! voice_transmit) || (voice_channel != Raptor::VoiceChannel::TEAM) )
			voice_transmit = Mic.Start( Raptor::VoiceChannel::TEAM );
		voice_channel = Raptor::VoiceChannel::TEAM;
	}
	else if( Input.ControlPressed( XWing::Control::VOICE_ALL ) )
	{
		if( (! voice_transmit) || (voice_channel != Raptor::VoiceChannel::ALL) )
			voice_transmit = Mic.Start( Raptor::VoiceChannel::ALL );
		voice_channel = Raptor::VoiceChannel::ALL;
	}
	else if( voice_transmit )
	{
		Mic.Stop();
		voice_transmit = false;
	}
	
	static double target_wait = 0.;
	
	if(!( target_crosshair
	||    target_nearest_enemy
	||    target_nearest_attacker
	||    target_newest
	||    target_newest_incoming
	||    target_target_attacker
	||    target_objective
	||    target_dockable
	||    target_nothing
	||    target_groupmate
	||    target_sync ))
	{
		error_beep_allowed = true;
		target_wait = 0.;
	}
	else if( target_crosshair && target_wait && (my_ship || my_turret) )
	{
		const GameObject *my_pos = my_turret ? (const GameObject*) my_turret : (const GameObject*) my_ship;
		if( my_pos->Lifetime.ElapsedSeconds() < target_wait )
			target_crosshair = false;
	}
	
	
	// Make sure the throttle value is legit.
	if( throttle > 1. )
		throttle = 1.;
	else if( throttle < 0. )
		throttle = 0.;
	
	
	// Apply controls to player's ship or turret.
	
	uint32_t target_id = 0;
	uint8_t target_subsystem = 0;
	const char *beep = NULL;
	const Ship *my_turret_parent = NULL;
	bool ejecting = false;
	static bool play_eject_sound = true;
	std::vector<Shot*> fire_shots;
	
	if( Cfg.SettingAsBool("swap_yaw_roll") )
	{
		double yaw_input = yaw;
		yaw = roll;
		roll = yaw_input;
	}
	
	if( my_turret )
	{
		target_id = my_turret->Target;
		
		my_turret_parent = my_turret->ParentShip();
		if( (! my_turret_parent) || (my_turret_parent->JumpProgress >= 1.) )
		{
			my_turret->SetYaw( yaw + roll );
			my_turret->SetPitch( pitch * (Cfg.SettingAsBool("turret_invert") ? -1. : 1.) );
			my_turret->Firing = firing;
			if( zerolag && firing && (my_turret->FiringClock.ElapsedSeconds() >= my_turret->ShotDelay()) )
			{
				fire_shots = my_turret->NextShots();
				my_turret->PredictedShots ++;
			}
		}
		
		// Level off the ship if we moved from cockpit to gunner.
		roll = 0.;
		pitch = 0.;
		yaw = 0.;
	}
	
	if( my_ship )
	{
		if( ! my_turret )
			target_id = my_ship->Target;
		target_subsystem = my_ship->TargetSubsystem;
		
		if( (my_ship->Health > 0.) && (my_ship->JumpProgress >= 1.) )
		{
			my_ship->SetRoll( roll, dt );
			my_ship->SetPitch( pitch, dt );
			my_ship->SetYaw( yaw, dt );
			
#ifdef DEATHSTAR_GRAVITY
			if( (! my_turret) && ((! deathstar) || ( (throttle > 0.1) || (my_ship->GetThrottle() * 0.999 > throttle) )) )
#else
			if( ! my_turret )
#endif
			{
				my_ship->Firing = firing;
				if( zerolag && firing && my_ship->SelectedWeapon && my_ship->AmmoForWeapon() && ((zerolag >= 2) || (my_ship->MaxAmmo() < 0)) && (my_ship->LastFired() >= my_ship->ShotDelay()) )
				{
					fire_shots = my_ship->NextShots();
					my_ship->PredictedShots.push_back( my_ship->CurrentFiringMode() | (my_ship->SelectedWeapon << 4) );
				}
				
				std::string engine_sound = my_ship->SetThrottleGetSound( throttle, dt );
				
				if( engine_sound.length() )
				{
					// Just use the engine sound (and s_engine_volume) playback code in ProcessPacket rather than copy-pasting it here.
					Packet play_sound( XWing::Packet::ENGINE_SOUND );
					play_sound.AddString( engine_sound );
					//play_sound.AddUInt( my_ship->ID );  // Not needed now, but may be helpful if we ever want spectators to hear engine sounds.
					ProcessPacket( &play_sound );
					
					// If we have any player turret gunners, send the sound to the server, so the server can send it to gunners.
					std::list<Turret*> turrets = my_ship->AttachedTurrets();
					for( std::list<Turret*>::const_iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
					{
						if( (*turret_iter)->PlayerID && ((*turret_iter)->PlayerID != PlayerID) )
						{
							Net.Send( &play_sound );
							break;  // The server will pass this on to all gunners, so we only send it once.
						}
					}
				}
				
				ejecting = Input.ControlPressed( XWing::Control::EJECT );
				if( ejecting )
				{
					double eject_held = EjectHeld.ElapsedSeconds();
					if( eject_held >= 3. )
					{
						EjectHeld.Reset();
						Packet eject( XWing::Packet::EXPLOSION );
						eject.AddUInt( my_ship->ID );
						Net.Send( &eject );
					}
					else if( play_eject_sound && (eject_held >= 0.25) )
					{
						Snd.Play( Res.GetSound("eject.wav") );
						play_eject_sound = false;
					}
				}
			}
			
#ifdef DEATHSTAR_GRAVITY
			if( deathstar )
			{
				// Apply gravity when flying too slowly over the Death Star.
				// FIXME: Currently disabled because this implementation doesn't quite work as intended.
				double speed = my_ship->MotionVector.Length();
				double falling = my_ship->PrevMotionVector.Dot(&(deathstar->Up)) * -1.;
				if( (speed - falling < 50.) || (speed < 20.) )
				{
					double falling2 = my_ship->MotionVector.Dot(&(deathstar->Up)) * -1.;
					if( falling > falling2 )
						my_ship->MotionVector -= deathstar->Up * (falling - falling2);
					my_ship->MotionVector -= deathstar->Up * dt * DEATHSTAR_GRAVITY;
					double new_speed = my_ship->MotionVector.Length();
					if( new_speed > my_ship->MaxSpeed() )
						my_ship->MotionVector.ScaleTo( my_ship->MaxSpeed() );
					else
					{
						falling2 = my_ship->MotionVector.Dot(&(deathstar->Up)) * -1.;
						double prev_speed = my_ship->PrevMotionVector.Length();
						if( (falling2 > 0.) && (speed < prev_speed) )
							my_ship->MotionVector.ScaleTo( prev_speed );
					}
				}
			}
#endif
		}
		else
		{
			my_ship->RollRate = 0.;
			my_ship->PitchRate = 0.;
			my_ship->YawRate = 0.;
			my_ship->Firing = false;
			target_id = my_ship->NextCheckpoint;
			target_subsystem = 0;
			
			if( my_ship->Health > 0. )  // Arriving from hyperspace.
			{
				my_ship->SetThrottle( 1., 999. );
				throttle = 1.;
			}
		}
	}
	
	if( (my_ship && (my_ship->Health > 0.) && (my_ship->JumpProgress >= 1.)) || my_turret )
	{
		// Apply targeting.
		
		const GameObject *my_pos = my_turret ? (const GameObject*) my_turret : (const GameObject*) my_ship;
		uint32_t my_ship_id = my_turret_parent ? my_turret_parent->ID : (my_ship ? my_ship->ID : 0);
		uint32_t my_checkpoint = my_turret_parent ? my_turret_parent->NextCheckpoint : (my_ship ? my_ship->NextCheckpoint : 0);
		uint8_t my_team = my_turret ? my_turret->Team : my_ship->Team;
		
		if( target_nothing || (my_ship && my_ship->JumpedOut) )
		{
			if( target_id )
			{
				target_id = 0;
				target_subsystem = 0;
				if( target_nothing )
					beep = "beep.wav";
			}
		}
		else if( target_crosshair )
		{
			uint32_t id = 0;
			uint8_t subsystem = target_subsystem;
			
			Vec3D my_fwd = my_turret ? my_turret->GunPos().Fwd : my_ship->Fwd;
			Pos3D ahead( my_pos );
			ahead.MoveAlong( &my_fwd, 30000. );
			
			Ship *best_ship = NULL;
			double best_dot = 0., best_dist = 0.;
			std::string best_hit;
			
			if( my_checkpoint )
			{
				const GameObject *checkpoint = Data.GetObject( my_checkpoint );
				if( checkpoint )
				{
					Vec3D vec_to_checkpoint( checkpoint->X - my_pos->X, checkpoint->Y - my_pos->Y, checkpoint->Z - my_pos->Z );
					vec_to_checkpoint.ScaleTo( 1. );
					double dot = my_fwd.Dot( &vec_to_checkpoint );
					if( dot > 0. )
					{
						best_dot  = dot;
						best_dist = my_pos->Dist( checkpoint );
						id = my_checkpoint;
					}
				}
			}
			
			for( std::vector<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				if( (*ship_iter)->ID != my_ship_id )
				{
					Ship *ship = *ship_iter;
					
					if( ship->Health <= 0. )
						continue;
					if( (ship->JumpProgress < 1.) || ship->JumpedOut )
						continue;
					
					if( (ship->Category() == ShipClass::CATEGORY_CAPITAL) && ship->ComplexCollisionDetection() )
					{
						// Don't bother checking capital ships that aren't directly ahead of us.
						double ship_triagonal = ship->Shape.GetTriagonal();
						if( Math3D::PointToLineSegDist( ship, my_pos, &ahead ) > ship_triagonal )
							continue;
						Pos3D ship_far_end = *ship + my_fwd * ship_triagonal / 2.;
						Vec3D vec_to_end = ship_far_end - my_pos;
						if( my_fwd.Dot(&vec_to_end) < 0.9 )
							continue;
						
						std::string hit;
						double dist = ship->Shape.DistanceFromLine( ship, NULL, NULL, &hit, ship->Exploded(), ship->ExplosionSeed(), my_pos, &ahead );
						if( (dist < (ship_triagonal * 0.01)) && ! hit.empty() )
						{
							if( best_hit.empty() || (dist < best_dist) )
							{
								id = ship->ID;
								best_ship = ship;
								best_hit  = hit;
								best_dist = dist;
								best_dot  = 1.;
								continue;
							}
						}
					}
					
					Vec3D vec_to_ship( ship->X - my_pos->X, ship->Y - my_pos->Y, ship->Z - my_pos->Z );
					vec_to_ship.ScaleTo( 1. );
					double dot = my_fwd.Dot( &vec_to_ship );
					
					if( (dot > 0.) && ((dot > best_dot) || ! id) )
					{
						id = ship->ID;
						best_ship = ship;
						best_dot  = dot;
					}
				}
			}
			
			for( std::vector<Shot*>::const_iterator shot_iter = missiles.begin(); shot_iter != missiles.end(); shot_iter ++ )
			{
				const Shot *shot = *shot_iter;
				
				Vec3D vec_to_shot( shot->X - my_pos->X, shot->Y - my_pos->Y, shot->Z - my_pos->Z );
				vec_to_shot.ScaleTo( 1. );
				double dot = my_fwd.Dot( &vec_to_shot );
				
				if( (dot > 0.) && ((dot > best_dot) || ! id) )
				{
					id = shot->ID;
					subsystem = 0;
					best_ship = NULL;
					best_dot  = dot;
				}
			}
			
			if( best_ship )
				subsystem = best_hit.empty() ? 0 : best_ship->SubsystemID( best_hit );
			
			if( (target_id != id) || (target_subsystem != subsystem) )
			{
				target_id = id;
				target_subsystem = subsystem;
				beep = "beep.wav";
			}
			
			// Checking capital ships takes a while, so don't tank the framerate.
			target_wait = my_pos->Lifetime.ElapsedSeconds() + 0.25;
		}
		else if( target_nearest_enemy || target_nearest_attacker || target_target_attacker )
		{
			double best = 0.;
			uint8_t category = ShipClass::CATEGORY_TARGET;
			uint32_t id = 0;
			const Ship *victim = my_ship;
			
			// FIXME: Dirty hack to fix error beep being played after successful retargeting.
			if( target_target_attacker )
			{
				victim = (Ship*) Data.GetObject( target_id );
				if( victim && (victim->Type() == XWing::Object::SHIP) && (victim->Team != my_team) )
					error_beep_allowed = false;
			}
			
			for( std::vector<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// FIXME: Dirty hack to make sure we don't rapidly cycle in FFA.
				if( target_target_attacker && ! error_beep_allowed )
					break;
				
				if( (*ship_iter)->ID != my_ship_id )
				{
					const Ship *ship = *ship_iter;
					
					if( ship->Health <= 0. )
						continue;
					if( (ship->JumpProgress < 1.) || ship->JumpedOut )
						continue;
					if( my_team && (ship->Team == my_team) )
						continue;
					
					if( target_nearest_attacker || target_target_attacker )
					{
						if( ship->Category() == ShipClass::CATEGORY_CAPITAL )
							continue;
						
						// FIXME: Optimize by skipping the entire loop if (target_target_attacker || target_target_attacker) && ! victim.
						if( ! victim )
							continue;
						if( victim->Type() != XWing::Object::SHIP )
							continue;
						if( ship->Dist(victim) > 8000. )
							continue;
						
						if( (ship->Target != victim->ID)
						&& ! ((victim->HitByID == ship->ID) && (victim->HitClock.ElapsedSeconds() < 2.) && ! (victim->HitFlags & Ship::HIT_REPAIR)) )
						{
							std::list<Turret*> attached_turrets = ship->AttachedTurrets();
							bool found_turret = false;
							for( std::list<Turret*>::const_iterator turret_iter = attached_turrets.begin(); turret_iter != attached_turrets.end(); turret_iter ++ )
							{
								if( ((*turret_iter)->Target == victim->ID)
								||  ((victim->HitByID == (*turret_iter)->ID) && (victim->HitClock.ElapsedSeconds() < 2.) && ! (victim->HitFlags & Ship::HIT_REPAIR)) )
								{
									found_turret = true;
									break;
								}
							}
							if( ! found_turret )
								continue;
						}
					}
					
					// Prefer fighters/bombers/gunboats over transports/capital.
					uint8_t ship_category = ship->Category();
					if( (ship_category > category) && (ship_category >= ShipClass::CATEGORY_TRANSPORT) )
						continue;
					if( (category >= ShipClass::CATEGORY_TRANSPORT) && (ship_category < category) )
						id = 0;
					
					double dist = my_pos->Dist( ship );
					if( (dist < best) || ! id )
					{
						best = dist;
						id = ship->ID;
						category = ship_category;
					}
				}
			}
			
			// FIXME: Dirty hack to make sure we don't rapidly cycle in FFA.
			if( target_target_attacker && id )
				error_beep_allowed = false;
			
			// Keep existing target if there's no attacker.
			if( (! id) && ! target_nearest_enemy )
			{
				if( error_beep_allowed )
					beep = "beep_error.wav";
			}
			else if( target_id != id )
			{
				target_id = id;
				target_subsystem = 0;
				beep = "beep.wav";
			}
		}
		else if( target_newest_incoming )
		{
			double best = 0.;
			uint32_t id = 0;
			
			for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
			{
				if( obj_iter->second->Type() == XWing::Object::SHOT )
				{
					Shot *shot = (Shot*) obj_iter->second;
					
					if( shot->Seeking != my_ship_id )
						continue;
					
					double lifetime = shot->Lifetime.ElapsedSeconds();
					if( (lifetime < best) || ! id )
					{
						best = lifetime;
						id = shot->ID;
					}
				}
			}
			
			if( ! id )
				beep = "beep_error.wav";
			else if( target_id != id )
			{
				target_id = id;
				target_subsystem = 0;
				beep = "beep.wav";
			}
		}
		else if( target_objective )
		{
			uint32_t id = 0;
			std::string mission_objs = Data.PropertyAsString("mission_objs");
			
			if( (((GameType == XWing::GameType::TEAM_RACE) || (GameType == XWing::GameType::FFA_RACE)) && (mission_objs.empty() || (mission_objs == "default"))) || (mission_objs == "checkpoint") )
				id = my_checkpoint;
			else
			{
				double best_dist = 0.;
				const Ship *best = NULL;
				std::vector<std::string> objectives = Str::SplitToVector( mission_objs, "," );
				
				for( std::vector<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					if( (*ship_iter)->ID != my_ship_id )
					{
						const Ship *ship = *ship_iter;
						
						if( ship->Health <= 0. )
							continue;
						if( (ship->JumpProgress < 1.) || ship->JumpedOut )
							continue;
						
						if( mission_objs.empty() || (mission_objs == "default") )
						{
							if( my_team && (ship->Team == my_team) && (ship->Category() != ShipClass::CATEGORY_TARGET) && (GameType != XWing::GameType::CAPITAL_SHIP_HUNT) )
								continue;
							if( (ship->Group != 255) && ! ship->IsMissionObjective )
								continue;
							if( best && best->IsMissionObjective && ! ship->IsMissionObjective )
								continue;
						}
						else
						{
							// If the mission has defined custom objectives, match only those.
							bool match = false;
							for( size_t i = 0; i < objectives.size(); i ++ )
							{
								std::string objective = objectives[ i ];
								if( Str::BeginsWith( ship->Name, objective ) )
								{
									match = true;
									break;
								}
								
								if( my_team )
								{
									if( Str::BeginsWith( objective, "friendly " ) )
									{
										if( ship->Team != my_team )
											continue;
										objective = objective.substr( strlen("friendly ") );
									}
									else if( ship->Team == my_team )
										continue;
								}
								
								if( ((objective == "capital"   ) && (ship->Category() == ShipClass::CATEGORY_CAPITAL  ))
								||  ((objective == "fighters"  ) && (ship->Category() == ShipClass::CATEGORY_FIGHTER  ))
								||  ((objective == "gunboats"  ) && (ship->Category() == ShipClass::CATEGORY_GUNBOAT  ))
								||  ((objective == "bombers"   ) && (ship->Category() == ShipClass::CATEGORY_BOMBER   ))
								||  ((objective == "transports") && (ship->Category() == ShipClass::CATEGORY_TRANSPORT))
								||  ((objective == "target"    ) && (ship->Category() == ShipClass::CATEGORY_TARGET   ))
								||  (ship->Class && Str::BeginsWith( ship->Class->ShortName, objective )) )
								{
									match = true;
									break;
								}
							}
							if( ! match )
								continue;
						}
						
						double dist = ship->Dist( my_pos );
						
						if( (dist < best_dist) || ! best )
						{
							best = ship;
							best_dist = dist;
						}
					}
				}
				
				id = best ? best->ID : 0;
			}
			
			if( ! id )
				beep = "beep_error.wav";
			else if( target_id != id )
			{
				target_id = id;
				target_subsystem = 0;
				beep = "beep.wav";
			}
		}
		else if( target_dockable )
		{
			double best_dist = 0.;
			const Ship *best = NULL;
			
			for( std::vector<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				if( (*ship_iter)->ID != my_ship_id )
				{
					const Ship *ship = *ship_iter;
					
					if( ship->Health <= 0. )
						continue;
					if( (ship->JumpProgress < 1.) || ship->JumpedOut )
						continue;
					if( ! ship->HasDockingBays() )
						continue;
					if( my_team && (ship->Team != my_team) )
						continue;
					
					double dist = ship->Dist( my_pos );
					
					if( (dist < best_dist) || ! best )
					{
						best = ship;
						best_dist = dist;
					}
				}
			}
			
			if( ! best )
				beep = "beep_error.wav";
			else if( target_id != best->ID )
			{
				target_id = best->ID;
				target_subsystem = 0;
				beep = "beep.wav";
			}
		}
		else if( target_groupmate )
		{
			double best_dist = 0.;
			bool best_player = false;
			uint32_t id = 0;
			const Ship *ship = my_turret ? my_turret->ParentShip() : my_ship;
			if( ship && my_team )
			{
				const Player *my_player = Data.GetPlayer( PlayerID );
				uint8_t my_group = my_player ? my_player->PropertyAsInt( "group", ship->Group ) : ship->Group;
				uint8_t my_skin = my_group;
				if( ship->Class && ! my_skin )
					my_skin = ship->Class->DefaultSkinGroup();  // X-Wings are Red, B-Wing are Blue, etc...
				
				for( std::vector<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					if( (*ship_iter == ship) || ((*ship_iter)->Team != my_team) || ((*ship_iter)->Health <= 0.) )
						continue;
					
					// Prefer player groupmates.
					if( best_player && ! (*ship_iter)->PlayerID )
						continue;
					
					uint8_t group = (*ship_iter)->Group;
					if( (group >= 100) && (group < 200) && ! (*ship_iter)->PlayerID )  // Ignore automatic AI respawn groups.
						group = 0;
					uint8_t skin = group;
					if( (*ship_iter)->Class && ! skin )
						skin = (*ship_iter)->Class->DefaultSkinGroup();
					
					// Must match same skin/group.
					if( skin != my_skin )
						continue;
					
					// If no skin (TIEs, YT-1300, etc) only match AI ships that are the same class.
					if( (ship->Class != (*ship_iter)->Class) && ! (skin || (*ship_iter)->PlayerID) )
						continue;
					
					// Finally prefer nearer ships.
					double dist = my_pos->Dist(*ship_iter);
					if( (dist < best_dist) || ((*ship_iter)->PlayerID && ! best_player) || ! id )
					{
						id = (*ship_iter)->ID;
						best_dist = dist;
						best_player = (*ship_iter)->PlayerID;
					}
				}
			}
			
			if( ! id )
				beep = "beep_error.wav";
			else if( target_id != id )
			{
				target_id = id;
				target_subsystem = 0;
				beep = "beep.wav";
			}
		}
		else if( target_sync )
		{
			uint32_t id = 0;
			const Ship *ship = my_ship;
			
			if( my_turret )
			{
				ship = my_turret->ParentShip();
				if( ship && (ship->PlayerID != PlayerID) && ship->Target )
					id = ship->Target;
			}
			
			if( ship && my_team && ! id )
			{
				std::list<Turret*> turrets = ship->AttachedTurrets();
				
				for( std::list<Turret*>::const_iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
				{
					if( ((*turret_iter)->PlayerID != PlayerID) && ((*turret_iter)->PlayerID || ! (*turret_iter)->ParentControl) && (*turret_iter)->Target )
					{
						// Only receive target data about alive ships that are not our own.
						const Ship *potential_target = (const Ship*) Data.GetObject( (*turret_iter)->Target );
						if( (! potential_target) || (potential_target->Type() != XWing::Object::SHIP) || (potential_target->Health <= 0.f) || (potential_target == ship) )
							continue;
						
						id = (*turret_iter)->Target;
						if( (*turret_iter)->PlayerID )
							break;  // Prefer player-selected targets.
					}
				}
				
				if( (! id) && (! my_turret) )
				{
					double best_dist = 0.;
					bool best_player = false;
					uint8_t my_group = ship ? ship->Group : 0;
					Player *my_player = Data.GetPlayer( PlayerID );
					if( my_player )
						my_group = my_player->PropertyAsInt("group",my_group);
					
					for( std::vector<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
					{
						if( (*ship_iter != ship) && ((*ship_iter)->Team == my_team) && ((*ship_iter)->Group == my_group) && ((*ship_iter)->Health > 0.) && (*ship_iter)->Target )
						{
							// Only receive target data about alive ships that are not our own.
							const Ship *potential_target = (const Ship*) Data.GetObject( (*ship_iter)->Target );
							if( (! potential_target) || (potential_target->Type() != XWing::Object::SHIP) || (potential_target->Health <= 0.f) || (potential_target == ship) )
								continue;
							
							// When not in a flight group, target other players on your team not in a flight group, or friendly AI ships of same category.
							if( (! my_group) && ship && (ship->Category() != (*ship_iter)->Category()) && ! (*ship_iter)->PlayerID )
								continue;
							
							// Primarily prefer targets from players.
							if( best_player && ! (*ship_iter)->PlayerID )
								continue;
							
							// Secondarily prefer targets from nearer ships.
							double dist = my_pos->Dist(*ship_iter);
							if( (dist < best_dist) || ((*ship_iter)->PlayerID && ! best_player) || ! id )
							{
								id = (*ship_iter)->Target;
								best_dist = dist;
								best_player = (*ship_iter)->PlayerID;
							}
						}
					}
				}
			}
			
			if( ! id )
				beep = "beep_error.wav";
			else if( target_id != id )
			{
				target_id = id;
				target_subsystem = 0;
				beep = "beep.wav";
			}
		}
		else if( target_newest )
		{
			double best = 0.;
			uint32_t id = 0;
			
			for( std::vector<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				if( (*ship_iter)->ID != my_ship_id )
				{
					const Ship *ship = *ship_iter;
					
					if( ship->Health <= 0. )
						continue;
					if( (ship->JumpProgress < 1.) || ship->JumpedOut )
						continue;
					
					// FIXME: Should this only target enemies?  It currently does not check team.
					
					double lifetime = ship->Lifetime.ElapsedSeconds();
					if( (lifetime < best) || ! id )
					{
						best = lifetime;
						id = ship->ID;
					}
				}
			}
			
			if( target_id != id )
			{
				target_id = id;
				target_subsystem = 0;
				beep = "beep.wav";
			}
		}
		
		// Unselect dead target before it can respawn.
		if( target_id )
		{
			GameObject *target_obj = Data.GetObject( target_id );
			if( target_obj )
			{
				if( target_obj->Type() == XWing::Object::SHIP )
				{
					Ship *target_ship = (Ship*) target_obj;
					if( ((target_ship->Health < 0.) && (target_ship->DeathClock.ElapsedSeconds() > 4.)) || (target_ship->JumpProgress < 1.) || target_ship->JumpedOut )
						target_id = 0;
				}
			}
			else
				target_id = 0;
			if( ! target_id )
				target_subsystem = 0;
		}
	}
	
	if( my_turret )
		my_turret->Target = target_id;
	
	if( my_ship )
	{
		const GameObject *target = target_id ? Data.GetObject(target_id) : NULL;
		
		// If we had a subsystem targeted that was destroyed, untarget it.
		if( target && target_subsystem && (target->Type() == XWing::Object::SHIP) )
		{
			const Ship *target_ship = (const Ship*) target;
			std::map<std::string,double>::const_iterator subsystem_iter = target_ship->Subsystems.find( target_ship->SubsystemName( target_subsystem ) );
			if( (subsystem_iter != target_ship->Subsystems.end()) && (subsystem_iter->second <= 0.) )  // Can't lock onto destroyed subsystems.
				target_subsystem = 0;
		}
		
		// Set target ID and update missile lock progress.
		int old_lock = my_ship->TargetLock * 9;
		my_ship->UpdateTarget( target, target_subsystem, dt );
		int new_lock = my_ship->TargetLock * 9;
		if( new_lock > old_lock )
		{
			if( new_lock == 9 )
				Snd.Play( Res.GetSound("locked.wav") );
			else if( new_lock < 9 )
				Snd.Play( Res.GetSound("locking.wav") );
		}
	}
	
	if( ! ejecting )
	{
		EjectHeld.Reset();
		play_eject_sound = true;
	}
	
	// If we did anything that calls for a beep, play the sound now.
	if( beep )
	{
		bool error_beep = (strcmp( beep, "beep.wav" ) != 0);
		if( error_beep_allowed || ! error_beep )
		{
			Snd.Play( Res.GetSound(beep) );
			error_beep_allowed = ! (error_beep || target_sync); // This will become true again when we are no longer holding down targeting buttons.
		}
	}
	
	
	// Now update all objects.
	RaptorGame::Update( dt );
	
	
	// ZeroLag shots.
	for( std::vector<Shot*>::iterator shot_iter = fire_shots.begin(); shot_iter != fire_shots.end(); shot_iter ++ )
	{
		(*shot_iter)->Predicted = true;
		(*shot_iter)->Lifetime.TimeScale = Data.TimeScale;
		Data.AddObject( *shot_iter );
		ClientShots[ (*shot_iter)->ShotType ][ (*shot_iter)->WeaponIndex ].push_back( *shot_iter );
	}
	
	// Clean up any lasers that never synchronized.
	for( std::map< uint8_t, std::map< int, std::deque<Shot*> > >::iterator shot_type_iter = ClientShots.begin(); shot_type_iter != ClientShots.end(); shot_type_iter ++ )
	{
		for( std::map< int, std::deque<Shot*> >::iterator shot_weapon_iter = shot_type_iter->second.begin(); shot_weapon_iter != shot_type_iter->second.end(); shot_weapon_iter ++ )
		{
			while( shot_weapon_iter->second.size() && (shot_weapon_iter->second.front()->Lifetime.ElapsedSeconds() >= shot_weapon_iter->second.front()->MaxLifetime()) )
			{
				if( Cfg.SettingAsBool("debug") )
					Console.Print( std::string("DEBUG: ZeroLag Shot ") + Num::ToString( (int) shot_weapon_iter->second.front()->ID ) + std::string(" (WeaponIndex ") + Num::ToString( (int) shot_weapon_iter->second.front()->WeaponIndex ) + std::string(") never synchronized."), TextConsole::MSG_ERROR );
				Data.RemoveObject( shot_weapon_iter->second.front()->ID );
				shot_weapon_iter->second.pop_front();
			}
		}
	}
	
	
	if( State >= XWing::State::FLYING )
	{
		// Add any flyby sounds.
		
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
		{
			// Don't add flyby sounds for the ship we're watching.
			if( observed_ship && (obj_iter->first == observed_ship->ID) )
				continue;
			
			// Don't add multiple sounds for the same object flying by.
			if( Snd.ObjectPans.find( obj_iter->first ) != Snd.ObjectPans.end() )
				continue;
			
			// Don't play sounds for the same object too frequently.
			std::map<uint32_t,Clock>::const_iterator recent_pan = Snd.RecentPans.find( obj_iter->first );
			if( (recent_pan != Snd.RecentPans.end()) && (recent_pan->second.ElapsedSeconds() < Rand::Double(4.,5.)) )
				continue;
			
			// Calculate the object's motion relative to the camera's.
			Vec3D relative_motion = obj_iter->second->MotionVector;
			if( observed_ship )
				relative_motion -= observed_ship->MotionVector;
			double speed = relative_motion.Length();
			
			// Check for ship flybys.
			if( (obj_iter->second->Type() == XWing::Object::SHIP) && (Cam.Dist( obj_iter->second ) < std::min<double>( 150., 50. + speed )) )
			{
				Ship *ship = (Ship*) obj_iter->second;
				
				// Dead ships tell no tales.
				if( ship->Health <= 0. )
					continue;
				
				// Don't add flyby sounds for ships that aren't here.
				if( ship->JumpedOut )
					continue;
				
				// Make sure this ship has any flyby sounds.
				if( !( ship->Class && ship->Class->FlybySounds.size() ) )
					continue;
				
				// Relative motion isn't everything; the ship making the flyby sound must be moving as well!
				if( ship->MotionVector.Length() < ship->Class->FlybySounds.begin()->first )
					continue;
				
				// Different sounds for different speeds and ships.
				const char *sound = ship->FlybySound( speed );
				if( sound )
					Snd.PlayFromObject( Res.GetSound(sound), obj_iter->first, 7. );
			}
		}
	}
	
	
	if( State == XWing::State::FLYING )
	{
		if( Cfg.SettingAsBool("s_imuse") )
		{
			// Dynamic music enabled: Update music according to the action.
			
			if( Snd.MusicSubdir.compare( 0, 6, "iMUSE/" ) != 0 )
				Snd.StopMusic();
			bool found_music = false;
			
			if( observed_ship && (observed_ship->Health <= 0.) )
			{
				Snd.PlayMusicSubdirNext( "iMUSE/Death" );
				found_music = true;
			}
			
			if( (! found_music) && observed_ship && (GameType == XWing::GameType::BATTLE_OF_YAVIN) )
			{
				// Find the important elements of the Battle of Yavin scenario.
				
				DeathStar *deathstar = NULL;
				Ship *exhaust_port = NULL;
				
				for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
				{
					if( obj_iter->second->Type() == XWing::Object::SHIP )
					{
						Ship *this_ship = (Ship*) obj_iter->second;
						if( this_ship->Category() == ShipClass::CATEGORY_TARGET )
							exhaust_port = this_ship;
					}
					else if( obj_iter->second->Type() == XWing::Object::DEATH_STAR )
						deathstar = (DeathStar*) obj_iter->second;
					
					if( deathstar && exhaust_port )
						break;
				}
				
				// Change music for trench running.
				
				if( deathstar && (observed_ship->DistAlong( &(deathstar->Up), deathstar ) < 0.) )
				{
					if( observed_ship->Dist(exhaust_port) <= 5000. )
						Snd.PlayMusicSubdirNext( "iMUSE/TrenchNear" );
					else
						Snd.PlayMusicSubdirNext( "iMUSE/Trench" );
					
					found_music = true;
				}
			}
			
			if( ! found_music )
				Snd.PlayMusicSubdirNext( "iMUSE/Combat" );
		}
		else
		{
			// Dynamic music disabled: Play full-length songs.
			
			if( Snd.MusicSubdir.compare( 0, 6, "iMUSE/" ) == 0 )
			{
				Snd.StopMusic();
				Snd.PlayMusicSubdir( "Flight" );
			}
		}
	}
	
	
	AsteroidLOD = Raptor::Game->Cfg.SettingAsDouble("g_asteroid_lod",1.);
	
	
	if( Achievements.size() && (AchievementClock.Progress() >= 1.) )
	{
		Mix_Chunk *sound = Res.GetSound( Achievements.front() );
		int channel = Snd.Play( sound );
		if( channel >= 0 )
		{
			Snd.AttenuateFor = channel;
			Snd.MusicAttenuate = 0.75;
			Snd.SoundAttenuate = 0.5;
		}
		Achievements.pop();
		AchievementClock.Reset( 3. );
	}
}


bool XWingGame::HandleEvent( SDL_Event *event )
{
	if( State >= XWing::State::FLYING )
	{
		uint8_t control = Input.EventBound( event );
		
		if( ControlPressed( control ) )
			return true;
	}
	
	return RaptorGame::HandleEvent( event );
}


bool XWingGame::ControlPressed( uint8_t control )
{
	bool weapon_next = false;
	bool firing_mode_next = false;
	bool shield_shunt = false;
	bool target_next = false;
	bool target_prev = false;
	bool target_next_enemy = false;
	bool target_prev_enemy = false;
	bool target_next_friendly = false;
	bool target_prev_friendly = false;
	bool target_next_player = false;
	bool target_prev_player = false;
	bool target_next_subsystem = false;
	bool target_prev_subsystem = false;
	uint32_t *target_stored = NULL;
	uint8_t *subsystem_stored = NULL;
	bool observe_next = false;
	bool observe_prev = false;
	uint8_t change_seat = 0;
	bool chewie_take_the_wheel = false;
	bool change_view = false;
	bool pause = false;
	
	// See if we can handle this.
	if( control ==  XWing::Control::THROTTLE_33  )       // SDLK_LEFTBRACKET
		observe_prev = true;
	else if( control ==  XWing::Control::THROTTLE_66  )  // SDLK_RIGHTBRACKET
		observe_next = true;
	else if( control ==  XWing::Control::WEAPON  )
		weapon_next = true;
	else if( control ==  XWing::Control::MODE  )
		firing_mode_next = true;
	else if( control ==  XWing::Control::SHIELD_DIR  )
		shield_shunt = true;
	else if( control ==  XWing::Control::TARGET_PREV  )
		target_prev = true;
	else if( control ==  XWing::Control::TARGET_NEXT  )
		target_next = true;
	else if( control ==  XWing::Control::TARGET_PREV_ENEMY  )
	{
		target_prev_enemy = true;
		if( Cfg.SettingAsString("spectator_view") != "instruments" )
			observe_prev = true;
	}
	else if( control ==  XWing::Control::TARGET_NEXT_ENEMY  )
	{
		target_next_enemy = true;
		if( Cfg.SettingAsString("spectator_view") != "instruments" )
			observe_next = true;
	}
	else if( control ==  XWing::Control::TARGET_PREV_FRIENDLY  )
		target_prev_friendly = true;
	else if( control ==  XWing::Control::TARGET_NEXT_FRIENDLY  )
		target_next_friendly = true;
	else if( control ==  XWing::Control::TARGET_PREV_PLAYER  )
		target_prev_player = true;
	else if( control ==  XWing::Control::TARGET_NEXT_PLAYER  )
		target_next_player = true;
	else if( control ==  XWing::Control::TARGET_PREV_SUBSYSTEM  )
	{
		target_prev_subsystem = true;
		observe_prev = true;
	}
	else if( control ==  XWing::Control::TARGET_NEXT_SUBSYSTEM  )
	{
		target_next_subsystem = true;
		observe_next = true;
	}
	else if( control ==  XWing::Control::TARGET1  )
	{
		target_stored = &(StoredTargets[ 0 ]);
		subsystem_stored = &(StoredSubsystems[ 0 ]);
	}
	else if( control ==  XWing::Control::TARGET2  )
	{
		target_stored = &(StoredTargets[ 1 ]);
		subsystem_stored = &(StoredSubsystems[ 1 ]);
	}
	else if( control ==  XWing::Control::TARGET3  )
	{
		target_stored = &(StoredTargets[ 2 ]);
		subsystem_stored = &(StoredSubsystems[ 2 ]);
	}
	else if( control ==  XWing::Control::TARGET4  )
	{
		target_stored = &(StoredTargets[ 3 ]);
		subsystem_stored = &(StoredSubsystems[ 3 ]);
	}
	else if( control ==  XWing::Control::SEAT_COCKPIT  )
		change_seat = 1;
	else if( control ==  XWing::Control::SEAT_GUNNER1  )
		change_seat = 2;
	else if( control ==  XWing::Control::SEAT_GUNNER2  )
		change_seat = 3;
	else if( control ==  XWing::Control::CHEWIE_TAKE_THE_WHEEL  )
		chewie_take_the_wheel = true;
	else if( (control >=  XWing::Control::VIEW_COCKPIT ) && (control <=  XWing::Control::VIEW_INSTRUMENTS ) )
		change_view = true;
	else if( control ==  XWing::Control::PAUSE  )
		pause = true;
	else
		return false;
	
	
	if( change_seat )
	{
		Packet packet = Packet( XWing::Packet::CHANGE_SEAT );
		packet.AddUChar( change_seat - 1 );
		if( change_seat >= 2 )
			packet.AddUChar( TurretFiringMode );
		Net.Send( &packet );
		
		// The server determines if we can change seats, so no need to find our ship below.
		return true;
	}
	
	if( chewie_take_the_wheel )
	{
		Packet packet = Packet( XWing::Packet::TOGGLE_COPILOT );
		Net.Send( &packet );
		
		// The server handles copilot toggle, so no need to find our ship below.
		return true;
	}
	
	if( pause )
	{
		if( Data.TimeScale < 0.0000011 )
		{
			Packet info = Packet( Raptor::Packet::INFO );
			info.AddUShort( 1 );
			info.AddString( "time_scale" );
			info.AddString( "1" );
			Raptor::Game->Net.Send( &info );
			Data.TimeScale = 1.;  // Resume immediately.  This is necessary for ui_pause hand-off between menus.
		}
		else if( Raptor::Server->IsRunning() || Data.PropertyAsBool("allow_pause") )
		{
			Packet info = Packet( Raptor::Packet::INFO );
			info.AddUShort( 1 );
			info.AddString( "time_scale" );
			info.AddString( "0.000001" );
			Raptor::Game->Net.Send( &info );
			Data.TimeScale = 0.000001;  // Pause immediately.  This is necessary for ui_pause hand-off between menus.
		}
		return true;
	}
	
	
	// Build a list of all ships because we'll refer to it often, and find the player's ship and/or turret.
	std::vector<Ship*> ships;
	std::vector<Shot*> missiles;
	Ship *my_ship = NULL;
	Turret *my_turret = NULL;
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->Type() == XWing::Object::SHIP )
		{
			Ship *ship = (Ship*) obj_iter->second;
			ships.push_back( ship );
			
			if( ship->PlayerID == PlayerID )
				my_ship = ship;
		}
		else if( obj_iter->second->Type() == XWing::Object::TURRET )
		{
			Turret *turret = (Turret*) obj_iter->second;
			if( turret->PlayerID == PlayerID )
				my_turret = turret;
		}
		else if( obj_iter->second->Type() == XWing::Object::SHOT )
		{
			Shot *shot = (Shot*) obj_iter->second;
			if( shot->CollisionType() == XWing::Object::SHOT_MISSILE )
				missiles.push_back( shot );
		}
	}
	
	
	if( change_view )
	{
		bool alive = (my_ship && (ObservedShipID == my_ship->ID)) || my_turret;
		bool screensaver = Cfg.SettingAsBool( "screensaver" );
		std::string var = (alive || screensaver || (Cfg.SettingAsString("view") != "auto")) ? "view" : "spectator_view";
		
		std::string value = "auto";
		if( control ==  XWing::Control::VIEW_COCKPIT )
			value = alive ? "auto" : "cockpit";
		else if( control ==  XWing::Control::VIEW_CROSSHAIR )
			value = "crosshair";
		else if( control ==  XWing::Control::VIEW_CHASE )
			value = "chase";
		else if( control ==  XWing::Control::VIEW_PADLOCK )
			value = "padlock";
		else if( control ==  XWing::Control::VIEW_STATIONARY )
			value = "stationary";
		else if( control ==  XWing::Control::VIEW_CINEMA )
			value = "cinema";
		else if( control ==  XWing::Control::VIEW_FIXED )
			value = "fixed";
		else if( control ==  XWing::Control::VIEW_SELFIE )
			value = "selfie";
		else if( control ==  XWing::Control::VIEW_GUNNER )
			value = "gunner";
		else if( control ==  XWing::Control::VIEW_CYCLE )
			value = "cycle";
		else if( control ==  XWing::Control::VIEW_INSTRUMENTS )
			value = "instruments";
		
		if( value == Cfg.SettingAsString(var) )  // View buttons toggle.
			value = "auto";
		
		Cfg.Settings[ var ] = value;
		if( var != "view" )
			Cfg.Settings[ "view" ] = "auto";
		else if( screensaver )
			Cfg.Settings[ "spectator_view" ] = "auto";
		
		if( value == "stationary" )
			LookYaw = LookPitch = 0.;  // Don't reapply view offset after dropping the camera.
		
		return true;
	}
	
	
	// Apply controls to player's ship or turret.
	
	uint32_t target_id = 0;
	uint8_t target_subsystem = 0;
	const char *beep = NULL;
	const Ship *my_turret_parent = NULL;
	
	if( my_turret )
	{
		target_id = my_turret->Target;
		my_turret_parent = my_turret->ParentShip();
		
		if( firing_mode_next && my_turret->GunWidth )
		{
			my_turret->FiringMode = (my_turret->FiringMode == 1) ? 2 : 1;
			Snd.Play( (my_turret->FiringMode == 1) ? Res.GetSound("beep_laser1.wav") : Res.GetSound("beep_laser2.wav") );
			TurretFiringMode = my_turret->FiringMode;
		}
	}
	else if( my_ship && (my_ship->Health > 0.) )
	{
		target_id = my_ship->Target;
		target_subsystem = my_ship->TargetSubsystem;
		
		if( weapon_next || firing_mode_next )
		{
			bool changed = weapon_next ? my_ship->NextWeapon() : my_ship->NextFiringMode();
			if( changed )
			{
				// Changed weapon or mode; play a beep for the new mode.
				
				bool heavy_weapon = (my_ship->SelectedWeapon == Shot::TYPE_MISSILE) || (my_ship->SelectedWeapon == Shot::TYPE_TORPEDO);
				bool ion = (my_ship->SelectedWeapon == Shot::TYPE_ION_CANNON);
				if( my_ship->CurrentFiringMode() <= 1 )
					Snd.Play( heavy_weapon ? Res.GetSound("beep_torpedo1.wav") : (ion ? Res.GetSound("beep_ion1.wav") : Res.GetSound("beep_laser1.wav")) );
				else if( my_ship->CurrentFiringMode() == 2 )
					Snd.Play( heavy_weapon ? Res.GetSound("beep_torpedo2.wav") : (ion ? Res.GetSound("beep_ion2.wav") : Res.GetSound("beep_laser2.wav")) );
				else
					Snd.Play( heavy_weapon ? Res.GetSound("beep_torpedo2.wav") : (ion ? Res.GetSound("beep_ion3.wav") : Res.GetSound("beep_laser3.wav")) );
			}
			else if( firing_mode_next )
			{
				// See if there are linked turrets that can change firing mode.
				
				uint8_t new_mode = 0;
				std::set<Turret*> updated_turrets;
				std::list<Turret*> turrets = my_ship->AttachedTurrets();
				for( std::list<Turret*>::iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
				{
					if( (*turret_iter)->PlayerID || ! (*turret_iter)->ParentControl )
						continue;
					if( ! (*turret_iter)->GunWidth )
						continue;
					
					if( ! new_mode )
						new_mode = ((*turret_iter)->FiringMode == 1) ? 2 : 1;
					
					(*turret_iter)->FiringMode = new_mode;
					updated_turrets.insert( *turret_iter );
				}
				
				if( new_mode )
				{
					Snd.Play( (new_mode > 1) ? Res.GetSound("beep_laser2.wav") : Res.GetSound("beep_laser1.wav") );
					changed = true;
					
					// Turret firing is controlled by the server, so we need to send our new mode.
					int8_t precision = -127;
					Packet update_packet( Raptor::Packet::UPDATE );
					update_packet.AddChar( precision );
					update_packet.AddUInt( updated_turrets.size() );
					for( std::set<Turret*>::const_iterator update_iter = updated_turrets.begin(); update_iter != updated_turrets.end(); update_iter ++ )
					{
						update_packet.AddUInt( (*update_iter)->ID );
						(*update_iter)->AddToUpdatePacketFromClient( &update_packet, precision );
					}
					Net.Send( &update_packet );
				}
			}
			
			if( ! changed )
				Snd.Play( Res.GetSound("beep_error.wav") );
		}
		else if( shield_shunt )
		{
			// Angle the deflector shields; play a beep for the new direction.
			
			if( my_ship->MaxShield() )
			{
				if( my_ship->ShieldPos == Ship::SHIELD_CENTER )
				{
					my_ship->SetShieldPos( Ship::SHIELD_REAR );
					Snd.Play( Res.GetSound("beep_sr.wav") );
				}
				else if( my_ship->ShieldPos == Ship::SHIELD_REAR )
				{
					my_ship->SetShieldPos( Ship::SHIELD_FRONT );
					Snd.Play( Res.GetSound("beep_sf.wav") );
				}
				else
				{
					my_ship->SetShieldPos( Ship::SHIELD_CENTER );
					Snd.Play( Res.GetSound("beep_sc.wav") );
				}
			}
			else
				Snd.Play( Res.GetSound("beep_error.wav") );
		}
	}
	else if( observe_next || observe_prev )
	{
		// Player has no ship nor turret, and we're cycling through ships to observe.
		
		Ship *observed_ship = NULL;
		Ship *prev_ship = NULL;
		bool found = false, find_last = false;
		
		for( std::vector<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
		{
			// Don't spectate the Death Star exhaust port.
			if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
				continue;
			
			// Don't observe long-dead ships.
			if( ((*ship_iter)->Health <= 0.) && ((*ship_iter)->DeathClock.ElapsedSeconds() >= 6.) )
				continue;
			
			// When spectating from gunner view, skip any ships without turrets.
			if( (Cfg.SettingAsString("spectator_view") == "gunner") && ( ! (*ship_iter)->AttachedTurret() ) )
				continue;
			
			prev_ship = observed_ship;
			observed_ship = *ship_iter;
			
			// If we'd selected a specific ship to watch, keep going until we find it.
			if( (observed_ship->ID >= ObservedShipID) && (! find_last) )
			{
				if( observe_next )
					observe_next = false;
				else if( observe_prev )
				{
					if( prev_ship )
					{
						observed_ship = prev_ship;
						found = true;
						break;
					}
					else
						find_last = true;
				}
				else
				{
					found = true;
					break;
				}
			}
		}
		
		if( (! found) && (! observe_prev) )
		{
			for( std::vector<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
					continue;
				
				observed_ship = *ship_iter;
				break;
			}
		}
		
		if( observed_ship )
			ObservedShipID = observed_ship->ID;
	}
	
	
	// Apply targeting.
	
	if( target_next || target_prev || target_next_enemy || target_prev_enemy || target_next_friendly || target_prev_friendly || target_next_player || target_prev_player )
	{
		uint32_t my_ship_id = my_turret_parent ? my_turret_parent->ID : (my_ship ? my_ship->ID : 0);
		uint8_t my_team = my_turret ? my_turret->Team : (my_ship ? my_ship->Team : 0);
		
		std::map<uint32_t,const GameObject*> potential_targets;
		for( std::vector<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
		{
			if( (*ship_iter)->ID != my_ship_id )
			{
				const Ship *ship = *ship_iter;
				
				if( ship->Health <= 0. )
					continue;
				if( (ship->JumpProgress < 1.) || ship->JumpedOut )
					continue;
				if( (target_next_enemy || target_prev_enemy) && my_team && (ship->Team == my_team) )
					continue;
				if( (target_next_friendly || target_prev_friendly) && ((! my_team) || (ship->Team != my_team)) )
					continue;
				if( (target_next_player || target_prev_player) && ! ship->Owner() )
					continue;
				
				potential_targets[ ship->ID ] = ship;
			}
		}
		
		if( target_next || target_prev )
		{
			for( std::vector<Shot*>::const_iterator shot_iter = missiles.begin(); shot_iter != missiles.end(); shot_iter ++ )
				potential_targets[ (*shot_iter)->ID ] = *shot_iter;
		}
		
		std::map<uint32_t,const GameObject*>::iterator target_iter = potential_targets.find( target_id );
		if( target_prev || target_prev_enemy || target_prev_friendly || target_prev_player ) // Prev
		{
			if( target_iter == potential_targets.begin() )
				target_iter = potential_targets.end();
			else if( potential_targets.size() )
				target_iter --;
		}
		else // Next
		{
			if( target_iter == potential_targets.end() )
				target_iter = potential_targets.begin();
			else
				target_iter ++;
		}
		
		uint32_t id = (target_iter != potential_targets.end()) ? target_iter->first : 0;
		if( !( id || target_id ) || ! potential_targets.size() )
			Snd.Play( Res.GetSound("beep_error.wav") );
		else if( id != target_id )
		{
			target_id = id;
			target_subsystem = 0;
			beep = "beep.wav";
		}
	}
	else if( target_stored )
	{
		if( Input.ControlPressed(  XWing::Control::TARGET_STORE  ) )
		{
			*target_stored = target_id;
			if( subsystem_stored )
				*subsystem_stored = target_subsystem;
			beep = "beep_store.wav";
		}
		else if( *target_stored || target_id )
		{
			const Ship *target_ship = (const Ship*) Data.GetObject( *target_stored );
			if( target_ship && (target_ship->Type() == XWing::Object::SHIP) && (target_ship->Health <= 0.) )
				target_ship = NULL;
			
			if( ! target_ship )
				beep = "beep_error.wav";
			else if( (target_id != *target_stored) || (subsystem_stored && (target_subsystem != *subsystem_stored)) )
			{
				target_id = *target_stored;
				if( subsystem_stored )
					target_subsystem = *subsystem_stored;
				beep = "beep.wav";
			}
		}
	}
	else if( my_ship && (target_next_subsystem || target_prev_subsystem) )
	{
		const Ship *target = (const Ship*) Data.GetObject(target_id);
		if( target && (target->Type() == XWing::Object::SHIP) )
		{
			std::set<uint8_t> shield_generators, other_subsystems;
			uint8_t subsystem_num = 1;
			for( std::map<std::string,double>::const_iterator subsystem_iter = target->Subsystems.begin(); subsystem_iter != target->Subsystems.end(); subsystem_iter ++ )
			{
				if( subsystem_iter->second > 0. )
				{
					// Shield Generators first, then everything else.
					if( strncmp( subsystem_iter->first.c_str(), "ShieldGen", strlen("ShieldGen") ) == 0 )
						shield_generators.insert( subsystem_num );
					else
						other_subsystems.insert( subsystem_num );
				}
				subsystem_num ++;
			}
			
			std::vector<uint8_t> valid_subsystems;
			for( std::set<uint8_t>::const_iterator subsystem_num_iter = shield_generators.begin(); subsystem_num_iter != shield_generators.end(); subsystem_num_iter ++ )
				valid_subsystems.push_back( *subsystem_num_iter );
			for( std::set<uint8_t>::const_iterator subsystem_num_iter = other_subsystems.begin(); subsystem_num_iter != other_subsystems.end(); subsystem_num_iter ++ )
				valid_subsystems.push_back( *subsystem_num_iter );
			
			std::vector<uint8_t>::iterator valid_iter = std::find( valid_subsystems.begin(), valid_subsystems.end(), my_ship->TargetSubsystem );
			if( valid_iter == valid_subsystems.end() )
				my_ship->TargetSubsystem = 0;
			
			if( valid_subsystems.size() )
			{
				if( target_next_subsystem )
				{
					if( ! my_ship->TargetSubsystem )
						target_subsystem = *(valid_subsystems.begin());
					else if( valid_iter != valid_subsystems.end() )
					{
						valid_iter ++;
						target_subsystem = (valid_iter != valid_subsystems.end()) ? *valid_iter : 0;
					}
					else
						target_subsystem = 0;
				}
				else if( target_prev_subsystem )
				{
					if( ! my_ship->TargetSubsystem )
						target_subsystem = *(valid_subsystems.rbegin());
					else if( target_subsystem == *(valid_subsystems.begin()) )
						target_subsystem = 0;
					else if( valid_iter != valid_subsystems.end() )
					{
						valid_iter --;
						target_subsystem = *valid_iter;
					}
					else
						target_subsystem = 0;
				}
				beep = "beep.wav";
			}
		}
		else
			target_subsystem = 0;
	}
	
	if( my_turret )
		my_turret->Target = target_id;
	
	if( my_ship )
		my_ship->UpdateTarget( target_id ? Data.GetObject(target_id) : NULL, target_subsystem );
	
	if( !( (my_ship && (my_ship->Health > 0.)) || my_turret ) )
	{
		beep = NULL;
		if( target_id )
			ObservedShipID = target_id;
	}
	
	// If we did anything that calls for a beep, play the sound now.
	if( beep )
		Snd.Play( Res.GetSound(beep) );
	
	return true;
}


bool XWingGame::HandleCommand( std::string cmd, std::vector<std::string> *params )
{
	if( cmd == "ship" )
	{
		if( params && params->size() )
			SetPlayerProperty( "ship", params->at(0) );
		else if( State >= Raptor::State::CONNECTED )
		{
			const Player *player = Data.GetPlayer( PlayerID );
			if( player )
			{
				std::string ship = player->PropertyAsString("ship");
				if( ship.length() )
					Console.Print( std::string("ship: ") + ship );
				else
					Console.Print( "ship: Auto-Assign" );
			}
		}
	}
	else if( cmd == "group" )
	{
		if( params && params->size() )
			SetPlayerProperty( "group", params->at(0) );
		else if( State >= Raptor::State::CONNECTED )
		{
			const Player *player = Data.GetPlayer( PlayerID );
			if( player )
			{
				int group = player->PropertyAsInt("group");
				if( group )
					Console.Print( std::string("group: ") + Num::ToString(group) );
				else
					Console.Print( "group: None" );
			}
		}
	}
	else if( cmd == "pause" )
	{
		ControlPressed(  XWing::Control::PAUSE  );
	}
	else if( cmd == "music" )
	{
		if( params && params->size() )
			Snd.PlayMusicSubdir( params->at(0) );
		else if( Mix_PlayingMusic() )
			Console.Print( std::string("Playing music dir: ") + Snd.MusicSubdir );
		else
			Console.Print( "Usage: music <subdir>" );
	}
	else if( cmd == "skip" )
	{
		if( Mix_PlayingMusic() )
			Mix_HaltMusic();
	}
	else if( cmd == "pew" )
	{
		if( Res.SearchPath.front() == "Sounds/Silly" )
		{
			Res.SearchPath.pop_front();
			Console.Print( "Silly sounds disabled." );
		}
		else
		{
			Res.SearchPath.push_front( "Sounds/Silly" );
			Console.Print( "Silly sounds enabled!" );
		}
		
		Snd.StopSounds();
		Res.DeleteSounds();
	}
	else
		return RaptorGame::HandleCommand( cmd, params );
	return true;
}


void XWingGame::MessageReceived( std::string text, uint32_t type )
{
	bool important = false;
	
	if( (type == TextConsole::MSG_CHAT) || (type == TextConsole::MSG_TEAM) )
	{
		Snd.Play( Res.GetSound("chat.wav") );
		important = true;
	}
	else
		important = (Raptor::Game->Head.VR && Raptor::Game->Gfx.DrawTo) || strchr( text.c_str(), '!' ) || strstr( text.c_str(), Cfg.SettingAsString("name","!").c_str() );
	
	OverlayScroll = Cfg.SettingAsDouble("overlay_scroll",0.);
	if( important && (OverlayScroll < 0.1) )
		OverlayScroll = 0.1;
	
	RaptorGame::MessageReceived( text, type );
}


bool XWingGame::ProcessPacket( Packet *packet )
{
	static Clock alarm_time;
	
	packet->Rewind();
	PacketType type = packet->Type();
	
	if( type == XWing::Packet::EXPLOSION )
	{
		double x = packet->NextDouble();
		double y = packet->NextDouble();
		double z = packet->NextDouble();
		double dx = packet->NextFloat();
		double dy = packet->NextFloat();
		double dz = packet->NextFloat();
		float size = packet->NextFloat();
		float loudness = packet->NextFloat();
		
		int subexplosions = 20;
		if( packet->Remaining() )
			subexplosions = packet->NextUChar();
		
		double speed_scale = 1.;
		if( packet->Remaining() )
			speed_scale = packet->NextFloat();
		
		double speed_scale_sub = speed_scale;
		if( packet->Remaining() )
			speed_scale_sub = packet->NextFloat();
		
		double effects_quality = Cfg.SettingAsDouble("g_effects",1.);
		
		if( State >= XWing::State::FLYING )
		{
			Pos3D pos( x, y, z );
			Vec3D motion_vec( dx, dy, dz );
			Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), size, Res.GetSound("explosion.wav"), loudness, &pos, &motion_vec, Rand::Bool() ? 360. : -360., speed_scale ) );
			Snd.PlayPanned( Res.GetSound("explosion_2.wav"), pos.X, pos.Y, pos.Z, powf( loudness, 1.75f ) );
			Snd.PlayPanned( Res.GetSound("explosion_3.wav"), pos.X, pos.Y, pos.Z, powf( loudness, 7.f ) );
			double sqrt_size = sqrt(size);
			subexplosions *= effects_quality;
			for( int i = 0; i < subexplosions; i ++ )
			{
				Vec3D rand( Rand::Double(-1.,1.), Rand::Double(-1.,1.), Rand::Double(-1.,1.) );
				double size_scale = Rand::Double(1.,6.);
				pos.SetPos( x + rand.X * size * 0.01, y + rand.Y * size * 0.01, z + rand.Z * size * 0.01 );
				motion_vec.Set( dx + rand.X * sqrt_size * 20. / size_scale, dy + rand.Y * sqrt_size * 20. / size_scale, dz + rand.Z * sqrt_size * 20. / size_scale );
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), sqrt_size * size_scale, NULL, 0., &pos, &motion_vec, (Rand::Bool() ? 360. : -360.) * Rand::Double(0.9,1.1), Rand::Double(0.8,2.4) * speed_scale_sub ) );
				if( size_scale > 4. )
					Data.Effects.back().Lifetime.CountUpToSecs = Rand::Double(0.,0.3) * Data.Effects.back().Anim.Speed / speed_scale_sub;
			}
		}
		
		return true;
	}
	
	else if( type == XWing::Packet::SHOT_HIT_SHIP )
	{
		uint32_t ship_id = packet->NextUInt();
		double health = packet->NextFloat();
		double shield_f = packet->NextFloat();
		double shield_r = packet->NextFloat();
		uint8_t flags = packet->NextUChar();
		const char *subsystem = packet->NextString();
		double subsystem_health = subsystem[0] ? packet->NextFloat() : 0.;
		uint8_t shot_type = packet->NextUChar();
		double x = packet->NextDouble();
		double y = packet->NextDouble();
		double z = packet->NextDouble();
		double shot_dx = packet->NextFloat();
		double shot_dy = packet->NextFloat();
		double shot_dz = packet->NextFloat();
		uint32_t fired_from = packet->NextUInt();
		
		Ship *ship = NULL;
		double old_health = 0.;
		double old_shield_f = 0., old_shield_r = 0.;
		double old_subsystem_health = 0.;
		double ship_dx = 0., ship_dy = 0., ship_dz = 0.;
		
		GameObject *ship_obj = Data.GetObject( ship_id );
		if( ship_obj && (ship_obj->Type() == XWing::Object::SHIP) )
		{
			ship = (Ship*) ship_obj;
			old_health = ship->Health;
			old_shield_f = ship->ShieldF;
			old_shield_r = ship->ShieldR;
			ship_dx = ship->MotionVector.X;
			ship_dy = ship->MotionVector.Y;
			ship_dz = ship->MotionVector.Z;
			ship->HitClock.Reset();
			ship->HitFlags = flags;
			ship->HitByID = fired_from;
			ship->SetHealth( health );
			ship->ShieldF = shield_f;
			ship->ShieldR = shield_r;
			if( subsystem[0] )
			{
				old_subsystem_health = ship->Subsystems[ subsystem ];
				ship->Subsystems[ subsystem ] = subsystem_health;
			}
		}
		
		if( State >= XWing::State::FLYING )
		{
			Pos3D pos( x, y, z );
			Vec3D motion_vec( ship_dx, ship_dy, ship_dz );
			bool shielded = false, lost_shield = false;
			double bp_radius = 4.;
			double bp_time = 0.;
			static Clock crit_clock;
			
			if( ship )
			{
				Mix_Chunk *sound = NULL;
				double loudness = 1.;
				if( ship->Category() == ShipClass::CATEGORY_TARGET )
				{
					if( ship->Health < old_health )
					{
						sound = Res.GetSound("torpedo_enter.wav");
						loudness = 4.;
					}
				}
				else if( ship->ID == ObservedShipID )
				{
					if( (ship->Health < old_health) || (subsystem_health < old_subsystem_health) )
					{
						sound = Res.GetSound("damage_hull.wav");
						if( ship->PlayerID == Raptor::Game->PlayerID )
						{
							if( ship->Health > 0. )
							{
								Snd.Play( Res.GetSound("damage_hull_2.wav") );
								if( ((ship->Health < ship->MaxHealth() * 0.5) || (subsystem_health < old_subsystem_health)) && (crit_clock.ElapsedSeconds() > 1.) )
								{
									crit_clock.Reset();
									Snd.Play( Res.GetSound("damage_crit.wav") );
								}
							}
							else
								Snd.Play( Res.GetSound("explosion_2.wav") );
						}
					}
					else if( (ship->ShieldF < old_shield_f) || (ship->ShieldR < old_shield_r) )
						sound = Res.GetSound("damage_shield.wav");
					loudness = 2.;
					
					if( (ship->Health > 0.) && ( ((old_shield_f > 0.) && (shield_f <= 0.)) || ((old_shield_r > 0.) && (shield_r <= 0.)) ) )
					{
						bool player_in_ship = (ship->PlayerID == Raptor::Game->PlayerID);
						if( ! player_in_ship )
						{
							std::list<Turret*> turrets = ship->AttachedTurrets();
							for( std::list<Turret*>::const_iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
							{
								if( (*turret_iter)->PlayerID == Raptor::Game->PlayerID )
								{
									player_in_ship = true;
									break;
								}
							}
						}
						if( player_in_ship )
						{
							double min_radius = Cfg.SettingAsDouble( "s_shield_alarm_radius", 10. );
							if( min_radius && (ship->Radius() >= min_radius) && (alarm_time.ElapsedSeconds() >= 1.) )
							{
								Mix_Chunk *alarm = Res.GetSound("shield_alarm.wav");
								Snd.Play( alarm );
								Snd.Play( alarm );
								alarm_time.Reset();
							}
						}
					}
				}
				else
				{
					if( (ship->Health < old_health) || (subsystem_health < old_subsystem_health) )
						sound = Res.GetSound("hit_hull.wav");
					else if( (ship->ShieldF < old_shield_f) || (ship->ShieldR < old_shield_r) )
						sound = Res.GetSound("hit_shield.wav");
				}
				if( sound )
					Snd.PlayAt( sound, x, y, z, loudness );
				
				double shield_damage = (old_shield_f + old_shield_r) - (ship->ShieldF + ship->ShieldR);
				double hull_damage = old_health - ship->Health;
				double shield_scale = ((shot_type == Shot::TYPE_MISSILE) || (shot_type == Shot::TYPE_TORPEDO)) ? 1. : 0.3;
				double knock_scale = (shield_damage * shield_scale + hull_damage) / 20.;
				Vec3D knock( shot_dx - ship_dx, shot_dy - ship_dy, shot_dz - ship_dz );
				if( knock.Length() > 1. )
					knock.ScaleTo( 1. );
				ship->KnockCockpit( &knock, knock_scale );
				
				shielded = (subsystem_health >= old_subsystem_health) && ! hull_damage;
				lost_shield = ((old_shield_f > 0.) && (ship->ShieldF <= 0.)) || ((old_shield_r > 0.) && (ship->ShieldR <= 0.));
			}
			
			if( ship && (ship->Category() == ShipClass::CATEGORY_TARGET) && (shot_type == Shot::TYPE_TORPEDO) )
				bp_radius = 0.;
			else if( shot_type == Shot::TYPE_TORPEDO )
			{
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 10., Res.GetSound("explosion.wav"), (ship->PlayerID == Raptor::Game->PlayerID) ? 10. : 2.5, &pos, &motion_vec, Rand::Bool() ? 360. : -360., 1.5 ) );
				bp_radius = shielded ? 4. : 8.;
				bp_time = 0.125;
			}
			else if( shot_type == Shot::TYPE_MISSILE )
			{
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 17., Res.GetSound("explosion.wav"), (ship->PlayerID == Raptor::Game->PlayerID) ? 10. : 2.5, &pos, &motion_vec, Rand::Bool() ? 450. : -450., 2.5 ) );
				bp_radius = shielded ? 3. : 6.;
				bp_time = 0.0625;
			}
			else if( shot_type == Shot::TYPE_ION_CANNON )
				bp_radius = 3.;
			else if( shot_type == Shot::TYPE_SUPERLASER )
			{
				bp_radius = 25.;
				bp_time = 0.25;
			}
			else
			{
				if( Cfg.SettingAsDouble("g_effects",1.) > 0. )
				{
					motion_vec.ScaleBy( 0.75 );
					Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), lost_shield ? 9. : (shielded ? 3.5 : 4.), NULL, 0., &pos, &motion_vec, 0., 7. ) );
				}
				bp_radius = shielded ? 2. : 4.;
			}
			
			if( BlastPoints && bp_radius )
			{
				if( (! shielded) && (ship->Category() != ShipClass::CATEGORY_CAPITAL) )
					ship->SetBlastPoint( x, y, z, bp_radius, bp_time );
				ship->SetBlastPoint( x + shot_dx * 0.007, y + shot_dy * 0.007, z + shot_dz * 0.007, bp_radius, bp_time );
			}
		}
		
		return true;
	}
	
	else if( type == XWing::Packet::SHOT_HIT_TURRET )
	{
		uint32_t turret_id = packet->NextUInt();
		double health = packet->NextFloat();
		uint8_t shot_type = packet->NextUChar();
		double x = packet->NextDouble();
		double y = packet->NextDouble();
		double z = packet->NextDouble();
		
		Turret *turret = NULL;
		double dx = 0., dy = 0., dz = 0.;
		
		GameObject *turret_obj = Data.GetObject( turret_id );
		if( turret_obj && (turret_obj->Type() == XWing::Object::TURRET) )
		{
			turret = (Turret*) turret_obj;
			dx = turret->MotionVector.X;
			dy = turret->MotionVector.Y;
			dz = turret->MotionVector.Z;
			turret->SetHealth( health );
		}
		
		if( State >= XWing::State::FLYING )
		{
			Pos3D pos( x, y, z );
			Vec3D motion_vec( dx, dy, dz );
			double bp_radius = 4.;
			double bp_time = 0.;
			
			if( turret )
			{
				Mix_Chunk *sound = Res.GetSound("hit_hull.wav");
				double loudness = 1.;
				Snd.PlayAt( sound, x, y, z, loudness );
			}
			
			if( shot_type == Shot::TYPE_TORPEDO )
			{
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 10., Res.GetSound("explosion.wav"), (turret->PlayerID == Raptor::Game->PlayerID) ? 10. : 2.5, &pos, &motion_vec, Rand::Bool() ? 360. : -360., 1.5 ) );
				bp_radius = 16.;
				bp_time = 0.125;
			}
			else if( shot_type == Shot::TYPE_MISSILE )
			{
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 17., Res.GetSound("explosion.wav"), (turret->PlayerID == Raptor::Game->PlayerID) ? 10. : 2.5, &pos, &motion_vec, Rand::Bool() ? 450. : -450., 2.5 ) );
				bp_radius = 12.;
				bp_time = 0.0625;
			}
			else if( shot_type == Shot::TYPE_ION_CANNON )
				bp_radius = 3.;
			else
			{
				if( Cfg.SettingAsDouble("g_effects",1.) > 0. )
				{
					motion_vec.ScaleBy( 0.75 );
					Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 4., NULL, 0., &pos, &motion_vec, 0., 7. ) );
				}
			}
			
			if( BlastPoints )
				turret->SetBlastPoint( x, y, z, bp_radius, bp_time );
		}
		
		return true;
	}
	
	else if( type == XWing::Packet::SHOT_HIT_HAZARD )
	{
		uint8_t shot_type = packet->NextUChar();
		double x = packet->NextDouble();
		double y = packet->NextDouble();
		double z = packet->NextDouble();
		
		uint32_t hazard_id = 0;
		if( packet->Remaining() )
			hazard_id = packet->NextUInt();
		
		if( State >= XWing::State::FLYING )
		{
			Pos3D pos( x, y, z );
			Vec3D motion_vec( 0., 0., 0. );
			double bp_radius = 0.;
			double bp_time = 0.;
			if( shot_type == Shot::TYPE_TORPEDO )
			{
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 10., Res.GetSound("explosion.wav"), 2.5, &pos, &motion_vec, Rand::Bool() ? 360. : -360., 1.5 ) );
				bp_radius = 40.;
				bp_time = 0.125;
			}
			else if( shot_type == Shot::TYPE_MISSILE )
			{
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 17., Res.GetSound("explosion.wav"), 2.5, &pos, &motion_vec, Rand::Bool() ? 450. : -450., 2.5 ) );
				bp_radius = 30.;
				bp_time = 0.0625;
			}
			else if( (shot_type == Shot::TYPE_TURBO_LASER_GREEN) || (shot_type == Shot::TYPE_TURBO_LASER_RED) )
			{
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 7., NULL, 0., &pos, &motion_vec, 0., 7. ) );
				bp_radius = 20.;
			}
			else if( (shot_type != Shot::TYPE_ION_CANNON) && (Cfg.SettingAsDouble("g_effects",1.) > 0.) )
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 2., NULL, 0., &pos, &motion_vec, 0., 7. ) );
			
			if( BlastPoints && bp_radius )
			{
				Asteroid *hazard = (Asteroid*) Data.GetObject( hazard_id );
				if( hazard && (hazard->Type() == XWing::Object::ASTEROID) )
					hazard->SetBlastPoint( x, y, z, bp_radius, bp_time );
			}
		}
		return true;
	}
	
	else if( type == XWing::Packet::MISC_HIT_SHIP )
	{
		uint32_t ship_id = packet->NextUInt();
		double health = packet->NextFloat();
		double shield_f = packet->NextFloat();
		double shield_r = packet->NextFloat();
		uint8_t flags = packet->NextUChar();
		const char *subsystem = packet->NextString();
		double subsystem_health = subsystem[0] ? packet->NextFloat() : 0.;
		double x = packet->NextDouble();
		double y = packet->NextDouble();
		double z = packet->NextDouble();
		double radius = packet->NextDouble();
		
		Ship *ship = NULL;
		double old_health = 0.;
		double old_shields = 0.;
		
		GameObject *ship_obj = Data.GetObject( ship_id );
		if( ship_obj && (ship_obj->Type() == XWing::Object::SHIP) )
		{
			ship = (Ship*) ship_obj;
			old_health = ship->Health;
			old_shields = ship->ShieldF + ship->ShieldR;
			ship->HitClock.Reset();
			ship->HitFlags = flags;
			ship->SetHealth( health );
			ship->ShieldF = shield_f;
			ship->ShieldR = shield_r;
			if( subsystem[0] )
				ship->Subsystems[ subsystem ] = subsystem_health;
		}
		
		if( State >= XWing::State::FLYING )
		{
			if( ship )
			{
				Mix_Chunk *sound = NULL;
				double loudness = 1.;
				if( ship->ID == ObservedShipID )
				{
					if( ship->Health < old_health )
						sound = Res.GetSound("damage_hull.wav");
					else if( ship->ShieldF + ship->ShieldR < old_shields )
						sound = Res.GetSound("damage_shield.wav");
					loudness = 2.;
				}
				if( sound )
					Snd.PlayAt( sound, x, y, z, loudness );
				
				double total_damage = (old_shields + old_health) - (ship->ShieldF + ship->ShieldR + ship->Health);
				double knock_scale = total_damage / 20.;
				Vec3D knock( ship->X - x, ship->Y - y, ship->Z - z );
				if( knock.Length() > 1. )
					knock.ScaleTo( 1. );
				ship->KnockCockpit( &knock, knock_scale );
				ship->SetBlastPoint( x, y, z, radius, 0.25 );
			}
		}
		return true;
	}
	
	else if( type == XWing::Packet::REPAIR )
	{
		uint32_t ship_id = packet->NextUInt();
		double health = packet->NextFloat();
		
		Ship *ship = (Ship*) Data.GetObject( ship_id );
		if( ship && (ship->Type() == XWing::Object::SHIP) )
		{
			ship->ShieldF = ship->ShieldR = 0.;
			ship->SetHealth( health );
			ship->HitFlags = Ship::HIT_HULL | Ship::HIT_REPAIR;
			ship->HitClock.Reset();
			
			if( State >= XWing::State::FLYING )
			{
				Mix_Chunk *sound = Res.GetSound("repair.wav");
				Snd.PlayAt( sound, ship->X, ship->Y, ship->Z, 1. );
				if( ship->ID == ObservedShipID )
					Snd.Play( sound );
				
				int sparks = 5.1 * Cfg.SettingAsDouble("g_effects",1.) + 0.5;
				double spark_time = 0.75 / (sparks + 1.);
				for( int i = 0; i < sparks; i ++ )
				{
					Pos3D pos = *ship + ( ship->Fwd * Rand::Double(-0.2,0.) * i + ship->Up * Rand::Double(-0.1,0.1) + ship->Right * Rand::Double(-0.1,0.1) ) * ship->Radius();
					Vec3D motion_vec = ship->MotionVector + ship->Up * Rand::Double(-2.,0.) + ship->Fwd * Rand::Double(-0.5,0.) + ship->Right * Rand::Double(-0.5,0.5);
					Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 1., NULL, 0., &pos, &motion_vec, Rand::Bool() ? -600. : 600., 10. ) );
					Data.Effects.back().Lifetime.CountUpToSecs = i * spark_time;
					Data.Effects.back().MoveAlong( &(Data.Effects.back().MotionVector), Data.Effects.back().Lifetime.CountUpToSecs );
				}
			}
		}
		return true;
	}
	
	else if( type == XWing::Packet::REARM )
	{
		uint32_t ship_id = packet->NextUInt();
		uint8_t count = packet->NextUChar();
		
		Ship *ship = (Ship*) Data.GetObject( ship_id );
		if( ship && (ship->Type() == XWing::Object::SHIP) && ship->Rearm( count ) )
		{
			if( State >= XWing::State::FLYING )
			{
				Mix_Chunk *sound = Res.GetSound("rearm.wav");
				if( ship->ID == ObservedShipID )
					Snd.Play( sound );
				else
					Snd.PlayFromObject( sound, ship, 0.5 );
			}
			ship->KnockCockpit( &(ship->Fwd), -0.5 );
		}
		return true;
	}
	
	else if( type == XWing::Packet::ENGINE_SOUND )
	{
		Mix_Chunk *sound = Res.GetSound( packet->NextString() );
		
		if( packet->Remaining() )
		{
			uint32_t ship_id = packet->NextUInt();
			if( ship_id != ObservedShipID )
				sound = NULL;
		}
		
		if( sound )
		{
			double engine_volume = Cfg.SettingAsDouble("s_engine_volume",1.);
			if( engine_volume > 0. )
				// FIXME: For now this is an easier way to adjust engine volume than adding playback categories to SoundOut.
				Snd.Play( sound, 0, std::max<int>( 0, std::min<int>( 254, (1. - engine_volume) * 255. )) );
		}
		
		return true;
	}
	
	else if( type == XWing::Packet::JUMP_OUT )
	{
		uint32_t ship_id = packet->NextUInt();
		
		Ship *ship = (Ship*) Data.GetObject( ship_id );
		if( ship && (ship->Type() == XWing::Object::SHIP) )
		{
			ship->JumpedOut = true;
			ship->JumpProgress = 0.;
			ship->Lifetime.Reset();
			ship->MotionVector = ship->Fwd * ship->MaxSpeed();
			
			if( State >= XWing::State::FLYING )
			{
				if( ship->PlayerID == Raptor::Game->PlayerID )
					Snd.Play( Res.GetSound("jump_out_cockpit.wav") );
				else
				{
					Mix_Chunk *sound = Res.GetSound("jump_out.wav");
					double loudness = ((ship->Category() == ShipClass::CATEGORY_CAPITAL) || (ship->Category() == ShipClass::CATEGORY_TRANSPORT)) ? 50. : 5.;
					Snd.PlayAt( sound, ship->X, ship->Y, ship->Z, loudness );
					Snd.PlayFromObject( sound, ship, loudness * 0.75 );
				}
			}
		}
		return true;
	}
	
	else if( type == XWing::Packet::TIME_REMAINING )
	{
		float remaining = packet->NextFloat();
		if( remaining )
			RoundTimer.Reset( remaining );
		else
			RoundTimer.CountUpToSecs = 0.;
		return true;
	}
	
	else if( type == XWing::Packet::TIME_TO_RESPAWN )
	{
		float remaining = packet->NextFloat();
		if( remaining )
			RespawnTimer.Reset( remaining );
		else
			RespawnTimer.CountUpToSecs = 0.;
		return true;
	}
	
	else if( type == XWing::Packet::CHECKPOINT )
	{
		uint32_t ship_id = packet->NextUInt();
		uint32_t next_checkpoint = packet->NextUInt();
		uint8_t flags = packet->NextUChar();
		
		if( State >= XWing::State::FLYING )
		{
			Ship *ship = (Ship*) Data.GetObject( ship_id );
			if( ship && (ship->Type() == XWing::Object::SHIP) && (next_checkpoint != ship->NextCheckpoint) )
			{
				ship->NextCheckpoint = next_checkpoint;
				ship->Target         = next_checkpoint;
				const Player *owner = ship->Owner();
				if( owner && (owner->ID == PlayerID) )
					Snd.Play( Res.GetSound("checkpoint.wav") );
				else
				{
					// Let turret gunners hear when their ship passes a checkpoint.
					std::set<Player*> gunners = ship->PlayersInTurrets();
					for( std::set<Player*>::const_iterator gunner_iter = gunners.begin(); gunner_iter != gunners.end(); gunner_iter ++ )
					{
						if( (*gunner_iter)->ID == Raptor::Game->PlayerID )
						{
							Snd.Play( Res.GetSound("checkpoint.wav") );
							break;
						}
					}
				}
			}
		}
		
		if( flags & 0x01 )  // CheckpointFirstTouch
		{
			for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
			{
				if( obj_iter->second->Type() == XWing::Object::SHIP )
				{
					Ship *ship = (Ship*) obj_iter->second;
					ship->NextCheckpoint = next_checkpoint;
					if( ship->Target )
					{
						const GameObject *target = Data.GetObject( ship->Target );
						if( target && (target->Type() == XWing::Object::CHECKPOINT) )
						{
							ship->Target = next_checkpoint;
							const Player *owner = ship->Owner();
							if( owner && (owner->ID == PlayerID) )
								Snd.Play( Res.GetSound("beep.wav") );
						}
					}
				}
			}
		}
	}
	
	else if( type == XWing::Packet::LOBBY )
	{
		ChangeState( XWing::State::LOBBY );
		
		if( packet->Remaining() )
		{
			MissionList.clear();
			uint16_t mission_count = packet->NextUShort();
			for( uint16_t i = 0; i < mission_count; i ++ )
			{
				std::string mission_id = packet->NextString();
				MissionList[ mission_id ] = packet->NextString();
			}
			
			if( packet->Remaining() )
			{
				ServerFeatures.clear();
				uint8_t feature_count = packet->NextUChar();
				for( uint8_t i = 0; i < feature_count; i ++ )
					ServerFeatures.insert( packet->NextUInt() );
			}
		}
		
		return true;
	}
	
	else if( type == XWing::Packet::FLY )
	{
		if( packet->Remaining() )
			GameType = packet->NextUInt();
		
		ChangeState( XWing::State::FLYING );
		return true;
	}
	
	else if( type == XWing::Packet::GAMETYPE )
	{
		GameType = packet->NextUInt();
		return true;
	}
	
	else if( type == XWing::Packet::ROUND_ENDED )
	{
		GameType = packet->NextUInt();
		Victor   = packet->NextUShort();
		
		// The server sends alternate victory music (0 for defeat) if all players are on the same team and lose.
		uint16_t victor_music = Victor;
		victor_music = packet->NextUShort();
		
		if( State >= XWing::State::FLYING )
		{
			std::string music_name = "defeat.dat";
			
			// Victory checking is by player ID for FFA gametypes.
			if( (GameType == XWing::GameType::FFA_ELIMINATION) || (GameType == XWing::GameType::FFA_DEATHMATCH) || (GameType == XWing::GameType::FFA_RACE) )
			{
				if( Victor == PlayerID )
					music_name = "victory.dat";
				else if( Victor )
				{
					// If we're spectating and any non-AI player wins, play the victory tune.
					Player *player = Data.GetPlayer( PlayerID );
					if( player && (player->PropertyAsString("team") == "Spectator") )
						music_name = "victory.dat";
				}
			}
			// Usually assume it was a team game.
			else
			{
				if( victor_music == XWing::Team::REBEL )
					music_name = "rebel.dat";
				else if( victor_music == XWing::Team::EMPIRE )
					music_name = "empire.dat";
			}
			
			Mix_Music *music = Res.GetMusic(music_name);
			if( music )
			{
				Snd.StopMusic();
				Snd.PlayMusicOnce( music );
			}
		}
		
		ChangeState( XWing::State::ROUND_ENDED );
		
		return true;
	}
	
	else if( type == XWing::Packet::MISSION_COMPLETE )
	{
		std::string current_mission = packet->NextString();
		std::string next_mission    = packet->NextString();
		
		if( next_mission.length() && Str::BeginsWith( current_mission, "rebel" )
		&& ( Str::EqualsInsensitive( current_mission, Cfg.SettingAsString("rebel_mission") )
		 || (Str::EqualsInsensitive( current_mission, "rebel1" ) && Str::EqualsInsensitive( Cfg.SettingAsString("rebel_mission"), "rebel0" )) ) )
		{
			Cfg.Settings[ "rebel_mission"    ] = next_mission;
			Cfg.Settings[ "rebel_difficulty" ] = Data.PropertyAsString( "ai_skill", "1" );
			if( ! Cfg.HasSetting("empire_difficulty") )
				Cfg.Settings[ "empire_difficulty" ] = Cfg.Settings[ "rebel_difficulty" ];
		}
		else if( next_mission.length() && Str::BeginsWith( current_mission, "empire" )
		&& ( Str::EqualsInsensitive( current_mission, Cfg.SettingAsString("empire_mission") )
		 || (Str::EqualsInsensitive( current_mission, "empire1" ) && Str::EqualsInsensitive( Cfg.SettingAsString("empire_mission"), "empire0" )) ) )
		{
			Cfg.Settings[ "empire_mission"    ] = next_mission;
			Cfg.Settings[ "empire_difficulty" ] = Data.PropertyAsString( "ai_skill", "1" );
			if( ! Cfg.HasSetting("rebel_difficulty") )
				Cfg.Settings[ "rebel_difficulty" ] = Cfg.Settings[ "empire_difficulty" ];
		}
		
		ClientConfig campaign;
		campaign.Settings[ "rebel_mission"     ] = Cfg.SettingAsString( "rebel_mission",  "rebel0"  );
		campaign.Settings[ "empire_mission"    ] = Cfg.SettingAsString( "empire_mission", "empire0" );
		campaign.Settings[ "rebel_difficulty"  ] = Cfg.SettingAsString( "rebel_difficulty",  "1"    );
		campaign.Settings[ "empire_difficulty" ] = Cfg.SettingAsString( "empire_difficulty", "1"    );
		campaign.Save( "campaign.cfg", false );
	}
	
	else if( type == XWing::Packet::TOGGLE_COPILOT )
	{
		Snd.Play( Res.GetSound("chewie.wav") );
		return true;
	}
	
	else if( type == XWing::Packet::ACHIEVEMENT )
	{
		Achievements.push( packet->NextString() );
		if( AchievementClock.Progress() >= 1. )
			AchievementClock.Reset( 2.25 );
		return true;
	}
	
	return RaptorGame::ProcessPacket( packet );
}


void XWingGame::ChangeState( int state )
{
	RespawnTimer.Reset( 0. );
	
	if( state < XWing::State::FLYING )
	{
		GameType = XWing::GameType::UNDEFINED;
		memset( StoredTargets, 0, sizeof(StoredTargets) );
		ClientShots.clear();
	}
	
	if( state != XWing::State::ROUND_ENDED )
		Victor = XWing::Team::NONE;
	
	while( Achievements.size() )
		Achievements.pop();
	AchievementClock.Reset( 0. );
	
	if( state == XWing::State::LOBBY )
		ShowLobby();
	else if( state == XWing::State::FLYING )
		BeginFlying();
	
	RaptorGame::ChangeState( state );
	
	if( (state == XWing::State::LOBBY) && Cfg.SettingAsBool("screensaver") && ! Raptor::Server->IsRunning() )
		ChangeState( XWing::State::FLYING );
}


void XWingGame::Disconnected( void )
{
	Mouse.ShowCursor = true;
	Data.TimeScale = 1.;
	ServerFeatures.clear();
	bool show_disconnected_popup = true;
	
	if( State >= XWing::State::LOBBY )
	{
		if( Cfg.SettingAsBool( "s_menu_music" ) )
			Snd.PlayMusicSubdir( "Menu" );
		else
			Snd.StopMusic();
		
		Layers.RemoveAll();
		Layers.Add( new MainMenu() );
		
		if( CampaignTeam )
			show_disconnected_popup = false;
		CampaignTeam = XWing::Team::NONE;
		
		MissionList.clear();
	}
	
	if( show_disconnected_popup )
		RaptorGame::Disconnected();
	
	#ifdef WIN32
		if( Cfg.SettingAsBool("saitek_enable") )
		{
			for( int i = 0; i < 20; i ++ )
				Saitek.SetX52ProLED( i, true );
			
			Saitek.SetX52ProLED( SaitekX52ProLED::ARed, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::BRed, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::DRed, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::ERed, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::T1Red, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::T3Red, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::T5Red, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::Hat2Red, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::ClutchRed, false );
			
			for( int i = 0; i < 3; i ++ )
				Saitek.SetX52ProMFD( i, "" );
			
			Saitek.ClearFIPImage();
		}
	#endif
}


void XWingGame::ShowLobby( void )
{
	ObservedShipID = 0;
	
	if( Cfg.SettingAsBool("screensaver") )
	{
		if( Raptor::Server->IsRunning() && (State <= XWing::State::LOBBY) )
		{
			// FIXME: Should this send an INFO packet instead of directly modifying server data from the client thread?
			Raptor::Server->Data.Lock.Lock();
			Raptor::Server->Data.Properties["gametype"]              = Cfg.SettingAsString( "screensaver_gametype",        "fleet"   );
			Raptor::Server->Data.Properties["dm_kill_limit"]         = Cfg.SettingAsString( "screensaver_kill_limit",      "0"       );
			Raptor::Server->Data.Properties["tdm_kill_limit"]        = Cfg.SettingAsString( "screensaver_kill_limit",      "0"       );
			Raptor::Server->Data.Properties["team_race_checkpoints"] = Cfg.SettingAsString( "screensaver_checkpoints",     "0"       );
			Raptor::Server->Data.Properties["ffa_race_checkpoints"]  = Cfg.SettingAsString( "screensaver_checkpoints",     "0"       );
			Raptor::Server->Data.Properties["hunt_time_limit"]       = Cfg.SettingAsString( "screensaver_time_limit",      "0"       );
			Raptor::Server->Data.Properties["yavin_time_limit"]      = Cfg.SettingAsString( "screensaver_time_limit",      "0"       );
			Raptor::Server->Data.Properties["race_time_limit"]       = Cfg.SettingAsString( "screensaver_time_limit",      "0"       );
			Raptor::Server->Data.Properties["ai_waves"]              = Cfg.SettingAsString( "screensaver_ai_waves",        "7"       );
			Raptor::Server->Data.Properties["ai_flock"]              = Cfg.SettingAsString( "screensaver_ai_flock",        "true"    );
			Raptor::Server->Data.Properties["ai_respawn"]            = Cfg.SettingAsString( "screensaver_ai_respawn",      "true"    );
			Raptor::Server->Data.Properties["ai_grouped"]            = Cfg.SettingAsString( "screensaver_ai_grouped",      "false"   );
			Raptor::Server->Data.Properties["spawn"]                 = Cfg.SettingAsString( "screensaver_spawn",           "GUN,YT1300,T/A" );
			Raptor::Server->Data.Properties["rebel_fighter"]         = Cfg.SettingAsString( "screensaver_rebel_fighter",   "X/W"     );
			Raptor::Server->Data.Properties["rebel_bomber"]          = Cfg.SettingAsString( "screensaver_rebel_bomber",    "A/W"     );
			Raptor::Server->Data.Properties["rebel_cruiser"]         = Cfg.SettingAsString( "screensaver_rebel_cruiser",   "CRV"     );
			Raptor::Server->Data.Properties["rebel_cruisers"]        = Cfg.SettingAsString( "screensaver_rebel_cruisers",  "3"       );
			Raptor::Server->Data.Properties["rebel_frigate"]         = Cfg.SettingAsString( "screensaver_rebel_frigate",   "FRG"     );
			Raptor::Server->Data.Properties["rebel_frigates"]        = Cfg.SettingAsString( "screensaver_rebel_frigates",  "1"       );
			Raptor::Server->Data.Properties["rebel_flagship"]        = Cfg.SettingAsString( "screensaver_rebel_flagship",  "FRG"     );
			Raptor::Server->Data.Properties["empire_fighter"]        = Cfg.SettingAsString( "screensaver_empire_fighter",  "T/I"     );
			Raptor::Server->Data.Properties["empire_bomber"]         = Cfg.SettingAsString( "screensaver_empire_bomber",   "T/F"     );
			Raptor::Server->Data.Properties["empire_cruiser"]        = Cfg.SettingAsString( "screensaver_empire_cruiser",  "INT"     );
			Raptor::Server->Data.Properties["empire_cruisers"]       = Cfg.SettingAsString( "screensaver_empire_cruisers", "3"       );
			Raptor::Server->Data.Properties["empire_frigate"]        = Cfg.SettingAsString( "screensaver_empire_frigate",  "ISD"     );
			Raptor::Server->Data.Properties["empire_frigates"]       = Cfg.SettingAsString( "screensaver_empire_frigates", "1"       );
			Raptor::Server->Data.Properties["empire_flagship"]       = Cfg.SettingAsString( "screensaver_empire_flagship", "ISD"     );
			Raptor::Server->Data.Properties["yavin_rebel_fighter"]   = Cfg.SettingAsString( "screensaver_rebel_fighter",   "X/W"     );
			Raptor::Server->Data.Properties["yavin_rebel_bomber"]    = Cfg.SettingAsString( "screensaver_rebel_bomber",    "Y/W"     );
			Raptor::Server->Data.Properties["yavin_empire_fighter"]  = Cfg.SettingAsString( "screensaver_empire_fighter",  "T/F"     );
			Raptor::Server->Data.Properties["defending_team"]        = Cfg.SettingAsString( "screensaver_defending_team",  "rebel"   );
			Raptor::Server->Data.Properties["asteroids"]             = Cfg.SettingAsString( "screensaver_asteroids",       "64"      );
			Raptor::Server->Data.Properties["bg"]                    = Cfg.SettingAsString( "screensaver_bg",              "nebula2" );
			Raptor::Server->Data.Properties["time_scale"]            = Cfg.SettingAsString( "screensaver_time_scale",      "1"       );
			Raptor::Server->Data.Properties["allow_ship_change"]     = "true";
			Raptor::Server->Data.Properties["allow_team_change"]     = "true";
			Raptor::Server->Data.Properties["allow_pause"]           = "true";
			Data.Properties = Raptor::Server->Data.Properties;
			Raptor::Server->Data.Lock.Unlock();
			Cfg.Load( "screensaver.cfg" );
		}
		
		SetPlayerProperty( "ship", "Spectator" );
		
		std::string player_name = Cfg.SettingAsString( "name", "Screensaver", "Screensaver" );
		if( ! Str::ContainsInsensitive( player_name, "Screensaver" ) )
			player_name += std::string(" Screensaver");
		SetPlayerProperty( "name", player_name );
		
		if( Raptor::Server->IsRunning() )
		{
			Packet fly = Packet( XWing::Packet::FLY );
			Net.Send( &fly );
		}
		
		Mouse.ShowCursor = false;
	}
	else // Not screensaver.
	{
		Mouse.ShowCursor = true;
		
		if( State != XWing::State::LOBBY )
		{
			if( Cfg.SettingAsBool( "s_menu_music" ) )
				Snd.PlayMusicSubdir( "Lobby" );
			else
				Snd.StopMusic();
			
			Layers.RemoveAll();
			Layers.Add( new LobbyMenu() );
		}
		
		if( CampaignTeam && (State <= XWing::State::LOBBY) && Raptor::Server->IsRunning() )
		{
			Packet info( Raptor::Packet::INFO );
			info.AddUShort( 5 );
			
			info.AddString( "gametype" );
			info.AddString( "mission" );
			
			info.AddString( "mission" );
			info.AddString( (CampaignTeam == XWing::Team::EMPIRE) ? Cfg.SettingAsString("empire_mission","empire0") : Cfg.SettingAsString("rebel_mission","rebel0") );
			
			info.AddString( "ai_skill" );
			info.AddString( (CampaignTeam == XWing::Team::EMPIRE) ? Cfg.SettingAsString("empire_difficulty","1") : Cfg.SettingAsString("rebel_difficulty","1") );
			
			info.AddString( "campaign" );
			info.AddString( (CampaignTeam == XWing::Team::EMPIRE) ? "empire" : "rebel" );
			
			info.AddString( "permissions" );
			info.AddString( "admin" );
			
			Net.Send( &info );
		}
	}
	
	#ifdef WIN32
		if( Cfg.SettingAsBool("saitek_enable") )
		{
			for( int i = 0; i < 20; i ++ )
				Saitek.SetX52ProLED( i, true );
			
			Saitek.SetX52ProLED( SaitekX52ProLED::AGreen, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::BGreen, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::DGreen, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::EGreen, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::T1Green, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::T3Green, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::T5Green, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::Hat2Green, false );
			Saitek.SetX52ProLED( SaitekX52ProLED::ClutchGreen, false );
			
			for( int i = 0; i < 3; i ++ )
				Saitek.SetX52ProMFD( i, "" );
			
			Saitek.ClearFIPImage();
		}
	#endif
}


void XWingGame::BeginFlying( void )
{
	if( State != XWing::State::FLYING )
	{
		if( Cfg.SettingAsBool( "s_game_music" ) )
			Snd.PlayMusicSubdir( "Flight" );
		else
			Snd.StopMusic();
		
		// Always start flying in default view (1st person cockpit/gunner).
		bool screensaver = Cfg.SettingAsBool( "screensaver" );
		Cfg.Settings[ "view" ] = screensaver ? Cfg.SettingAsString( "screensaver_view", "cycle" ) : "auto";
		
		ObservedShipID = 0;
		LookYaw = 0.;
		LookPitch = 0.;
		Head.Recenter();
		
		RoundTimer.Reset( 0. );
		
		Layers.RemoveAll();
		Layers.Add( new RenderLayer() );
		
		if( screensaver )
		{
			// Use "view" rather than "spectator_view" in screensaver.
			Cfg.Settings[ "spectator_view" ] = "auto";
			
			// Add an input handler to quit the screensaver when necessary.
			AddScreensaverLayer();
		}
		
		Mouse.ShowCursor = false;
	}
}


GameObject *XWingGame::NewObject( uint32_t id, uint32_t type )
{
	if( type == XWing::Object::SHOT )
		return new Shot( id );
	else if( type == XWing::Object::SHIP )
		return new Ship( id );
	else if( type == XWing::Object::TURRET )
		return new Turret( id );
	else if( type == XWing::Object::ASTEROID )
		return new Asteroid( id );
	else if( type == XWing::Object::DEATH_STAR_BOX )
		return new DeathStarBox( id );
	else if( type == XWing::Object::DEATH_STAR )
		return new DeathStar( id );
	else if( type == XWing::Object::CHECKPOINT )
		return new Checkpoint( id );
	else if( type == XWing::Object::SHIP_CLASS )
		return new ShipClass( id );
	
	return RaptorGame::NewObject( id, type );
}


const ShipClass *XWingGame::GetShipClass( const std::string &name ) const
{
	for( std::map<uint32_t,GameObject*>::const_iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->Type() == XWing::Object::SHIP_CLASS )
		{
			const ShipClass *sc = (const ShipClass*) obj_iter->second;
			if( strcasecmp( sc->ShortName.c_str(), name.c_str() ) == 0 )
				return sc;
		}
	}
	
	return NULL;
}
