/*
 *  XWingServer.cpp
 */

#include "XWingServer.h"

#include <cstddef>
#include <cmath>
#include <stdint.h>
#include <list>
#include "XWingDefs.h"
#include "Packet.h"
#include "Rand.h"
#include "Num.h"
#include "Str.h"
#include "Ship.h"
#include "Shot.h"
#include "Asteroid.h"
#include "Turret.h"
#include "DeathStar.h"
#include "DeathStarBox.h"


XWingServer::XWingServer( std::string version ) : RaptorServer( "X-Wing Revival", version )
{
	GameType = XWing::GameType::TEAM_ELIMINATION;
	PlayersTakeEmptyShips = false;
	Respawn = false;
	RoundEndedDelay = 3.;
	KillLimit = 10;
	TimeLimit = 0;
	AIFlock = false;
	CountdownFrom = 5;
}


XWingServer::~XWingServer()
{
}


void XWingServer::Started( void )
{
	Data.Properties["gametype"] = "yavin";
	Data.Properties["ai_waves"] = "3";
	Data.Properties["ai_empire_ratio"] = "1";
	Data.Properties["ai_flock"] = "false";
	Data.Properties["bg"] = "stars";
	Data.Properties["respawn"] = "true";
	Data.Properties["dm_kill_limit"] = "10";
	Data.Properties["tdm_kill_limit"] = "20";
	Data.Properties["yavin_time_limit"] = "15";
	Data.Properties["yavin_turrets"] = "120";
	Data.Properties["asteroids"] = "32";
	Data.Properties["permissions"] = "all";
	Alerts.clear();
	Waypoints.clear();
	Squadrons.clear();
	
	State = XWing::State::LOBBY;
}


bool XWingServer::ProcessPacket( Packet *packet, ConnectedClient *from_client )
{
	packet->Rewind();
	PacketType type = packet->Type();
	
	if( type == XWing::Packet::FLY )
	{
		if( State >= XWing::State::FLYING )
			BeginFlying();
		else if( (Data.Players.size() <= 1) && (State < XWing::State::COUNTDOWN) )
			BeginFlying();
		else
			ToggleCountdown();
		
		return true;
	}
	
	return RaptorServer::ProcessPacket( packet, from_client );
}


void XWingServer::AcceptedClient( ConnectedClient *client )
{
	// Localhost client gets admin rights.
	if( client->PlayerID && (((client->IP & 0xFF000000) >> 24) == 127) )
	{
		Player *player = Data.GetPlayer( client->PlayerID );
		if( player )
			player->Properties["admin"] = "true";
	}
	
	RaptorServer::AcceptedClient( client );
	
	Packet lobby_packet( XWing::Packet::LOBBY );
	client->Send( &lobby_packet );
}


void XWingServer::DroppedClient( ConnectedClient *client )
{
	RaptorServer::DroppedClient( client );
}


