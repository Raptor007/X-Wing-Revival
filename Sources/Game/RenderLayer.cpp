/*
 *  RenderLayer.cpp
 */

#include "RenderLayer.h"

#include <cmath>
#include <algorithm>
#include "Graphics.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "XWingDefs.h"
#include "XWingGame.h"
#include "Rand.h"
#include "Randomizer.h"
#include "Num.h"
#include "Math3D.h"
#include "IngameMenu.h"
#include "PrefsMenu.h"
#include "DeathStar.h"
#include "DeathStarBox.h"
#include "Asteroid.h"
#include "Turret.h"
#include "Checkpoint.h"

#ifdef WIN32
#include "SaitekX52Pro.h"
#endif


RenderLayer::RenderLayer( void )
{
	Name = "RenderLayer";
	ReadControls = true;
	
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	BigFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 21 );
	SmallFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 15 );
	RadarDirectionFont = Raptor::Game->Res.GetFont( "ProFont.ttf", 22 );
	ScreenFont = Raptor::Game->Res.GetFont( "ProFont.ttf", 48 );
	Mic.BecomeInstance( Raptor::Game->Res.GetAnimation("mic.ani") );
	
	for( int i = 0; i < STAR_COUNT * 3; i ++ )
		Stars[ i ] = Rand::Double( -10000., 10000. );
	
	for( int i = 0; i < DEBRIS_COUNT; i ++ )
		Debris[ i ].SetPos( Rand::Double( -DEBRIS_DIST, DEBRIS_DIST ), Rand::Double( -DEBRIS_DIST, DEBRIS_DIST ), Rand::Double( -DEBRIS_DIST, DEBRIS_DIST ) );
	
	MessageOutput = new MessageOverlay( Raptor::Game->Res.GetFont( "AgencyFB.ttf", 21 ) );
	AddElement( MessageOutput );
	
	AddElement( MessageInput = new TextBox( NULL, Raptor::Game->Res.GetFont( "SegoeUI.ttf", 16 ), Font::ALIGN_TOP_LEFT ) );
	MessageInput->ReturnDeselects = false;
	MessageInput->PassReturn = true;
	MessageInput->EscDeselects = false;
	MessageInput->PassEsc = true;
	MessageInput->TabDeselects = false;
	MessageInput->PassTab = true;
	MessageInput->ClickOutDeselects = false;
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
	Cam.Fwd.Set( 1., 0., 0. );
	Cam.Up.Set( 0., 0., 1. );
	Cam.FixVectors();
}


RenderLayer::~RenderLayer()
{
}


void RenderLayer::SetBackground( void )
{
	if( BackgroundName != Raptor::Game->Data.PropertyAsString("bg") )
	{
		BackgroundName = Raptor::Game->Data.PropertyAsString("bg");
		
		if( ! Raptor::Game->Data.PropertyAsString("bg").empty() )
			Background.BecomeInstance( Raptor::Game->Res.GetAnimation( Raptor::Game->Data.PropertyAsString("bg") + std::string(".ani") ) );
		else
			Background.BecomeInstance( Raptor::Game->Res.GetAnimation("stars.ani") );
	}
}


void RenderLayer::SetWorldLights( bool deathstar, float ambient_scale, const std::vector<Vec3D> *obstructions )
{
	Color ambient;
	Vec3D dir[ 4 ];
	Color color[ 4 ];
	float wrap_around[ 4 ];
	
	if( deathstar )
	{
		ambient.Set( 0.7f, 0.7f, 0.7f, 1.f );
		
		dir[ 0 ].Set( -0.6, 0., 0.8 );
		dir[ 0 ].ScaleTo( 1. );
		color[ 0 ].Set( 0.85, 0.85, 0.85, 1.f );
		wrap_around[ 0 ] = 0.;
		
		dir[ 1 ].Set( 0.15, 0.57, 0.8 );
		dir[ 1 ].ScaleTo( 1. );
		color[ 1 ].Set( 0.8, 0.85, 0.8, 1.f );
		wrap_around[ 1 ] = 0.125;
		
		dir[ 2 ].Set( -0.01, -0.86, 0.51 );
		dir[ 2 ].ScaleTo( 1. );
		color[ 2 ].Set( 0.2, 0.29, 0.41, 1.f );
		wrap_around[ 2 ] = 0.25;
		
		dir[ 3 ].Set( 0.2, 0.1, 0.9 );
		dir[ 3 ].ScaleTo( 1. );
		color[ 3 ].Set( 0.25, 0.125, 0.2, 1.f );
		wrap_around[ 3 ] = 0.125;
	}
	else if( Raptor::Game->Data.PropertyAsString("bg") == "nebula" )
	{
		ambient.Set( 0.75f, 0.7f, 0.75f, 1.f );
		
		dir[ 0 ].Set( 0.897, -0.389, 0.208 );
		dir[ 0 ].ScaleTo( 1. );
		color[ 0 ].Set( 1.3, 1.3, 1.29, 1.f );
		wrap_around[ 0 ] = 0.5;
		
		dir[ 1 ].Set( -0.748, 0.66, -0.07 );
		dir[ 1 ].ScaleTo( 1. );
		color[ 1 ].Set( 0.6, 0.2, 0.1, 1.f );
		wrap_around[ 1 ] = 1.0;
		
		dir[ 2 ].Set( 0.555, -0.832, -0.023 );
		dir[ 2 ].ScaleTo( 1. );
		color[ 2 ].Set( 0.1, 0.05, 0.5, 1.f );
		wrap_around[ 2 ] = 0.5;
		
		dir[ 3 ].Set( -0.421, -0.396, -0.821 );
		dir[ 3 ].ScaleTo( 1. );
		color[ 3 ].Set( 0.01, 0.1, 0.05, 1.f );
		wrap_around[ 3 ] = 0.125;
	}
	else if( Raptor::Game->Data.PropertyAsString("bg") == "nebula2" )
	{
		ambient.Set( 0.85f, 0.74f, 0.75f, 1.f );
		
		dir[ 0 ].Set( 0.604, -0.384, 0.698 );
		dir[ 0 ].ScaleTo( 1. );
		color[ 0 ].Set( 1.31f, 1.3f, 1.29f, 1.f );
		wrap_around[ 0 ] = 0.75;
		
		dir[ 1 ].Set( 0.7, -0.579, -0.419 );
		dir[ 1 ].ScaleTo( 1. );
		color[ 1 ].Set( 0.f, 0.25f, 0.9f, 1.f );
		wrap_around[ 1 ] = 0.15;
		
		dir[ 2 ].Set( 0.255, 0.747, 0.594 );
		dir[ 2 ].ScaleTo( 1. );
		color[ 2 ].Set( 0.f, 0.3f, 0.8f, 1.f );
		wrap_around[ 2 ] = 0.2;
		
		dir[ 3 ].Set( 0.372, -0.285, 0.884 );
		dir[ 3 ].ScaleTo( 1. );
		color[ 3 ].Set( 0.5f, 0.3f, 0.1f, 1.f );
		wrap_around[ 3 ] = 0.25;
	}
	else
	{
		ambient.Set( 0.72f, 0.7f, 0.8f, 1.f );
		
		dir[ 0 ].Set( 0.07, -0.8, 0.6 );
		dir[ 0 ].ScaleTo( 1. );
		color[ 0 ].Set( 1.3, 1.3, 1.29, 1.f );
		wrap_around[ 0 ] = 0.5;
		
		dir[ 1 ].Set( 0.9, 0.3, 0.25 );
		dir[ 1 ].ScaleTo( 1. );
		color[ 1 ].Set( 0.22, 0.2, 0.2, 1.f );
		wrap_around[ 1 ] = 0.875;
		
		dir[ 2 ].Set( -0.01, -0.86, -0.51 );
		dir[ 2 ].ScaleTo( 1. );
		color[ 2 ].Set( 0.2, 0.15, 0.3, 1.f );
		wrap_around[ 2 ] = 1.0;
		
		dir[ 3 ].Set( -0.35, -0.75, -0.56 );
		dir[ 3 ].ScaleTo( 1. );
		color[ 3 ].Set( 0.03, 0.08, 0.05, 1.f );
		wrap_around[ 3 ] = 0.125;
	}
	
	ambient *= ambient_scale;
	Raptor::Game->ShaderMgr.Set3f( "AmbientLight", ambient.Red, ambient.Green, ambient.Blue );
	
	char uniform_name[ 128 ] = "";
	for( int i = 0; i < 4; i ++ )
	{
		if( obstructions )
		{
			for( std::vector<Vec3D>::const_iterator obst_iter = obstructions->begin(); obst_iter != obstructions->end(); obst_iter ++ )
			{
				double obst_length = obst_iter->Length();
				if( ! obst_length )
					continue;
				
				Vec3D obst_dir = obst_iter->Unit();
				double similarity = dir[ i ].Dot( &obst_dir );
				if( similarity < 0. )
					continue;
				
				color[ i ] /= pow( obst_length, sqrt(similarity) );
			}
		}
		
		snprintf( uniform_name, 128, "DirectionalLight%iDir", i );
		Raptor::Game->ShaderMgr.Set3f( uniform_name, dir[ i ].X, dir[ i ].Y, dir[ i ].Z );
		snprintf( uniform_name, 128, "DirectionalLight%iColor", i );
		Raptor::Game->ShaderMgr.Set3f( uniform_name, color[ i ].Red, color[ i ].Green, color[ i ].Blue );
		snprintf( uniform_name, 128, "DirectionalLight%iWrapAround", i );
		Raptor::Game->ShaderMgr.Set1f( uniform_name, wrap_around[ i ] );
	}
}


void RenderLayer::SetDynamicLights( Pos3D *pos, Pos3D *offset, int dynamic_lights, std::list<Shot*> *shots, std::list<Effect*> *effects )
{
	char uniform_name[ 128 ] = "";
	
	ClearDynamicLights();
	
	// Sort all possible light sources by proximity to the object we are drawing.
	std::multimap<double,Pos3D*> nearest_shots   = pos->Nearest( (std::list<Pos3D*> *) shots,   dynamic_lights );
	std::multimap<double,Pos3D*> nearest_effects = pos->Nearest( (std::list<Pos3D*> *) effects, dynamic_lights );
	
	// If there are any SuperLaser shots, make sure at least the nearest of them is used as a light source.
	const Shot *superlaser = NULL;
	if( dynamic_lights )
	{
		for( std::multimap<double,Pos3D*>::iterator shot_iter = nearest_shots.begin(); shot_iter != nearest_shots.end(); shot_iter ++ )
		{
			if( ((const Shot*)( shot_iter->second ))->ShotType == Shot::TYPE_SUPERLASER )
			{
				superlaser = (const Shot*) shot_iter->second;
				nearest_shots.erase( shot_iter );
				break;
			}
		}
		if( shots && ! superlaser )
		{
			double nearest = 0.;
			for( std::list<Shot*>::const_iterator shot_iter = shots->begin(); shot_iter != shots->end(); shot_iter ++ )
			{
				if( (*shot_iter)->ShotType == Shot::TYPE_SUPERLASER )
				{
					double dist = pos->Dist( *shot_iter );
					if( (dist < nearest) || ! superlaser )
					{
						superlaser = *shot_iter;
						nearest = dist;
					}
				}
			}
		}
	}
	
	for( int i = 0; i < dynamic_lights; i ++ )
	{
		Shot *shot     = (Shot*)(   nearest_shots.size()   ? nearest_shots.begin()->second   : NULL );
		Effect *effect = (Effect*)( nearest_effects.size() ? nearest_effects.begin()->second : NULL );
		const Pos3D *nearest = NULL;
		Color color( 1.f, 1.f, 1.f, 15.f );
		
		#define EXPLOSION_LIGHT_RADIUS (effect->Size * 4.5)
		
		if( shot && effect )
		{
			if( nearest_shots.begin()->first - shot->LightColor().Alpha < nearest_effects.begin()->first - EXPLOSION_LIGHT_RADIUS )
				effect = NULL;
			else
				shot = NULL;
		}
		
		if( superlaser )
		{
			nearest = (const Pos3D*) superlaser;
			color = superlaser->LightColor();
			superlaser = NULL;
		}
		else if( shot )
		{
			nearest = (const Pos3D*) shot;
			color = shot->LightColor();
			nearest_shots.erase( nearest_shots.begin() );
		}
		else if( effect )
		{
			nearest = (const Pos3D*) effect;
			float animation_time = effect->Anim.LoopTime() * effect->Anim.PlayCount;
			if( animation_time )
			{
				float progress = effect->Anim.Timer.ElapsedSeconds() / animation_time;
				color.Green = std::max<float>( 0.25f, 1.f - 2.f * progress * progress );
				color.Blue = std::max<float>( 0.f, 1.f - 3.f * progress );
				color.Alpha = EXPLOSION_LIGHT_RADIUS * sinf( sqrtf( progress ) * M_PI );
			}
			nearest_effects.erase( nearest_effects.begin() );
		}
		else
			break;
		
		snprintf( uniform_name, 128, "PointLight%iPos", i );
		if( offset )
			Raptor::Game->ShaderMgr.Set3f( uniform_name, nearest->X - offset->X, nearest->Y - offset->Y, nearest->Z - offset->Z );
		else
			Raptor::Game->ShaderMgr.Set3f( uniform_name, nearest->X, nearest->Y, nearest->Z );
		
		snprintf( uniform_name, 128, "PointLight%iColor", i );
		Raptor::Game->ShaderMgr.Set3f( uniform_name, color.Red, color.Green, color.Blue );
		snprintf( uniform_name, 128, "PointLight%iRadius", i );
		Raptor::Game->ShaderMgr.Set1f( uniform_name, color.Alpha );
	}
}


void RenderLayer::SetBlastPoints( int blastpoints, std::vector<BlastPoint> *bp_vec )
{
	if( ! bp_vec )
	{
		ClearBlastPoints( blastpoints );
		return;
	}
	
	for( int i = 0; i < blastpoints; i ++ )
	{
		std::string index = std::string("[") + Num::ToString( i ) + std::string("]");
		if( i < (int) bp_vec->size() )
		{
			Vec3D bp_pos = bp_vec->at( i );  // NOTE: In v0.5.2+ the vertex shader translates these to worldspace.
			double radius = bp_vec->at( i ).Radius * std::min<double>( 1., bp_vec->at( i ).Lifetime.Progress() );
			Raptor::Game->ShaderMgr.Set3f( (std::string("BlastPoint")  + index).c_str(), bp_pos.X, bp_pos.Y, bp_pos.Z );
			Raptor::Game->ShaderMgr.Set1f( (std::string("BlastRadius") + index).c_str(), radius );
		}
		else
		{
			Raptor::Game->ShaderMgr.Set3f( (std::string("BlastPoint")  + index).c_str(), 0., 0., 0. );
			Raptor::Game->ShaderMgr.Set1f( (std::string("BlastRadius") + index).c_str(), 0. );
		}
	}
}


