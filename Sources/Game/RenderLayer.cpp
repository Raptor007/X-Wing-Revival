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


enum
{
	VIEW_AUTO = 0,
	VIEW_COCKPIT,
	VIEW_GUNNER,
	VIEW_CHASE,
	VIEW_CINEMA,
	VIEW_FIXED,
	VIEW_STATIONARY,
	VIEW_INSTRUMENTS,
	VIEW_CYCLE
};


RenderLayer::RenderLayer( void )
{
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	BigFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 21 );
	SmallFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 15 );
	RadarDirectionFont = Raptor::Game->Res.GetFont( "ProFont.ttf", 22 );
	ScreenFont = Raptor::Game->Res.GetFont( "ProFont.ttf", 48 );
	
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
		ambient.Set( 0.75f, 0.75f, 0.75f, 1.f );
		
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
	else
	{
		ambient.Set( 0.75f, 0.75f, 0.75f, 1.f );
		
		dir[ 0 ].Set( 0.07, -0.8, 0.6 );
		dir[ 0 ].ScaleTo( 1. );
		color[ 0 ].Set( 1.3, 1.3, 1.29, 1.f );
		wrap_around[ 0 ] = 0.5;
		
		dir[ 1 ].Set( 0.9, 0.3, 0.25 );
		dir[ 1 ].ScaleTo( 1. );
		color[ 1 ].Set( 0.22, 0.2, 0.2, 1.f );
		wrap_around[ 1 ] = 1.0;
		
		dir[ 2 ].Set( -0.01, -0.86, -0.51 );
		dir[ 2 ].ScaleTo( 1. );
		color[ 2 ].Set( 0.2, 0.15, 0.3, 1.f );
		wrap_around[ 2 ] = 0.5;
		
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


void RenderLayer::ClearWorldLights( void )
{
	Raptor::Game->ShaderMgr.Set3f( "AmbientLight", 1.0, 1.0, 1.0 );
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


void RenderLayer::Draw( void )
{
	glPushMatrix();
	
	
	// Update the 2D drawable area.
	
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	bool vr = Raptor::Game->Head.VR && Raptor::Game->Gfx.DrawTo;
	if( vr )
	{
		Rect.x = Raptor::Game->Gfx.W/2 - 640/2;
		Rect.y = Raptor::Game->Gfx.H/2 - 480/2;
		Rect.w = 640;
		Rect.h = 480;
	}
	
	bool screensaver = Raptor::Game->Cfg.SettingAsBool("screensaver") || Raptor::Game->Cfg.SettingAsBool("cinematic");
	
	
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
		
		if( vr )
		{
			MessageInput->Rect.w /= 2;
			MessageInput->Rect.x += MessageInput->Rect.w / 2;
		}
	}
	
	if( IsTop() )
	{
		Raptor::Game->Mouse.ShowCursor = false;
		((XWingGame*)( Raptor::Game ))->ReadMouse = (Raptor::Game->Cfg.SettingAsString("mouse_mode") != "disabled");
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
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
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
			if( obj_iter->second->PlayerID == Raptor::Game->PlayerID )
				player_turret = (Turret*) obj_iter->second;
		}
	}
	
	std::list<Effect*> effects;
	for( std::list<Effect>::iterator effect_iter = Raptor::Game->Data.Effects.begin(); effect_iter != Raptor::Game->Data.Effects.end(); effect_iter ++ )
	{
		if( effect_iter->Lifetime.Progress() >= 1. )  // Don't use effects that haven't started yet as light sources.
			effects.push_back( &*effect_iter );
	}
	
	
	// Determine which ship we're observing, and in what view.
	
	Ship *observed_ship = NULL;
	Ship *player_ship = NULL;
	Turret *observed_turret = NULL;
	int view = VIEW_AUTO;
	
	if( player_turret )
	{
		// Player is controlling a turret.
		
		observed_turret = player_turret;
		player_ship = player_turret->ParentShip();
		observed_ship = player_ship;
		view = VIEW_GUNNER;
	}
	else
	{
		// Look for the player's ship.
		
		for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
		{
			if( (*ship_iter)->PlayerID == Raptor::Game->PlayerID )
			{
				player_ship = *ship_iter;
				
				// Only observe the player's ship if alive or recently dead.
				if( (player_ship->Health > 0.) || (player_ship->DeathClock.ElapsedSeconds() < 6.) )
				{
					observed_ship = player_ship;
					view = VIEW_COCKPIT;
				}
				break;
			}
		}
	}
	
	
	// Determine the selected view.
	
	std::string view_str = Raptor::Game->Cfg.SettingAsString( ((view == VIEW_COCKPIT) || (view == VIEW_GUNNER)) ? "view" : "spectator_view" );
	if( view_str == "cockpit" )
		view = VIEW_COCKPIT;
	else if( view_str == "gunner" )
		view = VIEW_GUNNER;
	else if( view_str == "crosshair" )
		view = observed_turret ? VIEW_GUNNER : VIEW_COCKPIT;
	else if( view_str == "chase" )
		view = VIEW_CHASE;
	else if( Str::BeginsWith( view_str, "cinema" ) )
		view = VIEW_CINEMA;
	else if( view_str == "fixed" )
		view = VIEW_FIXED;
	else if( view_str == "stationary" )
		view = VIEW_STATIONARY;
	else if( view_str == "instruments" )
		view = VIEW_INSTRUMENTS;
	else if( view_str == "cycle" )
		view = VIEW_CYCLE;
	
	if( ! (observed_ship || observed_turret) )
	{
		// This player has no ship or turret alive; let's watch somebody else.
		// FIXME: Refactor the huge mess below into a single loop to find the best ship to observe.
		
		Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
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
		
		// First try to observe a specific ship ID (who we were watching before).
		if( ((XWingGame*)( Raptor::Game ))->ObservedShipID )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port or any long-dead ships.
				if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
					continue;
				if( ((*ship_iter)->Health <= 0.) && ((*ship_iter)->DeathClock.ElapsedSeconds() >= 6.) && (view != VIEW_INSTRUMENTS) )
					continue;
				
				// If we'd selected a specific ship to watch, keep going until we find it.
				if( (*ship_iter)->ID == ((XWingGame*)( Raptor::Game ))->ObservedShipID )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// Try to observe a player-controlled ship in the player's flight group.
		if( (! observed_ship) && player_team && player_group )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port or any long-dead ships.
				if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
					continue;
				if( ((*ship_iter)->Health <= 0.) && ((*ship_iter)->DeathClock.ElapsedSeconds() >= 6.) && (view != VIEW_INSTRUMENTS) )
					continue;
				
				// If our flight group has other player-controlled ships, watch them.
				if( ((*ship_iter)->Team == player_team) && ((*ship_iter)->Group == player_group) && (*ship_iter)->Owner() )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// Try to observe anyone in the player's flight group.
		if( (! observed_ship) && player_team && player_group )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port or any long-dead ships.
				if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
					continue;
				if( ((*ship_iter)->Health <= 0.) && ((*ship_iter)->DeathClock.ElapsedSeconds() >= 6.) && (view != VIEW_INSTRUMENTS) )
					continue;
				
				// If our flight group has other ships, watch them.
				if( ((*ship_iter)->Team == player_team) && ((*ship_iter)->Group == player_group) )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// Try to observe the next ship alive on the player's team after the ID we were observing.
		if( (! observed_ship) && player_team )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
					continue;
				
				// Don't start observing dead ships.
				if( (*ship_iter)->Health <= 0. )
					continue;
				
				// If we'd selected a specific ship to watch, keep going until we find it.
				if( ((*ship_iter)->Team == player_team) && ((*ship_iter)->ID >= ((XWingGame*)( Raptor::Game ))->ObservedShipID) )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// Try to observe anyone alive on the player's team.
		if( (! observed_ship) && player_team )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
					continue;
				
				// Don't start observing dead ships.
				if( (*ship_iter)->Health <= 0. )
					continue;
				
				// If our team has other ships, watch them.
				if( (*ship_iter)->Team == player_team )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// Try to observe anyone alive or recently dead on the player's team.
		if( (! observed_ship) && player_team )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port or any long-dead ships.
				if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
					continue;
				if( ((*ship_iter)->Health <= 0.) && ((*ship_iter)->DeathClock.ElapsedSeconds() >= 6.) && (view != VIEW_INSTRUMENTS) )
					continue;
				
				// If our team has other ships, watch them.
				if( (*ship_iter)->Team == player_team )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// No teammates to watch; try to observe the next non-capital ship alive after a specific ship ID.
		if( ! observed_ship )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
					continue;
				
				// Don't observe dead ships.
				if( (*ship_iter)->Health <= 0. )
					continue;
				
				// If we'd selected a specific ship to watch, keep going until we find it.
				if( ((*ship_iter)->ID >= ((XWingGame*)( Raptor::Game ))->ObservedShipID) && (*ship_iter)->PlayersCanFly() )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// Observe anybody alive that isn't a capital ship.
		if( ! observed_ship )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
					continue;
				
				// Don't observe dead ships.
				if( (*ship_iter)->Health <= 0. )
					continue;
				
				// If this is not a capital ship, observe it.
				if( (*ship_iter)->PlayersCanFly() )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// Not too picky now, observe anybody alive.
		if( ! observed_ship )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
					continue;
				
				// Don't observe dead ships.
				if( (*ship_iter)->Health <= 0. )
					continue;
				
				observed_ship = *ship_iter;
				break;
			}
		}
		
		// Last ditch effort, observe anybody recently dead.
		if( ! observed_ship )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->Category() == ShipClass::CATEGORY_TARGET )
					continue;
				
				// Don't observe long-dead ships.
				if( ((*ship_iter)->Health <= 0.) && ((*ship_iter)->DeathClock.ElapsedSeconds() >= 6.) && (view != VIEW_INSTRUMENTS) )
					continue;
				
				observed_ship = *ship_iter;
				break;
			}
		}
	}
	
	
	if( view == VIEW_CYCLE )
	{
		double cycle_time = Raptor::Game->Cfg.SettingAsDouble("view_cycle_time",7.);
		if( cycle_time <= 0. )
			cycle_time = 7.;
		
		double cam_picker = observed_ship ? fmod( observed_ship->Lifetime.ElapsedSeconds(), cycle_time * 5. * Raptor::Game->Data.TimeScale ) : fmod( PlayTime.ElapsedSeconds(), cycle_time * 5. );
		if( cam_picker < cycle_time )
			view = VIEW_CINEMA;
		else if( cam_picker < cycle_time * 2. )
			view = (observed_ship && (observed_ship->Category() == ShipClass::CATEGORY_CAPITAL) && observed_ship->AttachedTurret()) ? VIEW_GUNNER : VIEW_COCKPIT;
		else if( cam_picker < cycle_time * 3. )
			view = (observed_ship && observed_ship->Category() == ShipClass::CATEGORY_CAPITAL) ? VIEW_CINEMA : VIEW_CHASE;
		else if( cam_picker < cycle_time * 4. )
			view = VIEW_FIXED;
		else
			view = (observed_ship && observed_ship->AttachedTurret()) ? VIEW_GUNNER : VIEW_CHASE;
	}
	
	
	if( (view == VIEW_GUNNER) && ! player_turret )
	{
		// Spectating from gunner view.  Find a turret to watch.
		
		const Player *me = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
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
			
			if( parent_id == ((XWingGame*)( Raptor::Game ))->ObservedShipID )
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
		||  (player_ship && (observed_turret->ParentShip() != player_ship)) )
			observed_turret = NULL;
		
		if( observed_turret )
			observed_ship = observed_turret->ParentShip();
		else
			view = observed_ship ? VIEW_CHASE : VIEW_AUTO;
	}
	
	
	// Determine which view we'll actually render.
	
	if( view == VIEW_AUTO )
		view = vr ? VIEW_FIXED : VIEW_CINEMA;
	
	if( (view == VIEW_COCKPIT) && ((! observed_ship) || (observed_ship->Health <= 0.)) )
		view = (observed_turret && ! observed_ship) ? VIEW_GUNNER : VIEW_CHASE;
	else if( (view == VIEW_GUNNER) && ! observed_turret )
		view = VIEW_CHASE;
	
	if( view == VIEW_STATIONARY )
	{
		observed_ship = NULL;
		observed_turret = NULL;
	}
	
	
	Player *observed_player = NULL;
	GameObject *target_obj = NULL;
	Ship *target = NULL;
	Ship *dead_target = NULL;
	double jump_progress = 2.;
	
	
	if( observed_ship )
	{
		((XWingGame*)( Raptor::Game ))->ObservedShipID = observed_ship->ID;
		
		jump_progress = observed_ship->JumpProgress;
		
		if( observed_ship->Target )
		{
			target_obj = Raptor::Game->Data.GetObject( observed_ship->Target );
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
		if( view == VIEW_CINEMA )
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
				view = VIEW_FIXED;
		}
		
		
		if( view == VIEW_COCKPIT )
			Cam = observed_ship->HeadPos();
		else if( (view == VIEW_GUNNER) && observed_turret )
			Cam = observed_turret->HeadPos();
		else if( view == VIEW_CHASE )
		{
			Cam.Copy( &(observed_ship->PrevPos) );
			Cam.SetPos( observed_ship->X, observed_ship->Y, observed_ship->Z );
		}
		else
			Cam.Copy( observed_ship );
		
		if( view == VIEW_FIXED )
		{
			// Fixed-angle external camera.
			Cam.Fwd.Set( 1., 0., 0. );
			Cam.Up.Set( 0., 0., 1. );
			Cam.FixVectors();
		}
		
		// Apply camera angle change.
		Cam.Yaw( ((XWingGame*)( Raptor::Game ))->LookYaw );
		Cam.Pitch( ((XWingGame*)( Raptor::Game ))->LookPitch );
		
		if( view == VIEW_CINEMA )
		{
			// Cinematic camera.
			
			// Point the camera at one ship looking through to the other.
			Cam.Up.Set( 0., 0., 1. );
			Vec3D vec_to_other( cinema_view_with->X - observed_ship->X, cinema_view_with->Y - observed_ship->Y, cinema_view_with->Z - observed_ship->Z );
			vec_to_other.ScaleTo( 1. );
			Cam.Fwd = vec_to_other;
			Cam.FixVectors();
			
			// Apply camera angle change.
			Cam.Yaw( ((XWingGame*)( Raptor::Game ))->LookYaw );
			Cam.Pitch( ((XWingGame*)( Raptor::Game ))->LookPitch );
			
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
					if( (*ship_iter)->Category() != ShipClass::CATEGORY_CAPITAL )
						continue;
					double dist = Cam.Dist(*ship_iter);
					if( dist < (*ship_iter)->Radius() )
						Cam.MoveAlong( &(Cam.Fwd), dist - (*ship_iter)->Radius() );
				}
			}
			
			// Apply camera angle change again.
			Cam.Yaw( ((XWingGame*)( Raptor::Game ))->LookYaw );
			Cam.Pitch( ((XWingGame*)( Raptor::Game ))->LookPitch );
		}
		
		else if( (view == VIEW_CHASE) || (view == VIEW_FIXED) )
		{
			// Move camera back from the ship, moreso when dead or jumping in from hyperspace.
			double camera_dist = -20. - observed_ship->Shape.GetTriagonal();
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
					if( (*ship_iter)->Category() != ShipClass::CATEGORY_CAPITAL )
						continue;
					double dist = Cam.Dist(*ship_iter);
					if( dist < (*ship_iter)->Radius() )
						Cam.MoveAlong( &(Cam.Fwd), dist - (*ship_iter)->Radius() );
				}
			}
		}
	}
	else
	{
		((XWingGame*)( Raptor::Game ))->ObservedShipID = 0;
		
		if( observed_turret )
		{
			// Viewing a Death Star surface turret.
			Cam = observed_turret->HeadPos();
			view = VIEW_GUNNER;
		}
	}
	
	// Determine which player we are observing.
	if( observed_turret )
		observed_player = observed_turret->Owner();
	if( observed_ship && ! observed_player )
		observed_player = observed_ship->Owner();
	
	// Move the first person view while jumping in from hyperspace.
	if( observed_ship && (jump_progress < 1.) && (view == VIEW_COCKPIT) )
		Cam.MoveAlong( &(observed_ship->Fwd), observed_ship->CockpitDrawOffset() );
	else if( observed_ship && (jump_progress < 1.) && (view == VIEW_GUNNER) && (observed_ship == observed_turret->ParentShip()) )
		Cam.MoveAlong( &(observed_ship->Fwd), observed_ship->DrawOffset() );
	
	// This allows head tracking to happen after camera placement.
	Raptor::Game->Cam.Copy( &Cam );
	
	
	// When viewing from turret, show its selected target instead of the ship's.
	if( observed_turret && (view == VIEW_GUNNER) )
	{
		target_obj = Raptor::Game->Data.GetObject( observed_turret->Target );
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
	
	bool use_shaders = Raptor::Game->Cfg.SettingAsBool("g_shader_enable");
	if( use_shaders )
	{
		Raptor::Game->ShaderMgr.ResumeShaders();
		Raptor::Game->ShaderMgr.Set3f( "CamPos", Raptor::Game->Cam.X, Raptor::Game->Cam.Y, Raptor::Game->Cam.Z );
		SetWorldLights( deathstar );
		ClearDynamicLights();
		Raptor::Game->ShaderMgr.StopShaders();
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
		
		if( (enemy_lock >= 1.f) && (observed_ship->PlayerID == Raptor::Game->PlayerID) )
		{
			static Clock lock_warn;
			if( (lock_warn.Progress() >= 1.) && ! incoming )
				Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("lock_warn.wav") );
			lock_warn.Reset( 2. );
		}
	}
	
	
	// Render to textures before drawing anything else.
	
	bool need_target_holo = Raptor::Game->FrameTime || (observed_turret && (view == VIEW_GUNNER));
	if( Raptor::Game->Gfx.Framebuffers && Raptor::Game->FrameTime )
	{
		bool changed_framebuffer = false;
		double display_noise = (observed_ship && (observed_ship->Health < (observed_ship->MaxHealth() * 0.5))) ? Raptor::Game->Cfg.SettingAsDouble("g_display_noise",1.) : 0.;
		
		if( observed_ship && (observed_ship->Health > 0.) && ((view == VIEW_COCKPIT) || (view == VIEW_INSTRUMENTS) || Raptor::Game->Cfg.SettingAsBool("saitek_enable")) )
		{
			Framebuffer *health_framebuffer = Raptor::Game->Res.GetFramebuffer( "health" );
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
				
				Raptor::Game->Gfx.Clear( r, g, b );
				
				
				// Render shield display.
				
				health_framebuffer->Setup2D();
				
				GLuint shield_texture = Raptor::Game->Res.GetAnimation("shield.ani")->CurrentFrame();
				
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
					
					if( recent_hit && ! observed_ship->HitRear )
					{
						red   = std::max<float>( red,   recent_hit );
						green = std::max<float>( green, recent_hit );
						blue  = std::max<float>( blue,  recent_hit );
					}
					
					float alpha = std::min<float>( shield_f / 10., jump_progress - 1.3 );
					if( alpha < recent_hit )
						alpha = recent_hit;
					if( alpha < 1.f )
					{
						red   *= alpha;
						green *= alpha;
						blue  *= alpha;
					}
					
					Raptor::Game->Gfx.DrawRect2D( 0, 0, health_framebuffer->W, health_framebuffer->H / 2, shield_texture, red, green, blue, 1.f );
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
					
					if( recent_hit && observed_ship->HitRear )
					{
						red   = std::max<float>( red,   recent_hit );
						green = std::max<float>( green, recent_hit );
						blue  = std::max<float>( blue,  recent_hit );
					}
					
					float alpha = std::min<float>( shield_r / 10., jump_progress - 1.3 );
					if( alpha < recent_hit )
						alpha = recent_hit;
					if( alpha < 1.f )
					{
						red   *= alpha;
						green *= alpha;
						blue  *= alpha;
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
				if( observed_ship->Class && Str::BeginsWith( observed_ship->Class->ShortName, "T/I" ) ) // FIXME: Base this on something like Shape.MaxFwd/MinFwd!
					above_cam.MoveAlong( &(observed_ship->Fwd), ship_size * 0.2 );
				above_cam.FOV = (max_shield > 0.) ? 33. : 20.;
				
				health_framebuffer->Setup3D( &(above_cam) );
				
				float red = 0.f, green = 1.f, blue = 0.f;
				if( observed_ship->Health < observed_ship->MaxHealth() )
					red = 1.f;
				if( observed_ship->Health < (observed_ship->MaxHealth() * 0.7) )
					green = 0.f;
				
				if( recent_hit && (observed_ship->HitRear ? (shield_r <= 0.) : (shield_f <= 0.)) )
				{
					red   = std::max<float>( red,   recent_hit );
					green = std::max<float>( green, recent_hit );
					blue  = std::max<float>( blue,  recent_hit );
				}
				
				glColor4f( red, green, blue, 1.f );
				
				if( use_shaders )
				{
					Raptor::Game->ShaderMgr.Select( Raptor::Game->Res.GetShader("model_hud") );
					Raptor::Game->ShaderMgr.ResumeShaders();
					Raptor::Game->ShaderMgr.Set3f( "AmbientLight", red, green, blue );
				}
				
				observed_ship->Draw();
				
				if( use_shaders )
				{
					Raptor::Game->ShaderMgr.StopShaders();
					Raptor::Game->ShaderMgr.Select( Raptor::Game->Res.GetShader("model") );
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
				
				
				// Fade screens in when jumping in from hyperspace.
				
				if( jump_progress < 1.3 )
				{
					health_framebuffer->Setup2D( 0., 0., 1., 1. );
					Raptor::Game->Gfx.DrawRect2D( 0., 0., 1., 1., 0, 0.f,0.f,0.f,1.f );
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
			
			
			Framebuffer *target_framebuffer = Raptor::Game->Res.GetFramebuffer( "target" );
			if( target_framebuffer && target_framebuffer->Select() )
			{
				// Render target display.
				
				changed_framebuffer = true;
				if( ((view != VIEW_GUNNER) || ! observed_turret) && (observed_ship->Category() != ShipClass::CATEGORY_CAPITAL) && (observed_ship->Category() != ShipClass::CATEGORY_TARGET) )
					need_target_holo = false;
				Raptor::Game->Gfx.Clear();
				
				double eject_held = ((XWingGame*)( Raptor::Game ))->EjectHeld.ElapsedSeconds();
				if( eject_held >= 0.5 )
				{
					double unused = 0.;
					if( modf( eject_held * 6., &unused ) >= 0.5 )
						Raptor::Game->Gfx.Clear( 1.f, 0.f, 0.f );
					
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
					Camera cam_to_target( Raptor::Game->Cam );
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
							Raptor::Game->ShaderMgr.Select( Raptor::Game->Res.GetShader("model_hud") );
							Raptor::Game->ShaderMgr.ResumeShaders();
							Raptor::Game->ShaderMgr.Set3f( "AmbientLight", 0., 0., 0. );
						}
						
						target_ship->Draw();
						
						if( use_shaders )
						{
							Raptor::Game->ShaderMgr.StopShaders();
							Raptor::Game->ShaderMgr.Select( Raptor::Game->Res.GetShader("model") );
						}
						
						glLineWidth( 2.f );
						target_ship->DrawWireframe();
						
						if( observed_ship->TargetSubsystem )
						{
							std::string subsystem_name = target_ship->SubsystemName( observed_ship->TargetSubsystem );
							glDisable( GL_DEPTH_TEST );
							
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
							
							glEnable( GL_DEPTH_TEST );
						}
					}
					else if( target_obj->Type() == XWing::Object::TURRET )
					{
						Turret *target_turret = (Turret*) target_obj;
						
						if( target_turret->GunShape )
							cam_to_target.MoveAlong( &(cam_to_target.Fwd), 75. - target_turret->GunShape->GetTriagonal() * 0.044 );
						
						if( use_shaders )
						{
							Raptor::Game->ShaderMgr.Select( Raptor::Game->Res.GetShader("model_hud") );
							Raptor::Game->ShaderMgr.ResumeShaders();
							Raptor::Game->ShaderMgr.Set3f( "AmbientLight", 0., 0., 0. );
						}
						
						target_turret->Draw( false );  // Prevent changing to deathstar shader on Battle of Yavin.
						
						if( use_shaders )
						{
							Raptor::Game->ShaderMgr.StopShaders();
							Raptor::Game->ShaderMgr.Select( Raptor::Game->Res.GetShader("model") );
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
						Raptor::Game->Gfx.DrawLine3D(
							target_obj->X + front.X * 15., target_obj->Y + front.Y * 15., target_obj->Z + front.Z * 15.,
							target_obj->X - front.X * 10., target_obj->Y - front.Y * 10., target_obj->Z - front.Z * 10.,
							7.f, r,g,b,1.f );
						Raptor::Game->Gfx.DrawLine3D(
							target_obj->X + front.X * 15.,              target_obj->Y + front.Y * 15.,              target_obj->Z + front.Z * 15.,
							target_obj->X + front.X * 5. + out.X * 10., target_obj->Y + front.Y * 5. + out.Y * 10., target_obj->Z + front.Z * 5. + out.Z * 10.,
							5.f, r,g,b,1.f );
						Raptor::Game->Gfx.DrawLine3D(
							target_obj->X + front.X * 15.,              target_obj->Y + front.Y * 15.,              target_obj->Z + front.Z * 15.,
							target_obj->X + front.X * 5. - out.X * 10., target_obj->Y + front.Y * 5. - out.Y * 10., target_obj->Z + front.Z * 5. - out.Z * 10.,
							5.f, r,g,b,1.f );
					}
					else
					{
						Camera original_cam( Raptor::Game->Cam );
						Raptor::Game->Cam = cam_to_target;  // Shot::Draw looks at Raptor::Game->Cam to calculate draw vectors.
						target_obj->Draw();
						Raptor::Game->Cam = original_cam;
					}
					
					
					target_framebuffer->Setup2D();
					
					uint32_t target_team = XWing::Team::NONE;
					std::string target_name = "";
					std::string target_status = "";
					bool target_seeking_us = false;
					
					if( target )
					{
						target_name = target->Name;
						target_team = target->Team;
						Player *target_player = target->Owner();
						if( target_player )
							target_name = target_player->Name;
						
						if( target->Class && (target->Class->Category != ShipClass::CATEGORY_TARGET) )
						{
							size_t max_name_length = std::max<int>( 1, 14 - target->Class->ShortName.length() );
							target_name = target->Class->ShortName + std::string(": ") + ((target_name.length() > max_name_length) ? target_name.substr(0,max_name_length) : target_name);
						}
						
						if( target->Health < (target->MaxHealth() * 0.7) )
							target_status = "Hull Damaged";
						else if( (target->MaxShield() > 0.) && ((target->ShieldF <= 0.) || (target->ShieldR <= 0.)) )
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
							
							const GameObject *from_obj = Raptor::Game->Data.GetObject( target_shot->FiredFrom );
							if( from_obj->Type() == XWing::Object::SHIP )
								target_team = ((const Ship*)( from_obj ))->Team;
							else if( from_obj->Type() == XWing::Object::TURRET )
								target_team = ((const Turret*)( from_obj ))->Team;
							
							if( target_shot->Seeking )
							{
								const GameObject *seeking_obj = Raptor::Game->Data.GetObject( target_shot->Seeking );
								if( seeking_obj && (seeking_obj->PlayerID == Raptor::Game->PlayerID) && (seeking_obj->Type() != XWing::Object::SHOT) )
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
					ScreenFont->DrawText( target_status, target_framebuffer->W / 2, 10 + ScreenFont->PointSize, Font::ALIGN_TOP_CENTER, 1.f, 1.f, 0.5f, 1.f );
					
					int dist = vec_to_target.Length();
					if( target && observed_ship->TargetSubsystem )
					{
						dist = target->TargetCenter( observed_ship->TargetSubsystem ).Dist( observed_ship );
						
						// Draw selected subsystem.
						if( observed_ship && observed_ship->TargetSubsystem )
						{
							std::string subsystem_name = target->SubsystemName( observed_ship->TargetSubsystem );
							
							if( ! Raptor::Game->Cfg.SettingAsBool("ugly_names",false) )
							{
								// Clean up name.
								const char *subsystem_name_cstr = subsystem_name.c_str();
								if( strncmp( subsystem_name_cstr, "ShieldGen", strlen("ShieldGen") ) == 0 )
									subsystem_name = "Shield Generator";
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
							
							ScreenFont->DrawText( subsystem_name, target_framebuffer->W / 2, target_framebuffer->H - 10 - ScreenFont->PointSize, Font::ALIGN_BOTTOM_CENTER, 0.9f, 0.9f, 0.9f, 1.f );
						}
					}
					
					// Draw target distance.
					ScreenFont->DrawText( "Dist: ", 10, target_framebuffer->H - 10, Font::ALIGN_BOTTOM_LEFT, 0.5f, 0.5f, 1.f, 1.f );
					ScreenFont->DrawText( Num::ToString(dist), target_framebuffer->W - 10, target_framebuffer->H - 10, Font::ALIGN_BOTTOM_RIGHT, 0.5f, 0.5f, 1.f, 1.f );
				}
				
				
				// Fade screens in when jumping in from hyperspace.
				
				if( jump_progress < 1.5 )
				{
					target_framebuffer->Setup2D( 0., 0., 1., 1. );
					Raptor::Game->Gfx.DrawRect2D( 0., 0., 1., 1., 0, 0.f,0.f,0.f,1.f );
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
			
			
			Framebuffer *intercept_framebuffer = Raptor::Game->Res.GetFramebuffer( "intercept" );
			if( intercept_framebuffer && intercept_framebuffer->Select() )
			{
				// Render intercept display.
				
				changed_framebuffer = true;
				bool lock_display = (observed_ship->SelectedWeapon == Shot::TYPE_TORPEDO) || (observed_ship->SelectedWeapon == Shot::TYPE_MISSILE);
				
				if( lock_display )
				{
					// Draw torpedo lock-on display.
					
					Raptor::Game->Gfx.Clear( 0.f, 0.0f, 0.f );
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
								Raptor::Game->Gfx.DrawCircle2D( x, y, 10., 6, 0, 1.f, 1.f, 0.f, 1.f );
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
						Raptor::Game->Gfx.Clear( 0.f, 0.f, 0.5f );
					else
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
					
					
					glLineWidth( 2.f );
					
					
					// Draw the pointy bits.
					
					intercept_framebuffer->Setup2D( -1., -1., 1., 1. );
					
					float r = 0.f, g = 1.f, b = 0.f;
					
					if( will_hit )
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
								Raptor::Game->Gfx.DrawCircle2D( tx, ty, 0.07, 6, 0, 0.f, 0.25f, 1.f, 1.f );
							else
								Raptor::Game->Gfx.DrawCircle2D( tx, ty, 0.07, 6, 0, 0.f, 0.5f, 0.f, 1.f );
						}
						
						if( draw_intercept )
							Raptor::Game->Gfx.DrawCircle2D( ix, iy, 0.1, 4, 0, r, g, b, 1.f );
					}
				}
				
				
				// Fade screens in when jumping in from hyperspace.
				
				if( jump_progress < 1.4 )
				{
					intercept_framebuffer->Setup2D( 0., 0., 1., 1. );
					Raptor::Game->Gfx.DrawRect2D( 0., 0., 1., 1., 0, 0.f,0.f,0.f,1.f );
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
			
			
			Framebuffer *throttle_framebuffer = Raptor::Game->Res.GetFramebuffer( "throttle" );
			if( throttle_framebuffer && throttle_framebuffer->Select() )
			{
				// Render throttle display.
				
				changed_framebuffer = true;
				Raptor::Game->Gfx.Clear( 0.f, 0.f, 0.f );
				
				
				throttle_framebuffer->Setup2D( 0., 0., 1., 1. );
				
				double throttle = observed_ship->GetThrottle();
				Raptor::Game->Gfx.DrawRect2D( 0., 1. - throttle, 1., 1., 0, 0.125f * (float)throttle, 0.25f + 0.25f * (float)throttle, 1.f, 1.f );
				
				
				// Fade screens in when jumping in from hyperspace.
				
				if( jump_progress < 1.15 )
					Raptor::Game->Gfx.DrawRect2D( 0., 0., 1., 1., 0, 0.f,0.f,0.f,1.f );
			}
			
			
			// NOTE: Add any new displays here.
		}
		
		// Return to default framebuffer.
		if( changed_framebuffer )
			Raptor::Game->Gfx.SelectDefaultFramebuffer();
	}
	
	
	// Update Saitek LEDs and MFDs based on player state.
	
	#ifdef WIN32
	if( Raptor::Game->FrameTime )
		UpdateSaitek( observed_ship, (observed_ship == player_ship), view );
	#endif
	
	
	// Special case "spectator_view instruments" to draw instrument panel on a second PC.
	
	if( view == VIEW_INSTRUMENTS )
	{
		glColor4f( 1.f, 1.f, 1.f, 1.f );
		Raptor::Game->Gfx.Clear();
		
		if( observed_ship && (observed_ship->Health > 0.) )
		{
			if( Raptor::Game->Gfx.AspectRatio < 1.5 )
			{
				Raptor::Game->Gfx.Setup2D( 0., 1. );
				Raptor::Game->Gfx.DrawRect2D( -0.5, 0.,   1.5,  1.,   Raptor::Game->Res.GetAnimation("brushed.ani")->CurrentFrame() );
				Raptor::Game->Gfx.DrawRect2D( 0.1,  0.65, 0.9,  0.05, Raptor::Game->Res.GetTexture("*intercept") );
				Raptor::Game->Gfx.DrawRect2D( 0.1,  0.99, 0.35, 0.66, Raptor::Game->Res.GetTexture("*health") );
				Raptor::Game->Gfx.DrawRect2D( 0.65, 0.99, 0.9,  0.66, Raptor::Game->Res.GetTexture("*target") );
				Raptor::Game->Gfx.DrawRect2D( 0.45, 0.95, 0.55, 0.7,  Raptor::Game->Res.GetTexture("*throttle") );
			}
			else
			{
				Raptor::Game->Gfx.Setup2D( -1., 1. );
				Raptor::Game->Gfx.DrawRect2D( -2., -1., 2., 1., Raptor::Game->Res.GetAnimation("brushed.ani")->CurrentFrame() );
				double side_w = std::max<double>( 0.72, std::min<double>( 1.5, Raptor::Game->Gfx.AspectRatio - 1.3 ) );
				double side_h = side_w * 4. / 3.;
				double mid_w = std::min<double>( 2., 2. * (Raptor::Game->Gfx.AspectRatio - side_w) - 0.05 );
				double mid_h = mid_w * 0.75;
				Raptor::Game->Gfx.DrawRect2D( mid_w * -0.5, mid_h - 0.9, mid_w * 0.5, -0.9, Raptor::Game->Res.GetTexture("*intercept") );
				Raptor::Game->Gfx.DrawRect2D( 0.02 - Raptor::Game->Gfx.AspectRatio, 0.98, side_w + 0.02 - Raptor::Game->Gfx.AspectRatio, 0.98 - side_h, Raptor::Game->Res.GetTexture("*health") );
				Raptor::Game->Gfx.DrawRect2D( Raptor::Game->Gfx.AspectRatio - side_w - 0.02, 0.98, Raptor::Game->Gfx.AspectRatio - 0.02, 0.98 - side_h, Raptor::Game->Res.GetTexture("*target") );
				Raptor::Game->Gfx.DrawRect2D( -0.1, 0.95, 0.1, mid_h - 0.85, Raptor::Game->Res.GetTexture("*throttle") );
			}
		}
		
		if( (observed_player || observed_ship) && ! MessageInput->Visible )
		{
			Raptor::Game->Gfx.Setup2D();
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
		MessageOutput->ScrollTime = ((const XWingGame*)( Raptor::Game ))->OverlayScroll;
		
		glColor4f( 1.f, 1.f, 1.f, 1.f );
		glPopMatrix();
		return;
	}
	
	
	// Set up 3D rendering for the scene.
	
	Raptor::Game->Cam.FOV = vr ? Raptor::Game->Cfg.SettingAsDouble("vr_fov") : Raptor::Game->Cfg.SettingAsDouble("g_fov");
	Raptor::Game->Gfx.Setup3D( &(Raptor::Game->Cam) );
	Raptor::Game->Gfx.Clear();
	
	int dynamic_lights = Raptor::Game->Cfg.SettingAsInt("g_shader_point_lights");
	
	float crosshair_red = 0.5f;
	float crosshair_green = 0.75f;
	float crosshair_blue = 1.f;
	int ammo = observed_ship ? observed_ship->AmmoForWeapon() : -1;
	
	Model *cockpit_3d = NULL;
	
	
	// Draw background visual elements.
	
	SetBackground();
	DrawBackground();
	DrawStars();
	DrawDebris();
	
	
	// Draw the cockpit if we're doing 3D cockpit.
	
	if( observed_ship && ((view == VIEW_COCKPIT) || (view == VIEW_GUNNER)) )
	{
		if( jump_progress < 1. )
		{
			// Background visuals fade in when arriving from hyperspace.
			Raptor::Game->Gfx.Setup2D( 0., 0., 1., 1. );
			Raptor::Game->Gfx.DrawRect2D( 0,0,1,1, 0, 0.f,0.f,0.f,(1.f - jump_progress) );
			Raptor::Game->Gfx.Setup3D( &(Raptor::Game->Cam) );
		}
		
		if( observed_turret )
		{
			ammo = -1;
			
			if( target )
			{
				// See if the crosshair is lined up so the next shot would hit.
				std::map<int,Shot*> test_shots = observed_turret->NextShots();
				for( std::map<int,Shot*>::iterator shot_iter = test_shots.begin(); shot_iter != test_shots.end(); shot_iter ++ )
				{
					if( target->WillCollide( shot_iter->second, 4. ) )
					{
						crosshair_red = 0.25f;
						crosshair_green = 1.f;
						crosshair_blue = 0.25f;
						
						static Clock beep_aim;
						if( beep_aim.Progress() >= 1. )
							Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("beep_aim.wav") );
						beep_aim.Reset( 0.2 );
					}
					delete shot_iter->second;
				}
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
		else if( target )
		{
			// See if the crosshair is lined up so the next shot would hit.
			std::map<int,Shot*> test_shots = observed_ship->NextShots();
			for( std::map<int,Shot*>::iterator shot_iter = test_shots.begin(); shot_iter != test_shots.end(); shot_iter ++ )
			{
				if( target->WillCollide( shot_iter->second, 4. ) )
				{
					crosshair_red = 0.25f;
					crosshair_green = 1.f;
					crosshair_blue = 0.25f;
					
					static Clock beep_aim;
					if( beep_aim.Progress() >= 1. )
						Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("beep_aim.wav") );
					beep_aim.Reset( 0.2 );
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
		
		
		// Cockpit.
		
		if( observed_ship->Class && (view == VIEW_COCKPIT) && (view_str != "crosshair") )
		{
			if( observed_ship->Group && Raptor::Game->Cfg.SettingAsBool("g_group_skins",true) )
			{
				std::map<uint8_t,std::string>::const_iterator skin_iter = observed_ship->Class->GroupCockpits.find( observed_ship->Group );
				if( (skin_iter != observed_ship->Class->GroupCockpits.end()) && skin_iter->second.length() )
					cockpit_3d = Raptor::Game->Res.GetModel( skin_iter->second );
			}
			
			if( !( cockpit_3d && cockpit_3d->VertexCount() ) )
			{
				if( vr && ! observed_ship->Class->CockpitModelVR.empty() )
					cockpit_3d = Raptor::Game->Res.GetModel( observed_ship->Class->CockpitModelVR );
				else
					cockpit_3d = Raptor::Game->Res.GetModel( observed_ship->Class->CockpitModel );
			}
		}
		
		if( cockpit_3d && cockpit_3d->Objects.size() )
		{
			// Draw the 3D cockpit.
			
			if( use_shaders )
				Raptor::Game->ShaderMgr.ResumeShaders();
			
			bool change_light_for_cockpit = false;
			if( Raptor::Game->ShaderMgr.Active() )
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
			}
			
			// Technically the correct scale is 0.022, but we may need to make it higher to avoid near-plane clipping.
			double model_scale = observed_ship->Class ? observed_ship->Class->ModelScale : 0.022;
			double cockpit_scale = vr ? model_scale : std::max<double>( model_scale, Raptor::Game->Gfx.ZNear * 0.25 );
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
			cockpit_pos.FixVectors();
			Raptor::Game->Cam.Copy( &cockpit_pos );
			Raptor::Game->Cam.Yaw( ((XWingGame*)( Raptor::Game ))->LookYaw );
			Raptor::Game->Cam.Pitch( ((XWingGame*)( Raptor::Game ))->LookPitch );
			Raptor::Game->Gfx.Setup3D( &(Raptor::Game->Cam) );
			if( use_shaders )
				Raptor::Game->ShaderMgr.Set3f( "CamPos", Raptor::Game->Cam.X, Raptor::Game->Cam.Y, Raptor::Game->Cam.Z );
			if( (! vr) || Raptor::Game->Cfg.SettingAsBool("vr_sway",false) )
			{
				cockpit_pos.Yaw(   observed_ship->YawRate   * 0.01 );
				cockpit_pos.Pitch( observed_ship->PitchRate * 0.005 );
				cockpit_pos.Roll(  observed_ship->RollRate  * 0.015 );
			}
			cockpit_pos.MoveAlong( &(cockpit_pos.Fwd),   cockpit_fwd   * cockpit_scale );
			cockpit_pos.MoveAlong( &(cockpit_pos.Up),    cockpit_up    * cockpit_scale );
			cockpit_pos.MoveAlong( &(cockpit_pos.Right), cockpit_right * cockpit_scale );
			cockpit_3d->DrawAt( &cockpit_pos, cockpit_scale );
			
			// Restore camera position for scene render.
			Raptor::Game->Cam.Copy( &Cam );
			Raptor::Game->Gfx.Setup3D( &(Raptor::Game->Cam) );
			
			// Reset world lights to normal.
			if( change_light_for_cockpit )
				SetWorldLights( deathstar );
			
			if( use_shaders )
			{
				// Restore camera position in shaders for lighting effects.
				Raptor::Game->ShaderMgr.Set3f( "CamPos", Raptor::Game->Cam.X, Raptor::Game->Cam.Y, Raptor::Game->Cam.Z );
				Raptor::Game->ShaderMgr.StopShaders();
			}
		}
		else
			// Not drawing a 3D cockpit.
			cockpit_3d = NULL;
	}
	
	
	// Draw game objects.
	
	double draw_dist = Raptor::Game->Cfg.SettingAsDouble( "g_draw_dist", 6000. );
	
	if( use_shaders )
		Raptor::Game->ShaderMgr.ResumeShaders();
	
	std::multimap<double,Renderable> sorted_renderables;
	
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
	{
		// We'll draw shots after everything else, so don't draw them now.
		if( obj_iter->second->Type() == XWing::Object::SHOT )
			continue;
		
		if( obj_iter->second->Type() == XWing::Object::SHIP )
		{
			Ship *ship = (Ship*) obj_iter->second;
			
			if( (ship->Health <= 0.) && (ship->Category() == ShipClass::CATEGORY_CAPITAL) && (ship->DeathClock.ElapsedSeconds() > ship->PiecesDangerousTime()) )
			{
				// Disintegrating ships draw later.
				sorted_renderables.insert( std::pair<double,Renderable>( ship->DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ) + sqrt(ship->Radius()), ship ) );
				continue;
			}
			
			// Don't draw an external view of a ship whose cockpit we're in.
			if( (view == VIEW_COCKPIT) && (ship == observed_ship) && (ship->Health > 0.) )
				continue;
			
			// Don't draw tiny ships far away.
			double size = ship->Shape.GetTriagonal();
			if( (Raptor::Game->Cam.Dist(ship) * (vr ? 20. : 10.) / size) > draw_dist )
				continue;
			
			// Don't draw ships that are entirely behind us.
			if( ship->DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ) < -size )
				continue;
			
			// Draw engine glows later, with the other transparent renderables.
			if( ship->Health > 0. )
			{
				double ship_throttle = ship->GetThrottle();
				if( (ship_throttle >= 0.75) && ship->Engines.size() && Raptor::Game->Cfg.SettingAsBool("g_engine_glow",true) )
				{
					float engine_alpha = (ship_throttle - 0.75f) * 4.f;
					std::map<ShipEngine*,Pos3D> engines = ship->EnginePositions();
					for( std::map<ShipEngine*,Pos3D>::const_iterator engine_iter = engines.begin(); engine_iter != engines.end(); engine_iter ++ )
						sorted_renderables.insert( std::pair<double,Renderable>( engine_iter->second.DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ), Renderable( engine_iter->first, &(engine_iter->second), engine_alpha ) ) );
				}
			}
		}
		else if( obj_iter->second->Type() == XWing::Object::TURRET )
		{
			// Don't draw turrets far away.
			if( Raptor::Game->Cam.Dist(obj_iter->second) > draw_dist )
				continue;
			
			// Don't draw turrets that are entirely behind us.
			if( obj_iter->second->DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ) < -100. )
				continue;
		}
		else if( obj_iter->second->Type() == XWing::Object::DEATH_STAR_BOX )
		{
			DeathStarBox *box = (DeathStarBox*) obj_iter->second;
			
			// Don't draw boxes far away.
			if( Raptor::Game->Cam.Dist(box) > draw_dist )
				continue;
			
			// Don't draw boxes that are entirely behind us.
			if( obj_iter->second->DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ) < (box->L + box->W + box->H) / -2. )
				continue;
		}

		if( dynamic_lights && Raptor::Game->ShaderMgr.Active() )
		{
			Pos3D *pos = (obj_iter->second->Type() == XWing::Object::DEATH_STAR) ? (Pos3D*) &(Raptor::Game->Cam) : (Pos3D*) obj_iter->second;
			SetDynamicLights( pos, NULL, dynamic_lights, &shots, &effects );
		}
		
		glPushMatrix();
		obj_iter->second->Draw();
		glPopMatrix();
	}
	
	if( Raptor::Game->ShaderMgr.Active() )
	{
		// Remove dynamic lights and material properties so other things will render correctly.
		ClearMaterial();
		ClearDynamicLights();
	}
	
	if( use_shaders )
		Raptor::Game->ShaderMgr.StopShaders();
	
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	
	
	// Draw anything with alpha back-to-front.
	
	for( std::list<Shot*>::iterator shot_iter = shots.begin(); shot_iter != shots.end(); shot_iter ++ )
		sorted_renderables.insert( std::pair<double,Renderable>( (*shot_iter)->DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ), *shot_iter ) );
	
	for( std::list<Effect*>::iterator effect_iter = effects.begin(); effect_iter != effects.end(); effect_iter ++ )
		sorted_renderables.insert( std::pair<double,Renderable>( (*effect_iter)->DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ), *effect_iter ) );
	
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
				Raptor::Game->ShaderMgr.ResumeShaders();
				if( dynamic_lights )
					SetDynamicLights( renderable->second.ShipPtr, NULL, dynamic_lights, &shots, &effects );
			}
			
			renderable->second.ShipPtr->Draw();
			
			if( use_shaders )
				Raptor::Game->ShaderMgr.StopShaders();
		}
		else if( renderable->second.EnginePtr )
			renderable->second.EnginePtr->DrawAt( &(renderable->second.EnginePos), renderable->second.EngineAlpha );
		
		glPopMatrix();
	}
	
	
	// Reset the color, dynamic lights, and material properties so other things will render correctly.
	
	if( use_shaders )
	{
		Raptor::Game->ShaderMgr.ResumeShaders();
		
		if( Raptor::Game->ShaderMgr.Active() )
		{
			ClearMaterial();
			ClearDynamicLights();
		}
		
		Raptor::Game->ShaderMgr.StopShaders();
	}
	
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	
	
	// Draw 2D UI elements.
	
	GameObject *observed_object = NULL;
	if( observed_ship && (view == VIEW_COCKPIT) )
		observed_object = observed_ship;
	else if( observed_turret && (view == VIEW_GUNNER) )
		observed_object = observed_turret;
	
	if( observed_object )
	{
		uint8_t observed_team = (observed_object == observed_ship) ? observed_ship->Team : observed_turret->Team;
		
		if( (jump_progress < 1.) && observed_ship )
		{
			// Draw hyperspace lines.
			
			Randomizer r( observed_ship->Lifetime.TimeVal.tv_sec );
			for( size_t i = 0; i < 333; i ++ )
			{
				Pos3D pos1( observed_object );
				pos1.MoveAlong( &(observed_ship->Right), r.Double( -2000., 2000. ) );
				pos1.MoveAlong( &(observed_ship->Up),    r.Double( -2000., 2000. ) );
				pos1.MoveAlong( &(observed_ship->Fwd),   r.Double(  5000., 5500. ) );
				Pos3D pos2( &pos1 );
				pos2.MoveAlong( &(observed_ship->Fwd),  -9900. * (1. - jump_progress) );
				Raptor::Game->Gfx.DrawLine3D( pos1.X, pos1.Y, pos1.Z, pos2.X, pos2.Y, pos2.Z, (i % 3) ? 2.5f : 1.f, 1.f,1.f,1.f,1.f );
			}
		}
		
		
		if( target || (target_obj && ( (target_obj->Type() == XWing::Object::SHOT) || (target_obj->Type() == XWing::Object::CHECKPOINT) )) )
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
			else if( target_obj->Type() == XWing::Object::CHECKPOINT )
			{
				const Checkpoint *checkpoint = (const Checkpoint*) target_obj;
				w = h = (checkpoint->Radius * 0.75);
			}
			
			Vec3D up = Raptor::Game->Cam.Up * h / 2.;
			Vec3D right = Raptor::Game->Cam.Right * w / 2.;
			float red = 1.f, green = 1.f, blue = 1.f, alpha = 1.f;
			
			std::deque<Pos3D> positions;
			positions.push_back( target_obj );
			
			// Draw target box around selected subsystem.
			if( target && observed_ship && (observed_ship->TargetSubsystem) )
				positions.push_back( target->TargetCenter( observed_ship->TargetSubsystem ) );
			
			// Turret gunners get another target box for the intercept point.
			if( observed_turret && (target_obj->MotionVector.Length() > 40.) )
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
				Vec3D vec_to_target( pos.X - Raptor::Game->Cam.X, pos.Y - Raptor::Game->Cam.Y, pos.Z - Raptor::Game->Cam.Z );
				double dist = vec_to_target.Length();
				if( dist > max_dist )
				{
					vec_to_target.ScaleTo( 1. );
					pos.MoveAlong( &vec_to_target, max_dist - dist );
				}
				
				Raptor::Game->Gfx.DrawLine3D( pos.X - right.X + up.X, pos.Y - right.Y + up.Y, pos.Z - right.Z + up.Z, pos.X - right.X / 2. + up.X, pos.Y - right.Y / 2. + up.Y, pos.Z - right.Z / 2. + up.Z, 1.f, red, green, blue, alpha );
				Raptor::Game->Gfx.DrawLine3D( pos.X - right.X + up.X, pos.Y - right.Y + up.Y, pos.Z - right.Z + up.Z, pos.X - right.X + up.X / 2., pos.Y - right.Y + up.Y / 2., pos.Z - right.Z + up.Z / 2., 1.f, red, green, blue, alpha );
				
				Raptor::Game->Gfx.DrawLine3D( pos.X + right.X + up.X, pos.Y + right.Y + up.Y, pos.Z + right.Z + up.Z, pos.X + right.X / 2. + up.X, pos.Y + right.Y / 2. + up.Y, pos.Z + right.Z / 2. + up.Z, 1.f, red, green, blue, alpha );
				Raptor::Game->Gfx.DrawLine3D( pos.X + right.X + up.X, pos.Y + right.Y + up.Y, pos.Z + right.Z + up.Z, pos.X + right.X + up.X / 2., pos.Y + right.Y + up.Y / 2., pos.Z + right.Z + up.Z / 2., 1.f, red, green, blue, alpha );
				
				Raptor::Game->Gfx.DrawLine3D( pos.X + right.X - up.X, pos.Y + right.Y - up.Y, pos.Z + right.Z - up.Z, pos.X + right.X / 2. - up.X, pos.Y + right.Y / 2. - up.Y, pos.Z + right.Z / 2. - up.Z, 1.f, red, green, blue, alpha );
				Raptor::Game->Gfx.DrawLine3D( pos.X + right.X - up.X, pos.Y + right.Y - up.Y, pos.Z + right.Z - up.Z, pos.X + right.X - up.X / 2., pos.Y + right.Y - up.Y / 2., pos.Z + right.Z - up.Z / 2., 1.f, red, green, blue, alpha );
				
				Raptor::Game->Gfx.DrawLine3D( pos.X - right.X - up.X, pos.Y - right.Y - up.Y, pos.Z - right.Z - up.Z, pos.X - right.X / 2. - up.X, pos.Y - right.Y / 2. - up.Y, pos.Z - right.Z / 2. - up.Z, 1.f, red, green, blue, alpha );
				Raptor::Game->Gfx.DrawLine3D( pos.X - right.X - up.X, pos.Y - right.Y - up.Y, pos.Z - right.Z - up.Z, pos.X - right.X - up.X / 2., pos.Y - right.Y - up.Y / 2., pos.Z - right.Z - up.Z / 2., 1.f, red, green, blue, alpha );
				
				if( observed_turret )
				{
					// Change the parameters for the intercept box.
					red   = crosshair_red;
					green = crosshair_green;
					blue  = crosshair_blue;
					up.ScaleBy( 0.8 );
					right.ScaleBy( 0.8 );
				}
				else
				{
					// FIXME: Get actual size of subsystem!
					up.ScaleTo( 17. );
					right.ScaleTo( 17. );
				}
			}
			
			glEnable( GL_DEPTH_TEST );
		}
		
		
		if( jump_progress >= 1. )
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
			glLineWidth( Raptor::Game->Cfg.SettingAsDouble("g_crosshair_thickness",1.5) );
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
				
				std::map<int,Shot*> all_weapons = (observed_object == observed_ship) ? observed_ship->AllShots() : std::map<int,Shot*>();
				for( std::map<int,Shot*>::iterator shot_iter = all_weapons.begin(); shot_iter != all_weapons.end(); shot_iter ++ )
				{
					weapon_vec.Set( shot_iter->second->X - observed_object->X, shot_iter->second->Y - observed_object->Y, shot_iter->second->Z - observed_object->Z );
					relative_weapon_vec.Set( weapon_vec.Dot(&(observed_object->Right)), weapon_vec.Dot(&(observed_object->Up)) );
					relative_weapon_vec.ScaleTo( 1. );
					weapon_pos.Copy( &crosshair_pos );

					if( (shot_iter->second->ShotType == Shot::TYPE_TORPEDO) || (shot_iter->second->ShotType == Shot::TYPE_MISSILE) )
						relative_weapon_vec.ScaleBy( 0.9 );
					else if( shot_iter->second->ShotType == Shot::TYPE_ION_CANNON )
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
				for( std::map<int,Shot*>::iterator shot_iter = all_weapons.begin(); shot_iter != all_weapons.end(); shot_iter ++ )
				{
					weapon_vec.Set( shot_iter->second->X - observed_object->X, shot_iter->second->Y - observed_object->Y, shot_iter->second->Z - observed_object->Z );
					relative_weapon_vec.Set( weapon_vec.Dot(&(observed_object->Right)), weapon_vec.Dot(&(observed_object->Up)) );
					relative_weapon_vec.ScaleTo( 1. );
					weapon_pos.Copy( &crosshair_pos );
					
					if( (shot_iter->second->ShotType == Shot::TYPE_TORPEDO) || (shot_iter->second->ShotType == Shot::TYPE_MISSILE) )
						relative_weapon_vec.ScaleBy( 0.9 );
					else if( shot_iter->second->ShotType == Shot::TYPE_ION_CANNON )
						relative_weapon_vec.ScaleBy( 0.95 );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X * 3.6 );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y * 3.6 );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
					
					delete shot_iter->second;
				}
			glEnd();
			
			// Draw big dots for the next shots to fire.
			glPointSize( 5.f );
			glBegin( GL_POINTS );
			std::map<int,Shot*> next_weapons = (observed_object == observed_ship) ? observed_ship->NextShots() : std::map<int,Shot*>();
				for( std::map<int,Shot*>::iterator shot_iter = next_weapons.begin(); shot_iter != next_weapons.end(); shot_iter ++ )
				{
					weapon_vec.Set( shot_iter->second->X - observed_object->X, shot_iter->second->Y - observed_object->Y, shot_iter->second->Z - observed_object->Z );
					relative_weapon_vec.Set( weapon_vec.Dot(&(observed_object->Right)), weapon_vec.Dot(&(observed_object->Up)) );
					relative_weapon_vec.ScaleTo( 1. );
					weapon_pos.Copy( &crosshair_pos );
					
					if( (shot_iter->second->ShotType == Shot::TYPE_TORPEDO) || (shot_iter->second->ShotType == Shot::TYPE_MISSILE) )
						relative_weapon_vec.ScaleBy( 0.9 );
					else if( shot_iter->second->ShotType == Shot::TYPE_ION_CANNON )
						relative_weapon_vec.ScaleBy( 0.95 );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X * 3.6 );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y * 3.6 );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
					
					delete shot_iter->second;
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
			
			if( (target || dead_target) && need_target_holo )
			{
				// Show holographic targetting display.
				
				Ship *observed_target = target ? target : dead_target;
				Vec3D vec_to_target( observed_target->X - observed_object->X, observed_target->Y - observed_object->Y, observed_target->Z - observed_object->Z );
				vec_to_target.ScaleTo( 1. );
				Camera cam_to_target( Raptor::Game->Cam );
				cam_to_target.Fwd.Copy( &vec_to_target );
				cam_to_target.Up.Copy( &(observed_object->Up) );
				cam_to_target.FixVectors();
				cam_to_target.SetPos( observed_target->X, observed_target->Y, observed_target->Z );
				cam_to_target.MoveAlong( &(cam_to_target.Fwd), -0.75 );
				cam_to_target.Pitch( 25. );
				cam_to_target.Yaw( ((XWingGame*)( Raptor::Game ))->LookYaw );
				cam_to_target.Pitch( ((XWingGame*)( Raptor::Game ))->LookPitch );
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
				
				Raptor::Game->Gfx.Setup3D( &(cam_to_target) );
				
				glLineWidth( 1.f );
				observed_target->DrawWireframe( &holo_color1, holo_scale );
				
				glLineWidth( 2.f );
				observed_target->DrawWireframe( &holo_color2, holo_scale );
				
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
				
				if( vr )
				{
					Pos3D name_pos = cam_to_target;
					name_pos.SetPos( observed_target->X, observed_target->Y, observed_target->Z );
					name_pos.MoveAlong( &(name_pos.Up), -0.1 );
					BigFont->DrawText3D( target_name, &name_pos, Font::ALIGN_TOP_CENTER, red, green, blue, holo_color1.Alpha, 0.002 );
				}
				else
				{
					Raptor::Game->Gfx.Setup2D();
					BigFont->DrawText( target_name, Rect.x + Rect.w/2 + 2, Rect.h - 8, Font::ALIGN_BOTTOM_CENTER, 0.f, 0.f, 0.f, alpha * 0.8f );
					BigFont->DrawText( target_name, Rect.x + Rect.w/2, Rect.h - 10, Font::ALIGN_BOTTOM_CENTER, red, green, blue, alpha * 1.f );
				}
			}
			
			
			if( ! vr )
			{
				// Draw the radar.
				
				Raptor::Game->Gfx.Setup2D( Raptor::Game->Gfx.H / -2, Raptor::Game->Gfx.H / 2 );
				double h = Raptor::Game->Gfx.H / 2.;
				RadarDirectionFont->DrawText( "F", -1.304 * h, -0.829 * h, Font::ALIGN_TOP_RIGHT, 0.f, 0.f, 1.f, 0.75f );
				RadarDirectionFont->DrawText( "R", 1.304 * h, -0.829 * h, Font::ALIGN_TOP_LEFT, 0.f, 0.f, 1.f, 0.75f );
				
				Raptor::Game->Gfx.Setup2D( -1., 1. );
				Raptor::Game->Gfx.DrawCircle2D( -1.233, -0.9, 0.1, 32, 0, 0.f, 0.f, 0.f, 0.75f );
				Raptor::Game->Gfx.DrawCircle2D( 1.233, -0.9, 0.1, 32, 0, 0.f, 0.f, 0.f, 0.75f );
				Raptor::Game->Gfx.DrawCircleOutline2D( -1.233, -0.9, 0.1, 32, 1.f, 0.f, 0.f, 1.f, 0.75f );
				Raptor::Game->Gfx.DrawCircleOutline2D( 1.233, -0.9, 0.1, 32, 1.f, 0.f, 0.f, 1.f, 0.75f );
				
				Pos3D *radar_ref = (Pos3D*) &(Raptor::Game->Cam);
				
				for( std::map<uint32_t,GameObject*>::iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
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
						}
						else if( shot )
						{
							red = 1.f;
							green = 1.f;
							blue = 0.f;
							radius = 0.003;
						}
						else if( type == XWing::Object::ASTEROID )
						{
							green = 0.4f;
							blue = 0.3f;
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
								radius = 0.004;
							}
						}
						
						Raptor::Game->Gfx.DrawCircle2D( x, y, radius, 6, 0, red, green, blue, 1.f );
					}
				}
				
				if( target_obj )
				{
					// Draw target in radar.
					
					Vec3D vec_to_target( target_obj->X - radar_ref->X, target_obj->Y - radar_ref->Y, target_obj->Z - radar_ref->Z );
					if( target && observed_ship && observed_ship->TargetSubsystem )
						vec_to_target = target->TargetCenter( observed_ship->TargetSubsystem ) - *radar_ref;
					
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
					}
					else if( target_obj->Type() == XWing::Object::SHOT )
					{
						red = 1.f;
						green = 1.f;
						blue = 0.f;
						radius = 0.003;
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
					
					Raptor::Game->Gfx.DrawBox2D( x - 0.01, y - 0.01, x + 0.01, y + 0.01, 1.f, 1.f, 1.f, 1.f, 1.f );
					Raptor::Game->Gfx.DrawCircle2D( x, y, radius, 6, 0, red, green, blue, 1.f );
				}
			}
		}
	}
	
	
	// If we're spectating, show who we're watching.
	
	if( (observed_player && (observed_player->ID == Raptor::Game->PlayerID)) || screensaver || vr )
		;
	else if( observed_player )
	{
		Raptor::Game->Gfx.Setup2D();
		BigFont->DrawText( "Watching: " + observed_player->Name, Rect.x + Rect.w/2 + 2, Rect.y + 12, Font::ALIGN_TOP_CENTER, 0.f,0.f,0.f,0.8f );
		BigFont->DrawText( "Watching: " + observed_player->Name, Rect.x + Rect.w/2, Rect.y + 10, Font::ALIGN_TOP_CENTER );
	}
	else if( observed_ship )
	{
		Raptor::Game->Gfx.Setup2D();
		BigFont->DrawText( "Watching: " + observed_ship->Name, Rect.x + Rect.w/2 + 2, Rect.y + 12, Font::ALIGN_TOP_CENTER, 0.f,0.f,0.f,0.8f );
		BigFont->DrawText( "Watching: " + observed_ship->Name, Rect.x + Rect.w/2, Rect.y + 10, Font::ALIGN_TOP_CENTER );
	}
	
	
	// If the round is over, player is holding the scores key, or player is dead in VR, show the scores.
	
	bool draw_scores = (Raptor::Game->State >= XWing::State::ROUND_ENDED) && ! screensaver;
	if( ! draw_scores )
		draw_scores = Raptor::Game->Input.ControlPressed( ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::SCORES ] );
	if( ! draw_scores )
		draw_scores = vr && observed_ship && (observed_ship->PlayerID == Raptor::Game->PlayerID) && (observed_ship->Health <= 0.);
	if( draw_scores )
		DrawScores();
	
	
	// Flash "PAUSED" or current time scale if altered, unless in screensaver or cinematic mode.
	
	if( (Raptor::Game->Data.TimeScale != 1.) && ! screensaver )
	{
		float r = fabsf( cos( PlayTime.ElapsedSeconds() * M_PI ) );
		float g = 0.5f + r * 0.5f;
		float b = 1.f;
		Raptor::Game->Gfx.Setup2D();
		if( Raptor::Game->Data.TimeScale < 0.01 )
		{
			BigFont->DrawText( "PAUSED", Rect.x + Rect.w/2 + 2, Rect.y + Rect.h * 0.47 + 2, Font::ALIGN_MIDDLE_CENTER, 0.f,0.f,0.f,0.8f );
			BigFont->DrawText( "PAUSED", Rect.x + Rect.w/2,     Rect.y + Rect.h * 0.47,     Font::ALIGN_MIDDLE_CENTER, r,  g,  b,  1.f );
		}
		else
		{
			BigFont->DrawText( Num::ToString(Raptor::Game->Data.TimeScale) + std::string(" X"), Rect.x + Rect.w/2 + 2, Rect.y + 66, Font::ALIGN_MIDDLE_CENTER, 0.f,0.f,0.f,0.8f );
			BigFont->DrawText( Num::ToString(Raptor::Game->Data.TimeScale) + std::string(" X"), Rect.x + Rect.w/2,     Rect.y + 64, Font::ALIGN_MIDDLE_CENTER, r,  g,  b,  1.f );
		}
	}
	
	
	// Don't show non-chat messages in VR unless the chat box is open or show scores button is held.
	
	MessageOutput->Visible = true;
	MessageOutput->SetTypeToDraw( TextConsole::MSG_NORMAL, draw_scores || MessageInput->Visible || (! (vr || screensaver)) || Raptor::Game->Cfg.SettingAsBool("vr_messages") );
	MessageOutput->ScrollTime = ((const XWingGame*)( Raptor::Game ))->OverlayScroll;
	
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


