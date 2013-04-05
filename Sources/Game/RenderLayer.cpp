/*
 *  RenderLayer.cpp
 */

#include "RenderLayer.h"

#include <cmath>
#include "Graphics.h"
#include "Rand.h"
#include "RaptorGame.h"
#include "MessageOverlay.h"
#include "IngameMenu.h"
#include "XWingDefs.h"
#include "XWingGame.h"
#include "Ship.h"
#include "Shot.h"
#include "Num.h"
#include "Math3D.h"
#include "Shader.h"
#include "ShaderManager.h"


enum
{
	VIEW_COCKPIT,
	VIEW_CHASE,
	VIEW_CINEMA,
	VIEW_CINEMA2
};


RenderLayer::RenderLayer( void )
{
	Rect.x = 0;
	Rect.w = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	if( ! Raptor::Game->Data.Properties["bg"].empty() )
		Background.BecomeInstance( Raptor::Game->Res.GetAnimation( Raptor::Game->Data.Properties["bg"] + std::string(".ani") ) );
	else
		Background.BecomeInstance( Raptor::Game->Res.GetAnimation("stars.ani") );
	
	BigFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 24 );
	SmallFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 16 );
	RadarDirectionFont = Raptor::Game->Res.GetFont( "ProFont.ttf", 22 );
	ScreenFont = Raptor::Game->Res.GetFont( "ProFont.ttf", 48 );
	
	for( int i = 0; i < STAR_COUNT; i ++ )
		Stars[ i ].Set( Rand::Double( -10000., 10000. ), Rand::Double( -10000., 10000. ), Rand::Double( -10000., 10000. ) );
	
	for( int i = 0; i < DEBRIS_COUNT; i ++ )
		Debris[ i ].SetPos( Rand::Double( -DEBRIS_DIST, DEBRIS_DIST ), Rand::Double( -DEBRIS_DIST, DEBRIS_DIST ), Rand::Double( -DEBRIS_DIST, DEBRIS_DIST ) );
	
	if( ! Raptor::Game->Cfg.SettingAsBool("screensaver") )
		AddElement( new MessageOverlay( this, Raptor::Game->Res.GetFont( "TimesNR.ttf", 16 ) ) );
	
	AddElement( MessageInput = new TextBox( this, NULL, Raptor::Game->Res.GetFont( "TimesNR.ttf", 17 ), Font::ALIGN_TOP_LEFT ) );
	MessageInput->ReturnDeselects = false;
	MessageInput->PassReturn = true;
	MessageInput->EscDeselects = false;
	MessageInput->PassEsc = true;
	MessageInput->Visible = false;
	MessageInput->TextRed = 1.f;
	MessageInput->TextGreen = 1.f;
	MessageInput->TextBlue = 1.f;
	MessageInput->TextAlpha = 0.5f;
	MessageInput->SelectedTextRed = 1.f;
	MessageInput->SelectedTextGreen = 1.f;
	MessageInput->SelectedTextBlue = 0.5f;
	MessageInput->SelectedTextAlpha = 1.f;
	MessageInput->Red = 0.f;
	MessageInput->Green = 0.f;
	MessageInput->Blue = 1.f;
	MessageInput->Alpha = 0.75f;
	MessageInput->SelectedRed = MessageInput->Red;
	MessageInput->SelectedGreen = MessageInput->Green;
	MessageInput->SelectedBlue = MessageInput->Blue;
	MessageInput->SelectedAlpha = MessageInput->Alpha;
	
	((XWingGame*)( Raptor::Game ))->ObservedShipID = 0;
}


RenderLayer::~RenderLayer()
{
}


