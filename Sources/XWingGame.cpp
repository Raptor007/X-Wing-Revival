/*
 *  XWingGame.cpp
 */

#include "XWingGame.h"

#include <cstddef>
#include <cmath>
#include <dirent.h>
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


XWingGame::XWingGame( std::string version ) : RaptorGame( "X-Wing Revival", version )
{
	ObservedShipID = 0;
	LookYaw = 0.;
	LookPitch = 0.;
	ThumbstickLook = true;
	AsteroidLOD = 1.;
	memset( Controls, 0, sizeof(Controls) );
	
	// Analog Axes
	Controls[ XWing::Control::ROLL              ] = Input.AddControl( "Roll",      "Pedal"             );
	Controls[ XWing::Control::ROLL_INVERTED     ] = Input.AddControl( "Roll-",     "Pedal",    1., -1. );
	Controls[ XWing::Control::PITCH             ] = Input.AddControl( "Pitch"                          );
	Controls[ XWing::Control::PITCH_INVERTED    ] = Input.AddControl( "Pitch-",    "",         1., -1. );
	Controls[ XWing::Control::YAW               ] = Input.AddControl( "Yaw",       "Pedal"             );
	Controls[ XWing::Control::YAW_INVERTED      ] = Input.AddControl( "Yaw-",      "Pedal",    1., -1. );
	Controls[ XWing::Control::THROTTLE          ] = Input.AddControl( "Throttle",  "Throttle", 1.,  0. );
	Controls[ XWing::Control::THROTTLE_INVERTED ] = Input.AddControl( "Throttle-", "Throttle", 0.,  1. );
	Controls[ XWing::Control::LOOK_X            ] = Input.AddControl( "LookX"                          );
	Controls[ XWing::Control::LOOK_X_INVERTED   ] = Input.AddControl( "LookX-",    "",         1., -1. );
	Controls[ XWing::Control::LOOK_Y            ] = Input.AddControl( "LookY"                          );
	Controls[ XWing::Control::LOOK_Y_INVERTED   ] = Input.AddControl( "LookY-",    "",         1., -1. );
	
	// Held Digital Buttons
	Controls[ XWing::Control::ROLL_LEFT               ] = Input.AddControl( "RollLeft" );
	Controls[ XWing::Control::ROLL_RIGHT              ] = Input.AddControl( "RollRight" );
	Controls[ XWing::Control::PITCH_UP                ] = Input.AddControl( "PitchUp" );
	Controls[ XWing::Control::PITCH_DOWN              ] = Input.AddControl( "PitchDown" );
	Controls[ XWing::Control::YAW_LEFT                ] = Input.AddControl( "YawLeft" );
	Controls[ XWing::Control::YAW_RIGHT               ] = Input.AddControl( "YawRight" );
	Controls[ XWing::Control::THROTTLE_UP             ] = Input.AddControl( "ThrottleUp" );
	Controls[ XWing::Control::THROTTLE_DOWN           ] = Input.AddControl( "ThrottleDown" );
	Controls[ XWing::Control::THROTTLE_0              ] = Input.AddControl( "Throttle0" );
	Controls[ XWing::Control::THROTTLE_33             ] = Input.AddControl( "Throttle33" );
	Controls[ XWing::Control::THROTTLE_66             ] = Input.AddControl( "Throttle66" );
	Controls[ XWing::Control::THROTTLE_100            ] = Input.AddControl( "Throttle100" );
	Controls[ XWing::Control::FIRE                    ] = Input.AddControl( "Fire" );
	Controls[ XWing::Control::TARGET_NOTHING          ] = Input.AddControl( "TargetClear" );
	Controls[ XWing::Control::TARGET_CROSSHAIR        ] = Input.AddControl( "TargetCrosshair" );
	Controls[ XWing::Control::TARGET_NEAREST_ENEMY    ] = Input.AddControl( "TargetEnemy" );
	Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ] = Input.AddControl( "TargetAttacker" );
	Controls[ XWing::Control::TARGET_NEAREST_INCOMING ] = Input.AddControl( "TargetIncoming" );
	Controls[ XWing::Control::TARGET_OBJECTIVE        ] = Input.AddControl( "TargetObjective" );
	Controls[ XWing::Control::TARGET_NEWEST           ] = Input.AddControl( "TargetNewest" );
	Controls[ XWing::Control::TARGET_GROUPMATE        ] = Input.AddControl( "TargetGroupmate" );
	Controls[ XWing::Control::TARGET_SYNC             ] = Input.AddControl( "TargetDataLink" );
	Controls[ XWing::Control::LOOK_UP                 ] = Input.AddControl( "LookUp" );
	Controls[ XWing::Control::LOOK_DOWN               ] = Input.AddControl( "LookDown" );
	Controls[ XWing::Control::LOOK_LEFT               ] = Input.AddControl( "LookLeft" );
	Controls[ XWing::Control::LOOK_RIGHT              ] = Input.AddControl( "LookRight" );
	Controls[ XWing::Control::LOOK_UP_LEFT            ] = Input.AddControl( "LookUpLeft" );
	Controls[ XWing::Control::LOOK_UP_RIGHT           ] = Input.AddControl( "LookUpRight" );
	Controls[ XWing::Control::LOOK_DOWN_LEFT          ] = Input.AddControl( "LookDownLeft" );
	Controls[ XWing::Control::LOOK_DOWN_RIGHT         ] = Input.AddControl( "LookDownRight" );
	Controls[ XWing::Control::LOOK_CENTER             ] = Input.AddControl( "LookCenter" );
	Controls[ XWing::Control::SCORES                  ] = Input.AddControl( "Scores" );
	
	// Pressed Digital Buttons
	Controls[ XWing::Control::WEAPON                  ] = Input.AddControl( "WeaponNext" );
	Controls[ XWing::Control::MODE                    ] = Input.AddControl( "WeaponMode" );
	Controls[ XWing::Control::SHIELD_DIR              ] = Input.AddControl( "AngleDeflector" );
	Controls[ XWing::Control::TARGET_PREV             ] = Input.AddControl( "TargetPrev" );
	Controls[ XWing::Control::TARGET_NEXT             ] = Input.AddControl( "TargetNext" );
	Controls[ XWing::Control::TARGET_PREV_ENEMY       ] = Input.AddControl( "TargetPrevEnemy" );
	Controls[ XWing::Control::TARGET_NEXT_ENEMY       ] = Input.AddControl( "TargetNextEnemy" );
	Controls[ XWing::Control::TARGET_PREV_FRIENDLY    ] = Input.AddControl( "TargetPrevFriendly" );
	Controls[ XWing::Control::TARGET_NEXT_FRIENDLY    ] = Input.AddControl( "TargetNextFriendly" );
	Controls[ XWing::Control::SEAT_COCKPIT            ] = Input.AddControl( "Cockpit" );
	Controls[ XWing::Control::SEAT_GUNNER1            ] = Input.AddControl( "Gunner1" );
	Controls[ XWing::Control::SEAT_GUNNER2            ] = Input.AddControl( "Gunner2" );
	Controls[ XWing::Control::CHEWIE_TAKE_THE_WHEEL   ] = Input.AddControl( "Chewie" );
	Controls[ XWing::Control::CHAT                    ] = Input.AddControl( "Chat" );
	Controls[ XWing::Control::MENU                    ] = Input.AddControl( "Menu" );
}


