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
	RespawnDelay = 10.;
	RoundEndedDelay = 3.;
	KillLimit = 10;
	TimeLimit = 0;
	AIFlock = false;
	DefendingTeam = XWing::Team::NONE;
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
	Data.Properties["hunt_time_limit"] = "5";
	Data.Properties["defending_team"] = "empire";
	Data.Properties["empire_ship"] = "isd2";
	Data.Properties["rebel_ship"] = "frg";
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
		std::set<uint32_t> add_object_ids;
		std::set<uint32_t> remove_object_ids;
		
		// Gather info important to the update.
		double round_time_remaining = RoundTimeRemaining();
		
		
		// Build a list of all ships.
		
		std::map<uint32_t,Ship*> ships;
		std::map<uint32_t,Turret*> turrets;
		std::map<uint32_t,Asteroid*> asteroids;
		DeathStar *deathstar = NULL;
		
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHIP )
				ships[ obj_iter->second->ID ] = (Ship*) obj_iter->second;
			else if( obj_iter->second->Type() == XWing::Object::TURRET )
				turrets[ obj_iter->second->ID ] = (Turret*) obj_iter->second;
			else if( obj_iter->second->Type() == XWing::Object::ASTEROID )
				asteroids[ obj_iter->second->ID ] = (Asteroid*) obj_iter->second;
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
				Player *player1 = Data.GetPlayer( ship1->PlayerID );
				Player *player2 = Data.GetPlayer( ship2->PlayerID );
				bool died1 = false, died2 = false;
				
				if( prev_health1 > 0. )
				{
					ship1->AddDamage( ship2->CollisionPotential / 2., ship2->CollisionPotential / 2., collision_iter->FirstObject.length() ? collision_iter->FirstObject.c_str() : NULL );
					std::map<std::string,double>::const_iterator subsystem_iter = ship1->Subsystems.find( collision_iter->FirstObject );
					
					if( (subsystem_iter != ship1->Subsystems.end()) && (subsystem_iter->second <= 0.) )
					{
						// Show an explosion to players for the destroyed subsystem.
						std::map<std::string,ModelObject>::iterator obj_iter = ship1->Shape.Objects.find( subsystem_iter->first );
						double radius = ship1->Radius();
						Vec3D offset;
						if( obj_iter != ship1->Shape.Objects.end() )
						{
							radius = obj_iter->second.GetMaxRadius();
							Pos3D center = obj_iter->second.GetCenterPoint();
							offset = (ship1->Fwd * center.X) + (ship1->Up * center.Y) + (ship1->Right * center.Z);
						}
						Packet explosion( XWing::Packet::EXPLOSION );
						explosion.AddDouble( ship1->X + offset.X );
						explosion.AddDouble( ship1->Y + offset.Y );
						explosion.AddDouble( ship1->Z + offset.Z );
						explosion.AddFloat( ship1->MotionVector.X );
						explosion.AddFloat( ship1->MotionVector.Y );
						explosion.AddFloat( ship1->MotionVector.Z );
						explosion.AddFloat( radius * 4. );
						explosion.AddFloat( log(radius) );
						Net.SendAll( &explosion );
					}
					
					if( ship1->Health <= 0. )
					{
						died1 = true;
						send_scores = true;
						
						// Show an explosion to players.
						Packet explosion( XWing::Packet::EXPLOSION );
						explosion.AddDouble( ship1->X );
						explosion.AddDouble( ship1->Y );
						explosion.AddDouble( ship1->Z );
						explosion.AddFloat( ship1->MotionVector.X );
						explosion.AddFloat( ship1->MotionVector.Y );
						explosion.AddFloat( ship1->MotionVector.Z );
						explosion.AddFloat( ship1->Radius() * 3. );
						explosion.AddFloat( log( ship1->Radius() ) );
						Net.SendAll( &explosion );
						
						// Adjust score.
						if( player1 && (State < XWing::State::ROUND_ENDED) )
							player1->Properties["deaths"] = Num::ToString( atoi( player1->Properties["deaths"].c_str() ) + 1 );
					}
					else
					{
						// Send the hit.
						Packet hit( XWing::Packet::MISC_HIT_SHIP );
						hit.AddUInt( ship1->ID );
						hit.AddFloat( ship1->Health );
						hit.AddFloat( ship1->ShieldF );
						hit.AddFloat( ship1->ShieldR );
						if( subsystem_iter != ship1->Subsystems.end() )
						{
							hit.AddString( subsystem_iter->first.c_str() );
							hit.AddFloat( subsystem_iter->second );
						}
						else
							hit.AddString( "" );
						hit.AddDouble( ship2->X );
						hit.AddDouble( ship2->Y );
						hit.AddDouble( ship2->Z );
						Net.SendAll( &hit );
					}
				}
				
				if( prev_health2 > 0. )
				{
					ship2->AddDamage( ship1->CollisionPotential / 2., ship1->CollisionPotential / 2., collision_iter->SecondObject.length() ? collision_iter->SecondObject.c_str() : NULL );
					std::map<std::string,double>::const_iterator subsystem_iter = ship2->Subsystems.find( collision_iter->SecondObject );
					
					if( (subsystem_iter != ship2->Subsystems.end()) && (subsystem_iter->second <= 0.) )
					{
						// Show an explosion to players for the destroyed subsystem.
						std::map<std::string,ModelObject>::iterator obj_iter = ship2->Shape.Objects.find( subsystem_iter->first );
						double radius = ship2->Radius();
						Vec3D offset;
						if( obj_iter != ship2->Shape.Objects.end() )
						{
							radius = obj_iter->second.GetMaxRadius();
							Pos3D center = obj_iter->second.GetCenterPoint();
							offset = (ship2->Fwd * center.X) + (ship2->Up * center.Y) + (ship2->Right * center.Z);
						}
						Packet explosion( XWing::Packet::EXPLOSION );
						explosion.AddDouble( ship2->X + offset.X );
						explosion.AddDouble( ship2->Y + offset.Y );
						explosion.AddDouble( ship2->Z + offset.Z );
						explosion.AddFloat( ship2->MotionVector.X );
						explosion.AddFloat( ship2->MotionVector.Y );
						explosion.AddFloat( ship2->MotionVector.Z );
						explosion.AddFloat( radius * 4. );
						explosion.AddFloat( log(radius) );
						Net.SendAll( &explosion );
					}
					
					if( ship2->Health <= 0. )
					{
						died2 = true;
						send_scores = true;
						
						// Show an explosion to players.
						Packet explosion( XWing::Packet::EXPLOSION );
						explosion.AddDouble( ship2->X );
						explosion.AddDouble( ship2->Y );
						explosion.AddDouble( ship2->Z );
						explosion.AddFloat( ship2->MotionVector.X );
						explosion.AddFloat( ship2->MotionVector.Y );
						explosion.AddFloat( ship2->MotionVector.Z );
						explosion.AddFloat( ship2->Radius() * 3. );
						explosion.AddFloat( log( ship2->Radius() ) );
						Net.SendAll( &explosion );
						
						// Adjust score.
						if( player2 && (State < XWing::State::ROUND_ENDED) )
							player2->Properties["deaths"] = Num::ToString( atoi( player2->Properties["deaths"].c_str() ) + 1 );
					}
					else
					{
						// Send the hit.
						Packet hit( XWing::Packet::MISC_HIT_SHIP );
						hit.AddUInt( ship2->ID );
						hit.AddFloat( ship2->Health );
						hit.AddFloat( ship2->ShieldF );
						hit.AddFloat( ship2->ShieldR );
						if( subsystem_iter != ship2->Subsystems.end() )
						{
							hit.AddString( subsystem_iter->first.c_str() );
							hit.AddFloat( subsystem_iter->second );
						}
						else
							hit.AddString( "" );
						hit.AddDouble( ship1->X );
						hit.AddDouble( ship1->Y );
						hit.AddDouble( ship1->Z );
						Net.SendAll( &hit );
					}
				}
				
				if( died1 && (ship2->Health > 0.) )
					ship1->CollisionPotential = 0.;
				if( died2 && (ship1->Health > 0.) )
					ship2->CollisionPotential = 0.;
				
				if( (died1 || died2) && (State < XWing::State::ROUND_ENDED) )
				{
					// Notify players.
					Packet message( Raptor::Packet::MESSAGE );
					char cstr[ 1024 ] = "";
					if( died1 && died2 )
						snprintf( cstr, 1024, "%s and %s collided.", player1 ? player1->Name.c_str() : ship1->Name.c_str(), player2 ? player2->Name.c_str() : ship2->Name.c_str() );
					else if( died1 )
					{
						if( prev_health2 > 0. )
							snprintf( cstr, 1024, "%s ran into %s.", player1 ? player1->Name.c_str() : ship1->Name.c_str(), player2 ? player2->Name.c_str() : ship2->Name.c_str() );
						else
							snprintf( cstr, 1024, "%s ran into pieces of %s.", player1 ? player1->Name.c_str() : ship1->Name.c_str(), player2 ? player2->Name.c_str() : ship2->Name.c_str() );
					}
					else
					{
						if( prev_health1 > 0. )
							snprintf( cstr, 1024, "%s ran into %s.", player2 ? player2->Name.c_str() : ship2->Name.c_str(), player1 ? player1->Name.c_str() : ship1->Name.c_str() );
						else
							snprintf( cstr, 1024, "%s ran into pieces of %s.", player2 ? player2->Name.c_str() : ship2->Name.c_str(), player1 ? player1->Name.c_str() : ship1->Name.c_str() );
					}
					message.AddString( cstr );
					Net.SendAll( &message );
				}
				
				if( Data.Properties.find("debug") != Data.Properties.end() )
				{
					if( collision_iter->FirstObject.length() )
					{
						Packet message( Raptor::Packet::MESSAGE );
						char cstr[ 1024 ] = "";
						snprintf( cstr, 1024, "%s object %s.", player1 ? player1->Name.c_str() : ship1->Name.c_str(), collision_iter->FirstObject.c_str() );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
					
					if( collision_iter->SecondObject.length() )
					{
						Packet message( Raptor::Packet::MESSAGE );
						char cstr[ 1024 ] = "";
						snprintf( cstr, 1024, "%s object %s.", player2 ? player2->Name.c_str() : ship2->Name.c_str(), collision_iter->SecondObject.c_str() );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
			}
			
			else if( (collision_iter->first->Type() == XWing::Object::SHIP) && (collision_iter->second->Type() == XWing::Object::SHOT) )
			{
				Shot *shot = (Shot*) collision_iter->second;
				Ship *ship = (Ship*) collision_iter->first;
				
				/*
				// Make sure each shot only hits one thing.
				if( remove_object_ids.find(shot->ID) != remove_object_ids.end() )
					continue;
				*/
				
				double prev_health = ship->Health;
				
				if( collision_iter->FirstObject.length() )
				{
					ship->AddDamage( shot->Damage() / 2., shot->Damage() / 2., collision_iter->FirstObject.c_str() );
					
					if( Data.Properties.find("debug") != Data.Properties.end() )
					{
						Packet message( Raptor::Packet::MESSAGE );
						char cstr[ 1024 ] = "";
						snprintf( cstr, 1024, "Shot hit %s.", collision_iter->FirstObject.c_str() );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
				else
				{
					Vec3D vec_to_shot( shot->PrevPos.X - ship->X, shot->PrevPos.Y - ship->Y, shot->PrevPos.Z - ship->Z );
					if( ship->Fwd.Dot(&vec_to_shot) >= 0. )
						ship->AddDamage( shot->Damage(), 0. );
					else
						ship->AddDamage( 0., shot->Damage() );
				}
				
				if( prev_health > 0. )
				{
					std::map<std::string,double>::const_iterator subsystem_iter = ship->Subsystems.find( collision_iter->FirstObject );
					
					// Send the hit.
					Packet shot_hit( XWing::Packet::SHOT_HIT_SHIP );
					shot_hit.AddUInt( ship->ID );
					shot_hit.AddFloat( ship->Health );
					shot_hit.AddFloat( ship->ShieldF );
					shot_hit.AddFloat( ship->ShieldR );
					if( subsystem_iter != ship->Subsystems.end() )
					{
						shot_hit.AddString( subsystem_iter->first.c_str() );
						shot_hit.AddFloat( subsystem_iter->second );
					}
					else
						shot_hit.AddString( "" );
					shot_hit.AddUInt( shot->ShotType );
					shot_hit.AddDouble( shot->X );
					shot_hit.AddDouble( shot->Y );
					shot_hit.AddDouble( shot->Z );
					Net.SendAll( &shot_hit );
					
					if( (subsystem_iter != ship->Subsystems.end()) && (subsystem_iter->second <= 0.) )
					{
						// Show an explosion to players for the destroyed subsystem.
						std::map<std::string,ModelObject>::iterator obj_iter = ship->Shape.Objects.find( subsystem_iter->first );
						double radius = ship->Radius();
						Vec3D offset;
						if( obj_iter != ship->Shape.Objects.end() )
						{
							radius = obj_iter->second.GetMaxRadius();
							Pos3D center = obj_iter->second.GetCenterPoint();
							offset = ship->Fwd * center.X + ship->Up * center.Y + ship->Right * center.Z;
						}
						Packet explosion( XWing::Packet::EXPLOSION );
						explosion.AddDouble( ship->X + offset.X );
						explosion.AddDouble( ship->Y + offset.Y );
						explosion.AddDouble( ship->Z + offset.Z );
						explosion.AddFloat( ship->MotionVector.X );
						explosion.AddFloat( ship->MotionVector.Y );
						explosion.AddFloat( ship->MotionVector.Z );
						explosion.AddFloat( radius * 3. );
						explosion.AddFloat( log(radius) );
						Net.SendAll( &explosion );
					}
				}
				
				// Dirty hack to make sure the shot is only removed once.
				shot->Lifetime.Reset();
				remove_object_ids.insert( shot->ID );
				
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
						explosion.AddFloat( ship->Radius() * 3. );
						explosion.AddFloat( log( ship->Radius() ) );
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
							else if( ship->ShipType == Ship::TYPE_ISD2 )
								add_score = 37000;
							else if( ship->ShipType == Ship::TYPE_CORVETTE )
								add_score = 100;
							else if( ship->ShipType == Ship::TYPE_CALAMARI_CRUISER )
								add_score = 5400;
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
				
				/*
				// Make sure each shot only hits one thing.
				if( remove_object_ids.find(shot->ID) != remove_object_ids.end() )
					continue;
				*/
				
				double prev_health = ship->Health;
				
				if( collision_iter->SecondObject.length() )
				{
					ship->AddDamage( shot->Damage() / 2., shot->Damage() / 2., collision_iter->SecondObject.c_str() );
					
					if( Data.Properties.find("debug") != Data.Properties.end() )
					{
						Packet message( Raptor::Packet::MESSAGE );
						char cstr[ 1024 ] = "";
						snprintf( cstr, 1024, "Shot hit %s.", collision_iter->SecondObject.c_str() );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
				else
				{
					Vec3D vec_to_shot( shot->PrevPos.X - ship->X, shot->PrevPos.Y - ship->Y, shot->PrevPos.Z - ship->Z );
					if( ship->Fwd.Dot(&vec_to_shot) >= 0. )
						ship->AddDamage( shot->Damage(), 0. );
					else
						ship->AddDamage( 0., shot->Damage() );
				}
				
				if( prev_health > 0. )
				{
					std::map<std::string,double>::const_iterator subsystem_iter = ship->Subsystems.find( collision_iter->SecondObject );
					
					// Send the hit.
					Packet shot_hit( XWing::Packet::SHOT_HIT_SHIP );
					shot_hit.AddUInt( ship->ID );
					shot_hit.AddFloat( ship->Health );
					shot_hit.AddFloat( ship->ShieldF );
					shot_hit.AddFloat( ship->ShieldR );
					if( subsystem_iter != ship->Subsystems.end() )
					{
						shot_hit.AddString( subsystem_iter->first.c_str() );
						shot_hit.AddFloat( subsystem_iter->second );
					}
					else
						shot_hit.AddString( "" );
					shot_hit.AddUInt( shot->ShotType );
					shot_hit.AddDouble( shot->X );
					shot_hit.AddDouble( shot->Y );
					shot_hit.AddDouble( shot->Z );
					Net.SendAll( &shot_hit );
					
					if( (subsystem_iter != ship->Subsystems.end()) && (subsystem_iter->second <= 0.) )
					{
						// Show an explosion to players for the destroyed subsystem.
						std::map<std::string,ModelObject>::iterator obj_iter = ship->Shape.Objects.find( subsystem_iter->first );
						double radius = ship->Radius();
						Vec3D offset;
						if( obj_iter != ship->Shape.Objects.end() )
						{
							radius = obj_iter->second.GetMaxRadius();
							Pos3D center = obj_iter->second.GetCenterPoint();
							offset = ship->Fwd * center.X + ship->Up * center.Y + ship->Right * center.Z;
						}
						Packet explosion( XWing::Packet::EXPLOSION );
						explosion.AddDouble( ship->X + offset.X );
						explosion.AddDouble( ship->Y + offset.Y );
						explosion.AddDouble( ship->Z + offset.Z );
						explosion.AddFloat( ship->MotionVector.X );
						explosion.AddFloat( ship->MotionVector.Y );
						explosion.AddFloat( ship->MotionVector.Z );
						explosion.AddFloat( radius * 3. );
						explosion.AddFloat( log(radius) );
						Net.SendAll( &explosion );
					}
				}
				
				// Dirty hack to make sure the shot is only removed once.
				shot->Lifetime.Reset();
				remove_object_ids.insert( shot->ID );
				
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
						explosion.AddFloat( ship->Radius() * 3. );
						explosion.AddFloat( log( ship->Radius() ) );
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
				
				/*
				// Make sure each shot only hits one thing.
				if( remove_object_ids.find(shot->ID) != remove_object_ids.end() )
					continue;
				*/
				
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
				remove_object_ids.insert( shot->ID );
				
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
				
				/*
				// Make sure each shot only hits one thing.
				if( remove_object_ids.find(shot->ID) != remove_object_ids.end() )
					continue;
				*/
				
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
				remove_object_ids.insert( shot->ID );
				
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
						if( fabs( ship->DistAlong( &(deathstar->Right), deathstar ) ) <= (deathstar->TrenchWidth / 2.) )
							ship->MotionVector -= deathstar->Right * ship->MotionVector.Dot( &(deathstar->Right) );
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
					explosion.AddFloat( ship->Radius() * 3. );
					explosion.AddFloat( log( ship->Radius() ) );
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
						if( fabs( ship->DistAlong( &(deathstar->Right), deathstar ) ) <= (deathstar->TrenchWidth / 2.) )
							ship->MotionVector -= deathstar->Right * ship->MotionVector.Dot( &(deathstar->Right) );
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
					explosion.AddFloat( ship->Radius() * 3. );
					explosion.AddFloat( log( ship->Radius() ) );
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
					
					/*
					// Make sure each shot only hits one thing.
					if( remove_object_ids.find(shot->ID) != remove_object_ids.end() )
						continue;
					*/
					
					// Dirty hack to make sure the shot is only removed once.
					shot->Lifetime.Reset();
					remove_object_ids.insert( shot->ID );
					
					if( (shot->ShotType == Shot::TYPE_TORPEDO) || (shot->ShotType == Shot::TYPE_MISSILE) )
					{
						Packet shot_hit( XWing::Packet::SHOT_HIT_HAZARD );
						shot_hit.AddUInt( shot->ShotType );
						shot_hit.AddDouble( shot->PrevPos.X );
						shot_hit.AddDouble( shot->PrevPos.Y );
						shot_hit.AddDouble( shot->PrevPos.Z );
						Net.SendAll( &shot_hit );
					}
					
					if( (collision_iter->second->Type() == XWing::Object::ASTEROID) && shot->AsteroidDamage() )
					{
						Asteroid *asteroid = (Asteroid*) collision_iter->second;
						remove_object_ids.insert( asteroid->ID );
						
						// Show an explosion to players.
						Packet explosion( XWing::Packet::EXPLOSION );
						explosion.AddDouble( asteroid->X );
						explosion.AddDouble( asteroid->Y );
						explosion.AddDouble( asteroid->Z );
						explosion.AddFloat( asteroid->MotionVector.X );
						explosion.AddFloat( asteroid->MotionVector.Y );
						explosion.AddFloat( asteroid->MotionVector.Z );
						explosion.AddFloat( asteroid->Radius * 1.5 );
						explosion.AddFloat( log( asteroid->Radius / 10. ) );
						Net.SendAll( &explosion );
					}
				}
				if( collision_iter->second->Type() == XWing::Object::SHOT )
				{
					Shot *shot = (Shot*) collision_iter->second;
					
					/*
					// Make sure each shot only hits one thing.
					if( remove_object_ids.find(shot->ID) != remove_object_ids.end() )
						continue;
					*/
					
					// Dirty hack to make sure the shot is only removed once.
					shot->Lifetime.Reset();
					remove_object_ids.insert( shot->ID );
					
					if( (shot->ShotType == Shot::TYPE_TORPEDO) || (shot->ShotType == Shot::TYPE_MISSILE) )
					{
						Packet shot_hit( XWing::Packet::SHOT_HIT_HAZARD );
						shot_hit.AddUInt( shot->ShotType );
						shot_hit.AddDouble( shot->PrevPos.X );
						shot_hit.AddDouble( shot->PrevPos.Y );
						shot_hit.AddDouble( shot->PrevPos.Z );
						Net.SendAll( &shot_hit );
					}
					
					if( (collision_iter->first->Type() == XWing::Object::ASTEROID) && shot->AsteroidDamage() )
					{
						Asteroid *asteroid = (Asteroid*) collision_iter->first;
						remove_object_ids.insert( asteroid->ID );
						
						// Show an explosion to players.
						Packet explosion( XWing::Packet::EXPLOSION );
						explosion.AddDouble( asteroid->X );
						explosion.AddDouble( asteroid->Y );
						explosion.AddDouble( asteroid->Z );
						explosion.AddFloat( asteroid->MotionVector.X );
						explosion.AddFloat( asteroid->MotionVector.Y );
						explosion.AddFloat( asteroid->MotionVector.Z );
						explosion.AddFloat( asteroid->Radius * 1.5 );
						explosion.AddFloat( log( asteroid->Radius / 10. ) );
						Net.SendAll( &explosion );
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
			
			// For ships with a complex collision model, apply explosion to server-side model when dead.
			if( ship->Health <= 0. )
				ship->Explode( dt );
			
			
			// If this ship has no player, use AI control.
			
			if( (! player) && (ship->Health > 0.) )
			{
				GameObject *target = NULL;
				
				if( ship->Radius() > 500. )
				{
					// Big ships shoot asteroids out of their way.
					
					double dist_fwd = 0., dist_up = 0., dist_right = 0.;
					double l = ship->Shape.GetLength(), h = ship->Shape.GetHeight(), w = ship->Shape.GetWidth();
					
					for( std::map<uint32_t,Asteroid*>::iterator asteroid_iter = asteroids.begin(); asteroid_iter != asteroids.end(); asteroid_iter ++ )
					{
						dist_fwd = asteroid_iter->second->DistAlong( &(ship->Fwd), ship );
						dist_up = asteroid_iter->second->DistAlong( &(ship->Up), ship );
						dist_right = asteroid_iter->second->DistAlong( &(ship->Right), ship );
						
						if( (dist_fwd < l * 0.6) && (dist_fwd > l * -0.5) && (fabs(dist_up) < h * 0.55) && (fabs(dist_right) < w * 0.55) )
						{
							target = asteroid_iter->second;
							break;
						}
					}
				}
				
				if( (! target) && (ship->ShipType != Ship::TYPE_EXHAUST_PORT) )
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
							
							// Don't attack turrets from below (to prevent accidentally ramming capital ships).
							if( ship->DistAlong( &(potential_target->Up), potential_target ) < -50. )
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
					
					// Make AI ships select torpedos when attacking appropriate targets.
					if( (ship->ShipType == Ship::TYPE_XWING) || (ship->ShipType == Ship::TYPE_YWING) )
					{
						if( (target->Type() == XWing::Object::SHIP) && (((Ship*)( target ))->ShipType == Ship::TYPE_EXHAUST_PORT) )
						{
							if( ship->SelectedWeapon != Shot::TYPE_TORPEDO )
								ship->NextWeapon();
							if( ship->FiringMode < 2 )
								ship->NextFiringMode();
						}
						else if( ship->Ammo[ Shot::TYPE_TORPEDO ] && (target->Type() == XWing::Object::SHIP) && ! ((Ship*)( target ))->PlayersCanFly() )
						{
							if( ship->SelectedWeapon != Shot::TYPE_TORPEDO )
								ship->NextWeapon();
							if( ship->FiringMode > 1 )
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
					
					double dodge_dist = 100. + ship->Radius();
					double firing_dist = 1000.;
					if( target->Type() == XWing::Object::SHIP )
						dodge_dist += ((Ship*)( target ))->Radius();
					else if( target->Type() == XWing::Object::TURRET )
						firing_dist = 2000.;
					ship->Firing = (i_dot_fwd > 0.9) && (dist_to_intercept < firing_dist);
					if( (ship->ShipType != Ship::TYPE_EXHAUST_PORT) && ! ship->PlayersCanFly() )
						ship->Firing = (dist_to_target < ship->Radius() + 1000.);
					
					// Don't shoot at dead things.
					if( (target->Type() == XWing::Object::SHIP) && (((Ship*)( target ))->Health <= 0.) )
						ship->Firing = false;
					else if( (target->Type() == XWing::Object::TURRET) && (((Turret*)( target ))->Health <= 0.) )
						ship->Firing = false;
					
					if( target->Type() == XWing::Object::ASTEROID )
					{
						ship->SetPitch( 0. );
						ship->SetYaw( 0. );
						ship->SetRoll( 0. );
						ship->SetThrottle( 1., dt );
					}
					else if( t_dot_fwd >= 0. )
					{
						if( dist_to_intercept < dodge_dist )
						{
							// Damn close ahead: Avoid collision!
							ship->SetPitch( (i_dot_up >= 0.) ? -1. : 1. );
							ship->SetYaw( (i_dot_right >= 0.) ? -1. : 1. );
							ship->SetRoll( (i_dot_right >= 0.) ? -1. : 1. );
							ship->SetThrottle( 1., dt );
							//ship->SetShieldPos( Ship::SHIELD_CENTER );
						}
						else if( (dist_to_target < dodge_dist + 500.) && (t_dot_fwd < 0.25) )
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
					/*
					else if( (dist_to_target < dodge_dist + 150.) && (t_dot_fwd < -0.25) && (vec_to_target.Dot( &(ship->Fwd) ) < -0.25) )
					{
						// Behind and aiming at us: Shake 'em!
						ship->SetPitch( cos( ship->Lifetime.ElapsedSeconds() * 3. ) * 0.4 + 0.6 );
						ship->SetYaw( sin( ship->Lifetime.ElapsedSeconds() ) );
						ship->SetRoll( sin( ship->Lifetime.ElapsedSeconds() ) );
						ship->SetThrottle( 1., dt );
						//ship->SetShieldPos( Ship::SHIELD_REAR );
					}
					*/
					else if( dist_to_target < dodge_dist + 200. )
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
							if( ship->ShipType == Ship::TYPE_ISD2 || ship->ShipType == Ship::TYPE_CORVETTE )
								;
							else if( ship->ShipType == Ship::TYPE_YWING )
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
							if( (Str::FindInsensitive( trench_runners->second, "isd2" ) >= 0) && (ship->ShipType == Ship::TYPE_ISD2) )
								waypoint_list = &(Waypoints[ 0 ]);
							if( (Str::FindInsensitive( trench_runners->second, "crv" ) >= 0) && (ship->ShipType == Ship::TYPE_CORVETTE) )
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
				else if( GameType == XWing::GameType::DEFEND_DESTROY )
				{
					if( Waypoints[ 0 ].size() )
					{
						// In Defend/Destroy, the capital ships circle eachother.
						if( ship->IsMissionObjective )
							waypoint_list = &(Waypoints[ 0 ]);
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
		
		uint32_t ship_count = 0, rebel_count = 0, empire_count = 0, rebel_objectives = 0, empire_objectives = 0;
		uint32_t last_ship = 0;
		std::map< uint32_t, std::map<uint8_t,Pos3D> > group_spawns;
		
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHOT )
			{
				Shot *shot = (Shot*) obj_iter->second;
				if( shot->Lifetime.ElapsedSeconds() > shot->MaxLifetime() )
					remove_object_ids.insert( shot->ID );
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
					{
						rebel_count ++;
						if( ship->IsMissionObjective )
							rebel_objectives ++;
					}
					else if( ship->Team == XWing::Team::EMPIRE )
					{
						empire_count ++;
						if( ship->IsMissionObjective )
							empire_objectives ++;
					}
					
					if( ship->Firing && (ship->FiringClocks[ ship->SelectedWeapon ].ElapsedSeconds() >= ship->ShotDelay()) )
					{
						GameObject *target = Data.GetObject( ship->Target );
						std::map<int,Shot*> shots = ship->NextShots( target );
						ship->JustFired();
						
						for( std::map<int,Shot*>::iterator shot_iter = shots.begin(); shot_iter != shots.end(); shot_iter ++ )
						{
							uint32_t shot_id = Data.AddObject( shot_iter->second );
							add_object_ids.insert( shot_id );
						}
					}
				}
				else if( ship->DeathClock.ElapsedSeconds() >= RespawnDelay )
				{
					if( ship->CanRespawn )
					{
						bool ready_to_spawn = true;
						
						// If part of a group, wait to spawn with them.
						if( ship->Group )
						{
							for( std::map<uint32_t,Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
							{
								if( (ship_iter->second != ship) && (ship_iter->second->Team == ship->Team) && (ship_iter->second->Group == ship->Group) && ((ship_iter->second->Health > 0.) || (ship_iter->second->DeathClock.ElapsedSeconds() < RespawnDelay)) && (group_spawns[ ship->Team ].find( ship->Group ) == group_spawns[ ship->Team ].end()) )
									ready_to_spawn = false;
							}
						}
						
						// By default, the position is randomized.
						bool fixed_spawn = false;
						Pos3D spawn_at;
						
						// Defend/Destroy mode requires fighters to spawn with their capital ship.
						if( ready_to_spawn && (GameType == XWing::GameType::DEFEND_DESTROY) )
						{
							ready_to_spawn = false;
							fixed_spawn = true;
							for( std::map<uint32_t,Ship*>::const_iterator spawn_at_iter = ships.begin(); spawn_at_iter != ships.end(); spawn_at_iter ++ )
							{
								const Ship *spawn_at_ship = spawn_at_iter->second;
								if( spawn_at_ship->IsMissionObjective && (spawn_at_ship->Team == ship->Team) && (spawn_at_ship->Health > 0.) )
								{
									ready_to_spawn = true;
									spawn_at.Copy( spawn_at_ship );
									spawn_at.MoveAlong( &(spawn_at_ship->Up), Rand::Double( -20., 20. ) - spawn_at_ship->Radius() );
									spawn_at.MoveAlong( &(spawn_at_ship->Fwd), Rand::Double( -50., 50. ) );
									spawn_at.Fwd.Copy( &(spawn_at_ship->Right) );
									Vec3D to_center( -(spawn_at_ship->X), -(spawn_at_ship->Y), -(spawn_at_ship->Z) );
									to_center.ScaleTo( 1. );
									double towards_center = spawn_at.Fwd.Dot(&to_center);
									if( towards_center < -0.5 )
										spawn_at.Fwd.ScaleBy( -1. );
									else if( (towards_center < 0.5) && Rand::Bool() )
										spawn_at.Fwd.ScaleBy( -1. );
									spawn_at.MoveAlong( &(spawn_at.Fwd), spawn_at_ship->Radius() * 1.5 + Rand::Double( 0., 50. ) );
									break;
								}
							}
						}
						
						if( ready_to_spawn )
						{
							ship->Reset();
							ship->SpecialUpdate = true;
							if( fixed_spawn )
							{
								ship->SetPos( spawn_at.X, spawn_at.Y, spawn_at.Z );
								ship->Fwd.Copy( &(spawn_at.Fwd) );
								ship->Up.Copy( &(spawn_at.Up) );
							}
							else if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
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
								ship->SetUpVec( 0., 0., 1. );
								ship->SetFwdVec( -(ship->X), -(ship->Y), -(ship->Z) );
							}
							ship->FixVectors();
							
							if( ship->Group )
							{
								// Spawn groups together.
								if( group_spawns[ ship->Team ].find( ship->Group ) != group_spawns[ ship->Team ].end() )
								{
									group_spawns[ ship->Team ][ ship->Group ].MoveAlong( &(group_spawns[ ship->Team ][ ship->Group ].Fwd), -8. * ship->Radius() );
									group_spawns[ ship->Team ][ ship->Group ].MoveAlong( &(group_spawns[ ship->Team ][ ship->Group ].Right), 4. * ship->Radius() );
									ship->Copy( &(group_spawns[ ship->Team ][ ship->Group ]) );
								}
								else
									group_spawns[ ship->Team ][ ship->Group ].Copy( ship );
							}
							
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
					else
						remove_object_ids.insert( ship->ID );
				}
			}
			else if( obj_iter->second->Type() == XWing::Object::TURRET )
			{
				Turret *turret = (Turret*) obj_iter->second;
				
				if( turret->Health > 0. )
				{
					GameObject *target = NULL;
					GameObject *parent = NULL;
					Ship *parent_ship = NULL;
					
					if( turret->ParentID )
					{
						parent = Data.GetObject( turret->ParentID );
						if( parent && (parent->Type() == XWing::Object::SHIP) )
							parent_ship = (Ship*) parent;
						
						if( parent_ship && turret->ParentControl )
						{
							// This turret uses the parent's target.
							turret->Target = ((Ship*) parent)->Target;
							target = Data.GetObject( turret->Target );
						}
					}
					
					if( (! target) && (! turret->ParentControl) )
					{
						// If the turret doesn't have a target yet, and is allowed to select its own, pick one.
						
						// Set initial values very far away; any ships closer than this are worth considering.
						double nearest_enemy = 100000.;
						double nearest_friendly = nearest_enemy;
						Ship *friendly = NULL;
						
						for( std::map<uint32_t,Ship*>::iterator target_iter = ships.begin(); target_iter != ships.end(); target_iter ++ )
						{
							// Don't target itself or its parent ship.
							if( (target_iter->first == turret->ID) || (target_iter->first == turret->ParentID) )
								continue;
							
							// Ignore ships that have been dead for a while.
							if( (target_iter->second->Health <= 0.) && (target_iter->second->DeathClock.ElapsedSeconds() > 4.) )
								continue;
							
							// If the turret has a forced target direction, consider ships within that arc only.
							if( turret->TargetArc < 360. )
							{
								Vec3D vec_to_target( target_iter->second->X - turret->X, target_iter->second->Y - turret->Y, target_iter->second->Z - turret->Z );
								if( turret->TargetDir.AngleBetween( vec_to_target ) > (turret->TargetArc / 2.) )
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
							turret->SetYaw( (fabs(i_dot_right) >= 0.25) ? Num::Sign(i_dot_right) : (i_dot_right*4.) );
							
							if( turret->ParentControl )
								turret->Firing = parent_ship && parent_ship->Firing;
							else
								turret->Firing = (dist_to_target < turret->MaxFiringDist) && ((i_dot_up > -0.01) || (t_dot_up > -0.01));
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
							add_object_ids.insert( shot_id );
						}
					}
				}
				else
					remove_object_ids.insert( turret->ID );
			}
		}
		
		
		// Tell the clients about any objects we've added, like shots.
		
		std::list<GameObject*> add_objects;
		
		for( std::set<uint32_t>::iterator id_iter = add_object_ids.begin(); id_iter != add_object_ids.end(); id_iter ++ )
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
			
			for( std::set<uint32_t>::iterator id_iter = remove_object_ids.begin(); id_iter != remove_object_ids.end(); id_iter ++ )
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
		
		if( (GameType == XWing::GameType::BATTLE_OF_YAVIN) && (! empire_objectives) )
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
				if( (! empire_objectives) || ((rebel_count == 0) && (! Respawn)) )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 3.;
				}
			}
			else if( GameType == XWing::GameType::CAPITAL_SHIP_HUNT )
			{
				if( DefendingTeam == XWing::Team::EMPIRE )
				{
					if( (! empire_objectives) || ((rebel_count == 0) && (! Respawn)) )
					{
						State = XWing::State::ROUND_WILL_END;
						RoundEndedDelay = 5.;
					}
				}
				else if( DefendingTeam == XWing::Team::REBEL )
				{
					if( (! rebel_objectives) || ((empire_count == 0) && (! Respawn)) )
					{
						State = XWing::State::ROUND_WILL_END;
						RoundEndedDelay = 5.;
					}
				}
			}
			else if( GameType == XWing::GameType::DEFEND_DESTROY )
			{
				if( (! empire_objectives) || (! rebel_objectives) || ((rebel_count == 0) && (empire_count == 0) && (! Respawn)) )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 5.;
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
					if( ! empire_objectives )
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
				else if( GameType == XWing::GameType::CAPITAL_SHIP_HUNT )
				{
					if( DefendingTeam == XWing::Team::EMPIRE )
					{
						if( ! empire_objectives )
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
					else if( DefendingTeam == XWing::Team::REBEL )
					{
						if( rebel_objectives )
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
				}
				else if( GameType == XWing::GameType::DEFEND_DESTROY )
				{
					if( rebel_objectives > empire_objectives )
					{
						victor = XWing::Team::REBEL;
						victor_name = "The Rebel Alliance";
					}
					else if( empire_objectives > rebel_objectives )
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
		RespawnDelay = 10.;
		PlayersTakeEmptyShips = false;
		bool ffa = false;
		KillLimit = 0;
		TimeLimit = 0;
		uint32_t empire_ship_type = Ship::TYPE_ISD2;
		uint32_t rebel_ship_type = Ship::TYPE_CORVETTE;
		Alerts.clear();
		Waypoints.clear();
		Squadrons.clear();
		
		if( Data.Properties["gametype"] == "team_elim" )
		{
			PlayersTakeEmptyShips = (Data.Properties["respawn"] == "true");
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
			RespawnDelay = 5.;
		}
		else if( Data.Properties["gametype"] == "ffa_dm" )
		{
			GameType = XWing::GameType::FFA_DEATHMATCH;
			ffa = true;
			Respawn = true;
			KillLimit = atoi( Data.Properties["dm_kill_limit"].c_str() );
			RespawnDelay = 5.;
		}
		else if( Data.Properties["gametype"] == "yavin" )
		{
			GameType = XWing::GameType::BATTLE_OF_YAVIN;
			TimeLimit = atoi( Data.Properties["yavin_time_limit"].c_str() );
			Respawn = (Data.Properties["respawn"] == "true");
			RespawnDelay = 10.;
			
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
		else if( Data.Properties["gametype"] == "hunt" )
		{
			GameType = XWing::GameType::CAPITAL_SHIP_HUNT;
			TimeLimit = atoi( Data.Properties["hunt_time_limit"].c_str() );
			Respawn = (Data.Properties["respawn"] == "true");
			DefendingTeam = (Data.Properties["defending_team"] == "rebel") ? XWing::Team::REBEL : XWing::Team::EMPIRE;
			RespawnDelay = 15.;
		}
		else if( Data.Properties["gametype"] == "def_des" )
		{
			GameType = XWing::GameType::DEFEND_DESTROY;
			Respawn = (Data.Properties["respawn"] == "true");
			RespawnDelay = 15.;
		}
		
		if( Data.Properties["rebel_ship"] == "crv" )
			rebel_ship_type = Ship::TYPE_CORVETTE;
		else if( Data.Properties["rebel_ship"] == "frg" )
			rebel_ship_type = Ship::TYPE_NEBULON_B;
		else if( Data.Properties["rebel_ship"] == "crs" )
			rebel_ship_type = Ship::TYPE_CALAMARI_CRUISER;
		else if( Data.Properties["rebel_ship"] == "isd2" )
			rebel_ship_type = Ship::TYPE_ISD2;
		else if( Data.Properties["rebel_ship"] == "x/w" )
			rebel_ship_type = Ship::TYPE_XWING;
		else if( Data.Properties["rebel_ship"] == "y/w" )
			rebel_ship_type = Ship::TYPE_YWING;
		else if( Data.Properties["rebel_ship"] == "t/f" )
			rebel_ship_type = Ship::TYPE_TIE_FIGHTER;
		
		if( Data.Properties["empire_ship"] == "crv" )
			empire_ship_type = Ship::TYPE_CORVETTE;
		else if( Data.Properties["empire_ship"] == "frg" )
			empire_ship_type = Ship::TYPE_NEBULON_B;
		else if( Data.Properties["empire_ship"] == "crs" )
			empire_ship_type = Ship::TYPE_CALAMARI_CRUISER;
		else if( Data.Properties["empire_ship"] == "isd2" )
			empire_ship_type = Ship::TYPE_ISD2;
		else if( Data.Properties["empire_ship"] == "x/w" )
			empire_ship_type = Ship::TYPE_XWING;
		else if( Data.Properties["empire_ship"] == "y/w" )
			empire_ship_type = Ship::TYPE_YWING;
		else if( Data.Properties["empire_ship"] == "t/f" )
			empire_ship_type = Ship::TYPE_TIE_FIGHTER;
		
		if( Data.Properties["ai_flock"] == "true" )
			AIFlock = true;
		else
			AIFlock = false;
		
		// Clear all objects and tell the clients to do likewise.
		Data.ClearObjects();
		Packet obj_clear = Packet( Raptor::Packet::OBJECTS_CLEAR );
		Net.SendAll( &obj_clear );
		
		// Add gametype objects.
		if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
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
			
			// Allow server variable to set distance to exhaust port.
			double trench_length = atof( Data.Properties["yavin_dist"].c_str() );
			if( trench_length <= 0. )
				trench_length = 15000.;
			
			// Exhaust port floor.
			DeathStarBox *exhaust_box = new DeathStarBox();
			exhaust_box->Copy( deathstar );
			exhaust_box->W = deathstar->TrenchWidth;
			exhaust_box->L = exhaust_box->W;
			exhaust_box->H = exhaust_box->W;
			exhaust_box->MoveAlong( &(deathstar->Fwd), trench_length );
			exhaust_box->MoveAlong( &(deathstar->Up), exhaust_box->H / 4. - deathstar->TrenchDepth );
			exhaust_box->Pitch( 15. );
			Data.AddObject( exhaust_box );
			
			// Exhaust port.
			Ship *exhaust_port = new Ship();
			exhaust_port->Copy( exhaust_box );
			exhaust_port->SetType( Ship::TYPE_EXHAUST_PORT );
			exhaust_port->IsMissionObjective = true;
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
			back_wall->MoveAlong( &(deathstar->Fwd), trench_length + 200. );
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
			back_turret->MoveAlong( &(deathstar->Fwd), trench_length - 100. );
			back_turret->MoveAlong( &(deathstar->Up), -deathstar->TrenchDepth );
			Data.AddObject( back_turret );
			
			// Wall in front of last turret.
			DeathStarBox *front_wall = new DeathStarBox();
			front_wall->Copy( deathstar );
			front_wall->W = deathstar->TrenchWidth;
			front_wall->H = 10.;
			front_wall->L = front_wall->H / 2.;
			front_wall->MoveAlong( &(deathstar->Fwd), trench_length - 200. );
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
				box->MoveAlong( &(deathstar->Fwd), Rand::Double( trench_length - 1000., trench_length + 1000. ) );
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
				box->MoveAlong( &(deathstar->Fwd), Rand::Double( -10000., trench_length + 5000. ) );
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
			for( double fwd = 2550.; fwd < (trench_length - 699.9); fwd += 900. )
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
			for( double fwd = 2800.; fwd < (trench_length - 499.9); fwd += 900. )
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
			for( double fwd = -1000.; fwd < (trench_length - 999.9); fwd += 300. )
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
		else
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
			
			// Add hunt target ships.
			if( GameType == XWing::GameType::CAPITAL_SHIP_HUNT )
			{
				if( DefendingTeam == XWing::Team::EMPIRE )
				{
					Ship *ship = SpawnShip( empire_ship_type, DefendingTeam );
					ship->IsMissionObjective = true;
					ship->CanRespawn = false;
					ship->X = Rand::Double( -3500., -2500. );
					ship->Y = Rand::Double( -500., 500. );
					ship->Z = Rand::Double( -1000., -500. );
					ship->SetFwdVec( 1., 0., 0. );
					ship->SetUpVec( 0., 0., 1. );
					ship->SetThrottle( 0.1, 120. );
					if( empire_ship_type == Ship::TYPE_ISD2 )
						ship->Name = "Devastator";
					else if( empire_ship_type == Ship::TYPE_NEBULON_B )
						ship->Name = "Warspite";
					else
						ship->Name = "Empire Flagship";
				}
				else
				{
					Ship *ship = SpawnShip( rebel_ship_type, DefendingTeam );
					ship->IsMissionObjective = true;
					ship->CanRespawn = false;
					ship->X = Rand::Double( 2500., 3500. );
					ship->Y = Rand::Double( -500., 500. );
					ship->Z = Rand::Double( -1000., -500. );
					ship->SetFwdVec( -1., 0., 0. );
					ship->SetUpVec( 0., 0., 1. );
					ship->SetThrottle( 0.1, 120. );
					if( rebel_ship_type == Ship::TYPE_CORVETTE )
						ship->Name = "Tantive IV";
					else if( rebel_ship_type == Ship::TYPE_NEBULON_B )
						ship->Name = "Redemption";
					else if( rebel_ship_type == Ship::TYPE_CALAMARI_CRUISER )
						ship->Name = "Independence";
					else
						ship->Name = "Rebel Flagship";
				}
			}
			
			// Add Defend/Destroy base ships and their circular route.
			else if( GameType == XWing::GameType::DEFEND_DESTROY )
			{
				Ship *rebel_ship = SpawnShip( rebel_ship_type, XWing::Team::REBEL );
				rebel_ship->IsMissionObjective = true;
				rebel_ship->CanRespawn = false;
				rebel_ship->X = 2000.;
				rebel_ship->Y = 0.;
				rebel_ship->Z = 0.;
				rebel_ship->SetFwdVec( 0., -1., 0. );
				rebel_ship->SetUpVec( 0., 0., 1. );
				rebel_ship->SetThrottle( 1., 120. );
				if( rebel_ship_type == Ship::TYPE_CORVETTE )
					rebel_ship->Name = "Korolev";
				else if( rebel_ship_type == Ship::TYPE_NEBULON_B )
					rebel_ship->Name = "Redemption";
				else if( rebel_ship_type == Ship::TYPE_CALAMARI_CRUISER )
					rebel_ship->Name = "Independence";
				else
					rebel_ship->Name = "Rebel Flagship";
				
				Ship *empire_ship = SpawnShip( empire_ship_type, XWing::Team::EMPIRE );
				empire_ship->IsMissionObjective = true;
				empire_ship->CanRespawn = false;
				empire_ship->X = -2000.;
				empire_ship->Y = 0.;
				empire_ship->Z = 0.;
				empire_ship->SetFwdVec( 0., 1., 0. );
				empire_ship->SetUpVec( 0., 0., 1. );
				empire_ship->SetThrottle( 1., 120. );
				if( empire_ship_type == Ship::TYPE_ISD2 )
					empire_ship->Name = "Devastator";
				else if( empire_ship_type == Ship::TYPE_NEBULON_B )
					empire_ship->Name = "Warspite";
				else
					empire_ship->Name = "Empire Flagship";
				
				for( double theta = 2. * M_PI; theta - 0.01 >= 0. * M_PI; theta -= M_PI / 8. )
					Waypoints[ 0 ].push_back(Pos3D( 2000. * cos(theta), 2000. * sin(theta), 0. ));
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
		
		std::map< uint32_t, std::map<uint8_t,Pos3D> > group_spawns;
		
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
			if( (Data.Players.size() - spectator_count) == 1 )
			{
				// Only one player, so pick the best team for the scenario.
				if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
					rebel = true;
				else if( GameType == XWing::GameType::CAPITAL_SHIP_HUNT )
					rebel = (DefendingTeam != XWing::Team::REBEL);
			}
			
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
			else if( player_iter->second->Properties["ship"] == "CRV" )
				ship_type = Ship::TYPE_CORVETTE;
			
			if( ! ffa )
			{
				if( rebel && (ship_type == Ship::TYPE_TIE_FIGHTER) )
					ship_type = Ship::TYPE_XWING;
				else if( (! rebel) && ((ship_type == Ship::TYPE_XWING) || (ship_type == Ship::TYPE_YWING)) )
					ship_type = Ship::TYPE_TIE_FIGHTER;
			}
			
			int group = ffa ? 0 : atoi( player_iter->second->Properties["group"].c_str() );
			std::string squadron = rebel ? "Rogue" : "Omega";
			
			Ship *ship = new Ship();
			ship->PlayerID = player_iter->second->ID;
			ship->Team = (ffa ? XWing::Team::NONE : (rebel ? XWing::Team::REBEL : XWing::Team::EMPIRE));
			ship->Group = group;
			ship->CanRespawn = Respawn;
			ship->SetType( ship_type );
			ship->Name = squadron + std::string(" ") + Num::ToString( (int) Squadrons[ squadron ].size() + 1 );
			ship->X = Rand::Double( -100., 100. ) + (rebel ? 1500. : -1500.);
			ship->Y = Rand::Double( -100., 100. ) * (ffa ? 10. : 1.);
			ship->Z = Rand::Double( -100., 100. ) * (ffa ? 10. : 1.);
			ship->SetFwdVec( (rebel && (GameType != XWing::GameType::BATTLE_OF_YAVIN) ? -1. : 1.), 0., 0. );
			ship->SetUpVec( 0., 0., 1. );
			
			if( ship->Group )
			{
				// Spawn groups together.
				if( group_spawns[ ship->Team ].find( ship->Group ) != group_spawns[ ship->Team ].end() )
				{
					group_spawns[ ship->Team ][ ship->Group ].MoveAlong( &(group_spawns[ ship->Team ][ ship->Group ].Fwd), -8. * ship->Radius() );
					group_spawns[ ship->Team ][ ship->Group ].MoveAlong( &(group_spawns[ ship->Team ][ ship->Group ].Right), 4. * ship->Radius() );
					ship->Copy( &(group_spawns[ ship->Team ][ ship->Group ]) );
				}
				else
					group_spawns[ ship->Team ][ ship->Group ].Copy( ship );
			}
			
			Data.AddObject( ship );
			ShipScores[ ship->ID ] = 0;
			Squadrons[ squadron ].insert( ship->ID );
		}
		
		// Add AI ships based on the number of waves selected.
		int ai_in_first_wave = 4;
		int ai_in_other_waves = 4;
		if( (GameType == XWing::GameType::TEAM_ELIMINATION) || (GameType == XWing::GameType::FFA_ELIMINATION) )
		{
			ai_in_first_wave = 8;
			ai_in_other_waves = 6;
		}
		else if( (GameType == XWing::GameType::TEAM_DEATHMATCH) || (GameType == XWing::GameType::FFA_DEATHMATCH) )
		{
			ai_in_first_wave = 6;
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
				
				uint32_t ship_type = rebel ? rebel_ship_type : (uint32_t) Ship::TYPE_TIE_FIGHTER;
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
			std::list<std::string> spawn_list = Str::SplitToList( spawn->second, "," );
			for( std::list<std::string>::iterator spawn_iter = spawn_list.begin(); spawn_iter != spawn_list.end(); spawn_iter ++ )
			{
				if( *spawn_iter == "isd2" )
				{
					Ship *ship = SpawnShip( Ship::TYPE_ISD2, XWing::Team::EMPIRE );
					ship->CanRespawn = false;
					ship->X = Rand::Double( -1000., 1000. );
					ship->Y = Rand::Double( -2500., -1500. );
					ship->Z = Rand::Double( -1000., 1000. ) + ((GameType == XWing::GameType::BATTLE_OF_YAVIN) ? 2500. : 0.);
					ship->SetFwdVec( 0., 1., 0. );
					ship->SetUpVec( 0., 0., 1. );
					ship->SetThrottle( 0.1, 120. );
					ship->Name = "Imperator";
				}
				else if( *spawn_iter == "crv" )
				{
					Ship *ship = SpawnShip( Ship::TYPE_CORVETTE, XWing::Team::REBEL );
					ship->CanRespawn = false;
					ship->X = Rand::Double( -500., 500. );
					ship->Y = Rand::Double( 1500., 2500. );
					ship->Z = Rand::Double( -500., 500. ) + ((GameType == XWing::GameType::BATTLE_OF_YAVIN) ? 2000. : 0.);
					ship->SetFwdVec( 0., -1., 0. );
					ship->SetUpVec( 0., 0., 1. );
					ship->SetThrottle( 0.1, 120. );
					ship->Name = "Korolev";
				}
				else if( *spawn_iter == "frg" )
				{
					Ship *ship = SpawnShip( Ship::TYPE_NEBULON_B, XWing::Team::REBEL );
					ship->CanRespawn = false;
					ship->X = Rand::Double( -500., 500. );
					ship->Y = Rand::Double( 1500., 2500. );
					ship->Z = Rand::Double( -500., 500. ) + ((GameType == XWing::GameType::BATTLE_OF_YAVIN) ? 2000. : 0.);
					ship->SetFwdVec( 0., -1., 0. );
					ship->SetUpVec( 0., 0., 1. );
					ship->SetThrottle( 0.1, 120. );
					ship->Name = "Salvation";
				}
				else if( *spawn_iter == "frg-i" )
				{
					Ship *ship = SpawnShip( Ship::TYPE_NEBULON_B, XWing::Team::EMPIRE );
					ship->CanRespawn = false;
					ship->X = Rand::Double( -500., 500. );
					ship->Y = Rand::Double( -2500., -1500. );
					ship->Z = Rand::Double( -500., 500. ) + ((GameType == XWing::GameType::BATTLE_OF_YAVIN) ? 2000. : 0.);
					ship->SetFwdVec( 0., -1., 0. );
					ship->SetUpVec( 0., 0., 1. );
					ship->SetThrottle( 0.1, 120. );
					ship->Name = "Warspite";
				}
				else if( *spawn_iter == "crs" )
				{
					Ship *ship = SpawnShip( Ship::TYPE_CALAMARI_CRUISER, XWing::Team::REBEL );
					ship->CanRespawn = false;
					ship->X = Rand::Double( -1000., 1000. );
					ship->Y = Rand::Double( 1500., 2500. );
					ship->Z = Rand::Double( -1000., 1000. ) + ((GameType == XWing::GameType::BATTLE_OF_YAVIN) ? 2500. : 0.);
					ship->SetFwdVec( 0., -1., 0. );
					ship->SetUpVec( 0., 0., 1. );
					ship->SetThrottle( 0.1, 120. );
					ship->Name = "Defiance";
				}
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
				
				bool ffa = (GameType == XWing::GameType::FFA_DEATHMATCH);
				uint32_t team = (ffa ? XWing::Team::NONE : (rebel ? XWing::Team::REBEL : XWing::Team::EMPIRE));
				
				// By default, the global Respawn variable allows respawn, and the position is randomized.
				bool ready_to_spawn = Respawn;
				bool fixed_spawn = false;
				Pos3D spawn_at;
				
				// Defend/Destroy mode requires fighters to spawn with their capital ship.
				if( ready_to_spawn && (GameType == XWing::GameType::DEFEND_DESTROY) )
				{
					ready_to_spawn = false;
					fixed_spawn = true;
					for( std::map<uint32_t,GameObject*>::const_iterator spawn_at_iter = Data.GameObjects.begin(); spawn_at_iter != Data.GameObjects.end(); spawn_at_iter ++ )
					{
						if( spawn_at_iter->second->Type() != XWing::Object::SHIP )
							continue;
						
						const Ship *spawn_at_ship = (const Ship*)( spawn_at_iter->second );
						
						if( spawn_at_ship->IsMissionObjective && (spawn_at_ship->Team == team) && (spawn_at_ship->Health > 0.) )
						{
							ready_to_spawn = true;
							spawn_at.Copy( spawn_at_ship );
							spawn_at.MoveAlong( &(spawn_at_ship->Up), Rand::Double( -20., 20. ) - spawn_at_ship->Radius() );
							spawn_at.MoveAlong( &(spawn_at_ship->Fwd), Rand::Double( -50., 50. ) );
							spawn_at.Fwd.Copy( &(spawn_at_ship->Right) );
							Vec3D to_center( -(spawn_at_ship->X), -(spawn_at_ship->Y), -(spawn_at_ship->Z) );
							to_center.ScaleTo( 1. );
							double towards_center = spawn_at.Fwd.Dot(&to_center);
							if( towards_center < -0.5 )
								spawn_at.Fwd.ScaleBy( -1. );
							else if( (towards_center < 0.5) && Rand::Bool() )
								spawn_at.Fwd.ScaleBy( -1. );
							spawn_at.MoveAlong( &(spawn_at.Fwd), spawn_at_ship->Radius() * 1.5 + Rand::Double( 0., 50. ) );
							break;
						}
					}
				}
				
				if( ready_to_spawn )
				{
					uint32_t ship_type = rebel ? Ship::TYPE_XWING : Ship::TYPE_TIE_FIGHTER;
					if( player_iter->second->Properties["ship"] == "X/W" )
						ship_type = Ship::TYPE_XWING;
					else if( player_iter->second->Properties["ship"] == "Y/W" )
						ship_type = Ship::TYPE_YWING;
					else if( player_iter->second->Properties["ship"] == "T/F" )
						ship_type = Ship::TYPE_TIE_FIGHTER;
					else if( player_iter->second->Properties["ship"] == "ISD2" )
						ship_type = Ship::TYPE_ISD2;
					else if( player_iter->second->Properties["ship"] == "CRV" )
						ship_type = Ship::TYPE_CORVETTE;
					
					int group = ffa ? 0 : atoi( player_iter->second->Properties["group"].c_str() );
					std::string squadron = rebel ? "Rogue" : "Omega";
					
					Ship *ship = new Ship();
					ship->PlayerID = player_iter->second->ID;
					ship->Team = team;
					ship->Group = group;
					ship->CanRespawn = true;
					ship->SetType( ship_type );
					ship->Name = squadron + std::string(" ") + Num::ToString( (int) Squadrons[ squadron ].size() + 1 );
					if( fixed_spawn )
					{
						ship->SetPos( spawn_at.X, spawn_at.Y, spawn_at.Z );
						ship->Fwd.Copy( &(spawn_at.Fwd) );
						ship->Up.Copy( &(spawn_at.Up) );
					}
					else if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
					{
						ship->SetPos( Rand::Double(-500.,500.) + (rebel ? 1000. : -3000.), Rand::Double(-500.,500.), Rand::Double(0.,1000.) );
						ship->SetUpVec( 0., 0., 1. );
						ship->SetFwdVec( 1., 0., 0. );
					}
					else
					{
						ship->X = Rand::Double(-500.,500.) + (rebel ? 1500. : -1500.);
						ship->Y = Rand::Double(-500.,500.) * (ffa ? 4. : 1.);
						ship->Z = Rand::Double(-500.,500.) * (ffa ? 4. : 1.);
						ship->SetUpVec( 0., 0., 1. );
						ship->SetFwdVec( (rebel && (GameType != XWing::GameType::BATTLE_OF_YAVIN) ? -1. : 1.), 0., 0. );
					}
					ship->FixVectors();
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


Ship *XWingServer::SpawnShip( uint32_t ship_type, uint32_t team )
{
	Ship *ship = new Ship();
	ship->Data = &Data;
	ship->Team = team;
	ship->SetType( ship_type );
	Data.AddObject( ship );
	ShipScores[ ship->ID ] = 0;
	
	std::map<std::string,ModelObject>::const_iterator turret_iter = ship->Shape.Objects.find( "Turrets" );
	if( turret_iter != ship->Shape.Objects.end() )
	{
		Vec3D up( 0., 1., 0. );
		for( std::vector<Vec3D>::const_iterator point_iter = turret_iter->second.Points.begin(); point_iter != turret_iter->second.Points.end(); point_iter ++ )
		{
			Turret *turret = new Turret();
			turret->Data = &Data;
			turret->Weapon = (team == XWing::Team::EMPIRE) ? Shot::TYPE_TURBO_LASER_GREEN : Shot::TYPE_TURBO_LASER_RED;
			if( turret->Weapon == Shot::TYPE_TURBO_LASER_RED )
				turret->FiringMode = 1;
			turret->Attach( ship, &(*point_iter), &up, false );
			Data.AddObject( turret );
		}
		for( std::vector< std::vector<Vec3D> >::const_iterator line_iter = turret_iter->second.Lines.begin(); line_iter != turret_iter->second.Lines.end(); line_iter ++ )
		{
			if( line_iter->size() >= 2 )
			{
				up = (*line_iter).at( 1 ) - (*line_iter).at( 0 );
				up.ScaleTo( 1. );
				Turret *turret = new Turret();
				turret->Data = &Data;
				turret->Weapon = (team == XWing::Team::EMPIRE) ? Shot::TYPE_TURBO_LASER_GREEN : Shot::TYPE_TURBO_LASER_RED;
				if( turret->Weapon == Shot::TYPE_TURBO_LASER_RED )
					turret->FiringMode = 1;
				if( line_iter->size() >= 3 )
				{
					turret->Fwd = (*line_iter).at( 2 ) - (*line_iter).at( 0 );
					turret->Fwd.ScaleTo( 1. );
					turret->TargetArc = 180.;
				}
				turret->Attach( ship, &((*line_iter).at( 0 )), &up, false );
				Data.AddObject( turret );
			}
		}
	}
	
	return ship;
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