void RenderLayer::Draw( void )
{
	glPushMatrix();
	
	
	// Update contained elements.
	
	if( Raptor::Game->Console.IsActive() )
	{
		Selected = NULL;
		MessageInput->Visible = false;
	}
	else if( MessageInput->IsSelected() )
	{
		MessageInput->Rect.x = 3;
		MessageInput->Rect.y = MessageInput->Rect.x;
		MessageInput->Rect.w = Rect.w - (MessageInput->Rect.x * 2);
		MessageInput->Rect.h = MessageInput->TextFont->GetHeight();
	}
	
	if( IsTop() )
	{
		Raptor::Game->Mouse.ShowCursor = false;
		((XWingGame*)( Raptor::Game ))->ReadMouse = true;
	}
	
	
	// Build a list of all ships, because we'll refer to it often.
	// Also build a list of shots for use when determining dynamic lights.
	
	std::list<Ship*> ships;
	std::list<Shot*> shots;
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->Type() == XWing::Object::SHIP )
			ships.push_back( (Ship*) obj_iter->second );
		else if( obj_iter->second->Type() == XWing::Object::SHOT )
			shots.push_back( (Shot*) obj_iter->second );
	}
	
	
	// Determine which ship we're observing.
	
	Ship *observed_ship = NULL;
	GameObject *target_obj = NULL;
	Ship *target = NULL;
	int view = VIEW_CINEMA;
	
	
	// Look for the player's ship.

	for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
	{
		if( (*ship_iter)->PlayerID == Raptor::Game->PlayerID )
		{
			observed_ship = *ship_iter;
			view = VIEW_COCKPIT;
			break;
		}
	}
	
	
	if( ! observed_ship )
	{
		// This player has no ship, let's watch somebody else.
		
		bool found = false, find_last = false;
		
		for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
		{
			// Don't spectate the Death Star exhaust port.
			if( (*ship_iter)->ShipType == Ship::TYPE_EXHAUST_PORT )
				continue;
			
			observed_ship = *ship_iter;
			
			// If we'd selected a specific ship to watch, keep going until we find it.
			if( (observed_ship->ID >= ((XWingGame*)( Raptor::Game ))->ObservedShipID) && (!find_last) )
			{
				found = true;
				break;
			}
		}
		
		if( ! found )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->ShipType == Ship::TYPE_EXHAUST_PORT )
					continue;
				
				observed_ship = *ship_iter;
				break;
			}
		}
	}
	
	
	// Determine which view we'll render.
	
	std::string view_str = Raptor::Game->Cfg.SettingAsString("view");
	if( view != VIEW_COCKPIT )
		view_str = Raptor::Game->Cfg.SettingAsString("spectator_view");
	
	if( view_str == "cockpit" )
		view = VIEW_COCKPIT;
	else if( view_str == "chase" )
		view = VIEW_CHASE;
	else if( view_str == "cinema" )
		view = VIEW_CINEMA;
	else if( view_str == "cinema2" )
		view = VIEW_CINEMA2;
	else if( view_str == "cycle" )
	{
		double cycle_time = Raptor::Game->Cfg.SettingAsDouble("view_cycle_time");
		if( cycle_time <= 0. )
			cycle_time = 7.;
		
		double cam_picker = observed_ship ? fmod( observed_ship->Lifetime.ElapsedSeconds(), cycle_time * 4. ) : 0.;
		if( cam_picker >= cycle_time * 3. )
			view = VIEW_CHASE;
		else if( cam_picker >= cycle_time * 2. )
			view = VIEW_COCKPIT;
		else if( cam_picker >= cycle_time )
			view = VIEW_CINEMA2;
		else
			view = VIEW_CINEMA;
	}
	
	if( (view == VIEW_COCKPIT) && ((! observed_ship) || (observed_ship->Health <= 0.)) )
		view = VIEW_CHASE;
	
	
	Player *observed_player = NULL;
	Vec3D cam_motion_vec( 0., 0., 0. );
	
	if( observed_ship )
	{
		((XWingGame*)( Raptor::Game ))->ObservedShipID = observed_ship->ID;
		observed_player = Raptor::Game->Data.GetPlayer( observed_ship->PlayerID );
		
		if( observed_ship->Target )
		{
			target_obj = Raptor::Game->Data.GetObject( observed_ship->Target );
			if( target_obj && (target_obj->Type() == XWing::Object::SHIP) )
			{
				target = (Ship*) target_obj;
				if( target->Health <= 0. )
					target = NULL;
			}
		}
		
		
		Ship *cinema_view_with = NULL;
		if( (view == VIEW_CINEMA) || (view == VIEW_CINEMA2) )
		{
			double best = 0.;
			
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				if( (*ship_iter)->ID != observed_ship->ID )
				{
					Ship *ship = *ship_iter;
					
					if( ship->Team != observed_ship->Team )
					{
						double dist = observed_ship->Dist( ship );
						if( (dist < best) || (! cinema_view_with) )
						{
							best = dist;
							cinema_view_with = ship;
						}
					}
				}
			}
			
			if( ! cinema_view_with )
			{
				for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					if( (*ship_iter)->ID != observed_ship->ID )
					{
						Ship *ship = *ship_iter;
						
						double dist = observed_ship->Dist( ship );
						if( (dist < best) || (! cinema_view_with) )
						{
							best = dist;
							cinema_view_with = ship;
						}
					}
				}
			}
			
			if( ! cinema_view_with )
				view = VIEW_CHASE;
		}
		
		
		Raptor::Game->Cam.Copy( observed_ship );
		
		if( (view != VIEW_COCKPIT) || Raptor::Game->Cfg.SettingAsBool("g_3d_cockpit") )
		{
			// Apply camera angle change.
			Raptor::Game->Cam.Yaw( ((XWingGame*)( Raptor::Game ))->LookYaw );
			Raptor::Game->Cam.Pitch( ((XWingGame*)( Raptor::Game ))->LookPitch );
		}
		
		if( (view == VIEW_CINEMA) || (view == VIEW_CINEMA2) )
		{
			// Cinematic camera.
			
			if( view == VIEW_CINEMA2 )
				Raptor::Game->Cam.Up.Set( 0., 0., 1. );
			
			// Point the camera at one ship looking through to the other.
			Vec3D vec_to_other( cinema_view_with->X - observed_ship->X, cinema_view_with->Y - observed_ship->Y, cinema_view_with->Z - observed_ship->Z );
			vec_to_other.ScaleTo( 1. );
			Raptor::Game->Cam.Fwd = vec_to_other;
			Raptor::Game->Cam.FixVectors();
			
			// Apply camera angle change.
			Raptor::Game->Cam.Yaw( ((XWingGame*)( Raptor::Game ))->LookYaw );
			Raptor::Game->Cam.Pitch( ((XWingGame*)( Raptor::Game ))->LookPitch );
			
			// Move the camera up or down.
			Raptor::Game->Cam.MoveAlong( &(Raptor::Game->Cam.Fwd), -30. - observed_ship->Shape.GetTriagonal() );
			if( view == VIEW_CINEMA2 )
				Raptor::Game->Cam.MoveAlong( &(Raptor::Game->Cam.Up), observed_ship->Shape.GetTriagonal() * Vec3D(0.,0.,1.).Dot(&vec_to_other) + observed_ship->Shape.GetHeight() );
			else
				Raptor::Game->Cam.MoveAlong( &(Raptor::Game->Cam.Up), observed_ship->Shape.GetTriagonal() * observed_ship->Up.Dot(&vec_to_other) + observed_ship->Shape.GetHeight() );
			
			// Point the camera at the mid-point (in 3D space) between the two ships.
			Vec3D mid_point( (cinema_view_with->X + observed_ship->X) / 2., (cinema_view_with->Y + observed_ship->Y) / 2., (cinema_view_with->Z + observed_ship->Z) / 2. );
			Vec3D vec_to_mid( mid_point.X - Raptor::Game->Cam.X, mid_point.Y - Raptor::Game->Cam.Y, mid_point.Z - Raptor::Game->Cam.Z);
			vec_to_mid.ScaleTo( 1. );
			Raptor::Game->Cam.Fwd = vec_to_mid;
			Raptor::Game->Cam.FixVectors();
			
			// Apply camera angle change again.
			Raptor::Game->Cam.Yaw( ((XWingGame*)( Raptor::Game ))->LookYaw );
			Raptor::Game->Cam.Pitch( ((XWingGame*)( Raptor::Game ))->LookPitch );
		}
		
		else if( view == VIEW_CHASE )
		{
			// Chase camera.
			
			Raptor::Game->Cam.MoveAlong( &(Raptor::Game->Cam.Fwd), -30. - observed_ship->Shape.GetTriagonal() );
		}
		
		cam_motion_vec = observed_ship->MotionVector;
	}
	else
		((XWingGame*)( Raptor::Game ))->ObservedShipID = 0;
	
	
	// Set up shaders.
	
	bool use_shaders = Raptor::Game->Cfg.SettingAsBool("g_shader_enable");
	if( use_shaders )
	{
		Raptor::Game->ShaderMgr.ResumeShaders();
		Raptor::Game->ShaderMgr.Set3f( "CamPos", Raptor::Game->Cam.X, Raptor::Game->Cam.Y, Raptor::Game->Cam.Z );
		Raptor::Game->ShaderMgr.Set3f( "DynamicLightPos", 0., 0., 0. );
		Raptor::Game->ShaderMgr.Set3f( "DynamicLightColor", 0., 0., 0. );
		Raptor::Game->ShaderMgr.StopShaders();
	}
	
	
	// Render to textures before drawing anything else.
	
	bool use_framebuffers = false;
	
	#if !( defined(_ppc_) || defined(__ppc__) )
	{
		use_framebuffers = Raptor::Game->Cfg.SettingAsBool("g_framebuffers");
		if( use_framebuffers )
		{
			bool changed_framebuffer = false;
			
			if( observed_ship && (view == VIEW_COCKPIT) )
			{
				Framebuffer *health_framebuffer = Raptor::Game->Res.GetFramebuffer( "health" );
				if( health_framebuffer && health_framebuffer->Select() )
				{
					// Render hull and shield status.
					
					changed_framebuffer = true;
					Raptor::Game->Gfx.Clear();
					
					
					// Render shield display.
					
					health_framebuffer->Setup2D();
					
					GLuint shield_texture = Raptor::Game->Res.GetAnimation("shield.ani")->CurrentFrame();
					
					if( observed_ship->ShieldF > 0. )
					{
						float red = 0.f, green = 1.f, blue = 0.f;
						if( observed_ship->ShieldF >= (observed_ship->MaxShield() / 2.) )
							red = (observed_ship->MaxShield() - observed_ship->ShieldF) / (observed_ship->MaxShield() / 2.);
						else
						{
							red = 1.f;
							green = observed_ship->ShieldF / (observed_ship->MaxShield() / 2.);
						}
						
						Raptor::Game->Gfx.DrawRect2D( 0, 0, health_framebuffer->W, health_framebuffer->H / 2, shield_texture, red, green, blue, 1.f );
					}
					
					if( observed_ship->ShieldR > 0. )
					{
						float red = 0.f, green = 1.f, blue = 0.f;
						if( observed_ship->ShieldR >= (observed_ship->MaxShield() / 2.) )
							red = (observed_ship->MaxShield() - observed_ship->ShieldR) / (observed_ship->MaxShield() / 2.);
						else
						{
							red = 1.f;
							green = observed_ship->ShieldR / (observed_ship->MaxShield() / 2.);
						}
						
						Raptor::Game->Gfx.DrawRect2D( 0, health_framebuffer->H, health_framebuffer->W, health_framebuffer->H / 2, shield_texture, red, green, blue, 1.f );
					}
					
					
					// Render ship to display hull status.
					
					Camera above_cam( Raptor::Game->Cam );
					above_cam.Copy( observed_ship );
					above_cam.Fwd.Copy( &(observed_ship->Up) );
					above_cam.Fwd *= -1.;
					above_cam.Up.Copy( &(observed_ship->Fwd) );
					above_cam.Right.Copy( &(observed_ship->Right) );
					double ship_size = observed_ship->Shape.GetTriagonal();
					above_cam.MoveAlong( &(above_cam.Fwd), -2. * ship_size );
					above_cam.FOV = (observed_ship->MaxShield() > 0.) ? 33. : 20.;
					
					health_framebuffer->Setup3D( &(above_cam) );
					
					float red = -1.f, green = 1.f, blue = -1.f;
					if( observed_ship->Health < observed_ship->MaxHealth() )
						red = 1.;
					if( observed_ship->Health < (observed_ship->MaxHealth() * 0.7) )
						green = -1.;
					
					glColor4f( red, green, blue, 1.f );
					
					if( use_shaders )
					{
						Pos3D light( observed_ship->X, observed_ship->Y, observed_ship->Z );
						light.MoveAlong( &(observed_ship->Up), observed_ship->Shape.GetHeight() );
						Raptor::Game->ShaderMgr.ResumeShaders();
						Raptor::Game->ShaderMgr.Set3f( "DynamicLightPos", light.X, light.Y, light.Z );
						Raptor::Game->ShaderMgr.Set3f( "DynamicLightColor", red * ship_size * 7., green * ship_size * 7., blue * ship_size * 7. );
					}
					
					observed_ship->Shape.DrawAt( observed_ship );
					
					if( use_shaders )
					{
						Raptor::Game->ShaderMgr.Set3f( "DynamicLightPos", 0., 0., 0. );
						Raptor::Game->ShaderMgr.Set3f( "DynamicLightColor", 0., 0., 0. );
						Raptor::Game->ShaderMgr.StopShaders();
					}
					
					glColor4f( 1.f, 1.f, 1.f, 1.f );
					
					
					// Show shield direction.
					
					if( observed_ship->MaxShield() > 0. )
					{
						health_framebuffer->Setup2D();
						
						GLuint pos_texture = 0;
						if( observed_ship->ShieldPos == Ship::SHIELD_FRONT )
							pos_texture = Raptor::Game->Res.GetAnimation("shields_f.ani")->CurrentFrame();
						else if( observed_ship->ShieldPos == Ship::SHIELD_REAR )
							pos_texture = Raptor::Game->Res.GetAnimation("shields_r.ani")->CurrentFrame();
						else
							pos_texture = Raptor::Game->Res.GetAnimation("shields_c.ani")->CurrentFrame();
						
						Raptor::Game->Gfx.DrawRect2D( health_framebuffer->W - 42, 10, health_framebuffer->W - 10, 74, pos_texture );
					}
					
					
					// Draw noise if we're damaged.
					
					if( observed_ship->Health < (observed_ship->MaxHealth() * 0.5) )
					{
						health_framebuffer->Setup2D();
						
						glPointSize( 1.f );
						
						glBegin( GL_POINTS );
							
							float brightness = 1.;
							int dots = (observed_ship->MaxHealth() - observed_ship->Health) * 20.;
							for( int i = 0; i < dots; i ++ )
							{
								brightness = Rand::Double( 0.5, 1. );
								glColor4f( brightness, brightness, brightness, 1.f );
								glVertex2d( Rand::Int( 0, health_framebuffer->W ), Rand::Int( 0, health_framebuffer->H ) );
							}
							
						glEnd();
					}
				}
				
				
				Framebuffer *target_framebuffer = Raptor::Game->Res.GetFramebuffer( "target" );
				if( target_framebuffer && target_framebuffer->Select() )
				{
					// Render target display.
					
					changed_framebuffer = true;
					Raptor::Game->Gfx.Clear();
					
					if( target_obj && ((target_obj->Type() == XWing::Object::SHIP) || (target_obj->Type() == XWing::Object::SHOT)) )
					{
						// Draw target.
						
						double cam_dist = 75.;
						if( target )
							cam_dist = 2. * target->Shape.GetTriagonal();
						
						Vec3D vec_to_target( target_obj->X - observed_ship->X, target_obj->Y - observed_ship->Y, target_obj->Z - observed_ship->Z );
						Camera cam_to_target( Raptor::Game->Cam );
						cam_to_target.Fwd.Copy( &vec_to_target );
						cam_to_target.Fwd.ScaleTo( 1. );
						cam_to_target.Up.Copy( &(observed_ship->Up) );
						cam_to_target.FixVectors();
						cam_to_target.SetPos( target_obj->X, target_obj->Y, target_obj->Z );
						cam_to_target.MoveAlong( &(cam_to_target.Fwd), -cam_dist );
						cam_to_target.FOV = 30.;
						
						target_framebuffer->Setup3D( &(cam_to_target) );
						
						if( use_shaders )
							Raptor::Game->ShaderMgr.ResumeShaders();
						
						target_obj->Draw();
						
						if( use_shaders )
							Raptor::Game->ShaderMgr.StopShaders();
						
						
						target_framebuffer->Setup2D();
						
						float red = 1.f, green = 1.f, blue = 1.f;
						
						
						// Draw target name and determine status text.
						
						std::string target_name = "";
						std::string target_status = "";

						red = 1.f;
						green = 1.f;
						blue = 0.f;
						
						if( target )
						{
							target_name = target->Name;
							Player *target_player = Raptor::Game->Data.GetPlayer( target->PlayerID );
							if( target_player )
								target_name = target_player->Name;
							
							if( target->Health < (target->MaxHealth() * 0.7) )
								target_status = "Hull Damaged";
							else if( (target->MaxShield() > 0.) && ((target->ShieldF <= 0.) || (target->ShieldR <= 0.)) )
								target_status = "Shields Down";
							else
								target_status = "OK";
							
							if( observed_ship->Team && (target->Team == observed_ship->Team) )
							{
								red = 0.f;
								green = 1.f;
								blue = 0.f;
							}
							else
							{
								red = 1.f;
								green = 0.f;
								blue = 0.f;
							}
						}
						else
						{
							if( target_obj->Type() == XWing::Object::TURRET )
							{
								target_name = "Turbolaser";
							}
							else if( target_obj->Type() == XWing::Object::SHOT )
							{
								Shot *target_shot = (Shot*) target_obj;
								if( target_shot->ShotType == Shot::TYPE_TORPEDO )
									target_name = "Torpedo";
								else if( target_shot->ShotType == Shot::TYPE_MISSILE )
									target_name = "Missile";
								
								if( target_shot->Seeking )
								{
									GameObject *seeking_obj = Raptor::Game->Data.GetObject( target_shot->Seeking );
									if( seeking_obj && (seeking_obj->PlayerID == Raptor::Game->PlayerID) && (seeking_obj->Type() == XWing::Object::SHIP) )
										target_status = std::string("Incoming!");
								}
							}
						}
						
						ScreenFont->DrawText( target_name, target_framebuffer->W / 2, 10, Font::ALIGN_TOP_CENTER, red, green, blue, 1.f );
						
						
						// Draw target status (health).
						
						red = 1.f;
						green = 1.f;
						blue = 0.5f;
						
						ScreenFont->DrawText( target_status, target_framebuffer->W / 2, 10 + ScreenFont->PointSize, Font::ALIGN_TOP_CENTER, red, green, blue, 1.f );
						
						
						// Draw target distance.
						
						red = 0.5;
						green = 0.5;
						blue = 1.f;
						
						ScreenFont->DrawText( "Dist: ", 10, target_framebuffer->H - 10, Font::ALIGN_BOTTOM_LEFT, red, green, blue, 1.f );
						ScreenFont->DrawText( Num::ToString( (int) vec_to_target.Length() ), target_framebuffer->W - 10, target_framebuffer->H - 10, Font::ALIGN_BOTTOM_RIGHT, red, green, blue, 1.f );
					}
					
					
					// Draw noise if we're damaged.
					
					if( observed_ship->Health < (observed_ship->MaxHealth() * 0.5) )
					{
						target_framebuffer->Setup2D();
						
						glPointSize( 1.f );
						
						glBegin( GL_POINTS );
							
							float brightness = 1.;
							int dots = (observed_ship->MaxHealth() - observed_ship->Health) * 20.;
							for( int i = 0; i < dots; i ++ )
							{
								brightness = Rand::Double( 0.5, 1. );
								glColor4f( brightness, brightness, brightness, 1.f );
								glVertex2d( Rand::Int( 0, target_framebuffer->W ), Rand::Int( 0, target_framebuffer->H ) );
							}
							
						glEnd();
					}
				}
				
				
				Framebuffer *intercept_framebuffer = Raptor::Game->Res.GetFramebuffer( "intercept" );
				if( intercept_framebuffer && intercept_framebuffer->Select() )
				{
					// Render intercept display.
					
					changed_framebuffer = true;
					Raptor::Game->Gfx.Clear( 0.f, 0.25f, 0.f );
					
					
					// See if the crosshair is lined up so the next shot would hit.
					
					bool will_hit = false;
					Vec3D shot_vec;
					
					if( target || (target_obj && (target_obj->Type() == XWing::Object::SHOT)) )
					{
						std::map<int,Shot*> test_shots = observed_ship->NextShots();
						for( std::map<int,Shot*>::iterator shot_iter = test_shots.begin(); shot_iter != test_shots.end(); shot_iter ++ )
						{
							shot_vec.Copy( &(shot_iter->second->MotionVector) );
							if( target_obj->WillCollide( shot_iter->second, 4. ) )
								will_hit = true;
							
							delete shot_iter->second;
						}
					}
					
					
					// Draw the pointy bits.
					
					intercept_framebuffer->Setup2D( -1., -1., 1., 1. );
					
					float r = 0.f, g = 1.f, b = 0.f;
					
					if( will_hit )
					{
						r = 1.f;
						b = 1.f;
					}
					
					glColor4f( r, g, b, 1.f );
					
					glBegin( GL_LINES );
						
						// Top
						glVertex2d( -1.3, -2. );
						glVertex2d( 0., -0.25 );
						glVertex2d( 0., -0.25 );
						glVertex2d( 1.3, -2. );
						
						// Bottom
						glVertex2d( -1.3, 2. );
						glVertex2d( 0., 0.25 );
						glVertex2d( 0., 0.25 );
						glVertex2d( 1.3, 2. );
						
						// Left
						glVertex2d( -2., -1.3 );
						glVertex2d( -0.25, 0. );
						glVertex2d( -0.25, 0. );
						glVertex2d( -2., 1.3 );
						
						// Right
						glVertex2d( 2., -1.3 );
						glVertex2d( 0.25, 0. );
						glVertex2d( 0.25, 0. );
						glVertex2d( 2., 1.3 );
						
						// Top-Left
						glVertex2d( -0.35, 0.71 );
						glVertex2d( -0.59, 0.59 );
						glVertex2d( -0.59, 0.59 );
						glVertex2d( -0.71, 0.35 );
						
						// Top-Right
						glVertex2d( 0.35, 0.71 );
						glVertex2d( 0.59, 0.59 );
						glVertex2d( 0.59, 0.59 );
						glVertex2d( 0.71, 0.35 );
						
						// Bottom-Right
						glVertex2d( 0.35, -0.71 );
						glVertex2d( 0.59, -0.59 );
						glVertex2d( 0.59, -0.59 );
						glVertex2d( 0.71, -0.35 );
						
						// Bottom-Left
						glVertex2d( -0.35, -0.71 );
						glVertex2d( -0.59, -0.59 );
						glVertex2d( -0.59, -0.59 );
						glVertex2d( -0.71, -0.35 );
						
					glEnd();
					
					
					// Draw the intercept point.
					
					if( target || (target_obj && (target_obj->Type() == XWing::Object::SHOT)) )
					{
						shot_vec -= target_obj->MotionVector;
						
						Vec3D vec_to_target( target_obj->X - observed_ship->X, target_obj->Y - observed_ship->Y, target_obj->Z - observed_ship->Z );
						double time_to_target = vec_to_target.Length() / shot_vec.Length();
						
						Vec3D vec_to_intercept = vec_to_target;
						vec_to_intercept.X += target_obj->MotionVector.X * time_to_target;
						vec_to_intercept.Y += target_obj->MotionVector.Y * time_to_target;
						vec_to_intercept.Z += target_obj->MotionVector.Z * time_to_target;
						
						vec_to_intercept.ScaleTo( 1. );
						
						if( vec_to_intercept.Dot( &(observed_ship->Fwd) ) > 0. )
						{
							double x = vec_to_intercept.Dot( &(observed_ship->Right) ) * 7.;
							double y = vec_to_intercept.Dot( &(observed_ship->Up) ) * -7.;
							
							intercept_framebuffer->Setup2D( -1., 1. );
							
							Raptor::Game->Gfx.DrawCircle2D( x, y, 0.1, 4, 0, r, g, b, 1.f );
						}
					}
					
					
					// Draw noise if we're damaged.
					
					if( observed_ship->Health < (observed_ship->MaxHealth() * 0.5) )
					{
						intercept_framebuffer->Setup2D();
						
						glPointSize( 1.f );
						
						glBegin( GL_POINTS );
							
							float brightness = 1.;
							int dots = (observed_ship->MaxHealth() - observed_ship->Health) * 20.;
							for( int i = 0; i < dots; i ++ )
							{
								brightness = Rand::Double( 0.5, 1. );
								
								glColor4f( brightness - 0.5f, brightness, brightness - 0.5f, 1.f );
								glVertex2d( Rand::Int( 0, intercept_framebuffer->W ), Rand::Int( 0, intercept_framebuffer->H ) );
							}
							
						glEnd();
					}
				}
				
				
				// NOTE: Add any new displays here.
			}
			
			// Return to default framebuffer.
			if( changed_framebuffer )
				Raptor::Game->Gfx.SelectDefaultFramebuffer();
		}
	}
	#endif
	
	
	// Set up 3D rendering for the scene.
	
	double fov = Raptor::Game->Cfg.SettingAsDouble("g_fov");
	if( ! fov )
		fov = -59.;
	Raptor::Game->Cam.FOV = fov;
	
	Raptor::Game->Gfx.Setup3D( &(Raptor::Game->Cam) );
	Raptor::Game->Gfx.Clear();
	
	int dynamic_lights = Raptor::Game->Cfg.SettingAsInt("g_dynamic_lights");
	
	float crosshair_red = 0.5f;
	float crosshair_green = 0.75f;
	float crosshair_blue = 1.f;
	int ammo = -1;
	if( observed_ship )
		ammo = observed_ship->Ammo[ observed_ship->SelectedWeapon ];
	
	Model *cockpit_3d = NULL;
	Animation *cockpit_2d = NULL;
	
	
	// Draw background visual elements.
	
	DrawBackground();
	DrawStars();
	DrawDebris();
	
	
	// Draw the cockpit if we're doing 3D cockpit.
	
	if( observed_ship && (view == VIEW_COCKPIT) )
	{
		if( (observed_ship->SelectedWeapon == Shot::TYPE_TORPEDO) || (observed_ship->SelectedWeapon == Shot::TYPE_MISSILE) )
		{
			crosshair_red = 1.f;
			crosshair_green = 1.f;
			crosshair_blue = 0.f;
		}
		
		// See if the crosshair is lined up so the next shot would hit.
		
		if( target )
		{
			std::map<int,Shot*> test_shots = observed_ship->NextShots();
			for( std::map<int,Shot*>::iterator shot_iter = test_shots.begin(); shot_iter != test_shots.end(); shot_iter ++ )
			{
				if( target->WillCollide( shot_iter->second, 4. ) )
				{
					if( (observed_ship->SelectedWeapon == Shot::TYPE_TORPEDO) || (observed_ship->SelectedWeapon == Shot::TYPE_MISSILE) )
					{
						/*
						crosshair_red = 1.f;
						crosshair_green = 0.f;
						crosshair_blue = 0.f;
						*/
					}
					else
					{
						crosshair_red = 0.25f;
						crosshair_green = 1.f;
						crosshair_blue = 0.25f;
					}
				}
				
				delete shot_iter->second;
			}
		}
		
		if( ammo == 0 )
		{
			crosshair_red = 0.75f;
			crosshair_green = 0.75f;
			crosshair_blue = 0.75f;
		}
		
		
		// Load cockpits.
		
		bool g_3d_cockpit = Raptor::Game->Cfg.SettingAsBool("g_3d_cockpit");
		bool g_hq_cockpit = Raptor::Game->Cfg.SettingAsBool("g_hq_cockpit");
		
		if( observed_ship->ShipType == Ship::TYPE_XWING )
		{
			if( g_3d_cockpit )
			{
				if( g_hq_cockpit )
					cockpit_3d = Raptor::Game->Res.GetModel("x-wing_cockpit_hq.obj");
				if( ! cockpit_3d )
					cockpit_3d = Raptor::Game->Res.GetModel("x-wing_cockpit.obj");
			}
			cockpit_2d = Raptor::Game->Res.GetAnimation("cockpit_xwing.ani");
		}
		else if( observed_ship->ShipType == Ship::TYPE_YWING )
		{
			if( g_3d_cockpit )
			{
				if( g_hq_cockpit )
					cockpit_3d = Raptor::Game->Res.GetModel("y-wing_cockpit_hq.obj");
				if( ! cockpit_3d )
					cockpit_3d = Raptor::Game->Res.GetModel("y-wing_cockpit.obj");
			}
		}
		else if( observed_ship->ShipType == Ship::TYPE_TIE_FIGHTER )
		{
			if( g_3d_cockpit )
			{
				if( g_hq_cockpit )
					cockpit_3d = Raptor::Game->Res.GetModel("tie-fighter_cockpit_hq.obj");
				if( ! cockpit_3d )
					cockpit_3d = Raptor::Game->Res.GetModel("tie-fighter_cockpit.obj");
			}
			cockpit_2d = Raptor::Game->Res.GetAnimation("cockpit_tie.ani");
		}
		
		
		if( cockpit_3d && cockpit_3d->Objects.size() )
		{
			// We won't be drawing the 2D cockpit.
			cockpit_2d = NULL;
			
			// Draw the 3D cockpit.
			
			if( use_shaders )
				Raptor::Game->ShaderMgr.ResumeShaders();
			
			if( dynamic_lights && Raptor::Game->ShaderMgr.Active() )
			{
				Shot *nearest_shot = (Shot*) observed_ship->Nearest( (std::list<Pos3D*> *) &shots );
				if( nearest_shot )
				{
					Raptor::Game->ShaderMgr.Set3f( "DynamicLightPos", nearest_shot->X, nearest_shot->Y, nearest_shot->Z );
					Color dynamic_light_color = nearest_shot->LightColor();
					Raptor::Game->ShaderMgr.Set3f( "DynamicLightColor", dynamic_light_color.Red, dynamic_light_color.Green, dynamic_light_color.Blue );
				}
				else
				{
					Raptor::Game->ShaderMgr.Set3f( "DynamicLightPos", 0., 0., 0. );
					Raptor::Game->ShaderMgr.Set3f( "DynamicLightColor", 0., 0., 0. );
				}
			}
			
			double cockpit_scale = 0.125;
			double cockpit_fwd = 0.;
			double cockpit_up = 0.;
			double cockpit_right = 0.;
			double cockpit_tilt = 0.;
			
			if( observed_ship->ShipType == Ship::TYPE_XWING )
			{
				cockpit_fwd = (g_hq_cockpit ? 26. : 12.);
				cockpit_up = (g_hq_cockpit ? -33. : -27.);
				cockpit_right = (g_hq_cockpit ? 0. : -0.75);
				cockpit_tilt = (g_hq_cockpit ? -6. : 0.);
			}
			else if( observed_ship->ShipType == Ship::TYPE_YWING )
			{
				cockpit_fwd = (g_hq_cockpit ? -200. : -300.);
				cockpit_up = (g_hq_cockpit ? -40.: -38.);
				cockpit_right = (g_hq_cockpit ? 0. : 1.5);
				cockpit_tilt = (g_hq_cockpit ? 3. : 8.);
			}
			else if( observed_ship->ShipType == Ship::TYPE_TIE_FIGHTER )
			{
				cockpit_fwd = (g_hq_cockpit ? -5. : 0.);
			}
			
			Pos3D cockpit_pos( observed_ship );
			cockpit_pos.Fwd.RotateAround( &(cockpit_pos.Right), cockpit_tilt );
			cockpit_pos.FixVectors();
			cockpit_pos.MoveAlong( &(cockpit_pos.Fwd), cockpit_fwd * cockpit_scale );
			cockpit_pos.MoveAlong( &(cockpit_pos.Up), cockpit_up * cockpit_scale );
			cockpit_pos.MoveAlong( &(cockpit_pos.Right), cockpit_right * cockpit_scale );
			cockpit_3d->DrawAt( &cockpit_pos, cockpit_scale );
			
			if( use_shaders )
				Raptor::Game->ShaderMgr.StopShaders();
		}
		else
		{
			// Not drawing a 3D cockpit.
			cockpit_3d = NULL;
			
			// If we loaded a 2D cockpit, make sure it's valid.
			if( cockpit_2d && ! cockpit_2d->Frames.size() )
				cockpit_2d = NULL;
		}
	}
	
	
	// Draw game objects.
	
	if( use_shaders )
		Raptor::Game->ShaderMgr.ResumeShaders();
	
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
	{
		// We'll draw shots after everything else, so don't draw them now.
		if( obj_iter->second->Type() == XWing::Object::SHOT )
			continue;
		
		if( obj_iter->second->Type() == XWing::Object::SHIP )
		{
			Ship *ship = (Ship*) obj_iter->second;
			
			if( ship->Health <= 0. )
				ship->Explode( Raptor::Game->FrameTime );
			
			// Don't draw an external view of a ship whose cockpit we're in.
			else if( (view == VIEW_COCKPIT) && (ship == observed_ship) )
				continue;
			
			// Don't draw tiny ships far away.  Fighters should draw at about 5k from camera.
			double size = ship->Shape.GetTriagonal();
			if( (size * Raptor::Game->Gfx.H) < (Raptor::Game->Cam.Dist(ship) * 2.) )
				continue;
			
			// Don't draw ships that are entirely behind us.
			if( ship->DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ) < -size )
				continue;
		}
		else if( obj_iter->second->Type() == XWing::Object::TURRET )
		{
			// Don't draw turrets far away.
			if( Raptor::Game->Cam.Dist(obj_iter->second) > 6000. )
				continue;
		}
		else if( obj_iter->second->Type() == XWing::Object::DEATH_STAR_BOX )
		{
			// Don't draw boxes far away.
			if( Raptor::Game->Cam.Dist(obj_iter->second) > 6000. )
				continue;
		}
		
		if( dynamic_lights && Raptor::Game->ShaderMgr.Active() )
		{
			// Recast the list of Shot pointers to a list of Pos3D pointers.
			std::list<Pos3D*> *shot_list = (std::list<Pos3D*> *) &shots;
			Shot *nearest_shot = NULL;
			
			// If we're drawing the Death Star, the dynamic light should be the nearest shot to the camera.
			if( obj_iter->second->Type() == XWing::Object::DEATH_STAR )
				nearest_shot = (Shot*) Raptor::Game->Cam.Nearest( shot_list );
			else
				nearest_shot = (Shot*) obj_iter->second->Nearest( shot_list );
			
			if( nearest_shot )
			{
				Raptor::Game->ShaderMgr.Set3f( "DynamicLightPos", nearest_shot->X, nearest_shot->Y, nearest_shot->Z );
				Color dynamic_light_color = nearest_shot->LightColor();
				Raptor::Game->ShaderMgr.Set3f( "DynamicLightColor", dynamic_light_color.Red, dynamic_light_color.Green, dynamic_light_color.Blue );
			}
			else
			{
				Raptor::Game->ShaderMgr.Set3f( "DynamicLightPos", 0., 0., 0. );
				Raptor::Game->ShaderMgr.Set3f( "DynamicLightColor", 0., 0., 0. );
			}
		}
		
		glPushMatrix();
		obj_iter->second->Draw();
		glPopMatrix();
	}
	
	if( Raptor::Game->ShaderMgr.Active() )
	{
		// Remove dynamic lights and material properties so other things will render correctly.
		Raptor::Game->ShaderMgr.Set3f( "AmbientColor", 1.f, 1.f, 1.f );
		Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", 0.f, 0.f, 0.f );
		Raptor::Game->ShaderMgr.Set3f( "DynamicLightPos", 0., 0., 0. );
		Raptor::Game->ShaderMgr.Set3f( "DynamicLightColor", 0., 0., 0. );
	}
	
	if( use_shaders )
		Raptor::Game->ShaderMgr.StopShaders();
	
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	
	
	// Draw anything with alpha back-to-front.
	
	std::map< double, std::list<Renderable> > sorted_renderables;
	
	for( std::list<Shot*>::iterator shot_iter = shots.begin(); shot_iter != shots.end(); shot_iter ++ )
		sorted_renderables[ (*shot_iter)->DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ) ].push_back( *shot_iter );
	
	for( std::list<Effect>::iterator effect_iter = Raptor::Game->Data.Effects.begin(); effect_iter != Raptor::Game->Data.Effects.end(); effect_iter ++ )
		sorted_renderables[ effect_iter->DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ) ].push_back( &(*effect_iter) );
	
	for( std::map< double, std::list<Renderable> >::reverse_iterator renderable_iter1 = sorted_renderables.rbegin(); renderable_iter1 != sorted_renderables.rend(); renderable_iter1 ++ )
	{
		for( std::list<Renderable>::iterator renderable_iter2 = renderable_iter1->second.begin(); renderable_iter2 != renderable_iter1->second.end(); renderable_iter2 ++ )
		{
			glPushMatrix();
			if( renderable_iter2->ShotPtr )
				renderable_iter2->ShotPtr->Draw();
			else if( renderable_iter2->EffectPtr )
				renderable_iter2->EffectPtr->Draw();
			glPopMatrix();
		}
	}
	
	
	// Reset the color, dynamic lights, and material properties so other things will render correctly.
	
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	
	if( use_shaders )
	{
		Raptor::Game->ShaderMgr.ResumeShaders();
		
		if( Raptor::Game->ShaderMgr.Active() )
		{
			Raptor::Game->ShaderMgr.Set3f( "AmbientColor", 1.f, 1.f, 1.f );
			Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", 0.f, 0.f, 0.f );
			Raptor::Game->ShaderMgr.Set3f( "DynamicLightPos", 0., 0., 0. );
			Raptor::Game->ShaderMgr.Set3f( "DynamicLightColor", 0., 0., 0. );
		}
		
		Raptor::Game->ShaderMgr.StopShaders();
	}
	
	
	// Draw 2D UI elements.
	
	if( observed_ship && (view == VIEW_COCKPIT) )
	{
		if( target || (target_obj && (target_obj->Type() == XWing::Object::SHOT)) )
		{
			// Draw target box.
			
			double h = 10.;
			double w = 10.;
			double max_dist = 2000.;
			
			if( target )
			{
				h = fabs( Raptor::Game->Cam.Up.Dot( &(target->Up) ) * target->Shape.GetHeight() ) + fabs( Raptor::Game->Cam.Up.Dot( &(target->Fwd) ) * target->Shape.GetLength() ) + fabs( Raptor::Game->Cam.Up.Dot( &(target->Right) ) * target->Shape.GetWidth() );
				w = fabs( Raptor::Game->Cam.Right.Dot( &(target->Up) ) * target->Shape.GetHeight() ) + fabs( Raptor::Game->Cam.Right.Dot( &(target->Fwd) ) * target->Shape.GetLength() ) + fabs( Raptor::Game->Cam.Right.Dot( &(target->Right) ) * target->Shape.GetWidth() );
				max_dist = target->Shape.GetTriagonal() * 50.;
			}
			
			Vec3D up = Raptor::Game->Cam.Up * h / 2.;
			Vec3D right = Raptor::Game->Cam.Right * w / 2.;
			
			Pos3D pos;
			pos.Copy( target_obj );
			Vec3D vec_to_target( pos.X - Raptor::Game->Cam.X, pos.Y - Raptor::Game->Cam.Y, pos.Z - Raptor::Game->Cam.Z );
			double dist = vec_to_target.Length();
			if( dist > max_dist )
			{
				vec_to_target.ScaleTo( 1. );
				pos.MoveAlong( &vec_to_target, max_dist - dist );
			}
			
			glDisable( GL_DEPTH_TEST );
			
			Raptor::Game->Gfx.DrawLine3D( pos.X - right.X + up.X, pos.Y - right.Y + up.Y, pos.Z - right.Z + up.Z, pos.X - right.X / 2. + up.X, pos.Y - right.Y / 2. + up.Y, pos.Z - right.Z / 2. + up.Z, 1.f, 1.f, 1.f, 1.f );
			Raptor::Game->Gfx.DrawLine3D( pos.X - right.X + up.X, pos.Y - right.Y + up.Y, pos.Z - right.Z + up.Z, pos.X - right.X + up.X / 2., pos.Y - right.Y + up.Y / 2., pos.Z - right.Z + up.Z / 2., 1.f, 1.f, 1.f, 1.f );
			
			Raptor::Game->Gfx.DrawLine3D( pos.X + right.X + up.X, pos.Y + right.Y + up.Y, pos.Z + right.Z + up.Z, pos.X + right.X / 2. + up.X, pos.Y + right.Y / 2. + up.Y, pos.Z + right.Z / 2. + up.Z, 1.f, 1.f, 1.f, 1.f );
			Raptor::Game->Gfx.DrawLine3D( pos.X + right.X + up.X, pos.Y + right.Y + up.Y, pos.Z + right.Z + up.Z, pos.X + right.X + up.X / 2., pos.Y + right.Y + up.Y / 2., pos.Z + right.Z + up.Z / 2., 1.f, 1.f, 1.f, 1.f );
			
			Raptor::Game->Gfx.DrawLine3D( pos.X + right.X - up.X, pos.Y + right.Y - up.Y, pos.Z + right.Z - up.Z, pos.X + right.X / 2. - up.X, pos.Y + right.Y / 2. - up.Y, pos.Z + right.Z / 2. - up.Z, 1.f, 1.f, 1.f, 1.f );
			Raptor::Game->Gfx.DrawLine3D( pos.X + right.X - up.X, pos.Y + right.Y - up.Y, pos.Z + right.Z - up.Z, pos.X + right.X - up.X / 2., pos.Y + right.Y - up.Y / 2., pos.Z + right.Z - up.Z / 2., 1.f, 1.f, 1.f, 1.f );
			
			Raptor::Game->Gfx.DrawLine3D( pos.X - right.X - up.X, pos.Y - right.Y - up.Y, pos.Z - right.Z - up.Z, pos.X - right.X / 2. - up.X, pos.Y - right.Y / 2. - up.Y, pos.Z - right.Z / 2. - up.Z, 1.f, 1.f, 1.f, 1.f );
			Raptor::Game->Gfx.DrawLine3D( pos.X - right.X - up.X, pos.Y - right.Y - up.Y, pos.Z - right.Z - up.Z, pos.X - right.X - up.X / 2., pos.Y - right.Y - up.Y / 2., pos.Z - right.Z - up.Z / 2., 1.f, 1.f, 1.f, 1.f );
			
			glEnable( GL_DEPTH_TEST );
		}
		
		
		// Draw the 3D crosshair.
		{
			glDisable( GL_DEPTH_TEST );
			Pos3D crosshair_pos( observed_ship );
			crosshair_pos.MoveAlong( &(crosshair_pos.Fwd), 5. );
			glColor4f( crosshair_red, crosshair_green, crosshair_blue, 1.f );
			
			glPointSize( 2.f );
			glBegin( GL_POINTS );
				glVertex3d( crosshair_pos.X, crosshair_pos.Y, crosshair_pos.Z );
			glEnd();
			
			Vec3D up( &(crosshair_pos.Up) );
			Vec3D right( &(crosshair_pos.Right) );
			up.ScaleTo( 0.1 );
			right.ScaleTo( 0.1 );
			
			Vec3D weapon_vec;
			Vec2D relative_weapon_vec;
			Pos3D weapon_pos;
			
			glBegin( GL_LINES );
				glVertex3d( crosshair_pos.X - right.X + up.X, crosshair_pos.Y - right.Y + up.Y, crosshair_pos.Z - right.Z + up.Z );
				glVertex3d( crosshair_pos.X - right.X / 2. + up.X, crosshair_pos.Y - right.Y / 2. + up.Y, crosshair_pos.Z - right.Z / 2. + up.Z );
				glVertex3d( crosshair_pos.X - right.X + up.X, crosshair_pos.Y - right.Y + up.Y, crosshair_pos.Z - right.Z + up.Z );
				glVertex3d( crosshair_pos.X - right.X + up.X / 2., crosshair_pos.Y - right.Y + up.Y / 2., crosshair_pos.Z - right.Z + up.Z / 2. );
				glVertex3d( crosshair_pos.X + right.X + up.X, crosshair_pos.Y + right.Y + up.Y, crosshair_pos.Z + right.Z + up.Z );
				glVertex3d( crosshair_pos.X + right.X / 2. + up.X, crosshair_pos.Y + right.Y / 2. + up.Y, crosshair_pos.Z + right.Z / 2. + up.Z );
				glVertex3d( crosshair_pos.X + right.X + up.X, crosshair_pos.Y + right.Y + up.Y, crosshair_pos.Z + right.Z + up.Z );
				glVertex3d( crosshair_pos.X + right.X + up.X / 2., crosshair_pos.Y + right.Y + up.Y / 2., crosshair_pos.Z + right.Z + up.Z / 2. );
				glVertex3d( crosshair_pos.X + right.X - up.X, crosshair_pos.Y + right.Y - up.Y, crosshair_pos.Z + right.Z - up.Z );
				glVertex3d( crosshair_pos.X + right.X / 2. - up.X, crosshair_pos.Y + right.Y / 2. - up.Y, crosshair_pos.Z + right.Z / 2. - up.Z );
				glVertex3d( crosshair_pos.X + right.X - up.X, crosshair_pos.Y + right.Y - up.Y, crosshair_pos.Z + right.Z - up.Z );
				glVertex3d( crosshair_pos.X + right.X - up.X / 2., crosshair_pos.Y + right.Y - up.Y / 2., crosshair_pos.Z + right.Z - up.Z / 2. );
				glVertex3d( crosshair_pos.X - right.X - up.X, crosshair_pos.Y - right.Y - up.Y, crosshair_pos.Z - right.Z - up.Z );
				glVertex3d( crosshair_pos.X - right.X / 2. - up.X, crosshair_pos.Y - right.Y / 2. - up.Y, crosshair_pos.Z - right.Z / 2. - up.Z );
				glVertex3d( crosshair_pos.X - right.X - up.X, crosshair_pos.Y - right.Y - up.Y, crosshair_pos.Z - right.Z - up.Z );
				glVertex3d( crosshair_pos.X - right.X - up.X / 2., crosshair_pos.Y - right.Y - up.Y / 2., crosshair_pos.Z - right.Z - up.Z / 2. );
				
				std::map<int,Shot*> all_weapons = observed_ship->AllShots();
				for( std::map<int,Shot*>::iterator shot_iter = all_weapons.begin(); shot_iter != all_weapons.end(); shot_iter ++ )
				{
					weapon_vec.Set( shot_iter->second->X - observed_ship->X, shot_iter->second->Y - observed_ship->Y, shot_iter->second->Z - observed_ship->Z );
					relative_weapon_vec.Set( weapon_vec.Dot(&(observed_ship->Right)), weapon_vec.Dot(&(observed_ship->Up)) );
					relative_weapon_vec.ScaleTo( 1. );
					weapon_pos.Copy( &crosshair_pos );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X * 0.05 );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y * 0.05 );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X * 0.1 );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y * 0.1 );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
				}
			glEnd();
			
			glPointSize( 4.f );
			glBegin( GL_POINTS );
				std::map<int,Shot*> next_weapons = observed_ship->NextShots();
				for( std::map<int,Shot*>::iterator shot_iter = next_weapons.begin(); shot_iter != next_weapons.end(); shot_iter ++ )
				{
					weapon_vec.Set( shot_iter->second->X - observed_ship->X, shot_iter->second->Y - observed_ship->Y, shot_iter->second->Z - observed_ship->Z );
					relative_weapon_vec.Set( weapon_vec.Dot(&(observed_ship->Right)), weapon_vec.Dot(&(observed_ship->Up)) );
					relative_weapon_vec.ScaleTo( 1. );
					weapon_pos.Copy( &crosshair_pos );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X * 0.18 );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y * 0.18 );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
				}
			glEnd();
			
			if( ammo >= 0 )
			{
				crosshair_pos.MoveAlong( &up, -0.05 );
				ScreenFont->DrawText3D( Num::ToString(ammo), &crosshair_pos, Font::ALIGN_TOP_CENTER, 0.002 );
			}

			glEnable( GL_DEPTH_TEST );
		}
		
		
		if( cockpit_2d && ! cockpit_3d )
		{
			// Draw the 2D cockpit.
			Raptor::Game->Gfx.Setup2D();
			Raptor::Game->Gfx.DrawRect2D( Rect.w / 2 - Rect.h, 0, Rect.w / 2 + Rect.h, Rect.h, cockpit_2d->CurrentFrame(), 1.f, 1.f, 1.f, 1.f );
		}
		
		
		// Here we'll draw target info, unless it's done via cockpit screens.
		
		if( target )
		{
			if( !( cockpit_3d && use_framebuffers ) )
			{
				// Show old-style targetting display.
				
				Vec3D vec_to_target( target->X - observed_ship->X, target->Y - observed_ship->Y, target->Z - observed_ship->Z );
				vec_to_target.ScaleTo( 1. );
				Camera cam_to_target( Raptor::Game->Cam );
				cam_to_target.Fwd.Copy( &vec_to_target );
				cam_to_target.Up.Copy( &(target->Up) );
				cam_to_target.FixVectors();
				cam_to_target.SetPos( target->X, target->Y, target->Z );
				cam_to_target.MoveAlong( &(cam_to_target.Fwd), -1.3 );
				cam_to_target.Fwd.RotateAround( &(cam_to_target.Right), 25. );
				cam_to_target.FixVectors();
				
				Raptor::Game->Gfx.Setup3D( &(cam_to_target) );
				
				if( use_shaders )
					Raptor::Game->ShaderMgr.ResumeShaders();
				
				if( dynamic_lights && Raptor::Game->ShaderMgr.Active() )
				{
					Raptor::Game->ShaderMgr.Set3f( "DynamicLightPos", 0., 0., 0. );
					Raptor::Game->ShaderMgr.Set3f( "DynamicLightColor", 0., 0., 0. );
				}
				
				target->Shape.DrawAt( target, 0.3 / target->Shape.GetTriagonal() );
				
				if( use_shaders )
					Raptor::Game->ShaderMgr.StopShaders();
				
				Raptor::Game->Gfx.Setup2D();
				
				std::string target_name = target->Name;
				Player *target_player = Raptor::Game->Data.GetPlayer( target->PlayerID );
				if( target_player )
					target_name = target_player->Name;
				
				float red = 1.f, green = 1.f, blue = 1.f;
				
				if( observed_ship->Team && (target->Team == observed_ship->Team) )
				{
					red = 0.f;
					green = 1.f;
					blue = 0.f;
				}
				else
				{
					red = 1.f;
					green = 0.f;
					blue = 0.f;
				}
				
				BigFont->DrawText( target_name, Rect.w / 2 + 2, Rect.h - 8, Font::ALIGN_BOTTOM_CENTER, 0.f, 0.f, 0.f, 0.8f );
				BigFont->DrawText( target_name, Rect.w / 2, Rect.h - 10, Font::ALIGN_BOTTOM_CENTER, red, green, blue, 1.f );
			}
		}
		
		
		// Draw the radar.
		
		Raptor::Game->Gfx.Setup2D( Raptor::Game->Gfx.H / -2, Raptor::Game->Gfx.H / 2 );
		double h = Raptor::Game->Gfx.H / 2.;
		RadarDirectionFont->DrawText( "F", -1.304 * h, -0.829 * h, Font::ALIGN_TOP_RIGHT, 0.f, 0.f, 1.f, 0.75f );
		RadarDirectionFont->DrawText( "R", 1.304 * h, -0.829 * h, Font::ALIGN_TOP_LEFT, 0.f, 0.f, 1.f, 0.75f );
		
		Raptor::Game->Gfx.Setup2D( -1., 1. );
		Raptor::Game->Gfx.DrawCircle2D( -1.233, -0.9, 0.1, 32, 0, 0.f, 0.f, 0.f, 0.75f );
		Raptor::Game->Gfx.DrawCircle2D( 1.233, -0.9, 0.1, 32, 0, 0.f, 0.f, 0.f, 0.75f );
		Raptor::Game->Gfx.DrawCircleOutline2D( -1.233, -0.9, 0.1, 32, 0.f, 0.f, 1.f, 0.75f );
		Raptor::Game->Gfx.DrawCircleOutline2D( 1.233, -0.9, 0.1, 32, 0.f, 0.f, 1.f, 0.75f );
		
		Pos3D *radar_ref = (Pos3D*) &(Raptor::Game->Cam);
		
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
		{
			uint32_t type = obj_iter->second->Type();
			if( ((type == XWing::Object::SHIP) || (type == XWing::Object::ASTEROID) || (type == XWing::Object::SHOT)) && (obj_iter->second != observed_ship) )
			{
				GameObject *obj = obj_iter->second;
				Ship *ship = NULL;
				Shot *shot = NULL;
				
				if( type == XWing::Object::SHOT )
				{
					shot = (Shot*) obj_iter->second;
					
					if( (shot->ShotType != Shot::TYPE_TORPEDO) && (shot->ShotType != Shot::TYPE_MISSILE) )
						continue;
				}
				else if( type == XWing::Object::SHIP )
				{
					ship = (Ship*) obj_iter->second;
					
					if( ship->Health <= 0. )
						continue;
				}

				Vec3D vec_to_obj( obj->X - radar_ref->X, obj->Y - radar_ref->Y, obj->Z - radar_ref->Z );
				vec_to_obj.ScaleTo( 1. );
				double x = vec_to_obj.Dot( &(radar_ref->Right) ) * 0.1;
				double y = -0.9 - vec_to_obj.Dot( &(radar_ref->Up) ) * 0.1;
				if( vec_to_obj.Dot( &(radar_ref->Fwd) ) >= 0. )
					x -= 1.233;
				else
					x += 1.233;
				
				float red = 0.5f, green = 0.5f, blue = 0.5f;
				double radius = 0.005;
				
				if( ship )
				{
					if( observed_ship->Team && (ship->Team == observed_ship->Team) )
					{
						red = 0.f;
						green = 1.f;
						blue = 0.f;
					}
					else
					{
						red = 1.f;
						green = 0.f;
						blue = 0.f;
					}
				}
				else if( shot )
				{
					red = 1.f;
					green = 1.f;
					blue = 0.f;
					radius = 0.003;
				}
				
				Raptor::Game->Gfx.DrawCircle2D( x, y, radius, 6, 0, red, green, blue, 1.f );
			}
		}
		
		if( target_obj )
		{
			// Draw target in radar.
			
			Vec3D vec_to_target( target_obj->X - radar_ref->X, target_obj->Y - radar_ref->Y, target_obj->Z - radar_ref->Z );
			vec_to_target.ScaleTo( 1. );
			double x = vec_to_target.Dot( &(radar_ref->Right) ) * 0.1;
			double y = -0.9 - vec_to_target.Dot( &(radar_ref->Up) ) * 0.1;
			if( vec_to_target.Dot( &(radar_ref->Fwd) ) >= 0. )
				x -= 1.233;
			else
				x += 1.233;
			
			float red = 1.f, green = 1.f, blue = 1.f;
			double radius = 0.005;
			
			if( target )
			{
				if( observed_ship->Team && (target->Team == observed_ship->Team) )
				{
					red = 0.f;
					green = 1.f;
					blue = 0.f;
				}
				else
				{
					red = 1.f;
					green = 0.f;
					blue = 0.f;
				}
			}
			else if( target_obj->Type() == XWing::Object::SHOT )
			{
				red = 1.f;
				green = 1.f;
				blue = 0.f;
				radius = 0.003;
			}
			else
			{
				red = 0.5f;
				green = 0.5f;
				blue = 0.5f;
			}
			
			Raptor::Game->Gfx.DrawBox2D( x - 0.01, y - 0.01, x + 0.01, y + 0.01, 1.f, 1.f, 1.f, 1.f );
			Raptor::Game->Gfx.DrawCircle2D( x, y, radius, 6, 0, red, green, blue, 1.f );
		}
	}
	
	
	// If we're spectating, show who we're watching.
	
	if( (observed_player && (observed_player->ID == Raptor::Game->PlayerID)) || Raptor::Game->Cfg.SettingAsBool("screensaver") )
		;
	else if( observed_player )
	{
		Raptor::Game->Gfx.Setup2D();
		BigFont->DrawText( "Watching: " + observed_player->Name, Rect.w / 2 + 2, 12, Font::ALIGN_TOP_CENTER, 0.f,0.f,0.f,0.8f );
		BigFont->DrawText( "Watching: " + observed_player->Name, Rect.w / 2, 10, Font::ALIGN_TOP_CENTER );
	}
	else if( observed_ship )
	{
		Raptor::Game->Gfx.Setup2D();
		BigFont->DrawText( "Watching: " + observed_ship->Name, Rect.w / 2 + 2, 12, Font::ALIGN_TOP_CENTER, 0.f,0.f,0.f,0.8f );
		BigFont->DrawText( "Watching: " + observed_ship->Name, Rect.w / 2, 10, Font::ALIGN_TOP_CENTER );
	}
	
	
	// If the round is over, show the scores.
	
	if( ((Raptor::Game->State >= XWing::State::ROUND_ENDED) || Raptor::Game->Keys.KeyDown(SDLK_TAB)) && (! Raptor::Game->Cfg.SettingAsBool("screensaver")) )
		DrawScores();
	
	
	// Add any flyby sounds.
	
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
	{
		// Don't add flyby sounds for the ship we're watching.
		if( observed_ship && (obj_iter->first == observed_ship->ID) )
			continue;
		
		// Don't add two sounds for the same object flying by.
		if( Raptor::Game->Snd.ObjectPans.find( obj_iter->first ) != Raptor::Game->Snd.ObjectPans.end() )
			continue;
		
		// Calculate the object's motion relative to the camera's.
		Vec3D relative_motion = obj_iter->second->MotionVector;
		relative_motion -= cam_motion_vec;
		
		// Check for ship flybys.
		if( (obj_iter->second->Type() == XWing::Object::SHIP) && (Raptor::Game->Cam.Dist( obj_iter->second ) < 60.) && (obj_iter->second->MotionVector.Length() >= 20.) )
		{
			Ship *ship = (Ship*) obj_iter->second;
			
			// Dead ships tell no tales.
			if( ship->Health <= 0. )
				continue;
			
			// Different sounds for different speeds and ships.
			if( ship->ShipType == Ship::TYPE_TIE_FIGHTER )
			{
				if( relative_motion.Length() >= 100. )
					Raptor::Game->Snd.PlayFromObject( Raptor::Game->Res.GetSound("tie_fast.wav"), obj_iter->first, 2. );
				else if( relative_motion.Length() >= 25. )
					Raptor::Game->Snd.PlayFromObject( Raptor::Game->Res.GetSound("tie_slow.wav"), obj_iter->first, 2. );
			}
			else if( (ship->ShipType == Ship::TYPE_XWING) || (ship->ShipType == Ship::TYPE_YWING) )
			{
				if( relative_motion.Length() >= 80. )
					Raptor::Game->Snd.PlayFromObject( Raptor::Game->Res.GetSound("xwing_fast.wav"), obj_iter->first, 2. );
				else if( relative_motion.Length() >= 20. )
					Raptor::Game->Snd.PlayFromObject( Raptor::Game->Res.GetSound("xwing_slow.wav"), obj_iter->first, 2. );
			}
		}
	}
	
	
	glPopMatrix();
}


