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
#include "Ship.h"
#include "Shot.h"
#include "Asteroid.h"
#include "Turret.h"
#include "DeathStar.h"
#include "DeathStarBox.h"
#include "ShipClass.h"
#include "Checkpoint.h"


XWingGame::XWingGame( std::string version ) : RaptorGame( "X-Wing Revival", version )
{
	ZeroLagServer = false;
	GameType = XWing::GameType::UNDEFINED;
	CampaignTeam = XWing::Team::NONE;
	ObservedShipID = 0;
	memset( StoredTargets, 0, sizeof(StoredTargets) );
	View = XWing::View::AUTO;
	LookYaw = 0.;
	LookPitch = 0.;
	ThumbstickLook = true;
	AsteroidLOD = 1.;
	OverlayScroll = 0.;
	BlastPoints = 0;
	Victor = XWing::Team::NONE;
	memset( Controls, 0, sizeof(Controls) );
	
	// Analog Axes
	Controls[ XWing::Control::ROLL              ] = Input.AddControl( "Roll",      "Pedal",   -1.,  1. );
	Controls[ XWing::Control::ROLL_INVERTED     ] = Input.AddControl( "Roll-",     "Pedal",    1., -1. );
	Controls[ XWing::Control::PITCH             ] = Input.AddControl( "Pitch",     "",        -1.,  1. );
	Controls[ XWing::Control::PITCH_INVERTED    ] = Input.AddControl( "Pitch-",    "",         1., -1. );
	Controls[ XWing::Control::YAW               ] = Input.AddControl( "Yaw",       "Pedal",   -1.,  1. );
	Controls[ XWing::Control::YAW_INVERTED      ] = Input.AddControl( "Yaw-",      "Pedal",    1., -1. );
	Controls[ XWing::Control::THROTTLE          ] = Input.AddControl( "Throttle",  "Throttle", 1.,  0. );
	Controls[ XWing::Control::THROTTLE_INVERTED ] = Input.AddControl( "Throttle-", "Throttle" );
	Controls[ XWing::Control::LOOK_X            ] = Input.AddControl( "LookX",     "",        -1.,  1. );
	Controls[ XWing::Control::LOOK_X_INVERTED   ] = Input.AddControl( "LookX-",    "",         1., -1. );
	Controls[ XWing::Control::LOOK_Y            ] = Input.AddControl( "LookY",     "",        -1.,  1. );
	Controls[ XWing::Control::LOOK_Y_INVERTED   ] = Input.AddControl( "LookY-",    "",         1., -1. );
	
	// Analog or Digital Controls
	Controls[ XWing::Control::ROLL_LEFT               ] = Input.AddControl( "RollLeft",  "Pedal" );
	Controls[ XWing::Control::ROLL_RIGHT              ] = Input.AddControl( "RollRight", "Pedal" );
	Controls[ XWing::Control::PITCH_UP                ] = Input.AddControl( "PitchUp" );
	Controls[ XWing::Control::PITCH_DOWN              ] = Input.AddControl( "PitchDown" );
	Controls[ XWing::Control::YAW_LEFT                ] = Input.AddControl( "YawLeft",   "Pedal" );
	Controls[ XWing::Control::YAW_RIGHT               ] = Input.AddControl( "YawRight",  "Pedal" );
	
	// Held Digital Buttons
	Controls[ XWing::Control::THROTTLE_UP             ] = Input.AddControl( "ThrottleUp" );
	Controls[ XWing::Control::THROTTLE_DOWN           ] = Input.AddControl( "ThrottleDown" );
	Controls[ XWing::Control::THROTTLE_0              ] = Input.AddControl( "Throttle0" );
	Controls[ XWing::Control::THROTTLE_33             ] = Input.AddControl( "Throttle33" );
	Controls[ XWing::Control::THROTTLE_66             ] = Input.AddControl( "Throttle66" );
	Controls[ XWing::Control::THROTTLE_100            ] = Input.AddControl( "Throttle100" );
	Controls[ XWing::Control::FIRE                    ] = Input.AddControl( "Fire" );
	Controls[ XWing::Control::TARGET_STORE            ] = Input.AddControl( "TargetStore" );
	Controls[ XWing::Control::TARGET_NOTHING          ] = Input.AddControl( "TargetClear" );
	Controls[ XWing::Control::TARGET_CROSSHAIR        ] = Input.AddControl( "TargetCrosshair" );
	Controls[ XWing::Control::TARGET_NEAREST_ENEMY    ] = Input.AddControl( "TargetEnemy" );
	Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ] = Input.AddControl( "TargetAttacker" );
	Controls[ XWing::Control::TARGET_NEWEST_INCOMING  ] = Input.AddControl( "TargetIncoming" );
	Controls[ XWing::Control::TARGET_TARGET_ATTACKER  ] = Input.AddControl( "TargetTheirAttacker" );
	Controls[ XWing::Control::TARGET_OBJECTIVE        ] = Input.AddControl( "TargetObjective" );
	Controls[ XWing::Control::TARGET_DOCKABLE         ] = Input.AddControl( "TargetDockable" );
	Controls[ XWing::Control::TARGET_NEWEST           ] = Input.AddControl( "TargetNewest" );
	Controls[ XWing::Control::TARGET_GROUPMATE        ] = Input.AddControl( "TargetGroupmate" );
	Controls[ XWing::Control::TARGET_SYNC             ] = Input.AddControl( "TargetDataLink" );
	Controls[ XWing::Control::EJECT                   ] = Input.AddControl( "Eject" );
	Controls[ XWing::Control::LOOK_CENTER             ] = Input.AddControl( "LookCenter" );
	Controls[ XWing::Control::LOOK_UP                 ] = Input.AddControl( "LookUp" );
	Controls[ XWing::Control::LOOK_DOWN               ] = Input.AddControl( "LookDown" );
	Controls[ XWing::Control::LOOK_LEFT               ] = Input.AddControl( "LookLeft" );
	Controls[ XWing::Control::LOOK_RIGHT              ] = Input.AddControl( "LookRight" );
	Controls[ XWing::Control::LOOK_UP_LEFT            ] = Input.AddControl( "LookUpLeft" );
	Controls[ XWing::Control::LOOK_UP_RIGHT           ] = Input.AddControl( "LookUpRight" );
	Controls[ XWing::Control::LOOK_DOWN_LEFT          ] = Input.AddControl( "LookDownLeft" );
	Controls[ XWing::Control::LOOK_DOWN_RIGHT         ] = Input.AddControl( "LookDownRight" );
	Controls[ XWing::Control::GLANCE_UP               ] = Input.AddControl( "GlanceUp" );
	Controls[ XWing::Control::GLANCE_BACK             ] = Input.AddControl( "GlanceBack" );
	Controls[ XWing::Control::GLANCE_LEFT             ] = Input.AddControl( "GlanceLeft" );
	Controls[ XWing::Control::GLANCE_RIGHT            ] = Input.AddControl( "GlanceRight" );
	Controls[ XWing::Control::GLANCE_UP_LEFT          ] = Input.AddControl( "GlanceUpLeft" );
	Controls[ XWing::Control::GLANCE_UP_RIGHT         ] = Input.AddControl( "GlanceUpRight" );
	Controls[ XWing::Control::GLANCE_BACK_LEFT        ] = Input.AddControl( "GlanceBackLeft" );
	Controls[ XWing::Control::GLANCE_BACK_RIGHT       ] = Input.AddControl( "GlanceBackRight" );
	Controls[ XWing::Control::SCORES                  ] = Input.AddControl( "Scores" );
	
	// Pressed Digital Buttons
	Controls[ XWing::Control::WEAPON                  ] = Input.AddControl( "WeaponNext" );
	Controls[ XWing::Control::MODE                    ] = Input.AddControl( "WeaponMode" );
	Controls[ XWing::Control::SHIELD_DIR              ] = Input.AddControl( "AngleDeflector" );
	Controls[ XWing::Control::TARGET1                 ] = Input.AddControl( "Target1" );
	Controls[ XWing::Control::TARGET2                 ] = Input.AddControl( "Target2" );
	Controls[ XWing::Control::TARGET3                 ] = Input.AddControl( "Target3" );
	Controls[ XWing::Control::TARGET4                 ] = Input.AddControl( "Target4" );
	Controls[ XWing::Control::TARGET_PREV             ] = Input.AddControl( "TargetPrev" );
	Controls[ XWing::Control::TARGET_NEXT             ] = Input.AddControl( "TargetNext" );
	Controls[ XWing::Control::TARGET_PREV_ENEMY       ] = Input.AddControl( "TargetPrevEnemy" );
	Controls[ XWing::Control::TARGET_NEXT_ENEMY       ] = Input.AddControl( "TargetNextEnemy" );
	Controls[ XWing::Control::TARGET_PREV_FRIENDLY    ] = Input.AddControl( "TargetPrevFriendly" );
	Controls[ XWing::Control::TARGET_NEXT_FRIENDLY    ] = Input.AddControl( "TargetNextFriendly" );
	Controls[ XWing::Control::TARGET_PREV_PLAYER      ] = Input.AddControl( "TargetPrevPlayer" );
	Controls[ XWing::Control::TARGET_NEXT_PLAYER      ] = Input.AddControl( "TargetNextPlayer" );
	Controls[ XWing::Control::TARGET_PREV_SUBSYSTEM   ] = Input.AddControl( "TargetPrevSystem" );
	Controls[ XWing::Control::TARGET_NEXT_SUBSYSTEM   ] = Input.AddControl( "TargetNextSystem" );
	Controls[ XWing::Control::SEAT_COCKPIT            ] = Input.AddControl( "Cockpit" );
	Controls[ XWing::Control::SEAT_GUNNER1            ] = Input.AddControl( "Gunner1" );
	Controls[ XWing::Control::SEAT_GUNNER2            ] = Input.AddControl( "Gunner2" );
	Controls[ XWing::Control::CHEWIE_TAKE_THE_WHEEL   ] = Input.AddControl( "Chewie" );
	Controls[ XWing::Control::VIEW_COCKPIT            ] = Input.AddControl( "ViewCockpit" );
	Controls[ XWing::Control::VIEW_CROSSHAIR          ] = Input.AddControl( "ViewCrosshair" );
	Controls[ XWing::Control::VIEW_CHASE              ] = Input.AddControl( "ViewChase" );
	Controls[ XWing::Control::VIEW_PADLOCK            ] = Input.AddControl( "ViewPadlock" );
	Controls[ XWing::Control::VIEW_STATIONARY         ] = Input.AddControl( "ViewDropCam" );
	Controls[ XWing::Control::VIEW_CINEMA             ] = Input.AddControl( "ViewCinema" );
	Controls[ XWing::Control::VIEW_FIXED              ] = Input.AddControl( "ViewFixed" );
	Controls[ XWing::Control::VIEW_SELFIE             ] = Input.AddControl( "ViewPilot" );
	Controls[ XWing::Control::VIEW_GUNNER             ] = Input.AddControl( "ViewGunner" );
	Controls[ XWing::Control::VIEW_CYCLE              ] = Input.AddControl( "ViewCycle" );
	Controls[ XWing::Control::VIEW_INSTRUMENTS        ] = Input.AddControl( "ViewInstruments" );
	Controls[ XWing::Control::CHAT                    ] = Input.AddControl( "Chat" );
	Controls[ XWing::Control::MENU                    ] = Input.AddControl( "MainMenu" );
	Controls[ XWing::Control::PREFS                   ] = Input.AddControl( "PrefsMenu" );
	Controls[ XWing::Control::PAUSE                   ] = Input.AddControl( "Pause" );
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
}