XWingGame::~XWingGame()
{
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
	Cfg.KeyBinds[ SDLK_a            ] = Controls[ XWing::Control::THROTTLE_UP             ];
	Cfg.KeyBinds[ SDLK_z            ] = Controls[ XWing::Control::THROTTLE_DOWN           ];
	Cfg.KeyBinds[ SDLK_SPACE        ] = Controls[ XWing::Control::FIRE                    ];
	Cfg.KeyBinds[ SDLK_LCTRL        ] = Controls[ XWing::Control::TARGET_CROSSHAIR        ];
	Cfg.KeyBinds[ SDLK_e            ] = Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ];
	Cfg.KeyBinds[ SDLK_r            ] = Controls[ XWing::Control::TARGET_NEAREST_ENEMY    ];
	Cfg.KeyBinds[ SDLK_u            ] = Controls[ XWing::Control::TARGET_NEWEST           ];
	Cfg.KeyBinds[ SDLK_o            ] = Controls[ XWing::Control::TARGET_OBJECTIVE        ];
	Cfg.KeyBinds[ SDLK_i            ] = Controls[ XWing::Control::TARGET_NEAREST_INCOMING ];
	Cfg.KeyBinds[ SDLK_q            ] = Controls[ XWing::Control::TARGET_NOTHING          ];
	Cfg.KeyBinds[ SDLK_g            ] = Controls[ XWing::Control::TARGET_GROUPMATE        ];
	Cfg.KeyBinds[ SDLK_v            ] = Controls[ XWing::Control::TARGET_SYNC             ];
	Cfg.KeyBinds[ SDLK_CAPSLOCK     ] = Controls[ XWing::Control::TARGET_SYNC             ];
	Cfg.KeyBinds[ SDLK_KP7          ] = Controls[ XWing::Control::LOOK_UP_LEFT            ];
	Cfg.KeyBinds[ SDLK_KP8          ] = Controls[ XWing::Control::LOOK_UP                 ];
	Cfg.KeyBinds[ SDLK_KP9          ] = Controls[ XWing::Control::LOOK_UP_RIGHT           ];
	Cfg.KeyBinds[ SDLK_KP4          ] = Controls[ XWing::Control::LOOK_LEFT               ];
	Cfg.KeyBinds[ SDLK_KP5          ] = Controls[ XWing::Control::LOOK_CENTER             ];
	Cfg.KeyBinds[ SDLK_KP6          ] = Controls[ XWing::Control::LOOK_RIGHT              ];
	Cfg.KeyBinds[ SDLK_KP1          ] = Controls[ XWing::Control::LOOK_DOWN_LEFT          ];
	Cfg.KeyBinds[ SDLK_KP2          ] = Controls[ XWing::Control::LOOK_DOWN               ];
	Cfg.KeyBinds[ SDLK_KP3          ] = Controls[ XWing::Control::LOOK_DOWN_RIGHT         ];
	Cfg.KeyBinds[ SDLK_TAB          ] = Controls[ XWing::Control::SCORES                  ];
	
	Cfg.KeyBinds[ SDLK_w            ] = Controls[ XWing::Control::WEAPON                  ];
	Cfg.KeyBinds[ SDLK_x            ] = Controls[ XWing::Control::MODE                    ];
	Cfg.KeyBinds[ SDLK_s            ] = Controls[ XWing::Control::SHIELD_DIR              ];
	Cfg.KeyBinds[ SDLK_t            ] = Controls[ XWing::Control::TARGET_NEXT             ];
	Cfg.KeyBinds[ SDLK_y            ] = Controls[ XWing::Control::TARGET_PREV             ];
	Cfg.KeyBinds[ SDLK_s            ] = Controls[ XWing::Control::SHIELD_DIR              ];
	Cfg.KeyBinds[ SDLK_F1           ] = Controls[ XWing::Control::SEAT_COCKPIT            ];
	Cfg.KeyBinds[ SDLK_F2           ] = Controls[ XWing::Control::SEAT_GUNNER1            ];
	Cfg.KeyBinds[ SDLK_F3           ] = Controls[ XWing::Control::SEAT_GUNNER2            ];
	Cfg.KeyBinds[ SDLK_F4           ] = Controls[ XWing::Control::CHEWIE_TAKE_THE_WHEEL   ];
	Cfg.KeyBinds[ SDLK_RETURN       ] = Controls[ XWing::Control::CHAT                    ];
	Cfg.KeyBinds[ SDLK_KP_ENTER     ] = Controls[ XWing::Control::CHAT                    ];
	Cfg.KeyBinds[ SDLK_ESCAPE       ] = Controls[ XWing::Control::MENU                    ];
	Cfg.KeyBinds[ SDLK_F10          ] = Controls[ XWing::Control::MENU                    ];
	
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
	Cfg.JoyButtonBinds[ "Joy" ][  8 ] = Controls[ XWing::Control::TARGET_NEAREST_INCOMING ];  // T1
	Cfg.JoyButtonBinds[ "Joy" ][  9 ] = Controls[ XWing::Control::TARGET_NOTHING          ];  // T2
	Cfg.JoyButtonBinds[ "Joy" ][ 10 ] = Controls[ XWing::Control::TARGET_SYNC             ];  // T3
	Cfg.JoyButtonBinds[ "Joy" ][ 11 ] = Controls[ XWing::Control::TARGET_GROUPMATE        ];  // T4
	Cfg.JoyButtonBinds[ "Joy" ][ 12 ] = Controls[ XWing::Control::TARGET_PREV             ];  // T5
	Cfg.JoyButtonBinds[ "Joy" ][ 13 ] = Controls[ XWing::Control::TARGET_NEXT             ];  // T6
	Cfg.JoyButtonBinds[ "Joy" ][ 19 ] = Controls[ XWing::Control::TARGET_PREV_FRIENDLY    ];  // Hat 2 Up
	Cfg.JoyButtonBinds[ "Joy" ][ 20 ] = Controls[ XWing::Control::TARGET_NEXT_ENEMY       ];  // Hat 2 Right
	Cfg.JoyButtonBinds[ "Joy" ][ 21 ] = Controls[ XWing::Control::TARGET_NEXT_FRIENDLY    ];  // Hat 2 Down
	Cfg.JoyButtonBinds[ "Joy" ][ 22 ] = Controls[ XWing::Control::TARGET_PREV_ENEMY       ];  // Hat 2 Left
	Cfg.JoyButtonBinds[ "Joy" ][ 23 ] = Controls[ XWing::Control::TARGET_SYNC             ];  // Hat 3 Up
	Cfg.JoyButtonBinds[ "Joy" ][ 24 ] = Controls[ XWing::Control::TARGET_OBJECTIVE        ];  // Hat 3 Right
	Cfg.JoyButtonBinds[ "Joy" ][ 25 ] = Controls[ XWing::Control::TARGET_NOTHING          ];  // Hat 3 Down
	Cfg.JoyButtonBinds[ "Joy" ][ 26 ] = Controls[ XWing::Control::TARGET_GROUPMATE        ];  // Hat 3 Left
	Cfg.JoyButtonBinds[ "Joy" ][ 30 ] = Controls[ XWing::Control::TARGET_NEAREST_INCOMING ];  // Clutch
	
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_UP    ] = Controls[ XWing::Control::LOOK_UP    ];  // Hat 1 Up
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_RIGHT ] = Controls[ XWing::Control::LOOK_RIGHT ];  // Hat 1 Right
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_DOWN  ] = Controls[ XWing::Control::LOOK_DOWN  ];  // Hat 1 Down
	Cfg.JoyHatBinds[ "Joy" ][ 0 ][ SDL_HAT_LEFT  ] = Controls[ XWing::Control::LOOK_LEFT  ];  // Hat 1 Left
	
	Cfg.JoyAxisBinds[ "Pedal" ][ 0 ] = Controls[ XWing::Control::YAW_INVERTED ];  // Left Pedal
	Cfg.JoyAxisBinds[ "Pedal" ][ 1 ] = Controls[ XWing::Control::YAW          ];  // Right Pedal
	Cfg.JoyAxisBinds[ "Pedal" ][ 2 ] = Controls[ XWing::Control::YAW          ];  // Combined Slider
	
	Cfg.JoyAxisBinds[ "Throttle" ][ 0 ] = Controls[ XWing::Control::THROTTLE ];
	
	Cfg.JoyAxisBinds[ "Xbox" ][ 0 ] = Controls[ XWing::Control::YAW           ];  // Left Thumbstick X
	Cfg.JoyAxisBinds[ "Xbox" ][ 1 ] = Controls[ XWing::Control::PITCH         ];  // Left Thumbstick Y
	Cfg.JoyAxisBinds[ "Xbox" ][ 2 ] = Controls[ XWing::Control::ROLL_INVERTED ];  // Triggers
	Cfg.JoyAxisBinds[ "Xbox" ][ 3 ] = Controls[ XWing::Control::LOOK_Y        ];  // Right Thumbstick Y
	Cfg.JoyAxisBinds[ "Xbox" ][ 4 ] = Controls[ XWing::Control::LOOK_X        ];  // Right Thumbstick X
	
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
	Cfg.JoyButtonBinds[ "MFD" ][ 10 ] = Controls[ XWing::Control::TARGET_NEAREST_INCOMING ];  // Bottom #5
	Cfg.JoyButtonBinds[ "MFD" ][ 11 ] = Controls[ XWing::Control::TARGET_NEAREST_ENEMY    ];  // Bottom #4
	Cfg.JoyButtonBinds[ "MFD" ][ 12 ] = Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ];  // Bottom #3
	Cfg.JoyButtonBinds[ "MFD" ][ 13 ] = Controls[ XWing::Control::TARGET_NEAREST_INCOMING ];  // Bottom #2
	Cfg.JoyButtonBinds[ "MFD" ][ 14 ] = Controls[ XWing::Control::SHIELD_DIR              ];  // Bottom #1
	Cfg.JoyButtonBinds[ "MFD" ][ 15 ] = Controls[ XWing::Control::TARGET_NOTHING          ];  // Left #5
	Cfg.JoyButtonBinds[ "MFD" ][ 16 ] = Controls[ XWing::Control::WEAPON                  ];  // Left #4
	Cfg.JoyButtonBinds[ "MFD" ][ 17 ] = Controls[ XWing::Control::MODE                    ];  // Left #3
	Cfg.JoyButtonBinds[ "MFD" ][ 18 ] = Controls[ XWing::Control::TARGET_OBJECTIVE        ];  // Left #2
	Cfg.JoyButtonBinds[ "MFD" ][ 19 ] = Controls[ XWing::Control::SCORES                  ];  // Left #1
	
	Cfg.JoyAxisBinds[ "Wheel" ][ 0 ] = Controls[ XWing::Control::ROLL           ];  // Steering Wheel
	Cfg.JoyAxisBinds[ "Wheel" ][ 2 ] = Controls[ XWing::Control::THROTTLE       ];  // Accelerator Pedal
	Cfg.JoyAxisBinds[ "Wheel" ][ 3 ] = Controls[ XWing::Control::PITCH          ];  // Brake Pedal
	Cfg.JoyAxisBinds[ "Wheel" ][ 4 ] = Controls[ XWing::Control::PITCH_INVERTED ];  // Clutch Pedal
	
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
	
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_UP    ] = Controls[ XWing::Control::LOOK_UP    ];  // Shift Hat Up
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_RIGHT ] = Controls[ XWing::Control::LOOK_RIGHT ];  // Shift Hat Right
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_DOWN  ] = Controls[ XWing::Control::LOOK_DOWN  ];  // Shift Hat Down
	Cfg.JoyHatBinds[ "Wheel" ][ 0 ][ SDL_HAT_LEFT  ] = Controls[ XWing::Control::LOOK_LEFT  ];  // Shift Hat Left
}


