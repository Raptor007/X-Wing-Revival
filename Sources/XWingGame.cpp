/*
 *  XWingGame.cpp
 */

#include "XWingGame.h"

#include <cstddef>
#include <cmath>
#include <dirent.h>
#include "XWingDefs.h"
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
	ReadKeyboard = true;
	ReadMouse = true;
	
	ObservedShipID = 0;
	LookYaw = 0.;
	LookPitch = 0.;
	ThumbstickLook = true;
}


XWingGame::~XWingGame()
{
}


void XWingGame::SetDefaults( void )
{
	RaptorGame::SetDefaults();
	
	Cfg.Settings[ "name" ] = "Rookie One";
	
	Cfg.Settings[ "host_address" ] = "fiber.raptor007.com";
	
	Cfg.Settings[ "view" ] = "cockpit";
	Cfg.Settings[ "spectator_view" ] = "auto";
	
	Cfg.Settings[ "g_bg" ] = "true";
	Cfg.Settings[ "g_stars" ] = "0";
	Cfg.Settings[ "g_debris" ] = "500";
	Cfg.Settings[ "g_deathstar_detail" ] = "3";
	Cfg.Settings[ "g_crosshair_thickness" ] = "1.5";
	
	Cfg.Settings[ "s_menu_music" ] = "true";
	Cfg.Settings[ "s_game_music" ] = "true";
	Cfg.Settings[ "s_imuse" ] = "false";
	
	Cfg.Settings[ "joy_enable" ] = "true";
	Cfg.Settings[ "joy_swap_xz" ] = "false";
	Cfg.Settings[ "joy_deadzone" ] = "0.03";
	Cfg.Settings[ "joy_deadzone_thumbsticks" ] = "0.1";
	Cfg.Settings[ "joy_deadzone_triggers" ] = "0.02";
	Cfg.Settings[ "joy_smooth_x" ] = "0.125";
	Cfg.Settings[ "joy_smooth_y" ] = "0.125";
	Cfg.Settings[ "joy_smooth_z" ] = "0.5";
	Cfg.Settings[ "joy_smooth_pedals" ] = "0";
	Cfg.Settings[ "joy_smooth_thumbsticks" ] = "1";
	Cfg.Settings[ "joy_smooth_triggers" ] = "0.75";
	
	Cfg.Settings[ "mouse_mode" ] = "disabled";
	Cfg.Settings[ "mouse_invert" ] = "true";
	Cfg.Settings[ "mouse_smooth" ] = "0.25";
	
	#ifdef APPLE_POWERPC
		Cfg.Settings[ "g_dynamic_lights" ] = "1";
		Cfg.Settings[ "g_debris" ] = "200";
		Cfg.Settings[ "g_deathstar_detail" ] = "2";
	#endif
}