void XWingGame::SetDefaultControls( void )
{
	RaptorGame::SetDefaultControls();
	
	Cfg.KeyBinds[ SDLK_UP           ] = Controls[ XWing::Control::PITCH_DOWN              ];
	Cfg.KeyBinds[ SDLK_DOWN         ] = Controls[ XWing::Control::PITCH_UP                ];
	Cfg.KeyBinds[ SDLK_LEFT         ] = Controls[ XWing::Control::YAW_LEFT                ];
	Cfg.KeyBinds[ SDLK_RIGHT        ] = Controls[ XWing::Control::YAW_RIGHT               ];
	Cfg.KeyBinds[ SDLK_d            ] = Controls[ XWing::Control::ROLL_LEFT               ];
	Cfg.KeyBinds[ SDLK_f            ] = Controls[ XWing::Control::ROLL_RIGHT              ];
	Cfg.KeyBinds[ SDLK_BACKSLASH    ] = Controls[ XWing::Control::THROTTLE_0              ];
	Cfg.KeyBinds[ SDLK_LEFTBRACKET  ] = Controls[ XWing::Control::THROTTLE_33             ];
	Cfg.KeyBinds[ SDLK_RIGHTBRACKET ] = Controls[ XWing::Control::THROTTLE_66             ];
	Cfg.KeyBinds[ SDLK_BACKSPACE    ] = Controls[ XWing::Control::THROTTLE_100            ];
	Cfg.KeyBinds[ SDLK_EQUALS       ] = Controls[ XWing::Control::THROTTLE_UP             ];
	Cfg.KeyBinds[ SDLK_MINUS        ] = Controls[ XWing::Control::THROTTLE_DOWN           ];
	Cfg.KeyBinds[ SDLK_SPACE        ] = Controls[ XWing::Control::FIRE                    ];
	Cfg.KeyBinds[ SDLK_LSHIFT       ] = Controls[ XWing::Control::TARGET_STORE            ];
	Cfg.KeyBinds[ SDLK_RSHIFT       ] = Controls[ XWing::Control::TARGET_STORE            ];
	Cfg.KeyBinds[ SDLK_LCTRL        ] = Controls[ XWing::Control::TARGET_CROSSHAIR        ];
	Cfg.KeyBinds[ SDLK_e            ] = Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ];
	Cfg.KeyBinds[ SDLK_r            ] = Controls[ XWing::Control::TARGET_NEAREST_ENEMY    ];
	Cfg.KeyBinds[ SDLK_u            ] = Controls[ XWing::Control::TARGET_NEWEST           ];
	Cfg.KeyBinds[ SDLK_p            ] = Controls[ XWing::Control::TARGET_NEXT_PLAYER      ];
	Cfg.KeyBinds[ SDLK_o            ] = Controls[ XWing::Control::TARGET_OBJECTIVE        ];
	Cfg.KeyBinds[ SDLK_h            ] = Controls[ XWing::Control::TARGET_DOCKABLE         ];
	Cfg.KeyBinds[ SDLK_i            ] = Controls[ XWing::Control::TARGET_NEWEST_INCOMING  ];
	Cfg.KeyBinds[ SDLK_a            ] = Controls[ XWing::Control::TARGET_TARGET_ATTACKER  ];
	Cfg.KeyBinds[ SDLK_q            ] = Controls[ XWing::Control::TARGET_NOTHING          ];
	Cfg.KeyBinds[ SDLK_g            ] = Controls[ XWing::Control::TARGET_GROUPMATE        ];
	Cfg.KeyBinds[ SDLK_v            ] = Controls[ XWing::Control::TARGET_SYNC             ];
	Cfg.KeyBinds[ SDLK_CAPSLOCK     ] = Controls[ XWing::Control::TARGET_SYNC             ];
	Cfg.KeyBinds[ SDLK_k            ] = Controls[ XWing::Control::EJECT                   ];
	Cfg.KeyBinds[ SDLK_KP7          ] = Controls[ XWing::Control::LOOK_UP_LEFT            ];
	Cfg.KeyBinds[ SDLK_KP8          ] = Controls[ XWing::Control::LOOK_UP                 ];
	Cfg.KeyBinds[ SDLK_KP9          ] = Controls[ XWing::Control::LOOK_UP_RIGHT           ];
	Cfg.KeyBinds[ SDLK_KP4          ] = Controls[ XWing::Control::LOOK_LEFT               ];
	Cfg.KeyBinds[ SDLK_KP5          ] = Controls[ XWing::Control::LOOK_CENTER             ];
	Cfg.KeyBinds[ SDLK_KP6          ] = Controls[ XWing::Control::LOOK_RIGHT              ];
	Cfg.KeyBinds[ SDLK_KP1          ] = Controls[ XWing::Control::LOOK_DOWN_LEFT          ];
	Cfg.KeyBinds[ SDLK_KP2          ] = Controls[ XWing::Control::LOOK_DOWN               ];
	Cfg.KeyBinds[ SDLK_KP3          ] = Controls[ XWing::Control::LOOK_DOWN_RIGHT         ];
	Cfg.KeyBinds[ SDLK_INSERT       ] = Controls[ XWing::Control::GLANCE_UP_LEFT          ];
	Cfg.KeyBinds[ SDLK_HOME         ] = Controls[ XWing::Control::GLANCE_UP               ];
	Cfg.KeyBinds[ SDLK_PAGEUP       ] = Controls[ XWing::Control::GLANCE_UP_RIGHT         ];
	Cfg.KeyBinds[ SDLK_DELETE       ] = Controls[ XWing::Control::GLANCE_LEFT             ];
	Cfg.KeyBinds[ SDLK_END          ] = Controls[ XWing::Control::GLANCE_BACK             ];
	Cfg.KeyBinds[ SDLK_PAGEDOWN     ] = Controls[ XWing::Control::GLANCE_RIGHT            ];
	Cfg.KeyBinds[ SDLK_TAB          ] = Controls[ XWing::Control::SCORES                  ];
	
	Cfg.KeyBinds[ SDLK_w            ] = Controls[ XWing::Control::WEAPON                  ];
	Cfg.KeyBinds[ SDLK_x            ] = Controls[ XWing::Control::MODE                    ];
	Cfg.KeyBinds[ SDLK_s            ] = Controls[ XWing::Control::SHIELD_DIR              ];
	Cfg.KeyBinds[ SDLK_F5           ] = Controls[ XWing::Control::TARGET1                 ];
	Cfg.KeyBinds[ SDLK_F6           ] = Controls[ XWing::Control::TARGET2                 ];
	Cfg.KeyBinds[ SDLK_F7           ] = Controls[ XWing::Control::TARGET3                 ];
	Cfg.KeyBinds[ SDLK_F8           ] = Controls[ XWing::Control::TARGET4                 ];
	Cfg.KeyBinds[ SDLK_t            ] = Controls[ XWing::Control::TARGET_NEXT             ];
	Cfg.KeyBinds[ SDLK_y            ] = Controls[ XWing::Control::TARGET_PREV             ];
	Cfg.KeyBinds[ SDLK_COMMA        ] = Controls[ XWing::Control::TARGET_PREV_SUBSYSTEM   ];
	Cfg.KeyBinds[ SDLK_PERIOD       ] = Controls[ XWing::Control::TARGET_NEXT_SUBSYSTEM   ];
	Cfg.KeyBinds[ SDLK_s            ] = Controls[ XWing::Control::SHIELD_DIR              ];
	Cfg.KeyBinds[ SDLK_F1           ] = Controls[ XWing::Control::SEAT_COCKPIT            ];
	Cfg.KeyBinds[ SDLK_F2           ] = Controls[ XWing::Control::SEAT_GUNNER1            ];
	Cfg.KeyBinds[ SDLK_F3           ] = Controls[ XWing::Control::SEAT_GUNNER2            ];
	Cfg.KeyBinds[ SDLK_F4           ] = Controls[ XWing::Control::CHEWIE_TAKE_THE_WHEEL   ];
	Cfg.KeyBinds[ SDLK_1            ] = Controls[ XWing::Control::VIEW_COCKPIT            ];
	Cfg.KeyBinds[ SDLK_2            ] = Controls[ XWing::Control::VIEW_CROSSHAIR          ];
	Cfg.KeyBinds[ SDLK_3            ] = Controls[ XWing::Control::VIEW_CHASE              ];
	Cfg.KeyBinds[ SDLK_4            ] = Controls[ XWing::Control::VIEW_PADLOCK            ];
	Cfg.KeyBinds[ SDLK_5            ] = Controls[ XWing::Control::VIEW_STATIONARY         ];
	Cfg.KeyBinds[ SDLK_6            ] = Controls[ XWing::Control::VIEW_CINEMA             ];
	Cfg.KeyBinds[ SDLK_7            ] = Controls[ XWing::Control::VIEW_FIXED              ];
	Cfg.KeyBinds[ SDLK_8            ] = Controls[ XWing::Control::VIEW_GUNNER             ];
	Cfg.KeyBinds[ SDLK_9            ] = Controls[ XWing::Control::VIEW_CYCLE              ];
	Cfg.KeyBinds[ SDLK_0            ] = Controls[ XWing::Control::VIEW_INSTRUMENTS        ];
	Cfg.KeyBinds[ SDLK_SLASH        ] = Controls[ XWing::Control::VIEW_SELFIE             ];
	Cfg.KeyBinds[ SDLK_RETURN       ] = Controls[ XWing::Control::CHAT                    ];
	Cfg.KeyBinds[ SDLK_KP_ENTER     ] = Controls[ XWing::Control::CHAT                    ];
	Cfg.KeyBinds[ SDLK_ESCAPE       ] = Controls[ XWing::Control::MENU                    ];
	Cfg.KeyBinds[ SDLK_F9           ] = Controls[ XWing::Control::MENU                    ];
	Cfg.KeyBinds[ SDLK_F10          ] = Controls[ XWing::Control::PREFS                   ];
	Cfg.KeyBinds[ SDLK_PAUSE        ] = Controls[ XWing::Control::PAUSE                   ];
	
	Cfg.MouseBinds[ SDL_BUTTON_LEFT      ] = Controls[ XWing::Control::FIRE             ];
	Cfg.MouseBinds[ SDL_BUTTON_RIGHT     ] = Controls[ XWing::Control::TARGET_CROSSHAIR ];
	Cfg.MouseBinds[ SDL_BUTTON_MIDDLE    ] = Controls[ XWing::Control::WEAPON           ];
	Cfg.MouseBinds[ SDL_BUTTON_X1        ] = Controls[ XWing::Control::THROTTLE_DOWN    ];
	Cfg.MouseBinds[ SDL_BUTTON_X2        ] = Controls[ XWing::Control::THROTTLE_UP      ];
	Cfg.MouseBinds[ SDL_BUTTON_WHEELUP   ] = Controls[ XWing::Control::TARGET_PREV      ];
	Cfg.MouseBinds[ SDL_BUTTON_WHEELDOWN ] = Controls[ XWing::Control::TARGET_NEXT      ];
	
	Cfg.JoyAxisBinds[ "Joy" ][ 0 ] = Controls[ XWing::Control::ROLL     ];  // Stick X
	Cfg.JoyAxisBinds[ "Joy" ][ 1 ] = Controls[ XWing::Control::PITCH    ];  // Stick Y
	Cfg.JoyAxisBinds[ "Joy" ][ 2 ] = Controls[ XWing::Control::THROTTLE ];  // Throttle
	Cfg.JoyAxisBinds[ "Joy" ][ 3 ] = Controls[ XWing::Control::YAW      ];  // Twist
	
	Cfg.JoyButtonBinds[ "Joy" ][  0 ] = Controls[ XWing::Control::FIRE                    ];  // Trigger
	Cfg.JoyButtonBinds[ "Joy" ][  1 ] = Controls[ XWing::Control::WEAPON                  ];  // Fire
	Cfg.JoyButtonBinds[ "Joy" ][  2 ] = Controls[ XWing::Control::TARGET_CROSSHAIR        ];  // A
	Cfg.JoyButtonBinds[ "Joy" ][  3 ] = Controls[ XWing::Control::TARGET_NEAREST_ENEMY    ];  // B
	Cfg.JoyButtonBinds[ "Joy" ][  4 ] = Controls[ XWing::Control::MODE                    ];  // C
	Cfg.JoyButtonBinds[ "Joy" ][  5 ] = Controls[ XWing::Control::LOOK_CENTER             ];  // Pinkie
	Cfg.JoyButtonBinds[ "Joy" ][  6 ] = Controls[ XWing::Control::SHIELD_DIR              ];  // D
	Cfg.JoyButtonBinds[ "Joy" ][  7 ] = Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ];  // E
	Cfg.JoyButtonBinds[ "Joy" ][  8 ] = Controls[ XWing::Control::TARGET_NEWEST_INCOMING  ];  // T1
	Cfg.JoyButtonBinds[ "Joy" ][  9 ] = Controls[ XWing::Control::TARGET_SYNC             ];  // T2
	Cfg.JoyButtonBinds[ "Joy" ][ 10 ] = Controls[ XWing::Control::SEAT_COCKPIT            ];  // T3
	Cfg.JoyButtonBinds[ "Joy" ][ 11 ] = Controls[ XWing::Control::CHEWIE_TAKE_THE_WHEEL   ];  // T4
	Cfg.JoyButtonBinds[ "Joy" ][ 12 ] = Controls[ XWing::Control::SEAT_GUNNER1            ];  // T5
	Cfg.JoyButtonBinds[ "Joy" ][ 13 ] = Controls[ XWing::Control::SEAT_GUNNER2            ];  // T6
	Cfg.JoyButtonBinds[ "Joy" ][ 19 ] = Controls[ XWing::Control::TARGET1                 ];  // Hat 2 Up
	Cfg.JoyButtonBinds[ "Joy" ][ 20 ] = Controls[ XWing::Control::TARGET2                 ];  // Hat 2 Right
	Cfg.JoyButtonBinds[ "Joy" ][ 21 ] = Controls[ XWing::Control::TARGET3                 ];  // Hat 2 Down
	Cfg.JoyButtonBinds[ "Joy" ][ 22 ] = Controls[ XWing::Control::TARGET4                 ];  // Hat 2 Left
	Cfg.JoyButtonBinds[ "Joy" ][ 23 ] = Controls[ XWing::Control::TARGET_STORE            ];  // Hat 3 Up
	Cfg.JoyButtonBinds[ "Joy" ][ 24 ] = Controls[ XWing::Control::TARGET_NEXT             ];  // Hat 3 Right
	Cfg.JoyButtonBinds[ "Joy" ][ 25 ] = Controls[ XWing::Control::SCORES                  ];  // Hat 3 Down
	Cfg.JoyButtonBinds[ "Joy" ][ 26 ] = Controls[ XWing::Control::TARGET_PREV             ];  // Hat 3 Left
	Cfg.JoyButtonBinds[ "Joy" ][ 30 ] = Controls[ XWing::Control::TARGET_NEWEST_INCOMING  ];  // Clutch
	
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_UP    ] = Controls[ XWing::Control::GLANCE_UP    ];  // Hat 1 Up
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_RIGHT ] = Controls[ XWing::Control::GLANCE_RIGHT ];  // Hat 1 Right
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_DOWN  ] = Controls[ XWing::Control::GLANCE_BACK  ];  // Hat 1 Down
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_LEFT  ] = Controls[ XWing::Control::GLANCE_LEFT  ];  // Hat 1 Left
	
	if( Input.DeviceTypes.find("X52") != Input.DeviceTypes.end() )
	{
		#if SDL_VERSION_ATLEAST(2,0,0)
			Cfg.JoyAxisBinds[ "X52" ][ 0 ] = Controls[ XWing::Control::ROLL     ];  // Stick X
			Cfg.JoyAxisBinds[ "X52" ][ 1 ] = Controls[ XWing::Control::PITCH    ];  // Stick Y
			Cfg.JoyAxisBinds[ "X52" ][ 2 ] = Controls[ XWing::Control::THROTTLE ];  // Throttle
			Cfg.JoyAxisBinds[ "X52" ][ 5 ] = Controls[ XWing::Control::YAW      ];  // Twist
		#else
			Cfg.JoyAxisBinds[ "X52" ] = Cfg.JoyAxisBinds[ "Joy" ];
		#endif
		
		Cfg.JoyButtonBinds[ "X52" ] = Cfg.JoyButtonBinds[ "Joy" ];
		Cfg.JoyHatBinds   [ "X52" ] = Cfg.JoyHatBinds   [ "Joy" ];
		
		// If X52 has its own binds, other joysticks default to good settings for Cyborg Evo instead.
		Cfg.JoyButtonBinds[ "Joy" ].clear();
		Cfg.JoyButtonBinds[ "Joy" ][  0 ] = Controls[ XWing::Control::FIRE                    ];  // Trigger
		Cfg.JoyButtonBinds[ "Joy" ][  1 ] = Controls[ XWing::Control::WEAPON                  ];  // 2
		Cfg.JoyButtonBinds[ "Joy" ][  2 ] = Controls[ XWing::Control::MODE                    ];  // 3
		Cfg.JoyButtonBinds[ "Joy" ][  3 ] = Controls[ XWing::Control::TARGET_NEAREST_ENEMY    ];  // 4
		Cfg.JoyButtonBinds[ "Joy" ][  4 ] = Controls[ XWing::Control::LOOK_CENTER             ];  // 5
		Cfg.JoyButtonBinds[ "Joy" ][  5 ] = Controls[ XWing::Control::TARGET_CROSSHAIR        ];  // 6
		Cfg.JoyButtonBinds[ "Joy" ][  6 ] = Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ];  // F1
		Cfg.JoyButtonBinds[ "Joy" ][  7 ] = Controls[ XWing::Control::TARGET_NEWEST_INCOMING  ];  // F2
		Cfg.JoyButtonBinds[ "Joy" ][  8 ] = Controls[ XWing::Control::TARGET1                 ];  // F3
		Cfg.JoyButtonBinds[ "Joy" ][  9 ] = Controls[ XWing::Control::TARGET2                 ];  // F4
		Cfg.JoyButtonBinds[ "Joy" ][ 10 ] = Controls[ XWing::Control::SHIELD_DIR              ];  // >
		Cfg.JoyButtonBinds[ "Joy" ][ 11 ] = Controls[ XWing::Control::SCORES                  ];  // <
	}
	
	if( Input.DeviceTypes.find("SideWinder") != Input.DeviceTypes.end() )
	{
		#if SDL_VERSION_ATLEAST(2,0,0)
			Cfg.JoyAxisBinds[ "SideWinder" ][ 0 ] = Controls[ XWing::Control::ROLL     ];  // Stick X
			Cfg.JoyAxisBinds[ "SideWinder" ][ 1 ] = Controls[ XWing::Control::PITCH    ];  // Stick Y
			Cfg.JoyAxisBinds[ "SideWinder" ][ 2 ] = Controls[ XWing::Control::YAW      ];  // Twist
			Cfg.JoyAxisBinds[ "SideWinder" ][ 3 ] = Controls[ XWing::Control::THROTTLE ];  // Throttle
		#else
			Cfg.JoyAxisBinds[ "SideWinder" ] = Cfg.JoyAxisBinds[ "Joy" ];
		#endif
		
		Cfg.JoyButtonBinds[ "SideWinder" ][ 0 ] = Controls[ XWing::Control::FIRE                    ];  // Trigger
		Cfg.JoyButtonBinds[ "SideWinder" ][ 1 ] = Controls[ XWing::Control::MODE                    ];  // Stick Big Button
		Cfg.JoyButtonBinds[ "SideWinder" ][ 2 ] = Controls[ XWing::Control::TARGET_CROSSHAIR        ];  // Stick Top Button
		Cfg.JoyButtonBinds[ "SideWinder" ][ 3 ] = Controls[ XWing::Control::WEAPON                  ];  // Stick Bottom Button
		Cfg.JoyButtonBinds[ "SideWinder" ][ 4 ] = Controls[ XWing::Control::TARGET_NEAREST_ENEMY    ];  // A
		Cfg.JoyButtonBinds[ "SideWinder" ][ 5 ] = Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ];  // B
		Cfg.JoyButtonBinds[ "SideWinder" ][ 6 ] = Controls[ XWing::Control::TARGET_NEWEST_INCOMING  ];  // C
		Cfg.JoyButtonBinds[ "SideWinder" ][ 7 ] = Controls[ XWing::Control::SHIELD_DIR              ];  // D
		Cfg.JoyButtonBinds[ "SideWinder" ][ 8 ] = Controls[ XWing::Control::LOOK_CENTER             ];  // Shift
		
		Cfg.JoyHatBinds[ "SideWinder" ] = Cfg.JoyHatBinds[ "Joy" ];
	}
	
	Cfg.JoyAxisBinds[ "Pedal" ][ 0 ] = Controls[ XWing::Control::YAW_LEFT     ];  // Left Pedal
	Cfg.JoyAxisBinds[ "Pedal" ][ 1 ] = Controls[ XWing::Control::YAW_RIGHT    ];  // Right Pedal
	Cfg.JoyAxisBinds[ "Pedal" ][ 2 ] = Controls[ XWing::Control::YAW          ];  // Combined Slider
	
	Cfg.JoyAxisBinds[ "Throttle" ][ 0 ] = Controls[ XWing::Control::THROTTLE ];
	
	Cfg.JoyAxisBinds[ "Xbox" ][ 0 ] = Controls[ XWing::Control::YAW           ];  // Left Thumbstick X
	Cfg.JoyAxisBinds[ "Xbox" ][ 1 ] = Controls[ XWing::Control::PITCH         ];  // Left Thumbstick Y