void RenderLayer::DrawBackground( void )
{
	// Don't draw if g_bg=false.
	if( ! Raptor::Game->Cfg.SettingAsBool( "g_bg", true ) )
		return;
	
	double bg_dist = Raptor::Game->Cfg.SettingAsDouble( "g_bg_dist", 50000. );
	
	Raptor::Game->Gfx.DrawSphere3D( Raptor::Game->Cam.X, Raptor::Game->Cam.Y, Raptor::Game->Cam.Z, bg_dist, 32, Background.CurrentFrame(), Graphics::TEXTURE_MODE_Y_ASIN );
}


void RenderLayer::DrawStars( void )
{
	// Don't draw if g_stars=false.
	if( ! Raptor::Game->Cfg.SettingAsBool( "g_stars", true ) )
		return;
	
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	glPointSize( 0.5f );
	glDisable( GL_DEPTH_TEST );
	glBegin( GL_POINTS );
	
		for( int i = 0; i < STAR_COUNT; i ++ )
			glVertex3d( Raptor::Game->Cam.X + Stars[ i ].X, Raptor::Game->Cam.Y + Stars[ i ].Y, Raptor::Game->Cam.Z + Stars[ i ].Z );
	
	glEnd();
	glEnable( GL_DEPTH_TEST );
}