void XWingGame::SetDefaults( void )
{
	RaptorGame::SetDefaults();
	
	if( Cfg.Settings[ "name" ] == "Name" )
		Cfg.Settings[ "name" ] = "Rookie One";
	
	Cfg.Settings[ "host_address" ] = "www.raptor007.com";
	
	Cfg.Settings[ "view" ] = "auto";
	Cfg.Settings[ "spectator_view" ] = "auto";
	
	Cfg.Settings[ "g_bg" ] = "true";
	Cfg.Settings[ "g_stars" ] = "0";
	Cfg.Settings[ "g_debris" ] = "500";
	Cfg.Settings[ "g_asteroid_lod" ] = "1";
	Cfg.Settings[ "g_deathstar_detail" ] = "3";
	Cfg.Settings[ "g_crosshair_thickness" ] = "1.5";
	
	Cfg.Settings[ "s_engine_volume" ] = "0.75";
	Cfg.Settings[ "s_menu_music" ] = "true";
	Cfg.Settings[ "s_game_music" ] = "true";
	Cfg.Settings[ "s_imuse" ] = "false";
	
	Cfg.Settings[ "joy_enable" ] = "true";
	Cfg.Settings[ "joy_hide_binds" ] = "true";
	
	Cfg.Settings[ "mouse_mode" ] = "disabled";
	Cfg.Settings[ "mouse_invert" ] = "true";
	Cfg.Settings[ "mouse_smooth" ] = "0.25";
	
	Cfg.Settings[ "swap_yaw_roll" ] = "false";
	Cfg.Settings[ "turret_invert" ] = "false";
	
	#ifdef APPLE_POWERPC
		Cfg.Settings[ "g_dynamic_lights" ] = "1";
		Cfg.Settings[ "g_debris" ] = "200";
		Cfg.Settings[ "g_deathstar_detail" ] = "2";
	#endif
}