#ifndef WIN32
	Cfg.JoyAxisBinds[ "Xbox" ][ 2 ] = Controls[ XWing::Control::ROLL_LEFT     ];  // Left Trigger
	Cfg.JoyAxisBinds[ "Xbox" ][ 3 ] = Controls[ XWing::Control::LOOK_X        ];  // Right Thumbstick X
	Cfg.JoyAxisBinds[ "Xbox" ][ 4 ] = Controls[ XWing::Control::LOOK_Y        ];  // Right Thumbstick Y
	Cfg.JoyAxisBinds[ "Xbox" ][ 5 ] = Controls[ XWing::Control::ROLL_RIGHT    ];  // Right Trigger
#elif SDL_VERSION_ATLEAST(2,0,0)
	Cfg.JoyAxisBinds[ "Xbox" ][ 2 ] = Controls[ XWing::Control::LOOK_X        ];  // Right Thumbstick X
	Cfg.JoyAxisBinds[ "Xbox" ][ 3 ] = Controls[ XWing::Control::LOOK_Y        ];  // Right Thumbstick Y
	Cfg.JoyAxisBinds[ "Xbox" ][ 4 ] = Controls[ XWing::Control::ROLL_LEFT     ];  // Left Trigger
	Cfg.JoyAxisBinds[ "Xbox" ][ 5 ] = Controls[ XWing::Control::ROLL_RIGHT    ];  // Right Trigger