void RenderLayer::DrawDebris( void )
{
	int debris_count = std::min<int>( DEBRIS_COUNT, Raptor::Game->Cfg.SettingAsInt( "g_debris", 500 ) );
	
	for( int i = 0; i < debris_count; i ++ )
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
	Raptor::Game->Gfx.Setup2D();
	
	std::set<PlayerScore> scores;
	for( std::map<uint16_t,Player*>::iterator player_iter = Raptor::Game->Data.Players.begin(); player_iter != Raptor::Game->Data.Players.end(); player_iter ++ )
	{
		if( (! player_iter->second->PropertyAsString("team").empty()) && (player_iter->second->PropertyAsString("team") != "Spectator") )
			scores.insert( player_iter->second );
	}
	
	double remaining_secs = ((XWingGame*)( Raptor::Game ))->RoundTimer.RemainingSeconds();
	if( remaining_secs > 0. )
	{
		int minutes = remaining_secs / 60.;
		int seconds = Num::FPart( remaining_secs / 60. ) * 60.;
		char time_string[ 32 ] = "";
		snprintf( time_string, 32, "%i:%02i", minutes, seconds );
		
		BigFont->DrawText( std::string(time_string), Rect.x + Rect.w/2 + 2, 62, Font::ALIGN_MIDDLE_CENTER, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( std::string(time_string), Rect.x + Rect.w/2, 60, Font::ALIGN_MIDDLE_CENTER, 1.f, 1.f, 1.f, 1.f );
	}
	
	int y = Rect.y + 100;
	
	const XWingGame *game = (const XWingGame*) Raptor::Game;
	uint32_t gametype = game->GameType;
	bool ffa = (gametype == XWing::GameType::FFA_ELIMINATION) || (gametype == XWing::GameType::FFA_DEATHMATCH) || (gametype == XWing::GameType::FFA_RACE);
	bool objective = (gametype != XWing::GameType::TEAM_ELIMINATION) && (gametype != XWing::GameType::TEAM_DEATHMATCH) && (gametype != XWing::GameType::TEAM_RACE) && ! ffa;
	
	if( ! ffa )
	{
		float rebel_g  = (game->Victor == XWing::Team::REBEL)  ? fabsf(cos( PlayTime.ElapsedSeconds() * M_PI * 2. )) : 0.12f;
		float empire_g = (game->Victor == XWing::Team::EMPIRE) ? fabsf(cos( PlayTime.ElapsedSeconds() * M_PI * 2. )) : 0.37f;
		
		BigFont->DrawText( "Rebels", Rect.x + Rect.w/2 - 318, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( "Rebels", Rect.x + Rect.w/2 - 320, y, Font::ALIGN_MIDDLE_LEFT, 1.f, rebel_g, 0.12f, 1.f );
		BigFont->DrawText( "vs", Rect.x + Rect.w/2 + 2, y + 2, Font::ALIGN_MIDDLE_CENTER, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( "vs", Rect.x + Rect.w/2, y, Font::ALIGN_MIDDLE_CENTER, 0.75f, 0.75f, 0.75f, 1.f );
		BigFont->DrawText( "Empire", Rect.x + Rect.w/2 + 322, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( "Empire", Rect.x + Rect.w/2 + 320, y, Font::ALIGN_MIDDLE_RIGHT, 0.25f, empire_g, 1.f, 1.f );
		
		if( ! objective )
		{
			double rebel_score  = Raptor::Game->Data.PropertyAsInt("team_score_rebel");
			double empire_score = Raptor::Game->Data.PropertyAsInt("team_score_rebel");
			if( rebel_score >= empire_score )
				empire_g = 0.37f;
			if( empire_score >= rebel_score )
				rebel_g = 0.12f;
			
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("team_score_rebel"),  Rect.x + Rect.w/2 - 14, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f,   0.f,      0.f,   0.8f );
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("team_score_rebel"),  Rect.x + Rect.w/2 - 16, y,     Font::ALIGN_MIDDLE_RIGHT, 1.f,   rebel_g,  0.12f, 1.f );
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("team_score_empire"), Rect.x + Rect.w/2 + 18, y + 2, Font::ALIGN_MIDDLE_LEFT,  0.f,   0.f,      0.f,   0.8f );
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("team_score_empire"), Rect.x + Rect.w/2 + 16, y,     Font::ALIGN_MIDDLE_LEFT,  0.25f, empire_g, 1.f,   1.f );
		}
		
		y += BigFont->GetHeight() - 8;
		Raptor::Game->Gfx.DrawLine2D( Raptor::Game->Gfx.W / 2 - 318, y + 2, Rect.x + Rect.w/2 + 322, y + 2, 1.f, 0.f, 0.f, 0.f, 0.8f );
		Raptor::Game->Gfx.DrawLine2D( Raptor::Game->Gfx.W / 2 - 320, y, Rect.x + Rect.w/2 + 320, y, 1.f, 1.f, 1.f, 1.f, 1.f );
		y += 16;
	}
	
	SmallFont->DrawText( "Player", Rect.x + Rect.w/2 - 318, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f );
	SmallFont->DrawText( "Player", Rect.x + Rect.w/2 - 320, y, Font::ALIGN_MIDDLE_LEFT, 1.f, 1.f, 1.f, 1.f );
	if( objective )
	{
		SmallFont->DrawText( "Objective", Rect.x + Rect.w/2 + 82, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
		SmallFont->DrawText( "Objective", Rect.x + Rect.w/2 + 80, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f );
		SmallFont->DrawText( "Fighters", Rect.x + Rect.w/2 + 162, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
		SmallFont->DrawText( "Fighters", Rect.x + Rect.w/2 + 160, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f );
		SmallFont->DrawText( "Turrets", Rect.x + Rect.w/2 + 242, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
		SmallFont->DrawText( "Turrets", Rect.x + Rect.w/2 + 240, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f );
	}
	else
	{
		if( (gametype == XWing::GameType::TEAM_RACE) || (gametype == XWing::GameType::FFA_RACE) )
		{
			SmallFont->DrawText( "Checkpoints", Rect.x + Rect.w/2 + 162, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
			SmallFont->DrawText( "Checkpoints", Rect.x + Rect.w/2 + 160, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f );
		}
		SmallFont->DrawText( "Kills", Rect.x + Rect.w/2 + 242, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
		SmallFont->DrawText( "Kills", Rect.x + Rect.w/2 + 240, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f );
	}
	SmallFont->DrawText( "Deaths", Rect.x + Rect.w/2 + 322, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
	SmallFont->DrawText( "Deaths", Rect.x + Rect.w/2 + 320, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f );
	y += SmallFont->GetHeight();
	
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
		
		BigFont->DrawText( score_iter->PlayerData->Name, Rect.x + Rect.w/2 - 318, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( score_iter->PlayerData->Name, Rect.x + Rect.w/2 - 320, y, Font::ALIGN_MIDDLE_LEFT, red, green, blue, 1.f );
		if( objective )
		{
			BigFont->DrawText( Num::ToString(score_iter->CapitalKills), Rect.x + Rect.w/2 + 82, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( Num::ToString(score_iter->CapitalKills), Rect.x + Rect.w/2 + 80, y, Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f );
			BigFont->DrawText( Num::ToString(score_iter->Kills), Rect.x + Rect.w/2 + 162, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( Num::ToString(score_iter->Kills), Rect.x + Rect.w/2 + 160, y, Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f );
			BigFont->DrawText( Num::ToString(score_iter->TurretKills), Rect.x + Rect.w/2 + 242, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( Num::ToString(score_iter->TurretKills), Rect.x + Rect.w/2 + 240, y, Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f );
		}
		else
		{
			if( (gametype == XWing::GameType::TEAM_RACE) || (gametype == XWing::GameType::FFA_RACE) )
			{
				BigFont->DrawText( Num::ToString(score_iter->Score), Rect.x + Rect.w/2 + 162, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
				BigFont->DrawText( Num::ToString(score_iter->Score), Rect.x + Rect.w/2 + 160, y, Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f );
			}
			BigFont->DrawText( Num::ToString(score_iter->Kills), Rect.x + Rect.w/2 + 242, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( Num::ToString(score_iter->Kills), Rect.x + Rect.w/2 + 240, y, Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f );
		}
		BigFont->DrawText( Num::ToString(score_iter->Deaths), Rect.x + Rect.w/2 + 322, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( Num::ToString(score_iter->Deaths), Rect.x + Rect.w/2 + 320, y, Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f );
		
		y += BigFont->GetLineSkip();
	}
	
	if( Raptor::Game->Data.PropertyAsString("ai_score_kills").length() )
	{
		// FIXME: Should this appear in sorted order with the player scores?
		BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_name"),  Rect.x + Rect.w/2 - 318, y + 2, Font::ALIGN_MIDDLE_LEFT,  0.f,  0.f,  0.f,  0.8f );
		BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_name"),  Rect.x + Rect.w/2 - 320, y,     Font::ALIGN_MIDDLE_LEFT,  0.8f, 0.8f, 0.8f, 1.f );
		if( (gametype == XWing::GameType::TEAM_RACE) || (gametype == XWing::GameType::FFA_RACE) )
		{
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_kills"), Rect.x + Rect.w/2 + 162, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f,  0.f,  0.f,  0.8f );
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_kills"), Rect.x + Rect.w/2 + 160, y,     Font::ALIGN_MIDDLE_RIGHT, 0.8f, 0.8f, 0.8f, 1.f );
		}
		else
		{
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_kills"), Rect.x + Rect.w/2 + 242, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f,  0.f,  0.f,  0.8f );
			BigFont->DrawText( Raptor::Game->Data.PropertyAsString("ai_score_kills"), Rect.x + Rect.w/2 + 240, y,     Font::ALIGN_MIDDLE_RIGHT, 0.8f, 0.8f, 0.8f, 1.f );
		}
	}
	
	glPopMatrix();
}


#ifdef WIN32
void RenderLayer::UpdateSaitek( const Ship *ship, bool is_player, int view )
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
			snprintf( text_buffer, 17, "%-16.16s", target->Name.c_str() );
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


bool RenderLayer::HandleEvent( SDL_Event *event )
{
	if( Layer::HandleEvent( event ) )
		return true;
	if( MessageInput->IsSelected() )
		return false;
	
	XWingGame *game = (XWingGame*) Raptor::Game;
	uint8_t control = Raptor::Game->Input.EventBound( event );
	if( (control == game->Controls[ XWing::Control::CHAT ]) && IsTop() )
	{
		game->ReadKeyboard = false;
		
		Selected = MessageInput;
		MessageInput->Visible = true;
		
		return true;
	}
	else if( control == game->Controls[ XWing::Control::MENU ] )
	{
		Raptor::Game->Mouse.ShowCursor = true;
		game->ReadMouse = false;
		Raptor::Game->Layers.Add( new IngameMenu() );
		
		return true;
	}
	else if( control == game->Controls[ XWing::Control::PREFS ] )
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
}

Renderable::Renderable( Shot *shot )
{
	ShipPtr = NULL;
	ShotPtr = shot;
	EffectPtr = NULL;
	EnginePtr = NULL;
	EngineAlpha = 0.f;
}

Renderable::Renderable( Effect *effect )
{
	ShipPtr = NULL;
	ShotPtr = NULL;
	EffectPtr = effect;
	EnginePtr = NULL;
	EngineAlpha = 0.f;
}

Renderable::Renderable( ShipEngine *engine, const Pos3D *pos, float alpha )
{
	ShipPtr = NULL;
	ShotPtr = NULL;
	EffectPtr = NULL;
	EnginePtr = engine;
	EnginePos.Copy( pos );
	EngineAlpha = alpha;
}

Renderable::~Renderable()
{
}