void XWingGame::Setup( int argc, char **argv )
{
	bool screensaver = Cfg.SettingAsBool("screensaver");
	bool safemode = false;
	for( int i = 1; i < argc; i ++ )
	{
		if( strcasecmp( argv[ i ], "-safe" ) == 0 )
			safemode = true;
	}
	
	if( screensaver )
	{
		Cfg.Settings[ "s_menu_music" ] = "false";
		Cfg.Settings[ "s_game_music" ] = "false";
		Cfg.Settings[ "saitek_enable" ] = "false";
		Cfg.Settings[ "spectator_view" ] = Cfg.SettingAsString( "screensaver_view", "cycle" );
		int maxfps = Cfg.SettingAsInt( "maxfps", 60 );
		Cfg.Settings[ "sv_netrate" ] = Cfg.Settings[ "sv_maxfps" ] = (maxfps > 60) ? Num::ToString(maxfps) : "60";
	}
	
	if( safemode )
	{
		Cfg.Settings[ "precache" ] = "false";
		Cfg.Settings[ "g_group_skins" ] = "false";
	}
	
	// Set music to shuffle.
	Snd.ShuffleMusic = true;
	
	// Show a loading screen while precaching resources.
	Gfx.Clear( 0.f, 0.f, 0.f );
	FirstLoadScreen *load_screen = new FirstLoadScreen();
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
	
	if( ! screensaver )
		// Add the main menu to the now-empty Layers stack.
		Layers.Add( new MainMenu() );
	else
		// Add an input handler to quit the screensaver when necessary.
		AddScreensaverLayer();
	
	if( ! screensaver )
	{
		// Load other shaders.
		Res.GetShader("model_hud");
		Res.GetShader("deathstar");
	}
	
	// Load and select the model shader, but don't activate it yet.
	ShaderMgr.Select( Res.GetShader("model") );
	
	// Generate all framebuffers for render-to-texture.
	Res.GetFramebuffer( "health", 384, 512 );
	Res.GetFramebuffer( "target", 384, 512 );
	Res.GetFramebuffer( "intercept", 384, 256 );
	Res.GetFramebuffer( "throttle", 32, 256 );
	
	// Precache resources while the loading screen is still being shown.
	if( Cfg.SettingAsBool("precache",true) )
		Precache();
}