#else
	Cfg.JoyAxisBinds[ "Xbox" ][ 2 ] = Controls[ XWing::Control::ROLL_INVERTED ];  // Triggers
	Cfg.JoyAxisBinds[ "Xbox" ][ 3 ] = Controls[ XWing::Control::LOOK_Y        ];  // Right Thumbstick Y
	Cfg.JoyAxisBinds[ "Xbox" ][ 4 ] = Controls[ XWing::Control::LOOK_X        ];  // Right Thumbstick X
#endif
	
	Cfg.JoyButtonBinds[ "Xbox" ][ 0 ] = Controls[ XWing::Control::THROTTLE_DOWN    ];  // A
	Cfg.JoyButtonBinds[ "Xbox" ][ 1 ] = Controls[ XWing::Control::MODE             ];  // B
	Cfg.JoyButtonBinds[ "Xbox" ][ 2 ] = Controls[ XWing::Control::THROTTLE_UP      ];  // X
	Cfg.JoyButtonBinds[ "Xbox" ][ 3 ] = Controls[ XWing::Control::WEAPON           ];  // Y
	Cfg.JoyButtonBinds[ "Xbox" ][ 4 ] = Controls[ XWing::Control::TARGET_CROSSHAIR ];  // LB
	Cfg.JoyButtonBinds[ "Xbox" ][ 5 ] = Controls[ XWing::Control::FIRE             ];  // RB
	Cfg.JoyButtonBinds[ "Xbox" ][ 6 ] = Controls[ XWing::Control::SCORES           ];  // Back
	Cfg.JoyButtonBinds[ "Xbox" ][ 7 ] = Controls[ XWing::Control::SHIELD_DIR       ];  // Start
	Cfg.JoyButtonBinds[ "Xbox" ][ 9 ] = Controls[ XWing::Control::LOOK_CENTER      ];  // Right Thumbstick Click
	
	Cfg.JoyHatBinds[ "Xbox" ][ 0 ][ SDL_HAT_UP    ] = Controls[ XWing::Control::TARGET_NEAREST_ENEMY    ];  // D-Pad Up
	Cfg.JoyHatBinds[ "Xbox" ][ 0 ][ SDL_HAT_DOWN  ] = Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ];  // D-Pad Down
	Cfg.JoyHatBinds[ "Xbox" ][ 0 ][ SDL_HAT_LEFT  ] = Controls[ XWing::Control::TARGET_PREV_ENEMY       ];  // D-Pad Left
	Cfg.JoyHatBinds[ "Xbox" ][ 0 ][ SDL_HAT_RIGHT ] = Controls[ XWing::Control::TARGET_NEXT_ENEMY       ];  // D-Pad Right
	
	if( Input.DeviceTypes.find("Pad") != Input.DeviceTypes.end() )
	{
		Cfg.JoyAxisBinds  [ "Pad" ] = Cfg.JoyAxisBinds  [ "Xbox" ];
		Cfg.JoyButtonBinds[ "Pad" ] = Cfg.JoyButtonBinds[ "Xbox" ];
		Cfg.JoyHatBinds   [ "Pad" ] = Cfg.JoyHatBinds   [ "Xbox" ];
	}
	
	if( Input.DeviceTypes.find("MFD") != Input.DeviceTypes.end() )
	{
		Cfg.JoyButtonBinds[ "MFD" ][  0 ] = Controls[ XWing::Control::TARGET_SYNC             ];  // Top #1
		Cfg.JoyButtonBinds[ "MFD" ][  1 ] = Controls[ XWing::Control::TARGET_GROUPMATE        ];  // Top #2
		Cfg.JoyButtonBinds[ "MFD" ][  2 ] = Controls[ XWing::Control::LOOK_CENTER             ];  // Top #3
		Cfg.JoyButtonBinds[ "MFD" ][  3 ] = Controls[ XWing::Control::TARGET_PREV             ];  // Top #4
		Cfg.JoyButtonBinds[ "MFD" ][  4 ] = Controls[ XWing::Control::TARGET_NEXT             ];  // Top #5
		Cfg.JoyButtonBinds[ "MFD" ][  5 ] = Controls[ XWing::Control::TARGET_NEWEST           ];  // Right #1
		Cfg.JoyButtonBinds[ "MFD" ][  6 ] = Controls[ XWing::Control::TARGET_PREV_FRIENDLY    ];  // Right #2
		Cfg.JoyButtonBinds[ "MFD" ][  7 ] = Controls[ XWing::Control::TARGET_NEXT_FRIENDLY    ];  // Right #3
		Cfg.JoyButtonBinds[ "MFD" ][  8 ] = Controls[ XWing::Control::TARGET_PREV_ENEMY       ];  // Right #4
		Cfg.JoyButtonBinds[ "MFD" ][  9 ] = Controls[ XWing::Control::TARGET_NEXT_ENEMY       ];  // Right #5
		Cfg.JoyButtonBinds[ "MFD" ][ 10 ] = Controls[ XWing::Control::TARGET_NEWEST_INCOMING  ];  // Bottom #5
		Cfg.JoyButtonBinds[ "MFD" ][ 11 ] = Controls[ XWing::Control::TARGET_NEAREST_ENEMY    ];  // Bottom #4
		Cfg.JoyButtonBinds[ "MFD" ][ 12 ] = Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ];  // Bottom #3
		Cfg.JoyButtonBinds[ "MFD" ][ 13 ] = Controls[ XWing::Control::TARGET_NEWEST_INCOMING  ];  // Bottom #2
		Cfg.JoyButtonBinds[ "MFD" ][ 14 ] = Controls[ XWing::Control::SHIELD_DIR              ];  // Bottom #1
		Cfg.JoyButtonBinds[ "MFD" ][ 15 ] = Controls[ XWing::Control::TARGET_NOTHING          ];  // Left #5
		Cfg.JoyButtonBinds[ "MFD" ][ 16 ] = Controls[ XWing::Control::WEAPON                  ];  // Left #4
		Cfg.JoyButtonBinds[ "MFD" ][ 17 ] = Controls[ XWing::Control::MODE                    ];  // Left #3
		Cfg.JoyButtonBinds[ "MFD" ][ 18 ] = Controls[ XWing::Control::TARGET_OBJECTIVE        ];  // Left #2
		Cfg.JoyButtonBinds[ "MFD" ][ 19 ] = Controls[ XWing::Control::SCORES                  ];  // Left #1
	}
	
	Cfg.JoyAxisBinds[ "Wheel" ][ 0 ] = Controls[ XWing::Control::ROLL           ];  // Steering Wheel
#if SDL_VERSION_ATLEAST(2,0,0)
	Cfg.JoyAxisBinds[ "Wheel" ][ 1 ] = Controls[ XWing::Control::THROTTLE       ];  // Accelerator Pedal
	Cfg.JoyAxisBinds[ "Wheel" ][ 2 ] = Controls[ XWing::Control::PITCH_UP       ];  // Brake Pedal
#else
	Cfg.JoyAxisBinds[ "Wheel" ][ 2 ] = Controls[ XWing::Control::THROTTLE       ];  // Accelerator Pedal
	Cfg.JoyAxisBinds[ "Wheel" ][ 3 ] = Controls[ XWing::Control::PITCH_UP       ];  // Brake Pedal