void RenderLayer::ClearWorldLights( void )
{
	Raptor::Game->ShaderMgr.Set3f( "AmbientLight", 1., 1., 1. );
	Raptor::Game->ShaderMgr.Set3f( "DirectionalLight0Dir", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set3f( "DirectionalLight0Color", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set1f( "DirectionalLight0WrapAround", 0. );
	Raptor::Game->ShaderMgr.Set3f( "DirectionalLight1Dir", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set3f( "DirectionalLight1Color", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set1f( "DirectionalLight1WrapAround", 0. );
	Raptor::Game->ShaderMgr.Set3f( "DirectionalLight2Dir", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set3f( "DirectionalLight2Color", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set1f( "DirectionalLight2WrapAround", 0. );
	Raptor::Game->ShaderMgr.Set3f( "DirectionalLight3Dir", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set3f( "DirectionalLight3Color", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set1f( "DirectionalLight3WrapAround", 0. );
}


void RenderLayer::ClearDynamicLights( void )
{
	Raptor::Game->ShaderMgr.Set3f( "PointLight0Pos", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set3f( "PointLight0Color", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set1f( "PointLight0Radius", 0. );
	Raptor::Game->ShaderMgr.Set3f( "PointLight1Pos", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set3f( "PointLight1Color", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set1f( "PointLight1Radius", 0. );
	Raptor::Game->ShaderMgr.Set3f( "PointLight2Pos", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set3f( "PointLight2Color", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set1f( "PointLight2Radius", 0. );
	Raptor::Game->ShaderMgr.Set3f( "PointLight3Pos", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set3f( "PointLight3Color", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set1f( "PointLight3Radius", 0. );
}


void RenderLayer::ClearMaterial( void )
{
	Raptor::Game->ShaderMgr.Set3f( "AmbientColor", 1., 1., 1. );
	Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set3f( "SpecularColor", 0., 0., 0. );
	Raptor::Game->ShaderMgr.Set1f( "Alpha", 1. );
	Raptor::Game->ShaderMgr.Set1f( "Shininess", 0. );
}


void RenderLayer::ClearBlastPoints( int blastpoints )
{
	for( int i = 0; i < blastpoints; i ++ )
	{
		std::string index = std::string("[") + Num::ToString( i ) + std::string("]");
		Raptor::Game->ShaderMgr.Set3f( (std::string("BlastPoint")  + index).c_str(), 0., 0., 0. );
		Raptor::Game->ShaderMgr.Set1f( (std::string("BlastRadius") + index).c_str(), 0. );
	}
}


void RenderLayer::Draw( void )
{
	glPushMatrix();
	
	XWingGame *game = (XWingGame*) Raptor::Game;
	
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = game->Gfx.W;
	Rect.h = game->Gfx.H;
	
	bool vr = game->Head.VR && game->Gfx.DrawTo;
	if( vr )
	{
		Rect.x = game->Gfx.W/2 - 640/2;
		Rect.y = game->Gfx.H/2 - 480/2;
		Rect.w = 640;
		Rect.h = 480;
	}
	
	bool cinematic = (game->Cfg.SettingAsBool("cinematic") || game->Cfg.SettingAsBool("screensaver")) && ! game->Input.ControlPressed( XWing::Control::SCORES );
	
	
	// Update contained elements.
	
	if( game->Console.IsActive() )
	{
		Selected = NULL;
		MessageInput->Visible = false;
	}
	else if( MessageInput->IsSelected() )
	{
		MessageInput->Rect.x = 3;
		MessageInput->Rect.y = 3;
		MessageInput->Rect.w = Rect.w - (MessageInput->Rect.x * 2);
		MessageInput->Rect.h = MessageInput->TextFont->GetHeight();
		
		if( vr )
		{
			MessageInput->Rect.w /= 2;
			MessageInput->Rect.x += MessageInput->Rect.w / 2;
		}
	}
	
	if( IsTop() )
	{
		game->Mouse.ShowCursor = false;
		game->ReadMouse = (game->Cfg.SettingAsString("mouse_mode") != "disabled");
	}
	
	
	// Build a list of all ships, because we'll refer to it often.
	// Also build a list of shots for use when determining dynamic lights.
	// And keep track of the Death Star trench location for chase camera and darkening world lights.
	
	std::list<Ship*> ships;
	std::list<Turret*> turrets;
	Turret *player_turret = NULL;
	std::list<Shot*> shots;
	std::list<Asteroid*> asteroids;
	DeathStar *deathstar = NULL;
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = game->Data.GameObjects.begin(); obj_iter != game->Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->Type() == XWing::Object::SHIP )
			ships.push_back( (Ship*) obj_iter->second );
		else if( obj_iter->second->Type() == XWing::Object::SHOT )
			shots.push_back( (Shot*) obj_iter->second );
		else if( obj_iter->second->Type() == XWing::Object::ASTEROID )
			asteroids.push_back( (Asteroid*) obj_iter->second );
		else if( obj_iter->second->Type() == XWing::Object::DEATH_STAR )
			deathstar = (DeathStar*) obj_iter->second;
		else if( obj_iter->second->Type() == XWing::Object::TURRET )
		{
			turrets.push_back( (Turret*) obj_iter->second );
			if( obj_iter->second->PlayerID == game->PlayerID )
				player_turret = (Turret*) obj_iter->second;
		}
	}
	
	std::list<Effect*> effects;
	for( std::list<Effect>::iterator effect_iter = game->Data.Effects.begin(); effect_iter != game->Data.Effects.end(); effect_iter ++ )
	{
		if( effect_iter->Lifetime.Progress() >= 1. )  // Don't use effects that haven't started yet as light sources.
			effects.push_back( &*effect_iter );
	}
	
	
	// Determine which ship we're observing, and in what view.
	
	Ship *observed_ship = NULL;
	Ship *player_ship = NULL;
	Turret *observed_turret = NULL;
	game->View = XWing::View::AUTO;
	
	if( player_turret )
	{
		// Player is controlling a turret.
		
		observed_turret = player_turret;
		player_ship = player_turret->ParentShip();
		observed_ship = player_ship;
		game->View = XWing::View::GUNNER;
	}
	else
	{
		// Look for the player's ship.
		
		for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
		{
			if( (*ship_iter)->PlayerID == game->PlayerID )
			{
				player_ship = *ship_iter;
				
				// Only observe the player's ship if alive or recently dead.
				if( (player_ship->Health > 0.) || (player_ship->DeathClock.ElapsedSeconds() < 6.) )
				{
					observed_ship = player_ship;
					game->View = XWing::View::COCKPIT;
				}
				break;
			}
		}
	}
	
	
	// Determine the selected view.
	
	std::string view_str = game->Cfg.SettingAsString( ((game->View == XWing::View::COCKPIT) || (game->View == XWing::View::GUNNER)) ? "view" : "spectator_view" );
	if( view_str == "auto" )
		view_str = game->Cfg.SettingAsString("view");  // If spectator_view is auto but view is not, use view.
	if( view_str == "cockpit" )
		game->View = XWing::View::COCKPIT;
	else if( view_str == "gunner" )
		game->View = XWing::View::GUNNER;
	else if( (view_str == "crosshair") || (view_str == "padlock") )
		game->View = observed_turret ? XWing::View::GUNNER : XWing::View::COCKPIT;
	else if( view_str == "selfie" )
		game->View = XWing::View::SELFIE;
	else if( view_str == "chase" )
		game->View = XWing::View::CHASE;
	else if( Str::BeginsWith( view_str, "cinema" ) )
		game->View = XWing::View::CINEMA;
	else if( view_str == "fixed" )
		game->View = XWing::View::FIXED;
	else if( view_str == "stationary" )
		game->View = XWing::View::STATIONARY;
	else if( view_str == "instruments" )
		game->View = XWing::View::INSTRUMENTS;
	else if( view_str == "cycle" )
		game->View = XWing::View::CYCLE;
	
	
	if( ! (observed_ship || observed_turret) )
	{
		// This player has no ship or turret alive; let's watch somebody else.
		
		Player *player = game->Data.GetPlayer( game->PlayerID );
		uint8_t player_team = XWing::Team::NONE;
		uint8_t player_group = 0;
		
		if( player_ship )
		{
			player_team = player_ship->Team;
			player_group = player_ship->Group;
		}
		else if( player && (player->PropertyAsString("team") == "Rebel") )
			player_team = XWing::Team::REBEL;
		else if( player && (player->PropertyAsString("team") == "Empire") )
			player_team = XWing::Team::EMPIRE;
		
		if( player_team && player )
			player_group = player->PropertyAsInt("group",player_group);
		
		const GameObject *prev_observed = game->Data.GetObject( game->ObservedShipID );
		uint32_t prev_hit_by = (prev_observed && (prev_observed->Type() == XWing::Object::SHIP)) ? ((const Ship*)( prev_observed ))->HitByID : 0;
		
		uint8_t best_score = 0x00;
		double best_dist = FLT_MAX;
		bool early_game = (game->Data.GameTime.ElapsedSeconds() < 10.);
		
		for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
		{
			// Don't spectate the Death Star exhaust port or any long-dead or jumped-out ships.
			if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
				continue;
			if( ( (((*ship_iter)->Health <= 0.) && ((*ship_iter)->DeathClock.ElapsedSeconds() >= 6.)) || ((*ship_iter)->JumpedOut && ((*ship_iter)->JumpProgress >= 1.)) ) && (game->View != XWing::View::INSTRUMENTS) )
				continue;
			
			uint8_t ship_score = 0x00;
			double ship_dist = game->Cam.Dist( *ship_iter );
			
			// If we'd selected a specific ship to watch, choose that immediately.
			if( (*ship_iter)->ID == game->ObservedShipID )
			{
				observed_ship = *ship_iter;
				break;
			}
			
			// Prefer whomever just killed us, if they are still alive.
			if( ((*ship_iter)->ID == prev_hit_by) && ((*ship_iter)->Health > 0.) )
				ship_score |= 0x80;
			
			// Prefer player-owned live ships.
			if( (*ship_iter)->Owner() && ((*ship_iter)->Health > 0.) )
				ship_score |= 0x40;
			
			// Prefer non-capital ships, especially fighters at game start.
			if( ((*ship_iter)->Category() != ShipClass::CATEGORY_CAPITAL) && ((*ship_iter)->Category() != ShipClass::CATEGORY_TRANSPORT) )
			{
				if( early_game && ((*ship_iter)->Category() == ShipClass::CATEGORY_FIGHTER) )
					ship_score |= 0x30;
				else if( early_game && ((*ship_iter)->Category() == ShipClass::CATEGORY_GUNBOAT) )
					ship_score |= 0x20;
				else
					ship_score |= 0x10;
			}
			
			// Prefer live ships.
			if( (*ship_iter)->Health > 0. )
				ship_score |= 0x08;
			
			// Prefer ships in our flight group.
			if( (*ship_iter)->Team == player_team )
			{
				if( player_group && ((*ship_iter)->Group == player_group) )
					ship_score |= 0x04;
				/*
				else
					// If not in our flight group, prefer our team.
					// NOTE: Commented-out because it doesn't make sense to do this but also prefer our last attacker.
					ship_score |= 0x02;
				*/
			}
			
			/*
			// Prefer higher ID than previously observed ship.
			// NOTE: Commented-out because it doesn't make sense to do this when also sorting by distance.
			if( (*ship_iter)->ID >= game->ObservedShipID )
				ship_score |= 0x01;
			*/
			
			if( (ship_score > best_score) || ((ship_score == best_score) && (ship_dist < best_dist) && game->ObservedShipID) || ! observed_ship )
			{
				observed_ship = *ship_iter;
				best_score = ship_score;
				best_dist = ship_dist;
			}
		}
	}
	
	
	if( (game->View == XWing::View::CYCLE) || ((game->View == XWing::View::AUTO) && !(player_ship || player_turret || vr)) )
	{
		double cycle_time = game->Cfg.SettingAsDouble("view_cycle_time",7.);
		if( cycle_time <= 0. )
			cycle_time = 7.;

		uint8_t ship_category = observed_ship ? observed_ship->Category() : (uint8_t) ShipClass::CATEGORY_UNKNOWN;
		bool ship_turret = observed_ship && observed_ship->AttachedTurret();
		
		#define CYCLE_VIEWS 5
		double time_elapsed = observed_ship ? observed_ship->Lifetime.ElapsedSeconds() : PlayTime.ElapsedSeconds();
		if( ! game->Cfg.SettingAsBool("spectator_sync") )
			time_elapsed -= ((game->PlayerID - 1) % CYCLE_VIEWS) * cycle_time;  // Add variety if there are multiple spectators.
		double cam_picker = fmod( time_elapsed, cycle_time * CYCLE_VIEWS );
		
		if( cam_picker < cycle_time )
			game->View = XWing::View::CINEMA;
		else if( cam_picker < cycle_time * 2. )
			game->View = ((ship_category == ShipClass::CATEGORY_CAPITAL) && ship_turret) ? XWing::View::GUNNER : ((ship_category == ShipClass::CATEGORY_TRANSPORT) ? XWing::View::FIXED : XWing::View::COCKPIT);
		else if( cam_picker < cycle_time * 3. )
			game->View = (ship_category == ShipClass::CATEGORY_CAPITAL) ? XWing::View::FIXED : XWing::View::SELFIE;
		else if( cam_picker < cycle_time * 4. )
			game->View = (ship_category == ShipClass::CATEGORY_CAPITAL) ? XWing::View::CINEMA : XWing::View::CHASE;
		else if( cam_picker < cycle_time * 5. )
			game->View = ship_turret ? XWing::View::GUNNER : XWing::View::COCKPIT;
		else if( cam_picker < cycle_time * 6. )
			game->View = XWing::View::CINEMA;
		else
			game->View = ship_turret ? XWing::View::GUNNER : XWing::View::FIXED;
	}
	
	
	if( (game->View == XWing::View::GUNNER) && ! player_turret )
	{
		// Spectating from gunner view.  Find a turret to watch.
		
		const Player *me = game->Data.GetPlayer( game->PlayerID );
		uint8_t my_team = XWing::Team::NONE;
		std::string team_str = me->PropertyAsString("team");
		if( team_str == "Rebel" )
			my_team = XWing::Team::REBEL;
		else if( team_str == "Empire" )
			my_team = XWing::Team::EMPIRE;
		
		int best_score = 0;
		double best_lifetime = 0.;
		
		for( std::list<Turret*>::iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
		{
			Turret *turret = *turret_iter;
			Ship *parent = turret->ParentShip();
			uint32_t parent_id = parent ? parent->ID : 0;
			
			Pos3D *above = parent ? (Pos3D*) parent : (deathstar ? (Pos3D*) deathstar : NULL);
			Vec3D up = above ? above->Up : Vec3D(0,0,1);
			
			int score = (turret->Up.Dot( &up ) + 1.) * 127.9;
			score <<= 8;  // Score mask for being upright: 0x0000FF00
			
			if( above )
			{
				double height = turret->DistAlong( &(above->Up), above );
				if( above == deathstar )
					height += deathstar->TrenchDepth;
				score += std::min<int>( 0x000000FF, std::max<int>( 0, height ) );
			}
			
			if( parent_id == game->ObservedShipID )
				score += 0x01000000;
			
			if( turret->Team == my_team )
				score += 0x00400000;
			
			if( turret->PlayerID )
				score += 0x00200000;
			
			if( parent )
			{
				score += 0x00040000;
				
				if( parent == player_ship )
					score += 0x00800000;
				
				if( parent->PlayerID )
					score += 0x00080000;
				
				if( parent->Category() != ShipClass::CATEGORY_CAPITAL )
					score += 0x00020000;
				
				if( ! parent->IsMissionObjective )
					score += 0x00010000;
			}
			
			if( score > best_score )
			{
				observed_turret = turret;
				best_score = score;
				best_lifetime = turret->Lifetime.ElapsedSeconds();
			}
			else if( score == best_score )
			{
				double lifetime = turret->Lifetime.ElapsedSeconds();
				if( lifetime > best_lifetime )
				{
					observed_turret = turret;
					best_lifetime = lifetime;
				}
			}
		}
		
		if( (observed_ship && (observed_ship->Health < 0.))
		||  (observed_turret && player_ship && (observed_turret->ParentShip() != player_ship)) )
			observed_turret = NULL;
		
		if( observed_turret )
			observed_ship = observed_turret->ParentShip();
		else
			game->View = observed_ship ? XWing::View::CHASE : XWing::View::AUTO;
	}
	
	
	// Determine which view we'll actually render.
	
	bool ejected = (game->View == XWing::View::COCKPIT) && observed_ship && (observed_ship->Health <= 0.);
	
	if( game->View == XWing::View::AUTO )
		game->View = vr ? XWing::View::FIXED : XWing::View::CINEMA;
	
	if( (game->View == XWing::View::COCKPIT) && ((! observed_ship) || (observed_ship->Health <= 0.)) )
		game->View = (observed_turret && ! observed_ship) ? XWing::View::GUNNER : XWing::View::CHASE;
	else if( (game->View == XWing::View::GUNNER) && ! observed_turret )
		game->View = XWing::View::CHASE;
	
	if( game->View == XWing::View::STATIONARY )
	{
		observed_ship = NULL;
		observed_turret = NULL;
	}
	
	
	Player *observed_player = NULL;
	GameObject *target_obj = NULL;
	Ship *target = NULL;
	Ship *dead_target = NULL;
	double jump_progress = 2.;
	bool jumped_out = false, in_hyperspace = false;
	
	
	if( observed_ship )
	{
		game->ObservedShipID = observed_ship->ID;
		
		jump_progress = observed_ship->JumpProgress;
		jumped_out    = observed_ship->JumpedOut;
		in_hyperspace = jumped_out ? (jump_progress > 1.) : (jump_progress < 0.);
		
		if( observed_ship->Target )
		{
			target_obj = game->Data.GetObject( observed_ship->Target );
			if( target_obj && (target_obj->Type() == XWing::Object::SHIP) )
			{
				target = (Ship*) target_obj;
				if( target->Health <= 0. )
				{
					if( target->DeathClock.ElapsedSeconds() <= 2. )
						dead_target = target;
					target = NULL;
				}
			}
		}
		
		
		Ship *cinema_view_with = NULL;
		if( game->View == XWing::View::CINEMA )
		{
			double best = 0.;
			
			// Try to focus the cinema view on an alive or recently-dead enemy.
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				if( (*ship_iter)->ID != observed_ship->ID )
				{
					Ship *ship = *ship_iter;
					
					if( (ship->Team != observed_ship->Team) && ((ship->Health > 0.) || (ship->DeathClock.ElapsedSeconds() <= 5.)) )
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
			
			// If we found no suitable other ship with the original criteria, be less picky.
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
				game->View = XWing::View::FIXED;
		}
		
		
		if( game->View == XWing::View::COCKPIT )
			Cam = observed_ship->HeadPos();
		else if( game->View == XWing::View::SELFIE )
		{
			Cam = observed_ship->HeadPos();
			Cam.RotateAround( &(observed_ship->Up), 180. );
		}
		else if( (game->View == XWing::View::GUNNER) && observed_turret )
		{
			observed_turret->UpdatePos();
			Cam = observed_turret->HeadPos();
		}
		else
		{
			Cam.Copy( observed_ship );
			if( game->View == XWing::View::CHASE )
			{
				Cam.Yaw  ( (observed_ship->YawRate   + observed_ship->PrevYawRate  ) * -0.025 );
				Cam.Pitch( (observed_ship->PitchRate + observed_ship->PrevPitchRate) * -0.025 );
				Cam.Roll ( (observed_ship->RollRate  + observed_ship->PrevRollRate ) * -0.015 );
			}
		}
		
		if( (view_str == "padlock") && (observed_ship->Health > 0.) )
		{
			GameObject *target_obj = Raptor::Game->Data.GetObject( observed_turret ? observed_turret->Target : observed_ship->Target );
			if( target_obj )
			{
				Cam.Fwd = (((target && observed_ship->TargetSubsystem) ? target->TargetCenter( observed_ship->TargetSubsystem ) : *target_obj) - *observed_ship).Unit();
				Cam.FixVectors();
			}
		}
		
		if( game->View == XWing::View::FIXED )
		{
			// Fixed-angle external camera.
			Cam.Fwd.Set( 1., 0., 0. );
			Cam.Up.Set( 0., 0., 1. );
			Cam.FixVectors();
		}
		
		// Apply camera angle change.
		Cam.Yaw( game->LookYaw );
		Cam.Pitch( game->LookPitch );
		
		if( game->View == XWing::View::CINEMA )
		{
			// Cinematic camera.
			
			// Point the camera at one ship looking through to the other.
			Cam.Up.Set( 0., 0., 1. );
			Vec3D vec_to_other( cinema_view_with->X - observed_ship->X, cinema_view_with->Y - observed_ship->Y, cinema_view_with->Z - observed_ship->Z );
			vec_to_other.ScaleTo( 1. );
			Cam.Fwd = vec_to_other;
			Cam.FixVectors();
			
			// Apply camera angle change.
			Cam.Yaw( game->LookYaw );
			Cam.Pitch( game->LookPitch );
			
			// Move camera back from the ship, moreso when jumping in from hyperspace.
			double camera_dist = -30. - observed_ship->Shape.GetTriagonal();
			double lifetime = observed_ship->Lifetime.ElapsedSeconds();
			if( lifetime < 1.5 )
				camera_dist *= cos( lifetime * M_PI / 1.5 ) + 2.;
			Cam.MoveAlong( &(Cam.Fwd), camera_dist );
			
			// Move the camera up or down.
			Cam.MoveAlong( &(Cam.Up), observed_ship->Shape.GetTriagonal() * Vec3D(0.,0.,1.).Dot(&vec_to_other) + observed_ship->Shape.GetHeight() );
			
			// Point the camera at the mid-point (in 3D space) between the two ships.
			Vec3D mid_point( (cinema_view_with->X + observed_ship->X) / 2., (cinema_view_with->Y + observed_ship->Y) / 2., (cinema_view_with->Z + observed_ship->Z) / 2. );
			Vec3D vec_to_mid( mid_point.X - Cam.X, mid_point.Y - Cam.Y, mid_point.Z - Cam.Z );
			vec_to_mid.ScaleTo( 1. );
			Cam.Fwd = vec_to_mid;
			Cam.FixVectors();
			
			if( observed_ship->Health <= 0. )
			{
				// Make sure the camera is not inside another ship.
				for( std::list<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					if( (*ship_iter) == observed_ship )
						continue;
					if( (*ship_iter)->Health <= 0. )
						continue;
					if( (*ship_iter)->JumpedOut )
						continue;
					if( (*ship_iter)->Category() != ShipClass::CATEGORY_CAPITAL )
						continue;
					double dist = Cam.Dist(*ship_iter);
					if( dist < (*ship_iter)->Radius() )
						Cam.MoveAlong( &(Cam.Fwd), dist - (*ship_iter)->Radius() );
				}
			}
			
			// Apply camera angle change again.
			Cam.Yaw( game->LookYaw );
			Cam.Pitch( game->LookPitch );
		}
		
		else if( (game->View == XWing::View::CHASE) || (game->View == XWing::View::FIXED) )
		{
			// Move camera back from the ship, moreso when dead or jumping in from hyperspace.
			double camera_dist = -20. - observed_ship->Shape.GetTriagonal();
			if( ejected )
				camera_dist *= std::min<double>( 1., sqrt(observed_ship->DeathClock.ElapsedSeconds()) );
			double lifetime = observed_ship->Lifetime.ElapsedSeconds();
			if( lifetime < 1.5 )
				camera_dist *= cos( lifetime * M_PI / 1.5 ) + 2.;
			else if( observed_ship->Health <= 0. )
				camera_dist += observed_ship->DeathClock.ElapsedSeconds() * -35.;
			Cam.MoveAlong( &(Cam.Fwd), camera_dist );
			
			if( deathstar )
			{
				double dist_horizontal = fabs( observed_ship->DistAlong( &(deathstar->Right), deathstar ) );
				double dist_above = observed_ship->DistAlong( &(deathstar->Up), deathstar );
				double min_height = 10.;
				if( dist_horizontal < deathstar->TrenchWidth / 2. - 5. )
					min_height -= deathstar->TrenchDepth;
				if( dist_above < min_height )
					Cam.MoveAlong( &(deathstar->Up), min_height - dist_above );
			}
			
			if( observed_ship->Health <= 0. )
			{
				// Make sure the camera is not inside another ship.
				for( std::list<Ship*>::const_iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					if( (*ship_iter) == observed_ship )
						continue;
					if( (*ship_iter)->Health <= 0. )
						continue;
					if( (*ship_iter)->JumpedOut )
						continue;
					if( (*ship_iter)->Category() != ShipClass::CATEGORY_CAPITAL )
						continue;
					double dist = Cam.Dist(*ship_iter);
					if( dist < (*ship_iter)->Radius() )
						Cam.MoveAlong( &(Cam.Fwd), dist - (*ship_iter)->Radius() );
				}
			}
		}
		
		else if( game->View == XWing::View::SELFIE )
		{
			// Move camera in front of head position (looking back), moreso when dead.
			std::string ship_class = (observed_ship && observed_ship->Class) ? observed_ship->Class->ShortName : "T/F";
			double camera_dist = game->Cfg.SettingAsDouble("g_selfie_dist"); // FIXME: Read from ShipClass?
			double camera_up   = game->Cfg.SettingAsDouble("g_selfie_up");   //
			if( ! camera_dist )
			{
				if( observed_ship->Category() == ShipClass::CATEGORY_CAPITAL )
					camera_dist = 160.;
				else if( observed_ship->Category() == ShipClass::CATEGORY_TRANSPORT )
					camera_dist = 25.;
				// FIXME: Offsets to selfie cam should be defined in ShipClass, not hard-coded here!
				else if( Str::BeginsWith( ship_class, "T/" ) )
					camera_dist = 16.;
				else if( Str::BeginsWith( ship_class, "YT" ) )
				{
					camera_dist = 1.;
					if( ! camera_up )
						camera_up = 0.3;
				}
				else if( Str::BeginsWith( ship_class, "GUN" ) )
					camera_dist = 1.4;
				else
					camera_dist = 1.3;
			}
			
			if( observed_ship->Health <= 0. )
				camera_dist += observed_ship->DeathClock.ElapsedSeconds() * 66.;
			
			Cam.MoveAlong( &(observed_ship->Up), camera_up );
			Cam.MoveAlong( &(Cam.Fwd), camera_dist * -1. );
		}
	}
	else
	{
		game->ObservedShipID = 0;
		
		if( observed_turret )
		{
			// Viewing a Death Star surface turret.
			Cam = observed_turret->HeadPos();
			game->View = XWing::View::GUNNER;
		}
		else
			game->View = XWing::View::STATIONARY;
		
		// Apply camera angle change, then clear it to prevent endless spinning.
		Cam.Yaw( game->LookYaw );
		Cam.Pitch( game->LookPitch );
		game->LookYaw = game->LookPitch = 0.;
	}
	
	// Determine which player we are observing.
	if( observed_turret )
		observed_player = observed_turret->Owner();
	if( observed_ship && ! observed_player )
		observed_player = observed_ship->Owner();
	
	// Move the first person view while jumping in from hyperspace.
	if( observed_ship && (jump_progress < 1.) && (game->View == XWing::View::COCKPIT) )
		Cam.MoveAlong( &(observed_ship->Fwd), observed_ship->CockpitDrawOffset() );
	else if( observed_ship && (jump_progress < 1.) && (game->View == XWing::View::GUNNER) && (observed_ship == observed_turret->ParentShip()) )
		Cam.MoveAlong( &(observed_ship->Fwd), observed_ship->DrawOffset() );
	
	// This allows head tracking to happen after camera placement.
	game->Cam.Copy( &Cam );
	
	
	// When viewing from turret, show its selected target instead of the ship's.
	if( observed_turret && (game->View == XWing::View::GUNNER) )
	{
		target_obj = game->Data.GetObject( observed_turret->Target );
		if( target_obj && (target_obj->Type() == XWing::Object::SHIP) )
		{
			target = (Ship*) target_obj;
			if( target->Health <= 0. )
			{
				if( target->DeathClock.ElapsedSeconds() <= 2. )
					dead_target = target;
				target = NULL;
			}
			else
				dead_target = NULL;
		}
		else
		{
			target = NULL;
			dead_target = NULL;
		}
	}
	
	
	// Set up shaders.
	
	bool use_shaders = game->Cfg.SettingAsBool("g_shader_enable");
	int blastpoints = 0;
	static int prev_blastpoints = 0;
	if( use_shaders )
	{
		game->ShaderMgr.ResumeShaders();
		game->ShaderMgr.Set3f( "CamPos", game->Cam.X, game->Cam.Y, game->Cam.Z );
		SetWorldLights( deathstar );
		ClearDynamicLights();
		blastpoints = game->BlastPoints;
		ClearBlastPoints( std::max<int>( blastpoints, prev_blastpoints ) );
		prev_blastpoints = blastpoints;
		game->ShaderMgr.StopShaders();
	}
	
	
	// Check for enemy weapon locks.
	
	bool incoming = false;
	float enemy_lock = 0.f;
	if( observed_ship && (observed_ship->Health > 0.) )
	{
		// See if a missile/torpedo is already seeking us.
		for( std::list<Shot*>::iterator shot_iter = shots.begin(); shot_iter != shots.end(); shot_iter ++ )
		{
			if( (*shot_iter)->Seeking == observed_ship->ID )
			{
				incoming = true;
				enemy_lock = 2.f;
				break;
			}
		}
		
		if( ! incoming )
		{
			// See if anyone is locking onto us with missiles/torpedos.
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				if( ((*ship_iter)->Health > 0.) && ((*ship_iter)->Target == observed_ship->ID) && ((*ship_iter)->TargetLock > enemy_lock) )
				{
					enemy_lock = (*ship_iter)->TargetLock;
					if( enemy_lock >= 2.f )
						break;
				}
			}
		}
		
		if( (enemy_lock >= 1.f) && (observed_ship->PlayerID == game->PlayerID) )
		{
			static Clock lock_warn;
			if( (lock_warn.Progress() >= 1.) && ! incoming )
				game->Snd.Play( game->Res.GetSound("lock_warn.wav") );
			lock_warn.Reset( 2. );
		}
	}
	
	
	bool shots_checked = false, shot_on_target = false;
	
	
	// Render to textures before drawing anything else.
	
	bool need_target_holo = game->FrameTime || (observed_turret && (game->View == XWing::View::GUNNER));
	if( game->Gfx.Framebuffers && game->FrameTime )
	{
		bool changed_framebuffer = false;
		double display_noise = (observed_ship && (observed_ship->Health < (observed_ship->MaxHealth() * 0.25))) ? game->Cfg.SettingAsDouble("g_display_noise",1.) : 0.;
		
		if( observed_ship && (observed_ship->Health > 0.) && ((game->View == XWing::View::COCKPIT) || (game->View == XWing::View::INSTRUMENTS) || game->Cfg.SettingAsBool("saitek_enable")) )
		{
			Framebuffer *health_framebuffer = game->Res.GetFramebuffer( "health" );
			if( health_framebuffer && health_framebuffer->Select() )
			{
				// Render hull and shield status.
				
				changed_framebuffer = true;
				
				float r = 0.f, g = 0.f, b = 0.f;
				
				if( incoming )
				{
					if( (int) observed_ship->Lifetime.ElapsedMilliseconds() % 200 <= 50 )
						r = 0.5f;
				}
				else if( enemy_lock && ((int) observed_ship->Lifetime.ElapsedMilliseconds() % ((enemy_lock >= 1.f) ? 200 : 600) <= 50) )
					r = g = 0.25f;
				
				game->Gfx.Clear( r, g, b );
				
				
				// Render shield display.
				
				health_framebuffer->Setup2D();
				
				GLuint shield_texture = game->Res.GetAnimation("shield.ani")->CurrentFrame();
				
				double shield_f = observed_ship->ShieldF;
				double shield_r = observed_ship->ShieldR;
				double max_shield = observed_ship->MaxShield();
				if( (jump_progress > 1.) && (jump_progress < 2.) )
				{
					shield_f *= (jump_progress - 1.) * 2.;
					shield_r *= (jump_progress - 1.) * 2.;
				}
				double hit_when = observed_ship->HitClock.ElapsedSeconds();
				float recent_hit = ((hit_when < 0.5) && (hit_when > 0.04)) ? std::min<float>( 1.f, 2.75f * (0.5f - (float)hit_when) ) : 0.f;
				
				if( shield_f > 0. )
				{
					float red = 0.f, green = 1.f, blue = 0.f;
					
					float shield_percent = shield_f / max_shield;
					if( shield_percent > 1.f )
					{
						blue = shield_percent - 1.f;
						green = 1.125f - shield_percent * 0.125f;
					}
					else if( shield_percent >= 0.5f )
						red = 2.f - shield_percent * 2.f;
					else
					{
						red = 1.f;
						green = shield_percent * 2.f;
					}
					
					float recent_hit_f = (observed_ship->HitFlags & Ship::HIT_FRONT) ? recent_hit : 0.f;
					if( recent_hit_f )
					{
						red   = std::max<float>( red,   recent_hit_f );
						green = std::max<float>( green, recent_hit_f );
						blue  = std::max<float>( blue,  recent_hit_f );
					}
					
					float alpha = std::min<float>( shield_f / 10., jump_progress - 1.3 );
					if( alpha < recent_hit_f )
						alpha = recent_hit_f;
					if( alpha < 1.f )
					{
						red   *= alpha;
						green *= alpha;
						blue  *= alpha;
					}
					
					game->Gfx.DrawRect2D( 0, 0, health_framebuffer->W, health_framebuffer->H / 2, shield_texture, red, green, blue, 1.f );
				}
				
				if( shield_r > 0. )
				{
					float red = 0.f, green = 1.f, blue = 0.f;
					
					float shield_percent = shield_r / max_shield;
					if( shield_percent > 1.f )
					{
						blue = shield_percent - 1.f;
						green = 1.125f - shield_percent * 0.125f;
					}
					else if( shield_percent >= 0.5f )
						red = 2.f - shield_percent * 2.f;
					else
					{
						red = 1.f;
						green = shield_percent * 2.f;
					}
					
					float recent_hit_r = (observed_ship->HitFlags & Ship::HIT_REAR) ? recent_hit : 0.f;
					if( recent_hit_r )
					{
						red   = std::max<float>( red,   recent_hit_r );
						green = std::max<float>( green, recent_hit_r );
						blue  = std::max<float>( blue,  recent_hit_r );
					}
					
					float alpha = std::min<float>( shield_r / 10., jump_progress - 1.3 );
					if( alpha < recent_hit_r )
						alpha = recent_hit_r;
					if( alpha < 1.f )
					{
						red   *= alpha;
						green *= alpha;
						blue  *= alpha;
					}
					
					game->Gfx.DrawRect2D( 0, health_framebuffer->H, health_framebuffer->W, health_framebuffer->H / 2, shield_texture, red, green, blue, 1.f );
				}
				
				
				// Render ship to display hull status.
				
				Camera above_cam( game->Cam );
				above_cam.Copy( observed_ship );
				above_cam.Fwd.Copy( &(observed_ship->Up) );
				above_cam.Fwd *= -1.;
				above_cam.Up.Copy( &(observed_ship->Fwd) );
				above_cam.Right.Copy( &(observed_ship->Right) );
				double ship_size = observed_ship->Shape.GetTriagonal();
				above_cam.MoveAlong( &(above_cam.Fwd), -2. * ship_size );
				if( observed_ship->Class && Str::BeginsWith( observed_ship->Class->ShortName, "T/I" ) ) // FIXME: Base this on something like Shape.MaxFwd/MinFwd!
					above_cam.MoveAlong( &(observed_ship->Fwd), ship_size * 0.2 );
				above_cam.FOV = (max_shield > 0.) ? 33. : 20.;
				
				health_framebuffer->Setup3D( &(above_cam) );
				
				float red = 0.f, green = 1.f, blue = 0.f;
				if( observed_ship->Health < observed_ship->MaxHealth() )
					red = 1.f;
				if( observed_ship->Health < (observed_ship->MaxHealth() * 0.5) )
					green = 0.f;
				
				if( recent_hit && (observed_ship->HitFlags & Ship::HIT_HULL) )
				{
					red   = std::max<float>( red,   recent_hit );
					green = std::max<float>( green, recent_hit );
					blue  = std::max<float>( blue,  recent_hit );
				}
				
				glColor4f( red, green, blue, 1.f );
				
				if( use_shaders )
				{
					game->ShaderMgr.Select( game->Res.GetShader("model_hud") );
					game->ShaderMgr.ResumeShaders();
					game->ShaderMgr.Set3f( "AmbientLight", red, green, blue );
				}
				
				observed_ship->Draw();
				
				if( use_shaders )
				{
					game->ShaderMgr.StopShaders();
					game->ShaderMgr.Select( game->Res.GetShader("model") );
				}
				
				glColor4f( 1.f, 1.f, 1.f, 1.f );
				
				
				// Show shield direction.
				
				if( observed_ship->MaxShield() > 0. )
				{
					health_framebuffer->Setup2D();
					
					GLuint pos_texture = 0;
					if( observed_ship->ShieldPos == Ship::SHIELD_FRONT )
						pos_texture = game->Res.GetAnimation("shields_f.ani")->CurrentFrame();
					else if( observed_ship->ShieldPos == Ship::SHIELD_REAR )
						pos_texture = game->Res.GetAnimation("shields_r.ani")->CurrentFrame();
					else
						pos_texture = game->Res.GetAnimation("shields_c.ani")->CurrentFrame();
					
					game->Gfx.DrawRect2D( health_framebuffer->W - 42, 10, health_framebuffer->W - 10, 74, pos_texture );
				}
				
				
				// Fade screens in when jumping in from hyperspace.
				
				if( (jump_progress < 1.3) || in_hyperspace )
				{
					health_framebuffer->Setup2D( 0., 0., 1., 1. );
					game->Gfx.DrawRect2D( 0., 0., 1., 1., 0, 0.f,0.f,0.f,1.f );
				}
				
				
				// Draw noise if we're damaged.
				
				if( display_noise )
				{
					health_framebuffer->Setup2D();
					
					glPointSize( 1.f );
					
					glBegin( GL_POINTS );
						
						float brightness = 1.;
						int dots = (1. - observed_ship->Health / observed_ship->MaxHealth()) * display_noise * health_framebuffer->W * health_framebuffer->H / 66.6;
						for( int i = 0; i < dots; i ++ )
						{
							brightness = Rand::Double( 0.5, 1. );
							glColor4f( brightness, brightness, brightness, 1.f );
							glVertex2d( Rand::Int( 0, health_framebuffer->W ), Rand::Int( 0, health_framebuffer->H ) );
						}
						
					glEnd();
				}
			}
			
			
			Framebuffer *target_framebuffer = game->Res.GetFramebuffer( "target" );
			if( target_framebuffer && target_framebuffer->Select() )
			{
				// Render target display.
				
				changed_framebuffer = true;
				if( ((game->View != XWing::View::GUNNER) || ! observed_turret) && (observed_ship->Category() != ShipClass::CATEGORY_CAPITAL) && (observed_ship->Category() != ShipClass::CATEGORY_TARGET) )
					need_target_holo = false;
				game->Gfx.Clear();
				
				double eject_held = game->Input.ControlPressed( XWing::Control::EJECT ) ? game->EjectHeld.ElapsedSeconds() : 0.;
				if( eject_held >= 0.5 )
				{
					double unused = 0.;
					if( modf( eject_held * 6., &unused ) >= 0.5 )
						game->Gfx.Clear( 1.f, 0.f, 0.f );
					
					target_framebuffer->Setup2D();
					ScreenFont->DrawText( "EJECTING", target_framebuffer->W / 2, target_framebuffer->H / 2, Font::ALIGN_MIDDLE_CENTER, 1.f, 1.f, 1.f, 1.f );
				}
				else if( target_obj )
				{
					// Draw target.
					
					double cam_dist = 75.;
					if( target )
						cam_dist = 2. * target->Shape.GetTriagonal();
					
					// FIXME: Some edge cases look wrong, such as when the target is a Star Destroyer and the observed ship is behind its origin facing the tower.
					Vec3D vec_to_target( target_obj->X - observed_ship->X, target_obj->Y - observed_ship->Y, target_obj->Z - observed_ship->Z );
					Camera cam_to_target( game->Cam );
					cam_to_target.Offset.SetPos( 0., 0., 0. );
					cam_to_target.Fwd.Copy( &vec_to_target );
					cam_to_target.Fwd.ScaleTo( 1. );
					cam_to_target.Up.Copy( &(observed_ship->Up) );
					cam_to_target.FixVectors();
					cam_to_target.SetPos( target_obj->X, target_obj->Y, target_obj->Z );
					cam_to_target.MoveAlong( &(cam_to_target.Fwd), -cam_dist );
					cam_to_target.FOV = 30.;
					
					target_framebuffer->Setup3D( &(cam_to_target) );
					
					if( target_obj->Type() == XWing::Object::SHIP )
					{
						Ship *target_ship = (Ship*) target_obj;
						
						if( use_shaders )
						{
							game->ShaderMgr.Select( game->Res.GetShader("model_hud") );
							game->ShaderMgr.ResumeShaders();
							game->ShaderMgr.Set3f( "AmbientLight", 0., 0., 0. );
						}
						
						target_ship->Draw();
						
						if( use_shaders )
						{
							game->ShaderMgr.StopShaders();
							game->ShaderMgr.Select( game->Res.GetShader("model") );
						}
						
						glLineWidth( 2.f );
						target_ship->DrawWireframe();
						
						bool disabled_depth = false;
						
						if( observed_ship->TargetSubsystem )
						{
							glDisable( GL_DEPTH_TEST );
							disabled_depth = true;
							
							std::string subsystem_name = target_ship->SubsystemName( observed_ship->TargetSubsystem );
							if( target_ship->Shape.Objects.find( subsystem_name ) != target_ship->Shape.Objects.end() )
							{
								// Draw subsystem wireframe.
								std::set<std::string> objects;
								objects.insert( subsystem_name );
								Color wireframe_color( 1.f, 1.f, 1.f, 1.f );
								target_ship->Shape.Draw( target_ship, &objects, &wireframe_color );
							}
							else
							{
								// Subsystem has no corresponding object in client-side model, so just show the center point.
								glColor4f( 1.f, 1.f, 1.f, 1.f );
								Pos3D pos = target_ship->TargetCenter( observed_ship->TargetSubsystem );
								glPointSize( 7.f );
								glBegin( GL_POINTS );
									glVertex3d( pos.X, pos.Y, pos.Z );
								glEnd();
							}
						}
						
						// Draw hit/explosion effect when target is recently damaged.
						bool target_alive = (target_ship->Health > 0.);
						double hit_time = target_ship->HitClock.ElapsedSeconds();
						if( hit_time < (target_alive ? 0.125 : 0.5) )
						{
							if( ! disabled_depth )
							{
								glDisable( GL_DEPTH_TEST );
								disabled_depth = true;
							}
							
							Pos3D pos( target_ship );
							if( target_alive && target_ship->RecentBlastPoint )
							{
								pos.MoveAlong( &(pos.Fwd),   target_ship->RecentBlastPoint->X );
								pos.MoveAlong( &(pos.Up),    target_ship->RecentBlastPoint->Y );
								pos.MoveAlong( &(pos.Right), target_ship->RecentBlastPoint->Z );
							}
							
							float alpha = 1. - (target_alive ? (hit_time * 3.) : hit_time);
							game->Gfx.DrawSphere3D( pos.X, pos.Y, pos.Z, sqrt(target_ship->Radius()) * hit_time * (target_alive ? 10. : 20.), 8, 0, alpha,alpha,alpha,1.f );
						}
						
						if( disabled_depth )
							glEnable( GL_DEPTH_TEST );
					}
					else if( target_obj->Type() == XWing::Object::TURRET )
					{
						Turret *target_turret = (Turret*) target_obj;
						
						if( target_turret->GunShape )
							cam_to_target.MoveAlong( &(cam_to_target.Fwd), 75. - target_turret->GunShape->GetTriagonal() * 0.044 );
						
						if( use_shaders )
						{
							game->ShaderMgr.Select( game->Res.GetShader("model_hud") );
							game->ShaderMgr.ResumeShaders();
							game->ShaderMgr.Set3f( "AmbientLight", 0., 0., 0. );
						}
						
						target_turret->Draw( false );  // Prevent changing to deathstar shader on Battle of Yavin.
						
						if( use_shaders )
						{
							game->ShaderMgr.StopShaders();
							game->ShaderMgr.Select( game->Res.GetShader("model") );
						}
						
						glLineWidth( 2.f );
						target_turret->DrawWireframe();
					}
					else if( target_obj->Type() == XWing::Object::CHECKPOINT )
					{
						// Draw an arrow pointing toward the checkpoint.
						Vec3D vtt = vec_to_target.Unit();
						Vec3D fur( vtt.Dot(&(observed_ship->Fwd)), vtt.Dot(&(observed_ship->Up)), vtt.Dot(&(observed_ship->Right)) );
						Vec3D front = cam_to_target.Fwd * fur.X + cam_to_target.Up * fur.Y + cam_to_target.Right * fur.Z;
						Vec3D out = front;
						out.RotateAround( &(cam_to_target.Fwd), 180. );
						out = front.Cross( out ).Unit();
						float r = 0.5f, g = 0.5f, b = 1.f;
						if( fur.X < 0. )
						{
							r = 1.f;
							g = b = 0.f;
						}
						game->Gfx.DrawLine3D(
							target_obj->X + front.X * 15., target_obj->Y + front.Y * 15., target_obj->Z + front.Z * 15.,
							target_obj->X - front.X * 10., target_obj->Y - front.Y * 10., target_obj->Z - front.Z * 10.,
							7.f, r,g,b,1.f );
						game->Gfx.DrawLine3D(
							target_obj->X + front.X * 15.,              target_obj->Y + front.Y * 15.,              target_obj->Z + front.Z * 15.,
							target_obj->X + front.X * 5. + out.X * 10., target_obj->Y + front.Y * 5. + out.Y * 10., target_obj->Z + front.Z * 5. + out.Z * 10.,
							5.f, r,g,b,1.f );
						game->Gfx.DrawLine3D(
							target_obj->X + front.X * 15.,              target_obj->Y + front.Y * 15.,              target_obj->Z + front.Z * 15.,
							target_obj->X + front.X * 5. - out.X * 10., target_obj->Y + front.Y * 5. - out.Y * 10., target_obj->Z + front.Z * 5. - out.Z * 10.,
							5.f, r,g,b,1.f );
					}
					else
					{
						Camera original_cam( game->Cam );
						game->Cam = cam_to_target;  // Shot::Draw looks at Raptor::Game->Cam to calculate draw vectors.
						target_obj->Draw();
						game->Cam = original_cam;
					}
					
					
					target_framebuffer->Setup2D();
					
					uint32_t target_team = XWing::Team::NONE;
					std::string target_name = "";
					std::string target_status = "";
					bool target_seeking_us = false;
					bool classic = Raptor::Game->Cfg.SettingAsBool("ui_classic");
					
					if( target )
					{
						target_name = target->Name;
						target_team = target->Team;
						Player *target_player = target->Owner();
						if( target_player )
							target_name = target_player->Name;
						
						if( target->Class && (target->Class->Category != ShipClass::CATEGORY_TARGET) )
						{
							size_t max_name_length = std::max<int>( 1, 15 - target->Class->ShortName.length() );
							target_name = target->Class->ShortName + ((target_name.length() == max_name_length) ? std::string(":") : std::string(": ")) + ((target_name.length() > max_name_length) ? target_name.substr(0,max_name_length) : target_name);
						}
						
						if( ! classic )
						{
							if( target->Category() == ShipClass::CATEGORY_TARGET )
							{
								if( target->Health < (target->MaxHealth() * 0.7) )
									target_status = "SURFACE\nIMPACT";
							}
							else
							{
								char shield_str[ 5 ] = "NONE", hull_str[ 5 ] = "100%";
								double max_shield = target->MaxShield();
								if( max_shield )
								{
									double shield = std::min<double>( target->ShieldF, target->ShieldR );
									if( target->ShieldPos == Ship::SHIELD_FRONT )
										shield = target->ShieldF * 0.5;
									else if( target->ShieldPos == Ship::SHIELD_REAR )
										shield = target->ShieldR * 0.5;
									if( shield > 0. )
										snprintf( shield_str, sizeof(shield_str), "%3.0f%%", ceil( 100. * shield / max_shield ) );
									else
										snprintf( shield_str, sizeof(shield_str), "DOWN" );
								}
								snprintf( hull_str, sizeof(hull_str), "%3.0f%%", std::min<double>( 100., ceil(100. * target->Health / target->MaxHealth()) ) );
								target_status = std::string("SHLD      HULL\n") + std::string(shield_str) + std::string("      ") + std::string(hull_str);
							}
						}
						else if( target->Health < (target->MaxHealth() * 0.7) )
							target_status = "Hull Damaged";
						else if( (target->MaxShield() > 0.)
						&&  (   ((target->ShieldF <= 0.) && (target->ShieldPos != Ship::SHIELD_REAR))
						     || ((target->ShieldR <= 0.) && (target->ShieldPos != Ship::SHIELD_FRONT)) ) )
							target_status = "Shields Down";
						else
							target_status = "OK";
					}
					else
					{
						if( target_obj->Type() == XWing::Object::TURRET )
						{
							target_name = "Turbolaser";
							target_team = ((const Turret*)( target_obj ))->Team;
						}
						else if( target_obj->Type() == XWing::Object::SHOT )
						{
							const Shot *target_shot = (const Shot*) target_obj;
							if( target_shot->ShotType == Shot::TYPE_TORPEDO )
								target_name = "Torpedo";
							else if( target_shot->ShotType == Shot::TYPE_MISSILE )
								target_name = "Missile";
							
							const GameObject *from_obj = game->Data.GetObject( target_shot->FiredFrom );
							if( from_obj->Type() == XWing::Object::SHIP )
								target_team = ((const Ship*)( from_obj ))->Team;
							else if( from_obj->Type() == XWing::Object::TURRET )
								target_team = ((const Turret*)( from_obj ))->Team;
							
							if( target_shot->Seeking )
							{
								const GameObject *seeking_obj = game->Data.GetObject( target_shot->Seeking );
								if( seeking_obj && (seeking_obj->PlayerID == game->PlayerID) && (seeking_obj->Type() != XWing::Object::SHOT) )
									target_seeking_us = true;
							}
						}
						else if( target_obj->Type() == XWing::Object::CHECKPOINT )
							target_name = "Race Checkpoint";
					}
					
					float red = 1.f, green = 0.f, blue = 0.f;
					if( target_seeking_us && (fmod( target_obj->Lifetime.ElapsedSeconds(), 0.5 ) < 0.25) )
						green = 1.f;
					else if( target_obj->Type() == XWing::Object::CHECKPOINT )
					{
						red = 0.f;
						green = 1.f;
						blue = 1.f;
					}
					else if( observed_ship->Team && (target_team == observed_ship->Team) )
					{
						red = 0.f;
						green = 1.f;
					}
					
					// Draw target name.
					ScreenFont->DrawText( target_name, target_framebuffer->W / 2, 10, Font::ALIGN_TOP_CENTER, red, green, blue, 1.f );
					
					// Draw target status (health).
					ScreenFont->DrawText( target_status, target_framebuffer->W / 2, ScreenFont->PointSize + (classic ? 10 : 15), Font::ALIGN_TOP_CENTER, 1.f, 1.f, 0.5f, 1.f );
					
					int dist = vec_to_target.Length();
					if( target && observed_ship->TargetSubsystem )
					{
						dist = target->TargetCenter( observed_ship->TargetSubsystem ).Dist( observed_ship );
						
						// Draw selected subsystem.
						if( observed_ship && observed_ship->TargetSubsystem )
						{
							std::string subsystem_id = target->SubsystemName( observed_ship->TargetSubsystem );
							std::string subsystem_name = subsystem_id;
							
							if( ! game->Cfg.SettingAsBool("ugly_names",false) )
							{
								// Clean up name.
								const char *subsystem_name_cstr = subsystem_name.c_str();
								if( strncmp( subsystem_name_cstr, "ShieldGen", strlen("ShieldGen") ) == 0 )
									subsystem_name = classic ? "Shield Generator" : "SHIELD GEN";
								else
								{
									// Remove "Critical" prefix.
									if( strncmp( subsystem_name_cstr, "Critical", strlen("Critical") ) == 0 )
										subsystem_name = subsystem_name.substr( strlen("Critical") );
									
									// Remove trailing digits.
									while( subsystem_name.length() )
									{
										char last_char = *(subsystem_name.rbegin());
										if( (last_char >= '0') && (last_char <= '9') )
											subsystem_name.erase( subsystem_name.end() - 1 );
										else
											break;
									}
								}
							}
							
							if( ! classic )
							{
								// Display subsystem health.
								
								subsystem_name = Str::CapitalizedCopy(subsystem_name);
								if( subsystem_name.length() > strlen("SHIELD GEN ") )
									subsystem_name += std::string("\n");
								else while( subsystem_name.length() < strlen("SHIELD GEN ") )
									subsystem_name += std::string(" ");
								
								double max_health = target->SubsystemMaxHealth( observed_ship->TargetSubsystem );
								if( max_health )
								{
									char health_str[ 5 ] = "100%";
									double health = target->Subsystems[ subsystem_id ];
									if( health > 0. )
									{
										snprintf( health_str, sizeof(health_str), "%3.0f%%", ceil( 100. * health / max_health ) );
										subsystem_name += std::string(health_str);
									}
								}
							}
							
							ScreenFont->DrawText( subsystem_name, target_framebuffer->W / 2, target_framebuffer->H - ScreenFont->PointSize - (classic ? 10 : 25), Font::ALIGN_BOTTOM_CENTER, 0.9f, 0.9f, 0.9f, 1.f );
						}
					}
					
					// Draw target distance.
					if( classic )
					{
						ScreenFont->DrawText( "Dist:", 10, target_framebuffer->H - 10, Font::ALIGN_BOTTOM_LEFT, 0.5f, 0.5f, 1.f, 1.f );
						ScreenFont->DrawText( Num::ToString(dist), target_framebuffer->W - 10, target_framebuffer->H - 10, Font::ALIGN_BOTTOM_RIGHT, 0.5f, 0.5f, 1.f, 1.f );
					}
					else
					{
						char dist_buf[ 32 ];
						if( dist < 1000 )
							snprintf( dist_buf, sizeof(dist_buf),  "DIST:%5i",    dist );
						else
							snprintf( dist_buf, sizeof(dist_buf), " DIST:%5.1fK", dist / 1000. );
						ScreenFont->DrawText( dist_buf, target_framebuffer->W / 2, target_framebuffer->H - 20, Font::ALIGN_BOTTOM_CENTER, 0.5f, 0.5f, 1.f, 1.f );
					}
				}
				
				
				// Fade screens in when jumping in from hyperspace.
				
				if( (jump_progress < 1.5) || in_hyperspace )
				{
					target_framebuffer->Setup2D( 0., 0., 1., 1. );
					game->Gfx.DrawRect2D( 0., 0., 1., 1., 0, 0.f,0.f,0.f,1.f );
				}
				
				
				// Draw noise if we're damaged.
				
				if( display_noise )
				{
					target_framebuffer->Setup2D();
					
					glPointSize( 1.f );
					
					glBegin( GL_POINTS );
						
						float brightness = 1.;
						int dots = (1. - observed_ship->Health / observed_ship->MaxHealth()) * display_noise * target_framebuffer->W * target_framebuffer->H / 66.6;
						for( int i = 0; i < dots; i ++ )
						{
							brightness = Rand::Double( 0.5, 1. );
							glColor4f( brightness, brightness, brightness, 1.f );
							glVertex2d( Rand::Int( 0, target_framebuffer->W ), Rand::Int( 0, target_framebuffer->H ) );
						}
						
					glEnd();
				}
			}
			
			
			Framebuffer *intercept_framebuffer = game->Res.GetFramebuffer( "intercept" );
			if( intercept_framebuffer && intercept_framebuffer->Select() )
			{
				// Render intercept display.
				
				changed_framebuffer = true;
				bool lock_display = (observed_ship->SelectedWeapon == Shot::TYPE_TORPEDO) || (observed_ship->SelectedWeapon == Shot::TYPE_MISSILE);
				
				if( lock_display )
				{
					// Draw torpedo lock-on display.
					
					game->Gfx.Clear( 0.f, 0.0f, 0.f );
					intercept_framebuffer->Setup2D();
					
					float lock_wait = std::max<float>( 0.f, 1.f - observed_ship->TargetLock );
					bool lock_blink = ((lock_wait == 0.f) && observed_ship->LockingOn(target_obj)) ? !( ((int)( observed_ship->TargetLock * 100 )) % 2 ) : false;
					int cx = intercept_framebuffer->W / 2;
					int cy = intercept_framebuffer->H / 2 - 25;
					
					if( (lock_wait < 0.9f) && ! lock_blink )
					{
						// Draw red targeting lines.
						
						double lock_x = (intercept_framebuffer->W / 2) * lock_wait;
						glColor4f( 1.f, 0.f, 0.f, 1.f );
						glLineWidth( 4.f );
						
						glBegin( GL_LINES );
							glVertex2d( intercept_framebuffer->W / 2 - lock_x, 10 );
							glVertex2d( intercept_framebuffer->W / 2 - lock_x, intercept_framebuffer->H - 60 );
							glVertex2d( intercept_framebuffer->W / 2 + lock_x, 10 );
							glVertex2d( intercept_framebuffer->W / 2 + lock_x, intercept_framebuffer->H - 60 );
						glEnd();
						
						if( lock_wait == 0.f )
						{
							glBegin( GL_LINES );
								glVertex2d( 10, (intercept_framebuffer->H - 50) / 2 );
								glVertex2d( intercept_framebuffer->W - 10, (intercept_framebuffer->H - 50) / 2 );
							glEnd();
						}
					}
					
					// Draw targeting box outlines.
					
					glColor4f( 1.f, 1.f, 0.f, 1.f );
					
					glLineWidth( 3.f );
					
					glBegin( GL_LINE_STRIP );
						glVertex2d( 10, 10 );
						glVertex2d( 10, intercept_framebuffer->H - 60 );
						glVertex2d( intercept_framebuffer->W - 10, intercept_framebuffer->H - 60 );
						glVertex2d( intercept_framebuffer->W - 10, 10 );
						glVertex2d( 10, 10 );
					glEnd();
					
					glBegin( GL_LINE_STRIP );
						glVertex2d( intercept_framebuffer->W / 2 - 80, intercept_framebuffer->H );
						glVertex2d( intercept_framebuffer->W / 2 - 80, intercept_framebuffer->H - 55 );
						glVertex2d( intercept_framebuffer->W / 2 + 80, intercept_framebuffer->H - 55 );
						glVertex2d( intercept_framebuffer->W / 2 + 80, intercept_framebuffer->H );
					glEnd();
					
					if( lock_blink )
					{
						// Flash red chevrons when locked on.
						
						glColor4f( 1.f, 0.f, 0.f, 1.f );
						
						glBegin( GL_TRIANGLE_FAN );
							glVertex2d( cx + 10, cy + 10 );
							glVertex2d( cx + 30, cy + 50 );
							glVertex2d( cx + 30, cy + 30 );
							glVertex2d( cx + 50, cy + 30 );
						glEnd();
						
						glBegin( GL_TRIANGLE_FAN );
							glVertex2d( cx - 10, cy + 10 );
							glVertex2d( cx - 30, cy + 50 );
							glVertex2d( cx - 30, cy + 30 );
							glVertex2d( cx - 50, cy + 30 );
						glEnd();
						
						glBegin( GL_TRIANGLE_FAN );
							glVertex2d( cx + 10, cy - 10 );
							glVertex2d( cx + 30, cy - 50 );
							glVertex2d( cx + 30, cy - 30 );
							glVertex2d( cx + 50, cy - 30 );
						glEnd();
						
						glBegin( GL_TRIANGLE_FAN );
							glVertex2d( cx - 10, cy - 10 );
							glVertex2d( cx - 30, cy - 50 );
							glVertex2d( cx - 30, cy - 30 );
							glVertex2d( cx - 50, cy - 30 );
						glEnd();
					}
					else if( target && (target->Category() == ShipClass::CATEGORY_TARGET) && (lock_wait > 0.f) )
					{
						// Draw Death Star trench grid lines.
						
						glLineWidth( 2.f );
						
						glBegin( GL_LINES );
							
							glVertex2d( cx, cy );
							glVertex2d( cx - (intercept_framebuffer->H / 2 - 35) / 1.25, 10 );
							glVertex2d( cx, cy );
							glVertex2d( cx + (intercept_framebuffer->H / 2 - 35) / 1.25, 10 );
							glVertex2d( cx, cy );
							glVertex2d( cx - (intercept_framebuffer->H / 2 - 35) / 1.25, intercept_framebuffer->H - 60 );
							glVertex2d( cx, cy );
							glVertex2d( cx + (intercept_framebuffer->H / 2 - 35) / 1.25, intercept_framebuffer->H - 60 );
							
							glVertex2d( cx, cy );
							glVertex2d( 30, 10 );
							glVertex2d( cx, cy );
							glVertex2d( 30, intercept_framebuffer->H - 60 );
							glVertex2d( cx, cy );
							glVertex2d( intercept_framebuffer->W - 30, 10 );
							glVertex2d( cx, cy );
							glVertex2d( intercept_framebuffer->W - 30, intercept_framebuffer->H - 60 );
							
							glVertex2d( cx, cy );
							glVertex2d( 10, 10 + (intercept_framebuffer->H - 70) * 0.31 );
							glVertex2d( cx, cy );
							glVertex2d( 10, 10 + (intercept_framebuffer->H - 70) * 0.69 );
							glVertex2d( cx, cy );
							glVertex2d( intercept_framebuffer->W - 10, 10 + (intercept_framebuffer->H - 70) * 0.31 );
							glVertex2d( cx, cy );
							glVertex2d( intercept_framebuffer->W - 10, 10 + (intercept_framebuffer->H - 70) * 0.69 );
							
						glEnd();
						
						double dist = observed_ship->Dist( target_obj );
						for( int i = 0; i < 400; i += 100 )
						{
							double x = (intercept_framebuffer->W / 2 - 10) * pow( 1. - fmod( dist + i, 400. ) / 400., 2.0 );
							double y = std::min<double>( x * 1.25, intercept_framebuffer->H / 2 - 35 );
							
							glBegin( GL_LINE_STRIP );
								glVertex2d( cx - x, cy - y );
								glVertex2d( cx - x, cy + y );
								glVertex2d( cx + x, cy + y );
								glVertex2d( cx + x, cy - y );
							glEnd();
						}
					}
					else if( target && (target->Health > 0.) && (lock_wait > 0.f) )
					{
						// Draw dot for any other torpedo target.
						
						Vec3D vec_to_target( target_obj->X - observed_ship->X, target_obj->Y - observed_ship->Y, target_obj->Z - observed_ship->Z );
						if( observed_ship->TargetSubsystem )
							vec_to_target = target->TargetCenter( observed_ship->TargetSubsystem ) - observed_ship;
						
						if( vec_to_target.Dot( &(observed_ship->Fwd) ) > 0. )
						{
							vec_to_target.ScaleTo( 1. );
							double x = cx + vec_to_target.Dot( &(observed_ship->Right) ) * intercept_framebuffer->W * 0.75;
							double y = cy - vec_to_target.Dot( &(observed_ship->Up) ) * intercept_framebuffer->W * 0.75;
							if( (x > 15) && (x < intercept_framebuffer->W - 15) && (y > 15) && (y < intercept_framebuffer->H - 65) )
								game->Gfx.DrawCircle2D( x, y, 10., 6, 0, 1.f, 1.f, 0.f, 1.f );
						}
					}
					
					char lock_num[ 7 ] = "------";
					if( lock_wait < 1.f )
						snprintf( lock_num, 7, "%06.0f", 999999.4f * lock_wait * lock_wait * lock_wait * lock_wait );
					ScreenFont->DrawText( lock_num, intercept_framebuffer->W / 2, intercept_framebuffer->H, Font::ALIGN_BOTTOM_CENTER, 1.f, 0.f, 0.f, 1.f );
				}
				else
				{
					// Draw laser intercept display.
					
					if( observed_ship->SelectedWeapon == Shot::TYPE_ION_CANNON )
						game->Gfx.Clear( 0.f, 0.f, 0.5f );
					else
						game->Gfx.Clear( 0.f, 0.25f, 0.f );
					
					
					// See if the crosshair is lined up so the next shot would hit.
					
					Vec3D shot_vec;
					
					if( target || (target_obj && (target_obj->Type() == XWing::Object::SHOT)) )
					{
						std::vector<Shot*> test_shots = observed_ship->NextShots();
						for( std::vector<Shot*>::iterator shot_iter = test_shots.begin(); shot_iter != test_shots.end(); shot_iter ++ )
						{
							shot_vec.Copy( &((*shot_iter)->MotionVector) );
							if( (! shot_on_target) && target_obj->WillCollide( *shot_iter, (*shot_iter)->MaxLifetime() ) )
								shot_on_target = true;
							
							delete *shot_iter;
						}
					}
					
					shots_checked = true;
					
					
					glLineWidth( 2.f );
					
					
					// Draw the pointy bits.
					
					intercept_framebuffer->Setup2D( -1., -1., 1., 1. );
					
					float r = 0.f, g = 1.f, b = 0.f;
					
					if( shot_on_target )
					{
						r = 1.f;
						b = 1.f;
					}
					else if( observed_ship->SelectedWeapon == Shot::TYPE_ION_CANNON )
					{
						g = 0.75f;
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
						glVertex2d( -2., -1.2 );
						glVertex2d( -0.25, 0. );
						glVertex2d( -0.25, 0. );
						glVertex2d( -2., 1.2 );
						
						// Right
						glVertex2d( 2., -1.2 );
						glVertex2d( 0.25, 0. );
						glVertex2d( 0.25, 0. );
						glVertex2d( 2., 1.2 );
						
						// Top-Left
						glVertex2d( -0.07, 0.33 );
						glVertex2d( -0.22, 0.22 );
						glVertex2d( -0.22, 0.22 );
						glVertex2d( -0.33, 0.07 );
						
						// Top-Right
						glVertex2d( 0.07, 0.33 );
						glVertex2d( 0.22, 0.22 );
						glVertex2d( 0.22, 0.22 );
						glVertex2d( 0.33, 0.07 );
						
						// Bottom-Right
						glVertex2d( 0.07, -0.33 );
						glVertex2d( 0.22, -0.22 );
						glVertex2d( 0.22, -0.22 );
						glVertex2d( 0.33, -0.07 );
						
						// Bottom-Left
						glVertex2d( -0.07, -0.33 );
						glVertex2d( -0.22, -0.22 );
						glVertex2d( -0.22, -0.22 );
						glVertex2d( -0.33, -0.07 );
						
					glEnd();
					
					
					// Draw the target dots.
					
					if( (target && (target->Health > 0.f)) || (target_obj && (target_obj->Type() == XWing::Object::SHOT)) )
					{
						shot_vec -= target_obj->MotionVector;
						
						Vec3D vec_to_target( target_obj->X - observed_ship->X, target_obj->Y - observed_ship->Y, target_obj->Z - observed_ship->Z );
						if( target && observed_ship->TargetSubsystem )
							vec_to_target = target->TargetCenter( observed_ship->TargetSubsystem ) - observed_ship;
						
						double time_to_target = vec_to_target.Length() / shot_vec.Length();
						
						Vec3D vec_to_intercept = vec_to_target;
						vec_to_intercept.X += target_obj->MotionVector.X * time_to_target;
						vec_to_intercept.Y += target_obj->MotionVector.Y * time_to_target;
						vec_to_intercept.Z += target_obj->MotionVector.Z * time_to_target;
						
						vec_to_target.ScaleTo( 1. );
						vec_to_intercept.ScaleTo( 1. );
						
						bool draw_target = (vec_to_target.Dot( &(observed_ship->Fwd) ) > 0.);
						bool draw_intercept = (vec_to_intercept.Dot( &(observed_ship->Fwd) ) > 0.);
						
						double tx = vec_to_target.Dot( &(observed_ship->Right) ) * 7.;
						double ty = vec_to_target.Dot( &(observed_ship->Up) ) * -7.;
						double ix = vec_to_intercept.Dot( &(observed_ship->Right) ) * 7.;
						double iy = vec_to_intercept.Dot( &(observed_ship->Up) ) * -7.;
						
						if( draw_target || draw_intercept )
						{
							intercept_framebuffer->Setup2D( -1., 1. );
							
							glLineWidth( 1.f );
							
							if( observed_ship->SelectedWeapon == Shot::TYPE_ION_CANNON )
								glColor4f( 0.f, 0.25f, 1.f, 1.f );
							else
								glColor4f( 0.f, 0.5f, 0.f, 1.f );
							
							glBegin( GL_LINES );
								glVertex2d( tx, ty );
								glVertex2d( ix, iy );
							glEnd();
						}
						
						if( draw_target )
						{
							if( observed_ship->SelectedWeapon == Shot::TYPE_ION_CANNON )
								game->Gfx.DrawCircle2D( tx, ty, 0.07, 6, 0, 0.f, 0.25f, 1.f, 1.f );
							else
								game->Gfx.DrawCircle2D( tx, ty, 0.07, 6, 0, 0.f, 0.5f, 0.f, 1.f );
						}
						
						if( draw_intercept )
							game->Gfx.DrawCircle2D( ix, iy, 0.1, 4, 0, r, g, b, 1.f );
					}
				}
				
				
				// Fade screens in when jumping in from hyperspace.
				
				if( (jump_progress < 1.4) || in_hyperspace )
				{
					intercept_framebuffer->Setup2D( 0., 0., 1., 1. );
					game->Gfx.DrawRect2D( 0., 0., 1., 1., 0, 0.f,0.f,0.f,1.f );
				}
				
				
				// Draw noise if we're damaged.
				
				if( display_noise )
				{
					intercept_framebuffer->Setup2D();
					
					glPointSize( 1.f );
					
					glBegin( GL_POINTS );
						
						int dots = (1. - observed_ship->Health / observed_ship->MaxHealth()) * display_noise * intercept_framebuffer->W * intercept_framebuffer->H / 66.6;
						for( int i = 0; i < dots; i ++ )
						{
							float brightness = Rand::Double( 0.5, 1. );
							float r = brightness, g = brightness, b = brightness;
							if( observed_ship->SelectedWeapon == Shot::TYPE_ION_CANNON )
								r = g = brightness - 0.5f;
							else if( ! lock_display )
								r = b = brightness - 0.5f;
							glColor4f( r, g, b, 1.f );
							glVertex2d( Rand::Int( 0, intercept_framebuffer->W ), Rand::Int( 0, intercept_framebuffer->H ) );
						}
						
					glEnd();
				}
			}
			
			
			Framebuffer *throttle_framebuffer = game->Res.GetFramebuffer( "throttle" );
			if( throttle_framebuffer && throttle_framebuffer->Select() )
			{
				// Render throttle display.
				
				changed_framebuffer = true;
				game->Gfx.Clear( 0.f, 0.f, 0.f );
				
				
				throttle_framebuffer->Setup2D( 0., 0., 1., 1. );
				
				double throttle = observed_ship->GetThrottle();
				game->Gfx.DrawRect2D( 0., 1. - throttle, 1., 1., 0, 0.125f * (float)throttle, 0.25f + 0.25f * (float)throttle, 1.f, 1.f );
				
				
				// Fade screens in when jumping in from hyperspace.
				
				if( (jump_progress < 1.15) || in_hyperspace )
					game->Gfx.DrawRect2D( 0., 0., 1., 1., 0, 0.f,0.f,0.f,1.f );
			}
			
			
			// NOTE: Add any new displays here.
		}
		
		// Return to default framebuffer.
		if( changed_framebuffer )
			game->Gfx.SelectDefaultFramebuffer();
	}
	
	
	// Update Saitek LEDs and MFDs based on player state.
	
	#ifdef WIN32
	if( game->FrameTime )
		UpdateSaitek( observed_ship, (observed_ship == player_ship) );
	#endif
	
	
	// Special case "spectator_view instruments" to draw instrument panel on a second PC.
	
	if( game->View == XWing::View::INSTRUMENTS )
	{
		glColor4f( 1.f, 1.f, 1.f, 1.f );
		game->Gfx.Clear();
		
		if( observed_ship && (observed_ship->Health > 0.) )
		{
			if( game->Gfx.AspectRatio < 1.5 )
			{
				game->Gfx.Setup2D( 0., 1. );
				game->Gfx.DrawRect2D( -0.5, 0.,   1.5,  1.,   game->Res.GetAnimation("brushed.ani")->CurrentFrame() );
				game->Gfx.DrawRect2D( 0.1,  0.65, 0.9,  0.05, game->Res.GetTexture("*intercept") );
				game->Gfx.DrawRect2D( 0.1,  0.99, 0.35, 0.66, game->Res.GetTexture("*health") );
				game->Gfx.DrawRect2D( 0.65, 0.99, 0.9,  0.66, game->Res.GetTexture("*target") );
				game->Gfx.DrawRect2D( 0.45, 0.95, 0.55, 0.7,  game->Res.GetTexture("*throttle") );
			}
			else
			{
				game->Gfx.Setup2D( -1., 1. );
				game->Gfx.DrawRect2D( -2., -1., 2., 1., game->Res.GetAnimation("brushed.ani")->CurrentFrame() );
				double side_w = std::max<double>( 0.72, std::min<double>( 1.5, game->Gfx.AspectRatio - 1.3 ) );
				double side_h = side_w * 4. / 3.;
				double mid_w = std::min<double>( 2., 2. * (game->Gfx.AspectRatio - side_w) - 0.05 );
				double mid_h = mid_w * 0.75;
				game->Gfx.DrawRect2D( mid_w * -0.5, mid_h - 0.9, mid_w * 0.5, -0.9, game->Res.GetTexture("*intercept") );
				game->Gfx.DrawRect2D( 0.02 - game->Gfx.AspectRatio, 0.98, side_w + 0.02 - game->Gfx.AspectRatio, 0.98 - side_h, game->Res.GetTexture("*health") );
				game->Gfx.DrawRect2D( game->Gfx.AspectRatio - side_w - 0.02, 0.98, game->Gfx.AspectRatio - 0.02, 0.98 - side_h, game->Res.GetTexture("*target") );
				game->Gfx.DrawRect2D( -0.1, 0.95, 0.1, mid_h - 0.85, game->Res.GetTexture("*throttle") );
			}
		}
		
		if( (observed_player || observed_ship) && ! MessageInput->Visible )
		{
			game->Gfx.Setup2D();
			int x = Rect.x + Rect.w/2;
			int y = Rect.y + 2;
			Font *font = (Rect.h >= 600) ? BigFont : SmallFont;
			if( observed_ship && (observed_ship->Health > 0.) )
			{
				font->DrawText( observed_player ? observed_player->Name : observed_ship->Name, x    , y - 1, Font::ALIGN_TOP_CENTER, 0.0f,0.0f,0.0f,0.7f );
				font->DrawText( observed_player ? observed_player->Name : observed_ship->Name, x - 1, y - 1, Font::ALIGN_TOP_CENTER, 0.0f,0.0f,0.0f,0.7f );
				font->DrawText( observed_player ? observed_player->Name : observed_ship->Name, x    , y + 1, Font::ALIGN_TOP_CENTER, 1.0f,1.0f,1.0f,0.7f );
				font->DrawText( observed_player ? observed_player->Name : observed_ship->Name, x + 1, y + 1, Font::ALIGN_TOP_CENTER, 1.0f,1.0f,1.0f,0.7f );
			}
			font->DrawText( observed_player ? observed_player->Name : observed_ship->Name, x, y, Font::ALIGN_TOP_CENTER, 0.4f,0.4f,0.4f,0.9f );
		}
		
		MessageOutput->Visible = MessageInput->Visible;
		MessageOutput->ScrollTime = game->OverlayScroll;
		
		glColor4f( 1.f, 1.f, 1.f, 1.f );
		glPopMatrix();
		return;
	}
	
	
	// Set up 3D rendering for the scene.
	
	game->Cam.FOV = vr ? game->Cfg.SettingAsDouble("vr_fov") : game->Cfg.SettingAsDouble("g_fov");
	game->Gfx.Setup3D( &(game->Cam) );
	game->Gfx.Clear();
	
	int dynamic_lights = game->Cfg.SettingAsInt("g_shader_point_lights");
	
	float crosshair_red = 0.5f;
	float crosshair_green = 0.75f;
	float crosshair_blue = 1.f;
	int ammo = observed_ship ? observed_ship->AmmoForWeapon() : -1;
	
	Model *cockpit_3d = NULL;
	
	
	// Draw background visual elements.
	
	SetBackground();
	DrawBackground();
	DrawStars();
	
	if( ! in_hyperspace )
		DrawDebris( dynamic_lights, &shots, &effects );
	
	
	// Draw the cockpit if we're doing 3D cockpit.
	
	if( observed_ship && ((game->View == XWing::View::COCKPIT) || (game->View == XWing::View::GUNNER)) )
	{
		if( (jump_progress < 1.) || in_hyperspace )
		{
			// Background visuals fade in when arriving from hyperspace.
			float red = 0.f, green = 0.f;
			float blue = std::min<float>( 0.25f, jumped_out ? (jump_progress - 1.) * 2. : (jump_progress * -0.5) );
			float alpha = jumped_out ? 1.f : (1.f - jump_progress);
			if( jumped_out && (jump_progress > 0.99) && (jump_progress < 1.01) )
				red = green = blue = 1.f;
			game->Gfx.Setup2D( 0., 0., 1., 1. );
			game->Gfx.DrawRect2D( 0,0,1,1, 0, red,green,blue,alpha );
			game->Gfx.Setup3D( &(game->Cam) );
		}
		
		if( observed_turret )
		{
			ammo = -1;
			
			if( target && ! shots_checked )
			{
				// See if the crosshair is lined up so the next shot would hit.
				std::vector<Shot*> test_shots = observed_turret->NextShots();
				for( std::vector<Shot*>::iterator shot_iter = test_shots.begin(); shot_iter != test_shots.end(); shot_iter ++ )
				{
					if( (! shot_on_target) && target->WillCollide( *shot_iter, (*shot_iter)->MaxLifetime() ) )
						shot_on_target = true;
					
					delete *shot_iter;
				}
				
				shots_checked = true;
			}
		}
		else if( (observed_ship->SelectedWeapon == Shot::TYPE_TORPEDO) || (observed_ship->SelectedWeapon == Shot::TYPE_MISSILE) )
		{
			if( observed_ship->LockingOn(target) )
			{
				crosshair_red = 1.f;
				crosshair_green = (target && (observed_ship->TargetLock > 1.f)) ? 0.f : 1.f;
				crosshair_blue = 0.f;
			}
		}
		else if( target && ! shots_checked )
		{
			// See if the crosshair is lined up so the next shot would hit.
			std::vector<Shot*> test_shots = observed_ship->NextShots();
			for( std::vector<Shot*>::iterator shot_iter = test_shots.begin(); shot_iter != test_shots.end(); shot_iter ++ )
			{
				if( (! shot_on_target) && target->WillCollide( *shot_iter, (*shot_iter)->MaxLifetime() ) )
					shot_on_target = true;
				
				delete *shot_iter;
			}
			
			shots_checked = true;
		}
		
		if( ammo == 0 )
		{
			crosshair_red = 0.75f;
			crosshair_green = 0.75f;
			crosshair_blue = 0.75f;
		}
		else if( shot_on_target )
		{
			crosshair_red = 0.25f;
			crosshair_green = 1.f;
			crosshair_blue = 0.25f;
			
			static Clock beep_aim;
			if( beep_aim.Progress() >= 1. )
				game->Snd.Play( game->Res.GetSound("beep_aim.wav") );
			beep_aim.Reset( 0.2 );
		}
		
		
		// Cockpit.
		
		if( observed_ship->Class && (game->View == XWing::View::COCKPIT) && (view_str != "crosshair") )
		{
			if( observed_ship->Group && game->Cfg.SettingAsBool("g_group_skins",true) )
			{
				std::map<uint8_t,std::string>::const_iterator skin_iter = observed_ship->Class->GroupCockpits.find( observed_ship->Group );
				if( (skin_iter != observed_ship->Class->GroupCockpits.end()) && skin_iter->second.length() )
					cockpit_3d = game->Res.GetModel( skin_iter->second );
			}
			
			if( !( cockpit_3d && cockpit_3d->VertexCount() ) )
			{
				if( vr && ! observed_ship->Class->CockpitModelVR.empty() )
					cockpit_3d = game->Res.GetModel( observed_ship->Class->CockpitModelVR );
				else
					cockpit_3d = game->Res.GetModel( observed_ship->Class->CockpitModel );
			}
		}
		
		if( cockpit_3d && cockpit_3d->Objects.size() )
		{
			// Draw the 3D cockpit.
			
			if( use_shaders )
				game->ShaderMgr.ResumeShaders();
			
			bool change_light_for_cockpit = false;
			if( game->ShaderMgr.Active() )
			{
				float ambient_scale = 1.f;
				std::vector<Vec3D> obstructions;
				
				if( deathstar )
				{
					// Reduce cockpit ambient light if we're in the trench.
					double dist_in_trench = -1. * observed_ship->DistAlong( &(deathstar->Up), deathstar );
					if( dist_in_trench > 0. )
					{
						ambient_scale = std::max<float>( 0.2f, 1. - dist_in_trench * 1.5 / deathstar->TrenchDepth );
						change_light_for_cockpit = true;
					}
				}
				
				for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
				{
					// Look for light obstructions from large ships nearby.
					if( (*ship_iter)->Radius() < observed_ship->Radius() * 2. )
						continue;
					double obstructed = (*ship_iter)->Radius() / (*ship_iter)->Dist( observed_ship );
					if( obstructed > 1. )
					{
						Vec3D obstruction = **ship_iter - *observed_ship;
						obstruction.ScaleTo( obstructed );
						obstructions.push_back( obstruction );
						change_light_for_cockpit = true;
					}
				}
				
				for( std::list<Asteroid*>::iterator asteroid_iter = asteroids.begin(); asteroid_iter != asteroids.end(); asteroid_iter ++ )
				{
					// Look for light obstructions from asteroids nearby.
					double obstructed = (*asteroid_iter)->Radius * 2. / (*asteroid_iter)->Dist( observed_ship );
					if( obstructed > 1. )
					{
						Vec3D obstruction = **asteroid_iter - *observed_ship;
						obstruction.ScaleTo( obstructed );
						obstructions.push_back( obstruction );
						change_light_for_cockpit = true;
					}
				}
				
				if( change_light_for_cockpit )
					SetWorldLights( deathstar, ambient_scale, obstructions.size() ? &obstructions : NULL );
				
				if( dynamic_lights )
					SetDynamicLights( observed_ship, observed_ship, dynamic_lights, &shots, &effects );
				
				if( blastpoints )
					SetBlastPoints( blastpoints, &(observed_ship->BlastPoints) );
			}
			
			// Technically the correct scale is 0.022, but we may need to make it higher to avoid near-plane clipping.
			// NOTE: Changing the scale may bring engine glows too close, but that's probably okay since it doesn't happen in VR.
			double model_scale = observed_ship->Class ? observed_ship->Class->ModelScale : 0.022;
			double cockpit_scale = vr ? model_scale : std::max<double>( model_scale, game->Gfx.ZNear * 0.25 );
			double cockpit_fwd = 0.;
			double cockpit_up = 0.;
			double cockpit_right = 0.;
			
			if( observed_ship->Class )
			{
				if( vr && ! observed_ship->Class->CockpitModelVR.empty() )
				{
					cockpit_fwd   = -1. * observed_ship->Class->CockpitPosVR.X / model_scale;
					cockpit_up    = -1. * observed_ship->Class->CockpitPosVR.Y / model_scale;
					cockpit_right = -1. * observed_ship->Class->CockpitPosVR.Z / model_scale;
				}
				else
				{
					cockpit_fwd   = -1. * observed_ship->Class->CockpitPos.X / model_scale;
					cockpit_up    = -1. * observed_ship->Class->CockpitPos.Y / model_scale;
					cockpit_right = -1. * observed_ship->Class->CockpitPos.Z / model_scale;
				}
			}
			
			// Show acceleration and shot hits by moving the cockpit.
			cockpit_fwd   += observed_ship->CockpitOffset.X;
			cockpit_up    += observed_ship->CockpitOffset.Y;
			cockpit_right += observed_ship->CockpitOffset.Z;
			
			// Draw the cockpit as though it were at 0,0,0 to avoid Z-fighting issues.
			Pos3D cockpit_pos( observed_ship );
			cockpit_pos.SetPos(0,0,0);
			Pos3D camera_pos( &cockpit_pos );
			if( view_str == "padlock" )
			{
				GameObject *target_obj = Raptor::Game->Data.GetObject( observed_turret ? observed_turret->Target : observed_ship->Target );
				if( target_obj )
				{
					camera_pos.Fwd = (((target && observed_ship->TargetSubsystem) ? target->TargetCenter( observed_ship->TargetSubsystem ) : *target_obj) - *observed_ship).Unit();
					camera_pos.FixVectors();
				}
			}
			
			camera_pos.Yaw( game->LookYaw );
			camera_pos.Pitch( game->LookPitch );
			game->Cam.Copy( &camera_pos );
			game->Gfx.Setup3D( &(game->Cam) );
			if( use_shaders )
				game->ShaderMgr.Set3f( "CamPos", game->Cam.X, game->Cam.Y, game->Cam.Z );
			double sway = vr ? game->Cfg.SettingAsDouble("vr_sway") : game->Cfg.SettingAsDouble("g_sway",1.,1.);
			if( sway )
			{
				cockpit_pos.Yaw  ( sway * (observed_ship->YawRate   + observed_ship->PrevYawRate  ) * 0.005 );
				cockpit_pos.Pitch( sway * (observed_ship->PitchRate + observed_ship->PrevPitchRate) * 0.0025 );
				cockpit_pos.Roll ( sway * (observed_ship->RollRate  + observed_ship->PrevRollRate ) * 0.0075 );
			}
			cockpit_pos.MoveAlong( &(cockpit_pos.Fwd),   cockpit_fwd   * cockpit_scale );
			cockpit_pos.MoveAlong( &(cockpit_pos.Up),    cockpit_up    * cockpit_scale );
			cockpit_pos.MoveAlong( &(cockpit_pos.Right), cockpit_right * cockpit_scale );
			glEnable( GL_CULL_FACE );
			cockpit_3d->DrawAt( &cockpit_pos, cockpit_scale );
			glDisable( GL_CULL_FACE );
			
			// Restore camera position for scene render.
			game->Cam.Copy( &Cam );
			game->Gfx.Setup3D( &(game->Cam) );
			
			// Reset world lights to normal.
			if( change_light_for_cockpit )
				SetWorldLights( deathstar );
			
			if( use_shaders )
			{
				// Restore camera position in shaders for lighting effects.
				game->ShaderMgr.Set3f( "CamPos", game->Cam.X, game->Cam.Y, game->Cam.Z );
				game->ShaderMgr.StopShaders();
			}
		}
		else
			// Not drawing a 3D cockpit.
			cockpit_3d = NULL;
	}
	
	
	if( ((jump_progress >= 0.) && ! jumped_out) || (game->View != XWing::View::COCKPIT) )
	{
		// Draw game objects.
		
		if( use_shaders )
			game->ShaderMgr.ResumeShaders();
		
		double draw_dist = game->Cfg.SettingAsDouble( "g_draw_dist", 6000. );
		double blast_capital = game->Cfg.SettingAsDouble( "g_blast_capital", FLT_MAX );
		double blast_fighter = game->Cfg.SettingAsDouble( "g_blast_fighter", 10000. );
		double blast_turret = game->Cfg.SettingAsDouble( "g_blast_turret", 10000. );
		double blast_asteroid = game->Cfg.SettingAsDouble( "g_blast_asteroid", 4000. );
		bool engine_glow = game->Cfg.SettingAsBool( "g_engine_glow", true );
		float engine_glow_cockpit = game->Cfg.SettingAsDouble( "g_engine_glow_cockpit", 0.5 );
		
		std::multimap<double,Renderable> sorted_renderables;
		std::map< Shader*, std::set<Asteroid*> > asteroid_shaders;
		
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = game->Data.GameObjects.begin(); obj_iter != game->Data.GameObjects.end(); obj_iter ++ )
		{
			// When in hyperspace, turret gunner should only see their attached ship and its turrets.
			if( in_hyperspace && (game->View == XWing::View::GUNNER) && observed_turret && (observed_turret->ID != obj_iter->first) && (observed_turret->ParentID != obj_iter->first) )
			{
				if( (obj_iter->second->Type() == XWing::Object::TURRET) )
				{
					Turret *turret = (Turret*) obj_iter->second;
					if( turret->ParentID != observed_turret->ParentID )
						continue;
				}
				else
					continue;
			}
			
			// We'll draw shots after everything else, so don't draw them now.
			if( obj_iter->second->Type() == XWing::Object::SHOT )
			{
				sorted_renderables.insert( std::pair<double,Renderable>( obj_iter->second->DistAlong( &(game->Cam.Fwd), &(game->Cam) ), (Shot*) obj_iter->second ) );
				continue;
			}
			
			std::vector<BlastPoint> *bp_vec = NULL;
			
			if( obj_iter->second->Type() == XWing::Object::SHIP )
			{
				Ship *ship = (Ship*) obj_iter->second;
				
				if( (ship->Health <= 0.) && (ship->Category() == ShipClass::CATEGORY_CAPITAL) && (ship->DeathClock.ElapsedSeconds() > ship->PiecesDangerousTime()) )
				{
					// Disintegrating ships draw later.
					sorted_renderables.insert( std::pair<double,Renderable>( ship->DistAlong( &(game->Cam.Fwd), &(game->Cam) ) + sqrt(ship->Radius()), ship ) );
					continue;
				}
				
				// Don't draw an external view of a ship whose cockpit we're in.
				if( (game->View == XWing::View::COCKPIT) && (ship == observed_ship) && (ship->Health > 0.) )
				{
					if( view_str != "crosshair" )
					{
						// But do draw our engine glow (later).
						double ship_throttle = (ship->JumpProgress < 1.) ? 0. : ship->GetThrottle();
						if( (ship_throttle >= 0.75) && ship->Engines.size() && engine_glow )
						{
							float engine_alpha = (ship_throttle - 0.75f) * 4.f * ship->EngineFlicker * engine_glow_cockpit;
							std::map<ShipEngine*,Pos3D> engines = ship->EnginePositions();
							for( std::map<ShipEngine*,Pos3D>::const_iterator engine_iter = engines.begin(); engine_iter != engines.end(); engine_iter ++ )
							{
								double dist_fwd = engine_iter->second.DistAlong( &(game->Cam.Fwd), &(game->Cam) );
								if( dist_fwd > 0. )
									sorted_renderables.insert( std::pair<double,Renderable>( dist_fwd, Renderable( engine_iter->first, &(engine_iter->second), engine_alpha, ship_throttle ) ) );
							}
						}
					}
					continue;
				}
				
				// Don't draw tiny ships far away.
				double size = ship->Shape.GetTriagonal();
				double dist = game->Cam.Dist(ship);
				if( (dist * (vr ? 20. : 10.) / size) > draw_dist )
					continue;
				
				// Don't draw ships that are entirely behind us.
				if( ship->DistAlong( &(game->Cam.Fwd), &(game->Cam) ) < -size )
					continue;
				
				// Draw engine glows later, with the other transparent renderables.
				if( ship->Health > 0. )
				{
					double ship_throttle = ship->GetThrottle();
					if( (ship_throttle >= 0.75) && ship->Engines.size() && engine_glow )
					{
						float engine_alpha = (ship_throttle - 0.75f) * 4.f * ship->EngineFlicker;
						double engine_scale = std::min<double>( 2., ship_throttle );
						if( (game->View == XWing::View::GUNNER) && (ship == observed_ship) )
						{
							double engine_glow_gunner = game->Cfg.SettingAsDouble("g_engine_glow_gunner",0.75);
							engine_alpha *= (float) engine_glow_gunner;
							if( observed_ship->Category() != ShipClass::CATEGORY_CAPITAL )
								engine_scale *= engine_glow_gunner;
						}
						std::map<ShipEngine*,Pos3D> engines = ship->EnginePositions();
						for( std::map<ShipEngine*,Pos3D>::const_iterator engine_iter = engines.begin(); engine_iter != engines.end(); engine_iter ++ )
							sorted_renderables.insert( std::pair<double,Renderable>( engine_iter->second.DistAlong( &(game->Cam.Fwd), &(game->Cam) ), Renderable( engine_iter->first, &(engine_iter->second), engine_alpha, engine_scale ) ) );
					}
				}
				
				// Show ship blastpoints if enabled and close enough.
				if( dist <= ((ship->Category() == ShipClass::CATEGORY_CAPITAL) ? blast_capital : blast_fighter) )
					bp_vec = &(ship->BlastPoints);
			}
			else if( obj_iter->second->Type() == XWing::Object::ASTEROID )
			{
				Asteroid *asteroid = (Asteroid*) obj_iter->second;
				double dist = game->Cam.Dist(asteroid) - asteroid->Radius;
				
				// Don't draw asteroids far away.  (Asteroid::Draw does the rest of the checks.)
				if( dist > draw_dist )
					continue;
				
				if( use_shaders )
				{
					Shader *shader = asteroid->WantShader();
					if( shader )
					{
						// If this asteroid wants to use a different shader (far away / no blastpoints) delay rendering it to reduce shader changes.
						asteroid_shaders[ shader ].insert( asteroid );
						continue;
					}
					
					// Show asteroid blastpoints if enabled and close enough.
					if( dist <= blast_asteroid )
						bp_vec = &(asteroid->BlastPoints);
				}
			}
			else if( obj_iter->second->Type() == XWing::Object::TURRET )
			{
				Turret *turret = (Turret*) obj_iter->second;
				double dist = game->Cam.Dist(turret);
				
				// Don't draw turrets far away.
				if( dist > draw_dist )
					continue;
				
				// Don't draw turrets that are entirely behind us.
				if( turret->DistAlong( &(game->Cam.Fwd), &(game->Cam) ) < -100. )
					continue;
				
				// Show turret blastpoints if enabled and close enough.
				if( dist <= blast_turret )
					bp_vec = &(turret->BlastPoints);
			}
			else if( obj_iter->second->Type() == XWing::Object::DEATH_STAR_BOX )
			{
				DeathStarBox *box = (DeathStarBox*) obj_iter->second;
				
				// Don't draw boxes far away.
				if( game->Cam.Dist(box) > draw_dist )
					continue;
				
				// Don't draw boxes that are entirely behind us.
				if( obj_iter->second->DistAlong( &(game->Cam.Fwd), &(game->Cam) ) < (box->L + box->W + box->H) / -2. )
					continue;
			}
			
			if( dynamic_lights && game->ShaderMgr.Active() )
			{
				Pos3D *pos = (obj_iter->second->Type() == XWing::Object::DEATH_STAR) ? (Pos3D*) &(game->Cam) : (Pos3D*) obj_iter->second;
				SetDynamicLights( pos, NULL, dynamic_lights, &shots, &effects );
			}
			
			if( blastpoints && bp_vec && game->ShaderMgr.Active() )
				SetBlastPoints( blastpoints, bp_vec );
			
			glPushMatrix();
			obj_iter->second->Draw();
			glPopMatrix();
		}
		
		if( game->ShaderMgr.Active() )
		{
			Shader *prev_shader = game->ShaderMgr.Selected;
			bool changed_shader = false;
			
			for( std::map< Shader*, std::set<Asteroid*> >::iterator shader_iter = asteroid_shaders.begin(); shader_iter != asteroid_shaders.end(); shader_iter ++ )
			{
				game->ShaderMgr.SelectAndCopyVars( shader_iter->first );
				changed_shader = true;
				
				for( std::set<Asteroid*>::iterator asteroid_iter = shader_iter->second.begin(); asteroid_iter != shader_iter->second.end(); asteroid_iter ++ )
				{
					Asteroid *asteroid = *asteroid_iter;
					
					if( dynamic_lights )
						SetDynamicLights( asteroid, NULL, dynamic_lights, &shots, &effects );
					/*
					// NOTE: Commented-out because asteroid_shaders is only used when blastpoints are not being drawn.
					if( blastpoints )
						SetBlastPoints( blastpoints, &(asteroid->BlastPoints) );
					*/
					
					glPushMatrix();
					asteroid->Draw();
					glPopMatrix();
				}
			}
			
			if( changed_shader )
				game->ShaderMgr.Select( prev_shader );
			
			// Remove dynamic lights and material properties so other things will render correctly.
			ClearMaterial();
			ClearDynamicLights();
			ClearBlastPoints( std::max<int>( blastpoints, prev_blastpoints ) );
		}
		
		if( use_shaders )
			game->ShaderMgr.StopShaders();
		
		glColor4f( 1.f, 1.f, 1.f, 1.f );
		
		
		// Draw anything with alpha back-to-front.
		
		glDepthMask( GL_FALSE );  // Disable writing to depth buffer to avoid transparent edge fighting.
		
		for( std::list<Effect*>::iterator effect_iter = effects.begin(); effect_iter != effects.end(); effect_iter ++ )
			sorted_renderables.insert( std::pair<double,Renderable>( (*effect_iter)->DistAlong( &(game->Cam.Fwd), &(game->Cam) ), *effect_iter ) );
		
		for( std::multimap<double,Renderable>::reverse_iterator renderable = sorted_renderables.rbegin(); renderable != sorted_renderables.rend(); renderable ++ )
		{
			glPushMatrix();
			
			if( renderable->second.ShotPtr )
				renderable->second.ShotPtr->Draw();
			else if( renderable->second.EffectPtr )
				renderable->second.EffectPtr->Draw();
			else if( renderable->second.ShipPtr )
			{
				if( use_shaders )
				{
					game->ShaderMgr.ResumeShaders();
					if( dynamic_lights )
						SetDynamicLights( renderable->second.ShipPtr, NULL, dynamic_lights, &shots, &effects );
					if( blastpoints )
						SetBlastPoints( blastpoints, &(renderable->second.ShipPtr->BlastPoints) );
				}
				glDepthMask( GL_TRUE );
				
				renderable->second.ShipPtr->Draw();
				
				glDepthMask( GL_FALSE );
				if( use_shaders )
					game->ShaderMgr.StopShaders();
			}
			else if( renderable->second.EnginePtr )
				renderable->second.EnginePtr->DrawAt( &(renderable->second.EnginePos), renderable->second.EngineAlpha, renderable->second.EngineScale );
			
			glPopMatrix();
		}
		
		glDepthMask( GL_TRUE );
	}
	
	
	// Reset the color, dynamic lights, and material properties so other things will render correctly.
	
	if( use_shaders )
	{
		game->ShaderMgr.ResumeShaders();
		
		if( game->ShaderMgr.Active() )
		{
			ClearMaterial();
			ClearDynamicLights();
		}
		
		game->ShaderMgr.StopShaders();
	}
	
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	
	
	// Draw 2D UI elements.
	
	GameObject *observed_object = NULL;
	if( observed_ship && (game->View == XWing::View::COCKPIT) )
		observed_object = observed_ship;
	else if( observed_turret && (game->View == XWing::View::GUNNER) )
		observed_object = observed_turret;
	
	if( observed_object )
	{
		uint8_t observed_team = (observed_object == observed_ship) ? observed_ship->Team : observed_turret->Team;
		
		if( observed_ship && ((jump_progress < 1.) || in_hyperspace) )
		{
			// Draw hyperspace lines.
			
			double line_progress = jumped_out ? jump_progress : (1. - jump_progress);
			Randomizer r( in_hyperspace ? Rand::Int() : observed_ship->Lifetime.TimeVal.tv_sec );
			for( size_t i = 0; i < 333; i ++ )
			{
				Pos3D pos1( observed_object );
				pos1.MoveAlong( &(observed_ship->Right), r.Double( -2000., 2000. ) );
				pos1.MoveAlong( &(observed_ship->Up),    r.Double( -2000., 2000. ) );
				pos1.MoveAlong( &(observed_ship->Fwd),   r.Double(  5000., 5500. ) );
				Pos3D pos2( &pos1 );
				pos2.MoveAlong( &(observed_ship->Fwd),  -9900. * line_progress );
				game->Gfx.DrawLine3D( pos1.X, pos1.Y, pos1.Z, pos2.X, pos2.Y, pos2.Z, (i % 3) ? 2.5f : 1.f, 1.f,1.f,1.f,1.f );
			}
		}
		
		
		if( ( target || (target_obj && ( (target_obj->Type() == XWing::Object::SHOT) || (target_obj->Type() == XWing::Object::CHECKPOINT) )) ) && ! jumped_out )
		{
			// Draw target box.
			
			double l = 0.;  // NOTE: Default value 0 is checked to see if the up and right vectors still need to be set.
			double h = 10.;
			double w = 10.;
			double max_dist = 2000.;
			int style = Raptor::Game->Cfg.SettingAsInt( "ui_box_style", 1 );
			Vec3D up    = game->Cam.Up    * h / 2.;  // FIXME: These names are confusing in the context of ui_box_style 1.
			Vec3D right = game->Cam.Right * w / 2.;  //
			
			std::deque<Pos3D> positions;
			Pos3D target_center = target_obj;
			if( target )
			{
				// Adjust position of targeting box to better fit nearby capital ships.
				Vec3D vec_from_target = game->Cam - *target;
				std::map<std::string,ModelObject*>::const_iterator object_iter = target->Shape.Objects.find("Hull");
				if( (object_iter != target->Shape.Objects.end()) && (object_iter->second->Points.size()) )
				{
					Vec3D point = object_iter->second->Points.front();
					vec_from_target -= target->Fwd   * point.X;
					vec_from_target -= target->Up    * point.Y;
					vec_from_target -= target->Right * point.Z;
				}
				double target_length = target->Shape.GetLength();
				double lengths_away = std::max<double>( 0.125, vec_from_target.Length() ) / target_length;
				//double fwd_move = target->Fwd.Dot( vec_from_target.Unit() ) * std::min<double>( 0.5, game->Cfg.SettingAsDouble( "ui_box_shift", style ? 0.25 : 0. ) / (lengths_away * lengths_away) );
				double fwd_move = style ? (target->Fwd.Dot( vec_from_target.Unit() ) * std::min<double>( 0.5, 0.25 / (lengths_away * lengths_away) ) ) : 0.;
				if( lengths_away < 0.75 )
				{
					// Adjust if we're close to a capital ship to keep the box ahead of us.
					double close_factor = std::min<double>( 1., (0.75 - lengths_away) * 4. );
					fwd_move = fwd_move * (1. - close_factor) + (0.5 * game->Cam.Fwd.Dot(&(target->Fwd))) * close_factor;
				}
				target_center += target->Fwd * fwd_move * target_length;
			}
			positions.push_back( target_center );
			
			if( target )
			{
				max_dist = target->Shape.GetTriagonal() * 50.;
				if( style )
				{
					l = target->Shape.GetLength();
					h = target->Shape.GetHeight();
					w = target->Shape.GetWidth();
					Vec3D plane_normal = (style < 0) ? game->Cam.Fwd : (positions.front() - game->Cam).Unit();
					style = abs( style );
					double fwd_out   = fabs(target->Fwd.DotPlane(   plane_normal )) * l * 0.6;
					double up_out    = fabs(target->Up.DotPlane(    plane_normal )) * h * 0.6;
					double right_out = fabs(target->Right.DotPlane( plane_normal )) * w * 0.6;
					// FIXME: Smooth out the jumpiness when switching which is longest.
					if( (fwd_out >= up_out) && (fwd_out >= right_out) )
					{
						up = target->Fwd.AlongPlane( plane_normal ).Unit() * fwd_out;
						right = up.Unit().Cross( plane_normal ) * ((style == 2) ? fwd_out : ((up_out + right_out) * 0.7));
					}
					else if( up_out >= right_out )
					{
						up = target->Up.AlongPlane( plane_normal ).Unit() * up_out;
						right = up.Unit().Cross( plane_normal ) * ((style == 2) ? up_out : ((fwd_out + right_out) * 0.7));
					}
					else
					{
						up = target->Right.AlongPlane( plane_normal ).Unit() * right_out;
						right = up.Unit().Cross( plane_normal ) * ((style == 2) ? right_out : ((fwd_out + up_out) * 0.7));
					}
				}
				else
				{
					h = fabs( game->Cam.Up.Dot( &(target->Up) ) * target->Shape.GetHeight() ) + fabs( game->Cam.Up.Dot( &(target->Fwd) ) * target->Shape.GetLength() ) + fabs( game->Cam.Up.Dot( &(target->Right) ) * target->Shape.GetWidth() );
					w = fabs( game->Cam.Right.Dot( &(target->Up) ) * target->Shape.GetHeight() ) + fabs( game->Cam.Right.Dot( &(target->Fwd) ) * target->Shape.GetLength() ) + fabs( game->Cam.Right.Dot( &(target->Right) ) * target->Shape.GetWidth() );
				}
			}
			else if( target_obj->Type() == XWing::Object::CHECKPOINT )
			{
				const Checkpoint *checkpoint = (const Checkpoint*) target_obj;
				w = h = (checkpoint->Radius * 0.75);
			}
			
			if( ! l )  // If we're using ui_box_style 1/2 and targeting a ship, these vectors are already set.
			{
				up    = game->Cam.Up    * h / 2.;
				right = game->Cam.Right * w / 2.;
			}
			
			float red = 1.f, green = 1.f, blue = 1.f, alpha = 1.f;
			bool friendly = (observed_team && target && target->Team && (target->Team == observed_team)) || (target_obj->Type() == XWing::Object::CHECKPOINT);
			std::vector<double> box_color = game->Cfg.SettingAsDoubles( friendly ? "ui_box_color2" : "ui_box_color" );
			if( box_color.size() >= 3 )
			{
				red   = box_color[ 0 ];
				green = box_color[ 1 ];
				blue  = box_color[ 2 ];
				if( box_color.size() >= 4 )
					alpha = box_color[ 3 ];
			}
			else if( target->Team && Str::EqualsInsensitive( game->Cfg.SettingAsString( friendly ? "ui_box_color2" : "ui_box_color" ), "team" ) )
			{
				if( target->Team == XWing::Team::REBEL )
				{
					green = 0.4f;
					blue  = 0.3f;
				}
				else if( target->Team == XWing::Team::EMPIRE )
				{
					red   = 0.3f;
					green = 0.6f;
				}
			}
			else if( friendly )
				red = blue = 0.f;
			
			float thickness = game->Cfg.SettingAsDouble( "ui_box_line", 1.5f, 1.5f );
			
			// Draw target box around selected subsystem.
			// FIXME: For turrets, this assumes the turret and parent ship have the same ship/subsystem targeted!
			if( target && observed_ship && observed_ship->TargetSubsystem && (observed_ship->TargetSubsystem <= target->Subsystems.size()) )
				positions.push_back( target->TargetCenter( observed_ship->TargetSubsystem ) );
			
			// Turret gunners get another target box for the intercept point.
			if( observed_turret && (!( target && (target->Category() == ShipClass::CATEGORY_CAPITAL) )) && (target_obj->MotionVector.Length() > 40.) )
			{
				Pos3D gun = observed_turret->GunPos();
				Vec3D vec_to_target( target_obj->X - gun.X, target_obj->Y - gun.Y, target_obj->Z - gun.Z );
				double dist_to_target = vec_to_target.Length();
				Vec3D shot_vec = gun.Fwd;
				shot_vec.ScaleTo( 750. );  // Shot::Speed
				shot_vec += observed_turret->MotionVector;
				shot_vec.ScaleTo( 750. );  // Shot::Speed
				shot_vec -= target_obj->MotionVector;
				double time_to_target = dist_to_target / shot_vec.Length();
				Pos3D intercept = *target_obj + (target->MotionVector - observed_turret->MotionVector) * time_to_target;
				positions.push_back( intercept );
			}
			
			glDisable( GL_DEPTH_TEST );
			
			for( std::deque<Pos3D>::iterator pos_iter = positions.begin(); pos_iter != positions.end(); pos_iter ++ )
			{
				Pos3D pos = *pos_iter;
				Vec3D vec_to_target( pos.X - game->Cam.X, pos.Y - game->Cam.Y, pos.Z - game->Cam.Z );
				double dist = vec_to_target.Length();
				if( dist > max_dist )
				{
					vec_to_target.ScaleTo( 1. );
					pos.MoveAlong( &vec_to_target, max_dist - dist );
				}
				
				game->Gfx.DrawLine3D( pos.X - right.X + up.X, pos.Y - right.Y + up.Y, pos.Z - right.Z + up.Z, pos.X - right.X / 2. + up.X, pos.Y - right.Y / 2. + up.Y, pos.Z - right.Z / 2. + up.Z, thickness, red, green, blue, alpha );
				game->Gfx.DrawLine3D( pos.X - right.X + up.X, pos.Y - right.Y + up.Y, pos.Z - right.Z + up.Z, pos.X - right.X + up.X / 2., pos.Y - right.Y + up.Y / 2., pos.Z - right.Z + up.Z / 2., thickness, red, green, blue, alpha );
				
				game->Gfx.DrawLine3D( pos.X + right.X + up.X, pos.Y + right.Y + up.Y, pos.Z + right.Z + up.Z, pos.X + right.X / 2. + up.X, pos.Y + right.Y / 2. + up.Y, pos.Z + right.Z / 2. + up.Z, thickness, red, green, blue, alpha );
				game->Gfx.DrawLine3D( pos.X + right.X + up.X, pos.Y + right.Y + up.Y, pos.Z + right.Z + up.Z, pos.X + right.X + up.X / 2., pos.Y + right.Y + up.Y / 2., pos.Z + right.Z + up.Z / 2., thickness, red, green, blue, alpha );
				
				game->Gfx.DrawLine3D( pos.X + right.X - up.X, pos.Y + right.Y - up.Y, pos.Z + right.Z - up.Z, pos.X + right.X / 2. - up.X, pos.Y + right.Y / 2. - up.Y, pos.Z + right.Z / 2. - up.Z, thickness, red, green, blue, alpha );
				game->Gfx.DrawLine3D( pos.X + right.X - up.X, pos.Y + right.Y - up.Y, pos.Z + right.Z - up.Z, pos.X + right.X - up.X / 2., pos.Y + right.Y - up.Y / 2., pos.Z + right.Z - up.Z / 2., thickness, red, green, blue, alpha );
				
				game->Gfx.DrawLine3D( pos.X - right.X - up.X, pos.Y - right.Y - up.Y, pos.Z - right.Z - up.Z, pos.X - right.X / 2. - up.X, pos.Y - right.Y / 2. - up.Y, pos.Z - right.Z / 2. - up.Z, thickness, red, green, blue, alpha );
				game->Gfx.DrawLine3D( pos.X - right.X - up.X, pos.Y - right.Y - up.Y, pos.Z - right.Z - up.Z, pos.X - right.X - up.X / 2., pos.Y - right.Y - up.Y / 2., pos.Z - right.Z - up.Z / 2., thickness, red, green, blue, alpha );
				
				if( target && observed_ship && observed_ship->TargetSubsystem && (pos_iter == positions.begin()) )
				{
					if( style > 0 )
					{
						Vec3D plane_normal = (target->TargetCenter(observed_ship->TargetSubsystem) - game->Cam).Unit();
						double fwd_out   = fabs(target->Fwd.DotPlane(   plane_normal ));
						double up_out    = fabs(target->Up.DotPlane(    plane_normal ));
						double right_out = fabs(target->Right.DotPlane( plane_normal ));
						if( (fwd_out >= up_out) && (fwd_out >= right_out) )
						{
							up = target->Fwd.AlongPlane( plane_normal );
							right = up.Unit().Cross( plane_normal );
						}
						else if( up_out >= right_out )
						{
							up = target->Up.AlongPlane( plane_normal );
							right = up.Unit().Cross( plane_normal );
						}
						else
						{
							up = target->Right.AlongPlane( plane_normal );
							right = up.Unit().Cross( plane_normal );
						}
					}
					// FIXME: Get actual size of subsystem!
					up.ScaleTo( 17. );
					right.ScaleTo( 17. );
				}
				else if( observed_turret )
				{
					// Change the parameters for the intercept box.
					red   = crosshair_red;
					green = crosshair_green;
					blue  = crosshair_blue;
					up.ScaleBy( 0.8 );
					right.ScaleBy( 0.8 );
				}
			}
			
			glEnable( GL_DEPTH_TEST );
		}
		
		
		if( (jump_progress >= 1.) && ! in_hyperspace )
		{
			// Draw the 3D crosshair.
			
			glDisable( GL_DEPTH_TEST );
			glDisable( GL_TEXTURE_2D );
			
			Pos3D crosshair_pos = (observed_object == observed_ship) ? observed_ship->HeadPos() : observed_turret->HeadPos();
			crosshair_pos.MoveAlong( &(crosshair_pos.Fwd), 100. );
			glColor4f( crosshair_red, crosshair_green, crosshair_blue, std::min<float>( 1.f, jump_progress - 1. ) );
			
			glPointSize( 2.f );
			glBegin( GL_POINTS );
				glVertex3d( crosshair_pos.X, crosshair_pos.Y, crosshair_pos.Z );
			glEnd();
			
			Vec3D up( &(crosshair_pos.Up) );
			Vec3D right( &(crosshair_pos.Right) );
			up.ScaleTo( 2. );
			right.ScaleTo( 2. );
			
			Vec3D weapon_vec;
			Vec2D relative_weapon_vec;
			Pos3D weapon_pos;
			
			// Draw lines for all selected weapon ports.
			glLineWidth( game->Cfg.SettingAsDouble("g_crosshair_thickness",1.5) );
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
				
				// FIXME: Just iterate on the list of weapon ports in ShipClass instead of calling AllShots!
				std::vector<Shot*> all_weapons = (observed_object == observed_ship) ? observed_ship->AllShots() : std::vector<Shot*>();
				for( std::vector<Shot*>::iterator shot_iter = all_weapons.begin(); shot_iter != all_weapons.end(); shot_iter ++ )
				{
					weapon_vec.Set( (*shot_iter)->X - observed_object->X, (*shot_iter)->Y - observed_object->Y, (*shot_iter)->Z - observed_object->Z );
					relative_weapon_vec.Set( weapon_vec.Dot(&(observed_object->Right)), weapon_vec.Dot(&(observed_object->Up)) );
					relative_weapon_vec.ScaleTo( (relative_weapon_vec.Length() > 0.001) ? 1. : 0. );
					weapon_pos.Copy( &crosshair_pos );

					if( ((*shot_iter)->ShotType == Shot::TYPE_TORPEDO) || ((*shot_iter)->ShotType == Shot::TYPE_MISSILE) )
						relative_weapon_vec.ScaleBy( 0.9 );
					else if( (*shot_iter)->ShotType == Shot::TYPE_ION_CANNON )
						relative_weapon_vec.ScaleBy( 0.95 );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X * 2. );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y * 2. );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
				}
			glEnd();
			
			// Draw little dots for all selected weapon ports.
			glPointSize( 2.f );
			glBegin( GL_POINTS );
				for( std::vector<Shot*>::iterator shot_iter = all_weapons.begin(); shot_iter != all_weapons.end(); shot_iter ++ )
				{
					weapon_vec.Set( (*shot_iter)->X - observed_object->X, (*shot_iter)->Y - observed_object->Y, (*shot_iter)->Z - observed_object->Z );
					relative_weapon_vec.Set( weapon_vec.Dot(&(observed_object->Right)), weapon_vec.Dot(&(observed_object->Up)) );
					relative_weapon_vec.ScaleTo( (relative_weapon_vec.Length() > 0.001) ? 1. : 0. );
					weapon_pos.Copy( &crosshair_pos );
					
					if( ((*shot_iter)->ShotType == Shot::TYPE_TORPEDO) || ((*shot_iter)->ShotType == Shot::TYPE_MISSILE) )
						relative_weapon_vec.ScaleBy( 0.9 );
					else if( (*shot_iter)->ShotType == Shot::TYPE_ION_CANNON )
						relative_weapon_vec.ScaleBy( 0.95 );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X * 3.6 );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y * 3.6 );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
					
					delete *shot_iter;
				}
			glEnd();
			
			// Draw big dots for the next shots to fire.
			glPointSize( 7.f );
			glBegin( GL_POINTS );
				std::vector<Shot*> next_weapons = (observed_object == observed_ship) ? observed_ship->NextShots() : std::vector<Shot*>();
				for( std::vector<Shot*>::iterator shot_iter = next_weapons.begin(); shot_iter != next_weapons.end(); shot_iter ++ )
				{
					weapon_vec.Set( (*shot_iter)->X - observed_object->X, (*shot_iter)->Y - observed_object->Y, (*shot_iter)->Z - observed_object->Z );
					relative_weapon_vec.Set( weapon_vec.Dot(&(observed_object->Right)), weapon_vec.Dot(&(observed_object->Up)) );
					relative_weapon_vec.ScaleTo( (relative_weapon_vec.Length() > 0.001) ? 1. : 0. );
					weapon_pos.Copy( &crosshair_pos );
					
					if( ((*shot_iter)->ShotType == Shot::TYPE_TORPEDO) || ((*shot_iter)->ShotType == Shot::TYPE_MISSILE) )
						relative_weapon_vec.ScaleBy( 0.9 );
					else if( (*shot_iter)->ShotType == Shot::TYPE_ION_CANNON )
						relative_weapon_vec.ScaleBy( 0.95 );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X * 3.6 );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y * 3.6 );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
					
					delete *shot_iter;
				}
			glEnd();
			
			if( (observed_object == observed_ship) && observed_ship->SelectedWeapon && (ammo >= 0) )
			{
				crosshair_pos.MoveAlong( &up, -0.8 );
				ScreenFont->DrawText3D( Num::ToString(ammo), &crosshair_pos, Font::ALIGN_TOP_CENTER, 0.05 );
			}
			
			glEnable( GL_DEPTH_TEST );
		}
		
		
		if( jump_progress >= 1.35 )
		{
			// Here we'll draw target info, unless it's done via cockpit screens.
			
			if( (target || dead_target) && need_target_holo && ! jumped_out )
			{
				// Show holographic targetting display.
				
				Ship *observed_target = target ? target : dead_target;
				Vec3D vec_to_target( observed_target->X - observed_object->X, observed_target->Y - observed_object->Y, observed_target->Z - observed_object->Z );
				int dist = vec_to_target.Length();
				
				double holo_scale = 0.125 / observed_target->Shape.GetTriagonal();
				Color holo_color1( 0.25f, 0.75f, 1.f, 0.7f );
				Color holo_color2( 1.f,   1.f,   1.f, 0.4f );
				float red = 1.f, green = 1.f, blue = 1.f, alpha = 1.f;
				if( dead_target )
				{
					float dead_time = dead_target->DeathClock.ElapsedSeconds();
					alpha = std::max<float>( 0.f, 1.f - dead_time );
					holo_color1.Red   *= alpha;
					holo_color1.Green *= alpha;
					holo_color1.Alpha *= std::max<float>( 0.f, std::min<float>( 1.f, 2.f - dead_time ) );
					holo_color2.Alpha *= std::max<float>( 0.f, 1.f - dead_time * 0.5f );
					holo_scale /= 1. + dead_time * dead_target->ExplosionRate();
				}
				
				Pos3D holo_pos( observed_target ), head_pos( observed_object );
				
				if( ! vr )
				{
					Camera cam_to_target( game->Cam );
					cam_to_target.Fwd = vec_to_target.Unit();
					cam_to_target.Up.Copy( &(observed_object->Up) );
					cam_to_target.FixVectors();
					cam_to_target.SetPos( observed_target->X, observed_target->Y, observed_target->Z );
					cam_to_target.MoveAlong( &(cam_to_target.Fwd), -0.75 );
					cam_to_target.Pitch( 25. );
					cam_to_target.Yaw( game->LookYaw );
					cam_to_target.Pitch( game->LookPitch );
					game->Gfx.Setup3D( &(cam_to_target) );
				}
				else if( observed_turret )
				{
					head_pos = observed_turret->HeadPos();
					holo_pos.SetPos( head_pos.X, head_pos.Y, head_pos.Z );
					holo_pos.MoveAlong( &(head_pos.Fwd), 0.45 );
					holo_pos.MoveAlong( &(head_pos.Up), -0.2 );
				}
				else if( observed_ship )
				{
					head_pos = observed_ship->HeadPosVR();
					holo_pos.SetPos( head_pos.X, head_pos.Y, head_pos.Z );
					holo_pos.MoveAlong( &(head_pos.Fwd), 0.675 );
					holo_pos.MoveAlong( &(head_pos.Up), -0.315 );
				}
				
				glLineWidth( 1.f );
				observed_target->DrawWireframeAt( &holo_pos, &holo_color1, holo_scale );
				
				// Draw hit/explosion effect when target is recently damaged.
				double hit_time = observed_target->HitClock.ElapsedSeconds();
				if( hit_time < (target ? 0.125 : 0.5) )
				{
					float alpha = 1. - hit_time * (target ? 4. : 2.);
					float r = 1.f, g = 1.f;
					if( ! target )
					{
						r = sqrtf(alpha);
						g = sqrtf(r);
					}
					Pos3D pos( &holo_pos );
					if( (observed_target->Health > 0.) && observed_target->RecentBlastPoint )
					{
						pos.MoveAlong( &(pos.Fwd),   observed_target->RecentBlastPoint->X * holo_scale );
						pos.MoveAlong( &(pos.Up),    observed_target->RecentBlastPoint->Y * holo_scale );
						pos.MoveAlong( &(pos.Right), observed_target->RecentBlastPoint->Z * holo_scale );
					}
					game->Gfx.DrawSphere3D( pos.X, pos.Y, pos.Z, holo_scale * sqrt(observed_target->Radius()) * hit_time * (target ? (vr ? 10. : 7.) : 15.), 8, 0, r,g,1.f,alpha );
				}
				
				glLineWidth( 2.f );
				observed_target->DrawWireframeAt( &holo_pos, &holo_color2, holo_scale );
				
				std::string target_name = observed_target->Name;
				Player *target_player = observed_target->Owner();
				if( target_player )
					target_name = target_player->Name;
				
				if( observed_target->Class && (observed_target->Class->Category != ShipClass::CATEGORY_TARGET) )
					target_name = observed_target->Class->ShortName + std::string(": ") + target_name;
				
				if( observed_team && (observed_target->Team == observed_team) )
				{
					red = 0.f;
					green = vr ? 0.75f : 1.f;
					blue = 0.f;
				}
				else
				{
					red = 1.f;
					green = 0.f;
					blue = 0.f;
				}
				
				std::string target_status;
				bool classic = Raptor::Game->Cfg.SettingAsBool("ui_classic");
				std::string dist_label = "Dist:";
				std::string dist_str = Num::ToString(dist);
				
				if( ! classic )
				{
					char shield_str[ 5 ] = "----", hull_str[ 5 ] = "----";
					if( target )
					{
						double max_shield = target->MaxShield();
						if( max_shield )
						{
							double shield = std::min<double>( target->ShieldF, target->ShieldR );
							if( target->ShieldPos == Ship::SHIELD_FRONT )
								shield = target->ShieldF * 0.5;
							else if( target->ShieldPos == Ship::SHIELD_REAR )
								shield = target->ShieldR * 0.5;
							if( shield > 0. )
								snprintf( shield_str, sizeof(shield_str), "%3.0f%%", ceil( 100. * shield / max_shield ) );
							else
								snprintf( shield_str, sizeof(shield_str), "DOWN" );
						}
						else
							snprintf( shield_str, sizeof(shield_str), "NONE" );
						snprintf( hull_str, sizeof(hull_str), "%3.0f%%", ceil( 100. * target->Health / target->MaxHealth() ) );
					}
					target_status = std::string("SHLD:") + std::string(shield_str) + std::string("  HULL:") + std::string(hull_str);
					
					dist_label = "DIST:";
					char dist_buf[ 32 ];
					if( dist < 1000 )
						snprintf( dist_buf, sizeof(dist_buf), "%i ", dist );
					else
						snprintf( dist_buf, sizeof(dist_buf), "%.1fK", dist / 1000. );
					dist_str = std::string(dist_buf);
				}
				else if( ! target )
					target_status = "Destroyed";
				else if( target->Health < (target->MaxHealth() * 0.7) )
					target_status = "Hull Damaged";
				else if( (target->MaxShield() > 0.)
				&&  (   ((target->ShieldF <= 0.) && (target->ShieldPos != Ship::SHIELD_REAR))
				     || ((target->ShieldR <= 0.) && (target->ShieldPos != Ship::SHIELD_FRONT)) ) )
					target_status = "Shields Down";
				else
					target_status = "OK";
				
				if( vr )
				{
					Pos3D text_pos( &head_pos );
					text_pos.SetPos( holo_pos.X, holo_pos.Y, holo_pos.Z );
					
					// Draw target name.
					text_pos.Pitch( -45. );
					text_pos.MoveAlong( &(text_pos.Up), -0.1 );
					BigFont->DrawText3D( target_name, &text_pos, Font::ALIGN_BOTTOM_CENTER, red, green, blue, holo_color1.Alpha, 0.0015 );
					
					// Draw target status (health).
					BigFont->DrawText3D( target_status, &text_pos, Font::ALIGN_TOP_CENTER, 1.f, 1.f, 0.5f, holo_color1.Alpha, 0.001 );
					
					// Draw target distance.
					text_pos.MoveAlong( &(text_pos.Up), -0.025 );
					text_pos.MoveAlong( &(text_pos.Right), -0.075 );
					BigFont->DrawText3D( dist_label, &text_pos, Font::ALIGN_TOP_LEFT, holo_color1.Red, holo_color2.Green, holo_color2.Blue, holo_color1.Alpha, 0.001 );
					text_pos.MoveAlong( &(text_pos.Right), 0.15 );
					BigFont->DrawText3D( dist_str, &text_pos, Font::ALIGN_TOP_RIGHT, holo_color1.Red, holo_color2.Green, holo_color2.Blue, holo_color1.Alpha, 0.001 );
				}
				else
				{
					game->Gfx.Setup2D();
					
					// Draw target name.
					BigFont->DrawText( target_name, Rect.x + Rect.w/2 + 1, Rect.h - 32,  Font::ALIGN_BOTTOM_CENTER, 0.f, 0.f, 0.f, alpha * 0.8f );
					BigFont->DrawText( target_name, Rect.x + Rect.w/2,     Rect.h - 33, Font::ALIGN_BOTTOM_CENTER, red, green, blue, alpha );
					
					// Draw target status (health).
					SmallFont->DrawText( target_status, Rect.x + Rect.w/2 + 1, Rect.h - 16, Font::ALIGN_BOTTOM_CENTER, 0.f, 0.f, 0.f, alpha * 0.8f );
					SmallFont->DrawText( target_status, Rect.x + Rect.w/2,     Rect.h - 17, Font::ALIGN_BOTTOM_CENTER, 1.f, 1.f, 0.f, alpha );
					
					// Draw target distance.
					int dist_sep = classic ? 60 : 42;
					SmallFont->DrawText( dist_label, Rect.x + Rect.w/2 - (dist_sep-1), Rect.h,     Font::ALIGN_BOTTOM_LEFT, 0.f,  0.f,  0.f, alpha * 0.8f );
					SmallFont->DrawText( dist_label, Rect.x + Rect.w/2 -  dist_sep,    Rect.h - 2, Font::ALIGN_BOTTOM_LEFT, holo_color1.Red, holo_color1.Green, holo_color1.Blue, alpha );
					SmallFont->DrawText( dist_str,   Rect.x + Rect.w/2 + (dist_sep+1), Rect.h,     Font::ALIGN_BOTTOM_RIGHT, 0.f,  0.f,  0.f, alpha * 0.8f );
					SmallFont->DrawText( dist_str,   Rect.x + Rect.w/2 +  dist_sep,    Rect.h - 1, Font::ALIGN_BOTTOM_RIGHT, holo_color1.Red, holo_color1.Green, holo_color1.Blue, alpha );
				}
			}
			
			
			if( (jump_progress >= 1.25) && ! (vr || jumped_out) )
			{
				// Draw the radar.
				
				game->Gfx.Setup2D( game->Gfx.H / -2, game->Gfx.H / 2 );
				double h = game->Gfx.H / 2.;
				RadarDirectionFont->DrawText( "F", -1.304 * h, -0.829 * h, Font::ALIGN_TOP_RIGHT, 0.f, 0.f, 1.f, 0.75f, Raptor::Game->UIScale );
				RadarDirectionFont->DrawText( "R",  1.304 * h, -0.829 * h, Font::ALIGN_TOP_LEFT,  0.f, 0.f, 1.f, 0.75f, Raptor::Game->UIScale );
				
				game->Gfx.Setup2D( -1., 1. );
				game->Gfx.DrawCircle2D( -1.233, -0.9, 0.1, 32, 0, 0.f, 0.f, 0.f, 0.75f );
				game->Gfx.DrawCircle2D( 1.233, -0.9, 0.1, 32, 0, 0.f, 0.f, 0.f, 0.75f );
				
				Pos3D *radar_ref = (Pos3D*) &(game->Cam);
				
				for( std::map<uint32_t,GameObject*>::iterator obj_iter = game->Data.GameObjects.begin(); obj_iter != game->Data.GameObjects.end(); obj_iter ++ )
				{
					uint32_t type = obj_iter->second->Type();
					if( ((type == XWing::Object::SHIP) || (type == XWing::Object::ASTEROID) || (type == XWing::Object::SHOT) || (type == XWing::Object::CHECKPOINT))
					&&  (obj_iter->second != observed_object) )
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
							
							if( (ship->Health <= 0.) || ship->JumpedOut )
								continue;
						}

						Vec3D vec_to_obj( obj->X - radar_ref->X, obj->Y - radar_ref->Y, obj->Z - radar_ref->Z );
						vec_to_obj.ScaleTo( 1. );
						double x = vec_to_obj.Dot( &(radar_ref->Right) ) * 0.096;
						double y = -0.9 - vec_to_obj.Dot( &(radar_ref->Up) ) * 0.096;
						if( vec_to_obj.Dot( &(radar_ref->Fwd) ) >= 0. )
							x -= 1.233;
						else
							x += 1.233;
						
						float red = 0.5f, green = 0.5f, blue = 0.5f;
						double radius = 0.0035;
						
						if( ship )
						{
							if( observed_team && (ship->Team == observed_team) )
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
							if( ship->Category() == ShipClass::CATEGORY_CAPITAL )
								radius = 0.005;
							else if( (ship->Target == observed_object->ID) && ((int) observed_object->Lifetime.ElapsedMilliseconds() % ((ship->TargetLock >= 1.f) ? 200 : ((ship->TargetLock > 0.f) ? 400 : 800)) <= 50) )
								radius = 0.0055;
						}
						else if( shot )
						{
							red = 1.f;
							green = 1.f;
							blue = 0.f;
							radius = ((shot->Seeking == observed_object->ID) && ((int) observed_object->Lifetime.ElapsedMilliseconds() % 200 <= 50)) ? 0.0035 : 0.002;
						}
						else if( type == XWing::Object::ASTEROID )
						{
							green = 0.4f;
							blue = 0.3f;
							radius += 0.0005 + ((const Asteroid*) obj )->Radius * 0.00001;
						}
						else if( type == XWing::Object::CHECKPOINT )
						{
							if( observed_ship && (observed_ship->NextCheckpoint == obj->ID) )
							{
								red = 0.f;
								green = 1.f;
								blue = 1.f;
							}
							else
							{
								red = 0.f;
								green = 0.5f;
								blue = 0.5f;
								radius = 0.0025;
							}
						}
						
						game->Gfx.DrawCircle2D( x, y, radius, 6, 0, red, green, blue, 1.f );
					}
				}
				
				game->Gfx.DrawCircleOutline2D( -1.233, -0.9, 0.1, 32, 1.f, 0.f, 0.f, 1.f, 0.75f );
				game->Gfx.DrawCircleOutline2D( 1.233, -0.9, 0.1, 32, 1.f, 0.f, 0.f, 1.f, 0.75f );
				
				if( target_obj )
				{
					// Draw target in radar.
					
					Vec3D vec_to_target( target_obj->X - radar_ref->X, target_obj->Y - radar_ref->Y, target_obj->Z - radar_ref->Z );
					if( target && observed_ship && observed_ship->TargetSubsystem )
						vec_to_target = target->TargetCenter( observed_ship->TargetSubsystem ) - *radar_ref;
					
					vec_to_target.ScaleTo( 1. );
					double x = vec_to_target.Dot( &(radar_ref->Right) ) * 0.096;
					double y = -0.9 - vec_to_target.Dot( &(radar_ref->Up) ) * 0.096;
					if( vec_to_target.Dot( &(radar_ref->Fwd) ) >= 0. )
						x -= 1.233;
					else
						x += 1.233;
					
					float red = 1.f, green = 1.f, blue = 1.f;
					double radius = 0.0045;
					
					if( target )
					{
						if( observed_team && (target->Team == observed_team) )
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
						if( target->Category() == ShipClass::CATEGORY_CAPITAL )
							radius = 0.006;
						else if( (target->Target == observed_object->ID) && ((int) observed_object->Lifetime.ElapsedMilliseconds() % ((target->TargetLock >= 1.f) ? 200 : ((target->TargetLock > 0.f) ? 400 : 800)) <= 50) )
							radius = 0.0055;
					}
					else if( target_obj->Type() == XWing::Object::SHOT )
					{
						red = 1.f;
						green = 1.f;
						blue = 0.f;
						radius = ((((Shot*)target_obj)->Seeking == observed_object->ID) && ((int) observed_object->Lifetime.ElapsedMilliseconds() % 200 <= 50)) ? 0.0035 : 0.002;
					}
					else if( target_obj->Type() == XWing::Object::CHECKPOINT )
					{
						red = 0.f;
						green = 1.f;
						blue = 1.f;
					}
					else if( target_obj->Type() == XWing::Object::SHIP ) // Dead ship still targeted.
					{
						red = 0.5f;
						green = 0.5f;
						blue = 0.5f;
					}
					else // Turret or other misc target.
					{
						red = 0.7f;
						green = 0.f;
						blue = 0.f;
					}
					
					game->Gfx.DrawBox2D( x - 0.01, y - 0.01, x + 0.01, y + 0.01, 1.f, 1.f, 1.f, 1.f, 1.f );
					game->Gfx.DrawCircle2D( x, y, radius, 6, 0, red, green, blue, 1.f );
				}
			}
		}
	}
	
	
	float ui_scale = Raptor::Game->UIScale;
	int text_x = Rect.x + Rect.w/2;
	
	
	// If we're spectating, show who we're watching.
	
	if( (observed_player && (observed_player->ID == game->PlayerID)) || cinematic || vr )
		;
	else if( observed_player )
	{
		game->Gfx.Setup2D();
		BigFont->DrawText( "Watching: " + observed_player->Name, text_x + 2, Rect.y + 12 * ui_scale, Font::ALIGN_TOP_CENTER, 0.f,0.f,0.f,0.8f, ui_scale );
		BigFont->DrawText( "Watching: " + observed_player->Name, text_x,     Rect.y + 10 * ui_scale, Font::ALIGN_TOP_CENTER,                   ui_scale );
	}
	else if( observed_ship )
	{
		game->Gfx.Setup2D();
		BigFont->DrawText( "Watching: " + observed_ship->Name, text_x + 2, Rect.y + 12 * ui_scale, Font::ALIGN_TOP_CENTER, 0.f,0.f,0.f,0.8f, ui_scale );
		BigFont->DrawText( "Watching: " + observed_ship->Name, text_x,     Rect.y + 10 * ui_scale, Font::ALIGN_TOP_CENTER,                   ui_scale );
	}
	
	
	// If the round is over, player is holding the scores key, or player is dead in VR, show the scores.
	
	bool draw_scores = (game->State >= XWing::State::ROUND_ENDED) && ! cinematic;
	if( ! draw_scores )
		draw_scores = game->Input.ControlPressed( XWing::Control::SCORES );
	if( ! draw_scores )
		draw_scores = vr && observed_ship && (observed_ship->PlayerID == game->PlayerID) && (observed_ship->Health <= 0.) && ! cinematic;
	if( draw_scores )
		DrawScores();
	
	
	// If we're waiting to spawn, show the respawn clock and beep.
	
	double respawn_in = game->RespawnTimer.CountUpToSecs;
	if( respawn_in > 0. )
	{
		float respawn_time = game->RespawnTimer.RemainingSeconds();
		if( respawn_time > 0. )
		{
			static int prev_respawn_seconds = 0;
			int respawn_seconds = ceilf( respawn_time );
			
			if( !( cinematic || draw_scores ) )
			{
				std::string respawn_str = Num::ToString( respawn_seconds );
				Player *player = game->Data.GetPlayer( game->PlayerID );
				std::string player_team = player ? player->PropertyAsString("team") : "";
				float r = 1.f, g = 1.f, b = 1.f, a = std::min<float>( 1.f, fmodf( respawn_time, 1.f ) * 2.f );
				if( player_team == "Rebel" )
					g = b = a;
				else if( player_team == "Empire" )
					r = g = a*a*a;
				game->Gfx.Setup2D();
				BigFont->DrawText( respawn_str, text_x, Rect.y + 68 * ui_scale, Font::ALIGN_MIDDLE_CENTER, r,g,b,a, ui_scale );
			}
			
			if( (respawn_seconds != prev_respawn_seconds) && (respawn_seconds <= 3) )
				game->Snd.Play( game->Res.GetSound("beep_respawn.wav") );
			
			prev_respawn_seconds = respawn_seconds;
		}
	}
	else if( respawn_in < 0. )
	{
		float g = fabsf( cos( PlayTime.ElapsedSeconds() * M_PI ) ) * 0.5f;
		game->Gfx.Setup2D();
		BigFont->DrawText( "Flagship Lost", text_x, Rect.y + 68 * ui_scale, Font::ALIGN_MIDDLE_CENTER, 1.f,g,  0.f,1.f, ui_scale );
	}
	
	
	// Flash "PAUSED" or current time scale if altered, unless in screensaver or cinematic mode.
	
	if( (game->Data.TimeScale != 1.) && ! cinematic )
	{
		float r = fabsf( cos( PlayTime.ElapsedSeconds() * M_PI ) );
		float g = 0.5f + r * 0.5f;
		float b = 1.f;
		game->Gfx.Setup2D();
		if( game->Data.TimeScale < 0.0000011 )
		{
			BigFont->DrawText( "PAUSED", text_x + 2 * ui_scale, Rect.y + Rect.h * 0.47 + 2 * ui_scale, Font::ALIGN_MIDDLE_CENTER, 0.f,0.f,0.f,0.8f, ui_scale );
			BigFont->DrawText( "PAUSED", text_x,                Rect.y + Rect.h * 0.47,                Font::ALIGN_MIDDLE_CENTER, r,  g,  b,  1.f,  ui_scale );
		}
		else
		{
			BigFont->DrawText( Num::ToString(game->Data.TimeScale) + std::string(" X"), text_x + 2 * ui_scale, Rect.y + 48 * ui_scale, Font::ALIGN_MIDDLE_CENTER, 0.f,0.f,0.f,0.8f, ui_scale );
			BigFont->DrawText( Num::ToString(game->Data.TimeScale) + std::string(" X"), text_x,                Rect.y + 46 * ui_scale, Font::ALIGN_MIDDLE_CENTER, r,  g,  b,  1.f,  ui_scale );
		}
	}
	
	
	// Show who is talking on comms.
	
	DrawVoice();
	
	
	// Don't show non-chat messages in VR unless the chat box is open or show scores button is held.
	
	MessageOutput->Visible = true;
	MessageOutput->SetTypeToDraw( TextConsole::MSG_NORMAL, draw_scores || MessageInput->Visible || (! (vr || cinematic)) || game->Cfg.SettingAsBool("vr_messages") );
	MessageOutput->ScrollTime = ((const XWingGame*)( game ))->OverlayScroll;
	
	
	glPopMatrix();
}


void RenderLayer::DrawBackground( void )
{
	// Don't draw if g_bg=false.
	if( ! Raptor::Game->Cfg.SettingAsBool( "g_bg", true ) )
		return;
	
	double bg_dist = Raptor::Game->Cfg.SettingAsDouble( "g_bg_dist", std::min<double>( 50000., Raptor::Game->Gfx.ZFar * 0.875 ) );
	
	Raptor::Game->Gfx.DrawSphere3D( Raptor::Game->Cam.X, Raptor::Game->Cam.Y, Raptor::Game->Cam.Z, bg_dist, 32, Background.CurrentFrame(), Graphics::TEXTURE_MODE_Y_ASIN );
}


void RenderLayer::DrawStars( void )
{
	int star_count = std::min<int>( STAR_COUNT, Raptor::Game->Cfg.SettingAsInt( "g_stars", 0 ) );
	if( ! star_count )
		return;
	
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	glPointSize( 0.5f );
	glDisable( GL_DEPTH_TEST );
	
	glBegin( GL_POINTS );
	
		for( int i = 0; i < star_count; i ++ )
			glVertex3d( Raptor::Game->Cam.X + Stars[ i*3 ], Raptor::Game->Cam.Y + Stars[ i*3 + 1 ], Raptor::Game->Cam.Z + Stars[ i*3 + 2 ] );
	
	glEnd();
	
	/*
	// Disabled because the array doesn't stay centered on the camera.
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_DOUBLE, 0, Stars );
	glDrawArrays( GL_POINTS, 0, star_count );
	glDisableClientState( GL_VERTEX_ARRAY );
	*/
	
	glEnable( GL_DEPTH_TEST );
}


void RenderLayer::DrawDebris( int dynamic_lights, std::list<Shot*> *shots, std::list<Effect*> *effects )
{
	int debris_count = std::min<int>( DEBRIS_COUNT, Raptor::Game->Cfg.SettingAsInt( "g_debris", 500 ) );
	int debris_quality = Raptor::Game->Cfg.SettingAsInt( "g_debris_quality", 3, 1 );
	
	if( debris_quality <= 0 )
		return;
	
	GLuint texture = 0;
	bool use_shaders = (debris_quality >= 2) && Raptor::Game->Cfg.SettingAsBool("g_shader_enable");
	
	if( use_shaders )
	{
		Raptor::Game->ShaderMgr.Select( Raptor::Game->Res.GetShader("model") );
		Raptor::Game->ShaderMgr.ResumeShaders();
		
		// Debris material properties.
		texture = Raptor::Game->Res.GetTexture("grey.jpg");
		Raptor::Game->ShaderMgr.Set3f( "AmbientColor",  0.25,  0.25,  0.25 );
		Raptor::Game->ShaderMgr.Set3f( "DiffuseColor",  0.5,   0.5,   0.5 );
		Raptor::Game->ShaderMgr.Set3f( "SpecularColor", 0.125, 0.125, 0.125 );
		Raptor::Game->ShaderMgr.Set1f( "Alpha", 1. );
		Raptor::Game->ShaderMgr.Set1f( "Shininess", 10. );
		
		// Debris lighting.
		SetWorldLights( ((XWingGame*)( Raptor::Game ))->GameType == XWing::GameType::BATTLE_OF_YAVIN );
		if( (debris_quality >= 3) && dynamic_lights )
			SetDynamicLights( &(Raptor::Game->Cam), NULL, dynamic_lights, shots, effects );
		else
			ClearDynamicLights();
	}
	
	for( int i = 0; i < debris_count; i ++ )
	{
		double dist = Raptor::Game->Cam.Dist( &(Debris[ i ]) );
		if( dist > DEBRIS_DIST )
		{
			Debris[ i ].SetPos( Rand::Double( Raptor::Game->Cam.X - DEBRIS_DIST, Raptor::Game->Cam.X + DEBRIS_DIST ), Rand::Double( Raptor::Game->Cam.Y - DEBRIS_DIST, Raptor::Game->Cam.Y + DEBRIS_DIST ), Rand::Double( Raptor::Game->Cam.Z - DEBRIS_DIST, Raptor::Game->Cam.Z + DEBRIS_DIST ) );
			dist = Raptor::Game->Cam.Dist( &(Debris[ i ]) );
		}
		
		Raptor::Game->Gfx.DrawSphere3D( Debris[ i ].X, Debris[ i ].Y, Debris[ i ].Z, 0.1, 3, texture, 0.5f, 0.5f, 0.5f, 1.f );
	}
	
	if( use_shaders )
	{
		ClearMaterial();
		ClearDynamicLights();
		Raptor::Game->ShaderMgr.StopShaders();
	}
}


class PlayerScore
{
public:
	int Score;
	int Kills;
	int CapitalKills;
	int TurretKills;
	int Deaths;
	Player *PlayerData;
	
	PlayerScore( Player *p )
	{
		PlayerData = p;
		Score = p->PropertyAsInt("score");
		Kills = p->PropertyAsInt("kills");
		CapitalKills = p->PropertyAsInt("kills_c");
		TurretKills = p->PropertyAsInt("kills_t");
		Deaths = p->PropertyAsInt("deaths");
	}
	
	virtual ~PlayerScore(){}
	
	bool operator < ( const PlayerScore &other ) const
	{
		if( Score != other.Score )
			return (Score < other.Score);
		if( CapitalKills != other.CapitalKills )
			return (CapitalKills < other.CapitalKills);
		if( Kills != other.Kills )
			return (Kills < other.Kills);
		if( TurretKills != other.TurretKills )
			return (TurretKills < other.TurretKills);
		if( Deaths != other.Deaths )
			return (Deaths > other.Deaths);
		return (PlayerData->ID > other.PlayerData->ID);
	}
};


void RenderLayer::DrawScores( void )
{
	glPushMatrix();
	float ui_scale = Raptor::Game->UIScale;
	Raptor::Game->Gfx.Setup2D();
	
	std::set<PlayerScore> scores;
	for( std::map<uint16_t,Player*>::iterator player_iter = Raptor::Game->Data.Players.begin(); player_iter != Raptor::Game->Data.Players.end(); player_iter ++ )
	{
		if( (! player_iter->second->PropertyAsString("team").empty()) && (player_iter->second->PropertyAsString("team") != "Spectator") )
			scores.insert( player_iter->second );
	}
	
	double remaining_secs = ((XWingGame*)( Raptor::Game ))->RoundTimer.RemainingSeconds();
	if( (remaining_secs <= 0.) && Raptor::Game->Cfg.SettingAsBool("clock",true) )
		remaining_secs = ((XWingGame*)( Raptor::Game ))->RoundTimer.ElapsedSeconds();
	if( remaining_secs > 0. )
	{
		int minutes = remaining_secs / 60.;
		int seconds = Num::FPart( remaining_secs / 60. ) * 60.;
		char time_string[ 32 ] = "";
		snprintf( time_string, 32, "%i:%02i", minutes, seconds );
		
		BigFont->DrawText( std::string(time_string), Rect.x + Rect.w/2 + 2, Rect.y + 70 * ui_scale, Font::ALIGN_MIDDLE_CENTER, 0.f, 0.f, 0.f, 0.8f, ui_scale );
		BigFont->DrawText( std::string(time_string), Rect.x + Rect.w/2,     Rect.y + 68 * ui_scale, Font::ALIGN_MIDDLE_CENTER, 1.f, 1.f, 1.f, 1.f,  ui_scale );
	}
	
	int x = Rect.x + Rect.w/2;
	int y = Rect.y + 100 * ui_scale;
	
	XWingGame *game = (XWingGame*) Raptor::Game;
	uint32_t gametype = game->GameType;
	bool ffa = (gametype == XWing::GameType::FFA_ELIMINATION) || (gametype == XWing::GameType::FFA_DEATHMATCH) || (gametype == XWing::GameType::FFA_RACE);
	bool objective = (gametype != XWing::GameType::TEAM_ELIMINATION) && (gametype != XWing::GameType::TEAM_DEATHMATCH) && (gametype != XWing::GameType::TEAM_RACE) && (gametype != XWing::GameType::CTF) && ! ffa;
	
	if( ! ffa )
	{
		float rebel_g  = (game->Victor == XWing::Team::REBEL)  ? fabsf(cos( PlayTime.ElapsedSeconds() * M_PI * 2. )) : 0.12f;
		float empire_g = (game->Victor == XWing::Team::EMPIRE) ? fabsf(cos( PlayTime.ElapsedSeconds() * M_PI * 2. )) : 0.37f;
		
		BigFont->DrawText( "Rebels", x - 318 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_LEFT,   0.f,   0.f,      0.f,   0.8f, ui_scale );
		BigFont->DrawText( "Rebels", x - 320 * ui_scale, y,                Font::ALIGN_MIDDLE_LEFT,   1.f,   rebel_g,  0.12f, 1.f,  ui_scale );
		BigFont->DrawText( "vs",     x +   2 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_CENTER, 0.f,   0.f,      0.f,   0.8f, ui_scale );
		BigFont->DrawText( "vs",     x,                  y,                Font::ALIGN_MIDDLE_CENTER, 0.75f, 0.75f,    0.75f, 1.f,  ui_scale );
		BigFont->DrawText( "Empire", x + 322 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT,  0.f,   0.f,      0.f,   0.8f, ui_scale );
		BigFont->DrawText( "Empire", x + 320 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT,  0.25f, empire_g, 1.f,   1.f,  ui_scale );
		
		if( (! objective) && (game->Data.PropertyAsString("gametype") != "mission") )
		{
			double rebel_score  = Raptor::Game->Data.PropertyAsInt("team_score_rebel");
			double empire_score = Raptor::Game->Data.PropertyAsInt("team_score_rebel");
			if( rebel_score >= empire_score )
				empire_g = 0.37f;
			if( empire_score >= rebel_score )
				rebel_g = 0.12f;
			
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("team_score_rebel"),  x - 14 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f,   0.f,      0.f,   0.8f, ui_scale );
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("team_score_rebel"),  x - 16 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, 1.f,   rebel_g,  0.12f, 1.f,  ui_scale );
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("team_score_empire"), x + 18 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_LEFT,  0.f,   0.f,      0.f,   0.8f, ui_scale );
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("team_score_empire"), x + 16 * ui_scale, y,                Font::ALIGN_MIDDLE_LEFT,  0.25f, empire_g, 1.f,   1.f,  ui_scale );
		}
		
		y += (BigFont->GetHeight() - 8) * ui_scale;
		float thickness = std::max<float>( 1.f, ui_scale );
		Raptor::Game->Gfx.DrawLine2D( Raptor::Game->Gfx.W/2 - 318 * ui_scale, y + 2 * ui_scale, Raptor::Game->Gfx.W/2 + 322 * ui_scale, y + 2 * ui_scale, thickness, 0.f, 0.f, 0.f, 0.8f );
		Raptor::Game->Gfx.DrawLine2D( Raptor::Game->Gfx.W/2 - 320 * ui_scale, y,                Raptor::Game->Gfx.W/2 + 320 * ui_scale, y,                thickness, 1.f, 1.f, 1.f, 1.f );
		y += 16 * ui_scale;
	}
	
	SmallFont->DrawText( "Player", x - 318 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f, ui_scale );
	SmallFont->DrawText( "Player", x - 320 * ui_scale, y,                Font::ALIGN_MIDDLE_LEFT, 1.f, 1.f, 1.f, 1.f,  ui_scale );
	if( objective )
	{
		SmallFont->DrawText( "Objective", x +  82 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f, ui_scale );
		SmallFont->DrawText( "Objective", x +  80 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f,  ui_scale );
		SmallFont->DrawText( "Fighters",  x + 162 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f, ui_scale );
		SmallFont->DrawText( "Fighters",  x + 160 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f,  ui_scale );
		SmallFont->DrawText( "Turrets",   x + 242 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f, ui_scale );
		SmallFont->DrawText( "Turrets",   x + 240 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f,  ui_scale );
	}
	else
	{
		if( (gametype == XWing::GameType::TEAM_RACE) || (gametype == XWing::GameType::FFA_RACE) )
		{
			SmallFont->DrawText( "Checkpoints", x + 162 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f, ui_scale );
			SmallFont->DrawText( "Checkpoints", x + 160 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f,  ui_scale );
		}
		else if( gametype == XWing::GameType::CTF )
		{
			SmallFont->DrawText( "Captures", x + 162 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f, ui_scale );
			SmallFont->DrawText( "Captures", x + 160 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f,  ui_scale );
		}
		SmallFont->DrawText( "Kills", x + 242 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f, ui_scale );
		SmallFont->DrawText( "Kills", x + 240 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f,  ui_scale );
	}
	SmallFont->DrawText( "Deaths", x + 322 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f, ui_scale );
	SmallFont->DrawText( "Deaths", x + 320 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f,  ui_scale );
	y += SmallFont->GetHeight() * ui_scale;
	
	for( std::set<PlayerScore>::reverse_iterator score_iter = scores.rbegin(); score_iter != scores.rend(); score_iter ++ )
	{
		float red = 1.f, green = 1.f, blue = 1.f;
		
		if( ! ffa )
		{
			if( score_iter->PlayerData->PropertyAsString("team") == "Rebel" )
			{
				red = 1.f;
				
				if( score_iter->PlayerData->ID == Raptor::Game->PlayerID )
				{
					green = 0.37f;
					blue  = 0.25f;
				}
				else
				{
					green = 0.12f;
					blue  = 0.12f;
				}
			}
			else if( score_iter->PlayerData->PropertyAsString("team") == "Empire" )
			{
				blue = 1.f;
				
				if( score_iter->PlayerData->ID == Raptor::Game->PlayerID )
				{
					red   = 0.f;
					green = 0.62f;
				}
				else
				{
					red   = 0.25f;
					green = 0.37f;
				}
			}
		}
		else if( score_iter->PlayerData->ID == Raptor::Game->PlayerID )
			blue = 0.f;
		
		BigFont->DrawText( score_iter->PlayerData->Name, x - 318 * ui_scale, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f,   ui_scale );
		BigFont->DrawText( score_iter->PlayerData->Name, x - 320 * ui_scale, y,     Font::ALIGN_MIDDLE_LEFT, red, green, blue, 1.f, ui_scale );
		if( objective )
		{
			BigFont->DrawText( Num::ToString(score_iter->CapitalKills), x +  82 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f,   0.f,  0.8f, ui_scale );
			BigFont->DrawText( Num::ToString(score_iter->CapitalKills), x +  80 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f,  ui_scale );
			BigFont->DrawText( Num::ToString(score_iter->Kills),        x + 162 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f,   0.f,  0.8f, ui_scale );
			BigFont->DrawText( Num::ToString(score_iter->Kills),        x + 160 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f,  ui_scale );
			BigFont->DrawText( Num::ToString(score_iter->TurretKills),  x + 242 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f,   0.f,  0.8f, ui_scale );
			BigFont->DrawText( Num::ToString(score_iter->TurretKills),  x + 240 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f,  ui_scale );
		}
		else
		{
			if( (gametype == XWing::GameType::TEAM_RACE) || (gametype == XWing::GameType::FFA_RACE) || (gametype == XWing::GameType::CTF) )
			{
				BigFont->DrawText( Num::ToString(score_iter->Score), x + 162 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f,   0.f,  0.8f, ui_scale );
				BigFont->DrawText( Num::ToString(score_iter->Score), x + 160 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f,  ui_scale );
			}
			BigFont->DrawText( Num::ToString(score_iter->Kills), x + 242 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f,   0.f,  0.8f, ui_scale );
			BigFont->DrawText( Num::ToString(score_iter->Kills), x + 240 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f,  ui_scale );
		}
		BigFont->DrawText( Num::ToString(score_iter->Deaths), x + 322 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f,   0.f,  0.8f, ui_scale );
		BigFont->DrawText( Num::ToString(score_iter->Deaths), x + 320 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f,  ui_scale );
		
		y += BigFont->GetLineSkip() * ui_scale;
	}
	
	if( Raptor::Game->Data.PropertyAsString("ai_score_kills").length() )
	{
		// FIXME: Should this appear in sorted order with the player scores?
		BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_name"), x - 318 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_LEFT, 0.f,  0.f,  0.f,  0.8f, ui_scale );
		BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_name"), x - 320 * ui_scale, y,                Font::ALIGN_MIDDLE_LEFT, 0.8f, 0.8f, 0.8f, 1.f,  ui_scale );
		if( (gametype == XWing::GameType::TEAM_RACE) || (gametype == XWing::GameType::FFA_RACE) )
		{
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_kills"), x + 162 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f,  0.f,  0.f,  0.8f, ui_scale );
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_kills"), x + 160 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, 0.8f, 0.8f, 0.8f, 1.f,  ui_scale );
		}
		else
		{
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_kills"), x + 242 * ui_scale, y + 2 * ui_scale, Font::ALIGN_MIDDLE_RIGHT, 0.f,  0.f,  0.f,  0.8f, ui_scale );
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_kills"), x + 240 * ui_scale, y,                Font::ALIGN_MIDDLE_RIGHT, 0.8f, 0.8f, 0.8f, 1.f,  ui_scale );
		}
	}
	
	glPopMatrix();
}


void RenderLayer::DrawVoice( void )
{
	XWingGame *game = (XWingGame*) Raptor::Game;
	int voice_y = Rect.h;
	
	for( std::map<uint16_t,Player*>::const_iterator player_iter = game->Data.Players.begin(); player_iter != game->Data.Players.end(); player_iter ++ )
	{
		uint8_t voice_channel = player_iter->second->VoiceChannel();
		if( voice_channel )
		{
			std::string display_name = player_iter->second->Name;
			Color display_color;
			
			std::string team = player_iter->second->PropertyAsString("team");
			if( team.empty() )
			{
				team = game->Data.PropertyAsString("player_team");
				if( team.length() )
					team[ 0 ] = toupper( team[ 0 ] );
			}
			
			if( ! team.empty() )
				display_name += std::string(" [") + team + std::string("]");
			
			if( voice_channel == Raptor::VoiceChannel::TEAM )
			{
				display_color.Green = (player_iter->first == game->PlayerID) ? 1.f  : 0.875f;
				display_color.Red   = (player_iter->first == game->PlayerID) ? 0.5f : 0.f;
				display_color.Blue  = 0.f;
			}
			else if( Str::EqualsInsensitive( team, "Rebel" ) )
			{
				display_color.Red = 1.f;
				if( player_iter->first == game->PlayerID )
				{
					display_color.Green = 0.37f;
					display_color.Blue  = 0.25f;
				}
				else
				{
					display_color.Green = 0.12f;
					display_color.Blue  = 0.12f;
				}
			}
			else if( Str::EqualsInsensitive( team, "Empire" ) )
			{
				display_color.Blue = 1.f;
				if( player_iter->first == game->PlayerID )
				{
					display_color.Red   = 0.f;
					display_color.Green = 0.62f;
				}
				else
				{
					display_color.Red   = 0.25f;
					display_color.Green = 0.37f;
				}
			}
			else if( player_iter->first == game->PlayerID )
				display_color.Blue = 0.f;
			
			if( voice_y == Rect.h )
			{
				glPushMatrix();
				game->Gfx.Setup2D();
			}
			
			float ui_scale = Raptor::Game->UIScale;
			int font_ascent = BigFont->GetAscent() * ui_scale;
			GLuint mic_texture = Mic.CurrentFrame();
			
			game->Gfx.DrawRect2D( Rect.x + Rect.w - 1 - font_ascent, Rect.y + voice_y - 1 - font_ascent, Rect.x + Rect.w - 1, Rect.y + voice_y - 1, mic_texture, 0.f,0.f,0.f,0.8f );
			game->Gfx.DrawRect2D( Rect.x + Rect.w - 3 - font_ascent, Rect.y + voice_y - 3 - font_ascent, Rect.x + Rect.w - 3, Rect.y + voice_y - 3, mic_texture, display_color.Red,display_color.Green,display_color.Blue,display_color.Alpha );
			
			BigFont->DrawText( display_name, Rect.x + Rect.w - 3 - font_ascent, Rect.y + voice_y - 1, Font::ALIGN_BOTTOM_RIGHT, 0.f,0.f,0.f,0.8f,                                                             ui_scale );
			BigFont->DrawText( display_name, Rect.x + Rect.w - 5 - font_ascent, Rect.y + voice_y - 3, Font::ALIGN_BOTTOM_RIGHT, display_color.Red,display_color.Green,display_color.Blue,display_color.Alpha, ui_scale );
			
			voice_y -= font_ascent + 4 * ui_scale;
		}
	}
	
	if( voice_y < Rect.h )  // If anyone is talking we called glPushMatrix.
		glPopMatrix();
}


#ifdef WIN32
void RenderLayer::UpdateSaitek( const Ship *ship, bool is_player )
{
	if( ! Raptor::Game->Cfg.SettingAsBool("saitek_enable") )
		return;
	
	if( ship && (ship->Health > 0.) && is_player )
	{
		GameObject *target_obj = NULL;
		Ship *target = NULL;
		if( ship->Target )
		{
			target_obj = Raptor::Game->Data.GetObject( ship->Target );
			if( target_obj && (target_obj->Type() == XWing::Object::SHIP) )
				target = (Ship*) target_obj;
		}
		
		bool locked_on = (ship->TargetLock >= 1.f) && ship->LockingOn( target_obj );
		bool joy_r = false, joy_g = true;
		if( (ship->SelectedWeapon == Shot::TYPE_TORPEDO) || (ship->SelectedWeapon == Shot::TYPE_MISSILE) )
		{
			joy_r = true;
			joy_g = ! locked_on;
		}
		
		// See if a missile/torpedo is already seeking us or if an enemy is locking on.
		bool incoming = false, attacker = false, attacker_firing = false;
		float enemy_lock = 0.f;
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
		{
			if( (obj_iter->second->Type() == XWing::Object::SHOT) && (((const Shot*)( obj_iter->second ))->Seeking == ship->ID) )
				incoming = true;
			else if( (obj_iter->second->Type() == XWing::Object::SHIP) && (((const Ship*)( obj_iter->second ))->Health > 0.) && (((const Ship*)( obj_iter->second ))->Target == ship->ID) )
			{
				const Ship *enemy = (const Ship*) obj_iter->second;
				if( enemy->TargetLock > enemy_lock )
					enemy_lock = enemy->TargetLock;
				if( enemy->Category() != ShipClass::CATEGORY_CAPITAL )
				{
					attacker = true;
					if( enemy->Firing ) // FIXME: Won't work until we add Firing to Ship UpdatePacketFromServer.
						attacker_firing = true;
				}
			}
		}
		
		bool incoming_g = true, incoming_r = false;
		if( incoming )
		{
			incoming_g = false;
			incoming_r = ((int) ship->Lifetime.ElapsedMilliseconds() % 200 <= 50);
		}
		else if( enemy_lock )
			incoming_r = incoming_g = ((int) ship->Lifetime.ElapsedMilliseconds() % ((enemy_lock >= 1.f) ? 200 : 600) <= 50);
		
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::Fire, (ship->AmmoForWeapon() || (fmod( ship->Lifetime.ElapsedSeconds(), 0.5 ) < 0.25)) );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::Hat2Green, joy_g );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::AGreen, joy_g );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::BGreen, joy_g );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::Hat2Red, joy_r );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::ARed, joy_r );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::BRed, joy_r );
		
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::T1Green, true );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::T3Green, true );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::T5Green, true );
		
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::ERed, attacker );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::EGreen, ! attacker_firing );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::DRed, (ship->ShieldPos != Ship::SHIELD_CENTER) );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::DGreen, (ship->ShieldPos != Ship::SHIELD_FRONT) );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::ClutchRed, incoming_r );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::ClutchGreen, incoming_g );
		
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::Throttle, true );
		
		
		char text_buffer[ 32 ] = "";
		
		memset( text_buffer, 0, 32 );
		if( ship->MaxShield() )
			snprintf( text_buffer, 17, "Shld: %3i%%/%3i%%", (int)( ship->ShieldF / ship->MaxShield() * 100. + 0.5 ), (int)( ship->ShieldR / ship->MaxShield() * 100. + 0.5 ) );
		else if( ship->MaxHealth() )
			snprintf( text_buffer, 17, "Hull: %3i%%", (int)( ship->Health / ship->MaxHealth() * 100. + 0.5 ) );
		Raptor::Game->Saitek.SetX52ProMFD( 0, text_buffer );
		
		memset( text_buffer, 0, 32 );
		snprintf( text_buffer, 17, "Throttle: %3i%%", (int)( ship->GetThrottle() * 100. + 0.5 ) );
		Raptor::Game->Saitek.SetX52ProMFD( 1, text_buffer );
		
		memset( text_buffer, 0, 32 );
		if( target )
		{
			const Player *owner = target->Owner();
			snprintf( text_buffer, 17, "%-16.16s", (owner ? owner->Name : target->Name).c_str() );
			double dist = ship->Dist( target );
			if( locked_on )
				snprintf( text_buffer + 12, 5, "LOCK" );
			else if( dist >= 98950. )
				snprintf( text_buffer + 12, 5, ">99K" );
			else if( dist >= 9950. )
				snprintf( text_buffer + 11, 6, "%4.1fK", dist / 1000. );
			else if( dist >= 999.5 )
				snprintf( text_buffer + 12, 5, "%3.1fK", dist / 1000. );
			else
				snprintf( text_buffer + 13, 4, "%3.0f", dist );
		}
		Raptor::Game->Saitek.SetX52ProMFD( 2, text_buffer );
		
		
		if( Raptor::Game->Gfx.Framebuffers )
		{
			Framebuffer *fb = Raptor::Game->Res.GetFramebuffer( "fip", 320, 240 );
			if( fb && fb->Select() )
			{
				fb->Setup2D( 0, 240, 320, 0 );
				glColor4f( 1.f, 1.f, 1.f, 1.f );
				
				Raptor::Game->Gfx.Clear();
				Raptor::Game->Gfx.DrawRect2D( 0, 0, 320, 240, Raptor::Game->Res.GetTexture("*intercept") );
				Raptor::Game->Saitek.SetFIPImage( fb, 0 );
				
				Raptor::Game->Gfx.Clear();
				Raptor::Game->Gfx.DrawRect2D( 120, 0, 300, 240, Raptor::Game->Res.GetTexture("*target") );
				Raptor::Game->Gfx.DrawRect2D( 0, 40, 120, 200, Raptor::Game->Res.GetTexture("*health") );
				Raptor::Game->Gfx.DrawRect2D( 300, 0, 320, 240, Raptor::Game->Res.GetTexture("*throttle") );
				Raptor::Game->Saitek.SetFIPImage( fb, 1 );
				
				Raptor::Game->Gfx.SelectDefaultFramebuffer();
			}
		}
	}
	else
	{
		for( int i = 0; i < 20; i ++ )
			Raptor::Game->Saitek.SetX52ProLED( i, false );
		
		for( int i = 0; i < 3; i ++ )
			Raptor::Game->Saitek.SetX52ProMFD( i, "" );
		
		Raptor::Game->Saitek.ClearFIPImage();
	}
}
#endif