void XWingGame::Setup( int argc, char **argv )
{
	bool screensaver = false;
	if( Cfg.SettingAsBool("screensaver") )
	{
		screensaver = true;
		Cfg.Settings[ "s_menu_music" ] = "false";
		Cfg.Settings[ "s_game_music" ] = "false";
		Cfg.Settings[ "saitek_enable" ] = "false";
		Cfg.Settings[ "spectator_view" ] = "cinema2";
		int maxfps = Cfg.SettingAsInt( "maxfps", 60 );
		Cfg.Settings[ "sv_netrate" ] = Cfg.Settings[ "sv_maxfps" ] = (maxfps > 60) ? Num::ToString(maxfps) : "60";
	}
	
	bool safemode = false;
	for( int i = 1; i < argc; i ++ )
	{
		if( strcasecmp( argv[ i ], "-safe" ) == 0 )
			safemode = true;
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
	{
		// Add the main menu to the now-empty Layers stack.
		Layers.Add( new MainMenu() );
	}
	else
	{
		// Add an input handler to quit the screensaver when necessary.
		AddScreensaverLayer();
	}
	
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
	if( ! safemode )
		Precache();
}


void XWingGame::Precache( void )
{
	Res.GetAnimation("nebula.ani");
	Res.GetAnimation("explosion.ani");
	Res.GetAnimation("laser_red.ani");
	Res.GetAnimation("laser_green.ani");
	Res.GetModel("asteroid.obj");
	
	// Technically these are played at zero volume in the screensaver.
	Res.GetSound("laser_red.wav");
	Res.GetSound("laser_green.wav");
	Res.GetSound("explosion.wav");
	Res.GetSound("damage_hull.wav");
	Res.GetSound("damage_shield.wav");
	Res.GetSound("hit_hull.wav");
	Res.GetSound("hit_shield.wav");
	
	bool screensaver = Cfg.SettingAsBool("screensaver");
	if( ! screensaver )
	{
		Res.GetAnimation("stars.ani");
		Res.GetAnimation("bg_lobby.ani");
		
		Res.GetAnimation("special/health.ani");
		Res.GetAnimation("special/target.ani");
		Res.GetAnimation("special/intercept.ani");
		Res.GetAnimation("special/throttle.ani");
		
		Res.GetAnimation("shields_c.ani");
		Res.GetAnimation("shields_f.ani");
		Res.GetAnimation("shields_r.ani");
		
		Res.GetAnimation("torpedo.ani");
		Res.GetAnimation("missile.ani");
		Res.GetAnimation("deathstar.ani");
		
		Res.GetModel("turret_body.obj");
		Res.GetModel("turret_gun.obj");
		Res.GetModel("deathstar_detail.obj");
		Res.GetModel("deathstar_detail_bottom.obj");
		Res.GetModel("deathstar_box.obj");
		Res.GetModel("deathstar_exhaust_port.obj");
		
		Res.GetSound("beep.wav");
		Res.GetSound("locking.wav");
		Res.GetSound("locked.wav");
		Res.GetSound("chat.wav");
		Res.GetSound("incoming.wav");
		
		Res.GetSound("turbolaser_green.wav");
		Res.GetSound("torpedo.wav");
		Res.GetSound("torpedo_enter.wav");
		
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
		while( struct dirent *dir_entry_p = readdir(dir_p) )
		{
			if( ! dir_entry_p->d_name )
				continue;
			if( dir_entry_p->d_name[ 0 ] == '.' )
				continue;
			
			// FIXME: Look for a specific file extension.
			
			ShipClass sc;
			if( sc.Load( std::string("Ships/") + std::string(dir_entry_p->d_name) ) )
			{
				if( screensaver && (sc.ShortName != "X/W") && (sc.ShortName != "T/F") )
					continue;
				
				if( sc.ExternalModel.length() )
					Res.GetModel( sc.ExternalModel );
				if( sc.CockpitModel.length() && ! screensaver )
					Res.GetModel( sc.CockpitModel );
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
	Layers.Add( screensaver_layer );
}


void XWingGame::Update( double dt )
{
	// Update ship's motion changes based on client's controls.
	
	double roll = 0.;
	double pitch = 0.;
	double yaw = 0.;
	static double throttle = 0.;
	bool firing = false;
	
	bool target_crosshair = false;
	bool target_nearest_enemy = false;
	bool target_nearest_attacker = false;
	bool target_newest = false;
	bool target_nearest_incoming = false;
	bool target_nothing = false;
	
	double mouse_x_percent = Mouse.X * 2. / Gfx.W - 1.;
	double mouse_y_percent = Mouse.Y * 2. / Gfx.H - 1.;
	if( Cfg.SettingAsBool("mouse_correction") )
	{
		double min_dim = (Gfx.H < Gfx.W) ? Gfx.H : Gfx.W;
		mouse_x_percent *= Gfx.W / min_dim;
		mouse_y_percent *= Gfx.H / min_dim;
	}
	bool mouse_fly2 = (Cfg.SettingAsString("mouse_mode") == "fly2");
	
	if( ReadMouse )
	{
		// Read mouse position for pitch/yaw.
		if( (Cfg.SettingAsString("mouse_mode") == "fly") || mouse_fly2 )
		{
			if( mouse_fly2 )
				roll = (fabs(mouse_x_percent) <= 1.) ? mouse_x_percent : Num::Sign(mouse_x_percent);
			else
				yaw = (fabs(mouse_x_percent) <= 1.) ? mouse_x_percent : Num::Sign(mouse_x_percent);
			
			pitch = (fabs(mouse_y_percent) <= 1.) ? mouse_y_percent : Num::Sign(mouse_y_percent);
			
			// If mouse_invert=false, reverse the pitch.
			if( ! Cfg.SettingAsBool("mouse_invert",true) )
				pitch *= -1.;
			
			double smooth = Cfg.SettingAsDouble("mouse_smooth");
			yaw = fabs(pow( fabs(yaw), smooth + 1. )) * Num::Sign(yaw);
			pitch = fabs(pow( fabs(pitch), smooth + 1. )) * Num::Sign(pitch);
			
			// Read mouse buttons.
			if( Mouse.ButtonDown( SDL_BUTTON_LEFT ) )
				firing = true;
			if( Mouse.ButtonDown( SDL_BUTTON_RIGHT ) )
				target_crosshair = true;
			if( Mouse.ButtonDown( SDL_BUTTON_X1 ) )
				throttle -= FrameTime / 2.;
			if( Mouse.ButtonDown( SDL_BUTTON_X2 ) )
				throttle += FrameTime / 2.;
		}
		else if( (Cfg.SettingAsString("mouse_mode") == "look") && (! Head.VR) )
		{
			LookYaw = ( (fabs(mouse_x_percent) <= 1.) ? mouse_x_percent : Num::Sign(mouse_x_percent) ) * 180.;
			LookPitch = ( (fabs(mouse_y_percent) <= 1.) ? mouse_y_percent : Num::Sign(mouse_y_percent) ) * -90.;
			ThumbstickLook = false;
		}
	}
	
	if( Cfg.SettingAsBool("joy_enable",true) && Joy.Joysticks.size() )
	{
		double deadzone = Cfg.SettingAsDouble( "joy_deadzone", 0.03 );
		// FIXME: More distinct deadzone variables for different devices and axes?
		//bool stick = false;
		bool pedals = false;
		bool throttle_unit = false;
		bool joy_look = false;
		
		for( std::map<Uint8, JoystickState>::reverse_iterator joy_iter = Joy.Joysticks.rbegin(); joy_iter != Joy.Joysticks.rend(); joy_iter ++ )
		{
			if( Str::FindInsensitive( joy_iter->second.Name, "Pedal" ) >= 0 )
			{
				bool found_axis = false;
				double z = 0.;
				
				if( joy_iter->second.HasAxis(2) && ! Cfg.SettingAsBool("joy_separate_pedals",false) )
				{
					// Combined pedal axis (typical slider pedals).
					found_axis = true;
					z = joy_iter->second.Axis( 2, deadzone );
				}
				else if( joy_iter->second.HasAxis(0) || joy_iter->second.HasAxis(1) )
				{
					// Separate left/right (toe pedals).
					found_axis = true;
					double l = joy_iter->second.HasAxis(0) ? joy_iter->second.AxisScaled( 0, 0.,1., 0.,deadzone ) : 0.;
					double r = joy_iter->second.HasAxis(1) ? joy_iter->second.AxisScaled( 1, 0.,1., 0.,deadzone ) : 0.;
					z = r - l;
				}
				
				if( found_axis )
				{
					pedals = true;
					z = fabs(pow( fabs(z), Cfg.SettingAsDouble("joy_smooth_pedals") + 1. )) * Num::Sign(z);
					
					if( Cfg.SettingAsBool("joy_swap_xz") )
						roll = z;
					else
						yaw = z;
				}
			}
			else if( (Str::FindInsensitive( joy_iter->second.Name, "Throttle" ) >= 0) && joy_iter->second.HasAxis(0) )
			{
				throttle = joy_iter->second.AxisScaled( 0, 1.,0., 0.,deadzone );
				throttle_unit = true;
			}
			else if( Str::FindInsensitive( joy_iter->second.Name, "Xbox" ) >= 0 )
			{
				double deadzone_thumbsticks = Cfg.SettingAsDouble( "joy_deadzone_thumbsticks", 0.1 );
				double deadzone_triggers = Cfg.SettingAsDouble( "joy_deadzone_triggers", 0.02 );
				
				// Read controller's triggers for roll.
				double z = -1. * joy_iter->second.Axis( 2, deadzone_triggers );
				z = fabs(pow( fabs(z), Cfg.SettingAsDouble("joy_smooth_triggers") + 1. )) * Num::Sign(z);
				roll += z;
				
				// Read controller's thumbsticks.
				double smooth = Cfg.SettingAsDouble( "joy_smooth_thumbsticks", 1. );
				double x = joy_iter->second.Axis( 0, deadzone_thumbsticks );
				x = fabs(pow( fabs(x), smooth + 1. )) * Num::Sign(x);
				yaw += x;
				double y = joy_iter->second.Axis( 1, deadzone_thumbsticks );
				y = fabs(pow( fabs(y), smooth + 1. )) * Num::Sign(y);
				pitch += y;
				double look_yaw = joy_iter->second.Axis( 4, deadzone_thumbsticks );
				double look_pitch = joy_iter->second.Axis( 3, deadzone_thumbsticks );
				if( (fabs(look_yaw) > 0.5) || (fabs(look_pitch) > 0.5) )
					ThumbstickLook = true;
				if( ThumbstickLook )
				{
					LookYaw = 180. * fabs(pow( fabs(look_yaw), smooth + 1. )) * Num::Sign(look_yaw);
					LookPitch = -90. * fabs(pow( fabs(look_pitch), smooth + 1. )) * Num::Sign(look_pitch);
				}
				
				// Read controller's buttons.
				if( joy_iter->second.ButtonDown( 0 ) ) // A
					throttle -= FrameTime / 2.;
				if( joy_iter->second.ButtonDown( 2 ) ) // X
					throttle += FrameTime / 2.;
				if( joy_iter->second.ButtonDown( 4 ) ) // LB
					target_crosshair = true;
				if( joy_iter->second.ButtonDown( 5 ) ) // RB
					firing = true;
				
				// Read controller's D-pad.
				if( joy_iter->second.HatDir( 0, SDL_HAT_UP ) )
					target_nearest_attacker = true;
				if( joy_iter->second.HatDir( 0, SDL_HAT_DOWN ) )
					target_nearest_enemy = true;
			}
			else if( Str::FindInsensitive( joy_iter->second.Name, "F16 MFD" ) >= 0 )
			{
				if( joy_iter->second.ButtonDown( 0 ) ) // Top #1
					target_crosshair = true;
				if( joy_iter->second.ButtonDown( 15 ) ) // Left #5
					target_nothing = true;
				if( joy_iter->second.ButtonDown( 12 ) ) // Bottom #3
					target_nearest_enemy = true;
				if( joy_iter->second.ButtonDown( 11 ) ) // Bottom #4
					target_nearest_attacker = true;
				if( joy_iter->second.ButtonDown( 10 ) ) // Bottom #5
					target_nearest_incoming = true;
				if( joy_iter->second.ButtonDown( 4 ) ) // Top #5
					target_newest = true;
			}
			else
			{
				// Only read axis data if this is an analog stick (not a button pad).
				if( joy_iter->second.HasAxis(0) || joy_iter->second.HasAxis(1) || joy_iter->second.HasAxis(2) || joy_iter->second.HasAxis(3) )
				{
					//stick = true;
					
					// Read joystick's analog axes.
					double x = joy_iter->second.Axis( 0, deadzone );
					x = fabs(pow( fabs(x), Cfg.SettingAsDouble("joy_smooth_x") + 1. )) * Num::Sign(x);
					double y = joy_iter->second.Axis( 1, deadzone );
					y = fabs(pow( fabs(y), Cfg.SettingAsDouble("joy_smooth_y") + 1. )) * Num::Sign(y);
					double z = joy_iter->second.Axis( 3, deadzone );
					z = fabs(pow( fabs(z), Cfg.SettingAsDouble("joy_smooth_z") + 1. )) * Num::Sign(z);
					
					pitch = y;
					if( ! throttle_unit )
						throttle = joy_iter->second.AxisScaled( 2, 1.,0., 0.,deadzone );
					
					if( Cfg.SettingAsBool("joy_swap_xz") )
					{
						yaw = x;
						if( ! pedals )
							roll = z;
					}
					else
					{
						roll = x;
						if( ! pedals )
							yaw = z;
					}
				}
				
				// Read joystick's buttons.
				if( joy_iter->second.ButtonDown( 0 ) ) // Trigger
					firing = true;
				if( joy_iter->second.ButtonDown( 2 ) ) // A
					target_crosshair = true;
				if( joy_iter->second.ButtonDown( 3 ) ) // B
					target_nearest_enemy = true;
				if( joy_iter->second.ButtonDown( 5 ) ) // Pinkie
				{
					LookPitch = 0.;
					LookYaw = 0.;
					joy_look = true;
					Head.Recenter();
				}
				if( joy_iter->second.ButtonDown( 7 ) ) // E
					target_nearest_attacker = true;
				if( joy_iter->second.ButtonDown( 30 ) ) // Clutch
					target_nearest_incoming = true;
				
				// Read joystick's hat position.
				if( joy_iter->second.HatDir( 0, SDL_HAT_UP ) )
				{
					LookPitch += 90. * FrameTime;
					joy_look = true;
				}
				if( joy_iter->second.HatDir( 0, SDL_HAT_DOWN ) )
				{
					LookPitch -= 90. * FrameTime;
					joy_look = true;
				}
				if( joy_iter->second.HatDir( 0, SDL_HAT_LEFT ) )
				{
					LookYaw -= 90. * FrameTime;
					joy_look = true;
				}
				if( joy_iter->second.HatDir( 0, SDL_HAT_RIGHT ) )
				{
					LookYaw += 90. * FrameTime;
					joy_look = true;
				}
			}
		}
		
		if( joy_look )
			ThumbstickLook = false;
	}
	
	if( ReadKeyboard && ! Console.IsActive() )
	{
		if( Keys.KeyDown(SDLK_UP) )
			pitch -= 1.;
		if( Keys.KeyDown(SDLK_DOWN) )
			pitch += 1.;
		if( Keys.KeyDown(SDLK_LEFT) )
			yaw -= 1.;
		if( Keys.KeyDown(SDLK_RIGHT) )
			yaw += 1.;
		if( Keys.KeyDown(SDLK_d) )
		{
			if( mouse_fly2 )
				yaw -= 1.;
			else
				roll -= 1.;
		}
		if( Keys.KeyDown(SDLK_f) )
		{
			if( mouse_fly2 )
				yaw += 1.;
			else
				roll += 1.;
		}
		
		if( Keys.KeyDown(SDLK_BACKSPACE) )
			throttle = 1.;
		else if( Keys.KeyDown(SDLK_RIGHTBRACKET) )
			throttle = 0.6667;
		else if( Keys.KeyDown(SDLK_LEFTBRACKET) )
			throttle = 0.3333;
		else if( Keys.KeyDown(SDLK_BACKSLASH) )
			throttle = 0.;
		else if( Keys.KeyDown(SDLK_MINUS) )
			throttle -= FrameTime / 2.;
		else if( Keys.KeyDown(SDLK_EQUALS) )
			throttle += FrameTime / 2.;
		else if( Keys.KeyDown(SDLK_a) )
			throttle += FrameTime / 2.;
		else if( Keys.KeyDown(SDLK_z) )
			throttle -= FrameTime / 2.;
		
		if( Keys.KeyDown(SDLK_SPACE) )
			firing = true;
		if( Keys.KeyDown(SDLK_KP7) || Keys.KeyDown(SDLK_KP8) || Keys.KeyDown(SDLK_KP9) )
		{
			LookPitch += 90. * FrameTime;
			ThumbstickLook = false;
		}
		if( Keys.KeyDown(SDLK_KP1) || Keys.KeyDown(SDLK_KP2) || Keys.KeyDown(SDLK_KP3) )
		{
			LookPitch -= 90. * FrameTime;
			ThumbstickLook = false;
		}
		if( Keys.KeyDown(SDLK_KP1) || Keys.KeyDown(SDLK_KP4) || Keys.KeyDown(SDLK_KP7) )
		{
			LookYaw -= 90. * FrameTime;
			ThumbstickLook = false;
		}
		if( Keys.KeyDown(SDLK_KP3) || Keys.KeyDown(SDLK_KP6) || Keys.KeyDown(SDLK_KP9) )
		{
			LookYaw += 90. * FrameTime;
			ThumbstickLook = false;
		}
		if( Keys.KeyDown(SDLK_KP5) )
		{
			LookPitch = 0.;
			LookYaw = 0.;
			ThumbstickLook = false;
			Head.Recenter();
		}
		
		if( Keys.KeyDown(SDLK_LCTRL) )
			target_crosshair = true;
		if( Keys.KeyDown(SDLK_e) )
			target_nearest_attacker = true;
		if( Keys.KeyDown(SDLK_r) )
			target_nearest_enemy = true;
		if( Keys.KeyDown(SDLK_u) )
			target_newest = true;
		if( Keys.KeyDown(SDLK_i) )
			target_nearest_incoming = true;
		if( Keys.KeyDown(SDLK_q) )
			target_nothing = true;
	}
	
	// Build a list of all ships, because we'll refer to it often.
	std::list<Ship*> ships;
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->Type() == XWing::Object::SHIP )
			ships.push_back( (Ship*) obj_iter->second );
	}
	
	// Look for player's ship.
	Ship *my_ship = NULL;
	for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
	{
		if( (*ship_iter)->PlayerID == PlayerID )
		{
			my_ship = *ship_iter;
			break;
		}
	}
	
	
	// Make sure the throttle value is legit.
	if( throttle > 1. )
		throttle = 1.;
	else if( throttle < 0. )
		throttle = 0.;
	
	
	// Apply controls to player's ship.
	
	if( my_ship )
	{
		uint32_t target_id = my_ship->Target;
		
		if( my_ship->Health > 0. )
		{
			bool beep = false;
			
			my_ship->SetRoll( roll, dt );
			my_ship->SetPitch( pitch, dt );
			my_ship->SetYaw( yaw, dt );
			my_ship->SetThrottle( throttle, dt );
			my_ship->Firing = firing;
			
			
			// Apply targeting.
			
			if( target_crosshair )
			{
				double best = 0.;
				uint32_t id = 0;
				
				for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					if( (*ship_iter)->ID != my_ship->ID )
					{
						Ship *ship = *ship_iter;
						
						if( ship->Health <= 0. )
							continue;
						
						Vec3D vec_to_ship( ship->X - my_ship->X, ship->Y - my_ship->Y, ship->Z - my_ship->Z );
						vec_to_ship.ScaleTo( 1. );
						double dot = my_ship->Fwd.Dot( &vec_to_ship );
						if( (dot > 0.) && ((dot > best) || (! id)) )
						{
							best = dot;
							id = ship->ID;
						}
					}
				}
				
				if( target_id != id )
				{
					// Note: This clears the current target if no match was found.
					target_id = id;
					beep = true;
				}
			}
			else if( target_nearest_enemy || target_nearest_attacker )
			{
				double best = 0.;
				uint32_t id = 0;
				
				for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					if( (*ship_iter)->ID != my_ship->ID )
					{
						Ship *ship = *ship_iter;
						
						if( ship->Health <= 0. )
							continue;
						if( my_ship->Team && (ship->Team == my_ship->Team) )
							continue;
						if( target_nearest_attacker && (ship->Target != my_ship->ID) )
							continue;
						
						double dist = my_ship->Dist( ship );
						if( (dist < best) || (! id) )
						{
							best = dist;
							id = ship->ID;
						}
					}
				}
				
				if( target_id != id )
				{
					// Note: This clears the current target if no match was found.
					target_id = id;
					beep = true;
				}
			}
			else if( target_newest )
			{
				double best = 0.;
				uint32_t id = 0;
				
				for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					if( (*ship_iter)->ID != my_ship->ID )
					{
						Ship *ship = *ship_iter;
						
						if( ship->Health <= 0. )
							continue;
						
						double time = ship->Lifetime.ElapsedSeconds();
						if( (time < best) || (! id) )
						{
							best = time;
							id = ship->ID;
						}
					}
				}
				
				if( target_id != id )
				{
					target_id = id;
					beep = true;
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
						
						if( shot->Seeking != my_ship->ID )
							continue;
						
						double dist = my_ship->Dist( shot );
						if( (dist < best) || (! id) )
						{
							best = dist;
							id = shot->ID;
						}
					}
				}
				
				if( target_id != id )
				{
					// Note: This clears the current target if no match was found.
					target_id = id;
					beep = true;
				}
			}
			else if( target_nothing )
			{
				if( target_id )
				{
					target_id = 0;
					beep = true;
				}
			}
			
			
			// If we did anything that calls for a beep, play the sound now.
			if( beep )
				Snd.Play( Res.GetSound("beep.wav") );
			
			
			// Unselect dead target before it can respawn.
			if( target_id )
			{
				GameObject *target_obj = Data.GetObject( target_id );
				if( target_obj )
				{
					if( target_obj->Type() == XWing::Object::SHIP )
					{
						Ship *target_ship = (Ship*) target_obj;
						if( (target_ship->Health < 0.) && (target_ship->DeathClock.ElapsedSeconds() > 4.) )
							target_id = 0;
					}
				}
				else
					target_id = 0;
			}
		}
		else
		{
			my_ship->RollRate = 0.;
			my_ship->PitchRate = 0.;
			my_ship->YawRate = 0.;
			my_ship->Firing = false;
			target_id = 0;
		}
		
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
			
			// Check for ship flybys.
			if( (obj_iter->second->Type() == XWing::Object::SHIP) && (Cam.Dist( obj_iter->second ) < 67.) && (obj_iter->second->MotionVector.Length() >= 20.) )
			{
				Ship *ship = (Ship*) obj_iter->second;
				
				// Dead ships tell no tales.
				if( ship->Health <= 0. )
					continue;
				
				// Different sounds for different speeds and ships.
				double speed = relative_motion.Length();
				const char *sound = ship->FlybySound( speed );
				if( sound )
					Snd.PlayFromObject( Res.GetSound(sound), obj_iter->first, (speed > 100.) ? 10. : 5. );
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
			
			if( (! found_music) && observed_ship && (Data.Properties["gametype"] == "yavin") )
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
}