void XWingServer::Update( double dt )
{
	if( State >= XWing::State::FLYING )
	{
		// ======
		// Flying
		// ======
		
		// Update all objects and find collisions.
		RaptorServer::Update( dt );
		
		// Keep track of any objects we add or delete.
		std::list<uint32_t> add_object_ids;
		std::list<uint32_t> remove_object_ids;
		
		// Gather info important to the update.
		double round_time_remaining = RoundTimeRemaining();
		
		
		// Build a list of all ships.
		
		std::map<uint32_t,Ship*> ships;
		std::map<uint32_t,Turret*> turrets;
		DeathStar *deathstar = NULL;
		
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHIP )
				ships[ obj_iter->second->ID ] = (Ship*) obj_iter->second;
			else if( obj_iter->second->Type() == XWing::Object::TURRET )
				turrets[ obj_iter->second->ID ] = (Turret*) obj_iter->second;
			else if( obj_iter->second->Type() == XWing::Object::DEATH_STAR )
				deathstar = (DeathStar*) obj_iter->second;
		}
		
		
		// Deal with collisions.
		
		bool send_scores = false;
		
		for( std::list<Collision>::iterator collision_iter = Data.Collisions.begin(); collision_iter != Data.Collisions.end(); collision_iter ++ )
		{
			if( (collision_iter->first->Type() == XWing::Object::SHIP) && (collision_iter->second->Type() == XWing::Object::SHIP) )
			{
				// Two ships collided.
				
				Ship *ship1 = (Ship*) collision_iter->first;
				Ship *ship2 = (Ship*) collision_iter->second;
				double prev_health1 = ship1->Health;
				double prev_health2 = ship2->Health;
				ship1->SetHealth( 0. );
				ship2->SetHealth( 0. );
				
				if( (prev_health1 > 0.) || (prev_health2 > 0.) )
				{
					send_scores = true;
					
					Player *player1 = Data.GetPlayer( ship1->PlayerID );
					Player *player2 = Data.GetPlayer( ship2->PlayerID );
					
					// Show an explosion to players.
					Packet explosion( XWing::Packet::EXPLOSION );
					explosion.AddDouble( (ship1->X + ship2->X) / 2. );
					explosion.AddDouble( (ship1->Y + ship2->Y) / 2. );
					explosion.AddDouble( (ship1->Z + ship2->Z) / 2. );
					explosion.AddFloat( (ship1->MotionVector.X + ship2->MotionVector.X) / 2. );
					explosion.AddFloat( (ship1->MotionVector.Y + ship2->MotionVector.Y) / 2. );
					explosion.AddFloat( (ship1->MotionVector.Z + ship2->MotionVector.Z) / 2. );
					explosion.AddFloat( 20.f );
					explosion.AddFloat( 1.5f );
					Net.SendAll( &explosion );
					
					if( State < XWing::State::ROUND_ENDED )
					{
						send_scores = true;
						
						// Adjust scores.
						if( player1 && (prev_health1 > 0.) )
							player1->Properties["deaths"] = Num::ToString( atoi( player1->Properties["deaths"].c_str() ) + 1 );
						if( player2 && (prev_health2 > 0.) )
							player2->Properties["deaths"] = Num::ToString( atoi( player2->Properties["deaths"].c_str() ) + 1 );
						
						// Notify players.
						Packet message( Raptor::Packet::MESSAGE );
						char cstr[ 1024 ] = "";
						if( (prev_health1 > 0.) && (prev_health2 > 0.) )
							snprintf( cstr, 1024, "%s and %s collided.", player1 ? player1->Name.c_str() : ship1->Name.c_str(), player2 ? player2->Name.c_str() : ship2->Name.c_str() );
						else if( prev_health1 > 0. )
							snprintf( cstr, 1024, "%s ran into pieces of %s.", player1 ? player1->Name.c_str() : ship1->Name.c_str(), player2 ? player2->Name.c_str() : ship2->Name.c_str() );
						else
							snprintf( cstr, 1024, "%s ran into pieces of %s.", player2 ? player2->Name.c_str() : ship2->Name.c_str(), player1 ? player1->Name.c_str() : ship1->Name.c_str() );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
			}
			
			else if( (collision_iter->first->Type() == XWing::Object::SHIP) && (collision_iter->second->Type() == XWing::Object::SHOT) )
			{
				Shot *shot = (Shot*) collision_iter->second;
				Ship *ship = (Ship*) collision_iter->first;
				
				double prev_health = ship->Health;
				Vec3D vec_to_shot( shot->PrevPos.X - ship->X, shot->PrevPos.Y - ship->Y, shot->PrevPos.Z - ship->Z );
				if( ship->Fwd.Dot(&vec_to_shot) >= 0. )
					ship->AddDamage( shot->Damage(), 0. );
				else
					ship->AddDamage( 0., shot->Damage() );
				
				if( prev_health > 0. )
				{
					// Send the hit.
					Packet shot_hit( XWing::Packet::SHOT_HIT_SHIP );
					shot_hit.AddUInt( ship->ID );
					shot_hit.AddFloat( ship->Health );
					shot_hit.AddFloat( ship->ShieldF );
					shot_hit.AddFloat( ship->ShieldR );
					shot_hit.AddUInt( shot->ShotType );
					shot_hit.AddDouble( shot->X );
					shot_hit.AddDouble( shot->Y );
					shot_hit.AddDouble( shot->Z );
					Net.SendAll( &shot_hit );
				}
				
				// Dirty hack to make sure the shot is only removed once.
				shot->Lifetime.Reset();
				remove_object_ids.push_back( shot->ID );
				
				// This ship just died.
				if( (ship->Health <= 0.) && (prev_health > 0.) )
				{
					if( ship->ShipType != Ship::TYPE_EXHAUST_PORT )
					{
						// Show an explosion to players.
						Packet explosion( XWing::Packet::EXPLOSION );
						explosion.AddDouble( ship->X );
						explosion.AddDouble( ship->Y );
						explosion.AddDouble( ship->Z );
						explosion.AddFloat( ship->MotionVector.X );
						explosion.AddFloat( ship->MotionVector.Y );
						explosion.AddFloat( ship->MotionVector.Z );
						explosion.AddFloat( 14.f );
						explosion.AddFloat( 1.5f );
						Net.SendAll( &explosion );
					}
					
					// Get relevant data.
					Player *victim = Data.GetPlayer( ship->PlayerID );
					Player *killer = Data.GetPlayer( shot->PlayerID );
					Ship *killer_ship = (Ship*) Data.GetObject( shot->FiredFrom );
					
					if( State < XWing::State::ROUND_ENDED )
					{
						send_scores = true;
						
						int add_score = 0;
						if( killer_ship && ((! ship->Team) || (killer_ship->Team != ship->Team)) )
						{
							if( ship->ShipType == Ship::TYPE_EXHAUST_PORT )
								add_score = 1000000;
							else
								add_score = 1;
						}
						
						// Adjust individual scores.
						if( victim )
							victim->Properties["deaths"] = Num::ToString( atoi( victim->Properties["deaths"].c_str() ) + 1 );
						if( add_score )
						{
							if( killer )
								killer->Properties["kills"] = Num::ToString( atoi( killer->Properties["kills"].c_str() ) + add_score );
							else if( killer_ship )
								ShipScores[ killer_ship->ID ] += add_score;
						}
						
						// Adjust TDM scores.
						if( GameType == XWing::GameType::TEAM_DEATHMATCH )
						{
							uint32_t team = XWing::Team::NONE;
							
							if( killer )
							{
								if( killer->Properties["assigned_team"] == "Rebel" )
									team = XWing::Team::REBEL;
								else if( killer->Properties["assigned_team"] == "Empire" )
									team = XWing::Team::EMPIRE;
							}
							else if( killer_ship )
								team = killer_ship->Team;
							
							if( add_score && (team != XWing::Team::NONE) )
								TeamScores[ team ] ++;
						}
						
						// Notify players.
						const char *victim_name = victim ? victim->Name.c_str() : ship->Name.c_str();
						if( ship->ShipType == Ship::TYPE_EXHAUST_PORT )
							victim_name = "The Death Star";
						
						char cstr[ 1024 ] = "";
						if( killer || killer_ship )
							snprintf( cstr, 1024, "%s was destroyed by %s.", victim_name, killer ? killer->Name.c_str() : (killer_ship ? killer_ship->Name.c_str() : "somebody") );
						else
							snprintf( cstr, 1024, "%s was destroyed.", victim_name );
						
						Packet message( Raptor::Packet::MESSAGE );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
			}
			
			else if( (collision_iter->first->Type() == XWing::Object::SHOT) && (collision_iter->second->Type() == XWing::Object::SHIP) )
			{
				Shot *shot = (Shot*) collision_iter->first;
				Ship *ship = (Ship*) collision_iter->second;
				
				double prev_health = ship->Health;
				Vec3D vec_to_shot( shot->PrevPos.X - ship->X, shot->PrevPos.Y - ship->Y, shot->PrevPos.Z - ship->Z );
				if( ship->Fwd.Dot(&vec_to_shot) >= 0. )
					ship->AddDamage( shot->Damage(), 0. );
				else
					ship->AddDamage( 0., shot->Damage() );
				
				if( prev_health > 0. )
				{
					// Send the hit.
					Packet shot_hit( XWing::Packet::SHOT_HIT_SHIP );
					shot_hit.AddUInt( ship->ID );
					shot_hit.AddFloat( ship->Health );
					shot_hit.AddFloat( ship->ShieldF );
					shot_hit.AddFloat( ship->ShieldR );
					shot_hit.AddUInt( shot->ShotType );
					shot_hit.AddDouble( shot->X );
					shot_hit.AddDouble( shot->Y );
					shot_hit.AddDouble( shot->Z );
					Net.SendAll( &shot_hit );
				}
				
				// Dirty hack to make sure the shot is only removed once.
				shot->Lifetime.Reset();
				remove_object_ids.push_back( shot->ID );
				
				// This ship just died.
				if( (ship->Health <= 0.) && (prev_health > 0.) )
				{
					if( ship->ShipType != Ship::TYPE_EXHAUST_PORT )
					{
						// Show an explosion to players.
						Packet explosion( XWing::Packet::EXPLOSION );
						explosion.AddDouble( ship->X );
						explosion.AddDouble( ship->Y );
						explosion.AddDouble( ship->Z );
						explosion.AddFloat( ship->MotionVector.X );
						explosion.AddFloat( ship->MotionVector.Y );
						explosion.AddFloat( ship->MotionVector.Z );
						explosion.AddFloat( 14.f );
						explosion.AddFloat( 1.5f );
						Net.SendAll( &explosion );
					}
					
					// Get relevant data.
					Player *victim = Data.GetPlayer( ship->PlayerID );
					Player *killer = Data.GetPlayer( shot->PlayerID );
					Ship *killer_ship = (Ship*) Data.GetObject( shot->FiredFrom );
					
					if( State < XWing::State::ROUND_ENDED )
					{
						send_scores = true;
						
						int add_score = 0;
						if( killer_ship && ((! ship->Team) || (killer_ship->Team != ship->Team)) )
						{
							if( ship->ShipType == Ship::TYPE_EXHAUST_PORT )
								add_score = 1000000;
							else
								add_score = 1;
						}
						
						// Adjust individual scores.
						if( victim )
							victim->Properties["deaths"] = Num::ToString( atoi( victim->Properties["deaths"].c_str() ) + 1 );
						if( add_score )
						{
							if( killer )
								killer->Properties["kills"] = Num::ToString( atoi( killer->Properties["kills"].c_str() ) + add_score );
							else if( killer_ship )
								ShipScores[ killer_ship->ID ] += add_score;
						}
						
						// Adjust TDM scores.
						if( GameType == XWing::GameType::TEAM_DEATHMATCH )
						{
							uint32_t team = XWing::Team::NONE;
							
							if( killer )
							{
								if( killer->Properties["assigned_team"] == "Rebel" )
									team = XWing::Team::REBEL;
								else if( killer->Properties["assigned_team"] == "Empire" )
									team = XWing::Team::EMPIRE;
							}
							else if( killer_ship )
								team = killer_ship->Team;
							
							if( add_score && (team != XWing::Team::NONE) )
								TeamScores[ team ] ++;
						}
						
						// Notify players.
						const char *victim_name = victim ? victim->Name.c_str() : ship->Name.c_str();
						if( ship->ShipType == Ship::TYPE_EXHAUST_PORT )
							victim_name = "The Death Star";
						
						char cstr[ 1024 ] = "";
						if( killer || killer_ship )
							snprintf( cstr, 1024, "%s was destroyed by %s.", victim_name, killer ? killer->Name.c_str() : (killer_ship ? killer_ship->Name.c_str() : "somebody") );
						else
							snprintf( cstr, 1024, "%s was destroyed.", victim_name );
						
						Packet message( Raptor::Packet::MESSAGE );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
			}
			
			else if( (collision_iter->first->Type() == XWing::Object::TURRET) && (collision_iter->second->Type() == XWing::Object::SHOT) )
			{
				Shot *shot = (Shot*) collision_iter->second;
				Turret *turret = (Turret*) collision_iter->first;
				
				double prev_health = turret->Health;
				turret->AddDamage( shot->Damage() );
				
				if( prev_health > 0. )
				{
					// Send the hit.
					Packet shot_hit( XWing::Packet::SHOT_HIT_TURRET );
					shot_hit.AddUInt( turret->ID );
					shot_hit.AddFloat( turret->Health );
					shot_hit.AddUInt( shot->ShotType );
					shot_hit.AddDouble( shot->X );
					shot_hit.AddDouble( shot->Y );
					shot_hit.AddDouble( shot->Z );
					Net.SendAll( &shot_hit );
				}
				
				// Dirty hack to make sure the shot is only removed once.
				shot->Lifetime.Reset();
				remove_object_ids.push_back( shot->ID );
				
				// This turret just died.
				if( (turret->Health <= 0.) && (prev_health > 0.) )
				{
					// Show an explosion to players.
					Packet explosion( XWing::Packet::EXPLOSION );
					explosion.AddDouble( turret->X );
					explosion.AddDouble( turret->Y );
					explosion.AddDouble( turret->Z );
					explosion.AddFloat( turret->MotionVector.X );
					explosion.AddFloat( turret->MotionVector.Y );
					explosion.AddFloat( turret->MotionVector.Z );
					explosion.AddFloat( 15.f );
					explosion.AddFloat( 2.f );
					Net.SendAll( &explosion );
				}
			}
			
			else if( (collision_iter->first->Type() == XWing::Object::SHOT) && (collision_iter->second->Type() == XWing::Object::TURRET) )
			{
				Shot *shot = (Shot*) collision_iter->first;
				Turret *turret = (Turret*) collision_iter->second;
				
				double prev_health = turret->Health;
				turret->AddDamage( shot->Damage() );
				
				if( prev_health > 0. )
				{
					// Send the hit.
					Packet shot_hit( XWing::Packet::SHOT_HIT_TURRET );
					shot_hit.AddUInt( turret->ID );
					shot_hit.AddFloat( turret->Health );
					shot_hit.AddUInt( shot->ShotType );
					shot_hit.AddDouble( shot->X );
					shot_hit.AddDouble( shot->Y );
					shot_hit.AddDouble( shot->Z );
					Net.SendAll( &shot_hit );
				}
				
				// Dirty hack to make sure the shot is only removed once.
				shot->Lifetime.Reset();
				remove_object_ids.push_back( shot->ID );
				
				// This turret just died.
				if( (turret->Health <= 0.) && (prev_health > 0.) )
				{
					// Show an explosion to players.
					Packet explosion( XWing::Packet::EXPLOSION );
					explosion.AddDouble( turret->X );
					explosion.AddDouble( turret->Y );
					explosion.AddDouble( turret->Z );
					explosion.AddFloat( turret->MotionVector.X );
					explosion.AddFloat( turret->MotionVector.Y );
					explosion.AddFloat( turret->MotionVector.Z );
					explosion.AddFloat( 15.f );
					explosion.AddFloat( 2.f );
					Net.SendAll( &explosion );
				}
			}
			
			else if( (collision_iter->first->Type() == XWing::Object::SHIP) && (collision_iter->second->Type() == XWing::Object::TURRET) )
			{
				// A ship ran into a turret.
				
				Ship *ship = (Ship*) collision_iter->first;
				Turret *turret = (Turret*) collision_iter->second;
				double prev_health = ship->Health;
				ship->SetHealth( 0. );
				turret->SetHealth( 0. );
				
				if( prev_health > 0. )
				{
					send_scores = true;
					
					Player *player = Data.GetPlayer( ship->PlayerID );
					
					// Show an explosion to players.
					Packet explosion( XWing::Packet::EXPLOSION );
					explosion.AddDouble( turret->X );
					explosion.AddDouble( turret->Y );
					explosion.AddDouble( turret->Z );
					explosion.AddFloat( turret->MotionVector.X );
					explosion.AddFloat( turret->MotionVector.Y );
					explosion.AddFloat( turret->MotionVector.Z );
					explosion.AddFloat( 20.f );
					explosion.AddFloat( 2.f );
					Net.SendAll( &explosion );
					
					if( State < XWing::State::ROUND_ENDED )
					{
						send_scores = true;
						
						// Adjust scores.
						if( player )
							player->Properties["deaths"] = Num::ToString( atoi( player->Properties["deaths"].c_str() ) + 1 );
						
						// Notify players.
						Packet message( Raptor::Packet::MESSAGE );
						char cstr[ 1024 ] = "";
						snprintf( cstr, 1024, "%s ran into a turret.", player ? player->Name.c_str() : ship->Name.c_str() );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
			}
			
			else if( (collision_iter->first->Type() == XWing::Object::TURRET) && (collision_iter->second->Type() == XWing::Object::SHIP) )
			{
				// A ship ran into a turret.
				
				Ship *ship = (Ship*) collision_iter->second;
				Turret *turret = (Turret*) collision_iter->first;
				double prev_health = ship->Health;
				ship->SetHealth( 0. );
				turret->SetHealth( 0. );
				
				if( prev_health > 0. )
				{
					send_scores = true;
					
					Player *player = Data.GetPlayer( ship->PlayerID );
					
					// Show an explosion to players.
					Packet explosion( XWing::Packet::EXPLOSION );
					explosion.AddDouble( turret->X );
					explosion.AddDouble( turret->Y );
					explosion.AddDouble( turret->Z );
					explosion.AddFloat( turret->MotionVector.X );
					explosion.AddFloat( turret->MotionVector.Y );
					explosion.AddFloat( turret->MotionVector.Z );
					explosion.AddFloat( 20.f );
					explosion.AddFloat( 2.f );
					Net.SendAll( &explosion );
					
					if( State < XWing::State::ROUND_ENDED )
					{
						send_scores = true;
						
						// Adjust scores.
						if( player )
							player->Properties["deaths"] = Num::ToString( atoi( player->Properties["deaths"].c_str() ) + 1 );
						
						// Notify players.
						Packet message( Raptor::Packet::MESSAGE );
						char cstr[ 1024 ] = "";
						snprintf( cstr, 1024, "%s ran into a turret.", player ? player->Name.c_str() : ship->Name.c_str() );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
			}
			
			else if( collision_iter->first->Type() == XWing::Object::SHIP )
			{
				Ship *ship = (Ship*) collision_iter->first;
				GameObject *hazard = collision_iter->second;
				double prev_health = ship->Health;
				ship->SetHealth( 0. );
				
				// This ship just died.
				if( prev_health > 0. )
				{
					Player *victim = Data.GetPlayer( ship->PlayerID );
					
					// Don't let the dead ship's motion vector continue into the hazard.
					if( hazard->Type() == XWing::Object::DEATH_STAR )
					{
						DeathStar *deathstar = (DeathStar*) hazard;
						if( ship->MotionVector.Dot( &(deathstar->Up) ) < 0. )
							ship->MotionVector -= deathstar->Up * ship->MotionVector.Dot( &(deathstar->Up) );
					}
					else if( hazard->Type() == XWing::Object::DEATH_STAR_BOX )
					{
						if( ship->MotionVector.Dot( &(hazard->Up) ) < 0. )
							ship->MotionVector -= hazard->Up * ship->MotionVector.Dot( &(hazard->Up) );
					}
					else
					{
						Vec3D vec_to_hazard( hazard->X - ship->X, hazard->Y - ship->Y, hazard->Z - ship->Z );
						vec_to_hazard.ScaleTo( 1. );
						ship->MotionVector -= vec_to_hazard * ship->MotionVector.Dot( vec_to_hazard );
					}
					
					// Show an explosion to players.
					Packet explosion( XWing::Packet::EXPLOSION );
					explosion.AddDouble( ship->X );
					explosion.AddDouble( ship->Y );
					explosion.AddDouble( ship->Z );
					explosion.AddFloat( ship->MotionVector.X );
					explosion.AddFloat( ship->MotionVector.Y );
					explosion.AddFloat( ship->MotionVector.Z );
					explosion.AddFloat( 14.f );
					explosion.AddFloat( 1.5f );
					Net.SendAll( &explosion );
					
					if( State < XWing::State::ROUND_ENDED )
					{
						send_scores = true;
						
						// Adjust scores.
						if( victim )
							victim->Properties["deaths"] = Num::ToString( atoi( victim->Properties["deaths"].c_str() ) + 1 );
						
						// Notify players.
						Packet message( Raptor::Packet::MESSAGE );
						char cstr[ 1024 ] = "";
						const char *hazard_name = "something";
						if( hazard->Type() == XWing::Object::ASTEROID )
							hazard_name = "an asteroid";
						else if( (hazard->Type() == XWing::Object::DEATH_STAR) || (hazard->Type() == XWing::Object::DEATH_STAR_BOX) )
							hazard_name = "the Death Star";
						snprintf( cstr, 1024, "%s ran into %s.", victim ? victim->Name.c_str() : ship->Name.c_str(), hazard_name );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
			}
			
			else if( collision_iter->second->Type() == XWing::Object::SHIP )
			{
				Ship *ship = (Ship*) collision_iter->second;
				GameObject *hazard = collision_iter->first;
				double prev_health = ship->Health;
				ship->SetHealth( 0. );
				
				// This ship just died.
				if( prev_health > 0. )
				{
					Player *victim = Data.GetPlayer( ship->PlayerID );
					
					// Don't let the dead ship's motion vector continue into the hazard.
					if( hazard->Type() == XWing::Object::DEATH_STAR )
					{
						DeathStar *deathstar = (DeathStar*) hazard;
						if( ship->MotionVector.Dot( &(deathstar->Up) ) < 0. )
							ship->MotionVector -= deathstar->Up * ship->MotionVector.Dot( &(deathstar->Up) );
					}
					else if( hazard->Type() == XWing::Object::DEATH_STAR_BOX )
					{
						if( ship->MotionVector.Dot( &(hazard->Up) ) < 0. )
							ship->MotionVector -= hazard->Up * ship->MotionVector.Dot( &(hazard->Up) );
					}
					else
					{
						Vec3D vec_to_hazard( hazard->X - ship->X, hazard->Y - ship->Y, hazard->Z - ship->Z );
						vec_to_hazard.ScaleTo( 1. );
						ship->MotionVector -= vec_to_hazard * ship->MotionVector.Dot( vec_to_hazard );
					}
					
					// Show an explosion to players.
					Packet explosion( XWing::Packet::EXPLOSION );
					explosion.AddDouble( ship->X );
					explosion.AddDouble( ship->Y );
					explosion.AddDouble( ship->Z );
					explosion.AddFloat( ship->MotionVector.X );
					explosion.AddFloat( ship->MotionVector.Y );
					explosion.AddFloat( ship->MotionVector.Z );
					explosion.AddFloat( 14.f );
					explosion.AddFloat( 1.5f );
					Net.SendAll( &explosion );
					
					if( State < XWing::State::ROUND_ENDED )
					{
						send_scores = true;
						
						// Adjust scores.
						if( victim )
							victim->Properties["deaths"] = Num::ToString( atoi( victim->Properties["deaths"].c_str() ) + 1 );
						
						// Notify players.
						Packet message( Raptor::Packet::MESSAGE );
						char cstr[ 1024 ] = "";
						const char *hazard_name = "something";
						if( hazard->Type() == XWing::Object::ASTEROID )
							hazard_name = "an asteroid";
						else if( (hazard->Type() == XWing::Object::DEATH_STAR) || (hazard->Type() == XWing::Object::DEATH_STAR_BOX) )
							hazard_name = "the Death Star";
						snprintf( cstr, 1024, "%s ran into %s.", victim ? victim->Name.c_str() : ship->Name.c_str(), hazard_name );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
			}
			
			else
			{
				if( collision_iter->first->Type() == XWing::Object::SHOT )
				{
					Shot *shot = (Shot*) collision_iter->first;
					remove_object_ids.push_back( shot->ID );
					
					if( (shot->ShotType == Shot::TYPE_TORPEDO) || (shot->ShotType == Shot::TYPE_MISSILE) )
					{
						Packet shot_hit( XWing::Packet::SHOT_HIT_HAZARD );
						shot_hit.AddUInt( shot->ShotType );
						shot_hit.AddDouble( shot->PrevPos.X );
						shot_hit.AddDouble( shot->PrevPos.Y );
						shot_hit.AddDouble( shot->PrevPos.Z );
						Net.SendAll( &shot_hit );
					}
				}
				if( collision_iter->second->Type() == XWing::Object::SHOT )
				{
					Shot *shot = (Shot*) collision_iter->second;
					remove_object_ids.push_back( shot->ID );
					
					if( (shot->ShotType == Shot::TYPE_TORPEDO) || (shot->ShotType == Shot::TYPE_MISSILE) )
					{
						Packet shot_hit( XWing::Packet::SHOT_HIT_HAZARD );
						shot_hit.AddUInt( shot->ShotType );
						shot_hit.AddDouble( shot->PrevPos.X );
						shot_hit.AddDouble( shot->PrevPos.Y );
						shot_hit.AddDouble( shot->PrevPos.Z );
						Net.SendAll( &shot_hit );
					}
				}
			}
		}
		
		// If we changed any scores, send the updated list.
		if( send_scores )
			SendScores();
		
		
		// Figure out who is flying which ship.
		
		std::map<uint16_t,uint32_t> player_ships;
		for( std::map<uint32_t,Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
		{
			Player *player = NULL;
			if( ship_iter->second->PlayerID )
				player = Data.GetPlayer( ship_iter->second->PlayerID );
			if( player )
				player_ships[ player->ID ] = ship_iter->second->ID;
		}
		
		
		// Figure out who is waiting for a ship.
		
		std::map< uint32_t, std::list<Player*> > players_waiting;
		for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
		{
			Player *player = player_iter->second;
			if( player_ships.find( player->ID ) == player_ships.end() )
			{
				if( player->Properties["assigned_team"] == "Rebel" )
					players_waiting[ XWing::Team::REBEL ].push_back( player );
				else if( player->Properties["assigned_team"] == "Empire" )
					players_waiting[ XWing::Team::EMPIRE ].push_back( player );
			}
		}
		
		
		// Determine pilot reassignments and control AI ships.
		
		for( std::map<uint32_t,Ship*>::reverse_iterator ship_iter = ships.rbegin(); ship_iter != ships.rend(); ship_iter ++ )
		{
			Ship *ship = ship_iter->second;
			
			// Find the player whose ship this is.
			Player *player = NULL;
			if( ship->PlayerID )
			{
				player = Data.GetPlayer( ship->PlayerID );
				
				// If a player has left, remove their player ID from the ship.
				if( ! player )
				{
					ship->PlayerID = 0;
					ship->CanRespawn = false;
				}
			}
			
			// See if this ship is available to a player that's waiting for respawn.
			if( PlayersTakeEmptyShips && (! player) && (ship->Health > 0.) && ship->PlayersCanFly() )
			{
				if( players_waiting[ ship->Team ].size() )
				{
					player = players_waiting[ ship->Team ].front();
					ship->PlayerID = player->ID;
					ship->SpecialUpdate = true;
					players_waiting[ ship->Team ].pop_front();
					
					ship->SetYaw( 0. );
					ship->SetPitch( 0. );
					ship->SetRoll( 0. );
					ship->SetThrottle( 0.5, dt );

					// FIXME: Dirty hack to make sure the respawn is noticed!
					int precision = 0;
					Packet update_packet( Raptor::Packet::UPDATE );
					update_packet.AddChar( precision );
					update_packet.AddUInt( 1 );
					update_packet.AddUInt( ship->ID );
					ship->AddToUpdatePacketFromServer( &update_packet, precision );
					Net.SendAll( &update_packet );
				}
			}
			
			
			// If this ship has no player, use AI control.
			
			if( (! player) && (ship->Health > 0.) )
			{
				GameObject *target = NULL;
				
				if( ship->ShipType != Ship::TYPE_EXHAUST_PORT )
				{
					// Build a list of all the things this AI ship might want to target.
					std::vector<GameObject*> potential_targets;
					
					// Add enemy turrets to the list.
					for( std::map<uint32_t,Turret*>::iterator target_iter = turrets.begin(); target_iter != turrets.end(); target_iter ++ )
					{
						if( target_iter->first != ship->ID )
						{
							Turret *potential_target = target_iter->second;
							
							if( potential_target->Health <= 0. )
								continue;
							
							if( ship->Team && (potential_target->Team == ship->Team) )
								continue;
							
							potential_targets.push_back( potential_target );
						}
					}
					
					// Add enemy ships to the list.
					for( std::map<uint32_t,Ship*>::iterator target_iter = ships.begin(); target_iter != ships.end(); target_iter ++ )
					{
						if( target_iter->first != ship->ID )
						{
							Ship *potential_target = target_iter->second;
							
							if( (potential_target->Health <= 0.) && (potential_target->DeathClock.ElapsedSeconds() >= 1.) )
								continue;
							
							if( ship->Team && (potential_target->Team == ship->Team) )
								continue;
							
							potential_targets.push_back( potential_target );
						}
					}
					
					// If we're running the trench, filter out any targets outside it.
					if( deathstar && (ship->DistAlong( &(deathstar->Up), deathstar ) <= 0.) )
					{
						std::vector<GameObject*> trench_targets;
						for( std::vector<GameObject*>::iterator target_iter = potential_targets.begin(); target_iter != potential_targets.end(); target_iter ++ )
						{
							if( fabs( (*target_iter)->DistAlong( &(deathstar->Right), deathstar ) ) < (deathstar->TrenchWidth / 2.) )
								trench_targets.push_back( *target_iter );
						}
						potential_targets = trench_targets;
					}
					
					// Make sure we don't go chasing distant targets when there are nearby ones.
					std::vector<GameObject*> close_targets;
					for( std::vector<GameObject*>::iterator target_iter = potential_targets.begin(); target_iter != potential_targets.end(); target_iter ++ )
					{
						if( (*target_iter)->Dist( ship ) < 2000. )
							close_targets.push_back( *target_iter );
					}
					if( close_targets.size() )
					{
						potential_targets = close_targets;
						
						// If the exhaust port is close, go for that.
						for( std::vector<GameObject*>::iterator target_iter = potential_targets.begin(); target_iter != potential_targets.end(); target_iter ++ )
						{
							if( (*target_iter)->Type() == XWing::Object::SHIP )
							{
								Ship *potential_ship = (Ship*) *target_iter;
								if( potential_ship->ShipType == Ship::TYPE_EXHAUST_PORT )
								{
									potential_targets.clear();
									potential_targets.push_back( potential_ship );
									break;
								}
							}
						}
					}
					
					// Pick a target.
					if( potential_targets.size() > 1 )
					{
						int num = ship->ID;
						if( AIFlock )
							num = ship->Lifetime.ElapsedSeconds() / (5. + ship->ShipType % 3);
						target = potential_targets.at( num % potential_targets.size() );
					}
					else if( potential_targets.size() )
						target = potential_targets.at( 0 );
				}
				
				// This is scoped out here so the waypoint section can see it.
				double t_dot_fwd = 0.;
				
				if( target )
				{
					ship->Target = target->ID;
					
					// Make AI ships select torpedos when attacking the exhaust port.
					if( (ship->ShipType == Ship::TYPE_XWING) || (ship->ShipType == Ship::TYPE_YWING) )
					{
						if( (target->Type() == XWing::Object::SHIP) && (((Ship*)( target ))->ShipType == Ship::TYPE_EXHAUST_PORT) )
						{
							if( ship->SelectedWeapon != Shot::TYPE_TORPEDO )
								ship->NextWeapon();
							if( ship->FiringMode < 2 )
								ship->NextFiringMode();
						}
						else
						{
							if( ship->SelectedWeapon == Shot::TYPE_TORPEDO )
								ship->NextWeapon();
						}
					}
					
					Vec3D vec_to_target( target->X - ship->X, target->Y - ship->Y, target->Z - ship->Z );
					double dist_to_target = vec_to_target.Length();
					Vec3D shot_vec = ship->Fwd;
					shot_vec.ScaleTo( 600. );
					shot_vec -= target->MotionVector;
					double time_to_target = dist_to_target / shot_vec.Length();
					Vec3D vec_to_intercept = vec_to_target;
					vec_to_intercept.X += target->MotionVector.X * time_to_target;
					vec_to_intercept.Y += target->MotionVector.Y * time_to_target;
					vec_to_intercept.Z += target->MotionVector.Z * time_to_target;
					double dist_to_intercept = vec_to_intercept.Length();
					vec_to_target.ScaleTo( 1. );
					vec_to_intercept.ScaleTo( 1. );
					t_dot_fwd = vec_to_target.Dot( &(ship->Fwd) );
					double i_dot_fwd = vec_to_intercept.Dot( &(ship->Fwd) );
					double i_dot_up = vec_to_intercept.Dot( &(ship->Up) );
					double i_dot_right = vec_to_intercept.Dot( &(ship->Right) );
					
					double firing_dist = 1000.;
					if( target->Type() == XWing::Object::TURRET )
						firing_dist = 2000.;
					ship->Firing = (i_dot_fwd > 0.9) && (dist_to_intercept < firing_dist);
					if( ship->ShipType == Ship::TYPE_ISD2 )
						ship->Firing = (dist_to_target < 1500.);
					
					// Don't shoot at dead things.
					if( (target->Type() == XWing::Object::SHIP) && (((Ship*)( target ))->Health <= 0.) )
						ship->Firing = false;
					else if( (target->Type() == XWing::Object::TURRET) && (((Turret*)( target ))->Health <= 0.) )
						ship->Firing = false;
					
					if( t_dot_fwd >= 0. )
					{
						if( dist_to_intercept < 100. )
						{
							// Damn close ahead: Avoid collision!
							ship->SetPitch( (i_dot_up >= 0.) ? -1. : 1. );
							ship->SetYaw( (i_dot_right >= 0.) ? -1. : 1. );
							ship->SetRoll( (i_dot_right >= 0.) ? -1. : 1. );
							ship->SetThrottle( 1., dt );
							//ship->SetShieldPos( Ship::SHIELD_CENTER );
						}
						else if( (dist_to_target < 200.) && (t_dot_fwd < 0.25) )
						{
							// Off to the side, but too close to turn towards: Get some distance.
							ship->SetPitch( 0. );
							ship->SetYaw( i_dot_right / 4. );
							ship->SetRoll( i_dot_right / 2. );
							ship->SetThrottle( 1., dt );
							//ship->SetShieldPos( Ship::SHIELD_CENTER );
						}
						else
						{
							// Generally ahead of us: Aim at enemy.
							ship->SetPitch( (fabs(i_dot_up) > 0.2) ? Num::Sign(i_dot_up) : (i_dot_up*5.) );
							ship->SetYaw( (fabs(i_dot_right) >= 0.5) ? Num::Sign(i_dot_right) : (i_dot_right*2.) );
							ship->SetRoll( (fabs(i_dot_right) > 0.333) ? Num::Sign(i_dot_right) : (i_dot_right*3.) );
							ship->SetThrottle( ((dist_to_target < 300.) && (i_dot_fwd > 0.5)) ? (dist_to_target/300.) : 1., dt );
							//ship->SetShieldPos( Ship::SHIELD_FRONT );
						}
					}
					else if( (dist_to_target < 250.) && (t_dot_fwd < -0.25) && (vec_to_target.Dot( &(ship->Fwd) ) < -0.25) )
					{
						// Behind and aiming at us: Shake 'em!
						ship->SetPitch( 1. );
						ship->SetYaw( Num::EveryOther( ship->Lifetime.ElapsedSeconds(), 3. ) ? 1. : -1. );
						ship->SetRoll( Num::EveryOther( ship->Lifetime.ElapsedSeconds(), 3. ) ? 1. : -1. );
						ship->SetThrottle( 1., dt );
						//ship->SetShieldPos( Ship::SHIELD_REAR );
					}
					else if( dist_to_target < 300. )
					{
						// Behind, not aiming at us, but too close to turn towards: Get some distance.
						ship->SetPitch( 0. );
						ship->SetYaw( i_dot_right / 4. );
						ship->SetRoll( i_dot_right / 2. );
						ship->SetThrottle( 1., dt );
						//ship->SetShieldPos( Ship::SHIELD_REAR );
					}
					else
					{
						// Generally behind us: Turn towards enemy.
						ship->SetPitch( (i_dot_up >= 0.) ? 1. : -1. );
						ship->SetYaw( i_dot_right );
						ship->SetRoll( i_dot_right );
						ship->SetThrottle( 1. - i_dot_up/2., dt );
						//ship->SetShieldPos( Ship::SHIELD_CENTER );
					}
				}
				else
				{
					ship->Target = 0;
					ship->Firing = false;
					ship->SetPitch( 0. );
					ship->SetYaw( 0. );
					ship->SetRoll( 0. );
					ship->SetThrottle( 1., dt );
					t_dot_fwd = 0.;
				}
				
				// See if we should be chasing waypoints.
				Pos3D *waypoint = NULL;
				std::vector<Pos3D> *waypoint_list = NULL;
				if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
				{
					if( Waypoints[ 0 ].size() )
					{
						// See if we have a "trench_runners" server property.
						std::map<std::string,std::string>::iterator trench_runners = Data.Properties.find("trench_runners");
						if( (trench_runners == Data.Properties.end()) || (Str::FindInsensitive( trench_runners->second, "default" ) >= 0) )
						{
							// The default behavior is to select trench runners based on count-down (Y-Wings always run the trench). 
							if( ship->ShipType == Ship::TYPE_YWING )
								waypoint_list = &(Waypoints[ 0 ]);
							else if( (round_time_remaining <= 5. * 60. - 2.) && (ship->ID % 4 >= 1) )
								waypoint_list = &(Waypoints[ 0 ]);
							else if( (round_time_remaining <= 7. * 60. - 2.) && (ship->ID % 4 >= 2) )
								waypoint_list = &(Waypoints[ 0 ]);
							else if( ship->ID % 4 >= 3 )
								waypoint_list = &(Waypoints[ 0 ]);
						}
						else
						{
							// If server property "trench_runners" is set, force certain trench runners.
							if( (Str::FindInsensitive( trench_runners->second, "rebel" ) >= 0) && (ship->Team == XWing::Team::REBEL) )
								waypoint_list = &(Waypoints[ 0 ]);
							if( (Str::FindInsensitive( trench_runners->second, "empire" ) >= 0) && (ship->Team == XWing::Team::EMPIRE) )
								waypoint_list = &(Waypoints[ 0 ]);
							if( (Str::FindInsensitive( trench_runners->second, "x/w" ) >= 0) && (ship->ShipType == Ship::TYPE_XWING) )
								waypoint_list = &(Waypoints[ 0 ]);
							if( (Str::FindInsensitive( trench_runners->second, "y/w" ) >= 0) && (ship->ShipType == Ship::TYPE_YWING) )
								waypoint_list = &(Waypoints[ 0 ]);
							if( (Str::FindInsensitive( trench_runners->second, "t/f" ) >= 0) && (ship->ShipType == Ship::TYPE_TIE_FIGHTER) )
								waypoint_list = &(Waypoints[ 0 ]);
							if( Str::FindInsensitive( trench_runners->second, "all" ) >= 0 )
								waypoint_list = &(Waypoints[ 0 ]);
						}
						
						if( waypoint_list )
						{
							// X-Wings and Y-Wings should follow slightly different paths so they don't collide.
							if( (ship->ShipType == Ship::TYPE_XWING) && (Waypoints[ 1 ].size()) )
								waypoint_list = &(Waypoints[ 1 ]);
							else if( (ship->ShipType == Ship::TYPE_YWING) && (Waypoints[ 2 ].size()) )
								waypoint_list = &(Waypoints[ 2 ]);
						}
					}
				}
				
				// If we're chasing waypoints, pick the appropriate one.
				if( waypoint_list )
				{
					Vec3D vec_to_waypoint, vec_to_next;
					double waypoint_dist = 0.;
					for( size_t i = 0; i < waypoint_list->size(); i ++ )
					{
						Pos3D *wp = &(waypoint_list->at( i ));
						vec_to_waypoint.Set( wp->X - ship->X, wp->Y - ship->Y, wp->Z - ship->Z );
						if( i + 1 < waypoint_list->size() )
						{
							Pos3D *next_wp = &(waypoint_list->at( i + 1 ));
							vec_to_next.Set( next_wp->X - wp->X, next_wp->Y - wp->Y, next_wp->Z - wp->Z );
						}
						
						if( vec_to_waypoint.Dot( &vec_to_next ) > 0. )
						{
							double dist = vec_to_waypoint.Length();
							
							if( (! waypoint) || (dist < waypoint_dist) )
							{
								waypoint = wp;
								waypoint_dist = dist;
							}
						}
					}
				}
				
				// If we have a waypoint, fly towards it.
				if( waypoint )
				{
					Vec3D vec_to_waypoint( waypoint->X - ship->X, waypoint->Y - ship->Y, waypoint->Z - ship->Z );
					vec_to_waypoint.ScaleTo( 1. );
					double w_dot_fwd = vec_to_waypoint.Dot( &(ship->Fwd) );
					double w_dot_up = vec_to_waypoint.Dot( &(ship->Up) );
					double w_dot_right = vec_to_waypoint.Dot( &(ship->Right) );
					
					bool target_is_exhaust_port = ( target && (target->Type() == XWing::Object::SHIP) && ( ((Ship*)( target ))->ShipType == Ship::TYPE_EXHAUST_PORT ) );
					
					// Allow override for shooting at targets ahead of us near the waypoint.
					if( target && (ship->Dist(target) < 1000.) && (t_dot_fwd > 0.95) && (w_dot_fwd > 0.9) && (! target_is_exhaust_port) )
						;
					else
					{
						ship->SetPitch( (fabs(w_dot_up) > 0.2) ? Num::Sign(w_dot_up) : (w_dot_up*5.) );
						ship->SetYaw( (fabs(w_dot_right) >= 0.5) ? Num::Sign(w_dot_right) : (w_dot_right*2.) );
						ship->SetRoll( (fabs(w_dot_right) > 0.333) ? Num::Sign(w_dot_right) : (w_dot_right*3.) );
						ship->SetThrottle( 1., dt );
						
						// Don't fire at the waypoint just because our target is somewhere near the front of us.
						if( ! target_is_exhaust_port )
							ship->Firing = false;
					}
				}
			}
		}
		
		
		// Check for firing ships, expired shots, and ships to remove/respawn.
		
		uint32_t ship_count = 0, rebel_count = 0, empire_count = 0;
		uint32_t last_ship = 0;
		bool exhaust_port = false;
		
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHOT )
			{
				Shot *shot = (Shot*) obj_iter->second;
				if( shot->Lifetime.ElapsedSeconds() > 4. )
					remove_object_ids.push_back( shot->ID );
			}
			else if( obj_iter->second->Type() == XWing::Object::SHIP )
			{
				Ship *ship = (Ship*) obj_iter->second;
				
				if( ship->Health > 0. )
				{
					ship_count ++;
					if( ! last_ship )
						last_ship = ship->ID;
					
					if( ship->Team == XWing::Team::REBEL )
						rebel_count ++;
					else if( ship->Team == XWing::Team::EMPIRE )
						empire_count ++;
					
					if( ship->ShipType == Ship::TYPE_EXHAUST_PORT )
						exhaust_port = true;
					
					if( ship->Firing && (ship->FiringClocks[ ship->SelectedWeapon ].ElapsedSeconds() >= ship->ShotDelay()) )
					{
						uint8_t prev_weapon_index = ship->WeaponIndex;
						GameObject *target = Data.GetObject( ship->Target );
						std::map<int,Shot*> shots = ship->NextShots( target );
						ship->JustFired();
						
						for( std::map<int,Shot*>::iterator shot_iter = shots.begin(); shot_iter != shots.end(); shot_iter ++ )
						{
							uint32_t shot_id = Data.AddObject( shot_iter->second );
							add_object_ids.push_back( shot_id );
						}
						
						if( shots.size() && ((ship->Ammo[ ship->SelectedWeapon ] >= 0) || (ship->PlayerID && (ship->WeaponIndex != prev_weapon_index))) )
						{
							// Send updated ammo count and weapon index to everyone.
							Packet ammo_update( XWing::Packet::AMMO_UPDATE );
							ammo_update.AddUInt( ship->ID );
							ammo_update.AddUInt( ship->SelectedWeapon );
							ammo_update.AddChar( ship->Ammo[ ship->SelectedWeapon ] );
							ammo_update.AddUChar( ship->WeaponIndex );
							if( ship->Ammo[ ship->SelectedWeapon ] >= 0 )
								Net.SendAll( &ammo_update );
							else
								Net.SendToPlayer( &ammo_update, ship->PlayerID );
						}
					}
				}
				else if( ship->DeathClock.ElapsedSeconds() > 5. )
				{
					if( ship->CanRespawn )
					{
						ship->Reset();
						ship->SpecialUpdate = true;
						if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
						{
							bool rebel = (ship->Team == XWing::Team::REBEL);
							double minutes_remaining = round_time_remaining / 60.;
							double closer = (rebel ? 0. : -3000.);
							if( ship->PlayerID )
							{
								closer += 1000.;
								if( minutes_remaining < 10. )
									closer += 250. * (10. - minutes_remaining);
							}
							else if( minutes_remaining < 5. )
								closer += 500. * (5. - minutes_remaining);
							ship->SetPos( Rand::Double(-500.,500.) + closer, Rand::Double(-500.,500.), Rand::Double(0.,1000.) );
							ship->SetFwdVec( 1., 0., 0. );
							ship->SetUpVec( 0., 0., 1. );
						}
						else
						{
							ship->SetPos( Rand::Double(-2000.,2000.), Rand::Double(-2000.,2000.), Rand::Double(-2000.,2000.) );
							ship->SetFwdVec( -(ship->X), -(ship->Y), -(ship->Z) );
						}
						ship->FixVectors();
						
						// FIXME: Dirty hack to make sure the respawn is noticed!
						int precision = 0;
						Packet update_packet( Raptor::Packet::UPDATE );
						update_packet.AddChar( precision );
						update_packet.AddUInt( 1 );
						update_packet.AddUInt( ship->ID );
						ship->AddToUpdatePacketFromServer( &update_packet, precision );
						Net.SendAll( &update_packet );
					}
					else
						remove_object_ids.push_back( ship->ID );
				}
			}
			else if( obj_iter->second->Type() == XWing::Object::TURRET )
			{
				Turret *turret = (Turret*) obj_iter->second;
				
				if( turret->Health > 0. )
				{
					GameObject *target = NULL;
					
					if( turret->ParentID )
					{
						// Attached turrets should always target their ship's target.
						Ship *parent = (Ship*) Data.GetObject( turret->ParentID );
						if( parent )
							turret->Target = parent->Target;
						target = Data.GetObject( turret->Target );
					}
					else
					{
						// If the turret isn't attached, pick a target.
						
						// Set initial values very far away; any ships closer than this are worth considering.
						double nearest_enemy = 100000.;
						double nearest_friendly = nearest_enemy;
						Ship *friendly = NULL;
						
						for( std::map<uint32_t,Ship*>::iterator target_iter = ships.begin(); target_iter != ships.end(); target_iter ++ )
						{
							// Don't target itself or its parent ship.
							if( (target_iter->first == turret->ID) || (target_iter->first == turret->ParentID) )
								continue;
							
							// If the turret has a forced target direction, consider ships within that arc only.
							if( turret->TargetArc < 180. )
							{
								Vec3D vec_to_target( target_iter->second->X - turret->X, target_iter->second->Y - turret->Y, target_iter->second->Z - turret->Z );
								if( turret->TargetDir.AngleBetween( vec_to_target ) > turret->TargetArc )
									continue;
							}
							
							double dist = turret->Dist(target_iter->second);
							
							if( turret->Team && (target_iter->second->Team == turret->Team) )
							{
								// Keep track of the nearest friendly.
								if( dist < nearest_friendly )
								{
									friendly = target_iter->second;
									nearest_friendly = dist;
								}
							}
							else if( dist < nearest_enemy )
							{
								// Target the nearest enemy.
								target = target_iter->second;
								nearest_enemy = dist;
							}
						}
						
						// Deactivate when there's a friendly coming towards us, chasing close behind an enemy.
						if( target && friendly && turret->SafetyDistance )
						{
							Vec3D vec_to_friendly( friendly->X - turret->X, friendly->Y - turret->Y, friendly->Z - turret->Z );
							if( (friendly->MotionVector.Dot(&vec_to_friendly) < 0.) && ( (nearest_friendly < turret->SafetyDistance) || ((nearest_enemy < nearest_friendly) && (nearest_friendly - nearest_enemy < turret->SafetyDistance)) ) )
								target = NULL;
						}
					}
					
					turret->Firing = false;
					
					if( target )
					{
						Pos3D gun = turret->GunPos();
						Vec3D vec_to_target( target->X - gun.X, target->Y - gun.Y, target->Z - gun.Z );
						double dist_to_target = vec_to_target.Length();
						Vec3D shot_vec = gun.Fwd;
						shot_vec.ScaleTo( 400. );
						shot_vec -= target->MotionVector;
						double time_to_target = dist_to_target / shot_vec.Length();
						Vec3D vec_to_intercept = vec_to_target;
						vec_to_intercept.X += target->MotionVector.X * time_to_target * turret->AimAhead;
						vec_to_intercept.Y += target->MotionVector.Y * time_to_target * turret->AimAhead;
						vec_to_intercept.Z += target->MotionVector.Z * time_to_target * turret->AimAhead;
						vec_to_target.ScaleTo( 1. );
						vec_to_intercept.ScaleTo( 1. );
						double i_dot_fwd = vec_to_intercept.Dot( &(gun.Fwd) );
						double i_dot_up = vec_to_intercept.Dot( &(gun.Up) );
						double i_dot_right = vec_to_intercept.Dot( &(gun.Right) );
						double t_dot_up = vec_to_target.Dot( &(gun.Up) );
						
						turret->SetPitch( (fabs(i_dot_up) > 0.25) ? Num::Sign(i_dot_up) : (i_dot_up*4.) );
						
						if( i_dot_fwd > 0. )
						{
							turret->Firing = (dist_to_target < turret->MaxFiringDist) && ((i_dot_up > -0.01) || (t_dot_up > -0.01));
							turret->SetYaw( (fabs(i_dot_right) >= 0.25) ? Num::Sign(i_dot_right) : (i_dot_right*4.) );
						}
						else
							turret->SetYaw( Num::Sign(i_dot_right) );
					}
					else
					{
						// No target, so stop moving and shooting.
						turret->SetYaw( 0. );
						turret->SetPitch( 0. );
						turret->Firing = false;
					}
					
					if( turret->Firing && (turret->FiringClock.ElapsedSeconds() >= turret->ShotDelay()) )
					{
						GameObject *target = Data.GetObject( turret->Target );
						std::map<int,Shot*> shots = turret->NextShots( target );
						turret->JustFired();
						
						for( std::map<int,Shot*>::iterator shot_iter = shots.begin(); shot_iter != shots.end(); shot_iter ++ )
						{
							uint32_t shot_id = Data.AddObject( shot_iter->second );
							add_object_ids.push_back( shot_id );
						}
					}
				}
			}
		}
		
		
		// Tell the clients about any objects we've added, like shots.
		
		std::list<GameObject*> add_objects;
		
		for( std::list<uint32_t>::iterator id_iter = add_object_ids.begin(); id_iter != add_object_ids.end(); id_iter ++ )
		{
			GameObject *add_object = Data.GetObject( *id_iter );
			if( add_object )
				add_objects.push_back( add_object );
		}
		
		if( add_objects.size() )
		{
			Packet objects_add( Raptor::Packet::OBJECTS_ADD );
			objects_add.AddUInt( add_objects.size() );
			
			for( std::list<GameObject*>::iterator obj_iter = add_objects.begin(); obj_iter != add_objects.end(); obj_iter ++ )
			{
				objects_add.AddUInt( (*obj_iter)->ID );
				objects_add.AddUInt( (*obj_iter)->Type() );
				(*obj_iter)->AddToInitPacket( &objects_add );
			}
			
			Net.SendAll( &objects_add );
		}
		
		
		// Tell the clients about any objects we've removed, like expired shots.
		
		if( remove_object_ids.size() )
		{
			Packet objects_remove( Raptor::Packet::OBJECTS_REMOVE );
			objects_remove.AddUInt( remove_object_ids.size() );
			
			for( std::list<uint32_t>::iterator id_iter = remove_object_ids.begin(); id_iter != remove_object_ids.end(); id_iter ++ )
			{
				objects_remove.AddUInt( *id_iter );
				Data.RemoveObject( *id_iter );
			}
			
			Net.SendAll( &objects_remove );
		}
		
		
		// Check for any alerts to send.
		
		if( TimeLimit )
		{
			std::map<double,XWingServerAlert>::reverse_iterator alert_iter = Alerts.rbegin();
			
			if( (alert_iter != Alerts.rend()) && (alert_iter->first >= round_time_remaining) )
			{
				Packet play_sound( Raptor::Packet::PLAY_SOUND );
				play_sound.AddString( alert_iter->second.Sound );
				play_sound.AddChar( Num::UnitFloatTo8( 0.25 ) );
				play_sound.AddChar( Num::UnitFloatTo8( 0.5 ) );
				Net.SendAll( &play_sound );
				
				Packet message( Raptor::Packet::MESSAGE );
				message.AddString( alert_iter->second.Message );
				Net.SendAll( &message );
				
				Alerts.erase( --alert_iter.base() );
			}
		}
		
		
		// Send explosions if the Death Star is dying.
		
		if( (GameType == XWing::GameType::BATTLE_OF_YAVIN) && (! exhaust_port) )
		{
			double ended_secs = RoundEndedTimer.ElapsedSeconds();
			if( (ended_secs >= 3.) && Rand::Bool( FrameTime * ended_secs / 2. ) )
			{
				Packet explosion( XWing::Packet::EXPLOSION );
				explosion.AddDouble( Rand::Double(-2000.,2000.) );
				explosion.AddDouble( Rand::Double(-10000.,30000.) );
				explosion.AddDouble( Rand::Double(-1000.,200) );
				explosion.AddFloat( 0.f );
				explosion.AddFloat( 0.f );
				explosion.AddFloat( 0.f );
				explosion.AddFloat( Rand::Double(1000.,2000.) );
				explosion.AddFloat( 512.f );
				Net.SendAll( &explosion );
			}
		}
		
		
		// Check if the round is over.
		
		if( State < XWing::State::ROUND_WILL_END )
		{
			if( TimeLimit && (RoundTimer.ElapsedSeconds() >= TimeLimit * 60) )
			{
				State = XWing::State::ROUND_WILL_END;
				if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
					RoundEndedDelay = 7.7;
				else
					RoundEndedDelay = 0.;
			}
			else if( GameType == XWing::GameType::TEAM_ELIMINATION )
			{
				if( (rebel_count == 0) || (empire_count == 0) )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 3.;
				}
			}
			else if( GameType == XWing::GameType::FFA_ELIMINATION )
			{
				if( ship_count <= 1 )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 3.;
				}
			}
			else if( GameType == XWing::GameType::TEAM_DEATHMATCH )
			{
				if( KillLimit && ((TeamScores[ XWing::Team::REBEL ] >= KillLimit) || (TeamScores[ XWing::Team::EMPIRE ] >= KillLimit)) )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 0.;
				}
			}
			else if( GameType == XWing::GameType::FFA_DEATHMATCH )
			{
				for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
				{
					if( atoi( player_iter->second->Properties["kills"].c_str() ) >= KillLimit )
					{
						State = XWing::State::ROUND_WILL_END;
						RoundEndedDelay = 0.;
						break;
					}
				}
				
				for( std::map<uint32_t,int>::iterator score_iter = ShipScores.begin(); score_iter != ShipScores.end(); score_iter ++ )
				{
					if( score_iter->second >= KillLimit )
					{
						State = XWing::State::ROUND_WILL_END;
						RoundEndedDelay = 0.;
						break;
					}
				}
			}
			else if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
			{
				if( (! exhaust_port) || ((rebel_count == 0) && (! Respawn)) )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 3.;
				}
			}
			
			if( State == XWing::State::ROUND_WILL_END )
				RoundEndedTimer.Reset();
		}
		
		if( State == XWing::State::ROUND_WILL_END )
		{
			if( RoundEndedTimer.ElapsedSeconds() > RoundEndedDelay )
			{
				State = XWing::State::ROUND_ENDED;
				RoundEndedTimer.Reset();
				
				Alerts.clear();
				
				// Determine the victor.
				uint32_t victor = 0;
				std::string victor_name;
				if( GameType == XWing::GameType::TEAM_ELIMINATION )
				{
					if( rebel_count > empire_count )
					{
						victor = XWing::Team::REBEL;
						victor_name = "The Rebel Alliance";
					}
					else if( empire_count > rebel_count )
					{
						victor = XWing::Team::EMPIRE;
						victor_name = "The Galactic Empire";
					}
				}
				else if( GameType == XWing::GameType::FFA_ELIMINATION )
				{
					if( last_ship )
					{
						Ship *ship = (Ship*) Data.GetObject( last_ship );
						if( ship )
						{
							victor = ship->PlayerID;
							Player *player = Data.GetPlayer( ship->PlayerID );
							if( player )
								victor_name = player->Name;
							else
								victor_name = ship->Name;
						}
					}
				}
				else if( GameType == XWing::GameType::TEAM_DEATHMATCH )
				{
					if( TeamScores[ XWing::Team::REBEL ] > TeamScores[ XWing::Team::EMPIRE ] )
					{
						victor = XWing::Team::REBEL;
						victor_name = "The Rebel Alliance";
					}
					else if( TeamScores[ XWing::Team::EMPIRE ] > TeamScores[ XWing::Team::REBEL ] )
					{
						victor = XWing::Team::EMPIRE;
						victor_name = "The Galactic Empire";
					}
				}
				else if( GameType == XWing::GameType::FFA_DEATHMATCH )
				{
					for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
					{
						if( atoi( player_iter->second->Properties["kills"].c_str() ) >= KillLimit )
						{
							victor = player_iter->first;
							victor_name = player_iter->second->Name;
							break;
						}
					}
					
					if( ! victor )
					{
						for( std::map<uint32_t,int>::iterator score_iter = ShipScores.begin(); score_iter != ShipScores.end(); score_iter ++ )
						{
							if( score_iter->second >= KillLimit )
							{
								Ship *ship = (Ship*) Data.GetObject( score_iter->first );
								if( ship )
								{
									victor_name = ship->Name;
									break;
								}
							}
						}
					}
				}
				else if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
				{
					if( ! exhaust_port )
					{
						victor = XWing::Team::REBEL;
						victor_name = "The Rebel Alliance";
					}
					else
					{
						victor = XWing::Team::EMPIRE;
						victor_name = "The Galactic Empire";
					}
				}
				
				// Send round-ended packet (play round-ended music) to players.
				Packet round_ended( XWing::Packet::ROUND_ENDED );
				round_ended.AddUInt( GameType );
				round_ended.AddUInt( victor );
				Net.SendAll( &round_ended );
				
				// Send message packet to notify players of the victor.
				Packet message( Raptor::Packet::MESSAGE );
				if( victor_name == "" )
				{
					if( victor )
						message.AddString( "The round is over!" );
					else
						message.AddString( "The round ended in a draw!" );
				}
				else
					message.AddString( (victor_name + " has won the round!").c_str() );
				Net.SendAll( &message );
			}
		}
		
		// If the round has been over long enough, return to the lobby.
		
		if( (State == XWing::State::ROUND_ENDED) && (RoundEndedTimer.ElapsedSeconds() > 9.) )
		{
			State = XWing::State::LOBBY;
			
			Packet lobby( XWing::Packet::LOBBY );
			Net.SendAll( &lobby );
			
			Data.ClearObjects();
			
			Packet objects_clear( Raptor::Packet::OBJECTS_CLEAR );
			Net.SendAll( &objects_clear );
		}
	}
	
	else if( State == XWing::State::COUNTDOWN )
	{
		// =========
		// Countdown
		// =========
		
		int remaining = 1 + CountdownFrom - CountdownTimer.ElapsedSeconds();
		if( remaining < CountdownSent )
		{
			CountdownSent = remaining;
			if( remaining >= 0 )
			{
				Packet message( Raptor::Packet::MESSAGE );
				message.AddString( (std::string("Launching in ") + Num::ToString(remaining) + std::string("...")).c_str() );
				Net.SendAll( &message );
			}
			else
				BeginFlying();
		}
	}
}


void XWingServer::ToggleCountdown( void )
{
	if( State < XWing::State::COUNTDOWN )
	{
		CountdownTimer.Reset();
		CountdownSent = CountdownFrom + 1;
		State = XWing::State::COUNTDOWN;
	}
	else
	{
		Packet message( Raptor::Packet::MESSAGE );
		message.AddString( "Launch sequence aborted." );
		Net.SendAll( &message );
		
		State = XWing::State::LOBBY;
	}
}


void XWingServer::BeginFlying( void )
{
	// Don't allow someone to hit Fly while the round is ending.
	if( State > XWing::State::FLYING )
		return;
	
	if( State != XWing::State::FLYING )
	{
		// Starting a new round.
		
		GameType = XWing::GameType::TEAM_ELIMINATION;
		Respawn = false;
		PlayersTakeEmptyShips = false;
		bool ffa = false;
		KillLimit = 0;
		TimeLimit = 0;
		Alerts.clear();
		Waypoints.clear();
		Squadrons.clear();
		
		if( Data.Properties["gametype"] == "team_elim" )
		{
			if( Data.Properties["respawn"] == "true" )
				PlayersTakeEmptyShips = true;
		}
		else if( Data.Properties["gametype"] == "ffa_elim" )
		{
			GameType = XWing::GameType::FFA_ELIMINATION;
			ffa = true;
		}
		else if( Data.Properties["gametype"] == "team_dm" )
		{
			GameType = XWing::GameType::TEAM_DEATHMATCH;
			Respawn = true;
			KillLimit = atoi( Data.Properties["tdm_kill_limit"].c_str() );
		}
		else if( Data.Properties["gametype"] == "ffa_dm" )
		{
			GameType = XWing::GameType::FFA_DEATHMATCH;
			ffa = true;
			Respawn = true;
			KillLimit = atoi( Data.Properties["dm_kill_limit"].c_str() );
		}
		else if( Data.Properties["gametype"] == "yavin" )
		{
			GameType = XWing::GameType::BATTLE_OF_YAVIN;
			TimeLimit = atoi( Data.Properties["yavin_time_limit"].c_str() );
			Respawn = (Data.Properties["respawn"] == "true");
			
			if( TimeLimit >= 30 )
				Alerts[ 30. * 60. ] = XWingServerAlert( "deathstar_30min.wav", "The moon with the Rebel base will be in range in 30 minutes." );
			if( TimeLimit >= 15 )
				Alerts[ 15. * 60. ] = XWingServerAlert( "deathstar_15min.wav", "Stand-by alert: Death Star approaching.  Estimated time to firing range: 15 minutes." );
			if( TimeLimit >= 7 )
				Alerts[ 7. * 60. ] = XWingServerAlert( "deathstar_7min.wav", "The Rebel base will be in firing range in 7 minutes." );
			if( TimeLimit >= 5 )
				Alerts[ 5. * 60. ] = XWingServerAlert( "deathstar_5min.wav", "Death Star will be in range in 5 minutes." );
			if( TimeLimit >= 3 )
				Alerts[ 3. * 60. ] = XWingServerAlert( "deathstar_3min.wav", "Rebel base: 3 minutes and closing." );
			
			Alerts[ 60. ] = XWingServerAlert( "deathstar_1min.wav", "Rebel base: 1 minute and closing." );
			Alerts[ 30. ] = XWingServerAlert( "deathstar_30sec.wav", "Rebel base: 30 seconds and closing." );
			Alerts[ 0. ] = XWingServerAlert( "deathstar_0sec.wav", "The Death Star has cleared the planet!  The Death Star has cleared the planet!  Rebel base in range." );
		}
		
		if( Data.Properties["ai_flock"] == "true" )
			AIFlock = true;
		else
			AIFlock = false;
		
		// Clear all objects and tell the clients to do likewise.
		Data.ClearObjects();
		Packet obj_clear = Packet( Raptor::Packet::OBJECTS_CLEAR );
		Net.SendAll( &obj_clear );
		
		if( GameType != XWing::GameType::BATTLE_OF_YAVIN )
		{
			// Add asteroids to the level.
			int asteroids = atoi( Data.Properties["asteroids"].c_str() );
			for( int i = 0; i < asteroids; i ++ )
			{
				Asteroid *asteroid = new Asteroid();
				asteroid->X = Rand::Double( -1000., 1000. );
				asteroid->Y = Rand::Double( -1000., 1000. );
				asteroid->Z = Rand::Double( -1000., 1000. );
				Data.AddObject( asteroid );
			}
		}
		else
		{
			// Add Death Star stuff.
			
			// Surface.
			DeathStar *deathstar = new DeathStar();
			deathstar->X = 0.;
			deathstar->Y = 0.;
			deathstar->Z = -500.;
			deathstar->SetFwdVec( 1., 0., 0. );
			deathstar->SetUpVec( 0., 0., 1. );
			Data.AddObject( deathstar );
			
			// Exhaust port floor.
			DeathStarBox *exhaust_box = new DeathStarBox();
			exhaust_box->Copy( deathstar );
			exhaust_box->W = deathstar->TrenchWidth;
			exhaust_box->L = exhaust_box->W;
			exhaust_box->H = exhaust_box->W;
			exhaust_box->MoveAlong( &(deathstar->Fwd), 15000. );
			exhaust_box->MoveAlong( &(deathstar->Up), exhaust_box->H / 4. - deathstar->TrenchDepth );
			exhaust_box->Pitch( 15. );
			Data.AddObject( exhaust_box );
			
			// Exhaust port.
			Ship *exhaust_port = new Ship();
			exhaust_port->Copy( exhaust_box );
			exhaust_port->SetType( Ship::TYPE_EXHAUST_PORT );
			exhaust_port->CanRespawn = false;
			exhaust_port->Team = XWing::Team::EMPIRE;
			exhaust_port->Name = "Exhaust Port";
			exhaust_port->MoveAlong( &(exhaust_port->Up), exhaust_box->H / 2. );
			Data.AddObject( exhaust_port );
			
			// Back wall.
			DeathStarBox *back_wall = new DeathStarBox();
			back_wall->Copy( deathstar );
			back_wall->W = deathstar->TrenchWidth;
			back_wall->L = back_wall->W * 2.;
			back_wall->H = deathstar->TrenchDepth * 0.9;
			back_wall->MoveAlong( &(deathstar->Fwd), 15200. );
			back_wall->MoveAlong( &(deathstar->Up), back_wall->H / 2. - deathstar->TrenchDepth );
			Data.AddObject( back_wall );
			
			// Exhaust port trench turret.
			Turret *back_turret = new Turret();
			back_turret->Copy( deathstar );
			back_turret->Team = XWing::Team::EMPIRE;
			back_turret->Health = 149.9;
			back_turret->MinGunPitch = 5.;
			back_turret->MaxGunPitch = 90.;
			back_turret->SingleShotDelay = 0.25;
			back_turret->FiringMode = 2;
			back_turret->MoveAlong( &(deathstar->Fwd), 14900. );
			back_turret->MoveAlong( &(deathstar->Up), -deathstar->TrenchDepth );
			Data.AddObject( back_turret );
			
			// Wall in front of last turret.
			DeathStarBox *front_wall = new DeathStarBox();
			front_wall->Copy( deathstar );
			front_wall->W = deathstar->TrenchWidth;
			front_wall->H = 10.;
			front_wall->L = front_wall->H / 2.;
			front_wall->MoveAlong( &(deathstar->Fwd), 14800. );
			front_wall->MoveAlong( &(deathstar->Up), front_wall->H / 2. - deathstar->TrenchDepth );
			Data.AddObject( front_wall );
			
			// Extra surface turrets near exhaust port.
			for( int turret_num = 0; turret_num <= 10; turret_num ++ )
			{
				DeathStarBox *box = new DeathStarBox();
				box->Copy( deathstar );
				box->W = 11.;
				box->L = 11.;
				box->H = Rand::Double( 10., 30. );
				box->MoveAlong( &(deathstar->Up), box->H / 2. );
				box->MoveAlong( &(deathstar->Fwd), Rand::Double( 14000., 16000. ) );
				box->MoveAlong( &(deathstar->Right), deathstar->TrenchWidth * (Rand::Bool() ? 1. : -1.) * Rand::Double( 2., 10. ) );
				Data.AddObject( box );
				
				Turret *turret = new Turret();
				turret->Team = XWing::Team::EMPIRE;
				turret->Copy( box );
				turret->MoveAlong( &(box->Up), box->H / 2. );
				turret->Health = 149.9;
				turret->MinGunPitch = 0.;
				Data.AddObject( turret );
			}
			
			// Surface turrets.
			int yavin_turrets = atoi( Data.Properties["yavin_turrets"].c_str() );
			for( int turret_num = 0; turret_num <= yavin_turrets; turret_num ++ )
			{
				DeathStarBox *box = new DeathStarBox();
				box->Copy( deathstar );
				box->W = 11.;
				box->L = 11.;
				box->H = Rand::Double( 10., 80. );
				box->MoveAlong( &(deathstar->Up), box->H / 2. );
				box->MoveAlong( &(deathstar->Fwd), Rand::Double( -10000., 20000. ) );
				box->MoveAlong( &(deathstar->Right), deathstar->TrenchWidth * (Rand::Bool() ? 1. : -1.) * Rand::Double( 2., 50. ) );
				Data.AddObject( box );
				
				Turret *turret = new Turret();
				turret->Team = XWing::Team::EMPIRE;
				turret->Copy( box );
				turret->MoveAlong( &(box->Up), box->H / 2. );
				turret->Health = 100.;
				turret->MinGunPitch = -5.;
				Data.AddObject( turret );
			}
			
			// Trench bottom turrets.
			for( double fwd = 2550.; fwd < 14300.1; fwd += 900. )
			{
				Turret *turret = new Turret();
				turret->Copy( deathstar );
				turret->Team = XWing::Team::EMPIRE;
				turret->Health = 75.;
				turret->SingleShotDelay = Rand::Double( 1., 2. );
				turret->FiringMode = 1;
				turret->AimAhead = 2.25f;
				turret->MaxFiringDist = 6000.;
				turret->SafetyDistance = 1500.;
				turret->Fwd.Set( -1., 0., 0. );
				turret->TargetDir.Set( -1., 0., 0. );
				turret->TargetArc = 30.;
				turret->MaxGunPitch = 30.;
				turret->PitchGun( 10. );
				turret->MoveAlong( &(deathstar->Fwd), fwd );
				turret->MoveAlong( &(deathstar->Up), -deathstar->TrenchDepth );
				turret->MoveAlong( &(deathstar->Right), deathstar->TrenchWidth * Rand::Double( -0.2, 0.2 ) );
				Data.AddObject( turret );
			}
			
			// Trench side turrets.
			for( double fwd = 2800.; fwd < 14500.1; fwd += 900. )
			{
				Turret *turret = new Turret();
				turret->Copy( deathstar );
				turret->Team = XWing::Team::EMPIRE;
				turret->Health = 75.;
				turret->SingleShotDelay = Rand::Double( 1.5, 2.5 );
				turret->FiringMode = 1;
				turret->AimAhead = 2.f;
				turret->MaxFiringDist = 6000.;
				turret->SafetyDistance = 1500.;
				turret->Fwd.Set( -1., 0., 0. );
				turret->TargetDir.Set( -1., 0., 0. );
				turret->TargetArc = 30.;
				turret->MaxGunPitch = 20.;
				turret->PitchGun( 5. );
				turret->Up.Set( 0., Rand::Bool() ? 1. : -1., 0. );
				turret->MoveAlong( &(deathstar->Fwd), fwd );
				turret->MoveAlong( &(deathstar->Up), deathstar->TrenchDepth * Rand::Double( -0.3, -0.7 ) );
				turret->MoveAlong( &(turret->Up), deathstar->TrenchWidth / -2. );
				Data.AddObject( turret );
			}
			
			// Obstacles in trench.
			int prev_box_type = Rand::Int( 0, 3 );
			Pos3D waypoint;
			for( double fwd = -1000.; fwd < 14000.1; fwd += 300. )
			{
				DeathStarBox *box = new DeathStarBox();
				box->Copy( deathstar );
				box->MoveAlong( &(deathstar->Fwd), fwd );
				
				int box_type = Rand::Int( 0, 2 );
				if( box_type >= prev_box_type )
					box_type ++;
				prev_box_type = box_type;
				
				if( box_type == 0 )
				{
					// Across the bottom.
					box->H = (deathstar->TrenchDepth - 20.) * Rand::Double( 0.2, 0.45 );
					box->W = deathstar->TrenchWidth;
					box->L = box->H / 2.;
					box->MoveAlong( &(deathstar->Up), box->H / 2. - deathstar->TrenchDepth );
					
					if( fwd >= 2000. )
					{
						waypoint.Copy( box );
						waypoint.MoveAlong( &(deathstar->Up), deathstar->TrenchDepth / 2. );
						if( ! Waypoints[ 0 ].size() )
							waypoint.MoveAlong( &(deathstar->Up), deathstar->TrenchDepth / 2. );
						Waypoints[ 0 ].push_back( waypoint );
					}
				}
				else if( box_type == 1 )
				{
					// Across the top.
					box->H = (deathstar->TrenchDepth - 20.) * Rand::Double( 0.2, 0.45 );
					box->W = deathstar->TrenchWidth;
					box->L = box->H / 2.;
					box->MoveAlong( &(deathstar->Up), box->H / -2. );
					
					if( Waypoints[ 0 ].size() )
					{
						waypoint.Copy( box );
						waypoint.MoveAlong( &(deathstar->Up), deathstar->TrenchDepth / -2. );
						Waypoints[ 0 ].push_back( waypoint );
					}
				}
				else if( box_type == 2 )
				{
					// Across the left.
					box->W = (deathstar->TrenchWidth - 20.) * Rand::Double( 0.4, 0.7 );
					box->H = deathstar->TrenchDepth;
					box->L = box->W / 2.;
					box->MoveAlong( &(deathstar->Right), (box->W - deathstar->TrenchWidth) / 2. );
					box->MoveAlong( &(deathstar->Up), box->H / 2. - deathstar->TrenchDepth );
					
					if( Waypoints[ 0 ].size() )
					{
						waypoint.Copy( box );
						waypoint.MoveAlong( &(deathstar->Right), deathstar->TrenchWidth / 2. );
						Waypoints[ 0 ].push_back( waypoint );
					}
				}
				else if( box_type == 3 )
				{
					// Across the right.
					box->W = (deathstar->TrenchWidth - 20.) * Rand::Double( 0.4, 0.7 );
					box->H = deathstar->TrenchDepth;
					box->L = box->W / 2.;
					box->MoveAlong( &(deathstar->Right), (deathstar->TrenchWidth - box->W) / 2. );
					box->MoveAlong( &(deathstar->Up), box->H / 2. - deathstar->TrenchDepth );
					
					if( Waypoints[ 0 ].size() )
					{
						waypoint.Copy( box );
						waypoint.MoveAlong( &(deathstar->Right), deathstar->TrenchWidth / -2. );
						Waypoints[ 0 ].push_back( waypoint );
					}
				}
				Data.AddObject( box );	
			}
			
			// Attack.
			waypoint.Copy( exhaust_port );
			waypoint.MoveAlong( &(deathstar->Fwd), -200. );
			waypoint.MoveAlong( &(deathstar->Up), 3. );
			Waypoints[ 0 ].push_back( waypoint );
			
			// Escape.
			waypoint.MoveAlong( &(deathstar->Fwd), 1500. );
			waypoint.MoveAlong( &(deathstar->Up), 1500. );
			Waypoints[ 0 ].push_back( waypoint );
			
			// Loop back.
			waypoint.MoveAlong( &(deathstar->Fwd), -500. );
			waypoint.MoveAlong( &(deathstar->Up), 1000. );
			waypoint.MoveAlong( &(deathstar->Right), 500. );
			Waypoints[ 0 ].push_back( waypoint );
			waypoint.MoveAlong( &(deathstar->Fwd), -1500. );
			waypoint.MoveAlong( &(deathstar->Right), 1000. );
			Waypoints[ 0 ].push_back( waypoint );
			waypoint.MoveAlong( &(deathstar->Fwd), -500. );
			waypoint.MoveAlong( &(deathstar->Up), -500. );
			Waypoints[ 0 ].push_back( waypoint );
			waypoint.MoveAlong( &(deathstar->Fwd), -900. );
			Waypoints[ 0 ].push_back( waypoint );
			waypoint.MoveAlong( &(deathstar->Fwd), -500. );
			waypoint.MoveAlong( &(deathstar->Up), -500. );
			waypoint.MoveAlong( &(deathstar->Right), -500. );
			Waypoints[ 0 ].push_back( waypoint );
			waypoint.MoveAlong( &(deathstar->Fwd), -500. );
			waypoint.MoveAlong( &(deathstar->Right), -500. );
			Waypoints[ 0 ].push_back( waypoint );
			waypoint.MoveAlong( &(deathstar->Up), -500. );
			waypoint.MoveAlong( &(deathstar->Right), -500. );
			Waypoints[ 0 ].push_back( waypoint );
			waypoint.MoveAlong( &(deathstar->Fwd), 500. );
			waypoint.MoveAlong( &(deathstar->Up), -500. );
			Waypoints[ 0 ].push_back( waypoint );
			waypoint.MoveAlong( &(deathstar->Fwd), 150. );
			waypoint.MoveAlong( &(deathstar->Up), -100. );
			Waypoints[ 0 ].push_back( waypoint );
			
			for( size_t i = 0; i < Waypoints[ 0 ].size(); i ++ )
			{
				Waypoints[ 1 ].push_back( Waypoints[ 0 ][ i ] );
				Waypoints[ 2 ].push_back( Waypoints[ 0 ][ i ] );
				Waypoints[ 1 ][ i ].MoveAlong( &(deathstar->Up), 5.5 );
				Waypoints[ 2 ][ i ].MoveAlong( &(deathstar->Up), -5.5 );
			}
		}
		
		// Count the players who picked teams.
		uint32_t rebel_count = 0;
		uint32_t empire_count = 0;
		uint32_t spectator_count = 0;
		for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
		{
			if( player_iter->second->Properties["team"] == "Rebel" )
				rebel_count ++;
			else if( player_iter->second->Properties["team"] == "Empire" )
				empire_count ++;
			else if( player_iter->second->Properties["team"] == "Spectator" )
				spectator_count ++;
		}
		
		// Clear player scores and build a new list of ships.
		for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
		{
			player_iter->second->Properties["kills"] = "0";
			player_iter->second->Properties["deaths"] = "0";
			
			if( player_iter->second->Properties["team"] == "Spectator" )
			{
				player_iter->second->Properties["assigned_team"] = "Spectator";
				continue;
			}
			
			bool rebel = Rand::Bool();
			if( (GameType == XWing::GameType::BATTLE_OF_YAVIN) && ((Data.Players.size() - spectator_count) == 1) )
				rebel = true;
			
			if( player_iter->second->Properties["team"] == "Rebel" )
				rebel = true;
			else if( player_iter->second->Properties["team"] == "Empire" )
				rebel = false;
			else if( rebel_count < empire_count )
			{
				rebel = true;
				rebel_count ++;
			}
			else if( rebel_count > empire_count )
			{
				rebel = false;
				empire_count ++;
			}
			else if( rebel )
				rebel_count ++;
			else
				empire_count ++;
			
			player_iter->second->Properties["assigned_team"] = rebel ? "Rebel" : "Empire";
			
			uint32_t ship_type = rebel ? Ship::TYPE_XWING : Ship::TYPE_TIE_FIGHTER;
			if( player_iter->second->Properties["ship"] == "X/W" )
				ship_type = Ship::TYPE_XWING;
			else if( player_iter->second->Properties["ship"] == "Y/W" )
				ship_type = Ship::TYPE_YWING;
			else if( player_iter->second->Properties["ship"] == "T/F" )
				ship_type = Ship::TYPE_TIE_FIGHTER;
			else if( player_iter->second->Properties["ship"] == "ISD2" )
				ship_type = Ship::TYPE_ISD2;
			
			if( ! ffa )
			{
				if( rebel && (ship_type == Ship::TYPE_TIE_FIGHTER) )
					ship_type = Ship::TYPE_XWING;
				else if( (! rebel) && ((ship_type == Ship::TYPE_XWING) || (ship_type == Ship::TYPE_YWING)) )
					ship_type = Ship::TYPE_TIE_FIGHTER;
			}
			
			std::string squadron = rebel ? "Rogue" : "Omega";
			
			Ship *ship = new Ship();
			ship->PlayerID = player_iter->second->ID;
			ship->Team = (ffa ? XWing::Team::NONE : (rebel ? XWing::Team::REBEL : XWing::Team::EMPIRE));
			ship->CanRespawn = Respawn;
			ship->SetType( ship_type );
			ship->Name = squadron + std::string(" ") + Num::ToString( (int) Squadrons[ squadron ].size() + 1 );
			ship->X = Rand::Double( -100., 100. ) + (rebel ? 1500. : -1500.);
			ship->Y = Rand::Double( -100., 100. ) * (ffa ? 10. : 1.);
			ship->Z = Rand::Double( -100., 100. ) * (ffa ? 10. : 1.);
			ship->SetFwdVec( (rebel && (GameType != XWing::GameType::BATTLE_OF_YAVIN) ? -1. : 1.), 0., 0. );
			ship->SetUpVec( 0., 0., 1. );
			Data.AddObject( ship );
			ShipScores[ ship->ID ] = 0;
			Squadrons[ squadron ].insert( ship->ID );
		}
		
		// Add AI ships based on the number of waves selected.
		int ai_in_first_wave = 8;
		int ai_in_other_waves = 6;
		if( (GameType == XWing::GameType::TEAM_DEATHMATCH) || (GameType == XWing::GameType::FFA_DEATHMATCH) )
		{
			ai_in_first_wave = 6;
			ai_in_other_waves = 4;
		}
		else if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
		{
			ai_in_first_wave = 4;
			ai_in_other_waves = 4;
		}
		
		int ai_waves = atoi( Data.Properties["ai_waves"].c_str() );
		double ai_empire_ratio = atof( Data.Properties["ai_empire_ratio"].c_str() );
		
		for( int wave = 0; wave < ai_waves; wave ++ )
		{
			uint32_t rebel_ship_type = Ship::TYPE_XWING;
			std::string rebel_squadron = "Red";
			if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
			{
				if( Squadrons[ "Red" ].size() > Squadrons[ "Gold" ].size() * 2 )
				{
					rebel_ship_type = Ship::TYPE_YWING;
					rebel_squadron = "Gold";
				}
			}
			
			int ai_ships = wave ? ai_in_other_waves : (ai_in_first_wave - (rebel_count + empire_count)%2);
			for( int i = 0; i < ai_ships; i ++ )
			{
				bool rebel = Rand::Bool();
				if( rebel_count * ai_empire_ratio < empire_count )
				{
					rebel = true;
					rebel_count ++;
				}
				else if( rebel_count * ai_empire_ratio > empire_count )
				{
					rebel = false;
					empire_count ++;
				}
				else if( rebel )
					rebel_count ++;
				else
					empire_count ++;
				
				uint32_t ship_type = rebel ? rebel_ship_type : Ship::TYPE_TIE_FIGHTER;
				std::string squadron = rebel ? rebel_squadron : "Alpha";
				
				Ship *ship = new Ship();
				ship->PlayerID = 0;
				ship->Team = (ffa ? XWing::Team::NONE : (rebel ? XWing::Team::REBEL : XWing::Team::EMPIRE));
				ship->CanRespawn = Respawn;
				ship->SetType( ship_type );
				ship->Name = squadron + std::string(" ") + Num::ToString( (int) Squadrons[ squadron ].size() + 1 );
				if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
				{
					ship->X = Rand::Double( -150., 150. ) + (rebel ? 2000. : -2000.);
					ship->Y = Rand::Double( -200., 200. ) * (wave + 1.);
					ship->Z = Rand::Double( 100., 1000. );
					ship->SetFwdVec( 1., 0., 0. );
				}
				else
				{
					ship->X = Rand::Double( -150., 150. ) + (rebel ? 1200. : -1200.) + wave * (rebel ? 1800. : -1800.);
					ship->Y = Rand::Double( -200., 200. ) * (wave + 1.) * (ffa ? 10. : 1.);
					ship->Z = Rand::Double( -200., 200. ) * (wave + 1.) * (ffa ? 10. : 1.);
					ship->SetFwdVec( (rebel ? -1. : 1.), 0., 0. );
				}
				ship->SetUpVec( 0., 0., 1. );
				Data.AddObject( ship );
				ShipScores[ ship->ID ] = 0;
				Squadrons[ squadron ].insert( ship->ID );
			}
		}
		
		
		// Test cases.
		
		std::map<std::string,std::string>::iterator spawn = Data.Properties.find("spawn");
		if( spawn != Data.Properties.end() )
		{
			if( Str::FindInsensitive( spawn->second, "isd2" ) >= 0 )
			{
				Ship *ship = new Ship();
				ship->PlayerID = 0;
				ship->Team = XWing::Team::EMPIRE;
				ship->CanRespawn = false;
				ship->SetType( Ship::TYPE_ISD2 );
				ship->Name = "Imperator";
				ship->X = 0.;
				ship->Y = -2000.;
				ship->Z = 0.;
				ship->SetFwdVec( 0., 1., 0. );
				ship->SetUpVec( 0., 0., 1. );
				ship->SetThrottle( 0.1, 120. );
				Data.AddObject( ship );
				ShipScores[ ship->ID ] = 0;
			}
		}
		
		
		// Start the round timer and clear the scores.
		
		RoundTimer.Reset();
		TeamScores[ XWing::Team::REBEL ] = 0;
		TeamScores[ XWing::Team::EMPIRE ] = 0;
		ShipScores.clear();
	}
	else
	{
		// Late joiners want to Fly.
		
		
		// Count players currently on each team.
		
		uint32_t rebel_count = 0, empire_count = 0;
		
		for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
		{
			if( player_iter->second->Properties["assigned_team"] == "Rebel" )
				rebel_count ++;
			else if( player_iter->second->Properties["assigned_team"] == "Empire" )
				empire_count ++;
		}
		
		
		// Assign new players to teams.
		
		for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
		{
			if( player_iter->second->Properties["assigned_team"].empty() )
			{
				player_iter->second->Properties["kills"] = "0";
				player_iter->second->Properties["deaths"] = "0";
				
				if( player_iter->second->Properties["team"] == "Spectator" )
				{
					player_iter->second->Properties["assigned_team"] = "Spectator";
					continue;
				}
				
				bool rebel = Rand::Bool();
				if( player_iter->second->Properties["team"] == "Rebel" )
					rebel = true;
				else if( player_iter->second->Properties["team"] == "Empire" )
					rebel = false;
				else if( rebel_count < empire_count )
					rebel = true;
				else if( rebel_count > empire_count )
					rebel = false;
				
				if( rebel )
					rebel_count ++;
				else
					empire_count ++;
				
				player_iter->second->Properties["assigned_team"] = rebel ? "Rebel" : "Empire";
				
				if( Respawn )
				{
					bool ffa = (GameType == XWing::GameType::FFA_DEATHMATCH);
					
					uint32_t ship_type = rebel ? Ship::TYPE_XWING : Ship::TYPE_TIE_FIGHTER;
					if( player_iter->second->Properties["ship"] == "X/W" )
						ship_type = Ship::TYPE_XWING;
					else if( player_iter->second->Properties["ship"] == "Y/W" )
						ship_type = Ship::TYPE_YWING;
					else if( player_iter->second->Properties["ship"] == "T/F" )
						ship_type = Ship::TYPE_TIE_FIGHTER;
					else if( player_iter->second->Properties["ship"] == "ISD2" )
						ship_type = Ship::TYPE_ISD2;
					
					std::string squadron = rebel ? "Rogue" : "Omega";
					
					Ship *ship = new Ship();
					ship->PlayerID = player_iter->second->ID;
					ship->Team = (ffa ? XWing::Team::NONE : (rebel ? XWing::Team::REBEL : XWing::Team::EMPIRE));
					ship->CanRespawn = true;
					ship->SetType( ship_type );
					ship->Name = squadron + std::string(" ") + Num::ToString( (int) Squadrons[ squadron ].size() + 1 );
					if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
					{
						ship->SetPos( Rand::Double(-500.,500.) + (rebel ? 1000. : -3000.), Rand::Double(-500.,500.), Rand::Double(0.,1000.) );
						ship->SetFwdVec( 1., 0., 0. );
						ship->SetUpVec( 0., 0., 1. );
					}
					else
					{
						ship->X = Rand::Double(-500.,500.) + (rebel ? 1500. : -1500.);
						ship->Y = Rand::Double(-500.,500.) * (ffa ? 4. : 1.);
						ship->Z = Rand::Double(-500.,500.) * (ffa ? 4. : 1.);
						ship->SetFwdVec( (rebel && (GameType != XWing::GameType::BATTLE_OF_YAVIN) ? -1. : 1.), 0., 0. );
					}
					ship->SetUpVec( 0., 0., 1. );
					Data.AddObject( ship );
					ShipScores[ ship->ID ] = 0;
					Squadrons[ squadron ].insert( ship->ID );
				}
			}
		}
	}
	
	// Send list of objects to the clients.
	Packet obj_list = Packet( Raptor::Packet::OBJECTS_ADD );
	obj_list.AddUInt( Data.GameObjects.size() );
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
	{
		obj_list.AddUInt( obj_iter->second->ID );
		obj_list.AddUInt( obj_iter->second->Type() );
		obj_iter->second->AddToInitPacket( &obj_list );
	}
	Net.SendAll( &obj_list );
	
	// Tell clients to clear/update team scores.
	SendScores();
	
	// Switch to flight mode and tell the clients.
	State = XWing::State::FLYING;
	Packet fly = Packet( XWing::Packet::FLY );
	Net.SendAll( &fly );
}


void XWingServer::SendScores( void )
{
	for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
	{
		Packet player_properties( Raptor::Packet::PLAYER_PROPERTIES );
		player_properties.AddUShort( player_iter->second->ID );
		player_properties.AddUInt( 3 );
		player_properties.AddString( "kills" );
		player_properties.AddString( player_iter->second->Properties["kills"] );
		player_properties.AddString( "deaths" );
		player_properties.AddString( player_iter->second->Properties["deaths"] );
		player_properties.AddString( "assigned_team" );
		player_properties.AddString( player_iter->second->Properties["assigned_team"] );
		Net.SendAll( &player_properties );
	}
	
	// FIXME
	Data.Properties[ "team_score_rebel" ] = Num::ToString( TeamScores[ XWing::Team::REBEL ] );
	Data.Properties[ "team_score_empire" ] = Num::ToString( TeamScores[ XWing::Team::EMPIRE ] );
	Packet info( Raptor::Packet::INFO );
	info.AddUShort( 2 );
	info.AddString( "team_score_rebel" );
	info.AddString( Data.Properties[ "team_score_rebel" ] );
	info.AddString( "team_score_empire" );
	info.AddString( Data.Properties[ "team_score_empire" ] );
	Net.SendAll( &info );
	
	Packet time_remaining( XWing::Packet::TIME_REMAINING );
	time_remaining.AddFloat( TimeLimit ? RoundTimeRemaining() : 0 );
	Net.SendAll( &time_remaining );
}


double XWingServer::RoundTimeRemaining( void )
{
	if( TimeLimit )
		return TimeLimit * 60. - RoundTimer.ElapsedSeconds();
	
	return 60. * 60.;
}


// -----------------------------------------------------------------------------


XWingServerAlert::XWingServerAlert( void )
{
}


XWingServerAlert::XWingServerAlert( std::string sound, std::string message )
{
	Sound = sound;
	Message = message;
}


XWingServerAlert::~XWingServerAlert()
{
}
