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
#include "Num.h"
#include "Math3D.h"
#include "IngameMenu.h"
#include "DeathStar.h"
#include "DeathStarBox.h"

#ifdef WIN32
#include "SaitekX52Pro.h"
#endif


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
	
	MessageOutput = NULL;
	if( ! Raptor::Game->Cfg.SettingAsBool("screensaver") )
	{
		MessageOutput = new MessageOverlay( Raptor::Game->Res.GetFont( "AgencyFB.ttf", 21 ) );
		AddElement( MessageOutput );
	}
	
	AddElement( MessageInput = new TextBox( NULL, Raptor::Game->Res.GetFont( "SegoeUI.ttf", 16 ), Font::ALIGN_TOP_LEFT ) );
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


void RenderLayer::SetBackground( void )
{
	if( BackgroundName != Raptor::Game->Data.Properties["bg"] )
	{
		BackgroundName = Raptor::Game->Data.Properties["bg"];
		
		if( ! Raptor::Game->Data.Properties["bg"].empty() )
			Background.BecomeInstance( Raptor::Game->Res.GetAnimation( Raptor::Game->Data.Properties["bg"] + std::string(".ani") ) );
		else
			Background.BecomeInstance( Raptor::Game->Res.GetAnimation("stars.ani") );
	}
}