bool XWingGame::HandleEvent( SDL_Event *event )
{
	bool handled = false;
	
	
	// Figure out if this is a key/button-down event, and what kind.
	
	bool joy = false, joy_hat = false, key = false, mouse = false;
	int button = 0, joy_num = 0, hat_dir = 0;
	bool joy_xbox = false, joy_mfd = false;
	if( event->type == SDL_JOYBUTTONDOWN )
	{
		if( Cfg.SettingAsBool("joy_enable") )
		{
			joy = true;
			joy_num = event->jbutton.which;
			button = event->jbutton.button;
			joy_xbox = (Str::FindInsensitive( Joy.Joysticks[ joy_num ].Name, "Xbox" ) >= 0);
			joy_mfd = (Str::FindInsensitive( Joy.Joysticks[ joy_num ].Name, "F16 MFD" ) >= 0);
		}
	}
	else if( event->type == SDL_JOYHATMOTION )
	{
		if( Cfg.SettingAsBool("joy_enable") )
		{
			joy_hat = true;
			joy_num = event->jbutton.which;
			button = event->jhat.hat;
			hat_dir = event->jhat.value;
			joy_xbox = (Str::FindInsensitive( Joy.Joysticks[ joy_num ].Name, "Xbox" ) >= 0);
		}
	}
	else if( event->type == SDL_KEYDOWN )
	{
		if( ! Console.IsActive() )
		{
			key = true;
			button = event->key.keysym.sym;
		}
	}
	else if( event->type == SDL_MOUSEBUTTONDOWN )
	{
		mouse = true;
		button = event->button.button;
	}
	
	
	// See if we can handle this.
	
	if( (State >= XWing::State::FLYING) && (joy || joy_hat || key || mouse) )
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
		bool observe_next = false;
		bool observe_prev = false;
		
		if( joy )
		{
			if( !( joy_xbox || joy_mfd ) )
			{
				// Joystick Button
				
				handled = true;
				
				if( button == 1 ) // Fire
					weapon_next = true;
				else if( button == 4 ) // C
					firing_mode_next = true;
				else if( button == 6 ) // D
					shield_shunt = true;
				else if( button == 8 ) // T1
					target_prev = true;
				else if( button == 9 ) // T2
					target_next = true;
				else if( button == 10 ) // T3
					target_prev_friendly = true;
				else if( button == 11 ) // T4
					target_next_friendly = true;
				else if( button == 12 ) // T5
					target_prev_enemy = true;
				else if( button == 13 ) // T6
					target_next_enemy = true;
				else if( button == 19 ) // Hat 2 Up
					target_prev_friendly = true;
				else if( button == 21 ) // Hat 2 Down
					target_next_friendly = true;
				else if( button == 22 ) // Hat 2 Left
					target_prev_enemy = true;
				else if( button == 20 ) // Hat 2 Right
					target_next_enemy = true;
				else if( button == 23 ) // Hat 3 Up
					target_prev_friendly = true;
				else if( button == 25 ) // Hat 3 Down
					target_next_friendly = true;
				else if( button == 26 ) // Hat 3 Left
					target_prev_enemy = true;
				else if( button == 24 ) // Hat 3 Right
					target_next_enemy = true;
				else
					handled = false;
			}
			else if( joy_xbox )
			{
				// Xbox Button
				
				handled = true;
				
				if( button == 1 ) // B
					firing_mode_next = true;
				else if( button == 3 ) // Y
					weapon_next = true;
				else if( button == 6 ) // Back
					shield_shunt = true;
				else if( button == 7 ) // Start
					shield_shunt = true;
				else
					handled = false;
			}
			else
			{
				// MFD Button
				
				handled = true;
				
				if( button == 19 ) // Left #1
					target_prev = true;
				else if( button == 18 ) // Left #2
					target_next = true;
				else if( button == 17 ) // Left #3
					target_prev_friendly = true;
				else if( button == 16 ) // Left #4
					target_next_friendly = true;
				else if( button == 14 ) // Bottom #1
					target_prev_enemy = true;
				else if( button == 13 ) // Bottom #2
					target_next_enemy = true;
				else if( button == 5 ) // Right #1
					shield_shunt = true;
				else if( button == 8 ) // Right #4
					weapon_next = true;
				else if( button == 9 ) // Right #5
					firing_mode_next = true;
				else
					handled = false;
			}
		}
		else if( joy_hat )
		{
			if( joy_xbox && (button == 0) )
			{
				// Xbox D-Pad
				
				handled = true;
				
				if( hat_dir & SDL_HAT_LEFT )
					target_prev_enemy = true;
				else if( hat_dir & SDL_HAT_RIGHT )
					target_next_enemy = true;
				else
					handled = false;
			}
		}
		else if( key )
		{
			// Keyboard
			
			handled = true;
			
			if( button == SDLK_w )
				weapon_next = true;
			else if( button == SDLK_x )
				firing_mode_next = true;
			else if( button == SDLK_s )
				shield_shunt = true;
			else if( button == SDLK_t )
				target_next = true;
			else if( button == SDLK_y )
				target_prev = true;
			else if( button == SDLK_F1 )
			{
				if( Keys.KeyDown(SDLK_LSHIFT) )
					target_prev_friendly = true;
				else
					target_next_friendly = true;
			}
			else if( button == SDLK_F3 )
			{
				if( Keys.KeyDown(SDLK_LSHIFT) )
					target_prev_enemy = true;
				else
					target_next_enemy = true;
			}
			else if( button == SDLK_LEFTBRACKET )
				observe_prev = true;
			else if( button == SDLK_RIGHTBRACKET )
				observe_next = true;
			else
				handled = false;
		}
		else if( mouse )
		{
			handled = true;
			
			if( button == SDL_BUTTON_MIDDLE )
				weapon_next = true;
			else if( button == SDL_BUTTON_WHEELDOWN )
				target_next = true;
			else if( button == SDL_BUTTON_WHEELUP )
				target_prev = true;
			else
				handled = false;
		}
		
		// Build a list of all ships, because we'll refer to it often.
		std::list<Ship*> ships;
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHIP )
				ships.push_back( (Ship*) obj_iter->second );
		}
		
		// Look for player's ship.
		Ship *my_ship = NULL;
		for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
		{
			if( (*ship_iter)->PlayerID == PlayerID )
			{
				my_ship = *ship_iter;
				break;
			}
		}
		
		
		// Apply controls.
		
		if( my_ship && (my_ship->Health > 0.) )
		{
			bool beep = false;
			
			if( weapon_next )
				beep = my_ship->NextWeapon();
			else if( firing_mode_next )
				beep = my_ship->NextFiringMode();
			else if( shield_shunt )
			{
				// Set shield position.
				
				if( my_ship->ShieldPos == Ship::SHIELD_CENTER )
					my_ship->SetShieldPos( Ship::SHIELD_FRONT );
				else if( my_ship->ShieldPos == Ship::SHIELD_FRONT )
					my_ship->SetShieldPos( Ship::SHIELD_REAR );
				else
					my_ship->SetShieldPos( Ship::SHIELD_CENTER );
				
				if( my_ship->MaxShield() > 0. )
					beep = true;
			}
			else if( target_next || target_prev || target_next_enemy || target_prev_enemy || target_next_friendly || target_prev_friendly )
			{
				// Apply targeting.
				
				std::map<uint32_t,Ship*> potential_targets;
				for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					if( (*ship_iter)->ID != my_ship->ID )
					{
						Ship *ship = *ship_iter;
						
						if( ship->Health <= 0. )
							continue;
						if( (target_next_enemy || target_prev_enemy) && my_ship->Team && (ship->Team == my_ship->Team) )
							continue;
						if( (target_next_friendly || target_prev_friendly) && ((! my_ship->Team) || (ship->Team != my_ship->Team)) )
							continue;
						
						potential_targets[ ship->ID ] = ship;
					}
				}
				
				std::map<uint32_t,Ship*>::iterator target_iter = potential_targets.find( my_ship->Target );
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
				
				if( (target_iter != potential_targets.end()) && (target_iter->second->ID != my_ship->Target) )
				{
					my_ship->UpdateTarget( target_iter->second );
					beep = true;
				}
				else
				{
					// Note: This clears the current target if no match was found.
					beep = my_ship->Target;
					my_ship->UpdateTarget( NULL );
				}
			}
			
			
			// If we did anything that calls for a beep, play the sound now.
			if( beep )
				Snd.Play( Res.GetSound("beep.wav") );
		}
		else if( observe_next || observe_prev )
		{
			// Player has no ship, and we're cycling through ones to observe.
			
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
	}
	
	return handled;
}