void XWingGame::Precache( void )
{
	Res.GetAnimation("explosion.ani");
	Res.GetAnimation("laser_red.ani");
	Res.GetAnimation("laser_green.ani");
	Res.GetAnimation("torpedo.ani");
	
	// Technically these are played at zero volume in the screensaver.
	Res.GetSound("laser_red.wav");
	Res.GetSound("laser_green.wav");
	Res.GetSound("torpedo.wav");
	Res.GetSound("turbolaser_green.wav");
	Res.GetSound("explosion.wav");
	Res.GetSound("damage_hull.wav");
	Res.GetSound("damage_shield.wav");
	Res.GetSound("hit_hull.wav");
	Res.GetSound("hit_shield.wav");
	Res.GetSound("jump_in.wav");
	
	bool screensaver = Cfg.SettingAsBool("screensaver");
	if( screensaver )
	{
		Res.GetAnimation( Cfg.SettingAsString( "screensaver_bg", "stars" ) + std::string(".ani") );
		if( Cfg.SettingAsInt( "screensaver_asteroids", 32 ) )
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
		
		Res.GetAnimation("missile.ani");
		Res.GetAnimation("deathstar.ani");
		
		Res.GetModel("asteroid.obj");
		Res.GetModel("turret_body.obj");
		Res.GetModel("turret_gun.obj");
		Res.GetModel("deathstar_detail.obj");
		Res.GetModel("deathstar_detail_bottom.obj");
		Res.GetModel("deathstar_box.obj");
		Res.GetModel("deathstar_exhaust_port.obj");
		
		Res.GetSound("beep.wav");
		Res.GetSound("beep_error.wav");
		Res.GetSound("beep_aim.wav");
		Res.GetSound("beep_sc.wav");
		Res.GetSound("beep_sf.wav");
		Res.GetSound("beep_sr.wav");
		Res.GetSound("beep_laser1.wav");
		Res.GetSound("beep_laser2.wav");
		Res.GetSound("beep_laser3.wav");
		Res.GetSound("beep_torpedo1.wav");
		Res.GetSound("beep_torpedo2.wav");
		Res.GetSound("locking.wav");
		Res.GetSound("locked.wav");
		Res.GetSound("chat.wav");
		Res.GetSound("incoming.wav");
		
		Res.GetSound("laser_turret.wav");
		Res.GetSound("torpedo_enter.wav");
		Res.GetSound("jump_in_cockpit.wav");
		
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
	}
	
	if( DIR *dir_p = opendir("Ships") )
	{
		std::string screensaver_rebel, screensaver_empire, screensaver_rebel2, screensaver_empire2;
		std::string gametype = Cfg.SettingAsString( "screensaver_gametype", "fleet" );
		if( gametype == "yavin" )
		{
			screensaver_rebel  = Cfg.SettingAsString( "screensaver_rebel_fighter",  "X/W" );
			screensaver_rebel2 = Cfg.SettingAsString( "screensaver_rebel_bomber",   "Y/W" );
			screensaver_empire = Cfg.SettingAsString( "screensaver_empire_fighter", "T/F" );
		}
		else
		{
			screensaver_rebel  = Cfg.SettingAsString( "screensaver_rebel_fighter",  "X/W" );
			screensaver_empire = Cfg.SettingAsString( "screensaver_empire_fighter", "T/I" );
			if( gametype == "fleet" )
			{
				screensaver_rebel2  = Cfg.SettingAsString( "screensaver_rebel_bomber",  "YT1300" );
				screensaver_empire2 = Cfg.SettingAsString( "screensaver_empire_bomber", "T/F" );
			}
		}
		
		while( struct dirent *dir_entry_p = readdir(dir_p) )
		{
			if( ! dir_entry_p->d_name )
				continue;
			if( dir_entry_p->d_name[ 0 ] == '.' )
				continue;
			if( ! CStr::EndsWith( dir_entry_p->d_name, ".def" ) )
				continue;
			
			ShipClass sc;
			if( sc.Load( std::string("Ships/") + std::string(dir_entry_p->d_name) ) )
			{
				if( screensaver && (sc.ShortName != screensaver_rebel) && (sc.ShortName != screensaver_empire) && (sc.ShortName != screensaver_rebel2) && (sc.ShortName != screensaver_empire2) )
					continue;
				
				if( sc.ExternalModel.length() )
					Res.GetModel( sc.ExternalModel );
				if( sc.CockpitModel.length() )
					Res.GetModel( sc.CockpitModel );
				
				if( Cfg.SettingAsBool("g_group_skins",true) && ! screensaver )
				{
					for( std::map<uint8_t,std::string>::const_iterator skin_iter = sc.GroupSkins.begin(); skin_iter != sc.GroupSkins.end(); skin_iter ++ )
						Res.GetModel( skin_iter->second );
					for( std::map<uint8_t,std::string>::const_iterator skin_iter = sc.GroupCockpits.begin(); skin_iter != sc.GroupCockpits.end(); skin_iter ++ )
						Res.GetModel( skin_iter->second );
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
	screensaver_layer->IgnoreKeys.insert( SDLK_LEFTBRACKET );
	screensaver_layer->IgnoreKeys.insert( SDLK_RIGHTBRACKET );
	screensaver_layer->IgnoreKeys.insert( SDLK_KP1 );
	screensaver_layer->IgnoreKeys.insert( SDLK_KP2 );
	screensaver_layer->IgnoreKeys.insert( SDLK_KP3 );
	screensaver_layer->IgnoreKeys.insert( SDLK_KP4 );
	screensaver_layer->IgnoreKeys.insert( SDLK_KP5 );
	screensaver_layer->IgnoreKeys.insert( SDLK_KP6 );
	screensaver_layer->IgnoreKeys.insert( SDLK_KP7 );
	screensaver_layer->IgnoreKeys.insert( SDLK_KP8 );
	screensaver_layer->IgnoreKeys.insert( SDLK_KP9 );
	screensaver_layer->IgnoreKeys.insert( SDLK_F12 );
	screensaver_layer->IgnoreKeys.insert( SDLK_PRINT );
	Layers.Add( screensaver_layer );
}


void XWingGame::Update( double dt )
{
	// Update ship's motion changes based on client's controls.
	
	double roll = 0.;
	double pitch = 0.;
	double yaw = 0.;
	static double throttle = 1.;
	bool firing = false;
	static bool error_beep_allowed = true;
	
	bool target_crosshair = false;
	bool target_nearest_enemy = false;
	bool target_nearest_attacker = false;
	bool target_newest = false;
	bool target_nearest_incoming = false;
	bool target_objective = false;
	bool target_nothing = false;
	bool target_groupmate = false;
	bool target_sync = false;
	
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
	roll  = Num::Clamp( roll  + Input.ControlTotal(Controls[ XWing::Control::ROLL  ]) + Input.ControlTotal(Controls[ XWing::Control::ROLL_INVERTED  ]), -1., 1. );
	pitch = Num::Clamp( pitch + Input.ControlTotal(Controls[ XWing::Control::PITCH ]) + Input.ControlTotal(Controls[ XWing::Control::PITCH_INVERTED ]), -1., 1. );
	yaw   = Num::Clamp( yaw   + Input.ControlTotal(Controls[ XWing::Control::YAW   ]) + Input.ControlTotal(Controls[ XWing::Control::YAW_INVERTED   ]), -1., 1. );
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
			LookPitch = behind * 20.;
			LookYaw = ((LookYaw >= 0.) ? 180. : -180.) * behind + LookYaw * (1. - behind);
		}
	}
	
	// Held Digital Buttons
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
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_NEAREST_INCOMING ]) )
		target_nearest_incoming = true;
	if( Input.ControlPressed(Controls[ XWing::Control::TARGET_OBJECTIVE ]) )
		target_objective = true;
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
	
	if(!( target_crosshair
	||    target_nearest_enemy
	||    target_nearest_attacker
	||    target_newest
	||    target_nearest_incoming
	||    target_objective
	||    target_nothing
	||    target_groupmate
	||    target_sync ))
		error_beep_allowed = true;
	
	// Make sure the throttle value is legit.
	if( throttle > 1. )
		throttle = 1.;
	else if( throttle < 0. )
		throttle = 0.;
	
	
	// Apply controls to player's ship or turret.
	
	uint32_t target_id = 0;
	const char *beep = NULL;
	const Ship *my_turret_parent = NULL;
	
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
		
		if( (my_ship->Health > 0.) && (my_ship->JumpProgress >= 1.) )
		{
			my_ship->SetRoll( roll, dt );
			my_ship->SetPitch( pitch, dt );
			my_ship->SetYaw( yaw, dt );
			
			if( ! my_turret )
			{
				my_ship->Firing = firing;
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
			}
		}
		else
		{
			my_ship->RollRate = 0.;
			my_ship->PitchRate = 0.;
			my_ship->YawRate = 0.;
			my_ship->Firing = false;
			target_id = 0;
			
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
		
		const Pos3D *my_pos = my_turret ? (const Pos3D*) my_turret : (const Pos3D*) my_ship;
		uint32_t my_ship_id = my_turret_parent ? my_turret_parent->ID : (my_ship ? my_ship->ID : 0);
		uint8_t my_team = my_turret ? my_turret->Team : my_ship->Team;
		
		if( target_nothing )
		{
			if( target_id )
			{
				target_id = 0;
				beep = "beep.wav";
			}
		}
		else if( target_crosshair )
		{
			Vec3D my_fwd = my_turret ? my_turret->GunPos().Fwd : my_ship->Fwd;
			double best = 0.;
			uint32_t id = 0;
			
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				if( (*ship_iter)->ID != my_ship_id )
				{
					Ship *ship = *ship_iter;
					
					if( ship->Health <= 0. )
						continue;
					if( ship->JumpProgress < 1. )
						continue;
					
					Vec3D vec_to_ship( ship->X - my_pos->X, ship->Y - my_pos->Y, ship->Z - my_pos->Z );
					vec_to_ship.ScaleTo( 1. );
					double dot = my_fwd.Dot( &vec_to_ship );
					if( (dot > 0.) && ((dot > best) || ! id) )
					{
						best = dot;
						id = ship->ID;
					}
				}
			}
			
			if( target_id != id )
			{
				target_id = id;
				beep = "beep.wav";
			}
		}
		else if( target_nearest_enemy || target_nearest_attacker )
		{
			double best = 0.;
			uint8_t category = ShipClass::CATEGORY_TARGET;
			uint32_t id = 0;
			
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				if( (*ship_iter)->ID != my_ship_id )
				{
					Ship *ship = *ship_iter;
					
					if( ship->Health <= 0. )
						continue;
					if( ship->JumpProgress < 1. )
						continue;
					if( my_team && (ship->Team == my_team) )
						continue;
					
					// Prefer non-capital ship targets.
					uint8_t ship_category = ship->Category();
					if( (ship_category > category) && (ship_category >= ShipClass::CATEGORY_CAPITAL) )
						continue;
					
					if( target_nearest_attacker && (ship->Target != my_ship_id) )
					{
						std::list<Turret*> attached_turrets = ship->AttachedTurrets();
						bool found_turret = false;
						for( std::list<Turret*>::const_iterator turret_iter = attached_turrets.begin(); turret_iter != attached_turrets.end(); turret_iter ++ )
						{
							if( (*turret_iter)->Target == my_ship_id )
							{
								found_turret = true;
								break;
							}
						}
						if( ! found_turret )
							continue;
					}
					
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
			
			if( target_id != id )
			{
				target_id = id;
				beep = "beep.wav";
			}
		}
		else if( target_nearest_incoming )
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
					
					double dist = my_pos->Dist( shot );
					if( (dist < best) || ! id )
					{
						best = dist;
						id = shot->ID;
					}
				}
			}
			
			if( ! id )
				beep = "beep_error.wav";
			else if( target_id != id )
			{
				target_id = id;
				beep = "beep.wav";
			}
		}
		else if( target_objective )
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
					if( ship->JumpProgress < 1. )
						continue;
					if( my_team && (ship->Team == my_team) && (ship->Category() != ShipClass::CATEGORY_TARGET) )
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
			
			uint32_t id = best ? best->ID : 0;
			
			if( ! id )
				beep = "beep_error.wav";
			else if( target_id != id )
			{
				target_id = id;
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
						// Only receive target data about alive ships.
						const Ship *potential_target = (const Ship*) Data.GetObject( (*turret_iter)->Target );
						if( (! potential_target) || (potential_target->Type() != XWing::Object::SHIP) || (potential_target->Health <= 0.f) )
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
							// Only receive target data about alive ships.
							const Ship *potential_target = (const Ship*) Data.GetObject( (*ship_iter)->Target );
							if( (! potential_target) || (potential_target->Type() != XWing::Object::SHIP) || (potential_target->Health <= 0.f) )
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
					if( ship->JumpProgress < 1. )
						continue;
					
					// FIXME: Should this only target enemies?  It currently does not check team.
					
					double time = ship->Lifetime.ElapsedSeconds();
					if( (time < best) || ! id )
					{
						best = time;
						id = ship->ID;
					}
				}
			}
			
			if( target_id != id )
			{
				target_id = id;
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
					if( ((target_ship->Health < 0.) && (target_ship->DeathClock.ElapsedSeconds() > 4.)) || (target_ship->JumpProgress < 1.) )
						target_id = 0;
				}
			}
			else
				target_id = 0;
		}
	}
	
	if( my_turret )
		my_turret->Target = target_id;
	
	if( my_ship )
	{
		// Set target ID and update missile lock progress.
		int old_lock = my_ship->TargetLock * 9;
		my_ship->UpdateTarget( target_id ? Data.GetObject(target_id) : NULL, dt );
		int new_lock = my_ship->TargetLock * 9;
		if( new_lock > old_lock )
		{
			if( new_lock == 9 )
				Snd.Play( Res.GetSound("locked.wav") );
			else if( new_lock < 9 )
				Snd.Play( Res.GetSound("locking.wav") );
		}
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
	
	
	Ship *observed_ship = (Ship*) Data.GetObject( ObservedShipID );
	if( ! observed_ship )
		observed_ship = my_ship;
	
	
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
			
			if( (! found_music) && observed_ship && (Data.PropertyAsString("gametype") == "yavin") )
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
	bool observe_next = false;
	bool observe_prev = false;
	uint8_t change_seat = 0;
	bool chewie_take_the_wheel = false;
	
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
	else if( control == Controls[ XWing::Control::SEAT_COCKPIT ] )
		change_seat = 1;
	else if( control == Controls[ XWing::Control::SEAT_GUNNER1 ] )
		change_seat = 2;
	else if( control == Controls[ XWing::Control::SEAT_GUNNER2 ] )
		change_seat = 3;
	else if( control == Controls[ XWing::Control::CHEWIE_TAKE_THE_WHEEL ] )
		chewie_take_the_wheel = true;
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
	
	
	// Apply controls to player's ship or turret.
	
	uint32_t target_id = 0;
	bool beep = false;
	const Ship *my_turret_parent = NULL;
	
	if( my_turret )
	{
		target_id = my_turret->Target;
		my_turret_parent = my_turret->ParentShip();
		
		if( firing_mode_next && my_turret->Visible )
		{
			my_turret->FiringMode = (my_turret->FiringMode == 1) ? 2 : 1;
			Snd.Play( (my_turret->FiringMode == 1) ? Res.GetSound("beep_laser1.wav") : Res.GetSound("beep_laser2.wav") );
		}
	}
	else if( my_ship && (my_ship->Health > 0.) )
	{
		target_id = my_ship->Target;
		
		if( weapon_next || firing_mode_next )
		{
			bool changed = weapon_next ? my_ship->NextWeapon() : my_ship->NextFiringMode();
			if( changed )
			{
				// Changed weapon or mode; play a beep for the new mode.
				
				bool heavy_weapon = (my_ship->SelectedWeapon == Shot::TYPE_MISSILE) || (my_ship->SelectedWeapon == Shot::TYPE_TORPEDO);
				if( my_ship->CurrentFiringMode() <= 1 )
					Snd.Play( heavy_weapon ? Res.GetSound("beep_torpedo1.wav") : Res.GetSound("beep_laser1.wav") );
				else if( my_ship->CurrentFiringMode() == 2 )
					Snd.Play( heavy_weapon ? Res.GetSound("beep_torpedo2.wav") : Res.GetSound("beep_laser2.wav") );
				else
					Snd.Play( heavy_weapon ? Res.GetSound("beep_torpedo2.wav") : Res.GetSound("beep_laser3.wav") );
			}
			else
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
	
	if( target_next || target_prev || target_next_enemy || target_prev_enemy || target_next_friendly || target_prev_friendly )
	{
		uint32_t my_ship_id = my_turret_parent ? my_turret_parent->ID : (my_ship ? my_ship->ID : 0);
		uint8_t my_team = my_turret ? my_turret->Team : my_ship->Team;
		
		std::map<uint32_t,Ship*> potential_targets;
		for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
		{
			if( (*ship_iter)->ID != my_ship_id )
			{
				Ship *ship = *ship_iter;
				
				if( ship->Health <= 0. )
					continue;
				if( ship->JumpProgress < 1. )
					continue;
				if( (target_next_enemy || target_prev_enemy) && my_team && (ship->Team == my_team) )
					continue;
				if( (target_next_friendly || target_prev_friendly) && ((! my_team) || (ship->Team != my_team)) )
					continue;
				
				potential_targets[ ship->ID ] = ship;
			}
		}
		
		std::map<uint32_t,Ship*>::iterator target_iter = potential_targets.find( target_id );
		if( target_next || target_next_enemy || target_next_friendly )
		{
			if( target_iter == potential_targets.end() )
				target_iter = potential_targets.begin();
			else
				target_iter ++;
		}
		else if( target_prev || target_prev_enemy || target_prev_friendly )
		{
			if( target_iter == potential_targets.begin() )
				target_iter = potential_targets.end();
			else if( potential_targets.size() )
				target_iter --;
		}
		
		uint32_t id = (target_iter != potential_targets.end()) ? target_iter->first : 0;
		if( !( id || target_id ) )
			Snd.Play( Res.GetSound("beep_error.wav") );
		else if( id != target_id )
		{
			// Note: This clears the current target if no match was found.
			target_id = id;
			beep = true;
		}
	}
	
	if( my_turret )
		my_turret->Target = target_id;
	
	if( my_ship )
		my_ship->UpdateTarget( target_id ? Data.GetObject(target_id) : NULL );
	
	// If we did anything that calls for a beep, play the sound now.
	if( beep )
		Snd.Play( Res.GetSound("beep.wav") );
	
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


bool XWingGame::ProcessPacket( Packet *packet )
{
	packet->Rewind();
	PacketType type = packet->Type();
	
	if( type == Raptor::Packet::MESSAGE )
	{
		packet->NextString();
		if( packet->Remaining() )
		{
			uint32_t msg_type = packet->NextUInt();
			if( msg_type == TextConsole::MSG_CHAT )
				Snd.Play( Res.GetSound("chat.wav") );
		}
		
		// Don't return true, because we want RaptorGame to process the message too.
	}
	
	else if( type == XWing::Packet::EXPLOSION )
	{
		double x = packet->NextDouble();
		double y = packet->NextDouble();
		double z = packet->NextDouble();
		double dx = packet->NextFloat();
		double dy = packet->NextFloat();
		double dz = packet->NextFloat();
		float size = packet->NextFloat();
		float loudness = packet->NextFloat();
		
		if( State >= XWing::State::FLYING )
		{
			Pos3D pos( x, y, z );
			Vec3D motion_vec( dx, dy, dz );
			Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), size, Res.GetSound("explosion.wav"), loudness, &pos, &motion_vec, Rand::Bool() ? 360. : -360., 1. ) );
			for( int i = 0; i < 20; i ++ )
			{
				Vec3D rand( Rand::Double(-size,size), Rand::Double(-size,size), Rand::Double(-size,size) );
				rand.ScaleBy( 0.5 );
				pos.SetPos( x + rand.X, y + rand.Y, z + rand.Z );
				motion_vec.Set( dx + rand.X * fabs(rand.X), dy + rand.Y * fabs(rand.Y), dz + rand.Z * fabs(rand.Z) );
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), size * Rand::Double(0.1,0.5), NULL, 0., &pos, &motion_vec, Rand::Bool() ? 360. : -360., Rand::Double(0.5, 2.) ) );
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
			ship->HitRear = flags & 0x01;
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
						sound = Res.GetSound("damage_hull.wav");
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
				
				double total_damage = (old_shields + old_health) - (ship->ShieldF + ship->ShieldR + ship->Health);
				Vec3D knock( shot_dx - ship_dx, shot_dy - ship_dy, shot_dz - ship_dz );
				ship->KnockCockpit( &knock, total_damage );
			}
			
			if( ship && (ship->Category() == ShipClass::CATEGORY_TARGET) )
				;
			else if( shot_type == Shot::TYPE_TORPEDO )
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 5., Res.GetSound("explosion.wav"), 2.5, &pos, &motion_vec, Rand::Bool() ? 360. : -360., 1. ) );
			else if( shot_type == Shot::TYPE_MISSILE )
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 4., Res.GetSound("explosion.wav"), 2.5, &pos, &motion_vec, Rand::Bool() ? 360. : -360., 1. ) );
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
			
			if( turret )
			{
				Mix_Chunk *sound = Res.GetSound("hit_hull.wav");
				double loudness = 1.;
				Snd.PlayAt( sound, x, y, z, loudness );
			}
			
			if( shot_type == Shot::TYPE_TORPEDO )
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 5., Res.GetSound("explosion.wav"), 2.5, &pos, &motion_vec, Rand::Bool() ? 360. : -360., 1. ) );
			else if( shot_type == Shot::TYPE_MISSILE )
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 4., Res.GetSound("explosion.wav"), 2.5, &pos, &motion_vec, Rand::Bool() ? 360. : -360., 1. ) );
		}
		
		return true;
	}
	
	else if( type == XWing::Packet::SHOT_HIT_HAZARD )
	{
		uint8_t shot_type = packet->NextUChar();
		double x = packet->NextDouble();
		double y = packet->NextDouble();
		double z = packet->NextDouble();
		
		double dx = 0., dy = 0., dz = 0.;
		
		if( State >= XWing::State::FLYING )
		{
			Pos3D pos( x, y, z );
			Vec3D motion_vec( dx, dy, dz );
			
			if( shot_type == Shot::TYPE_TORPEDO )
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 5., Res.GetSound("explosion.wav"), 2.5, &pos, &motion_vec, Rand::Bool() ? 360. : -360., 1. ) );
			else if( shot_type == Shot::TYPE_MISSILE )
				Data.Effects.push_back( Effect( Res.GetAnimation("explosion.ani"), 4., Res.GetSound("explosion.wav"), 2.5, &pos, &motion_vec, Rand::Bool() ? 360. : -360., 1. ) );
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
			ship->HitRear = flags & 0x01;
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
				Vec3D knock( ship->X - x, ship->Y - y, ship->Z - z );
				ship->KnockCockpit( &knock, total_damage );
			}
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
	
	else if( type == XWing::Packet::TIME_REMAINING )
	{
		RoundTimer.Reset( packet->NextFloat() );
		return true;
	}
	
	else if( type == XWing::Packet::LOBBY )
	{
		ChangeState( XWing::State::LOBBY );
		return true;
	}
	
	else if( type == XWing::Packet::FLY )
	{
		ChangeState( XWing::State::FLYING );
		return true;
	}
	
	else if( type == XWing::Packet::ROUND_ENDED )
	{
		uint32_t game_type = packet->NextUInt();
		uint32_t victor = packet->NextUInt();
		
		if( State >= XWing::State::FLYING )
		{
			std::string music_name = "defeat.dat";
			
			// Victory checking is by player ID for FFA gametypes.
			if( (game_type == XWing::GameType::FFA_ELIMINATION) || (game_type == XWing::GameType::FFA_DEATHMATCH) )
			{
				if( victor == PlayerID )
					music_name = "victory.dat";
				else if( victor )
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
				if( victor == XWing::Team::REBEL )
					music_name = "rebel.dat";
				else if( victor == XWing::Team::EMPIRE )
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
	
	else if( type == XWing::Packet::TOGGLE_COPILOT )
	{
		Snd.Play( Res.GetSound("chewie.wav") );
		return true;
	}
	
	return RaptorGame::ProcessPacket( packet );
}


void XWingGame::ChangeState( int state )
{
	if( state == XWing::State::LOBBY )
		ShowLobby();
	else if( state == XWing::State::FLYING )
		BeginFlying();
	
	RaptorGame::ChangeState( state );
}


void XWingGame::Disconnected( void )
{
	Mouse.ShowCursor = true;
	
	if( State >= XWing::State::LOBBY )
	{
		if( Cfg.SettingAsBool( "s_menu_music" ) )
			Snd.PlayMusicSubdir( "Menu" );
		else
			Snd.StopMusic();
		
		Layers.RemoveAll();
		Layers.Add( new MainMenu() );
	}
	
	RaptorGame::Disconnected();
}


void XWingGame::ShowLobby( void )
{
	if( Cfg.SettingAsBool("screensaver") )
	{
		if( Raptor::Server->IsRunning() && (State <= XWing::State::LOBBY) )
		{
			// FIXME: Should this send an INFO packet instead of directly modifying server data from the client thread?
			Raptor::Server->Data.Properties["gametype"]             = Cfg.SettingAsString( "screensaver_gametype",        "fleet" );
			Raptor::Server->Data.Properties["dm_kill_limit"]        = Cfg.SettingAsString( "screensaver_kill_limit",      "0"     );
			Raptor::Server->Data.Properties["tdm_kill_limit"]       = Cfg.SettingAsString( "screensaver_kill_limit",      "0"     );
			Raptor::Server->Data.Properties["hunt_time_limit"]      = Cfg.SettingAsString( "screensaver_time_limit",      "0"     );
			Raptor::Server->Data.Properties["yavin_time_limit"]     = Cfg.SettingAsString( "screensaver_time_limit",      "0"     );
			Raptor::Server->Data.Properties["ai_waves"]             = Cfg.SettingAsString( "screensaver_ai_waves",        "7"     );
			Raptor::Server->Data.Properties["ai_flock"]             = Cfg.SettingAsString( "screensaver_ai_flock",        "true"  );
			Raptor::Server->Data.Properties["rebel_fighter"]        = Cfg.SettingAsString( "screensaver_rebel_fighter",   "X/W"   );
			Raptor::Server->Data.Properties["rebel_bomber"]         = Cfg.SettingAsString( "screensaver_rebel_bomber",    "YT1300");
			Raptor::Server->Data.Properties["rebel_cruiser"]        = Cfg.SettingAsString( "screensaver_rebel_cruiser",   "CRV"   );
			Raptor::Server->Data.Properties["rebel_cruisers"]       = Cfg.SettingAsString( "screensaver_rebel_cruisers",  "4"     );
			Raptor::Server->Data.Properties["rebel_flagship"]       = Cfg.SettingAsString( "screensaver_rebel_flagship",  "FRG"   );
			Raptor::Server->Data.Properties["empire_fighter"]       = Cfg.SettingAsString( "screensaver_empire_fighter",  "T/I"   );
			Raptor::Server->Data.Properties["empire_bomber"]        = Cfg.SettingAsString( "screensaver_empire_bomber",   "T/F"   );
			Raptor::Server->Data.Properties["empire_cruiser"]       = Cfg.SettingAsString( "screensaver_empire_cruiser",  "INT"   );
			Raptor::Server->Data.Properties["empire_cruisers"]      = Cfg.SettingAsString( "screensaver_empire_cruisers", "4"     );
			Raptor::Server->Data.Properties["empire_flagship"]      = Cfg.SettingAsString( "screensaver_empire_flagship", "ISD"   );
			Raptor::Server->Data.Properties["yavin_rebel_fighter"]  = Cfg.SettingAsString( "screensaver_rebel_fighter",   "X/W"   );
			Raptor::Server->Data.Properties["yavin_rebel_bomber"]   = Cfg.SettingAsString( "screensaver_rebel_bomber",    "Y/W"   );
			Raptor::Server->Data.Properties["yavin_empire_fighter"] = Cfg.SettingAsString( "screensaver_empire_fighter",  "T/F"   );
			Raptor::Server->Data.Properties["defending_team"]       = Cfg.SettingAsString( "screensaver_defending_team",  "rebel" );
			Raptor::Server->Data.Properties["spawn"]                = Cfg.SettingAsString( "screensaver_spawn",           ""      );
			Raptor::Server->Data.Properties["asteroids"]            = Cfg.SettingAsString( "screensaver_asteroids",       "32"    );
			Raptor::Server->Data.Properties["bg"]                   = Cfg.SettingAsString( "screensaver_bg",              "stars" );
			Raptor::Server->Data.Properties["allow_ship_change"]    = "true";
			Raptor::Server->Data.Properties["allow_team_change"]    = "true";
			Data.Properties = Raptor::Server->Data.Properties;
			Cfg.Load( "screensaver.cfg" );
		}
		
		SetPlayerProperty( "ship", "Spectator" );
		SetPlayerProperty( "name", "Screensaver" );
		
		bool game_in_progress = false;
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() != XWing::Object::SHIP_CLASS )
			{
				game_in_progress = true;
				break;
			}
		}
		
		if( game_in_progress || Raptor::Server->IsRunning() )
		{
			Packet fly = Packet( XWing::Packet::FLY );
			Raptor::Game->Net.Send( &fly );
		}
	}
	else
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
	if( type == XWing::Object::SHIP )
		return new Ship( id );
	else if( type == XWing::Object::SHOT )
		return new Shot( id );
	else if( type == XWing::Object::ASTEROID )
		return new Asteroid( id );
	else if( type == XWing::Object::TURRET )
		return new Turret( id );
	else if( type == XWing::Object::DEATH_STAR )
		return new DeathStar( id );
	else if( type == XWing::Object::DEATH_STAR_BOX )
		return new DeathStarBox( id );
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