void RenderLayer::SetWorldLights( float ambient_scale, const std::vector<const Vec3D*> *obstructions )
{
	Color ambient;
	Vec3D dir[ 4 ];
	Color color[ 4 ];
	float wrap_around[ 4 ];
	
	if( Raptor::Game->Data.Properties["bg"] == "nebula" )
	{
		ambient.Set( 0.75f, 0.75f, 0.75f, 1.f );
		
		dir[ 0 ].Set( 0.897, -0.389, 0.208 );
		dir[ 0 ].ScaleTo( 1. );
		color[ 0 ].Set( 1.3, 1.3, 1.29, 1.f );
		wrap_around[ 0 ] = 0.5;
		
		dir[ 1 ].Set( -0.748, 0.66, -0.07 );
		dir[ 1 ].ScaleTo( 1. );
		color[ 1 ].Set( 0.6, 0.1, 0.1, 1.f );
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
	else if( Raptor::Game->Data.Properties["gametype"] == "yavin" )
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
		
		dir[ 3 ].Set( 0.2, 0., 0.98 );
		dir[ 3 ].ScaleTo( 1. );
		color[ 3 ].Set( 0.25, 0.125, 0.2, 1.f );
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
			for( std::vector<const Vec3D*>::const_iterator obst_iter = obstructions->begin(); obst_iter != obstructions->end(); obst_iter ++ )
			{
				double obst_length = (*obst_iter)->Length();
				if( ! obst_length )
					continue;
				
				Vec3D obst( *obst_iter );
				if( obst_length > 1. )
					obst.ScaleTo( 1. );
				double similarity = dir[ i ].Dot( &obst );
				if( similarity < 0. )
					continue;
				
				wrap_around[ i ] -= similarity;
				if( wrap_around[ i ] < 0. )
					wrap_around[ i ] = 0.;
				
				dir[ i ] -= obst * similarity;
				color[ i ] *= dir[ i ].Length();
				dir[ i ].ScaleTo( 1. );
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
		((XWingGame*)( Raptor::Game ))->ReadMouse = true;
	}
	
	
	// Run performance test cases, if enabled.
	
	int test_shader_toggle = Raptor::Game->Cfg.SettingAsInt( "test_shader_toggle", 0 );
	for( int i = 0; i < test_shader_toggle; i ++ )
	{
		Raptor::Game->ShaderMgr.ResumeShaders();
		Raptor::Game->ShaderMgr.StopShaders();
	}
	
	int test_shader_vars = Raptor::Game->Cfg.SettingAsInt( "test_shader_vars", 0 );
	if( test_shader_vars )
	{
		Raptor::Game->ShaderMgr.ResumeShaders();
		for( int i = 0; i < test_shader_vars; i ++ )
		{
			SetWorldLights();
			ClearWorldLights();
		}
		Raptor::Game->ShaderMgr.StopShaders();
	}
	
	
	// Build a list of all ships, because we'll refer to it often.
	// Also build a list of shots for use when determining dynamic lights.
	// And keep track of the Death Star trench location for darkening world lights.
	
	std::list<Ship*> ships;
	std::list<Shot*> shots;
	DeathStar *death_star = NULL;
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->Type() == XWing::Object::SHIP )
			ships.push_back( (Ship*) obj_iter->second );
		else if( obj_iter->second->Type() == XWing::Object::SHOT )
			shots.push_back( (Shot*) obj_iter->second );
		else if( obj_iter->second->Type() == XWing::Object::DEATH_STAR )
			death_star = (DeathStar*) obj_iter->second;
	}
	
	
	// Determine which ship we're observing.
	
	Ship *observed_ship = NULL;
	Ship *player_ship = NULL;
	GameObject *target_obj = NULL;
	Ship *target = NULL;
	int view = VIEW_CINEMA;
	
	
	// Look for the player's ship.

	for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
	{
		if( (*ship_iter)->PlayerID == Raptor::Game->PlayerID )
		{
			player_ship = *ship_iter;
			
			// Only observe this if the ship is alive or recently dead.
			if( ((*ship_iter)->Health > 0.) || ((*ship_iter)->DeathClock.ElapsedSeconds() < 6.) )
			{
				observed_ship = *ship_iter;
				view = VIEW_COCKPIT;
			}
			break;
		}
	}
	
	
	if( ! observed_ship )
	{
		// This player has no ship, let's watch somebody else.
		
		// First try to observe a specific ship ID (who we were watching before).
		if( ((XWingGame*)( Raptor::Game ))->ObservedShipID )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->ShipType == Ship::TYPE_EXHAUST_PORT )
					continue;
				
				// Don't observe long-dead ships.
				if( ((*ship_iter)->Health <= 0.) && ((*ship_iter)->DeathClock.ElapsedSeconds() >= 6.) )
					continue;
				
				// If we'd selected a specific ship to watch, keep going until we find it.
				if( (*ship_iter)->ID == ((XWingGame*)( Raptor::Game ))->ObservedShipID )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// Try to observe anyone in the player's spawn group.
		if( (! observed_ship) && player_ship && player_ship->Team && player_ship->Group )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->ShipType == Ship::TYPE_EXHAUST_PORT )
					continue;
				
				// Don't observe long-dead ships.
				if( ((*ship_iter)->Health <= 0.) && ((*ship_iter)->DeathClock.ElapsedSeconds() >= 6.) )
					continue;
				
				// If our spawn group has other ships, watch them.
				if( ((*ship_iter)->Team == player_ship->Team) && ((*ship_iter)->Group == player_ship->Group) )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// Try to observe the next ship alive on the player's team after the ID we were observing.
		if( (! observed_ship) && player_ship && player_ship->Team )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->ShipType == Ship::TYPE_EXHAUST_PORT )
					continue;
				
				// Don't start observing dead ships.
				if( (*ship_iter)->Health <= 0. )
					continue;
				
				// If we'd selected a specific ship to watch, keep going until we find it.
				if( ((*ship_iter)->Team == player_ship->Team) && ((*ship_iter)->ID >= ((XWingGame*)( Raptor::Game ))->ObservedShipID) )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// Try to observe anyone alive on the player's team.
		if( (! observed_ship) && player_ship && player_ship->Team )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->ShipType == Ship::TYPE_EXHAUST_PORT )
					continue;
				
				// Don't start observing dead ships.
				if( (*ship_iter)->Health <= 0. )
					continue;
				
				// If our team has other ships, watch them.
				if( (*ship_iter)->Team == player_ship->Team )
				{
					observed_ship = *ship_iter;
					break;
				}
			}
		}
		
		// Try to observe anyone alive or recently dead on the player's team.
		if( (! observed_ship) && player_ship && player_ship->Team )
		{
			for( std::list<Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
			{
				// Don't spectate the Death Star exhaust port.
				if( (*ship_iter)->ShipType == Ship::TYPE_EXHAUST_PORT )
					continue;
				
				// Don't observe long-dead ships.
				if( ((*ship_iter)->Health <= 0.) && ((*ship_iter)->DeathClock.ElapsedSeconds() >= 6.) )
					continue;
				
				// If our team has other ships, watch them.
				if( (*ship_iter)->Team == player_ship->Team )
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
				if( (*ship_iter)->ShipType == Ship::TYPE_EXHAUST_PORT )
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
				if( (*ship_iter)->ShipType == Ship::TYPE_EXHAUST_PORT )
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
				if( (*ship_iter)->ShipType == Ship::TYPE_EXHAUST_PORT )
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
				if( (*ship_iter)->ShipType == Ship::TYPE_EXHAUST_PORT )
					continue;
				
				// Don't observe long-dead ships.
				if( ((*ship_iter)->Health <= 0.) && ((*ship_iter)->DeathClock.ElapsedSeconds() >= 6.) )
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
				view = VIEW_CHASE;
		}
		
		
		Cam.Copy( observed_ship );
		
		if( (view != VIEW_COCKPIT) || Raptor::Game->Cfg.SettingAsBool("g_3d_cockpit",true) )
		{
			// Apply camera angle change.
			Cam.Yaw( ((XWingGame*)( Raptor::Game ))->LookYaw );
			Cam.Pitch( ((XWingGame*)( Raptor::Game ))->LookPitch );
		}
		
		if( (view == VIEW_CINEMA) || (view == VIEW_CINEMA2) )
		{
			// Cinematic camera.
			
			if( view == VIEW_CINEMA2 )
				Cam.Up.Set( 0., 0., 1. );
			
			// Point the camera at one ship looking through to the other.
			Vec3D vec_to_other( cinema_view_with->X - observed_ship->X, cinema_view_with->Y - observed_ship->Y, cinema_view_with->Z - observed_ship->Z );
			vec_to_other.ScaleTo( 1. );
			Cam.Fwd = vec_to_other;
			Cam.FixVectors();
			
			// Apply camera angle change.
			Cam.Yaw( ((XWingGame*)( Raptor::Game ))->LookYaw );
			Cam.Pitch( ((XWingGame*)( Raptor::Game ))->LookPitch );
			
			// Move the camera up or down.
			Cam.MoveAlong( &(Cam.Fwd), -30. - observed_ship->Shape.GetTriagonal() );
			if( view == VIEW_CINEMA2 )
				Cam.MoveAlong( &(Cam.Up), observed_ship->Shape.GetTriagonal() * Vec3D(0.,0.,1.).Dot(&vec_to_other) + observed_ship->Shape.GetHeight() );
			else
				Cam.MoveAlong( &(Cam.Up), observed_ship->Shape.GetTriagonal() * observed_ship->Up.Dot(&vec_to_other) + observed_ship->Shape.GetHeight() );
			
			// Point the camera at the mid-point (in 3D space) between the two ships.
			Vec3D mid_point( (cinema_view_with->X + observed_ship->X) / 2., (cinema_view_with->Y + observed_ship->Y) / 2., (cinema_view_with->Z + observed_ship->Z) / 2. );
			Vec3D vec_to_mid( mid_point.X - Cam.X, mid_point.Y - Cam.Y, mid_point.Z - Cam.Z);
			vec_to_mid.ScaleTo( 1. );
			Cam.Fwd = vec_to_mid;
			Cam.FixVectors();
			
			// Apply camera angle change again.
			Cam.Yaw( ((XWingGame*)( Raptor::Game ))->LookYaw );
			Cam.Pitch( ((XWingGame*)( Raptor::Game ))->LookPitch );
		}
		
		else if( view == VIEW_CHASE )
		{
			// Chase camera.
			
			Cam.MoveAlong( &(Cam.Fwd), -20. - observed_ship->Shape.GetTriagonal() );
		}
	}
	else
		((XWingGame*)( Raptor::Game ))->ObservedShipID = 0;
	
	// This allows head tracking to happen after camera placement.
	Raptor::Game->Cam.Copy( &Cam );
	
	
	// Set up shaders.
	
	bool use_shaders = Raptor::Game->Cfg.SettingAsBool("g_shader_enable");
	if( use_shaders )
	{
		Raptor::Game->ShaderMgr.ResumeShaders();
		Raptor::Game->ShaderMgr.Set3f( "CamPos", Raptor::Game->Cam.X, Raptor::Game->Cam.Y, Raptor::Game->Cam.Z );
		SetWorldLights();
		ClearDynamicLights();
		Raptor::Game->ShaderMgr.StopShaders();
	}
	
	
	// Render to textures before drawing anything else.
	
	bool need_target_holo = Raptor::Game->FrameTime;
	bool use_framebuffers = Raptor::Game->Cfg.SettingAsBool("g_framebuffers");
	if( use_framebuffers && Raptor::Game->FrameTime )
	{
		bool changed_framebuffer = false;
		
		if( observed_ship && (observed_ship->Health > 0.) && ((view == VIEW_COCKPIT) || Raptor::Game->Cfg.SettingAsBool("saitek_enable")) )
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
				
				float red = 0.f, green = 1.f, blue = 0.f;
				if( observed_ship->Health < observed_ship->MaxHealth() )
					red = 1.f;
				if( observed_ship->Health < (observed_ship->MaxHealth() * 0.7) )
					green = 0.f;
				
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
				need_target_holo = false;
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
					
					if( target_obj->Type() == XWing::Object::SHIP )
					{
						if( use_shaders )
						{
							Raptor::Game->ShaderMgr.Select( Raptor::Game->Res.GetShader("model_hud") );
							Raptor::Game->ShaderMgr.ResumeShaders();
							Raptor::Game->ShaderMgr.Set3f( "AmbientLight", 0., 0., 0. );
						}
						
						target_obj->Draw();
						
						if( use_shaders )
						{
							Raptor::Game->ShaderMgr.StopShaders();
							Raptor::Game->ShaderMgr.Select( Raptor::Game->Res.GetShader("model") );
						}
						
						glLineWidth( 2.f );
						((Ship*)( target_obj ))->DrawWireframe();
						
						
					}
					else
					{
						if( use_shaders )
							Raptor::Game->ShaderMgr.ResumeShaders();
						
						target_obj->Draw();
						
						if( use_shaders )
							Raptor::Game->ShaderMgr.StopShaders();
					}
					
					
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
				
				
				glLineWidth( 2.f );
				
				
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
			
			
			Framebuffer *throttle_framebuffer = Raptor::Game->Res.GetFramebuffer( "throttle" );
			if( throttle_framebuffer && throttle_framebuffer->Select() )
			{
				// Render throttle display.
				
				changed_framebuffer = true;
				Raptor::Game->Gfx.Clear( 0.f, 0.f, 0.f );
				
				
				throttle_framebuffer->Setup2D( 0., 0., 1., 1. );
				
				Raptor::Game->Gfx.DrawRect2D( 0., 1. - observed_ship->GetThrottle(), 1., 1., 0, 0.f, 0.5f, 1.f, 1.f );
				
				
				// Draw noise if we're damaged.
				
				if( observed_ship->Health < (observed_ship->MaxHealth() * 0.5) )
				{
					throttle_framebuffer->Setup2D();
					
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
	
	
	// Set up 3D rendering for the scene.
	
	Raptor::Game->Cam.FOV = vr ? Raptor::Game->Cfg.SettingAsDouble("vr_fov") : Raptor::Game->Cfg.SettingAsDouble("g_fov");
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
	
	SetBackground();
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
		
		bool g_3d_cockpit = Raptor::Game->Cfg.SettingAsBool("g_3d_cockpit",true);
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
			
			bool change_light_for_cockpit = false;
			if( Raptor::Game->ShaderMgr.Active() )
			{
				float ambient_scale = 1.f;
				std::vector<const Vec3D*> obstructions;
				if( death_star )
				{
					// Reduce cockpit ambient light if we're in the trench.
					double dist_in_trench = -1. * observed_ship->DistAlong( &(death_star->Up), death_star );
					if( dist_in_trench > 0. )
					{
						ambient_scale = std::max<float>( 0.2f, 1. - dist_in_trench * 1.5 / death_star->TrenchDepth );
						change_light_for_cockpit = true;
					}
				}
				
				if( change_light_for_cockpit )
					SetWorldLights( ambient_scale, obstructions.size() ? &obstructions : NULL );
				
				if( dynamic_lights )
				{
					Shot *nearest_shot = NULL;
					std::set<Pos3D*> used_lights;
					char uniform_name[ 128 ] = "";
					
					ClearDynamicLights();
					
					for( int i = 0; i < dynamic_lights; i ++ )
					{
						nearest_shot = (Shot*) observed_ship->Nearest( (std::list<Pos3D*> *) &shots, i ? &used_lights : NULL );
						if( nearest_shot )
						{
							snprintf( uniform_name, 128, "PointLight%iPos", i );
							Raptor::Game->ShaderMgr.Set3f( uniform_name, nearest_shot->X - observed_ship->X, nearest_shot->Y - observed_ship->Y, nearest_shot->Z - observed_ship->Z );
							
							Color dynamic_light_color = nearest_shot->LightColor();
							snprintf( uniform_name, 128, "PointLight%iColor", i );
							Raptor::Game->ShaderMgr.Set3f( uniform_name, dynamic_light_color.Red, dynamic_light_color.Green, dynamic_light_color.Blue );
							snprintf( uniform_name, 128, "PointLight%iRadius", i );
							Raptor::Game->ShaderMgr.Set1f( uniform_name, dynamic_light_color.Alpha );
							
							used_lights.insert( nearest_shot );
						}
						else
							break;
					}
				}
			}
			
			// Technically the correct scale is 0.022, but we'll probably need to make it higher to avoid near-plane clipping.
			double cockpit_scale = vr ? 0.022 : std::max<double>( 0.022, Raptor::Game->Gfx.ZNear * 0.25 );
			double cockpit_fwd = 0.;
			double cockpit_up = 0.;
			double cockpit_right = 0.;
			
			if( observed_ship->ShipType == Ship::TYPE_XWING )
			{
				cockpit_fwd = (g_hq_cockpit ? 32. : 12.);
				cockpit_up = (g_hq_cockpit ? -37. : -27.);
				cockpit_right = (g_hq_cockpit ? 0. : -0.75);
			}
			else if( observed_ship->ShipType == Ship::TYPE_YWING )
			{
				cockpit_fwd = (g_hq_cockpit ? -190. : -300.);
				cockpit_up = (g_hq_cockpit ? -38.: -38.);
				cockpit_right = (g_hq_cockpit ? 0. : 1.5);
			}
			else if( observed_ship->ShipType == Ship::TYPE_TIE_FIGHTER )
			{
				cockpit_fwd = (g_hq_cockpit ? -14. : 0.);
			}
			
			// Draw the cockpit as though it were at 0,0,0 to avoid Z-fighting issues.
			Pos3D cockpit_pos( observed_ship );
			cockpit_pos.SetPos(0,0,0);
			cockpit_pos.FixVectors();
			Raptor::Game->Cam.Copy( &cockpit_pos );
			Raptor::Game->Gfx.Setup3D( &(Raptor::Game->Cam) );
			if( use_shaders )
				Raptor::Game->ShaderMgr.Set3f( "CamPos", Raptor::Game->Cam.X, Raptor::Game->Cam.Y, Raptor::Game->Cam.Z );
			cockpit_pos.MoveAlong( &(cockpit_pos.Fwd), cockpit_fwd * cockpit_scale );
			cockpit_pos.MoveAlong( &(cockpit_pos.Up), cockpit_up * cockpit_scale );
			cockpit_pos.MoveAlong( &(cockpit_pos.Right), cockpit_right * cockpit_scale );
			cockpit_3d->DrawAt( &cockpit_pos, cockpit_scale );
			
			// Restore camera position for scene render.
			Raptor::Game->Cam.Copy( &Cam );
			Raptor::Game->Gfx.Setup3D( &(Raptor::Game->Cam) );
			
			// Reset world lights to normal.
			if( change_light_for_cockpit )
				SetWorldLights();
			
			if( use_shaders )
			{
				// Restore camera position in shaders for lighting effects.
				Raptor::Game->ShaderMgr.Set3f( "CamPos", Raptor::Game->Cam.X, Raptor::Game->Cam.Y, Raptor::Game->Cam.Z );
				Raptor::Game->ShaderMgr.StopShaders();
			}
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
	
	double draw_dist = Raptor::Game->Cfg.SettingAsDouble( "g_draw_dist", 6000. );
	
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
			
			// Don't draw tiny ships far away.
			double size = ship->Shape.GetTriagonal();
			if( (Raptor::Game->Cam.Dist(ship) * (vr ? 20. : 10.) / size) > draw_dist )
				continue;
			
			// Don't draw ships that are entirely behind us.
			if( ship->DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ) < -size )
				continue;
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
			// Recast the list of Shot pointers to a list of Pos3D pointers.
			std::list<Pos3D*> *shot_list = (std::list<Pos3D*> *) &shots;
			Shot *nearest_shot = NULL;
			std::set<Pos3D*> used_lights;
			char uniform_name[ 128 ] = "";
			
			ClearDynamicLights();
			
			for( int i = 0; i < dynamic_lights; i ++ )
			{
				// If we're drawing the Death Star, the dynamic light should be the nearest shot to the camera.
				if( obj_iter->second->Type() == XWing::Object::DEATH_STAR )
					nearest_shot = (Shot*) Raptor::Game->Cam.Nearest( shot_list, i ? &used_lights : NULL );
				else
					nearest_shot = (Shot*) obj_iter->second->Nearest( shot_list, i ? &used_lights : NULL );
				
				if( nearest_shot )
				{
					snprintf( uniform_name, 128, "PointLight%iPos", i );
					Raptor::Game->ShaderMgr.Set3f( uniform_name, nearest_shot->X, nearest_shot->Y, nearest_shot->Z );
					
					Color dynamic_light_color = nearest_shot->LightColor();
					snprintf( uniform_name, 128, "PointLight%iColor", i );
					Raptor::Game->ShaderMgr.Set3f( uniform_name, dynamic_light_color.Red, dynamic_light_color.Green, dynamic_light_color.Blue );
					
					snprintf( uniform_name, 128, "PointLight%iRadius", i );
					Raptor::Game->ShaderMgr.Set1f( uniform_name, dynamic_light_color.Alpha );
					
					used_lights.insert( nearest_shot );
				}
				else
					break;
			}
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
			
			Raptor::Game->Gfx.DrawLine3D( pos.X - right.X + up.X, pos.Y - right.Y + up.Y, pos.Z - right.Z + up.Z, pos.X - right.X / 2. + up.X, pos.Y - right.Y / 2. + up.Y, pos.Z - right.Z / 2. + up.Z, 1.f, 1.f, 1.f, 1.f, 1.f );
			Raptor::Game->Gfx.DrawLine3D( pos.X - right.X + up.X, pos.Y - right.Y + up.Y, pos.Z - right.Z + up.Z, pos.X - right.X + up.X / 2., pos.Y - right.Y + up.Y / 2., pos.Z - right.Z + up.Z / 2., 1.f, 1.f, 1.f, 1.f, 1.f );
			
			Raptor::Game->Gfx.DrawLine3D( pos.X + right.X + up.X, pos.Y + right.Y + up.Y, pos.Z + right.Z + up.Z, pos.X + right.X / 2. + up.X, pos.Y + right.Y / 2. + up.Y, pos.Z + right.Z / 2. + up.Z, 1.f, 1.f, 1.f, 1.f, 1.f );
			Raptor::Game->Gfx.DrawLine3D( pos.X + right.X + up.X, pos.Y + right.Y + up.Y, pos.Z + right.Z + up.Z, pos.X + right.X + up.X / 2., pos.Y + right.Y + up.Y / 2., pos.Z + right.Z + up.Z / 2., 1.f, 1.f, 1.f, 1.f, 1.f );
			
			Raptor::Game->Gfx.DrawLine3D( pos.X + right.X - up.X, pos.Y + right.Y - up.Y, pos.Z + right.Z - up.Z, pos.X + right.X / 2. - up.X, pos.Y + right.Y / 2. - up.Y, pos.Z + right.Z / 2. - up.Z, 1.f, 1.f, 1.f, 1.f, 1.f );
			Raptor::Game->Gfx.DrawLine3D( pos.X + right.X - up.X, pos.Y + right.Y - up.Y, pos.Z + right.Z - up.Z, pos.X + right.X - up.X / 2., pos.Y + right.Y - up.Y / 2., pos.Z + right.Z - up.Z / 2., 1.f, 1.f, 1.f, 1.f, 1.f );
			
			Raptor::Game->Gfx.DrawLine3D( pos.X - right.X - up.X, pos.Y - right.Y - up.Y, pos.Z - right.Z - up.Z, pos.X - right.X / 2. - up.X, pos.Y - right.Y / 2. - up.Y, pos.Z - right.Z / 2. - up.Z, 1.f, 1.f, 1.f, 1.f, 1.f );
			Raptor::Game->Gfx.DrawLine3D( pos.X - right.X - up.X, pos.Y - right.Y - up.Y, pos.Z - right.Z - up.Z, pos.X - right.X - up.X / 2., pos.Y - right.Y - up.Y / 2., pos.Z - right.Z - up.Z / 2., 1.f, 1.f, 1.f, 1.f, 1.f );
			
			glEnable( GL_DEPTH_TEST );
		}
		
		
		// Draw the 3D crosshair.
		{
			glDisable( GL_DEPTH_TEST );
			glDisable( GL_TEXTURE_2D );
			
			Pos3D crosshair_pos( observed_ship );
			crosshair_pos.MoveAlong( &(crosshair_pos.Fwd), 100. );
			glColor4f( crosshair_red, crosshair_green, crosshair_blue, 1.f );
			
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
				
				std::map<int,Shot*> all_weapons = observed_ship->AllShots();
				for( std::map<int,Shot*>::iterator shot_iter = all_weapons.begin(); shot_iter != all_weapons.end(); shot_iter ++ )
				{
					weapon_vec.Set( shot_iter->second->X - observed_ship->X, shot_iter->second->Y - observed_ship->Y, shot_iter->second->Z - observed_ship->Z );
					relative_weapon_vec.Set( weapon_vec.Dot(&(observed_ship->Right)), weapon_vec.Dot(&(observed_ship->Up)) );
					relative_weapon_vec.ScaleTo( 1. );
					weapon_pos.Copy( &crosshair_pos );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X * 2. );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y * 2. );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
					
					//delete shot_iter->second;
				}
			glEnd();
			
			// Draw little dots for all selected weapon ports.
			glPointSize( 2.f );
			glBegin( GL_POINTS );
				for( std::map<int,Shot*>::iterator shot_iter = all_weapons.begin(); shot_iter != all_weapons.end(); shot_iter ++ )
				{
					weapon_vec.Set( shot_iter->second->X - observed_ship->X, shot_iter->second->Y - observed_ship->Y, shot_iter->second->Z - observed_ship->Z );
					relative_weapon_vec.Set( weapon_vec.Dot(&(observed_ship->Right)), weapon_vec.Dot(&(observed_ship->Up)) );
					relative_weapon_vec.ScaleTo( 1. );
					weapon_pos.Copy( &crosshair_pos );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X * 3.6 );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y * 3.6 );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
					
					delete shot_iter->second;
				}
			glEnd();
			
			// Draw big dots for the next shots to fire.
			glPointSize( 4.f );
			glBegin( GL_POINTS );
				std::map<int,Shot*> next_weapons = observed_ship->NextShots();
				for( std::map<int,Shot*>::iterator shot_iter = next_weapons.begin(); shot_iter != next_weapons.end(); shot_iter ++ )
				{
					weapon_vec.Set( shot_iter->second->X - observed_ship->X, shot_iter->second->Y - observed_ship->Y, shot_iter->second->Z - observed_ship->Z );
					relative_weapon_vec.Set( weapon_vec.Dot(&(observed_ship->Right)), weapon_vec.Dot(&(observed_ship->Up)) );
					relative_weapon_vec.ScaleTo( 1. );
					weapon_pos.Copy( &crosshair_pos );
					
					weapon_pos.MoveAlong( &right, relative_weapon_vec.X * 3.6 );
					weapon_pos.MoveAlong( &up, relative_weapon_vec.Y * 3.6 );
					glVertex3d( weapon_pos.X, weapon_pos.Y, weapon_pos.Z );
					
					delete shot_iter->second;
				}
			glEnd();
			
			if( ammo >= 0 )
			{
				crosshair_pos.MoveAlong( &up, -1. );
				ScreenFont->DrawText3D( Num::ToString(ammo), &crosshair_pos, Font::ALIGN_TOP_CENTER, 0.05 );
			}
			
			glEnable( GL_DEPTH_TEST );
		}
		
		
		if( cockpit_2d && ! cockpit_3d && ! vr )
		{
			// Draw the 2D cockpit.
			Raptor::Game->Gfx.Setup2D();
			Raptor::Game->Gfx.DrawRect2D( Rect.w / 2 - Rect.h, 0, Rect.w / 2 + Rect.h, Rect.h, cockpit_2d->CurrentFrame(), 1.f, 1.f, 1.f, 1.f );
			
			// When drawing the 2D cockpit, use the target hologram.
			need_target_holo = true;
		}
		
		
		// Here we'll draw target info, unless it's done via cockpit screens.
		
		if( target && need_target_holo )
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
				ClearDynamicLights();
			
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
			
			BigFont->DrawText( target_name, Rect.x + Rect.w/2 + 2, Rect.h - 8, Font::ALIGN_BOTTOM_CENTER, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( target_name, Rect.x + Rect.w/2, Rect.h - 10, Font::ALIGN_BOTTOM_CENTER, red, green, blue, 1.f );
		}
		
		
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
			
			Raptor::Game->Gfx.DrawBox2D( x - 0.01, y - 0.01, x + 0.01, y + 0.01, 1.f, 1.f, 1.f, 1.f, 1.f );
			Raptor::Game->Gfx.DrawCircle2D( x, y, radius, 6, 0, red, green, blue, 1.f );
		}
	}
	
	
	// If we're spectating, show who we're watching.
	
	if( (observed_player && (observed_player->ID == Raptor::Game->PlayerID)) || Raptor::Game->Cfg.SettingAsBool("screensaver") || vr )
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
	
	
	// If the round is over, show the scores.
	
	if( ((Raptor::Game->State >= XWing::State::ROUND_ENDED) || Raptor::Game->Keys.KeyDown(SDLK_TAB)) && (! Raptor::Game->Cfg.SettingAsBool("screensaver")) )
		DrawScores();
	
	
	// Don't show messages in VR unless the chat box is open.
	
	if( MessageOutput )
		MessageOutput->Visible = vr ? MessageInput->Visible : true;
	
	
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
	
	bool ffa = ( Raptor::Game->Data.Properties["gametype"].find("ffa_") == 0 );
	
	if( ! ffa )
	{
		BigFont->DrawText( "Rebels", Rect.x + Rect.w/2 - 298, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( "Rebels", Rect.x + Rect.w/2 - 300, y, Font::ALIGN_MIDDLE_LEFT, 1.f, 0.25f, 0.25f, 1.f );
		BigFont->DrawText( "vs", Rect.x + Rect.w/2 + 2, y + 2, Font::ALIGN_MIDDLE_CENTER, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( "vs", Rect.x + Rect.w/2, y, Font::ALIGN_MIDDLE_CENTER, 0.75f, 0.75f, 0.75f, 1.f );
		BigFont->DrawText( "Empire", Rect.x + Rect.w/2 + 302, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
		BigFont->DrawText( "Empire", Rect.x + Rect.w/2 + 300, y, Font::ALIGN_MIDDLE_RIGHT, 0.25f, 0.25f, 1.f, 1.f );
		
		if( Raptor::Game->Data.Properties["gametype"] == "team_dm" )
		{
			BigFont->DrawText( Raptor::Game->Data.Properties["team_score_rebel"], Rect.x + Rect.w/2 - 14, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( Raptor::Game->Data.Properties["team_score_rebel"], Rect.x + Rect.w/2 - 16, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 0.25f, 0.25f, 1.f );
			BigFont->DrawText( Raptor::Game->Data.Properties["team_score_empire"], Rect.x + Rect.w/2 + 18, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( Raptor::Game->Data.Properties["team_score_empire"], Rect.x + Rect.w/2 + 16, y, Font::ALIGN_MIDDLE_LEFT, 0.25f, 0.25f, 1.f, 1.f );
		}
		
		y += BigFont->GetHeight() - 8;
		Raptor::Game->Gfx.DrawLine2D( Raptor::Game->Gfx.W / 2 - 298, y + 2, Rect.x + Rect.w/2 + 302, y + 2, 1.f, 0.f, 0.f, 0.f, 0.8f );
		Raptor::Game->Gfx.DrawLine2D( Raptor::Game->Gfx.W / 2 - 300, y, Rect.x + Rect.w/2 + 300, y, 1.f, 1.f, 1.f, 1.f, 1.f );
		y += 16;
	}
	
	SmallFont->DrawText( "Player", Rect.x + Rect.w/2 - 298, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f );
	SmallFont->DrawText( "Player", Rect.x + Rect.w/2 - 300, y, Font::ALIGN_MIDDLE_LEFT, 1.f, 1.f, 1.f, 1.f );
	SmallFont->DrawText( "Kills", Rect.x + Rect.w/2 + 202, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
	SmallFont->DrawText( "Kills", Rect.x + Rect.w/2 + 200, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f );
	SmallFont->DrawText( "Deaths", Rect.x + Rect.w/2 + 302, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
	SmallFont->DrawText( "Deaths", Rect.x + Rect.w/2 + 300, y, Font::ALIGN_MIDDLE_RIGHT, 1.f, 1.f, 1.f, 1.f );
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
			
			BigFont->DrawText( str, Rect.x + Rect.w/2 - 298, y + 2, Font::ALIGN_MIDDLE_LEFT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( str, Rect.x + Rect.w/2 - 300, y, Font::ALIGN_MIDDLE_LEFT, red, green, blue, 1.f );
			BigFont->DrawText( Num::ToString(score_iter->first), Rect.x + Rect.w/2 + 202, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( Num::ToString(score_iter->first), Rect.x + Rect.w/2 + 200, y, Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f );
			BigFont->DrawText( (*player_iter)->Properties["deaths"], Rect.x + Rect.w/2 + 302, y + 2, Font::ALIGN_MIDDLE_RIGHT, 0.f, 0.f, 0.f, 0.8f );
			BigFont->DrawText( (*player_iter)->Properties["deaths"], Rect.x + Rect.w/2 + 300, y, Font::ALIGN_MIDDLE_RIGHT, red, green, blue, 1.f );
			
			y += BigFont->GetLineSkip();
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
		Ship *target = NULL;
		if( ship->Target )
		{
			GameObject *obj = Raptor::Game->Data.GetObject( ship->Target );
			if( obj && (obj->Type() == XWing::Object::SHIP) )
				target = (Ship*) obj;
		}
		
		std::map<uint32_t,int8_t>::const_iterator ammo_iter = ship->Ammo.find( ship->SelectedWeapon );
		if( (ammo_iter != ship->Ammo.end()) && (ammo_iter->second == 0) )
			Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::Fire, (fmod( ship->Lifetime.ElapsedSeconds(), 0.5 ) >= 0.25) );
		else
			Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::Fire, true );
		
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::Hat2Green, true );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::AGreen, true );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::BGreen, true );
		if( ship->SelectedWeapon == Shot::TYPE_TORPEDO )
		{
			Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::Hat2Red, true );
			Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::ARed, true );
			Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::BRed, true );
		}
		else
		{
			Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::Hat2Red, false );
			Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::ARed, false );
			Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::BRed, false );
		}
		
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::T1Green, true );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::T3Green, true );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::T5Green, true );
		
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::EGreen, true );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::DGreen, true );
		Raptor::Game->Saitek.SetX52ProLED( SaitekX52ProLED::ClutchGreen, true );
		
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
			double dist = ship->Dist(target);
			if( dist >= 98950. )
				snprintf( text_buffer + 12, 5, ">99K" );
			else if( dist >= 9950. )
				snprintf( text_buffer + 11, 6, "%4.1fK", dist / 1000. );
			else if( dist >= 999.5 )
				snprintf( text_buffer + 12, 5, "%3.1fK", dist / 1000. );
			else
				snprintf( text_buffer + 13, 4, "%3.0f", dist );
		}
		Raptor::Game->Saitek.SetX52ProMFD( 2, text_buffer );
		
		
		if( Raptor::Game->Cfg.SettingAsBool( "g_framebuffers", true ) )
		{
			Framebuffer *fb = Raptor::Game->Res.GetFramebuffer( "fip", 320, 240 );
			if( fb && fb->Select() )
			{
				fb->Setup2D( 0, 240, 320, 0 );
				glColor4f( 1.f, 1.f, 1.f, 1.f );
				
				Raptor::Game->Gfx.Clear();
				Raptor::Game->Gfx.DrawRect2D( 120, 0, 300, 240, Raptor::Game->Res.GetTexture("*target") );
				Raptor::Game->Gfx.DrawRect2D( 0, 40, 120, 200, Raptor::Game->Res.GetTexture("*health") );
				Raptor::Game->Gfx.DrawRect2D( 300, 0, 320, 240, Raptor::Game->Res.GetTexture("*throttle") );
				Raptor::Game->Saitek.SetFIPImage( fb, 0 );
				
				Raptor::Game->Gfx.Clear();
				Raptor::Game->Gfx.DrawRect2D( 0, 0, 320, 240, Raptor::Game->Res.GetTexture("*intercept") );
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
		
		return true;
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