#endif
	Cfg.JoyAxisBinds[ "Wheel" ][ 4 ] = Controls[ XWing::Control::PITCH_DOWN     ];  // Clutch Pedal
	
	Cfg.JoyButtonBinds[ "Wheel" ][  0 ] = Controls[ XWing::Control::TARGET_NEAREST_ENEMY    ];  // Shift Red Button #1 (Left)
	Cfg.JoyButtonBinds[ "Wheel" ][  1 ] = Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ];  // Shift Red Button #2
	Cfg.JoyButtonBinds[ "Wheel" ][  2 ] = Controls[ XWing::Control::TARGET_PREV             ];  // Shift Red Button #3
	Cfg.JoyButtonBinds[ "Wheel" ][  3 ] = Controls[ XWing::Control::TARGET_NEXT             ];  // Shift Red Button #4 (Right)
	Cfg.JoyButtonBinds[ "Wheel" ][  4 ] = Controls[ XWing::Control::YAW_RIGHT               ];  // Wheel Right Paddle
	Cfg.JoyButtonBinds[ "Wheel" ][  5 ] = Controls[ XWing::Control::YAW_LEFT                ];  // Wheel Left Paddle
	Cfg.JoyButtonBinds[ "Wheel" ][  6 ] = Controls[ XWing::Control::FIRE                    ];  // Wheel Top Right Button
	Cfg.JoyButtonBinds[ "Wheel" ][  7 ] = Controls[ XWing::Control::TARGET_CROSSHAIR        ];  // Wheel Top Left Button
	Cfg.JoyButtonBinds[ "Wheel" ][  8 ] = Controls[ XWing::Control::ROLL_RIGHT              ];  // 1st Gear
	Cfg.JoyButtonBinds[ "Wheel" ][  9 ] = Controls[ XWing::Control::ROLL_LEFT               ];  // 2nd Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 10 ] = Controls[ XWing::Control::PITCH_DOWN              ];  // 3rd Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 11 ] = Controls[ XWing::Control::PITCH_UP                ];  // 4th Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 12 ] = Controls[ XWing::Control::ROLL_LEFT               ];  // 5th Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 13 ] = Controls[ XWing::Control::ROLL_RIGHT              ];  // 6th Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 14 ] = Controls[ XWing::Control::TARGET_SYNC             ];  // Reverse Gear
	Cfg.JoyButtonBinds[ "Wheel" ][ 15 ] = Controls[ XWing::Control::SEAT_COCKPIT            ];  // Shift Top Black Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 16 ] = Controls[ XWing::Control::SEAT_GUNNER1            ];  // Shift Left Black Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 17 ] = Controls[ XWing::Control::LOOK_CENTER             ];  // Shift Bottom Black Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 18 ] = Controls[ XWing::Control::SEAT_GUNNER2            ];  // Shift Right Black Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 19 ] = Controls[ XWing::Control::MODE                    ];  // Wheel Middle Right Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 20 ] = Controls[ XWing::Control::SHIELD_DIR              ];  // Wheel Middle Left Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 21 ] = Controls[ XWing::Control::WEAPON                  ];  // Wheel Bottom Right Button
	Cfg.JoyButtonBinds[ "Wheel" ][ 22 ] = Controls[ XWing::Control::LOOK_CENTER             ];  // Wheel Bottom Left Button
	
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_UP    ] = Controls[ XWing::Control::GLANCE_UP    ];  // Shift Hat Up
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_RIGHT ] = Controls[ XWing::Control::GLANCE_RIGHT ];  // Shift Hat Right
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_DOWN  ] = Controls[ XWing::Control::GLANCE_BACK  ];  // Shift Hat Down
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_LEFT  ] = Controls[ XWing::Control::GLANCE_LEFT  ];  // Shift Hat Left
}


void XWingGame::SetDefaults( void )
{
	std::string rebel_mission  = Cfg.SettingAsString( "rebel_mission",  "rebel0"  );
	std::string empire_mission = Cfg.SettingAsString( "empire_mission", "empire0" );
	
	RaptorGame::SetDefaults();
	
	if( Cfg.Settings[ "name" ] == "Name" )
		Cfg.Settings[ "name" ] = "Rookie One";
	
	Cfg.Settings[ "host_address" ] = "www.raptor007.com";
	
	Cfg.Settings[ "view" ] = "auto";
	Cfg.Settings[ "spectator_view" ] = "auto";
	
	Cfg.Settings[ "g_bg" ] = "true";
	Cfg.Settings[ "g_stars" ] = "0";
	Cfg.Settings[ "g_debris" ] = "500";
	Cfg.Settings[ "g_engine_glow" ] = "true";
	Cfg.Settings[ "g_effects" ] = "1";
	Cfg.Settings[ "g_asteroid_lod" ] = "1";
	Cfg.Settings[ "g_deathstar_trench" ] = "4";
	Cfg.Settings[ "g_deathstar_surface" ] = "4";
	Cfg.Settings[ "g_crosshair_thickness" ] = "1.5";
	Cfg.Settings[ "g_shader_blastpoints" ] = "20";
	Cfg.Settings[ "g_shader_blastpoint_quality" ] = "2";
	
	Cfg.Settings[ "s_volume" ] = "0.2";
	Cfg.Settings[ "s_effect_volume" ] = "0.5";
	Cfg.Settings[ "s_engine_volume" ] = "0.9";
	Cfg.Settings[ "s_music_volume" ] = "0.8";
	Cfg.Settings[ "s_menu_music" ] = "true";
	Cfg.Settings[ "s_game_music" ] = "true";
	Cfg.Settings[ "s_imuse" ] = "false";
	
	Cfg.Settings[ "vr_sway" ] = "false";
	Cfg.Settings[ "vr_messages" ] = "false";
	
	Cfg.Settings[ "joy_enable" ] = "true";
	Cfg.Settings[ "joy_hide_binds" ] = "true";
	
	Cfg.Settings[ "mouse_mode" ] = "disabled";
	Cfg.Settings[ "mouse_invert" ] = "true";
	Cfg.Settings[ "mouse_smooth" ] = "0.25";
	
	Cfg.Settings[ "swap_yaw_roll" ] = "false";
	Cfg.Settings[ "turret_invert" ] = "false";
	
	Cfg.Settings[ "net_zerolag" ] = "2";
	
	Cfg.Settings[ "rebel_mission"  ] = rebel_mission;
	Cfg.Settings[ "empire_mission" ] = empire_mission;
	
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
	bool safemode = false;
	for( int i = 1; i < argc; i ++ )
	{
		if( strcasecmp( argv[ i ], "-safe" ) == 0 )
			safemode = true;
	}
	
	if( screensaver )
	{
		Cfg.Settings[ "g_vsync" ] = Cfg.SettingAsBool( "screensaver_vsync", true );
		Cfg.Settings[ "s_menu_music" ] = "false";
		Cfg.Settings[ "s_game_music" ] = "false";
		Cfg.Settings[ "saitek_enable" ] = "false";
		Cfg.Settings[ "view" ] = Cfg.SettingAsString( "screensaver_view", "cycle" );
		Cfg.Settings[ "spectator_view" ] = "auto";
		int maxfps = Cfg.SettingAsInt( "maxfps", 60 );
		Cfg.Settings[ "sv_netrate" ] = Cfg.Settings[ "sv_maxfps" ] = (maxfps > 60) ? Num::ToString(maxfps) : "60";
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
	Res.GetSound("damage_hull.wav");
	Res.GetSound("damage_shield.wav");
	Res.GetSound("hit_hull.wav");
	Res.GetSound("hit_shield.wav");
	Res.GetSound("jump_in.wav");
	
	bool screensaver = Cfg.SettingAsBool("screensaver");
	if( screensaver )
	{
		Res.GetAnimation( Cfg.SettingAsString( "screensaver_bg", "stars" ) + std::string(".ani") );
		if( Cfg.SettingAsBool( "screensaver_asteroids", true ) )
			Res.GetModel("asteroid.obj");
	}
	else
	{
		Res.GetAnimation("nebula.ani");
		Res.GetAnimation("stars.ani");
		Res.GetAnimation("bg_lobby.ani");
		
		Res.GetAnimation("special/health.ani");
		Res.GetAnimation("special/target.ani");
		Res.GetAnimation("special/intercept.ani");
		Res.GetAnimation("special/throttle.ani");
		
		Res.GetAnimation("shields_c.ani");
		Res.GetAnimation("shields_f.ani");
		Res.GetAnimation("shields_r.ani");
		
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
		Res.GetSound("beep_aim.wav");
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
		Res.GetSound("locking.wav");
		Res.GetSound("locked.wav");
		Res.GetSound("lock_warn.wav");
		Res.GetSound("incoming.wav");
		Res.GetSound("checkpoint.wav");
		Res.GetSound("chat.wav");
		Res.GetSound("eject.wav");
		
		Res.GetSound("damage_hull_2.wav");
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
					screensaver_ships.insert( Cfg.SettingAsString( "screensaver_rebel_bomber",  "Y/W" ) );
					screensaver_ships.insert( Cfg.SettingAsString( "screensaver_empire_bomber", "T/F" ) );
				}
			}
			
			std::string spawn = Cfg.SettingAsString( "screensaver_spawn", "T/A,YT1300" );
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
		if( (bind_iter->second == Controls[ XWing::Control::THROTTLE_33       ]) // Spectate Prev
		||  (bind_iter->second == Controls[ XWing::Control::THROTTLE_66       ]) // Spectate Next
		||  (bind_iter->second == Controls[ XWing::Control::LOOK_CENTER       ])
		||  (bind_iter->second == Controls[ XWing::Control::LOOK_UP           ])
		||  (bind_iter->second == Controls[ XWing::Control::LOOK_DOWN         ])
		||  (bind_iter->second == Controls[ XWing::Control::LOOK_LEFT         ])
		||  (bind_iter->second == Controls[ XWing::Control::LOOK_RIGHT        ])
		||  (bind_iter->second == Controls[ XWing::Control::LOOK_UP_LEFT      ])
		||  (bind_iter->second == Controls[ XWing::Control::LOOK_UP_RIGHT     ])
		||  (bind_iter->second == Controls[ XWing::Control::LOOK_DOWN_LEFT    ])
		||  (bind_iter->second == Controls[ XWing::Control::LOOK_DOWN_RIGHT   ])
		||  (bind_iter->second == Controls[ XWing::Control::GLANCE_UP         ])
		||  (bind_iter->second == Controls[ XWing::Control::GLANCE_BACK       ])
		||  (bind_iter->second == Controls[ XWing::Control::GLANCE_LEFT       ])
		||  (bind_iter->second == Controls[ XWing::Control::GLANCE_RIGHT      ])
		||  (bind_iter->second == Controls[ XWing::Control::GLANCE_UP_LEFT    ])
		||  (bind_iter->second == Controls[ XWing::Control::GLANCE_UP_RIGHT   ])
		||  (bind_iter->second == Controls[ XWing::Control::GLANCE_BACK_LEFT  ])
		||  (bind_iter->second == Controls[ XWing::Control::GLANCE_BACK_RIGHT ])
		||  (bind_iter->second == Controls[ XWing::Control::PAUSE ])
		|| ((bind_iter->second >= Controls[ XWing::Control::VIEW_COCKPIT ]) && (bind_iter->second <= Controls[ XWing::Control::VIEW_INSTRUMENTS ])) )
			screensaver_layer->IgnoreKeys.insert( bind_iter->first );
	}
	Layers.Add( screensaver_layer );
}