bool XWingGame::HandleCommand( std::string cmd, std::vector<std::string> *params )
{
	if( cmd == "pew" )
	{
		if( Raptor::Game->Res.SearchPath.front() == "Sounds/Silly" )
		{
			Raptor::Game->Res.SearchPath.pop_front();
			Raptor::Game->Console.Print( "Silly sounds disabled." );
		}
		else
		{
			Raptor::Game->Res.SearchPath.push_front( "Sounds/Silly" );
			Raptor::Game->Console.Print( "Silly sounds enabled!" );
		}
		
		Raptor::Game->Snd.StopSounds();
		Raptor::Game->Res.DeleteSounds();
		
		return true;
	}
	
	return false;
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
					if( player && (player->Properties["team"] == "Spectator") )
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
			if( music && Cfg.SettingAsBool("s_game_music") )
			{
				Snd.StopMusic();
				Snd.PlayMusicOnce( music );
			}
		}
		
		ChangeState( XWing::State::ROUND_ENDED );
		
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
		if( Raptor::Server->IsRunning() )
		{
			Raptor::Server->Data.Players[ PlayerID ]->Properties["ship"] = "Spectator";
			Raptor::Server->Data.Properties["gametype"] = "team_dm";
			Raptor::Server->Data.Properties["dm_kill_limit"] = "0";
			Raptor::Server->Data.Properties["tdm_kill_limit"] = "0";
			Raptor::Server->Data.Properties["ai_waves"] = "4";
			Raptor::Server->Data.Properties["ai_flock"] = "true";
			Raptor::Server->Data.Properties["asteroids"] = "32";
			Raptor::Server->Data.Properties["bg"] = "nebula";
			Data.Properties["bg"] = "nebula";
			((XWingServer*)( Raptor::Server ))->BeginFlying();
		}
		else
		{
			// Screensaver activated when there was already a local server running.
			
			Packet player_properties = Packet( Raptor::Packet::PLAYER_PROPERTIES );
			player_properties.AddUShort( Raptor::Game->PlayerID );
			player_properties.AddUInt( 2 );
			player_properties.AddString( "name" );
			player_properties.AddString( "Screensaver" );
			player_properties.AddString( "ship" );
			player_properties.AddString( "Spectator" );
			Raptor::Game->Net.Send( &player_properties );
			
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