bool RenderLayer::ControlDown( uint8_t control )
{
	if( MessageInput->IsSelected() )
		return false;
	
	XWingGame *game = (XWingGame*) Raptor::Game;
	
	if( ((control == XWing::Control::CHAT) || (control == XWing::Control::CHAT_TEAM)) && IsTop() )
	{
		game->ReadKeyboard = false;
		
		Selected = MessageInput;
		MessageInput->Visible = true;
		
		if( control == XWing::Control::CHAT_TEAM )
		{
			MessageInput->SelectedBlue = 0.f;
			MessageInput->SelectedGreen = 0.5f;
		}
		else
		{
			MessageInput->SelectedBlue = 1.f;
			MessageInput->SelectedGreen = 0.f;
		}
		
		return true;
	}
	else if( control == XWing::Control::MENU )
	{
		Raptor::Game->Mouse.ShowCursor = true;
		game->ReadMouse = false;
		IngameMenu *ingame_menu = new IngameMenu();
		Raptor::Game->Layers.Add( ingame_menu );
		ingame_menu->Paused = (Raptor::Game->Data.TimeScale < 0.0000011) && Raptor::Server->IsRunning() && Raptor::Game->Cfg.SettingAsBool("ui_pause",true);
		
		return true;
	}
	else if( control == XWing::Control::PREFS )
	{
		Raptor::Game->Mouse.ShowCursor = true;
		game->ReadMouse = false;
		Raptor::Game->Layers.Add( new PrefsMenu() );
		
		return true;
	}

	return false;
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
				message.AddString( ((player ? player->Name : std::string("Anonymous")) + Raptor::Game->ChatSeparator + msg).c_str() );
				message.AddUInt( (MessageInput->SelectedGreen > MessageInput->SelectedBlue) ? TextConsole::MSG_TEAM : TextConsole::MSG_CHAT );  // FIXME: This feels dirty.
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
	
	return false;
}


// -----------------------------------------------------------------------------


Renderable::Renderable( Ship *ship )
{
	ShipPtr = ship;
	ShotPtr = NULL;
	EffectPtr = NULL;
	EnginePtr = NULL;
	EngineAlpha = 0.f;
	EngineScale = 1.;
}

Renderable::Renderable( Shot *shot )
{
	ShipPtr = NULL;
	ShotPtr = shot;
	EffectPtr = NULL;
	EnginePtr = NULL;
	EngineAlpha = 0.f;
	EngineScale = 1.;
}

Renderable::Renderable( Effect *effect )
{
	ShipPtr = NULL;
	ShotPtr = NULL;
	EffectPtr = effect;
	EnginePtr = NULL;
	EngineAlpha = 0.f;
	EngineScale = 1.;
}

Renderable::Renderable( ShipEngine *engine, const Pos3D *pos, float alpha, double scale )
{
	ShipPtr = NULL;
	ShotPtr = NULL;
	EffectPtr = NULL;
	EnginePtr = engine;
	EnginePos.Copy( pos );
	EngineAlpha = alpha;
	EngineScale = scale;
}

Renderable::~Renderable()
{
}