void XWingGame::Update( double dt )
{
	// Apply time_scale to various timers.
	
	dt *= Data.TimeScale;
	
	RoundTimer.SetTimeScale( Data.TimeScale );
	
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
	std::list<Ship*> ships;
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
#ifdef DEATHSTAR_GRAVITY
		else if( obj_iter->second->Type() == XWing::Object::DEATH_STAR )
			deathstar = (DeathStar*) obj_iter->second;
#endif
	}
	
	Ship *observed_ship = (Ship*) Data.GetObject( ObservedShipID );
	if( ! observed_ship )
		observed_ship = my_ship;
	
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
		else if( (Cfg.SettingAsString("mouse_mode") == "look") && (! Head.VR) )
		{
			LookYaw = ( (fabs(mouse_x_percent) <= 1.) ? mouse_x_percent : Num::Sign(mouse_x_percent) ) * 180.;
			LookPitch = ( (fabs(mouse_y_percent) <= 1.) ? mouse_y_percent : Num::Sign(mouse_y_percent) ) * -90.;
			ThumbstickLook = false;
		}
	}
	
	// Analog Axes
	roll  = Num::Clamp( roll  + Input.ControlTotal(Controls[ XWing::Control::ROLL  ]) + Input.ControlTotal(Controls[ XWing::Control::ROLL_INVERTED  ]) + Input.ControlTotal(Controls[ XWing::Control::ROLL_RIGHT ]) - Input.ControlTotal(Controls[ XWing::Control::ROLL_LEFT ]), -1., 1. );
	pitch = Num::Clamp( pitch + Input.ControlTotal(Controls[ XWing::Control::PITCH ]) + Input.ControlTotal(Controls[ XWing::Control::PITCH_INVERTED ]) + Input.ControlTotal(Controls[ XWing::Control::PITCH_UP ]) - Input.ControlTotal(Controls[ XWing::Control::PITCH_DOWN ]), -1., 1. );
	yaw   = Num::Clamp( yaw   + Input.ControlTotal(Controls[ XWing::Control::YAW   ]) + Input.ControlTotal(Controls[ XWing::Control::YAW_INVERTED   ]) + Input.ControlTotal(Controls[ XWing::Control::YAW_RIGHT ]) - Input.ControlTotal(Controls[ XWing::Control::YAW_LEFT ]), -1., 1. );
	if( Input.HasControlAxis(Controls[ XWing::Control::THROTTLE ]) || Input.HasControlAxis(Controls[ XWing::Control::THROTTLE_INVERTED ]) )
		throttle = Num::Clamp( Input.ControlTotal(Controls[ XWing::Control::THROTTLE ]) + Input.ControlTotal(Controls[ XWing::Control::THROTTLE_INVERTED ]), 0., 1. );
	double look_x = Num::Clamp( Input.ControlValue(Controls[ XWing::Control::LOOK_X ]) + Input.ControlValue(Controls[ XWing::Control::LOOK_X_INVERTED ]), -1., 1. );
	double look_y = Num::Clamp( Input.ControlValue(Controls[ XWing::Control::LOOK_Y ]) + Input.ControlValue(Controls[ XWing::Control::LOOK_Y_INVERTED ]), -1., 1. );
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
	}
	
	// Held Digital Buttons
	double glance_x = Num::Clamp( Input.ControlTotal(Controls[ XWing::Control::GLANCE_RIGHT ]) + Input.ControlTotal(Controls[ XWing::Control::GLANCE_UP_RIGHT  ]) + Input.ControlTotal(Controls[ XWing::Control::GLANCE_BACK_RIGHT ]) - Input.ControlTotal(Controls[ XWing::Control::GLANCE_LEFT ]) - Input.ControlTotal(Controls[ XWing::Control::GLANCE_UP_LEFT ]) - Input.ControlTotal(Controls[ XWing::Control::GLANCE_BACK_LEFT ]), -1., 1. );
	double glance_y = Num::Clamp( Input.ControlTotal(Controls[ XWing::Control::GLANCE_BACK  ]) + Input.ControlTotal(Controls[ XWing::Control::GLANCE_BACK_LEFT ]) + Input.ControlTotal(Controls[ XWing::Control::GLANCE_BACK_RIGHT ]) - Input.ControlTotal(Controls[ XWing::Control::GLANCE_UP   ]) - Input.ControlTotal(Controls[ XWing::Control::GLANCE_UP_LEFT ]) - Input.ControlTotal(Controls[ XWing::Control::GLANCE_UP_RIGHT  ]), -1., 1. );
	if( glance_x || glance_y )
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
	if( Input.ControlPressed(Controls[ XWing::Control::ROLL_RIGHT ]) || Input.ControlPressed(Controls[ XWing::Control::ROLL_LEFT ]) )
		roll = Num::Clamp( Input.ControlTotal(Controls[ XWing::Control::ROLL_RIGHT ]) - Input.ControlTotal(Controls[ XWing::Control::ROLL_LEFT ]), -1., 1. );
	if( Input.ControlPressed(Controls[ XWing::Control::PITCH_UP ]) || Input.ControlPressed(Controls[ XWing::Control::PITCH_DOWN ]) )
		pitch = Num::Clamp( Input.ControlTotal(Controls[ XWing::Control::PITCH_UP ]) - Input.ControlTotal(Controls[ XWing::Control::PITCH_DOWN ]), -1., 1. );
	if( Input.ControlPressed(Controls[ XWing::Control::YAW_RIGHT ]) || Input.ControlPressed(Controls[ XWing::Control::YAW_LEFT ]) )
		yaw = Num::Clamp( Input.ControlTotal(Controls[ XWing::Control::YAW_RIGHT ]) - Input.ControlTotal(Controls[ XWing::Control::YAW_LEFT ]), -1., 1. );
	if( Input.ControlPressed(Controls[ XWing::Control::THROTTLE_UP ]) || Input.ControlPressed(Controls[ XWing::Control::THROTTLE_DOWN ]) )
	{
		double throttle_change = Num::Clamp( Input.ControlTotal(Controls[ XWing::Control::THROTTLE_UP ]) - Input.ControlTotal(Controls[ XWing::Control::THROTTLE_DOWN ]), -1., 1. );
		throttle += throttle_change * FrameTime / 2.;
	}
	if( Input.ControlPressed(Controls[ XWing::Control::THROTTLE_100 ]) )
		throttle = 1.;
	else if( Input.ControlPressed(Controls[ XWing::Control::THROTTLE_66 ]) )
		throttle = 0.6667;
	else if( Input.ControlPressed(Controls[ XWing::Control::THROTTLE_33 ]) )
		throttle = 0.3333;
	else if( Input.ControlPressed(Controls[ XWing::Control::THROTTLE_0 ]) )
		throttle = 0.;
	if( Input.ControlPressed(Controls[ XWing::Control::FIRE ]) )
		firing = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_NOTHING ]) )
		target_nothing = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_CROSSHAIR ]) )
		target_crosshair = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_NEAREST_ENEMY ]) )
		target_nearest_enemy = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ]) )
		target_nearest_attacker = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_TARGET_ATTACKER ]) )
		target_target_attacker = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_NEWEST_INCOMING ]) )
		target_newest_incoming = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_OBJECTIVE ]) )
		target_objective = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_DOCKABLE ]) )
		target_dockable = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_NEWEST ]) )
		target_newest = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_GROUPMATE ]) )
		target_groupmate = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_SYNC ]) )
		target_sync = true;
	if( Input.ControlPressed(Controls[ XWing::Control::LOOK_UP ]) || Input.ControlPressed(Controls[ XWing::Control::LOOK_UP_LEFT ]) || Input.ControlPressed(Controls[ XWing::Control::LOOK_UP_RIGHT ]) )
	{
		LookPitch += 90. * FrameTime;
		ThumbstickLook = false;
	}
	if( Input.ControlPressed(Controls[ XWing::Control::LOOK_DOWN ]) || Input.ControlPressed(Controls[ XWing::Control::LOOK_DOWN_LEFT ]) || Input.ControlPressed(Controls[ XWing::Control::LOOK_DOWN_RIGHT ]) )
	{
		LookPitch -= 90. * FrameTime;
		ThumbstickLook = false;
	}
	if( Input.ControlPressed(Controls[ XWing::Control::LOOK_LEFT ]) || Input.ControlPressed(Controls[ XWing::Control::LOOK_UP_LEFT ]) || Input.ControlPressed(Controls[ XWing::Control::LOOK_DOWN_LEFT ]) )
	{
		LookYaw -= 90. * FrameTime;
		ThumbstickLook = false;
	}
	if( Input.ControlPressed(Controls[ XWing::Control::LOOK_RIGHT ]) || Input.ControlPressed(Controls[ XWing::Control::LOOK_UP_RIGHT ]) || Input.ControlPressed(Controls[ XWing::Control::LOOK_DOWN_RIGHT ]) )
	{
		LookYaw += 90. * FrameTime;
		ThumbstickLook = false;
	}
	if( Input.ControlPressed(Controls[ XWing::Control::LOOK_CENTER ]) )
	{
		LookPitch = 0.;
		LookYaw   = 0.;
		ThumbstickLook = false;
		Head.Recenter();
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
	std::map<int,Shot*> fire_shots;
	
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
				if( ZeroLagServer )
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
				if( zerolag && firing && my_ship->SelectedWeapon && ((zerolag >= 2) || (my_ship->MaxAmmo() < 0)) && (my_ship->LastFired() >= my_ship->ShotDelay()) )
				{
					fire_shots = my_ship->NextShots();
					if( ZeroLagServer )
						my_ship->PredictedShots ++;
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
				
				ejecting = Input.ControlPressed(Controls[ XWing::Control::EJECT ]);
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
					my_ship->Fwd = my_ship->MotionVector.Unit();
					my_ship->FixVectors();
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
			ahead.MoveAlong( &my_fwd, 5000. );
			
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
			
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				if( (*ship_iter)->ID != my_ship_id )
				{
					Ship *ship = *ship_iter;
					
					if( ship->Health <= 0. )
						continue;
					if( (ship->JumpProgress < 1.) || ship->JumpedOut )
						continue;
					
					if( ship->ComplexCollisionDetection() )
					{
						std::string hit;
						double dist = ship->Shape.DistanceFromLine( ship, NULL, NULL, &hit, ship->Exploded(), ship->ExplosionSeed(), my_pos, &ahead );
						if( (dist < (ship->Shape.GetTriagonal() * 0.01)) && ! hit.empty() )
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
			
			if( best_ship )
				subsystem = best_hit.empty() ? 0 : best_ship->SubsystemID( best_hit );
			
			if( (target_id != id) || (target_subsystem != subsystem) )
			{
				target_id = id;
				target_subsystem = subsystem;
				beep = "beep.wav";
				target_wait = my_pos->Lifetime.ElapsedSeconds() + 0.05;
			}
		}
		else if( target_nearest_enemy || target_nearest_attacker || target_target_attacker )
		{
			double best = 0.;
			uint8_t category = ShipClass::CATEGORY_TARGET;
			uint32_t id = 0;
			
			// FIXME: Dirty hack to fix error beep being played after successful retargeting.
			if( target_target_attacker )
			{
				Ship *target_ship = (Ship*) Data.GetObject( target_id );
				if( target_ship && (target_ship->Type() == XWing::Object::SHIP) && (target_ship->Team != my_team) )
					error_beep_allowed = false;
			}
			
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// FIXME: Dirty hack to make sure we don't rapidly cycle in FFA.
				if( target_target_attacker && ! error_beep_allowed )
					break;
				
				if( (*ship_iter)->ID != my_ship_id )
				{
					Ship *ship = *ship_iter;
					
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
						
						// FIXME: Optimize by gathering victim data before this loop.
						const Ship *victim = target_target_attacker ? (Ship*) Data.GetObject( target_id ) : my_ship;
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
					
					// Prefer non-capital ship targets.
					uint8_t ship_category = ship->Category();
					if( (ship_category > category) && (ship_category >= ShipClass::CATEGORY_CAPITAL) )
						continue;
					
					// Prefer non-capital ship targets.
					if( (category >= ShipClass::CATEGORY_CAPITAL) && (ship_category < category) )
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
			
			if( (GameType == XWing::GameType::TEAM_RACE) || (GameType == XWing::GameType::FFA_RACE) )
				id = my_checkpoint;
			else
			{
				double best_dist = 0.;
				Ship *best = NULL;
				
				for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					if( (*ship_iter)->ID != my_ship_id )
					{
						Ship *ship = *ship_iter;
						
						if( ship->Health <= 0. )
							continue;
						if( (ship->JumpProgress < 1.) || ship->JumpedOut )
							continue;
						if( my_team && (ship->Team == my_team) && (ship->Category() != ShipClass::CATEGORY_TARGET) && (GameType != XWing::GameType::CAPITAL_SHIP_HUNT) )
							continue;
						if( (ship->Group != 255) && ! ship->IsMissionObjective )
							continue;
						if( best && best->IsMissionObjective && ! ship->IsMissionObjective )
							continue;
						
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
			Ship *best = NULL;
			
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				if( (*ship_iter)->ID != my_ship_id )
				{
					Ship *ship = *ship_iter;
					
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
			uint8_t my_group = ship ? ship->Group : 0;
			Player *my_player = Data.GetPlayer( PlayerID );
			if( my_player )
				my_group = my_player->PropertyAsInt("group",my_group);
			
			if( ship && my_team )
			{
				for( std::list<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					if( (*ship_iter != ship) && ((*ship_iter)->Team == my_team) && ((*ship_iter)->Group == my_group) && ((*ship_iter)->Health > 0.) )
					{
						// When not in a flight group, target other players on your team not in a flight group, or friendly AI ships of same category.
						if( (! my_group) && ship && (ship->Category() != (*ship_iter)->Category()) && ! (*ship_iter)->PlayerID )
							continue;
						
						// Primarily prefer player ships.
						if( best_player && ! (*ship_iter)->PlayerID )
							continue;
						
						// Secondarily prefer nearer ships.
						double dist = my_pos->Dist(*ship_iter);
						if( (dist < best_dist) || ((*ship_iter)->PlayerID && ! best_player) || ! id )
						{
							id = (*ship_iter)->ID;
							best_dist = dist;
							best_player = (*ship_iter)->PlayerID;
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
					
					for( std::list<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
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
			
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				if( (*ship_iter)->ID != my_ship_id )
				{
					Ship *ship = *ship_iter;
					
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
	for( std::map<int,Shot*>::iterator shot_iter = fire_shots.begin(); shot_iter != fire_shots.end(); shot_iter ++ )
	{
		shot_iter->second->Predicted = true;
		Data.AddObject( shot_iter->second );
		ClientShots[ shot_iter->second->ShotType ][ shot_iter->first ].push_back( shot_iter->second );
	}
	
	// Clean up any lasers that never synchronized.
	for( std::map< uint8_t, std::map< int, std::deque<Shot*> > >::iterator shot_type_iter = ClientShots.begin(); shot_type_iter != ClientShots.end(); shot_type_iter ++ )
	{
		for( std::map< int, std::deque<Shot*> >::iterator shot_weapon_iter = shot_type_iter->second.begin(); shot_weapon_iter != shot_type_iter->second.end(); shot_weapon_iter ++ )
		{
			while( shot_weapon_iter->second.size() && (shot_weapon_iter->second.front()->Lifetime.ElapsedSeconds() >= shot_weapon_iter->second.front()->MaxLifetime()) )
			{
				if( Cfg.SettingAsBool("debug") )
					Console.Print( std::string("DEBUG: ZeroLag Shot ") + Num::ToString( (int) shot_weapon_iter->second.front()->ID ) + std::string(" never synchronized."), TextConsole::MSG_ERROR );
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
	if( State < XWing::State::FLYING )
		return RaptorGame::HandleEvent( event );
	
	uint8_t control = Input.EventBound( event );
	
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
	if( control == Controls[ XWing::Control::THROTTLE_33 ] )       // SDLK_LEFTBRACKET
		observe_prev = true;
	else if( control == Controls[ XWing::Control::THROTTLE_66 ] )  // SDLK_RIGHTBRACKET
		observe_next = true;
	else if( control == Controls[ XWing::Control::WEAPON ] )
		weapon_next = true;
	else if( control == Controls[ XWing::Control::MODE ] )
		firing_mode_next = true;
	else if( control == Controls[ XWing::Control::SHIELD_DIR ] )
		shield_shunt = true;
	else if( control == Controls[ XWing::Control::TARGET_PREV ] )
		target_prev = true;
	else if( control == Controls[ XWing::Control::TARGET_NEXT ] )
		target_next = true;
	else if( control == Controls[ XWing::Control::TARGET_PREV_ENEMY ] )
	{
		target_prev_enemy = true;
		if( Cfg.SettingAsString("spectator_view") != "instruments" )
			observe_prev = true;
	}
	else if( control == Controls[ XWing::Control::TARGET_NEXT_ENEMY ] )
	{
		target_next_enemy = true;
		if( Cfg.SettingAsString("spectator_view") != "instruments" )
			observe_next = true;
	}
	else if( control == Controls[ XWing::Control::TARGET_PREV_FRIENDLY ] )
		target_prev_friendly = true;
	else if( control == Controls[ XWing::Control::TARGET_NEXT_FRIENDLY ] )
		target_next_friendly = true;
	else if( control == Controls[ XWing::Control::TARGET_PREV_PLAYER ] )
		target_prev_player = true;
	else if( control == Controls[ XWing::Control::TARGET_NEXT_PLAYER ] )
		target_next_player = true;
	else if( control == Controls[ XWing::Control::TARGET_PREV_SUBSYSTEM ] )
	{
		target_prev_subsystem = true;
		observe_prev = true;
	}
	else if( control == Controls[ XWing::Control::TARGET_NEXT_SUBSYSTEM ] )
	{
		target_next_subsystem = true;
		observe_next = true;
	}
	else if( control == Controls[ XWing::Control::TARGET1 ] )
	{
		target_stored = &(StoredTargets[ 0 ]);
		subsystem_stored = &(StoredSubsystems[ 0 ]);
	}
	else if( control == Controls[ XWing::Control::TARGET2 ] )
	{
		target_stored = &(StoredTargets[ 1 ]);
		subsystem_stored = &(StoredSubsystems[ 1 ]);
	}
	else if( control == Controls[ XWing::Control::TARGET3 ] )
	{
		target_stored = &(StoredTargets[ 2 ]);
		subsystem_stored = &(StoredSubsystems[ 2 ]);
	}
	else if( control == Controls[ XWing::Control::TARGET4 ] )
	{
		target_stored = &(StoredTargets[ 3 ]);
		subsystem_stored = &(StoredSubsystems[ 3 ]);
	}
	else if( control == Controls[ XWing::Control::SEAT_COCKPIT ] )
		change_seat = 1;
	else if( control == Controls[ XWing::Control::SEAT_GUNNER1 ] )
		change_seat = 2;
	else if( control == Controls[ XWing::Control::SEAT_GUNNER2 ] )
		change_seat = 3;
	else if( control == Controls[ XWing::Control::CHEWIE_TAKE_THE_WHEEL ] )
		chewie_take_the_wheel = true;
	else if( (control >= Controls[ XWing::Control::VIEW_COCKPIT ]) && (control <= Controls[ XWing::Control::VIEW_INSTRUMENTS ]) )
		change_view = true;
	else if( control == Controls[ XWing::Control::PAUSE ] )
		pause = true;
	else
		return RaptorGame::HandleEvent( event );
	
	
	if( change_seat )
	{
		Packet packet = Packet( XWing::Packet::CHANGE_SEAT );
		packet.AddUChar( change_seat - 1 );
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
		if( Data.TimeScale < 1. )
		{
			Packet info = Packet( Raptor::Packet::INFO );
			info.AddUShort( 1 );
			info.AddString( "time_scale" );
			info.AddString( "1" );
			Raptor::Game->Net.Send( &info );
		}
		else if( Raptor::Server->IsRunning() )
		{
			Packet info = Packet( Raptor::Packet::INFO );
			info.AddUShort( 1 );
			info.AddString( "time_scale" );
			info.AddString( "0.0001" );
			Raptor::Game->Net.Send( &info );
		}
		return true;
	}
	
	
	// Build a list of all ships because we'll refer to it often, and find the player's ship and/or turret.
	std::list<Ship*> ships;
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
	}
	
	
	if( change_view )
	{
		bool alive = (my_ship && (ObservedShipID == my_ship->ID)) || my_turret;
		std::string var = (alive || Cfg.SettingAsBool("screensaver") || (Cfg.SettingAsString("view") != "auto")) ? "view" : "spectator_view";
		
		std::string value = "auto";
		if( control == Controls[ XWing::Control::VIEW_COCKPIT ] )
			value = alive ? "auto" : "cockpit";
		else if( control == Controls[ XWing::Control::VIEW_CROSSHAIR ] )
			value = "crosshair";
		else if( control == Controls[ XWing::Control::VIEW_CHASE ] )
			value = "chase";
		else if( control == Controls[ XWing::Control::VIEW_PADLOCK ] )
			value = "padlock";
		else if( control == Controls[ XWing::Control::VIEW_STATIONARY ] )
			value = "stationary";
		else if( control == Controls[ XWing::Control::VIEW_CINEMA ] )
			value = "cinema";
		else if( control == Controls[ XWing::Control::VIEW_FIXED ] )
			value = "fixed";
		else if( control == Controls[ XWing::Control::VIEW_SELFIE ] )
			value = "selfie";
		else if( control == Controls[ XWing::Control::VIEW_GUNNER ] )
			value = "gunner";
		else if( control == Controls[ XWing::Control::VIEW_CYCLE ] )
			value = "cycle";
		else if( control == Controls[ XWing::Control::VIEW_INSTRUMENTS ] )
			value = "instruments";
		
		if( value == Cfg.SettingAsString(var) )  // View buttons toggle.
			value = "auto";
		
		Cfg.Settings[ var ] = value;
		if( var != "view" )
			Cfg.Settings[ "view" ] = "auto";
		
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
					my_ship->SetShieldPos( Ship::SHIELD_FRONT );
					Snd.Play( Res.GetSound("beep_sf.wav") );
				}
				else if( my_ship->ShieldPos == Ship::SHIELD_FRONT )
				{
					my_ship->SetShieldPos( Ship::SHIELD_REAR );
					Snd.Play( Res.GetSound("beep_sr.wav") );
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
		bool found = false, find_last = false;
		
		for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
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
			
			Ship *prev_ship = observed_ship;
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
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
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
		
		std::map<uint32_t,Ship*> potential_targets;
		for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
		{
			if( (*ship_iter)->ID != my_ship_id )
			{
				Ship *ship = *ship_iter;
				
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
		
		std::map<uint32_t,Ship*>::iterator target_iter = potential_targets.find( target_id );
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
		if( Input.ControlPressed( Controls[ XWing::Control::TARGET_STORE ] ) )
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
		beep = NULL;
	
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
	
	if( type == TextConsole::MSG_CHAT )
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
		double old_shields = 0.;
		double old_subsystem_health = 0.;
		double ship_dx = 0., ship_dy = 0., ship_dz = 0.;
		
		GameObject *ship_obj = Data.GetObject( ship_id );
		if( ship_obj && (ship_obj->Type() == XWing::Object::SHIP) )
		{
			ship = (Ship*) ship_obj;
			old_health = ship->Health;
			old_shields = ship->ShieldF + ship->ShieldR;
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
			bool shielded = false;
			double bp_radius = 4.;
			double bp_time = 0.;
			
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
						if( (ship->Health > 0.) && (ship->PlayerID == Raptor::Game->PlayerID) )
							Snd.Play( Res.GetSound("damage_hull_2.wav") );
					}
					else if( ship->ShieldF + ship->ShieldR < old_shields )
						sound = Res.GetSound("damage_shield.wav");
					loudness = 2.;
				}
				else
				{
					if( (ship->Health < old_health) || (subsystem_health < old_subsystem_health) )
						sound = Res.GetSound("hit_hull.wav");
					else if( ship->ShieldF + ship->ShieldR < old_shields )
						sound = Res.GetSound("hit_shield.wav");
				}
				if( sound )
					Snd.PlayAt( sound, x, y, z, loudness );
				
				double shield_damage = old_shields - (ship->ShieldF + ship->ShieldR);
				double hull_damage = old_health - ship->Health;
				double shield_scale = ((shot_type == Shot::TYPE_MISSILE) || (shot_type == Shot::TYPE_TORPEDO)) ? 1. : 0.3;
				double knock_scale = (shield_damage * shield_scale + hull_damage) / 20.;
				Vec3D knock( shot_dx - ship_dx, shot_dy - ship_dy, shot_dz - ship_dz );
				if( knock.Length() > 1. )
					knock.ScaleTo( 1. );
				ship->KnockCockpit( &knock, knock_scale );
				
				shielded = (subsystem_health >= old_subsystem_health) && ! hull_damage;
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
					Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), shielded ? 3.5 : 4., NULL, 0., &pos, &motion_vec, 0., 7. ) );
				}
				bp_radius = shielded ? 2. : 4.;
			}
			
			if( BlastPoints && bp_radius )
			{
				if( !( shielded || ship->ComplexCollisionDetection() ) )
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
		RoundTimer.Reset( packet->NextFloat() );
		return true;
	}
	
	else if( type == XWing::Packet::CHECKPOINT )
	{
		uint32_t ship_id = packet->NextUInt();
		uint32_t next_checkpoint = packet->NextUInt();
		
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
				std::set<uint32_t> features;
				uint8_t feature_count = packet->NextUChar();
				for( uint8_t i = 0; i < feature_count; i ++ )
					features.insert( packet->NextUInt() );
				
				ZeroLagServer = features.count( XWing::ServerFeature::ZERO_LAG );
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
		
		if( next_mission.length() && Str::BeginsWith( current_mission, "rebel" ) && Str::EqualsInsensitive( Cfg.SettingAsString("rebel_mission"), current_mission ) )
			Cfg.Settings["rebel_mission"] = next_mission;
		else if( next_mission.length() && Str::BeginsWith( current_mission, "empire" ) && Str::EqualsInsensitive( Cfg.SettingAsString("empire_mission"), current_mission ) )
			Cfg.Settings["empire_mission"] = next_mission;
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
	ZeroLagServer = false;
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
}


void XWingGame::ShowLobby( void )
{
	if( Cfg.SettingAsBool("screensaver") )
	{
		if( Raptor::Server->IsRunning() && (State <= XWing::State::LOBBY) )
		{
			// FIXME: Should this send an INFO packet instead of directly modifying server data from the client thread?
			Raptor::Server->Data.Lock.Lock();
			Raptor::Server->Data.Properties["gametype"]              = Cfg.SettingAsString( "screensaver_gametype",        "fleet" );
			Raptor::Server->Data.Properties["dm_kill_limit"]         = Cfg.SettingAsString( "screensaver_kill_limit",      "0"     );
			Raptor::Server->Data.Properties["tdm_kill_limit"]        = Cfg.SettingAsString( "screensaver_kill_limit",      "0"     );
			Raptor::Server->Data.Properties["team_race_checkpoints"] = Cfg.SettingAsString( "screensaver_checkpoints",     "0"     );
			Raptor::Server->Data.Properties["ffa_race_checkpoints"]  = Cfg.SettingAsString( "screensaver_checkpoints",     "0"     );
			Raptor::Server->Data.Properties["hunt_time_limit"]       = Cfg.SettingAsString( "screensaver_time_limit",      "0"     );
			Raptor::Server->Data.Properties["yavin_time_limit"]      = Cfg.SettingAsString( "screensaver_time_limit",      "0"     );
			Raptor::Server->Data.Properties["race_time_limit"]       = Cfg.SettingAsString( "screensaver_time_limit",      "0"     );
			Raptor::Server->Data.Properties["ai_waves"]              = Cfg.SettingAsString( "screensaver_ai_waves",        "7"     );
			Raptor::Server->Data.Properties["ai_flock"]              = Cfg.SettingAsString( "screensaver_ai_flock",        "true"  );
			Raptor::Server->Data.Properties["ai_respawn"]            = Cfg.SettingAsString( "screensaver_ai_respawn",      "true"  );
			Raptor::Server->Data.Properties["ai_grouped"]            = Cfg.SettingAsString( "screensaver_ai_grouped",      "false" );
			Raptor::Server->Data.Properties["spawn"]                 = Cfg.SettingAsString( "screensaver_spawn",           "T/A,YT1300");
			Raptor::Server->Data.Properties["rebel_fighter"]         = Cfg.SettingAsString( "screensaver_rebel_fighter",   "X/W"   );
			Raptor::Server->Data.Properties["rebel_bomber"]          = Cfg.SettingAsString( "screensaver_rebel_bomber",    "Y/W"   );
			Raptor::Server->Data.Properties["rebel_cruiser"]         = Cfg.SettingAsString( "screensaver_rebel_cruiser",   "CRV"   );
			Raptor::Server->Data.Properties["rebel_cruisers"]        = Cfg.SettingAsString( "screensaver_rebel_cruisers",  "3"     );
			Raptor::Server->Data.Properties["rebel_frigate"]         = Cfg.SettingAsString( "screensaver_rebel_frigate",   "FRG"   );
			Raptor::Server->Data.Properties["rebel_frigates"]        = Cfg.SettingAsString( "screensaver_rebel_frigates",  "1"     );
			Raptor::Server->Data.Properties["rebel_flagship"]        = Cfg.SettingAsString( "screensaver_rebel_flagship",  "FRG"   );
			Raptor::Server->Data.Properties["empire_fighter"]        = Cfg.SettingAsString( "screensaver_empire_fighter",  "T/I"   );
			Raptor::Server->Data.Properties["empire_bomber"]         = Cfg.SettingAsString( "screensaver_empire_bomber",   "T/F"   );
			Raptor::Server->Data.Properties["empire_cruiser"]        = Cfg.SettingAsString( "screensaver_empire_cruiser",  "INT"   );
			Raptor::Server->Data.Properties["empire_cruisers"]       = Cfg.SettingAsString( "screensaver_empire_cruisers", "3"     );
			Raptor::Server->Data.Properties["empire_frigate"]        = Cfg.SettingAsString( "screensaver_empire_frigate",  "ISD"   );
			Raptor::Server->Data.Properties["empire_frigates"]       = Cfg.SettingAsString( "screensaver_empire_frigates", "1"     );
			Raptor::Server->Data.Properties["empire_flagship"]       = Cfg.SettingAsString( "screensaver_empire_flagship", "ISD"   );
			Raptor::Server->Data.Properties["yavin_rebel_fighter"]   = Cfg.SettingAsString( "screensaver_rebel_fighter",   "X/W"   );
			Raptor::Server->Data.Properties["yavin_rebel_bomber"]    = Cfg.SettingAsString( "screensaver_rebel_bomber",    "Y/W"   );
			Raptor::Server->Data.Properties["yavin_empire_fighter"]  = Cfg.SettingAsString( "screensaver_empire_fighter",  "T/F"   );
			Raptor::Server->Data.Properties["defending_team"]        = Cfg.SettingAsString( "screensaver_defending_team",  "rebel" );
			Raptor::Server->Data.Properties["asteroids"]             = Cfg.SettingAsString( "screensaver_asteroids",       "16"    );
			Raptor::Server->Data.Properties["bg"]                    = Cfg.SettingAsString( "screensaver_bg",              "stars" );
			Raptor::Server->Data.Properties["allow_ship_change"]     = "true";
			Raptor::Server->Data.Properties["allow_team_change"]     = "true";
			Data.Properties = Raptor::Server->Data.Properties;
			Raptor::Server->Data.Lock.Unlock();
			Cfg.Load( "screensaver.cfg" );
		}
		
		SetPlayerProperty( "ship", "Spectator" );
		SetPlayerProperty( "name", "Screensaver" );
		
		if( Raptor::Server->IsRunning() )
		{
			Packet fly = Packet( XWing::Packet::FLY );
			Raptor::Game->Net.Send( &fly );
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
		
		if( CampaignTeam && (State <= XWing::State::LOBBY) )
		{
			Packet info( Raptor::Packet::INFO );
			info.AddUShort( 2 );
			info.AddString( "gametype" );
			info.AddString( "mission" );
			info.AddString( "mission" );
			info.AddString( (CampaignTeam == XWing::Team::EMPIRE) ? Cfg.SettingAsString("empire_mission","empire0") : Cfg.SettingAsString("rebel_mission","rebel0") );
			Net.Send( &info );
		}
	}
}


void XWingGame::BeginFlying( void )
{
	if( State != XWing::State::FLYING )
	{
		if( Cfg.SettingAsBool( "s_game_music" ) )
			Snd.PlayMusicSubdir( "Flight" );
		else
			Snd.StopMusic();
		
		ObservedShipID = 0;
		LookYaw = 0.;
		LookPitch = 0.;
		Head.Recenter();
		
		Layers.RemoveAll();
		Layers.Add( new RenderLayer() );
		
		if( Cfg.SettingAsBool("screensaver") )
		{
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