void RenderLayer::DrawDebris( void )
{
	// Don't draw if g_debris=false.
	if( ! Raptor::Game->Cfg.SettingAsBool( "g_debris", true ) )
		return;
	
	for( int i = 0; i < DEBRIS_COUNT; i ++ )
	{
		double dist = Raptor::Game->Cam.Dist( &(Debris[ i ]) );
		if( dist > DEBRIS_DIST )
		{
			Debris[ i ].SetPos( Rand::Double( Raptor::Game->Cam.X - DEBRIS_DIST, Raptor::Game->Cam.X + DEBRIS_DIST ), Rand::Double( Raptor::Game->Cam.Y - DEBRIS_DIST, Raptor::Game->Cam.Y + DEBRIS_DIST ), Rand::Double( Raptor::Game->Cam.Z - DEBRIS_DIST, Raptor::Game->Cam.Z + DEBRIS_DIST ) );
			dist = Raptor::Game->Cam.Dist( &(Debris[ i ]) );
		}
		
		Raptor::Game->Gfx.DrawSphere3D( Debris[ i ].X, Debris[ i ].Y, Debris[ i ].Z, 0.1, 3, 0, 0.5f, 0.5f, 0.5f, 1.f );
	}
}


void RenderLayer::DrawScores( void )
{
	glPushMatrix();
	Raptor::Game->Gfx.Setup2D();
	
	std::map< int, std::list<Player*> > scores;
	for( std::map<uint16_t,Player*>::iterator player_iter = Raptor::Game->Data.Players.begin(); player_iter != Raptor::Game->Data.Players.end(); player_iter ++ )
	{
		if( (! player_iter->second->Properties["assigned_team"].empty()) && (player_iter->second->Properties["assigned_team"] != "Spectator") )
			scores[ atoi( player_iter->second->Properties["kills"].c_str() ) ].push_back( player_iter->second );
	}
	
	int y = 100;
	
	bool ffa = ( Raptor::Game->Data.Properties["gametype"].find("ffa_") == 0 );
	
	if( ! ffa )
	{
		BigFont->DrawText( "Rebels", Raptor::Game->Gfx.W / 2 - 298, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( "Rebels", Raptor::Game->Gfx.W / 2 - 300, y, Font::ALIGN_MIDDLE_LEFT, 1.f, 0.25f, 0.25f, 1.f );
		BigFont->DrawText( "vs", Raptor::Game->Gfx.W / 2 + 2, y + 2, Font::ALIGN_MIDDLE_CENTER, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( "vs", Raptor::Game->Gfx.W / 2, y, Font::ALIGN_MIDDLE_CENTER, 0.75f, 0.75f, 0.75f, 1.f );
		BigFont->DrawText( "Empire", Raptor::Game->Gfx.W / 2 + 302, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( "Empire", Raptor::Game->Gfx.W / 2 + 300, y, Font::ALIGN_MIDDLE_RIGHT, 0.25f, 0.25f, 1.f, 1.f );
		
		if( Raptor::Game->Data.Properties["gametype"] == "team_dm" )
		{
			BigFont->DrawText( Raptor::Game->Data.Properties["team_score_rebel"], Raptor::Game->Gfx.W / 2 - 14, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( Raptor::Game->Data.Properties["team_score_rebel"], Raptor::Game->Gfx.W / 2 - 16, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 0.25f, 0.25f, 1.f );
			BigFont->DrawText( Raptor::Game->Data.Properties["team_score_empire"], Raptor::Game->Gfx.W / 2 + 18, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( Raptor::Game->Data.Properties["team_score_empire"], Raptor::Game->Gfx.W / 2 + 16, y, Font::ALIGN_MIDDLE_LEFT, 0.25f, 0.25f, 1.f, 1.f );
		}
		
		y += BigFont->GetHeight() - 8;
		Raptor::Game->Gfx.DrawLine2D( Raptor::Game->Gfx.W / 2 - 298, y + 2, Raptor::Game->Gfx.W / 2 + 302, y + 2, 0.f, 0.f, 0.f, 0.8f );
		Raptor::Game->Gfx.DrawLine2D( Raptor::Game->Gfx.W / 2 - 300, y, Raptor::Game->Gfx.W / 2 + 300, y, 1.f, 1.f, 1.f, 1.f );
		y += 16;
	}
	
	SmallFont->DrawText( "Player", Raptor::Game->Gfx.W / 2 - 298, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f );
	SmallFont->DrawText( "Player", Raptor::Game->Gfx.W / 2 - 300, y, Font::ALIGN_MIDDLE_LEFT, 1.f, 1.f, 1.f, 1.f );
	SmallFont->DrawText( "Kills", Raptor::Game->Gfx.W / 2 + 202, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
	SmallFont->DrawText( "Kills", Raptor::Game->Gfx.W / 2 + 200, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f );
	SmallFont->DrawText( "Deaths", Raptor::Game->Gfx.W / 2 + 302, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
	SmallFont->DrawText( "Deaths", Raptor::Game->Gfx.W / 2 + 300, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f );
	y += SmallFont->GetHeight();
	
	for( std::map< int, std::list<Player*> >::reverse_iterator score_iter = scores.rbegin(); score_iter != scores.rend(); score_iter ++ )
	{
		for( std::list<Player*>::iterator player_iter = score_iter->second.begin(); player_iter != score_iter->second.end(); player_iter ++ )
		{
			float red = 1.f, green = 1.f, blue = 1.f;
			
			if( ! ffa )
			{
				if( (*player_iter)->Properties["assigned_team"] == "Rebel" )
				{
					red = 1.f;
					
					if( (*player_iter)->ID == Raptor::Game->PlayerID )
					{
						green = 0.f;
						blue = 0.f;
					}
					else
					{
						green = 0.25f;
						blue = 0.25f;
					}
				}
				else if( (*player_iter)->Properties["assigned_team"] == "Empire" )
				{
					blue = 1.f;
					
					if( (*player_iter)->ID == Raptor::Game->PlayerID )
					{
						red = 0.f;
						green = 0.f;
					}
					else
					{
						red = 0.25f;
						green = 0.25f;
					}
				}
			}
			else if( (*player_iter)->ID == Raptor::Game->PlayerID )
				blue = 0.f;
			
			std::string str = (*player_iter)->Name;
			if( (*player_iter)->Properties["assigned_team"] != "" )
				str += std::string(" [") + (*player_iter)->Properties["assigned_team"] + std::string("]");

			BigFont->DrawText( str, Raptor::Game->Gfx.W / 2 - 298, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( str, Raptor::Game->Gfx.W / 2 - 300, y, Font::ALIGN_MIDDLE_LEFT, red, green, blue, 1.f );
			BigFont->DrawText( Num::ToString(score_iter->first), Raptor::Game->Gfx.W / 2 + 202, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( Num::ToString(score_iter->first), Raptor::Game->Gfx.W / 2 + 200, y, Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f );
			BigFont->DrawText( (*player_iter)->Properties["deaths"], Raptor::Game->Gfx.W / 2 + 302, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( (*player_iter)->Properties["deaths"], Raptor::Game->Gfx.W / 2 + 300, y, Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f );
			
			y += BigFont->GetHeight();
		}
	}
	
	
	glPopMatrix();
}


bool RenderLayer::KeyDown( SDLKey key )
{
	if( MessageInput->IsSelected() )
	{
		if( (key == SDLK_RETURN) || (key == SDLK_KP_ENTER) )
		{
			((XWingGame*)( Raptor::Game ))->ReadKeyboard = true;
			
			std::string msg = MessageInput->Text;
			MessageInput->Text = "";
			
			Selected = NULL;
			MessageInput->Visible = false;
			
			if( ! msg.empty() )
			{
				Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
				
				Packet message = Packet( Raptor::Packet::MESSAGE );
				message.AddString( ((player ? player->Name : std::string("Anonymous")) + std::string(":  ") + msg).c_str() );
				message.AddUInt( TextConsole::MSG_CHAT );
				Raptor::Game->Net.Send( &message );
			}
			
			return true;
		}
		else if( key == SDLK_ESCAPE )
		{
			((XWingGame*)( Raptor::Game ))->ReadKeyboard = true;
			
			MessageInput->Text = "";
			Selected = NULL;
			MessageInput->Visible = false;
			
			return true;
		}
		
		return MessageInput->KeyDown( key );
	}
	else if( (key == SDLK_RETURN) || (key == SDLK_KP_ENTER) )
	{
		((XWingGame*)( Raptor::Game ))->ReadKeyboard = false;
		
		Selected = MessageInput;
		MessageInput->Visible = true;
		
		return true;
	}
	else if( key == SDLK_ESCAPE )
	{
		Raptor::Game->Mouse.ShowCursor = true;
		((XWingGame*)( Raptor::Game ))->ReadMouse = false;
		Raptor::Game->Layers.Add( new IngameMenu() );
	}
	
	return false;
}


// -----------------------------------------------------------------------------


Renderable::Renderable( Shot *shot )
{
	ShotPtr = shot;
	EffectPtr = NULL;
}

Renderable::Renderable( Effect *effect )
{
	ShotPtr = NULL;
	EffectPtr = effect;
}

Renderable::~Renderable()
{
}
