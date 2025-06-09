/*
 *  XWingServer.cpp
 */

#include "XWingServer.h"

#include <cstddef>
#include <cmath>
#include <stdint.h>
#include <dirent.h>
#include <list>
#include <algorithm>
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
#include "DockingBay.h"
#include "Checkpoint.h"
#include "Math3D.h"
#include "RaptorGame.h"


XWingServer::XWingServer( std::string version ) : RaptorServer( "X-Wing Revival", version )
{
	GameType = XWing::GameType::TEAM_ELIMINATION;
	PlayerTeam = XWing::Team::NONE;
	PlayersTakeEmptyShips = false;
	Respawn = false;
	RespawnDelay = 10.;
	RebelCruiserRespawn = EmpireCruiserRespawn = 60.;
	AllowShipChange = true;
	AllowTeamChange = false;
	RoundEndedDelay = 3.;
	KillLimit = 10;
	TimeLimit = 0;
	Checkpoints = 20;
	CheckpointFirstTouch = false;
	DefendingTeam = XWing::Team::NONE;
	CollisionGroup = 0;
	CountdownFrom = 5;
	CountdownSent = 0;
}


XWingServer::~XWingServer()
{
}


std::map<std::string,std::string> XWingServer::DefaultProperties( void ) const
{
	std::map<std::string,std::string> defaults;
	defaults["gametype"] = "fleet";
	defaults["mission"] = "rebel0";
	defaults["ai_waves"] = "3";
	defaults["ai_skill"] = "1";
	defaults["ai_flock"] = "false";
	defaults["rebel_fighter"]  = "X/W";
	defaults["rebel_bomber"]   = "Y/W";
	defaults["rebel_cruiser"]  = "CRV";
	defaults["rebel_cruisers"] = "3";
	defaults["rebel_frigate"]  = "FRG";
	defaults["rebel_frigates"] = "2";
	defaults["rebel_flagship"] = "FRG";
	defaults["empire_fighter"]  = "T/F";
	defaults["empire_bomber"]   = "T/B";
	defaults["empire_cruiser"]  = "INT";
	defaults["empire_cruisers"] = "3";
	defaults["empire_frigate"]  = "VSD";
	defaults["empire_frigates"] = "2";
	defaults["empire_flagship"] = "ISD";
	defaults["yavin_rebel_fighter"]  = "X/W";
	defaults["yavin_rebel_bomber"]   = "Y/W";
	defaults["yavin_empire_fighter"] = "T/F";
	defaults["ai_empire_ratio"] = "1";
	defaults["respawn"] = "true";
	defaults["respawn_time"] = "";
	defaults["ai_finish"] = "false";
	defaults["dm_kill_limit"] = "10";
	defaults["tdm_kill_limit"] = "30";
	defaults["team_race_checkpoints"] = "30";
	defaults["ffa_race_checkpoints"] = "20";
	defaults["race_time_limit"] = "0";
	defaults["race_lap"] = "0";
	defaults["race_circuit"] = "false";
	defaults["yavin_time_limit"] = "15";
	defaults["yavin_turrets"] = "120";
	defaults["yavin_repair_bay"] = "true";
	defaults["hunt_time_limit"] = "10";
	defaults["defending_team"] = "empire";
	defaults["asteroids"] = "16";
	defaults["bg"] = "stars";
	defaults["allow_ship_change"] = "true";
	defaults["allow_team_change"] = "false";
	defaults["allow_pause"] = "false";
	defaults["permissions"] = "all";
	defaults["skip_countdown"] = "false";
	defaults["time_scale"] = "1";
	return defaults;
}


void XWingServer::Started( void )
{
	ShipClasses.clear();
	if( DIR *dir_p = opendir("Ships") )
	{
		while( struct dirent *dir_entry_p = readdir(dir_p) )
		{
			if( dir_entry_p->d_name[ 0 ] == '.' )
				continue;
			if( ! CStr::EndsWith( dir_entry_p->d_name, ".def" ) )
				continue;
			
			ShipClass sc;
			if( sc.Load( std::string("Ships/") + std::string(dir_entry_p->d_name) ) )
				ShipClasses.insert( sc );
		}
		closedir( dir_p );
	}
	
	Missions.clear();
	if( (Raptor::Game->Cfg.SettingAsString("screensaver_gametype") == "mission") || ! Raptor::Game->Cfg.SettingAsBool("screensaver") )  // FIXME: Thread locking?
	{
		if( DIR *dir_p = opendir("Missions") )
		{
			while( struct dirent *dir_entry_p = readdir(dir_p) )
			{
				if( dir_entry_p->d_name[ 0 ] == '.' )
					continue;
				if( ! CStr::EndsWith( dir_entry_p->d_name, ".def" ) )
					continue;
				
				std::string filename = std::string(dir_entry_p->d_name);
				Mission mission;
				if( mission.Load( std::string("Missions/") + filename ) )
					Missions[ filename.substr( 0, filename.length() - 4 ) ] = mission;
			}
			closedir( dir_p );
		}
	}
	
	ResetToStartingObjects();
	ResetToDefaultProperties();
	
	Alerts.clear();
	EventTriggers.clear();
	Waypoints.clear();
	Squadrons.clear();
	GroupStagger.clear();
	GroupJumpingIn.clear();
	TeamScores.clear();
	ShipScores.clear();
	Cheaters.clear();
	
	Raptor::Game->Cfg.Load( "server.cfg" );  // FIXME: Thread locking?
	
	SelectMission();
	
	if( State >= Raptor::State::CONNECTING )
		State = XWing::State::LOBBY;
}


void XWingServer::Stopped( void )
{
	Data.Clear();
	ShipClasses.clear();
	Alerts.clear();
	EventTriggers.clear();
	Waypoints.clear();
	Squadrons.clear();
	GroupStagger.clear();
	GroupJumpingIn.clear();
	TeamScores.clear();
	ShipScores.clear();
	Cheaters.clear();
}


bool XWingServer::HandleCommand( std::string cmd, std::vector<std::string> *params )
{
	if( RaptorServer::HandleCommand( cmd, params ) )
		return true;
	
	if( cmd == "defaults" )
	{
		ResetToDefaultProperties();
	}
	else if( cmd == "spawn" )
	{
		if( State < XWing::State::FLYING )
			ConsolePrint( "Patience, my young apprentice.", TextConsole::MSG_ERROR );
		else if( params && params->size() && GetShipClass(params->at(0)) )
		{
			EventTriggers[ MissionEvent::TRIGGER_ALWAYS ].push_back( MissionEvent( MissionEvent::TRIGGER_ALWAYS ) );
			EventTriggers[ MissionEvent::TRIGGER_ALWAYS ].back().Time = RoundTimer.ElapsedSeconds();  // Prevent JumpProgress from starting at 1.
			EventTriggers[ MissionEvent::TRIGGER_ALWAYS ].back().SpawnClass = params->at(0);
			if( params->size() >= 2 )
				EventTriggers[ MissionEvent::TRIGGER_ALWAYS ].back().SpawnName = params->at(1);
			for( size_t i = 2; i < params->size(); i ++ )
			{
				if( Str::EqualsInsensitive( params->at(i), "rebel" ) )
					EventTriggers[ MissionEvent::TRIGGER_ALWAYS ].back().SpawnFlags |= MissionEvent::SPAWNFLAG_REBEL;
				else if( Str::EqualsInsensitive( params->at(i), "empire" ) )
					EventTriggers[ MissionEvent::TRIGGER_ALWAYS ].back().SpawnFlags |= MissionEvent::SPAWNFLAG_EMPIRE;
				else if( Str::EqualsInsensitive( params->at(i), "objective" ) )
					EventTriggers[ MissionEvent::TRIGGER_ALWAYS ].back().SpawnFlags |= MissionEvent::SPAWNFLAG_OBJECTIVE;
				else if( Str::EqualsInsensitive( params->at(i), "respawn" ) )
					EventTriggers[ MissionEvent::TRIGGER_ALWAYS ].back().SpawnFlags |= MissionEvent::SPAWNFLAG_RESPAWN;
				/*
				else if( Str::EqualsInsensitive( params->at(i), "hero" ) )
					EventTriggers[ MissionEvent::TRIGGER_ALWAYS ].back().SpawnFlags |= MissionEvent::SPAWNFLAG_HERO;
				*/
			}
		}
		else
			ConsolePrint( "Usage: spawn <class> \"<name>\" [<team>]", TextConsole::MSG_ERROR );
	}
	else
		return false;
	return true;
}


bool XWingServer::ProcessPacket( Packet *packet, ConnectedClient *from_client )
{
	packet->Rewind();
	PacketType type = packet->Type();
	
	if( type == XWing::Packet::FLY )
	{
		if( State >= XWing::State::FLYING )
			BeginFlying( from_client ? from_client->PlayerID : 0 );
		else if( Data.PropertyAsBool("skip_countdown") || Raptor::Game->Cfg.SettingAsBool("screensaver") )
			BeginFlying();
		else
			ToggleCountdown( from_client ? Data.GetPlayer(from_client->PlayerID) : NULL );
		
		return true;
	}
	else if( type == XWing::Packet::LOBBY )
	{
		if( State >= XWing::State::FLYING )
		{
			Data.Lock.Lock();
			
			Player *player = from_client ? Data.GetPlayer( from_client->PlayerID ) : NULL;
			if( player )
				player->Properties[ "team" ] = "";
			
			size_t players_still_flying = 0;
			for( std::map<uint16_t,Player*>::const_iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
			{
				std::string assigned_team = player_iter->second->PropertyAsString("team");
				if( ! assigned_team.empty() )
					players_still_flying ++;
			}
			
			Data.Lock.Unlock();
			
			if( ! players_still_flying )
			{
				// If nobody else is flying, end the round.
				Net.SendAll( packet );
				
				State = XWing::State::LOBBY;
				ResetToStartingObjects();
				
				Packet message( Raptor::Packet::MESSAGE );
				message.AddString( player ? (std::string("Mission aborted by ") + player->Name + std::string(".")) : "Misson aborted." );
				Net.SendAll( &message );
			}
			else if( from_client )
			{
				// Send just this player back to the lobby.
				from_client->Send( packet );
				
				// Revoke any ship/turret ownership.
				Data.Lock.Lock();
				for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
				{
					if( (obj_iter->second->PlayerID == from_client->PlayerID) && (obj_iter->second->Type() != XWing::Object::SHOT) )
						obj_iter->second->PlayerID = 0;
				}
				Data.Lock.Unlock();
				
				if( player )
				{
					Packet message( Raptor::Packet::MESSAGE );
					message.AddString( player->Name + std::string(" exited to lobby.") );
					message.AddUInt( TextConsole::MSG_CHAT );
					Net.SendAll( &message );
				}
			}
		}
		
		return true;
	}
	else if( type == XWing::Packet::RESET_DEFAULTS )
	{
		if( State < XWing::State::FLYING )
			ResetToDefaultProperties();
	}
	else if( type == XWing::Packet::CHANGE_SEAT )
	{
		uint8_t seat = packet->NextUChar();
		uint8_t turret_mode = packet->Remaining() ? packet->NextUChar() : 0;
		
		Player *player = (from_client && from_client->PlayerID) ? Data.GetPlayer( from_client->PlayerID ) : NULL;
		if( player )
		{
			Ship *player_ship = NULL;
			Turret *player_turret = NULL;
			
			for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
			{
				if( obj_iter->second->Type() == XWing::Object::SHIP )
				{
					Ship *ship = (Ship*) obj_iter->second;
					if( ship->PlayerID == from_client->PlayerID )
						player_ship = ship;
				}
				else if( obj_iter->second->Type() == XWing::Object::TURRET )
				{
					Turret *turret = (Turret*) obj_iter->second;
					if( turret->PlayerID == from_client->PlayerID )
						player_turret = turret;
				}
			}
			
			std::set<GameObject*> update_objects;
			
			if( player_turret && ! seat )
			{
				// Moving from turret to cockpit.
				
				if( player_ship )
				{
					player_turret->PlayerID = 0;
					update_objects.insert( player_turret );
				}
				else
				{
					Ship *parent = player_turret->ParentShip();
					if( parent && (parent->Group != 255) && ! parent->IsMissionObjective )
					{
						if( parent->PlayerID && (parent->PlayerID != from_client->PlayerID) )
						{
							// If another player owns the ship, check where they are sitting.
							std::list<Turret*> turrets = parent->AttachedTurrets();
							for( std::list<Turret*>::const_iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
							{
								if( (*turret_iter)->PlayerID == parent->PlayerID )
								{
									// If the pilot is in a turret, allow this player to fly the ship.
									parent->PlayerID = 0;
									break;
								}
							}
							
							// Allow taking the cockpit seat from a disconnected player.
							if( parent->PlayerID && ! Data.GetPlayer(parent->PlayerID) )
								parent->PlayerID = 0;
							
							if( ! parent->PlayerID )
							{
								Packet message( Raptor::Packet::MESSAGE );
								message.AddString( player->Name + std::string(" took the pilot's seat.") );
								
								for( std::list<Turret*>::const_iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
									Net.SendToPlayer( &message, (*turret_iter)->PlayerID );
							}
						}
						
						if( ! parent->PlayerID )
						{
							parent->PlayerID = from_client->PlayerID;
							player_turret->PlayerID = 0;
							update_objects.insert( player_turret );
							update_objects.insert( parent );
						}
					}
				}
			}
			else if( seat )
			{
				// Moving to a turret.
				
				if( player_turret && ! player_ship )
					player_ship = player_turret->ParentShip();
				
				if( player_ship )
				{
					uint32_t target_id = player_ship->Target;
					Turret *turret = player_ship->AttachedTurret( seat - 1 );
					if( turret && ! (turret->PlayerID && Data.GetPlayer(turret->PlayerID)) )
					{
						if( player_turret && (player_turret != turret) )
						{
							// Moving from a different turret.
							target_id = player_turret->Target;
							player_turret->PlayerID = 0;
							// FIXME: Reset turret firing mode?
							update_objects.insert( player_turret );
						}
						
						turret->PlayerID = from_client->PlayerID;
						turret->Target = target_id;
						if( turret_mode && turret->GunWidth )
							turret->FiringMode = turret_mode;
						update_objects.insert( turret );
					}
				}
			}
			
			if( update_objects.size() )
			{
				// Notify everyone about changes of ownership.
				
				int8_t precision = 126;
				Packet update_packet( Raptor::Packet::UPDATE );
				update_packet.AddChar( precision );
				update_packet.AddUInt( update_objects.size() );
				for( std::set<GameObject*>::const_iterator update_iter = update_objects.begin(); update_iter != update_objects.end(); update_iter ++ )
				{
					update_packet.AddUInt( (*update_iter)->ID );
					(*update_iter)->AddToUpdatePacketFromServer( &update_packet, precision );
				}
				Net.SendAll( &update_packet );
			}
		}
		
		return true;
	}
	else if( type == XWing::Packet::TOGGLE_COPILOT )
	{
		Player *player = (from_client && from_client->PlayerID) ? Data.GetPlayer( from_client->PlayerID ) : NULL;
		if( player )
		{
			Turret *player_turret = NULL;
			for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
			{
				if( obj_iter->second->Type() == XWing::Object::TURRET )
				{
					Turret *turret = (Turret*) obj_iter->second;
					if( turret->PlayerID == from_client->PlayerID )
					{
						player_turret = turret;
						break;
					}
				}
			}
			
			if( player_turret )
			{
				Ship *parent = player_turret->ParentShip();
				if( parent && (parent->Group != 255) && ! parent->IsMissionObjective )
				{
					bool changed = false;
					std::list<Turret*> turrets = parent->AttachedTurrets();
					
					if( parent->PlayerID && (parent->PlayerID != from_client->PlayerID) && Data.GetPlayer(parent->PlayerID) )
					{
						// If another player owns the ship, check where they are sitting.
						for( std::list<Turret*>::const_iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
						{
							if( (*turret_iter)->PlayerID == parent->PlayerID )
							{
								// If the pilot is in a turret, allow this player to give control to the co-pilot (Chewie).
								parent->PlayerID = 0;
								changed = true;
								break;
							}
						}
					}
					else if( parent->PlayerID )
					{
						// Give the co-pilot control of your own ship.
						parent->PlayerID = 0;
						changed = true;
					}
					else
					{
						// Stop the co-pilot.
						parent->PlayerID = from_client->PlayerID;
						changed = true;
					}
					
					if( changed )
					{
						if( ! parent->PlayerID )
						{
							Packet chewie( XWing::Packet::TOGGLE_COPILOT );
							
							Packet message( Raptor::Packet::MESSAGE );
							message.AddString( player->Name + std::string(" handed the controls to the co-pilot.") );
							
							for( std::list<Turret*>::const_iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
							{
								Net.SendToPlayer( &chewie,  (*turret_iter)->PlayerID );
								Net.SendToPlayer( &message, (*turret_iter)->PlayerID );
							}
						}
						
						// Notify everyone about change of ownership.
						parent->SendUpdate( 126 );
					}
				}
			}
		}
		
		return true;
	}
	else if( (type == XWing::Packet::ENGINE_SOUND) || (type == Raptor::Packet::PLAY_SOUND) )
	{
		//std::string sound = packet->NextString();  // Not needed; we'll just forward this packet.
		
		Player *player = (from_client && from_client->PlayerID) ? Data.GetPlayer( from_client->PlayerID ) : NULL;
		if( player )
		{
			Ship *player_ship = NULL;
			
			for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
			{
				if( obj_iter->second->Type() == XWing::Object::SHIP )
				{
					Ship *ship = (Ship*) obj_iter->second;
					if( ship->PlayerID == from_client->PlayerID )
					{
						player_ship = ship;
						break;
					}
				}
				else if( obj_iter->second->Type() == XWing::Object::TURRET )
				{
					Turret *turret = (Turret*) obj_iter->second;
					if( turret->PlayerID == from_client->PlayerID )
					{
						player_ship = turret->ParentShip();
						break;
					}
				}
			}
			
			if( player_ship )
			{
				if( player_ship->PlayerID && (player_ship->PlayerID != player->ID) )
					Net.SendToPlayer( packet, player_ship->PlayerID );
				
				std::list<Turret*> turrets = player_ship->AttachedTurrets();
				for( std::list<Turret*>::const_iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
				{
					if( (*turret_iter)->PlayerID && ((*turret_iter)->PlayerID != player->ID) )
						Net.SendToPlayer( packet, (*turret_iter)->PlayerID );
				}
			}
		}
		
		return true;
	}
	else if( type == XWing::Packet::EXPLOSION )  // Self-destruct.
	{
		uint32_t obj_id = packet->NextUInt();
		
		GameObject *obj = Data.GetObject( obj_id );
		Player *owner = obj ? obj->Owner() : NULL;
		if( owner && (owner->ID == from_client->PlayerID) )
		{
			if( obj->Type() == XWing::Object::SHIP )
			{
				Ship *ship = (Ship*) obj;
				ship->SetHealth( 0. );
				
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
				if( ship->Category() == ShipClass::CATEGORY_CAPITAL )
				{
					explosion.AddUChar( 30 );    // Sub-Explosions
					explosion.AddFloat( 1.75f ); // Speed Scale
					explosion.AddFloat( 0.66f ); // Speed Scale Sub
				}
				Net.SendAll( &explosion );
				
				if( State < XWing::State::ROUND_ENDED )
				{
					ShipKilled( ship );
					
					// Notify players.
					Packet message( Raptor::Packet::MESSAGE );
					char cstr[ 1024 ] = "";
					snprintf( cstr, 1024, "%s ejected.", owner ? owner->Name.c_str() : ship->Name.c_str() );
					message.AddString( cstr );
					Net.SendAll( &message );
				}
			}
		}
		
		return true;
	}
	else if( type == XWing::Packet::UPLOAD_MISSION )
	{
		if( Data.PropertyAsString("permissions") != "all" )  // FIXME: Allow if player->Properties["admin"] == "true"?
		{
			if( from_client && from_client->PlayerID )
			{
				Packet message( Raptor::Packet::MESSAGE );
				message.AddString( "Mission upload disabled by host." );
				message.AddUInt( TextConsole::MSG_ERROR );
				Net.SendToPlayer( &message, from_client->PlayerID );
			}
			
			return true;
		}
		
		std::string mission_id   = packet->NextString();
		std::string mission_body = packet->NextString();
		
		Player *player = (from_client && from_client->PlayerID) ? Data.GetPlayer( from_client->PlayerID ) : NULL;
		Mission mission;
		
		if( player && mission_id.length() && mission_body.length() && mission.Parse( mission_body ) )
		{
			Missions[ mission_id + std::string("*") ] = mission;
			
			Packet message( Raptor::Packet::MESSAGE );
			message.AddString( player->Name + std::string(" uploaded mission: ") + mission.PropertyAsString( "mission_name", mission_id ) );
			message.AddUInt( TextConsole::MSG_CHAT );
			Net.SendAll( &message );
			
			// FIXME: Instead of this silly hack, create a new packet type to update mission list!
			if( State == XWing::State::LOBBY )
			{
				Packet lobby_packet( XWing::Packet::LOBBY );
				lobby_packet.AddUShort( Missions.size() );
				for( std::map<std::string,Mission>::iterator mission_iter = Missions.begin(); mission_iter != Missions.end(); mission_iter ++ )
				{
					lobby_packet.AddString( mission_iter->first );
					lobby_packet.AddString( mission_iter->second.PropertyAsString( "mission_name", mission_iter->first ) );
				}
				lobby_packet.AddUChar( 1 );
				lobby_packet.AddUInt( XWing::ServerFeature::ZERO_LAG );  // FIXME: For v0.4.x, probably not needed anymore.
				Net.SendAll( &lobby_packet );
			}
			
			SetProperty( "mission", mission_id + std::string("*") );
		}
		else if( from_client && from_client->PlayerID )
		{
			Packet message( Raptor::Packet::MESSAGE );
			message.AddString( std::string("Failed to parse uploaded mission: ") + mission_id );
			message.AddUInt( TextConsole::MSG_ERROR );
			Net.SendToPlayer( &message, from_client->PlayerID );
		}
		
		return true;
	}
	
	return RaptorServer::ProcessPacket( packet, from_client );
}


bool XWingServer::CompatibleVersion( std::string version ) const
{
	// Allow compatibility between minor revisions (such as 0.5 and 0.5.1).
	if( RaptorServer::CompatibleVersion(version) || (strncasecmp( version.c_str(), Version.c_str(), 3 ) == 0) )
		return true;
	
	return false;
}


void XWingServer::AcceptedClient( ConnectedClient *client )
{
	if( client->PlayerID && (((client->IP & 0xFF000000) >> 24) == 127) )
	{
		// Localhost client gets admin rights.
		Player *player = Data.GetPlayer( client->PlayerID );
		if( player )
			player->Properties["admin"] = "true";
		
		client->Precision = 1;
	}
	
	RaptorServer::AcceptedClient( client );
	
	Packet lobby_packet( XWing::Packet::LOBBY );
	lobby_packet.AddUShort( Missions.size() );
	for( std::map<std::string,Mission>::iterator mission_iter = Missions.begin(); mission_iter != Missions.end(); mission_iter ++ )
	{
		lobby_packet.AddString( mission_iter->first );
		lobby_packet.AddString( mission_iter->second.Properties["mission_name"] );
	}
	lobby_packet.AddUChar( 1 );
	lobby_packet.AddUInt( XWing::ServerFeature::ZERO_LAG );  // FIXME: For v0.4.x, probably not needed anymore.
	client->Send( &lobby_packet );
}


void XWingServer::DroppedClient( ConnectedClient *client )
{
	RaptorServer::DroppedClient( client );
}


bool XWingServer::SetPlayerProperty( Player *player, std::string name, std::string value, bool force )
{
	// Set player team in lobby so team voice works as expected.
	if( player && (State == XWing::State::LOBBY) && (name == "ship") )
	{
		const ShipClass *ship_class = GetShipClass( value );
		if( ship_class && ship_class->Team )
			RaptorServer::SetPlayerProperty( player, "team", (ship_class->Team == XWing::Team::EMPIRE) ? "Empire" : "Rebel" );
		else
			RaptorServer::SetPlayerProperty( player, "team", "" );
	}
	
	return RaptorServer::SetPlayerProperty( player, name, value, force );
}


void XWingServer::Update( double dt )
{
	dt *= Data.TimeScale;
	
	RoundTimer.SetTimeScale( Data.TimeScale );
	
	for( std::map<uint16_t,Clock>::iterator clock_iter = RespawnClocks.begin(); clock_iter != RespawnClocks.end(); clock_iter ++ )
		clock_iter->second.SetTimeScale( Data.TimeScale );
	
	// Endgame scores allow pausing, but otherwise ignore time scale.
	Data.Lock.Lock();
	size_t num_players = Data.Players.size();
	Data.Lock.Unlock();
	if( (Data.TimeScale == 1.) || ((Data.TimeScale < 0.01) && (num_players == 1)) )
		RoundEndedTimer.SetTimeScale( Data.TimeScale );
	
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
		double round_time = RoundTimer.ElapsedSeconds();
		double round_time_remaining = RoundTimeRemaining();
		
		// NOTE: Clients look at these properties to know what is allowed, so for now we let the server change them mid-game.
		AllowShipChange = Data.PropertyAsBool("allow_ship_change",true);
		AllowTeamChange = Data.PropertyAsBool("allow_team_change",false);
		
		
		// Build a list of all ships.
		
		std::map<uint32_t,Ship*> ships;
		std::list<Shot*> shots;
		std::map<uint32_t,Turret*> turrets;
		std::map<uint32_t,Asteroid*> asteroids;
		Ship *rebel_flagship = NULL, *empire_flagship = NULL, *exhaust_port = NULL;
		DeathStar *deathstar = NULL;
		std::set<Ship*> respawn_waiting;
		
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHIP )
			{
				Ship *ship = (Ship*) obj_iter->second;
				ships[ ship->ID ] = ship;
				if( (GameType == XWing::GameType::FLEET_BATTLE) && (ship->IsMissionObjective || ship->Group >= 254) && (ship->Health > 0.) && (ship->Team == XWing::Team::REBEL) )
				{
					if( ! rebel_flagship )
						rebel_flagship = ship;
					else if( ship->IsMissionObjective && ! rebel_flagship->IsMissionObjective )
						rebel_flagship = ship;
					else if( (ship->IsMissionObjective == rebel_flagship->IsMissionObjective) && (ship->Health > rebel_flagship->Health) )
						rebel_flagship = ship;
				}
				if( (GameType == XWing::GameType::FLEET_BATTLE) && (ship->IsMissionObjective || ship->Group >= 254) && (ship->Health > 0.) && (ship->Team == XWing::Team::EMPIRE) )
				{
					if( ! empire_flagship )
						empire_flagship = ship;
					else if( ship->IsMissionObjective && ! empire_flagship->IsMissionObjective )
						empire_flagship = ship;
					else if( (ship->IsMissionObjective == empire_flagship->IsMissionObjective) && (ship->Health > empire_flagship->Health) )
						empire_flagship = ship;
				}
				if( ship->Category() == ShipClass::CATEGORY_TARGET )
					exhaust_port = ship;
			}
			else if( obj_iter->second->Type() == XWing::Object::SHOT )
				shots.push_back( (Shot*) obj_iter->second );
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
				Player *player1 = ship1->Owner();
				Player *player2 = ship2->Owner();
				bool died1 = false, died2 = false;
				char punctuation = '.';
				
				#define SLOW_SPEED (100.)
				#define SLOW_DAMAGE (0.04)
				
				if( prev_health1 > 0. )
				{
					// If the other ship was already dead, divide its collision damage by how long it has been dead.
					double collision_damage = ship2->CollisionPotential;
					bool low_speed_complex = false;
					if( prev_health2 <= 0. )
						collision_damage /= (1 + ship2->DeathClock.ElapsedSeconds());
					else
					{
						// Both ships alive.  If the speed difference is low, gradually damage them.
						double relative_speed = (ship1->MotionVector - ship2->MotionVector).Length();
						if( relative_speed < SLOW_SPEED )
						{
							// Repeat collision check at 4Hz.
							if( (ship1->HitClock.ElapsedSeconds() < 0.25) && (ship2->HitClock.ElapsedSeconds() < 0.25) )
								continue;
							
							collision_damage = std::max<double>( SLOW_DAMAGE, (relative_speed / SLOW_SPEED) ) * std::min<double>( ship1->CollisionPotential, ship2->CollisionPotential );
							low_speed_complex = ship2->ComplexCollisionDetection();
						}
					}
					
					std::map<std::string,double>::const_iterator subsystem_iter = ship1->Subsystems.end();
					bool subsystem_was_intact = false;
					if( collision_iter->FirstObject.length() )
					{
						subsystem_iter = ship1->Subsystems.find( collision_iter->FirstObject );
						subsystem_was_intact = (subsystem_iter != ship1->Subsystems.end()) && (subsystem_iter->second > 0.);
						ship1->AddDamage( collision_damage / 2., collision_damage / 2., collision_iter->FirstObject.c_str() );
					}
					else if( low_speed_complex )
					{
						if( collision_iter->Location.X || collision_iter->Location.Y || collision_iter->Location.Z )
						{
							if( collision_iter->Location.DistAlong( &(ship1->Fwd), ship1 ) >= 0. )
								ship1->AddDamage( collision_damage, 0. );
							else
								ship1->AddDamage( 0., collision_damage );
						}
						else
							ship1->AddDamage( collision_damage / 2., collision_damage / 2. );
					}
					else
					{
						Vec3D vec_to_other( ship2->PrevPos.X - ship1->X, ship2->PrevPos.Y - ship1->Y, ship2->PrevPos.Z - ship1->Z );
						if( ship1->Fwd.Dot(&vec_to_other) >= 0. )
							ship1->AddDamage( collision_damage, 0. );
						else
							ship1->AddDamage( 0., collision_damage );
					}
					
					if( (subsystem_iter != ship1->Subsystems.end()) && (subsystem_iter->second <= 0.) )
					{
						// Show an explosion to players for the destroyed subsystem.
						std::map<std::string,ModelObject*>::iterator obj_iter = ship1->Shape.Objects.find( subsystem_iter->first );
						double radius = ship1->Radius();
						Vec3D offset;
						if( obj_iter != ship1->Shape.Objects.end() )
						{
							radius = obj_iter->second->GetMaxRadius();
							Pos3D center = obj_iter->second->GetCenterPoint();
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
						
						// Destroying any subsystem on a mission objective also counts as an objective destroyed.
						if( (ship1->IsMissionObjective || (ship1->Group == 255)) && (ship1->Health > 0.) && subsystem_was_intact )
						{
							Player *attacker = ship2->Owner();
							if( attacker )
							{
								std::string attacker_team = ChosenTeam( attacker );
								int add_score = 0; // Discourage kamikazi.
								if( ((ship1->Team == XWing::Team::REBEL) && (attacker_team == "Rebel")) || ((ship1->Team == XWing::Team::EMPIRE) && (attacker_team == "Empire")) )
									add_score = -1;
								SetPlayerProperty( attacker, "kills_c", Num::ToString( attacker->PropertyAsInt("kills_c") + add_score ) );
							}
						}
					}
					
					if( ship1->Health <= 0. )
					{
						died1 = true;
						
						if( ship1->IsMissionObjective || (ship1->Group == 255) )
							punctuation = '!';
						
						// Don't let debris and chase camera pass through a larger ship.
						if( ship1->Radius() * 5. < ship2->Radius() )
							ship1->MotionVector.Copy( &(ship2->MotionVector) );
						
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
						if( ship1->Category() == ShipClass::CATEGORY_CAPITAL )
						{
							explosion.AddUChar( 30 );    // Sub-Explosions
							explosion.AddFloat( 1.75f ); // Speed Scale
							explosion.AddFloat( 0.66f ); // Speed Scale Sub
						}
						Net.SendAll( &explosion );
						
						if( State < XWing::State::ROUND_ENDED )
						{
							ShipKilled( ship1, ((ship2->Category() == ShipClass::CATEGORY_CAPITAL) || (ship1->Team && (ship1->Team == ship2->Team))) ? NULL : ship2 );
							send_scores = true;
						}
					}
					else if( ship2->CollisionPotential )
					{
						// Send the hit.
						Packet hit( XWing::Packet::MISC_HIT_SHIP );
						hit.AddUInt( ship1->ID );
						hit.AddFloat( ship1->Health );
						hit.AddFloat( ship1->ShieldF );
						hit.AddFloat( ship1->ShieldR );
						hit.AddUChar( ship1->HitFlags );
						if( subsystem_iter != ship1->Subsystems.end() )
						{
							hit.AddString( subsystem_iter->first.c_str() );
							hit.AddFloat( subsystem_iter->second );
						}
						else
							hit.AddString( "" );
						if( collision_iter->Location.X || collision_iter->Location.X || collision_iter->Location.Z )
						{
							hit.AddDouble( collision_iter->Location.X );
							hit.AddDouble( collision_iter->Location.Y );
							hit.AddDouble( collision_iter->Location.Z );
						}
						else
						{
							hit.AddDouble( ship2->PrevPos.X );
							hit.AddDouble( ship2->PrevPos.Y );
							hit.AddDouble( ship2->PrevPos.Z );
						}
						hit.AddDouble( sqrt( ship2->Radius() ) * 4. * collision_damage / ship2->CollisionPotential );
						Net.SendAll( &hit );
					}
				}
				
				if( prev_health2 > 0. )
				{
					// If the other ship was already dead, divide its collision damage by how long it has been dead.
					double collision_damage = ship1->CollisionPotential;
					bool low_speed_complex = false;
					if( prev_health1 <= 0. )
						collision_damage /= (1 + ship1->DeathClock.ElapsedSeconds());
					else
					{
						// Both ships alive.  If the speed difference is low, gradually damage them.
						double relative_speed = (ship1->MotionVector - ship2->MotionVector).Length();
						if( relative_speed < SLOW_SPEED )
						{
							collision_damage = std::max<double>( SLOW_DAMAGE, (relative_speed / SLOW_SPEED) ) * std::min<double>( ship1->CollisionPotential, ship2->CollisionPotential );
							low_speed_complex = ship1->ComplexCollisionDetection();
						}
					}
					
					std::map<std::string,double>::const_iterator subsystem_iter = ship2->Subsystems.end();
					bool subsystem_was_intact = false;
					if( collision_iter->SecondObject.length() )
					{
						subsystem_iter = ship2->Subsystems.find( collision_iter->SecondObject );
						subsystem_was_intact = (subsystem_iter != ship2->Subsystems.end()) && (subsystem_iter->second > 0.);
						ship2->AddDamage( collision_damage / 2., collision_damage / 2., collision_iter->SecondObject.c_str() );
					}
					else if( low_speed_complex )
					{
						if( collision_iter->Location.X || collision_iter->Location.Y || collision_iter->Location.Z )
						{
							if( collision_iter->Location.DistAlong( &(ship2->Fwd), ship2 ) >= 0. )
								ship2->AddDamage( collision_damage, 0. );
							else
								ship2->AddDamage( 0., collision_damage );
						}
						else
							ship2->AddDamage( collision_damage / 2., collision_damage / 2. );
					}
					else
					{
						Vec3D vec_to_other( ship1->PrevPos.X - ship2->X, ship1->PrevPos.Y - ship2->Y, ship1->PrevPos.Z - ship2->Z );
						if( ship2->Fwd.Dot(&vec_to_other) >= 0. )
							ship2->AddDamage( collision_damage, 0. );
						else
							ship2->AddDamage( 0., collision_damage );
					}
					
					if( (subsystem_iter != ship2->Subsystems.end()) && (subsystem_iter->second <= 0.) )
					{
						// Show an explosion to players for the destroyed subsystem.
						std::map<std::string,ModelObject*>::iterator obj_iter = ship2->Shape.Objects.find( subsystem_iter->first );
						double radius = ship2->Radius();
						Vec3D offset;
						if( obj_iter != ship2->Shape.Objects.end() )
						{
							radius = obj_iter->second->GetMaxRadius();
							Pos3D center = obj_iter->second->GetCenterPoint();
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
						
						// Destroying any subsystem on a mission objective also counts as an objective destroyed.
						if( (ship2->IsMissionObjective || (ship2->Group == 255)) && (ship2->Health > 0.) && subsystem_was_intact )
						{
							Player *attacker = ship1->Owner();
							if( attacker )
							{
								std::string attacker_team = ChosenTeam( attacker );
								int add_score = 0; // Discourage kamikazi.
								if( ((ship2->Team == XWing::Team::REBEL) && (attacker_team == "Rebel")) || ((ship2->Team == XWing::Team::EMPIRE) && (attacker_team == "Empire")) )
									add_score = -1;
								SetPlayerProperty( attacker, "kills_c", Num::ToString( attacker->PropertyAsInt("kills_c") + add_score ) );
							}
						}
					}
					
					if( ship2->Health <= 0. )
					{
						died2 = true;
						
						if( ship2->IsMissionObjective || (ship2->Group == 255) )
							punctuation = '!';
						
						// Don't let debris and chase camera pass through a larger ship.
						if( ship2->Radius() * 5. < ship1->Radius() )
							ship2->MotionVector.Copy( &(ship1->MotionVector) );
						
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
						if( ship2->Category() == ShipClass::CATEGORY_CAPITAL )
						{
							explosion.AddUChar( 30 );    // Sub-Explosions
							explosion.AddFloat( 1.75f ); // Speed Scale
							explosion.AddFloat( 0.66f ); // Speed Scale Sub
						}
						Net.SendAll( &explosion );
						
						if( State < XWing::State::ROUND_ENDED )
						{
							ShipKilled( ship2, ((ship1->Category() == ShipClass::CATEGORY_CAPITAL) || (ship2->Team && (ship1->Team == ship2->Team))) ? NULL : ship1 );
							send_scores = true;
						}
					}
					else if( ship1->CollisionPotential )
					{
						// Send the hit.
						Packet hit( XWing::Packet::MISC_HIT_SHIP );
						hit.AddUInt( ship2->ID );
						hit.AddFloat( ship2->Health );
						hit.AddFloat( ship2->ShieldF );
						hit.AddFloat( ship2->ShieldR );
						hit.AddUChar( ship2->HitFlags );
						if( subsystem_iter != ship2->Subsystems.end() )
						{
							hit.AddString( subsystem_iter->first.c_str() );
							hit.AddFloat( subsystem_iter->second );
						}
						else
							hit.AddString( "" );
						if( collision_iter->Location.X || collision_iter->Location.X || collision_iter->Location.Z )
						{
							hit.AddDouble( collision_iter->Location.X );
							hit.AddDouble( collision_iter->Location.Y );
							hit.AddDouble( collision_iter->Location.Z );
						}
						else
						{
							hit.AddDouble( ship1->PrevPos.X );
							hit.AddDouble( ship1->PrevPos.Y );
							hit.AddDouble( ship1->PrevPos.Z );
						}
						hit.AddDouble( sqrt( ship1->Radius() ) * 4. * collision_damage / ship1->CollisionPotential );
						Net.SendAll( &hit );
					}
				}
				
				// If the impact killed one ship, its debris can no longer hurt the other.
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
						snprintf( cstr, 1024, "%s and %s collided%c", player1 ? player1->Name.c_str() : ship1->Name.c_str(), player2 ? player2->Name.c_str() : ship2->Name.c_str(), punctuation );
					else if( died1 )
					{
						if( prev_health2 > 0. )
							snprintf( cstr, 1024, "%s ran into %s%c", player1 ? player1->Name.c_str() : ship1->Name.c_str(), player2 ? player2->Name.c_str() : ship2->Name.c_str(), punctuation );
						else
							snprintf( cstr, 1024, "%s ran into pieces of %s%c", player1 ? player1->Name.c_str() : ship1->Name.c_str(), player2 ? player2->Name.c_str() : ship2->Name.c_str(), punctuation );
					}
					else
					{
						if( prev_health1 > 0. )
							snprintf( cstr, 1024, "%s ran into %s%c", player2 ? player2->Name.c_str() : ship2->Name.c_str(), player1 ? player1->Name.c_str() : ship1->Name.c_str(), punctuation );
						else
							snprintf( cstr, 1024, "%s ran into pieces of %s%c", player2 ? player2->Name.c_str() : ship2->Name.c_str(), player1 ? player1->Name.c_str() : ship1->Name.c_str(), punctuation );
					}
					message.AddString( cstr );
					Net.SendAll( &message );
				}
				
				if( Data.PropertyAsBool("debug") )
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
			
			else if( ((collision_iter->first->Type() == XWing::Object::SHIP) && (collision_iter->second->Type() == XWing::Object::SHOT))
			||       ((collision_iter->first->Type() == XWing::Object::SHOT) && (collision_iter->second->Type() == XWing::Object::SHIP)) )
			{
				// A shot hit a ship.
				
				Ship *ship = NULL;
				Shot *shot = NULL;
				if( collision_iter->first->Type() == XWing::Object::SHIP )
				{
					ship = (Ship*) collision_iter->first;
					shot = (Shot*) collision_iter->second;
				}
				else
				{
					ship = (Ship*) collision_iter->second;
					shot = (Shot*) collision_iter->first;
				}
				
				/*
				// Make sure each shot only hits one thing.
				if( remove_object_ids.find(shot->ID) != remove_object_ids.end() )
					continue;
				*/
				
				double prev_health = ship->Health;
				bool subsystem_was_intact = false;
				
				if( collision_iter->FirstObject.length() )
				{
					std::map<std::string,double>::const_iterator subsystem_iter = ship->Subsystems.find( collision_iter->FirstObject );
					subsystem_was_intact = (subsystem_iter != ship->Subsystems.end()) && (subsystem_iter->second > 0.);
					ship->AddDamage( shot->Damage() / 2., shot->Damage() / 2., collision_iter->FirstObject.c_str(), shot->FiredFrom, shot->HullDamage() );
				}
				else
				{
					Vec3D vec_to_shot( shot->PrevPos.X - ship->X, shot->PrevPos.Y - ship->Y, shot->PrevPos.Z - ship->Z );
					if( ship->Fwd.Dot(&vec_to_shot) >= 0. )
						ship->AddDamage( shot->Damage(), 0., NULL, shot->FiredFrom, shot->HullDamage() );
					else
						ship->AddDamage( 0., shot->Damage(), NULL, shot->FiredFrom, shot->HullDamage() );
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
					shot_hit.AddUChar( ship->HitFlags );
					if( subsystem_iter != ship->Subsystems.end() )
					{
						shot_hit.AddString( subsystem_iter->first.c_str() );
						shot_hit.AddFloat( subsystem_iter->second );
					}
					else
						shot_hit.AddString( "" );
					shot_hit.AddUChar( shot->ShotType );
					Pos3D shot_pos( shot->PrevPos );
					if( (shot->ShotType != Shot::TYPE_TORPEDO) && (shot->ShotType != Shot::TYPE_MISSILE) && (ship->Category() != ShipClass::CATEGORY_CAPITAL) )
					{
						double excess_dist = ship->DistAlong( &(shot->Fwd), &shot_pos ) - ship->Radius();
						if( excess_dist > 0. )
							shot_pos.MoveAlong( &(shot->Fwd), excess_dist );
					}
//#define PRECISE_HIT 1  // Commented-out because this sticks the hit sprites too far into the model.
#ifdef PRECISE_HIT
					if( collision_iter->Location.X || collision_iter->Location.Y || collision_iter->Location.Z )
					{
						shot_hit.AddDouble( collision_iter->Location.X );
						shot_hit.AddDouble( collision_iter->Location.Y );
						shot_hit.AddDouble( collision_iter->Location.Z );
					}
					else
#endif
					{
						shot_hit.AddDouble( shot_pos.X );
						shot_hit.AddDouble( shot_pos.Y );
						shot_hit.AddDouble( shot_pos.Z );
					}
					shot_hit.AddFloat( shot->MotionVector.X );
					shot_hit.AddFloat( shot->MotionVector.Y );
					shot_hit.AddFloat( shot->MotionVector.Z );
					shot_hit.AddUInt( shot->FiredFrom );  // FIXME: If fired from an AI turret, this should probably be replaced with the turret's ParentID.
					Net.SendAll( &shot_hit );
					
					GameObject *fired_from = Data.GetObject( shot->FiredFrom );
					TriggerEvent( MissionEvent::TRIGGER_ON_HIT, ship, fired_from );
					if( ship->Health < prev_health )
						TriggerEvent( MissionEvent::TRIGGER_ON_DAMAGE, ship, fired_from );
					
					if( (subsystem_iter != ship->Subsystems.end()) && (subsystem_iter->second <= 0.) )
					{
						// Show an explosion to players for the destroyed subsystem.
						std::map<std::string,ModelObject*>::iterator obj_iter = ship->Shape.Objects.find( subsystem_iter->first );
						double radius = ship->Radius();
						Vec3D offset;
						if( obj_iter != ship->Shape.Objects.end() )
						{
							radius = obj_iter->second->GetMaxRadius();
							Pos3D center = obj_iter->second->GetCenterPoint();
							offset = (ship->Fwd * center.X) + (ship->Up * center.Y) + (ship->Right * center.Z);
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
						if( ship->Category() == ShipClass::CATEGORY_CAPITAL )
						{
							explosion.AddUChar( 30 );    // Sub-Explosions
							explosion.AddFloat( 1.75f ); // Speed Scale
							explosion.AddFloat( 0.66f ); // Speed Scale Sub
						}
						Net.SendAll( &explosion );
						
						// Destroying any subsystem on a mission objective also counts as an objective destroyed.
						if( (ship->IsMissionObjective || (ship->Group == 255)) && (ship->Health > 0.) && subsystem_was_intact )
						{
							Player *attacker = shot->Owner();
							if( attacker )
							{
								std::string attacker_team = ChosenTeam( attacker );
								int add_score = 1;
								if( ((ship->Team == XWing::Team::REBEL) && (attacker_team == "Rebel")) || ((ship->Team == XWing::Team::EMPIRE) && (attacker_team == "Empire")) )
									add_score = -1;
								SetPlayerProperty( attacker, "kills_c", Num::ToString( attacker->PropertyAsInt("kills_c") + add_score ) );
							}
						}
					}
				}
				
				if( (shot->ShotType != Shot::TYPE_SUPERLASER) || ! Data.PropertyAsBool("superlaser_penetrate",true) )
					remove_object_ids.insert( shot->ID );
				
				// This ship just died.
				if( (ship->Health <= 0.) && (prev_health > 0.) )
				{
					if( ship->ExplosionRate() )
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
						if( ship->Category() == ShipClass::CATEGORY_CAPITAL )
						{
							explosion.AddUChar( 30 );    // Sub-Explosions
							explosion.AddFloat( 1.75f ); // Speed Scale
							explosion.AddFloat( 0.66f ); // Speed Scale Sub
						}
						Net.SendAll( &explosion );
					}
					
					// Get relevant data.
					Player *victim = ship->Owner();
					Player *killer = shot->Owner();
					GameObject *killer_obj = Data.GetObject( shot->FiredFrom );
					if( killer_obj && ! killer )
						killer = killer_obj->Owner();
					Ship   *killer_ship   = (killer_obj && (killer_obj->Type() == XWing::Object::SHIP))   ? (Ship*)   killer_obj : NULL;
					Turret *killer_turret = (killer_obj && (killer_obj->Type() == XWing::Object::TURRET)) ? (Turret*) killer_obj : NULL;
					if( killer_turret )
						killer_ship = killer_turret->ParentShip();
					
					if( State < XWing::State::ROUND_ENDED )
					{
						ShipKilled( ship, killer_obj, killer );
						send_scores = true;
						
						// Notify players.
						const char *victim_name = NULL;
						if( victim )
							victim_name = victim->Name.c_str();
						else if( ship->Name == "Exhaust Port" )
							victim_name = "The Death Star";
						else
							victim_name = ship->Name.c_str();
						
						char punctuation = (ship->IsMissionObjective || (ship->Group == 255)) ? '!' : '.';
						
						char cstr[ 1024 ] = "";
						if( (killer_obj == ship) || (victim && (victim == killer)) )
							snprintf( cstr, 1024, "%s bullseyed themself%c", victim_name, punctuation );
						else if( killer || killer_ship )
							snprintf( cstr, 1024, "%s was destroyed by %s%c", victim_name, killer ? killer->Name.c_str() : (killer_ship ? killer_ship->Name.c_str() : "somebody"), punctuation );
						else if( killer_turret )
							snprintf( cstr, 1024, "%s was destroyed by a turret%c", victim_name, punctuation );
						else
							snprintf( cstr, 1024, "%s was destroyed%c", victim_name, punctuation );
						
						Packet message( Raptor::Packet::MESSAGE );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
			}
			
			else if( ((collision_iter->first->Type() == XWing::Object::TURRET) && (collision_iter->second->Type() == XWing::Object::SHOT))
			||       ((collision_iter->first->Type() == XWing::Object::SHOT) && (collision_iter->second->Type() == XWing::Object::TURRET)) )
			{
				// A shot hit a turret.
				
				Turret *turret = NULL;
				Shot *shot = NULL;
				if( collision_iter->first->Type() == XWing::Object::TURRET )
				{
					turret = (Turret*) collision_iter->first;
					shot = (Shot*) collision_iter->second;
				}
				else
				{
					turret = (Turret*) collision_iter->second;
					shot = (Shot*) collision_iter->first;
				}
				
				/*
				// Make sure each shot only hits one thing.
				if( remove_object_ids.find(shot->ID) != remove_object_ids.end() )
					continue;
				*/
				
				double prev_health = turret->Health;
				turret->AddDamage( (shot->Damage() + shot->HullDamage()) / 2. );
				
				if( prev_health > 0. )
				{
					// Send the hit.
					Packet shot_hit( XWing::Packet::SHOT_HIT_TURRET );
					shot_hit.AddUInt( turret->ID );
					shot_hit.AddFloat( turret->Health );
					shot_hit.AddUChar( shot->ShotType );
					shot_hit.AddDouble( shot->PrevPos.X );
					shot_hit.AddDouble( shot->PrevPos.Y );
					shot_hit.AddDouble( shot->PrevPos.Z );
					Net.SendAll( &shot_hit );
					
					GameObject *fired_from = Data.GetObject( shot->FiredFrom );
					TriggerEvent( MissionEvent::TRIGGER_ON_HIT,    turret, fired_from );
					TriggerEvent( MissionEvent::TRIGGER_ON_DAMAGE, turret, fired_from );
				}
				
				if( (shot->ShotType != Shot::TYPE_SUPERLASER) || ! Data.PropertyAsBool("superlaser_penetrate",true) )
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
					
					// Update turret kill score.
					if( State < XWing::State::ROUND_ENDED )
					{
						// FIXME: Maybe rework ShipKilled to something like PlayableKilled and use it for turrets too?
						Player *victim = turret->Owner();
						if( victim )
						{
							SetPlayerProperty( victim, "deaths", Num::ToString( victim->PropertyAsInt("deaths") + 1 ) );
							send_scores = true;
						}
						Player *killer = shot->Owner();
						GameObject *killer_obj = Data.GetObject( shot->FiredFrom );
						if( killer_obj && ! killer )
							killer = killer_obj->Owner();
						uint8_t killer_team = XWing::Team::NONE;
						if( killer_obj && (killer_obj->Type() == XWing::Object::SHIP) )
							killer_team = ((Ship*)( killer_obj ))->Team;
						else if( killer_obj && (killer_obj->Type() == XWing::Object::TURRET) )
							killer_team = ((Turret*)( killer_obj ))->Team;
						if( killer && ((! turret->Team) || (killer_team != turret->Team)) )
						{
							SetPlayerProperty( killer, "kills_t", Num::ToString( killer->PropertyAsInt("kills_t") + 1 ) );
							send_scores = true;
						}
					}
				}
			}
			
			else if( ((collision_iter->first->Type() == XWing::Object::SHIP) && (collision_iter->second->Type() == XWing::Object::CHECKPOINT))
			||       ((collision_iter->first->Type() == XWing::Object::CHECKPOINT) && (collision_iter->second->Type() == XWing::Object::SHIP)) )
			{
				// A ship touched a race checkpoint.
				
				Ship *ship = NULL;
				Checkpoint *checkpoint = NULL;
				if( collision_iter->first->Type() == XWing::Object::SHIP )
				{
					ship = (Ship*) collision_iter->first;
					checkpoint = (Checkpoint*) collision_iter->second;
				}
				else
				{
					ship = (Ship*) collision_iter->second;
					checkpoint = (Checkpoint*) collision_iter->first;
				}
				
				if( (ship->NextCheckpoint == checkpoint->ID) && (ship->Health > 0.) )
				{
					ship->NextCheckpoint = checkpoint->Next;
					
					Packet checkpoint_hit( XWing::Packet::CHECKPOINT );
					checkpoint_hit.AddUInt( ship->ID );
					checkpoint_hit.AddUInt( ship->NextCheckpoint );
					checkpoint_hit.AddUChar( CheckpointFirstTouch ? 0x01 : 0x00 );
					Net.SendAll( &checkpoint_hit );
					
					if( State < XWing::State::ROUND_ENDED )
					{
						ShipScores[ ship->ID ] ++;
						Player *owner = ship->Owner();
						if( owner )
							SetPlayerProperty( owner, "score", Num::ToString( owner->PropertyAsInt("score") + 1 ) );
						
						if( GameType == XWing::GameType::TEAM_RACE )
							TeamScores[ ship->Team ] ++;
						
						send_scores = true;
						
						TriggerEvent( MissionEvent::TRIGGER_ON_CHECKPOINT, ship );
					}
					
					if( CheckpointFirstTouch )
					{
						for( std::map<uint32_t,Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
							ship_iter->second->NextCheckpoint = checkpoint->Next;
					}
				}
			}
			
			else if( ((collision_iter->first->Type() == XWing::Object::SHIP) && (collision_iter->second->Type() == XWing::Object::TURRET))
			||       ((collision_iter->first->Type() == XWing::Object::TURRET) && (collision_iter->second->Type() == XWing::Object::SHIP)) )
			{
				// A ship ran into a turret.
				
				Ship *ship = (Ship*) collision_iter->first;
				Turret *turret = (Turret*) collision_iter->second;
				if( collision_iter->first->Type() == XWing::Object::SHIP )
				{
					ship = (Ship*) collision_iter->first;
					turret = (Turret*) collision_iter->second;
				}
				else
				{
					ship = (Ship*) collision_iter->second;
					turret = (Turret*) collision_iter->first;
				}
				
				double prev_health = ship->Health;
				ship->SetHealth( 0. );
				turret->SetHealth( 0. );
				
				if( prev_health > 0. )
				{
					Player *victim = ship->Owner();
					
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
						ShipKilled( ship, turret );
						send_scores = true;
						
						// FIXME: Maybe rework ShipKilled to something like PlayableKilled and use it for turrets too?
						Player *gunner = turret->Owner();
						if( gunner )
							SetPlayerProperty( gunner, "deaths", Num::ToString( gunner->PropertyAsInt("deaths") + 1 ) );
						Player *pilot = ship->Owner();
						if( pilot && ((! turret->Team) || (ship->Team != turret->Team)) )
							SetPlayerProperty( pilot, "kills_t", Num::ToString( pilot->PropertyAsInt("kills_t") + 1 ) );
						
						// Notify players.
						Packet message( Raptor::Packet::MESSAGE );
						char cstr[ 1024 ] = "";
						snprintf( cstr, 1024, "%s ran into a turret.", victim ? victim->Name.c_str() : ship->Name.c_str() );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
			}
			
			else if( ((collision_iter->first->Type() == XWing::Object::SHIP) && (collision_iter->second->Type() == XWing::Object::DOCKING_BAY))
			||       ((collision_iter->first->Type() == XWing::Object::DOCKING_BAY) && (collision_iter->second->Type() == XWing::Object::SHIP)) )
			{
				// A ship is in a docking bay.
				
				Ship *ship = NULL;
				DockingBay *dock = NULL;
				if( collision_iter->first->Type() == XWing::Object::SHIP )
				{
					ship = (Ship*) collision_iter->first;
					dock = (DockingBay*) collision_iter->second;
				}
				else
				{
					ship = (Ship*) collision_iter->second;
					dock = (DockingBay*) collision_iter->first;
				}
				
				if( (ship->Health > 0.) && (ship->Health < ship->MaxHealth()) && (ship->HitClock.ElapsedSeconds() >= 1.5) && (dock->RepairClock.ElapsedSeconds() >= 1.5)
				&&   ship->Repair( 10. ) )
				{
					dock->RepairClock.Reset();
					
					// Send the repair.
					Packet repair( XWing::Packet::REPAIR );
					repair.AddUInt( ship->ID );
					repair.AddFloat( ship->Health );
					Net.SendAll( &repair );
				}
				else if( (ship->Health >= ship->MaxHealth()) && ((! ship->HitFlags) || (ship->HitClock.ElapsedSeconds() >= 2.5)) && (dock->RearmClock.ElapsedSeconds() >= 0.9) && (ship->LastFired() >= 0.9)
				&&        ship->Rearm( 1 ) )
				{
					dock->RearmClock.Reset();
					
					// Send the rearm.
					Packet rearm( XWing::Packet::REARM );
					rearm.AddUInt( ship->ID );
					rearm.AddUChar( 1 );
					Net.SendAll( &rearm );
				}
			}
			
			else if( (collision_iter->first->Type() == XWing::Object::SHIP) || (collision_iter->second->Type() == XWing::Object::SHIP) )
			{
				// A ship hit something else.
				
				Ship *ship = NULL;
				GameObject *hazard = NULL;
				if( collision_iter->first->Type() == XWing::Object::SHIP )
				{
					ship = (Ship*) collision_iter->first;
					hazard = collision_iter->second;
				}
				else
				{
					ship = (Ship*) collision_iter->second;
					hazard = collision_iter->first;
				}
				
				double prev_health = ship->Health;
				ship->SetHealth( 0. );
				
				// This ship just died.  // FIXME: At low speed use MISC_HIT_SHIP to apply damage instead of instant death!
				if( prev_health > 0. )
				{
					Player *victim = ship->Owner();
					
					// Don't let the dead ship's motion vector continue into the hazard.
					if( hazard->Type() == XWing::Object::DEATH_STAR )
					{
						DeathStar *deathstar = (DeathStar*) hazard;
						if( ship->MotionVector.Dot( &(deathstar->Up) ) < 0. )
							ship->MotionVector -= deathstar->Up * ship->MotionVector.Dot( &(deathstar->Up) );
						if( deathstar->WithinTrenchW(ship) )
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
					if( ship->Category() == ShipClass::CATEGORY_CAPITAL )
					{
						explosion.AddUChar( 30 );    // Sub-Explosions
						explosion.AddFloat( 1.75f ); // Speed Scale
						explosion.AddFloat( 0.66f ); // Speed Scale Sub
					}
					Net.SendAll( &explosion );
					
					// Add blastpoint to asteroid.
					if( hazard->Type() == XWing::Object::ASTEROID )
					{
						Packet shot_hit( XWing::Packet::SHOT_HIT_HAZARD ); // FIXME: Should this be its own packet type?
						shot_hit.AddUChar( Shot::TYPE_TORPEDO );
						shot_hit.AddDouble( ship->X );
						shot_hit.AddDouble( ship->Y );
						shot_hit.AddDouble( ship->Z );
						shot_hit.AddUInt( hazard->ID );
						Net.SendAll( &shot_hit );
					}
					
					if( State < XWing::State::ROUND_ENDED )
					{
						ShipKilled( ship );
						send_scores = true;
						
						// Notify players.
						Packet message( Raptor::Packet::MESSAGE );
						char cstr[ 1024 ] = "";
						const char *hazard_name = "something";
						if( hazard->Type() == XWing::Object::ASTEROID )
							hazard_name = "an asteroid";
						else if( (hazard->Type() == XWing::Object::DEATH_STAR) || (hazard->Type() == XWing::Object::DEATH_STAR_BOX) )
							hazard_name = "the Death Star";
						snprintf( cstr, 1024, "%s ran into %s%c", victim ? victim->Name.c_str() : ship->Name.c_str(), hazard_name, (ship->IsMissionObjective || (ship->Group == 255)) ? '!' : '.' );
						message.AddString( cstr );
						Net.SendAll( &message );
					}
				}
			}
			
			else if( (collision_iter->first->Type() == XWing::Object::SHOT) && (collision_iter->second->Type() == XWing::Object::SHOT) )
			{
				// Two shots collided.  (Not currently possible.)
				remove_object_ids.insert( collision_iter->first->ID );
				remove_object_ids.insert( collision_iter->second->ID );
				// FIXME: Explosion effect if either shot was missile/torpedo.
			}
			
			else if( (collision_iter->first->Type() == XWing::Object::SHOT) || (collision_iter->second->Type() == XWing::Object::SHOT) )
			{
				// A shot hit something else.
				
				Shot *shot = NULL;
				GameObject *hazard = NULL;
				if( collision_iter->first->Type() == XWing::Object::SHOT )
				{
					shot = (Shot*) collision_iter->first;
					hazard = collision_iter->second;
				}
				else
				{
					shot = (Shot*) collision_iter->first;
					hazard = collision_iter->second;
				}
				
				/*
				// Make sure each shot only hits one thing.
				if( remove_object_ids.find(shot->ID) != remove_object_ids.end() )
					continue;
				*/
				
				if( (shot->ShotType != Shot::TYPE_SUPERLASER) || ! Data.PropertyAsBool("superlaser_penetrate",true) )
					remove_object_ids.insert( shot->ID );
				
				if( (shot->ShotType == Shot::TYPE_TORPEDO) || (shot->ShotType == Shot::TYPE_MISSILE) || ((hazard->Type() == XWing::Object::ASTEROID) && shot->AsteroidDamage()) )
				{
					Packet shot_hit( XWing::Packet::SHOT_HIT_HAZARD );
					shot_hit.AddUChar( shot->ShotType );
					shot_hit.AddDouble( shot->PrevPos.X );
					shot_hit.AddDouble( shot->PrevPos.Y );
					shot_hit.AddDouble( shot->PrevPos.Z );
					shot_hit.AddUInt( hazard->ID );
					Net.SendAll( &shot_hit );
				}
				
				if( (hazard->Type() == XWing::Object::ASTEROID) && shot->AsteroidDamage() )
				{
					Asteroid *asteroid = (Asteroid*) hazard;
					asteroid->Health -= shot->AsteroidDamage();
					if( asteroid->Health <= 0. )
					{
						remove_object_ids.insert( asteroid->ID );
						
						// Show an explosion to players.
						Packet explosion( XWing::Packet::EXPLOSION );
						explosion.AddDouble( asteroid->X );
						explosion.AddDouble( asteroid->Y );
						explosion.AddDouble( asteroid->Z );
						explosion.AddFloat( asteroid->MotionVector.X );
						explosion.AddFloat( asteroid->MotionVector.Y );
						explosion.AddFloat( asteroid->MotionVector.Z );
						explosion.AddFloat( asteroid->Radius * 3.5 );        // Size
						explosion.AddFloat( log( asteroid->Radius / 20. ) ); // Loudness
						explosion.AddUChar( 0 );                             // Sub-Explosions
						explosion.AddFloat( 2.f );                           // Speed Scale
						Net.SendAll( &explosion );
						
						// If other asteroids were only held in place by touching this one, let them start spinning.
						std::set<Asteroid*> spin_asteroids;
						std::set<uint32_t> do_not_spin;
						for( std::map<uint32_t,Asteroid*>::iterator asteroid_iter = asteroids.begin(); asteroid_iter != asteroids.end(); asteroid_iter ++ )
						{
							if( (asteroid_iter->second->Health > 0.) && (do_not_spin.find(asteroid_iter->first) == do_not_spin.end())
							&&  (asteroid_iter->second->Dist( asteroid ) < (asteroid->Radius + asteroid_iter->second->Radius)) )
							{
								bool spin = true;
								std::map<uint32_t,Asteroid*>::iterator asteroid_iter2 = asteroid_iter;
								asteroid_iter2 ++;
								for( ; asteroid_iter2 != asteroids.end(); asteroid_iter2 ++ )
								{
									if( (asteroid_iter2->second->Health > 0.)
									&&  (asteroid_iter2->second->Dist( asteroid_iter->second ) < (asteroid_iter->second->Radius + asteroid_iter2->second->Radius)) )
									{
										do_not_spin.insert( asteroid_iter2->first );
										spin = false;
										break;
									}
								}
								if( spin )
									spin_asteroids.insert( asteroid_iter->second );
							}
						}
						if( spin_asteroids.size() )
						{
							Packet update_packet( Raptor::Packet::UPDATE );
							int8_t precision = -127;
							update_packet.AddChar( precision );
							update_packet.AddUInt( spin_asteroids.size() );
							for( std::set<Asteroid*>::iterator asteroid_iter = spin_asteroids.begin(); asteroid_iter != spin_asteroids.end(); asteroid_iter ++ )
							{
								double spin_scale = 10. * asteroid->Radius / (*asteroid_iter)->Radius;
								(*asteroid_iter)->RollRate  = Rand::Double( -spin_scale, spin_scale );
								(*asteroid_iter)->PitchRate = Rand::Double( -spin_scale, spin_scale );
								(*asteroid_iter)->YawRate   = Rand::Double( -spin_scale, spin_scale );
								
								update_packet.AddUInt( (*asteroid_iter)->ID );
								(*asteroid_iter)->AddToUpdatePacketFromServer( &update_packet, precision );
							}
							Net.SendAll( &update_packet );
						}
					}
				}
			}
		}
		
		// If we changed any scores, send the updated list.
		if( send_scores )
			SendScores();
		
		
		// Figure out who is flying which ship.
		
		bool respawn_coop = (Data.PropertyAsString("respawn") == "coop");
		std::map<uint16_t,Ship*> player_ships;
		size_t rebel_players = 0, empire_players = 0;
		size_t rebel_players_in_trench = 0;
		size_t player_ships_without_group = 0;
		bool ffa = (GameType == XWing::GameType::FFA_DEATHMATCH) || (GameType == XWing::GameType::FFA_ELIMINATION) || (GameType == XWing::GameType::FFA_RACE);
		for( std::map<uint32_t,Ship*>::iterator ship_iter = ships.begin(); ship_iter != ships.end(); ship_iter ++ )
		{
			const Player *player = ship_iter->second->Owner();
			if( player )
			{
				// Keep track of player-owned ships unless they changed to gunner after dying.
				if( (ship_iter->second->Health > 0.) || ! Str::EndsWith( ChosenShip(player), " Gunner" ) )
					player_ships[ player->ID ] = ship_iter->second;
				
				// No achivements for cheaters.
				if( /* (ai_skill <= 0) || */ ! ship_iter->second->PlayersCanFly() )
					Cheaters.insert( player->ID );
				
				// Count player ships on each team for AI behavior.
				if( ship_iter->second->Team == XWing::Team::REBEL )
				{
					rebel_players ++;
					if( deathstar && deathstar->WithinTrenchW(ship_iter->second) && deathstar->WithinTrenchH(ship_iter->second) )
						rebel_players_in_trench ++;
				}
				else if( ship_iter->second->Team == XWing::Team::EMPIRE )
					empire_players ++;
				
				// Update group immediately when player changes it.
				if( ! ffa )
					ship_iter->second->Group = player->PropertyAsInt("group");
				if( ! ship_iter->second->Group )
					ship_iter->second->Group = Data.PropertyAsInt("player_group");
				if( ! ship_iter->second->Group )
					player_ships_without_group ++;
				
				if( ship_iter->second->Health > 0. )
				{
					// Players in a live ship are not waiting to respawn.
					std::map<uint16_t,Clock>::iterator clock_iter = RespawnClocks.find( player->ID );
					if( clock_iter != RespawnClocks.end() )
						RespawnClocks.erase( clock_iter );
				}
			}
		}
		
		
		// Figure out who is waiting for a ship or turret.
		// FIXME: I'm sure this could be cleaner.  I made a mess trying to chase down a bug that turned out to be in Ship::Owner().
		
		std::map< uint8_t, std::list<Player*> > pilots_waiting;
		std::map< uint8_t, std::map< uint8_t,std::list<Player*> > > gunners_waiting;
		#define GUNNER_WAIT (std::min<double>( 4., RespawnDelay ))
		
		Data.Lock.Lock();
		
		for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
		{
			Player *player = player_iter->second;
			std::string chosen_team = ChosenTeam( player );
			
			// Don't worry about players still in the lobby.
			if( chosen_team.empty() )
				continue;
			
			std::map<uint16_t,Ship*>::iterator player_ship_iter = player_ships.find( player->ID );
			Ship *player_ship = (player_ship_iter != player_ships.end()) ? player_ship_iter->second : NULL;
			if( (! player_ship) || (player_ship->Health <= 0.) )
			{
				// Player has no ship, or their ship is dead.
				
				Turret *player_turret = NULL;
				uint8_t player_team = XWing::Team::NONE;
				for( std::map<uint32_t,Turret*>::const_iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
				{
					if( turret_iter->second->PlayerID == player->ID )
					{
						player_turret = turret_iter->second;
						player_team = player_turret->Team;
						break;
					}
				}
				
				std::string desired_ship = ChosenShip( player );
				if( desired_ship == "Spectator" )
				{
					// Remove player from turret if they switched to spectator and they don't own the ship.
					if( player_turret )
					{
						Ship *found_parent = player_turret->ParentShip();
						if( (! found_parent) || (found_parent->Owner() != player) )
						{
							player_turret->PlayerID = 0;
							player_turret->SendUpdate( 126 );
							player_turret = NULL;
						}
					}
					
					// Allow our old dead ship to be removed after we've switched to spectator.
					if( (! player_turret) && player_ship && (RespawnClocks[ player->ID ].ElapsedSeconds() >= GUNNER_WAIT) )
					{
						player_ship->PlayerID = 0;
						Player *other_owner = player_ship->Owner();
						if( other_owner == player )
							other_owner = NULL;
						if( (! other_owner) && (! player_ship->IsMissionObjective) && (player_ship->Group != 255) )
							player_ship->CanRespawn = false;
						player_ship->SendUpdate( 126 );
						player_ship = NULL;
					}
					
					if( AllowTeamChange && (! player_ship) && ! player_turret )
						SetPlayerProperty( player_iter->second, "team", "Spectator" );
				}
				else if( (Str::FindInsensitive( desired_ship, " Gunner" ) >= 0) && ((! player_ship) || (player_ship->DeathClock.ElapsedSeconds() >= GUNNER_WAIT)) )
				{
					player_team = Str::BeginsWith( desired_ship, "Rebel" ) ? XWing::Team::REBEL : XWing::Team::EMPIRE;
					
					// Remove player from turret if they changed team.
					if( player_turret && player_turret->Team && (player_turret->Team != player_team) )
					{
						player_turret->PlayerID = 0;
						player_turret->SendUpdate( 126 );
						player_turret = NULL;
					}
					
					// If nobody owns the ship the player is gunning for, and it's not a mission objective, treat it as the player's for respawn.
					if( player_turret && ! player_ship )
					{
						Ship *found_parent = player_turret->ParentShip();
						if( found_parent && (! found_parent->IsMissionObjective) && (found_parent->Group != 255) )
						{
							Player *found_parent_owner = found_parent->Owner();
							if( (found_parent_owner == player) || ! found_parent_owner )
								player_ship = found_parent;
						}
					}
					
					// Allow our old dead ship to be removed after we've switched to gunner.
					if( player_ship && (player_ship->Health < 0.) )
					{
						Player *other_pilot = NULL;
						std::set<Player*> players_in_turrets = player_ship->PlayersInTurrets();
						for( std::set<Player*>::const_iterator turret_player = players_in_turrets.begin(); turret_player != players_in_turrets.end(); turret_player ++ )
						{
							if( *turret_player == player )
								continue;
							std::string other_player_ship = ChosenShip( *turret_player );
							if( (other_player_ship != "Spectator") && (Str::FindInsensitive( other_player_ship, " Gunner" ) < 0) )
							{
								other_pilot = *turret_player;
								break;
							}
						}
						
						if( ! other_pilot )
						{
							// We've abandoned our ship, so don't let it respawn.
							if( (player_ship->Group != 255) && ! player_ship->IsMissionObjective )
								player_ship->CanRespawn = false;
							
							if( RespawnClocks[ player->ID ].ElapsedSeconds() >= GUNNER_WAIT )
							{
								player_ship->PlayerID = 0;
								player_ship->SendUpdate( 126 );
								player_ship = NULL;
							}
						}
						else
						{
							// Give the ship to someone else in it.
							player_ship->PlayerID = other_pilot->ID;
							player_ship->SendUpdate( 126 );
							player_ship = NULL;
						}
					}
					
					if( (! player_ship) && ! player_turret )
					{
						// Gunner is waiting for a turret.
						SetPlayerProperty( player, "team", (player_team == XWing::Team::REBEL) ? "Rebel" : "Empire" );
						uint8_t group = player->PropertyAsInt("group");
						if( ! group )
							group = Data.PropertyAsInt("player_group");
						gunners_waiting[ player_team ][ group ].push_back( player );
					}
				}
				else if( (! player_ship) && ! player_turret )
				{
					// Pilot is waiting for a ship.
					if( chosen_team == "Rebel" )
						player_team = XWing::Team::REBEL;
					else if( chosen_team == "Empire" )
						player_team = XWing::Team::EMPIRE;
					pilots_waiting[ player_team ].push_back( player );
				}
				
				if( player_turret )
				{
					// Players in a turret are not waiting to respawn.
					std::map<uint16_t,Clock>::iterator clock_iter = RespawnClocks.find( player->ID );
					if( clock_iter != RespawnClocks.end() )
						RespawnClocks.erase( clock_iter );
				}
				// If this is the first time we've seen this player without a ship or turret, start their respawn clock.
				else if( RespawnClocks.find( player->ID ) == RespawnClocks.end() )
					RespawnClocks[ player->ID ].Reset();
				else if( Respawn && (desired_ship != "Spectator") && ! Str::ContainsInsensitive( desired_ship, " Gunner" ) )
				{
					// Send time to respawn once per second.
					double respawn_waited = RespawnClocks[ player->ID ].ElapsedSeconds();
					if( (respawn_waited >= 1.75) && ((int)( respawn_waited + dt ) > (int) respawn_waited) )
					{
						if( chosen_team == "Rebel" )
							player_team = XWing::Team::REBEL;
						else if( chosen_team == "Empire" )
							player_team = XWing::Team::EMPIRE;
						
						float respawn_in = std::max<float>( 0.f, RespawnDelay - respawn_waited );
						if( (GameType == XWing::GameType::FLEET_BATTLE)
						&& (  ((player_team == XWing::Team::EMPIRE) && ! empire_flagship)
						   || ((player_team == XWing::Team::REBEL ) && ! rebel_flagship ) ))
							respawn_in = -1.f;
						
						// Wait to start on an even multiple of 5 seconds if it's a long respawn time.
						if( (respawn_in <= 5.f) || (respawn_waited >= 5.) || (( ((int) respawn_in) % 5 ) == 0) )
						{
							Packet time_to_respawn( XWing::Packet::TIME_TO_RESPAWN );
							time_to_respawn.AddFloat( respawn_in );
							Net.SendToPlayer( &time_to_respawn, player->ID );
						}
					}
				}
			}
		}
		
		Data.Lock.Unlock();
		
		
		int ai_optimize = Data.PropertyAsInt("ai_optimize",-1,1);
		if( ai_optimize < 0 )
			ai_optimize = MaxFPS * 0.334;
		uint32_t collision_groups = std::max<int>( 1, ai_optimize );
		CollisionGroup = (CollisionGroup + 1) % collision_groups;
		
		
		// Determine pilot reassignments and control AI ships.
		
		int ai_skill      = Data.PropertyAsInt("ai_skill",1);
		bool ai_pudu      = (ai_skill <  0);
		bool ai_easy      = (ai_skill <= 0);
		bool ai_hard      = (ai_skill >= 2);
		bool ai_jedi      = (ai_skill >= 3);
		bool ai_sith      = (ai_skill >= 4);
		bool ai_flock     = Data.PropertyAsBool("ai_flock");
		bool ai_wasteful  = Data.PropertyAsBool("ai_wasteful");
		bool ai_ceasefire = Data.PropertyAsBool("ai_ceasefire");
		bool rebel_ceasefire = Data.PropertyAsBool("rebel_ceasefire"), empire_ceasefire = Data.PropertyAsBool("empire_ceasefire");
		bool ai_stop      = Data.PropertyAsBool("ai_stop");
		bool ai_blind     = Data.PropertyAsBool("ai_blind");
		
		for( std::map<uint32_t,Ship*>::reverse_iterator ship_iter = ships.rbegin(); ship_iter != ships.rend(); ship_iter ++ )
		{
			Ship *ship = ship_iter->second;
			
			// Find the player whose ship this is.
			Player *player = NULL;
			if( ship->PlayerID )
			{
				player = ship->Owner();
				
				// If a player has left, remove their player ID and flight group from the ship.
				if( ! player )
				{
					ship->PlayerID = 0;
					if( ship->Group != 255 )
					{
						ship->Group = 0;
						ship->CanRespawn = false;
					}
				}
				// If the pilot disconnected but the gunner is still present, give them the ship.
				else if( ship->PlayerID && (ship->PlayerID != player->ID) && ! Data.GetPlayer(ship->PlayerID) )
					ship->PlayerID = player->ID;
			}
			
			// Cease in-system activities from jumped-out ships.
			if( ship->JumpedOut )
			{
				ship->RollRate = ship->PitchRate = ship->YawRate = 0.;
				ship->Firing = false;
				ship->Target = 0;
				ship->TargetLock = 0.f;
				ship->MotionVector = (ship->JumpProgress < 1.) ? (ship->Fwd * ship->MaxSpeed()) : 0.;
				continue;
			}
			
			// See if this ship is available to a player that's waiting for respawn.
			if( PlayersTakeEmptyShips && (! player) && (ship->Health > 0.) && ship->PlayersCanFly() )
			{
				if( ship->Team && pilots_waiting[ ship->Team ].size() )
				{
					player = pilots_waiting[ ship->Team ].front();
					ship->PlayerID = player->ID;
					player_ships[ player->ID ] = ship;
					pilots_waiting[ ship->Team ].pop_front();
					
					// Notify player that they are taking control of this ship.
					ship->SendUpdate( 126 );
				}
			}
			
			
			// If this ship has no player, use AI control.
			
			if( (! player) && (ship->Health > 0.) && (ship->Category() != ShipClass::CATEGORY_TARGET) )
			{
				double pitch = 0.;
				double yaw = 0.;
				double roll = 0.;
				double throttle = (State == XWing::State::ROUND_ENDED) ? 0.9 : (ai_easy ? 0.5 : 0.75);
				
				GameObject *target = NULL;
				
				if( ship->Category() == ShipClass::CATEGORY_CAPITAL )
				{
					// Capital ships shoot asteroids out of their way.
					
					double dist_fwd = 0., dist_up = 0., dist_right = 0.;
					double l = ship->Shape.GetLength(), h = ship->Shape.GetHeight(), w = ship->Shape.GetWidth();
					
					for( std::map<uint32_t,Asteroid*>::iterator asteroid_iter = asteroids.begin(); asteroid_iter != asteroids.end(); asteroid_iter ++ )
					{
						dist_fwd = asteroid_iter->second->DistAlong( &(ship->Fwd), ship );
						dist_up = asteroid_iter->second->DistAlong( &(ship->Up), ship );
						dist_right = asteroid_iter->second->DistAlong( &(ship->Right), ship );
						
						if( (dist_fwd < l) && (dist_fwd > l * -0.5) && (fabs(dist_up) < h * 0.75) && (fabs(dist_right) < w * 0.75) )
						{
							target = asteroid_iter->second;
							break;
						}
					}
				}
				
				size_t num_potential_targets = 0;
				
				if( (! target) && ! ship->JumpedOut )
				{
					// Build a list of all the things this AI ship might want to target.
					std::vector<GameObject*> potential_targets;
					
					if( (ship->WeaponCount(Shot::TYPE_LASER_RED) || ship->WeaponCount(Shot::TYPE_LASER_GREEN))
					&&  (ship->Category() != ShipClass::CATEGORY_TRANSPORT) && (ship->Category() != ShipClass::CATEGORY_CAPITAL) )
					{
						// Add enemy turrets to the list.
						for( std::map<uint32_t,Turret*>::iterator target_iter = turrets.begin(); target_iter != turrets.end(); target_iter ++ )
						{
							if( target_iter->first != ship->ID )
							{
								Turret *potential_target = target_iter->second;
								
								if( potential_target->Health <= 0. )
									continue;
								
								// Ignore invincible turrets, except Ace AI should attack player turrets on YT-1300.
								if( (! potential_target->CanBeHit) && ! (ai_hard && potential_target->PlayerID) )
									continue;
								
								if( ship->Team && (potential_target->Team == ship->Team) )
									continue;
								
								if( potential_target->ParentID )
								{
									// AI ships typically don't attack turrets attached to other ships.
									if( Respawn && ! ai_jedi )
										continue;
									
									// Jedi AI means high difficulty, so don't help players kill enemy ship turrets.
									if( (ship->Team == XWing::Team::REBEL) && rebel_players )
										continue;
									if( (ship->Team == XWing::Team::EMPIRE) && empire_players )
										continue;
									
									// Don't attack ship turrets from below (to prevent accidentally ramming capital ships).
									// FIXME: Should this take parent_ship->Radius() into consideration?
									if( ship->DistAlong( &(potential_target->Up), potential_target ) < -50. )
										continue;
									
									// Don't attack turrets attached to a ship that's still jumping in or has jumped out.
									Ship *parent_ship = potential_target->ParentShip();
									if( parent_ship && ((parent_ship->JumpProgress < 1.) || parent_ship->JumpedOut) )
										continue;
								}
								// Don't attack surface turrets too close to the exhaust port!
								else if( exhaust_port && deathstar && (potential_target->Dist(exhaust_port) < 10000.) && (potential_target->DistAlong( &(deathstar->Up), deathstar ) >= 0.) )
									continue;
								// Don't attack trench turrets from above!
								else if( deathstar && (ship->DistAlong( &(deathstar->Up), deathstar ) > 50.) && (potential_target->DistAlong( &(deathstar->Up), deathstar ) < 0.) )
									continue;
								
								potential_targets.push_back( potential_target );
							}
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
							
							// Don't attack a ship that's still jumping in or has jumped out.
							if( (potential_target->JumpProgress < 1.) || potential_target->JumpedOut )
								continue;
							
							// Don't attack players immediately upon jumping in.
							if( potential_target->PlayerID && (potential_target->Lifetime.ElapsedSeconds() < (3. - ai_skill)) )
								continue;
							
							// Transports only attack when threatened.
							if( (ship->Category() == ShipClass::CATEGORY_TRANSPORT) && (ship->HitByID != potential_target->ID) && (ship->Dist(potential_target) > 1500.) )
								continue;
							
							// Don't attack players at all when AI skill is Bantha Fodder.
							if( ai_pudu && potential_target->Owner() )
								continue;
							
							// Don't attack the exhaust port when out of torpedos.
							if( (GameType == XWing::GameType::BATTLE_OF_YAVIN) && potential_target->IsMissionObjective && ! ship->AmmoForWeapon(Shot::TYPE_TORPEDO) )
								continue;
							
							// Rebel ships should not got chasing distant TIEs on Battle of Yavin.
							if( deathstar && (ship->Team == XWing::Team::REBEL) && (potential_target->DistAlong( &(deathstar->Fwd), ship ) < -3000.) )
								continue;
							
							potential_targets.push_back( potential_target );
						}
					}
					
					// If we're running the trench, filter out any targets outside it or behind us.
					if( deathstar && deathstar->WithinTrenchH(ship) )
					{
						std::vector<GameObject*> trench_targets;
						for( std::vector<GameObject*>::iterator target_iter = potential_targets.begin(); target_iter != potential_targets.end(); target_iter ++ )
						{
							if( ((*target_iter)->DistAlong( &(ship->Fwd), ship ) > 0.) && deathstar->WithinTrenchW(*target_iter) )
								trench_targets.push_back( *target_iter );
						}
						potential_targets = trench_targets;
					}
					
					size_t players_on_team = (ship->Team == XWing::Team::REBEL) ? (rebel_players && ! empire_players) : ((ship->Team == XWing::Team::EMPIRE) ? (empire_players && ! rebel_players) : 0);
					Ship *team_flagship = (ship->Team == XWing::Team::REBEL) ? rebel_flagship : empire_flagship;
					bool able_to_respawn = Respawn;
					if( able_to_respawn && (GameType == XWing::GameType::FLEET_BATTLE) )
						able_to_respawn = team_flagship;
					
					// Bombers and cruisers go after enemy lead ship if their own is dead, to make the end of the battle more interesting.
					if( ((ship->Category() == ShipClass::CATEGORY_BOMBER) || (ship->Category() == ShipClass::CATEGORY_CAPITAL))
					&&  ((! able_to_respawn) || ((ship->Category() == ShipClass::CATEGORY_CAPITAL) && ai_sith && ! players_on_team)) )
					{
						if( empire_flagship && (std::find( potential_targets.begin(), potential_targets.end(), empire_flagship ) != potential_targets.end()) )
						{
							potential_targets.clear();
							potential_targets.push_back( empire_flagship );
						}
						else if( rebel_flagship && (std::find( potential_targets.begin(), potential_targets.end(), rebel_flagship ) != potential_targets.end()) )
						{
							potential_targets.clear();
							potential_targets.push_back( rebel_flagship );
						}
					}
					
					// Bombers should always go after primary targets.
					if( (ship->Category() == ShipClass::CATEGORY_BOMBER) && (GameType != XWing::GameType::BATTLE_OF_YAVIN) )
					{
						std::vector<GameObject*> primary_targets;
						for( std::vector<GameObject*>::iterator target_iter = potential_targets.begin(); target_iter != potential_targets.end(); target_iter ++ )
						{
							if( (*target_iter)->Type() == XWing::Object::SHIP )
							{
								// Bombers always go after primary objective, and go after secondary objectives unless they've lost their lead ship.
								const Ship *potential_ship = (const Ship*) *target_iter;
								if( potential_ship->IsMissionObjective || ((potential_ship->Group == 255) && able_to_respawn) )
									primary_targets.push_back( *target_iter );
								
								// Jedi AI should prioritize players attacking them; Jedi Master AI should always prioritize players.
								else if( ai_jedi && potential_ship->PlayerID )
								{
									if( ai_sith || (ship->HitByID == potential_ship->ID) || (potential_ship->Target == ship->ID) )
										primary_targets.push_back( *target_iter );
								}
							}
						}
						if( primary_targets.size() )
							potential_targets = primary_targets;
					}
					
					// Don't chase the squirrels.
					if( GameType != XWing::GameType::BATTLE_OF_YAVIN )
					{
						std::vector<GameObject*> not_squirrels;
						double ship_center_dist = sqrt( ship->X * ship->X + ship->Y * ship->Y + ship->Z * ship->Z );
						for( std::vector<GameObject*>::iterator target_iter = potential_targets.begin(); target_iter != potential_targets.end(); target_iter ++ )
						{
							double target_center_dist = sqrt( (*target_iter)->X * (*target_iter)->X + (*target_iter)->Y * (*target_iter)->Y + (*target_iter)->Z * (*target_iter)->Z );
							if( (target_center_dist > ship_center_dist) && ((ship_center_dist > 7000.) || (target_center_dist > 8000.)) )
								continue;
							
							not_squirrels.push_back( *target_iter );
						}
						if( not_squirrels.size() )
							potential_targets = not_squirrels;
					}
					
					// Make sure we don't go chasing distant targets when there are nearby ones.
					std::vector<GameObject*> close_targets;
					for( std::vector<GameObject*>::iterator target_iter = potential_targets.begin(); target_iter != potential_targets.end(); target_iter ++ )
					{
						double dist = (*target_iter)->Dist( ship );
						if( dist < 2000. )
							close_targets.push_back( *target_iter );
						
						if( (*target_iter)->Type() == XWing::Object::SHIP )
						{
							Ship *potential_ship = (Ship*) *target_iter;
							if( (dist < 3000.) && (GameType == XWing::GameType::BATTLE_OF_YAVIN) && potential_ship->IsMissionObjective )
							{
								// If the exhaust port is close, go for that.
								close_targets.clear();
								close_targets.push_back( potential_ship );
								break;
							}
						}
					}
					if( close_targets.size() )
						potential_targets = close_targets;
					
					// Rookie AI should prioritize attacking AI ships.
					if( ai_easy )
					{
						std::vector<GameObject*> ai_targets;
						for( std::vector<GameObject*>::iterator target_iter = potential_targets.begin(); target_iter != potential_targets.end(); target_iter ++ )
						{
							if( ! (*target_iter)->Owner() )
								ai_targets.push_back( *target_iter );
						}
						if( ai_targets.size() )
							potential_targets = ai_targets;
					}
					// Jedi Master AI should prioritize attacking players.
					else if( ai_sith )
					{
						std::vector<GameObject*> player_targets;
						for( std::vector<GameObject*>::iterator target_iter = potential_targets.begin(); target_iter != potential_targets.end(); target_iter ++ )
						{
							if( (*target_iter)->Owner() )
								player_targets.push_back( *target_iter );
						}
						if( player_targets.size() )
							potential_targets = player_targets;
					}
					// Ace AI fighters and gunboats with a flagship should prioritize defending it from attacking players.
					else if( ai_hard && ((ship->Category() == ShipClass::CATEGORY_FIGHTER) || (ship->Category() == ShipClass::CATEGORY_GUNBOAT))
					&&       team_flagship && team_flagship->HitByID && (team_flagship->HitClock.ElapsedSeconds() < ai_skill) )
					{
						GameObject *attacker = Data.GetObject( team_flagship->HitByID );
						if( attacker && attacker->PlayerID && (std::find( potential_targets.begin(), potential_targets.end(), attacker ) != potential_targets.end()) )
						{
							potential_targets.clear();
							potential_targets.push_back( attacker );
						}
					}
					
					// Jedi AI should prioritize players attacking them.
					if( ai_jedi )
					{
						std::vector<GameObject*> attackers;
						for( std::vector<GameObject*>::iterator target_iter = potential_targets.begin(); target_iter != potential_targets.end(); target_iter ++ )
						{
							if( ! (*target_iter)->PlayerID )
								continue;
							if( ship->HitByID == (*target_iter)->ID )
								attackers.push_back( *target_iter );
							else if( (*target_iter)->Type() == XWing::Object::SHIP )
							{
								const Ship *potential_ship = (const Ship*) *target_iter;
								if( potential_ship->Target == ship->ID )
									attackers.push_back( *target_iter );
							}
						}
						if( attackers.size() )
							potential_targets = attackers;
					}
					
					// Pick a target.
					num_potential_targets = potential_targets.size();
					if( num_potential_targets )
					{
						double adjusted_lifetime = ship->Lifetime.ElapsedSeconds() + ship->ID * 0.07;
						int num = ship->ID + ship->Lifetime.TimeVal.tv_sec + (int)( adjusted_lifetime * ((ship->Team == XWing::Team::EMPIRE) ? 0.005 : 0.007) );
						if( ai_flock )
							num = adjusted_lifetime / (5. + ((size_t)(ship->Class) / 8) % 3);
						else if( ship->HitByID )
						{
							GameObject *hit_by = Data.GetObject( ship->HitByID );
							if( hit_by && (ai_hard || ! hit_by->PlayerID) )
								num += ship->HitByID;
						}
						target = potential_targets.at( num % num_potential_targets );
					}
				}
				
				// Don't switch targets if we are about to lock on or recently locked on, unless we just took a hit.
				if( (ship->TargetLock > 0.5f) && (ship->TargetLock < 1.5f) && (ship->HitClock.ElapsedSeconds() > 1.) && ! ai_easy )
				{
					GameObject *prev_target = Data.GetObject( ship->Target );
					if( prev_target && (prev_target->Type() == XWing::Object::SHIP) )
					{
						const Ship *prev_target_ship = (const Ship*) prev_target;
						if( (prev_target_ship->Health > 0.) && (prev_target->DistAlong( &(ship->Fwd), ship ) > 0.) )
							target = prev_target;
					}
				}
				
				// These are scoped out here so the waypoint and dodging sections can see them.
				bool facing_target = false;
				bool go_easy = ai_easy;
				const GameObject *dodging = NULL;
				bool dodging_for_shot = false;
				uint8_t target_subsystem = 0;
				Player *target_owner = NULL;
				size_t players_on_team = (ship->Team == XWing::Team::REBEL) ? (rebel_players && ! empire_players) : ((ship->Team == XWing::Team::EMPIRE) ? (empire_players && ! rebel_players) : 0);
				size_t enemy_players = (ship->Team == XWing::Team::REBEL) ? (empire_players && ! rebel_players) : ((ship->Team == XWing::Team::EMPIRE) ? (rebel_players && ! empire_players) : (rebel_players + empire_players));
				
				if( target )
				{
					target_owner = target->Owner();
					Ship *target_ship = (target->Type() == XWing::Object::SHIP) ? (Ship*) target : NULL;
					if( target_owner )
						go_easy = ! ai_hard;
					Vec3D vec_to_target( target->X - ship->X, target->Y - ship->Y, target->Z - ship->Z );
					
					if( target_ship && target_ship->Subsystems.size()
					&&  ( (! target_ship->IsMissionObjective)
					   || (! Respawn)
					   || (ai_jedi && ! players_on_team)
					   || (fmod( ship->DeathClock.ElapsedSeconds(), 15. ) < (players_on_team ? (9. - ai_skill * 2.) : (3. + ai_skill * 3.))) ))
					{
						// Look for shield generators and attack those first.
						uint8_t subsystem_num = 1;
						for( std::map<std::string,double>::const_iterator subsystem_iter = target_ship->Subsystems.begin(); subsystem_iter != target_ship->Subsystems.end(); subsystem_iter ++ )
						{
							if( (subsystem_iter->second > 0.) && (strncmp( subsystem_iter->first.c_str(), "ShieldGen", strlen("ShieldGen") ) == 0) )
							{
								std::map<std::string,ModelObject*>::iterator object_iter = target_ship->Shape.Objects.find( subsystem_iter->first );
								if( object_iter != target_ship->Shape.Objects.end() )
								{
									Pos3D offset = object_iter->second->GetCenterPoint();
									vec_to_target += target_ship->Fwd   * offset.X;
									vec_to_target += target_ship->Up    * offset.Y;
									vec_to_target += target_ship->Right * offset.Z;
									target_subsystem = subsystem_num;
									break;
								}
							}
							subsystem_num ++;
						}
						
						// After the shield generators are gone, Jedi AI with no players on their team attack critical subsystems (Star Destroyer bridge).
						if( (! target_subsystem) && ((ai_jedi && ! players_on_team) || ! target_ship->IsMissionObjective) )
						{
							subsystem_num = 1;
							for( std::map<std::string,double>::const_iterator subsystem_iter = target_ship->Subsystems.begin(); subsystem_iter != target_ship->Subsystems.end(); subsystem_iter ++ )
							{
								if( (subsystem_iter->second > 0.) && (strncmp( subsystem_iter->first.c_str(), "Critical", strlen("Critical") ) == 0) )
								{
									std::map<std::string,ModelObject*>::iterator object_iter = target_ship->Shape.Objects.find( subsystem_iter->first );
									if( object_iter != target_ship->Shape.Objects.end() )
									{
										Pos3D offset = object_iter->second->GetCenterPoint();
										vec_to_target += target_ship->Fwd   * offset.X;
										vec_to_target += target_ship->Up    * offset.Y;
										vec_to_target += target_ship->Right * offset.Z;
										target_subsystem = subsystem_num;
										break;
									}
								}
								subsystem_num ++;
							}
						}
					}
					
					// If a ship's origin is in empty space (ex: Star Destroyer) the model object Hull defines the point to aim at.
					if( target_ship && (! target_subsystem) && (target_ship->Category() == ShipClass::CATEGORY_CAPITAL) && target_ship->ComplexCollisionDetection() )
					{
						std::map<std::string,ModelObject*>::const_iterator object_iter = target_ship->Shape.Objects.find("Hull");
						if( (object_iter != target_ship->Shape.Objects.end()) && object_iter->second->Points.size() )
						{
							Vec3D point = object_iter->second->Points.front();
							vec_to_target += target_ship->Fwd   * point.X;
							vec_to_target += target_ship->Up    * point.Y;
							vec_to_target += target_ship->Right * point.Z;
						}
					}
					
					uint8_t heavy_weapon = 0;
					if( ship->WeaponCount(Shot::TYPE_TORPEDO) )
						heavy_weapon = Shot::TYPE_TORPEDO;
					else if( ship->WeaponCount(Shot::TYPE_MISSILE) )
						heavy_weapon = Shot::TYPE_MISSILE;
					
					// Make AI ships select torpedos/missiles when attacking appropriate targets.
					bool use_heavy_weapon = false;
					if( heavy_weapon )
					{
						use_heavy_weapon = target_ship && (target_ship->IsMissionObjective || (target_ship->Group == 255) || (ship->Category() == ShipClass::CATEGORY_GUNBOAT) || ai_wasteful);
						bool limit_fire_rate = target_ship && (target_ship->Category() != ShipClass::CATEGORY_TARGET) && (target_ship->Category() != ShipClass::CATEGORY_CAPITAL);
						if( (! use_heavy_weapon) && (GameType != XWing::GameType::BATTLE_OF_YAVIN) && target_ship && (ai_hard || ! target_owner) && ((ship->TargetLock > 0.25f) || (ship->Dist(target_ship) > 1500.) || (ship->DistAlong( &(target_ship->Fwd), target_ship ) < -300.)) )
						{
							use_heavy_weapon = true;
							limit_fire_rate = ! (target_owner && ai_sith);
						}
						if( limit_fire_rate && ! ai_wasteful )
						{
							// Do not fire the heavy weapon again if we still have one going after this target.
							int available = ((ship->Category() == ShipClass::CATEGORY_GUNBOAT) && (ai_hard || ! target_owner)) ? 2 : 1;
							for( std::list<Shot*>::const_iterator shot_iter = shots.begin(); shot_iter != shots.end(); shot_iter ++ )
							{
								if( ((*shot_iter)->FiredFrom == ship->ID) && ((*shot_iter)->ShotType == heavy_weapon) && ((*shot_iter)->Seeking == target->ID) )
								{
									available --;
									if( ! available )
									{
										use_heavy_weapon = false;
										break;
									}
								}
							}
						}
						bool target_is_exhaust_port = (GameType == XWing::GameType::BATTLE_OF_YAVIN) && target_ship && target_ship->IsMissionObjective;
						if( use_heavy_weapon && (ship->AmmoForWeapon(heavy_weapon) || target_is_exhaust_port) && ! target_subsystem )
						{
							while( ship->SelectedWeapon != heavy_weapon )
								ship->NextWeapon();
							int desired_mode = target_is_exhaust_port ? ship->WeaponCount(heavy_weapon) : 1;
							if( ship->CurrentFiringMode() != desired_mode )
								ship->NextFiringMode();
						}
						else if( ship->SelectedWeapon == heavy_weapon )
							ship->NextWeapon();
					}
					
					// Determine when ion cannon use is appropriate.
					if( ship->WeaponCount(Shot::TYPE_ION_CANNON) && ! use_heavy_weapon )
					{
						if( (ship->SelectedWeapon == Shot::TYPE_ION_CANNON) && ship->Class && (ship->Class->Ammo.size() >= (heavy_weapon ? 3 : 2)) )
						{
							// We're using ion cannons but also have lasers; check if we should use those instead.
							if( (! target_ship)
							||  (target_ship->Category() != ShipClass::CATEGORY_CAPITAL)
							||  (target_ship->ShieldF < 30.)
							||  (target_ship->ShieldR < 30.)
							||  target_subsystem )
							{
								ship->NextWeapon();
								if( ship->SelectedWeapon == heavy_weapon )
									ship->NextWeapon();
							}
						}
						else if( target_ship && (target_ship->Category() == ShipClass::CATEGORY_CAPITAL)
						&&       (target_ship->ShieldF > 30.) && (target_ship->ShieldR > 30.)
						&&       ! target_subsystem )
						{
							while( ship->SelectedWeapon != Shot::TYPE_ION_CANNON )
								ship->NextWeapon();
						}
					}
					
					double dist_to_target = vec_to_target.Length();
					Vec3D shot_vec = ship->Fwd;
					shot_vec.ScaleTo( 800. );  // Shot::Speed
					shot_vec -= target->MotionVector;
					double relative_speed = shot_vec.Length();
					double time_to_target = relative_speed ? (dist_to_target / relative_speed) : 0.;
					Vec3D vec_to_intercept = vec_to_target;
					vec_to_intercept.X += target->MotionVector.X * time_to_target;
					vec_to_intercept.Y += target->MotionVector.Y * time_to_target;
					vec_to_intercept.Z += target->MotionVector.Z * time_to_target;
					double dist_to_intercept = vec_to_intercept.Length();
					vec_to_target.ScaleTo( 1. );
					vec_to_intercept.ScaleTo( 1. );
					double t_dot_fwd = vec_to_target.Dot( &(ship->Fwd) );
					double i_dot_fwd = vec_to_intercept.Dot( &(ship->Fwd) );
					double i_dot_up = vec_to_intercept.Dot( &(ship->Up) );
					double i_dot_right = vec_to_intercept.Dot( &(ship->Right) );
					
					double dodge_dist = 100. + ship->Radius();
					if( target_ship )
					{
						dodge_dist += target_ship->Radius();
						
						// Capital ships do not dodge fighters/bombers ahead.
						if( (ship->Category() == ShipClass::CATEGORY_CAPITAL) && (target_ship->Category() != ShipClass::CATEGORY_CAPITAL) )
							dodge_dist = 0.;
					}
					double run_dist = (ship->Category() == ShipClass::CATEGORY_CAPITAL) ? 0. : (dodge_dist * 4.);
					double firing_dist = ((target->Type() == XWing::Object::TURRET) && ! target->MotionVector.Length()) ? 2000. : (900. + dodge_dist);
					ship->Firing = (i_dot_fwd > 0.9) && (dist_to_intercept < firing_dist);
					
					// Ships using only turrets fire regardless of heading.
					if( ! ship->SelectedWeapon )
						ship->Firing = (dist_to_target < 1500.);
					
					// Ships using lasers use linked fire mode when engaging far-away targets.
					else if( ((ship->SelectedWeapon == Shot::TYPE_LASER_RED) || (ship->SelectedWeapon == Shot::TYPE_LASER_GREEN)) && (ship->WeaponCount( ship->SelectedWeapon ) >= 2) )
					{
						if( (dist_to_target > (go_easy ? 400. : 700.) + dodge_dist) && (ship->CurrentFiringMode() == 1) )
							ship->NextFiringMode();
						else if( (dist_to_target < (go_easy ? 150. : 500.) + dodge_dist) && (ship->CurrentFiringMode() > 1) )
							ship->NextFiringMode();
					}
					
					// Don't shoot at dead things.
					if( target_ship && (target_ship->Health <= 0.) )
						ship->Firing = false;
					else if( (target->Type() == XWing::Object::TURRET) && (((Turret*)( target ))->Health <= 0.) )
						ship->Firing = false;
					
					// Dodge any friendly fighters blocking our shot.
					if( ship->Firing && ship->Team && (ship->Category() != ShipClass::CATEGORY_CAPITAL) )
					{
						std::vector<Shot*> shots_to_fire = ship->AllShots();
						if( shots_to_fire.empty() )
						{
							Shot *shot = new Shot();
							shots_to_fire.push_back( shot );
							shot->Copy( ship );
							shot->FiredFrom = ship->ID;
							shot->PlayerID = ship->PlayerID;
							shot->ShotType = ship->SelectedWeapon ? ship->SelectedWeapon : (uint8_t) ((ship->Team == XWing::Team::EMPIRE) ? Shot::TYPE_TURBO_LASER_GREEN : Shot::TYPE_TURBO_LASER_RED);
							shot->Fwd = vec_to_intercept;
							shot->FixVectors();
							shot->MotionVector = vec_to_intercept * shot->Speed();
						}
						
						for( std::map<uint32_t,Ship*>::const_iterator ship_iter2 = ships.begin(); ship_iter2 != ships.end(); ship_iter2 ++ )
						{
							if( ship_iter2->second == ship )
								continue;
							if( ship_iter2->second->Category() == ShipClass::CATEGORY_CAPITAL )
								continue;
							if( ship_iter2->second->Team == ship->Team )
							{
								for( std::vector<Shot*>::const_iterator shot_iter = shots_to_fire.begin(); shot_iter != shots_to_fire.end(); shot_iter ++ )
								{
									if( (*shot_iter)->WillCollide( ship_iter2->second, time_to_target ) && ((! dodging) || (ship->Dist(ship_iter2->second) < ship->Dist(dodging))) )
									{
										ship->Firing = false;
										if( ! dodging )
											dodging_for_shot = true;
										dodging = ship_iter2->second;
										
										if( dodging_for_shot )
										{
											// See if we should also slow down to avoid crashing into the friendly.
											if( ship_iter2->second->Category() == ShipClass::CATEGORY_CAPITAL )
											{
												if( ship->WillCollide( ship_iter2->second, 2.5 ) )
													dodging_for_shot = false;
											}
											else
											{
												Vec3D vec_to_ship2 = *(ship_iter2->second) - *ship;
												if( fabs(vec_to_ship2.Unit().Dot(&(ship->Fwd))) < 0.25 ) // Ship off to the side; slow down when dodging.
													dodging_for_shot = false;
											}
										}
										
										break;  // We know a shot would hit this ship, so no need to keep checking the other shots.
									}
								}
							}
						}
						
						for( std::vector<Shot*>::const_iterator shot_iter = shots_to_fire.begin(); shot_iter != shots_to_fire.end(); shot_iter ++ )
							delete *shot_iter;
					}
					
					if( (t_dot_fwd >= 0.) && (dist_to_intercept < dodge_dist) )
					{
						// Damn close ahead: Avoid collision!
						pitch = (i_dot_up >= 0.) ? -1. : 1.;
						yaw = (i_dot_right >= 0.) ? -1. : 1.;
						roll = (i_dot_right >= 0.) ? -1. : 1.;
						throttle = 0.75;
						
						if( target->Type() == XWing::Object::TURRET )
						{
							Turret *target_turret = (Turret*) target;
							Ship *parent_ship = target_turret->ParentShip();
							if( parent_ship )
							{
								// Avoid hitting the ship our target turret is attached to.
								dodging = parent_ship;
								dodging_for_shot = false;
							}
						}
						
						// Ships that pitch faster at high speed (such as the Millenium Falcon) should maintain high throttle while dodging.
						if( ship->Class && (ship->Class->PitchFast > ship->Class->PitchSlow) && (ship->Category() != ShipClass::CATEGORY_CAPITAL) )
							throttle = 1.;
					}
					else if( target->Type() == XWing::Object::ASTEROID )
					{
						// Other than immediate avoidance, don't change course for asteroids.
						pitch = 0.;
						yaw = 0.;
						roll = 0.;
						throttle = 1.;
					}
					else if( t_dot_fwd >= 0.25 )
					{
						// Enemy generally ahead of us: Aim at them.
						pitch = Num::SignedPow( i_dot_up, go_easy ? 0.9 : 0.75 ) * 5.;
						yaw = Num::SignedPow( i_dot_right, go_easy ? 0.9 : 0.75 ) * 3.;
						roll = Num::SignedPow( i_dot_right, go_easy ? 0.8 : 0.66 ) * 7.;
						throttle = 1.;
						if( ship->Category() == ShipClass::CATEGORY_CAPITAL )
							;
						else if( (dist_to_target < 250.) && (i_dot_fwd > 0.5) && (! go_easy) && (ship->HitClock.ElapsedSeconds() > 5.) )
							throttle = dist_to_target / 250.;
						else if( ai_easy && (dist_to_target < 500.) && target_ship && (! target_ship->IsMissionObjective) && (target_ship->Group != 255) && ! target_owner )
							throttle = dist_to_target / 500.;
						facing_target = (t_dot_fwd > 0.95);
					}
					else if( (dist_to_target < run_dist) && (ship->HitClock.ElapsedSeconds() > ai_skill) )
					{
						// Enemy behind us but too close to turn towards: Get some distance.
						pitch = 0.;
						yaw = i_dot_right / 4.;
						roll = i_dot_right / 2.;
						throttle = 1.;
					}
					else
					{
						// Enemy behind us and far away: Turn towards them.
						pitch = (i_dot_up >= 0.) ? 1. : -1.;
						if( go_easy && (ship->Category() != ShipClass::CATEGORY_CAPITAL) )
							pitch *= 0.75;
						yaw = i_dot_right;
						roll = i_dot_right;
						throttle = 1. - i_dot_up/2.;
					}
				}
				else
				{
					// No target.
					ship->Firing = false;
				}
				
				// Try to avoid crashing into things.
				if( ! ai_blind )
				{
					bool small_ship = (ship->Category() != ShipClass::CATEGORY_CAPITAL);
					
					// See if we're about to run into something.
					//Pos3D hit_loc = CollisionAt[ ship->ID ];
					const GameObject *would_hit = Data.GetObject( CollisionWith[ ship->ID ] );
					if( (ship->ID % collision_groups) == CollisionGroup )
					{
						if( small_ship )
						{
							double pitch_rate = ship->Class ? std::min<double>( ship->Class->PitchSlow, ship->Class->PitchFast ) : ship->MaxPitch();
							would_hit = ship->FindCollision( pitch_rate ? (180. / pitch_rate) : 3. /* , &hit_loc */ );
						}
						else
						{
							would_hit = NULL;
							for( std::map<uint32_t,Ship*>::const_iterator ship_iter2 = ships.begin(); ship_iter2 != ships.end(); ship_iter2 ++ )
							{
								const Ship *other_ship = ship_iter2->second;
								if( ship == other_ship )
									continue;
								if( other_ship->Category() != ShipClass::CATEGORY_CAPITAL )
									continue;
								if( (other_ship->Health <= 0.) && (other_ship->DeathClock.ElapsedSeconds() >= other_ship->PiecesDangerousTime()) )
									continue;
								if( (other_ship->JumpProgress < 1.) || other_ship->JumpedOut )
									continue;
								if( ship->WillCollide( other_ship, 5. ) )
								{
									would_hit = other_ship;
									break;  // FIXME: Check for nearest?
								}
							}
						}
						CollisionWith[ ship->ID ] = would_hit ? would_hit->ID : 0;
						//CollisionAt[ ship->ID ] = hit_loc;
					}
					if( would_hit && (would_hit != target) )
					{
						if( would_hit->Type() == XWing::Object::SHOT )
						{
							// Capital ships never try to dodge shots.
							Shot *shot = (Shot*) would_hit;
							if( ! small_ship )
								;
							// Mission objectives should always try to dodge non-player shots.
							else if( (ship->IsMissionObjective || ai_jedi) && ((! shot->PlayerID) || ! ai_easy) )
								dodging = would_hit;
							else if( shot->Lifetime.ElapsedSeconds() > ((ship->HitClock.ElapsedSeconds() < (go_easy ? 2. : 3.)) ? 0.5 : 2.) )
							{
								// Dodge shots ahead (after some reaction time) to make head-to-head combat more interesting.
								Vec3D vec_to_obstacle = *shot - *ship;
								vec_to_obstacle.ScaleTo( 1. );
								if( vec_to_obstacle.Dot( &(ship->Fwd) ) > 0.25 )
								{
									dodging = would_hit;
									dodging_for_shot = false;
								}
							}
						}
						else if( would_hit->Type() == XWing::Object::SHIP )
						{
							const Ship *other_ship = (const Ship*) would_hit;
							if( ai_hard || (! target_owner) || other_ship->PlayerID || other_ship->IsMissionObjective || (other_ship->Group == 255) )
							{
								// AI always dodges player and objective ships.  Ace AI and any AI not chasing a player dodge all other ships too.
								dodging = would_hit;
								dodging_for_shot = false;
							}
						}
						else
						{
							// If we're about to hit something that's not a shot or ship, always try to dodge it.
							dodging = would_hit;
							dodging_for_shot = false;
						}
					}
					
					if( ! dodging )
					{
						// Check if we are uncomfortably close to another ship or an asteroid.
						double min_dist = 10.;
						
						for( std::map<uint32_t,Ship*>::const_iterator ship_iter2 = ships.begin(); ship_iter2 != ships.end(); ship_iter2 ++ )
						{
							const Ship *other_ship = ship_iter2->second;
							if( ship == other_ship )
								continue;
							if( target == other_ship )
								continue;
							if( small_ship && (! ai_hard) && target_owner && ! other_ship->PlayerID )  // Non-Ace ships chasing a player only avoid hitting players.
								continue;
							if( ! other_ship->CanCollideWithOwnType() )  // Don't dodge jumped-out ships, exhaust port, etc.
								continue;
							if( (other_ship->Health <= 0.) && (other_ship->DeathClock.ElapsedSeconds() > other_ship->PiecesDangerousTime()) )
								continue;
							if( (! small_ship) && (other_ship->Category() != ShipClass::CATEGORY_CAPITAL) )  // Capital ships do not avoid fighters at all.
								continue;
							
							double dist_to_other = ship->Dist(other_ship);
							
							if( small_ship && (other_ship->Category() == ShipClass::CATEGORY_CAPITAL) )
#define GLITCHY_DODGE 1
#ifndef GLITCHY_DODGE
								continue;
#else
							{
#if GLITCHY_DODGE < 2
								if( ! ship->IsMissionObjective )
									continue;
#endif
								if( (dist_to_other > other_ship->Shape.Triagonal()) || ai_blind )
									continue;
								
								double length = other_ship->Shape.Length;
								double width  = other_ship->Shape.Width;
								Pos3D pos( other_ship );
								
								std::map<std::string,ModelObject*>::const_iterator object_iter = other_ship->Shape.Objects.find("Hull");
								if( (object_iter != other_ship->Shape.Objects.end()) && (object_iter->second->Points.size()) )
								{
									Vec3D point = object_iter->second->Points.front();
									pos += other_ship->Fwd   * point.X;
									pos += other_ship->Up    * point.Y;
									pos += other_ship->Right * point.Z;
								}
								
								Pos3D bow       = pos + other_ship->Fwd * length * 0.5;
								Pos3D port      = pos - other_ship->Fwd * length * 0.5 - other_ship->Right * width * 0.5;
								Pos3D starboard = pos - other_ship->Fwd * length * 0.5 + other_ship->Right * width * 0.5;
								Vec3D to_pos = pos - *ship, to_bow = bow - *ship, to_port = port - *ship, to_starboard = starboard - *ship;
								if( (ship->Fwd.Dot(&to_pos) < 0.) && (ship->Fwd.Dot(&to_bow) < 0.) && (ship->Fwd.Dot(&to_port) < 0.) && (ship->Fwd.Dot(&to_starboard) < 0.) )
									continue;
								
								double dist_to_pos = to_pos.Length(), dist_to_bow = to_bow.Length(), dist_to_port = to_port.Length(), dist_to_starboard = to_starboard.Length();
								if( dist_to_other > dist_to_pos )
									dist_to_other = dist_to_pos;
								if( dist_to_other > dist_to_bow )
									dist_to_other = dist_to_bow;
								if( dist_to_other > dist_to_port )
									dist_to_other = dist_to_port;
								if( dist_to_other > dist_to_starboard )
									dist_to_other = dist_to_starboard;
							}
#endif
							double dist = dist_to_other - ship->Radius() - other_ship->Radius() * 1.05;
							if( dist < min_dist )
							{
								min_dist = dist;
								dodging = other_ship;
								dodging_for_shot = false;
							}
						}
						
						if( ai_hard || (! target_owner) || ! small_ship )
						{
							for( std::map<uint32_t,Asteroid*>::const_iterator asteroid_iter = asteroids.begin(); asteroid_iter != asteroids.end(); asteroid_iter ++ )
							{
								const Asteroid *asteroid = asteroid_iter->second;
								if( ! asteroid->CanCollideWithOtherTypes() )
									continue;
								double dist = ship->Dist(asteroid) - ship->Radius() - asteroid->Radius * 1.1;
								if( dist < min_dist )
								{
									min_dist = dist;
									dodging = asteroid;
									dodging_for_shot = false;
								}
							}
						}
					}
					
					if( small_ship && ! dodging && (ship->IsMissionObjective || ! go_easy) )
					{
						// Dodge the nearest incoming non-player torpedo/missile tracking us when it gets close.
						for( std::list<Shot*>::const_iterator shot_iter = shots.begin(); shot_iter != shots.end(); shot_iter ++ )
						{
							if( (*shot_iter)->PlayerID && (! ai_hard) && (ai_easy || ! ship->IsMissionObjective) )
								continue;
							if( ((*shot_iter)->Seeking == ship->ID) && ((! dodging) || (ship->Dist(*shot_iter) < ship->Dist(dodging))) )
							{
								Vec3D incoming_vec = *ship - *shot_iter;
								double yaw_time = 90. / (ship->Class ? std::min<double>( ship->Class->YawSlow, ship->Class->YawFast ) : ship->MaxYaw());
								if( ((*shot_iter)->Fwd.Dot( &incoming_vec ) > 0.) && ((incoming_vec.Length() / ((*shot_iter)->Speed() + ship->MaxSpeed())) <= yaw_time) )
								{
									dodging = *shot_iter;
									dodging_for_shot = false;
								}
							}
						}
					}
				}
				
				// If we have something to dodge, take evasive action.
				uint8_t angle_deflector = Ship::SHIELD_CENTER;
				if( dodging )
				{
					Vec3D vec_to_obstacle = *dodging - *ship;
					if( dodging->Type() == XWing::Object::DEATH_STAR )
						vec_to_obstacle = dodging->Up * -1.;
					else if( dodging->ComplexCollisionDetection() && (dodging->Type() == XWing::Object::SHIP) )
					{
						const Ship *dodging_ship = (const Ship*) dodging;
						if( dodging_ship->Category() == ShipClass::CATEGORY_CAPITAL )
						{
							Pos3D dodging_pos = *dodging;
							// If a ship's origin is in empty space (ex: Star Destroyer) the model object Hull defines the point to aim at.
							std::map<std::string,ModelObject*>::const_iterator object_iter = dodging_ship->Shape.Objects.find("Hull");
							if( (object_iter != dodging_ship->Shape.Objects.end()) && object_iter->second->Points.size() )
							{
								Vec3D point = object_iter->second->Points.front();
								dodging_pos += dodging->Up * point.Y;  // Just adjust the vertical position.
							}
							Pos3D bow       = dodging_pos + dodging->Fwd * dodging_ship->Shape.Length * 0.5;
							Pos3D stern     = dodging_pos + dodging->Fwd * dodging_ship->Shape.Length * -0.6;
							Pos3D fuselage  = Math3D::NearestPointOnLine( ship, &bow, &stern );
							Pos3D starboard = fuselage + dodging->Right * dodging_ship->Shape.Width * 0.4;
							Pos3D port      = fuselage + dodging->Right * dodging_ship->Shape.Width * -0.4;
							vec_to_obstacle = Math3D::NearestPointOnLine( ship, &starboard, &port ) - *ship;
						}
					}
					double dist_to_obstacle = vec_to_obstacle.Length();
					Vec3D relative_motion = ship->MotionVector - dodging->MotionVector;
					double relative_motion_length = relative_motion.Length();
					Vec3D vec_to_intercept = vec_to_obstacle;
					if( relative_motion_length && (relative_motion.Dot( &vec_to_obstacle ) >= 0.) )
					{
						double time_to_obstacle = dist_to_obstacle / relative_motion_length;
						vec_to_intercept += dodging->MotionVector * time_to_obstacle;
					}
					vec_to_obstacle.ScaleTo( 1. );
					vec_to_intercept.ScaleTo( 1. );
					Vec3D obstacle_motion = dodging->MotionVector;
					if( obstacle_motion.Length() )
						obstacle_motion.ScaleTo( 1. );
					double t_dot_fwd   = vec_to_obstacle.Dot(  &(ship->Fwd)   );
					double i_dot_fwd   = vec_to_intercept.Dot( &(ship->Fwd)   );
					double i_dot_up    = vec_to_intercept.Dot( &(ship->Up)    );
					double i_dot_right = vec_to_intercept.Dot( &(ship->Right) );
					double m_dot_fwd   = obstacle_motion.Dot(  &(ship->Fwd)   );
					double m_dot_right = obstacle_motion.Dot(  &(ship->Right) );
					
					// Keep object to dodge straight above or below (depending on intercept point above/below) with motion sideways.
					pitch = Num::SignedPow( i_dot_fwd, 0.5 ) * ((i_dot_up >= 0.) ? -15. : 15.);
					roll = Num::SignedPow( i_dot_right, 0.5 ) * ((i_dot_up >= 0.) ? 10. : -10.);
					if( (dodging->Type() == XWing::Object::SHIP) && (((const Ship*)( dodging ))->Category() == ShipClass::CATEGORY_CAPITAL) )
						yaw = pow( fabs(i_dot_fwd), 0.5 ) * ((i_dot_right >= 0.) ? -20. : 20.);
					else if( obstacle_motion.Length() )
						yaw = Num::SignedPow( m_dot_fwd, 0.5 ) * ((m_dot_right >= 0.) ? -20. : 20.);
					throttle = ((t_dot_fwd < 0.) || (dodging->Type() == XWing::Object::SHOT) || dodging_for_shot) ? 1. : (1. - t_dot_fwd * 0.5);
					
					// Ships that pitch faster at high speed (such as the Millenium Falcon) should maintain high throttle while dodging.
					if( ship->Class && (ship->Class->PitchFast > ship->Class->PitchSlow) && (ship->Category() != ShipClass::CATEGORY_CAPITAL) )
						throttle = 1.;
					
					// Jedi AI angle deflector shields towards incoming shots.
					if( dodging && (dodging->Type() == XWing::Object::SHOT) && ((players_on_team && ! enemy_players) ? ai_easy : ai_jedi) )
						angle_deflector = (m_dot_fwd > 0.) ? Ship::SHIELD_REAR : Ship::SHIELD_FRONT;
				}
				
				// Angle deflector shields when needed, otherwise reset back to center.
				if( (angle_deflector != Ship::SHIELD_CENTER) || (ship->HitClock.ElapsedSeconds() > 0.5) )
					ship->ShieldPos = angle_deflector;
				
				// Set target ID and update missile lock progress.
				ship->UpdateTarget( target, target_subsystem, dt );
				
				// Don't fire torpedos before we have a lock.
				if( ship->Firing && ship->SelectedWeapon && (ship->AmmoForWeapon(ship->SelectedWeapon) > 0) && (ship->TargetLock < (1.f + std::max<float>( 0.f, (target->PlayerID ? 0.6f : 0.3f) - ai_skill * 0.1f ))) )
					ship->Firing = false;
				
				// See if we should be chasing waypoints.
				Pos3D *waypoint = NULL;
				std::vector<Pos3D> *waypoint_list = NULL;
				if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
				{
					if( Waypoints[ 0 ].size() )
					{
						// See if we have a "trench_runners" server property.
						std::string trench_runners = Data.PropertyAsString("trench_runners");
						if( trench_runners.empty() || (Str::FindInsensitive( trench_runners, "default" ) >= 0) )
						{
							// The default behavior is to select trench runners based on count-down; Rebel bombers always run the trench.
							if( (ship->Category() == ShipClass::CATEGORY_CAPITAL) || (ship->Radius() > (deathstar->TrenchWidth * 0.25)) )
								;
							else if( ai_sith && (ship->Team == XWing::Team::EMPIRE) )
								waypoint_list = rebel_players_in_trench ? &(Waypoints[ 0 ]) : NULL;
							else if( (ship->Team == XWing::Team::REBEL) && (! ship->AmmoForWeapon(Shot::TYPE_TORPEDO)) && deathstar && (ship->DistAlong( &(deathstar->Up), deathstar ) > ship->Radius()) )
								waypoint_list = NULL;
							else if( (ship->Category() == ShipClass::CATEGORY_BOMBER) && (ship->Team == XWing::Team::REBEL) )
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
							if( (Str::FindInsensitive( trench_runners, "rebel" ) >= 0) && (ship->Team == XWing::Team::REBEL) )
								waypoint_list = &(Waypoints[ 0 ]);
							if( (Str::FindInsensitive( trench_runners, "empire" ) >= 0) && (ship->Team == XWing::Team::EMPIRE) )
								waypoint_list = &(Waypoints[ 0 ]);
							if( ship->Class && (Str::FindInsensitive( trench_runners, ship->Class->ShortName ) >= 0) )
								waypoint_list = &(Waypoints[ 0 ]);
							if( Str::FindInsensitive( trench_runners, "all" ) >= 0 )
								waypoint_list = &(Waypoints[ 0 ]);
						}
						
						if( waypoint_list && (ship->Team == XWing::Team::REBEL) )
						{
							// X-Wings and Y-Wings should follow slightly different paths so they don't collide.
							if( (ship->Category() == ShipClass::CATEGORY_BOMBER) && (Waypoints[ 1 ].size()) )
								waypoint_list = &(Waypoints[ 1 ]);
							else if( (ship->Category() == ShipClass::CATEGORY_FIGHTER) && (Waypoints[ 2 ].size()) )
								waypoint_list = &(Waypoints[ 2 ]);
						}
					}
				}
				else if( GameType == XWing::GameType::FLEET_BATTLE )
				{
					if( Waypoints[ 0 ].size() )
					{
						// In Fleet Battle, the capital ships circle eachother.
						if( ship->IsMissionObjective && ! dodging )
							waypoint_list = &(Waypoints[ 0 ]);
					}
				}
				else if( ((GameType == XWing::GameType::TEAM_RACE) || (GameType == XWing::GameType::FFA_RACE)) && ! dodging )
				{
					if( ! ship->NextCheckpoint )
					{
						for( std::map<uint32_t,GameObject*>::const_iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
						{
							if( obj_iter->second->Type() == XWing::Object::CHECKPOINT )
							{
								ship->NextCheckpoint = obj_iter->first;
								break;
							}
						}
					}
					
					waypoint = Data.GetObject( ship->NextCheckpoint );
					
					// Allow AI to go off-course to kill players.
					if( target && target->Owner() )
						waypoint = NULL;
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
							
							if( dist < 20. )
								continue;
							
							if( deathstar && deathstar->WithinTrenchH(wp) && ! deathstar->WithinTrenchH(ship) )
							{
								// Don't dive straight down into the trench.
								double dot_up = vec_to_waypoint.Unit().Dot( &(deathstar->Up) );
								if( dot_up < -0.95 )
									continue;
								if( (dist < 300.) && (dot_up < -0.75) )
									continue;
							}
							
							if( (! waypoint) || (dist < waypoint_dist) )
							{
								waypoint = wp;
								waypoint_dist = dist;
							}
						}
					}
				}
				
				// If we have a waypoint, fly towards it.
				// NOTE: This overrides dodging, so only set a waypoint if we're sure we want to proceed straight on course.
				if( waypoint )
				{
					Vec3D vec_to_waypoint( waypoint->X - ship->X, waypoint->Y - ship->Y, waypoint->Z - ship->Z );
					Vec3D unit_vec_to_waypoint = vec_to_waypoint.Unit();
					
					// Fly above the trench until it's a good angle to dive in.
					if( deathstar && deathstar->WithinTrenchH(waypoint) && ((unit_vec_to_waypoint.Dot( &(deathstar->Up) ) < -0.5) || ! deathstar->WithinTrenchW(ship)) )
					{
						vec_to_waypoint += deathstar->Up * deathstar->TrenchDepth;
						unit_vec_to_waypoint = vec_to_waypoint.Unit();
					}
					
					double w_dot_fwd = unit_vec_to_waypoint.Dot( &(ship->Fwd) );
					double w_dot_up = unit_vec_to_waypoint.Dot( &(ship->Up) );
					double w_dot_right = unit_vec_to_waypoint.Dot( &(ship->Right) );
					
					bool valid_target = false;
					double dist_to_target = 0.;
					bool target_is_exhaust_port = false;
					if( target )
					{
						dist_to_target = ship->Dist( target );
						bool ship_within_trench = deathstar && deathstar->WithinTrenchH( ship );
						target_is_exhaust_port = (GameType == XWing::GameType::BATTLE_OF_YAVIN) && (target->Type() == XWing::Object::SHIP) && ((Ship*)( target ))->IsMissionObjective;
						valid_target = facing_target && (w_dot_fwd > (ship_within_trench ? 0.995 : 0.95)) && (dist_to_target < 1000.);
						if( valid_target && (target->Type() == XWing::Object::TURRET) && (dist_to_target < vec_to_waypoint.Length() * 0.75) )
							valid_target = false;
					}
					
					if( valid_target && target_is_exhaust_port && ! ai_jedi )
					{
						// We don't typically want AI ships to accurately aim at the exhaust port.
						valid_target = false;
						if( (! rebel_players) && ! ai_easy )
						{
							// Make the Empire work for their win.
							valid_target = ai_hard || (dist_to_target > (600. + RoundTimer.RemainingSeconds()));
						}
					}
					
					// Allow turning towards targets that are ahead of us near the waypoint.
					if( valid_target )
						;
					else
					{
						pitch = Num::SignedPow( w_dot_up, 0.4 ) * 1.125;
						yaw = Num::SignedPow( w_dot_right, 0.5 ) * 1.25;
						if( w_dot_fwd > 0.9999 )
						{
							pitch *= (1. - w_dot_fwd) * 10000.;
							yaw   *= (1. - w_dot_fwd) * 10000.;
						}
						roll = (w_dot_fwd < 0.99) ? (w_dot_right * 2.) : ship->Right.Dot(0.,0.,1.);
						throttle = pow( w_dot_fwd, 0.7 ) + 0.1;
						
						// They do allow some dodging when chasing waypoints, but you can tell they don't like it.
						if( dodging && (dodging->DistAlong( &(ship->Fwd), ship ) > 0.) )
							throttle *= 0.75;
						
						if( GameType == XWing::GameType::FLEET_BATTLE )
						{
							if( (ship->Category() == ShipClass::CATEGORY_CAPITAL) && (ship->Up.Dot(0.,0.,1.) < 0.9) )
							{
								// Prevent capital flagships in Fleet Battle from rolling completely sideways.
								yaw = Num::Sign( yaw );
								roll = ship->Right.Dot(0.,0.,1.);
							}
							
							// Capital ships slow when approaching waypoint so they can turn more quickly towards the next one.
							if( (ship->Category() == ShipClass::CATEGORY_CAPITAL) && (waypoint->DistAlong( &(ship->Fwd), ship ) < 25.) )
								throttle *= 0.25;
							
							// Avoid ramming into the enemy flagship if we are overtaking it.
							Ship *avoid_overtaking = (ship->Team == XWing::Team::REBEL) ? empire_flagship : rebel_flagship;
							if( avoid_overtaking && ship->Dist(avoid_overtaking) )
							{
								Vec3D vec_to_other = *avoid_overtaking - *ship;
								vec_to_other.ScaleTo( 1. );
								double o_dot_fwd = ship->Fwd.Dot(&vec_to_other);
								if( (ship->Category() == ShipClass::CATEGORY_CAPITAL) || (avoid_overtaking->Category() != ShipClass::CATEGORY_CAPITAL) )
								{
									if( o_dot_fwd > 0.7 )
										throttle *= 0.25;
								}
								// If we are a fighter overtaking a capital ship, go above or below it.
								else if( (o_dot_fwd > 0.9)
								||       ship->WillCollide( avoid_overtaking, 3. )
								||       (  ( ship->Dist(avoid_overtaking) < avoid_overtaking->Shape.GetTriagonal() )
								         && ( fabs(ship->DistAlong( &(avoid_overtaking->Up), avoid_overtaking )) < (avoid_overtaking->Radius() + ship->Radius()) ) ))
									pitch = std::max<double>( 0., o_dot_fwd ) * ((ship->Up.Dot(&vec_to_other) >= 0.) ? -1. : 1.);
							}
						}
						else if( (GameType == XWing::GameType::TEAM_RACE) || (GameType == XWing::GameType::FFA_RACE) )
						{
							throttle *= players_on_team ? 0.9375 : (0.875 + ai_skill * 0.0625);
							
							// Roll to get ready for the next waypoint.
							if( w_dot_fwd > 0.98 )
							{
								Checkpoint *curr_checkpoint = (Checkpoint*) waypoint;
								Checkpoint *next_checkpoint = (Checkpoint*) Data.GetObject( curr_checkpoint->Next );
								if( next_checkpoint && next_checkpoint->Dist(curr_checkpoint) )
								{
									Vec3D ideal_up = (*next_checkpoint - *curr_checkpoint).Unit();
									roll = ship->Right.Dot(&ideal_up);
								}
							}
						}
						
						// Don't fire at the waypoint just because our target is somewhere near the front of us.
						if( ! target_is_exhaust_port )
							ship->Firing = false;
					}
				}
				else if( deathstar )
				{
					// Don't be like Porkins.
					double dist = ship->DistAlong( &(deathstar->Up), deathstar ) - ship->Radius();
					double time = dist / ship->MaxSpeed();
					double d_dot_fwd = deathstar->Up.Dot( &(ship->Fwd) );
					double d_dot_up = deathstar->Up.Dot( &(ship->Up) );
					double d_dot_right = deathstar->Up.Dot( &(ship->Right) );
					if( (time < 1.5) && (d_dot_fwd < 0.) )
					{
						if( d_dot_up < 0. )
							roll = (d_dot_right < 0.) ? -1. : 1.;
						else
							roll = d_dot_right;
						yaw *= 2.;
					}
					if( time < 0.75 )
					{
						pitch = std::max<double>( pitch, 1.25 - time );
						throttle = (d_dot_fwd < 0.) ? time : 1.;
					}
				}
				
				if( ai_ceasefire || (rebel_ceasefire && (ship->Team == XWing::Team::REBEL)) || (empire_ceasefire && (ship->Team == XWing::Team::EMPIRE)) )
					ship->Firing = false;
				
				if( ai_stop )
				{
					pitch = 0.;
					yaw = 0.;
					roll = 0.;
					throttle = 0.;
				}
				else if( ship->JumpProgress < 1. )
				{
					// Arriving from hyperspace.
					pitch = 0.;
					yaw = 0.;
					roll = 0.;
					ship->Firing = false;
					ship->Target = 0;
				}
				
				// Apply AI's desired controls.
				ship->SetPitch( pitch, dt );
				ship->SetYaw( yaw, dt );
				ship->SetRoll( roll, dt );
				std::string engine_sound = ship->SetThrottleGetSound( throttle, dt );
				
				if( engine_sound.length() )
				{
					Packet play_sound( XWing::Packet::ENGINE_SOUND );
					//play_sound.AddUInt( ship->ID );  // Not needed now, but may be helpful if we ever want spectators to hear engine sounds.
					play_sound.AddString( engine_sound );
					
					// If we have any player turret gunners, send the sound to them.
					std::list<Turret*> turrets = ship->AttachedTurrets();
					for( std::list<Turret*>::const_iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
					{
						if( (*turret_iter)->PlayerID )
							Net.SendToPlayer( &play_sound, (*turret_iter)->PlayerID );
					}
				}
			}
		}
		
		
		// Spawn new ships for players that changed from Spectator or Gunner to a ship.
		
		if( Respawn )
		{
			for( std::map< uint8_t, std::list<Player*> >::const_iterator team_iter = pilots_waiting.begin(); team_iter != pilots_waiting.end(); team_iter ++ )
			{
				for( std::list<Player*>::const_iterator waiting = team_iter->second.begin(); waiting != team_iter->second.end(); waiting ++ )
				{
					if( (RespawnClocks.find( (*waiting)->ID ) == RespawnClocks.end()) || (RespawnClocks[ (*waiting)->ID ].ElapsedSeconds() >= RespawnDelay) )
						BeginFlying( (*waiting)->ID, true );
				}
			}
		}
		
		
		// Update turret ownership.
		
		std::set<Turret*> turrets_updated;
		
		for( std::map<uint32_t,Turret*>::iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
		{
			Turret *turret = (Turret*) turret_iter->second;
			if( turret->PlayerID )
			{
				// Check if the previous owner is still here.
				Player *owner = turret->Owner();
				if( ! owner )
				{
					turret->PlayerID = 0;
					turrets_updated.insert( turret );
				}
			}
		}
		
		
		// Place waiting gunners into available turrets.
		
		for( std::map< uint8_t, std::map< uint8_t,std::list<Player*> > >::iterator team_iter = gunners_waiting.begin(); team_iter != gunners_waiting.end(); team_iter ++ )
		{
			for( std::map< uint8_t,std::list<Player*> >::iterator group_iter = team_iter->second.begin(); group_iter != team_iter->second.end(); group_iter ++ )
			{
				for( std::list<Player*>::iterator gunner_iter = group_iter->second.begin(); gunner_iter != group_iter->second.end(); )
				{
					std::list<Player*>::iterator next_gunner = gunner_iter;
					next_gunner ++;
					
					uint16_t gunner_player_id = (*gunner_iter)->ID;
					std::map<uint16_t,Clock>::iterator clock_iter = RespawnClocks.find( gunner_player_id );
					double gunner_waited = (clock_iter != RespawnClocks.end()) ? clock_iter->second.ElapsedSeconds() : GUNNER_WAIT;
					double wait_time = (round_time > 1.) ? ((player_ships_without_group && ! group_iter->first) ? 5.1 : GUNNER_WAIT) : 0.;
					
					for( std::map<uint32_t,Turret*>::reverse_iterator turret_iter = turrets.rbegin(); turret_iter != turrets.rend(); turret_iter ++ )
					{
						Turret *turret = (Turret*) turret_iter->second;
						if( (turret->Team == team_iter->first) && ! turret->PlayerID )
						{
							Ship *parent_ship = turret->ParentShip();
							uint8_t turret_group = (parent_ship && (parent_ship->Group != 255)) ? parent_ship->Group : 0;
							if( turret_group != group_iter->first )
								continue;
							if( gunner_waited < wait_time )
								continue;
							
							if( parent_ship && parent_ship->Owner() )
							{
								// Always spawn with any available player ship, but prefer those without a gunner yet.
								if( (gunner_waited < wait_time + 0.03) && parent_ship->PlayersInTurrets().size() )
									continue;
							}
							else if( parent_ship )
							{
								// Prefer player ships.
								if( gunner_waited < wait_time + 0.07 )
									continue;
								// Prefer non-objective ships.
								else if( (gunner_waited < wait_time + 0.1) && parent_ship && (parent_ship->Group == 255) )
									continue;
								else if( (gunner_waited < wait_time + 2.) && parent_ship && parent_ship->IsMissionObjective )
									continue;
							}
							else
							{
								// Prefer ship turrets rather than surface turrets.
								if( gunner_waited < wait_time + 0.13 )
									continue;
								// Prefer surface turrets rather than trench turrets.
								if( (gunner_waited < wait_time + 0.2) && deathstar && (turret->DistAlong( &(deathstar->Up), deathstar ) < 0.) )
									continue;
							}
							
							// If the ship we are moving to has no gunner in its first turret, go there instead.
							if( parent_ship )
							{
								Turret *ship_first_turret = parent_ship->AttachedTurret();
								if( ! ship_first_turret->PlayerID )
									turret = ship_first_turret;
							}
							
							// Assign gunner to turret and remove from waiting list.
							turret->PlayerID = gunner_player_id;
							turrets_updated.insert( turret );
							std::map<uint16_t,Clock>::iterator clock_iter = RespawnClocks.find( gunner_player_id );
							if( clock_iter != RespawnClocks.end() )
								RespawnClocks.erase( clock_iter );
							std::map<uint16_t,Ship*>::iterator player_ship = player_ships.find( gunner_player_id );
							if( player_ship != player_ships.end() )
								player_ship->second->CanRespawn = false;
							gunners_waiting[ turret->Team ][ turret_group ].erase( gunner_iter );
							break;
						}
					}
					
					gunner_iter = next_gunner;
				}
			}
		}
		
		
		// Notify of any turrets that changed ownership.
		
		for( std::set<Turret*>::iterator turret_iter = turrets_updated.begin(); turret_iter != turrets_updated.end(); turret_iter ++ )
			(*turret_iter)->SendUpdate( 126 );
		
		
		// Check for any mission events that might be ready.
		
		for( std::map< uint8_t, std::vector<MissionEvent> >::iterator trigger_iter = EventTriggers.begin(); trigger_iter != EventTriggers.end(); trigger_iter ++ )
		{
			for( std::vector<MissionEvent>::iterator event_iter = trigger_iter->second.begin(); event_iter != trigger_iter->second.end(); event_iter ++ )
				event_iter->FireWhenReady( &add_object_ids );
		}
		
		
		// Check for firing ships, expired shots, and ships to remove/respawn.
		
		uint32_t ship_count = 0, dead_count = 0, rebel_count = 0, empire_count = 0, rebel_objectives = 0, empire_objectives = 0;
		uint16_t live_rebel_players = 0, live_empire_players = 0, dead_rebel_players = 0, dead_empire_players = 0;
		uint32_t last_ship = 0;
		std::map< uint8_t, std::map<uint8_t,Pos3D> > group_spawns;
		std::map< uint8_t, std::map<uint8_t,int> > group_spawn_count;
		std::set<DockingBay*> remove_docking_bays;
		Ship *rebel_coop_respawn = NULL, *empire_coop_respawn = NULL;
		//double safest_rebel_respawn = 0., safest_empire_respawn = 0.;  // FIXME
		double respawn_time = Data.PropertyAsDouble( "respawn_time", RespawnDelay, RespawnDelay );
		
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
				
				double respawn_delay = respawn_time;
				if( (ship->Group == 255) && ship->Team )
					respawn_delay = (ship->Team == XWing::Team::REBEL) ? RebelCruiserRespawn : EmpireCruiserRespawn;
				
				if( ship->Health > 0. )
				{
					ship_count ++;
					if( ship_count > 1 )
						last_ship = 0;
					else if( ! last_ship )
						last_ship = ship->ID;
					
					if( ship->Team == XWing::Team::REBEL )
					{
						rebel_count ++;
						if( ship->IsMissionObjective || (ship->Group == 255) )
							rebel_objectives ++;
						else if( (GameType == XWing::GameType::TEAM_ELIMINATION) && ! ship->CanRespawn )
							rebel_objectives ++;
						if( ship->PlayerID )
						{
							live_rebel_players ++;
							rebel_coop_respawn = ship;  // FIXME: Check if safe distance from enemies!
						}
					}
					else if( ship->Team == XWing::Team::EMPIRE )
					{
						empire_count ++;
						if( ship->IsMissionObjective || (ship->Group == 255) )
							empire_objectives ++;
						else if( (GameType == XWing::GameType::TEAM_ELIMINATION) && ! ship->CanRespawn )
							empire_objectives ++;
						if( ship->PlayerID )
						{
							live_empire_players ++;
							empire_coop_respawn = ship;  // FIXME: Check if safe distance from enemies!
						}
					}
					
					if( (ship->Firing || ship->PredictedShots.size()) && (ship->FiringClocks[ ship->PredictedShots.size() ? ((ship->PredictedShots.front() & 0xF0) >> 4) : ship->SelectedWeapon ].ElapsedSeconds() >= ship->ShotDelay()) )
					{
						if( ship->PredictedShots.size() )
						{
							uint8_t predicted = ship->PredictedShots.front();
							ship->PredictedShots.pop_front();
							ship->SelectedWeapon                     = (predicted & 0xF0) >> 4;
							ship->FiringMode[ ship->SelectedWeapon ] =  predicted & 0x0F;
						}
						
						GameObject *target = Data.GetObject( ship->Target );
						bool locked = ship->LockingOn( target );
						std::vector<Shot*> shots = ship->NextShots( target );
						ship->JustFired();
						
						for( std::vector<Shot*>::iterator shot_iter = shots.begin(); shot_iter != shots.end(); shot_iter ++ )
						{
							uint32_t shot_id = Data.AddObject( *shot_iter );
							add_object_ids.insert( shot_id );
							
							// No missile lock if they weren't facing the target when they fired.
							if( ! locked )
								(*shot_iter)->Seeking = 0;
						}
					}
				}
				else if( respawn_delay && ship->CanRespawn )
				{
					if( ship->PlayerID )
					{
						if( ship->Team == XWing::Team::REBEL )
							live_rebel_players ++;
						else if( ship->Team == XWing::Team::EMPIRE )
							live_empire_players ++;
					}
					
					if( ship->DeathClock.ElapsedSeconds() >= respawn_delay )
					{
						bool ready_to_spawn = true;
						
						// If part of a group, wait to spawn with them.
						if( ship->Group && ! GroupJumpingIn.count( ship->ID ) )
						{
							for( std::map<uint32_t,Ship*>::iterator ship_iter2 = ships.begin(); ship_iter2 != ships.end(); ship_iter2 ++ )
							{
								if( (ship_iter2->second != ship) && (ship_iter2->second->Team == ship->Team) && (ship_iter2->second->Group == ship->Group) && ((ship_iter2->second->Health > 0.) || (ship_iter2->second->DeathClock.ElapsedSeconds() < respawn_delay)) && (group_spawns[ ship->Team ].find( ship->Group ) == group_spawns[ ship->Team ].end()) )
									ready_to_spawn = false;
							}
							
							if( ready_to_spawn )
							{
								GroupJumpingIn.insert( ship->ID );
								ready_to_spawn = false;
							}
						}
						
						// See if the player wants to change ships (which may also change team) or switch to spectator.
						if( ready_to_spawn && ship->PlayerID )
						{
							Player *player = ship->Owner();
							if( player )
							{
								std::string desired_ship = ChosenShip( player );
								if( (strcasecmp( desired_ship.c_str(), "Spectator" ) == 0) || (Str::FindInsensitive( desired_ship, " Gunner" ) >= 0) )
									ready_to_spawn = false;
								else if( AllowShipChange && desired_ship.length() )
								{
									const ShipClass *sc = GetShipClass( desired_ship );
									if( sc && (sc != ship->Class) )
									{
										bool darkside = Data.PropertyAsBool("darkside");
										bool allow_class_change = sc->PlayersCanFly() || darkside;
										if( (Data.PropertyAsString("gametype") == "mission") && (Data.PropertyAsString("player_ship").length() || Data.PropertyAsString("player_ships").length()) )
											allow_class_change = true;  // ChosenShip has already ensured a valid result.
										
										if( allow_class_change && ship->Team && sc->Team && (ship->Team != sc->Team) && ! (AllowTeamChange || darkside) )
											allow_class_change = false;
										
										if( allow_class_change )
										{
											if( ship->Team && sc->Team && (ship->Team != sc->Team) )
											{
												ship->Team = sc->Team;
												SetPlayerProperty( player, "team", (ship->Team == XWing::Team::REBEL) ? "Rebel" : "Empire" );
												
												Packet message( Raptor::Packet::MESSAGE );
												message.AddString( player->Name + std::string(" defected to the ") + std::string( (ship->Team == XWing::Team::REBEL) ? "Rebellion!" : "Empire!" ) );
												message.AddUInt( TextConsole::MSG_CHAT );
												Net.SendAll( &message );
											}
											
											ship->SetClass( sc );
										}
									}
								}
							}
						}
						
						// By default, the position is randomized.
						bool fixed_spawn = false;
						Pos3D spawn_at;
						
						// Fleet Battle mode requires fighters to spawn with their flagship.
						if( ready_to_spawn && (GameType == XWing::GameType::FLEET_BATTLE) )
						{
							ready_to_spawn = false;
							fixed_spawn = true;
							Ship *spawn_at_ship = NULL;
							
							if( (ship->Team == XWing::Team::REBEL) && rebel_flagship )
								spawn_at_ship = rebel_flagship;
							else if( (ship->Team == XWing::Team::EMPIRE) && empire_flagship )
								spawn_at_ship = empire_flagship;
							
							if( spawn_at_ship )
							{
								ready_to_spawn = true;
								double ship_height = ship->Shape.GetHeight();
								double spawn_at_height = spawn_at_ship->Shape.GetHeight();
								double vertical = (ship_height ? ship_height : ship->Radius()) + (spawn_at_height ? spawn_at_height : spawn_at_ship->Radius());
								if( (ship->Group == 255) && (vertical < 300.) )
									vertical = 300.;
								spawn_at.Copy( spawn_at_ship );
								
								if( spawn_at_ship->IsMissionObjective )
								{
									// Spawn below the flagship, facing inward.
									spawn_at.MoveAlong( &(spawn_at_ship->Up), Rand::Double( -20., 20. ) - vertical );
									spawn_at.MoveAlong( &(spawn_at_ship->Fwd), Rand::Double( ship->Radius() * -11.1, ship->Radius() * 11.1 ) );
									spawn_at.Fwd.Copy( &(spawn_at_ship->Right) );
									Vec3D to_center( -(spawn_at_ship->X), -(spawn_at_ship->Y), -(spawn_at_ship->Z) );
									to_center.ScaleTo( 1. );
									double towards_center = spawn_at.Fwd.Dot(&to_center);
									if( towards_center < 0. )
										spawn_at.Fwd.ScaleBy( -1. );
									spawn_at.FixVectors();
									spawn_at.MoveAlong( &(spawn_at.Fwd), spawn_at_ship->Radius() * 1.5 + Rand::Double( 0., 50. ) );
								}
								else
								{
									// When respawning at a cruiser, place spawn point away from the center, facing inward.
									Vec3D vec_from_center( spawn_at_ship->X, spawn_at_ship->Y, spawn_at_ship->Z );
									vec_from_center.ScaleTo( std::max<double>( 2500., vec_from_center.Length() + spawn_at_ship->Radius() * 2. + ship->Radius() * 4. + 500. ) );
									spawn_at.SetPos( vec_from_center.X, vec_from_center.Y, vec_from_center.Z );
									spawn_at.Up.Set( 0., 0., 1. );
									vec_from_center.ScaleTo( -1. );
									spawn_at.SetFwdVec( vec_from_center.X, vec_from_center.Y, vec_from_center.Z );
									spawn_at.FixVectors();
									spawn_at.MoveAlong( &(spawn_at.Up),    Rand::Double( -10., 10. ) * ship->Radius() );
									spawn_at.MoveAlong( &(spawn_at.Fwd),   Rand::Double(  -5.,  5. ) * ship->Radius() );
									spawn_at.MoveAlong( &(spawn_at.Right), Rand::Double( -10., 10. ) * ship->Radius() );
								}
							}
							
							// If capital ship is dead, respawn will no longer be possible.
							if( ! ready_to_spawn )
							{
								ship->CanRespawn = false;
								
								Player *player = ship->Owner();
								if( player )
								{
									Packet time_to_respawn( XWing::Packet::TIME_TO_RESPAWN );
									time_to_respawn.AddFloat( -1.f );
									Net.SendToPlayer( &time_to_respawn, player->ID );
								}
							}
						}
						else if( ready_to_spawn && ((GameType == XWing::GameType::TEAM_RACE) || (GameType == XWing::GameType::FFA_RACE)) )
						{
							// When killed in a race, spawn at the previous checkpoint.
							const Checkpoint *next_checkpoint = (const Checkpoint*) Data.GetObject( ship->NextCheckpoint );
							const Checkpoint *prev_checkpoint = (next_checkpoint && (next_checkpoint->Type() == XWing::Object::CHECKPOINT)) ? (const Checkpoint*) Data.GetObject(next_checkpoint->Prev) : NULL;
							if( prev_checkpoint )
							{
								spawn_at.Copy( prev_checkpoint );
								spawn_at.MoveAlong( &(prev_checkpoint->Fwd),   Rand::Double( ship->Radius() * -5., 10. ) );
								spawn_at.MoveAlong( &(prev_checkpoint->Up),    Rand::Double( -20., 20. ) );
								spawn_at.MoveAlong( &(prev_checkpoint->Right), Rand::Double( -20., 20. ) );
								fixed_spawn = true;
							}
						}
						
						if( ready_to_spawn && ship->Group && (GroupStagger[ ship->Group ].Progress() < 1.) )
							ready_to_spawn = false;
						
						if( ready_to_spawn )
						{
							ship->Reset();
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
							SpawnShipTurrets( ship, &add_object_ids );
							SpawnShipDockingBays( ship );
							
							if( ship->Group )
							{
								// Spawn flight groups together.
								int movement_iterations = GroupJumpingIn.size() - 1;
								for( int i = 0; i < movement_iterations; i ++ )
								{
									// Another ship from this group is spawning ahead.
									double radius = ship->Radius();
									if( ship->Group == 255 )
									{
										radius = std::max<double>( 250., ship->Radius() );
										group_spawns[ ship->Team ][ ship->Group ].MoveAlong( &(group_spawns[ ship->Team ][ ship->Group ].Fwd), -2.5 * radius );
									}
									else
									{
										group_spawns[ ship->Team ][ ship->Group ].MoveAlong( &(group_spawns[ ship->Team ][ ship->Group ].Fwd), -8. * radius );
										group_spawns[ ship->Team ][ ship->Group ].MoveAlong( &(group_spawns[ ship->Team ][ ship->Group ].Right), 4. * radius );
									}
									ship->Copy( &(group_spawns[ ship->Team ][ ship->Group ]) );
									if( ship->Group == 255 )
									{
										// Cruiser group should arrive in a scattered wedge, not in tight echelon formation like other groups.
										double spread = group_spawn_count[ ship->Team ][ ship->Group ] * std::min<double>( 200., radius );
										ship->MoveAlong( &(ship->Right), Rand::Double(  2.5, 3.5 ) * spread * ((group_spawn_count[ ship->Team ][ ship->Group ] % 2) ? 1. : -1.) );
										ship->MoveAlong( &(ship->Up),    Rand::Double( -4.0, 0.0 ) * spread );
									}
									group_spawn_count[ ship->Team ][ ship->Group ] ++;
								}
								if( ! movement_iterations )
								{
									// This is the lead ship of its flight group.
									group_spawns[ ship->Team ][ ship->Group ].Copy( ship );
									group_spawn_count[ ship->Team ][ ship->Group ] = 1;
								}
								
								GroupStagger[ ship->Group ].Reset( (ship->Category() == ShipClass::CATEGORY_CAPITAL) ? Rand::Double( 0.2, 0.6 ) : Rand::Double( 0.1, 0.4 ) );
								GroupJumpingIn.erase( GroupJumpingIn.find( ship->ID ) );
							}
							
							// Notify player that they are respawning.
							ship->SendUpdate( 127 );
							
							TriggerEvent( MissionEvent::TRIGGER_ON_RESPAWN, ship );
						}
					}
				}
				else  // Dead, cannot respawn.
				{
					dead_count ++;
					if( ship->PlayerID )
					{
						if( ship->Team == XWing::Team::REBEL )
							dead_rebel_players ++;
						else if( ship->Team == XWing::Team::EMPIRE )
							dead_empire_players ++;
					}
					
					if( ship->DeathClock.ElapsedSeconds() >= std::max<double>( 5., respawn_delay ) )
					{
						if( ship->PlayerID && respawn_coop && (Data.Players.size() > 1) )
							respawn_waiting.insert( ship );
						else
							remove_object_ids.insert( ship->ID );
					}
				}
			}
			else if( obj_iter->second->Type() == XWing::Object::TURRET )
			{
				Turret *turret = (Turret*) obj_iter->second;
				Ship *parent_ship = turret->ParentShip();
				
				if( turret->Health > 0. )
				{
					if( turret->PlayerID )
					{
						if( turret->Team == XWing::Team::REBEL )
							live_rebel_players ++;
						else if( turret->Team == XWing::Team::EMPIRE )
							live_empire_players ++;
					}
					
					if( parent_ship && parent_ship->JumpedOut )
					{
						turret->Firing = false;
						turret->Target = 0;
						turret->RollRate = turret->PitchRate = turret->YawRate = turret->GunPitchRate = 0.;
						continue;
					}
					
					GameObject *target = NULL;
					Player *target_owner = NULL;
					uint8_t target_subsystem = 0;
					
					Player *parent_owner = parent_ship ? parent_ship->Owner() : NULL;
					size_t players_on_team = (turret->Team == XWing::Team::REBEL) ? (rebel_players && ! empire_players) : ((turret->Team == XWing::Team::EMPIRE) ? (empire_players && ! rebel_players) : 0);
					
					if( turret->PlayerID )
						; // Turret is player controlled, so no AI control.
					else if( turret->Manual && parent_owner )
						turret->Target = 0;
					else if( parent_ship && turret->ParentControl )
					{
						// This turret uses the parent's target.
						turret->Target = parent_ship->Target;
						target = Data.GetObject( turret->Target );
						if( target && (target->Type() == XWing::Object::CHECKPOINT) )
							target = NULL;
						target_owner = target ? target->Owner() : NULL;
						target_subsystem = parent_ship->TargetSubsystem;
					}
					else if( parent_ship && (parent_ship->JumpProgress < 1.) )
						; // Turret attached to a ship that's jumping in should not act yet.
					else
					{
						// If the turret doesn't have a target yet, and is allowed to select its own, pick one.
						
						// Set initial values very far away; any ships closer than this are worth considering.
						double nearest_enemy = 100000.;
						double nearest_friendly = nearest_enemy;
						Ship *friendly = NULL;
						bool found_exhaust_port = false;
						
						for( std::map<uint32_t,Ship*>::iterator target_iter = ships.begin(); target_iter != ships.end(); target_iter ++ )
						{
							// Don't target itself or its parent ship.
							if( (target_iter->first == turret->ID) || (target_iter->first == turret->ParentID) )
								continue;
							
							// Ignore ships that have been dead for a while.
							if( (target_iter->second->Health <= 0.) && (target_iter->second->DeathClock.ElapsedSeconds() > 4.) )
								continue;
							
							// Death Star turrets stop firing a couple seconds after the exhaust port is hit.
							if( (GameType == XWing::GameType::BATTLE_OF_YAVIN) && target_iter->second->IsMissionObjective
							&&  ((target_iter->second->Health > 0.) || (target_iter->second->DeathClock.ElapsedSeconds() < 2.)) )
								found_exhaust_port = true;
							
							// Don't target ships that are still jumping in or have jumped out.
							if( (target_iter->second->JumpProgress < 1.) || target_iter->second->JumpedOut )
								continue;
							
							Player *target_iter_owner = target_iter->second->Owner();
							
							// Don't attack players immediately upon jumping in.
							if( target_iter_owner && (target_iter->second->Lifetime.ElapsedSeconds() < (2.5 - ai_skill)) )
								continue;
							
							// Don't attack players at all when AI skill is Bantha Fodder.
							if( target_iter_owner && ai_pudu )
								continue;
							
							Vec3D vec_to_target( target_iter->second->X - turret->X, target_iter->second->Y - turret->Y, target_iter->second->Z - turret->Z );
							
							// If the turret has a forced target direction, consider ships within that arc only.
							if( (turret->TargetArc < 360.) && (turret->TargetDir.AngleBetween( vec_to_target ) > (turret->TargetArc / 2.)) )
								continue;
							
							double dist = turret->Dist(target_iter->second);
							double adjusted_dist = dist;
							
							// Turrets with typical firing ranges should prefer targets that are not below them.
							if( turret->MinGunPitch >= -30. )
							{
								// FIXME: Use intercept point instead of position?
								double dot_up = vec_to_target.Unit().Dot(turret->Up);
								if( dot_up < -0.125 )
									adjusted_dist *= 1. - (dot_up + 0.125) * 7.;
							}
							
							if( turret->Team && (target_iter->second->Team == turret->Team) )
							{
								// Keep track of the nearest friendly.
								if( dist < nearest_friendly )
								{
									friendly = target_iter->second;
									nearest_friendly = dist;
								}
							}
							else if( ai_easy && (nearest_enemy > 550.) && (dist > 550.) && (dist < 1300.) && (target_iter_owner || target_owner) )
							{
								// Rookie AI should prioritize attacking AI ships within easy range to hit.
								if( (! target_iter_owner) && ((adjusted_dist < nearest_enemy) || (! target) || target_owner) )
								{
									target = target_iter->second;
									target_owner = target_iter_owner;
									nearest_enemy = adjusted_dist;
								}
							}
							else if( ai_sith && (dist < turret->MaxFiringDist * 1.1) && (target_iter_owner || target_owner) )
							{
								// Jedi Master AI should prioritize attacking players within range.
								if( target_iter_owner && ((adjusted_dist < nearest_enemy) || ! target_owner) )
								{
									target = target_iter->second;
									target_owner = target_iter_owner;
									nearest_enemy = adjusted_dist;
								}
							}
							else if( adjusted_dist < nearest_enemy )
							{
								// Target the nearest enemy.
								target = target_iter->second;
								target_owner = target_iter_owner;
								nearest_enemy = adjusted_dist;
							}
						}
						
						// Death Star turrets stop firing after the exhaust port is destroyed.
						if( target && (GameType == XWing::GameType::BATTLE_OF_YAVIN) && (! found_exhaust_port) && (! parent_ship) )
						{
							target = NULL;
							target_owner = NULL;
						}
						
						// Trench turrets deactivate when there's a friendly coming towards us, chasing close behind an enemy.
						if( target && friendly && turret->SafetyDistance )
						{
							Vec3D vec_to_friendly( friendly->X - turret->X, friendly->Y - turret->Y, friendly->Z - turret->Z );
							if( (friendly->MotionVector.Dot(&vec_to_friendly) < 0.) && ( (nearest_friendly < turret->SafetyDistance) || ((nearest_enemy < nearest_friendly) && (nearest_friendly - nearest_enemy < turret->SafetyDistance)) ) )
							{
								target = NULL;
								target_owner = NULL;
							}
						}
						
						// Ace AI turrets with no players on their team should attack shield generators that are in range.
						if( target && (target->Type() == XWing::Object::SHIP) && ai_hard && ! players_on_team )
						{
							const Ship *target_ship = (const Ship*) target;
							uint8_t subsystem_num = 1;
							for( std::map<std::string,double>::const_iterator subsystem_iter = target_ship->Subsystems.begin(); subsystem_iter != target_ship->Subsystems.end(); subsystem_iter ++ )
							{
								if( (subsystem_iter->second > 0.) && (strncmp( subsystem_iter->first.c_str(), "ShieldGen", strlen("ShieldGen") ) == 0) )
								{
									Vec3D vec_to_target = target_ship->TargetCenter( subsystem_num ) - turret->GunPos();
									if( vec_to_target.Length() < turret->MaxFiringDist )
									{
										target_subsystem = subsystem_num;
										break;
									}
								}
								subsystem_num ++;
							}
						}
					}
					
					if( turret->PlayerID )
						; // Turret is player controlled, so no AI control.
					else if( target )
					{
						Pos3D gun = turret->GunPos();
						Vec3D vec_to_target( target->X - gun.X, target->Y - gun.Y, target->Z - gun.Z );
						double dist_adjust = 0.;
						Ship *target_ship = (Ship*)( (target->Type() == XWing::Object::SHIP) ? target : NULL );
						if( target_ship && target_subsystem )
							vec_to_target = target_ship->TargetCenter( target_subsystem ) - gun;
						else if( target_ship && (target_ship->Category() == ShipClass::CATEGORY_CAPITAL) && target_ship->ComplexCollisionDetection() )
						{
							// If a ship's origin is in empty space (ex: Star Destroyer) the model object Hull defines the point to aim at.
							std::map<std::string,ModelObject*>::const_iterator object_iter = target_ship->Shape.Objects.find("Hull");
							if( (object_iter != target_ship->Shape.Objects.end()) && object_iter->second->Points.size() )
							{
								Vec3D point = object_iter->second->Points.front();
								vec_to_target += target_ship->Fwd   * point.X;
								vec_to_target += target_ship->Up    * point.Y;
								vec_to_target += target_ship->Right * point.Z;
							}
							dist_adjust = target_ship->Shape.GetTriagonal() / 2.;
						}
						double dist_to_target = vec_to_target.Length();
						Vec3D shot_vec = gun.Fwd;
						shot_vec.ScaleTo( 700. );  // Shot::Speed
						shot_vec += turret->MotionVector;
						shot_vec.ScaleTo( 700. );  // Shot::Speed
						shot_vec -= target->MotionVector;
						double time_to_target = dist_to_target / shot_vec.Length();
						Vec3D vec_to_intercept = vec_to_target;
						double aim_ahead = (ai_hard && target_owner && ! turret->ParentID) ? powf( turret->AimAhead, 0.75f ) : turret->AimAhead;
						vec_to_intercept += (target->MotionVector - turret->MotionVector) * time_to_target * aim_ahead;
						vec_to_target.ScaleTo( 1. );
						vec_to_intercept.ScaleTo( 1. );
						double i_dot_fwd = vec_to_intercept.Dot( &(gun.Fwd) );
						double i_dot_up = vec_to_intercept.Dot( &(gun.Up) );
						double i_dot_right = vec_to_intercept.Dot( &(gun.Right) );
						double t_dot_up = vec_to_target.Dot( &(gun.Up) );
						
						double aim_aggression = (turret->YawSpeed < 60.) ? 4. : 2.5; // FIXME: Should this be customizable?
						double aim_threshold = 1. / aim_aggression;
						
						turret->SetPitch( (fabs(i_dot_up) >= aim_threshold) ? Num::Sign(i_dot_up) : (i_dot_up * aim_aggression) );
						turret->Target = target->ID;
						turret->Firing = false;
						
						if( i_dot_fwd > 0. )
						{
							turret->SetYaw( (fabs(i_dot_right) >= aim_threshold) ? Num::Sign(i_dot_right) : (i_dot_right * aim_aggression) );
							
							if( turret->ParentControl )
								turret->Firing = parent_ship && parent_ship->Firing && ((i_dot_up > -0.02) || (t_dot_up > -0.02));
							else if( ! (ai_ceasefire || (rebel_ceasefire && (turret->Team == XWing::Team::REBEL)) || (empire_ceasefire && (turret->Team == XWing::Team::EMPIRE))) )
								turret->Firing = ((dist_to_target - dist_adjust) < turret->MaxFiringDist) && ((i_dot_up > -0.01) || (t_dot_up > -0.01));
							
							if( turret->Firing && parent_ship && (parent_ship->Category() == ShipClass::CATEGORY_CAPITAL) )
							{
								// Turrets should avoid shooting their parent ship.
								Pos3D gunpos = turret->GunPos();
								Pos3D ahead( &gunpos );
								ahead.MoveAlong( &(gunpos.Fwd), 50000. );
								gunpos.MoveAlong( &(gunpos.Fwd), 10. );
								std::string hit;
								double dist = parent_ship->Shape.DistanceFromLine( parent_ship, NULL, NULL, &hit, parent_ship->Exploded(), parent_ship->ExplosionSeed(), &gunpos, &ahead );
								if( (dist < 0.1) && ! hit.empty() )
									turret->Firing = false;
							}
						}
						else
							turret->SetYaw( i_dot_right ? Num::Sign(i_dot_right) : 1. );
					}
					else if( (turret->ParentControl || turret->Manual) && parent_owner && (turret->RelativeFwd.AngleBetween( Vec3D(1.,0.,0) ) <= (turret->TargetArc / 2.)) )
					{
						// Linked or manual turret, attached to a player ship with no selected target, allowed to aim forward: Turn towards forward.
						Pos3D gun = turret->GunPos();
						double dot_fwd = parent_ship->Fwd.Dot( &(gun.Fwd) );
						double dot_up = parent_ship->Fwd.Dot( &(gun.Up) );
						double dot_right = parent_ship->Fwd.Dot( &(gun.Right) );
						turret->SetPitch( (fabs(dot_up) >= 0.25) ? Num::Sign(dot_up) : (dot_up * 4.) );
						if( dot_fwd > 0. )
							turret->SetYaw( (fabs(dot_right) >= 0.25) ? Num::Sign(dot_right) : (dot_right * 4.) );
						else
							turret->SetYaw( Num::Sign(dot_right) );
						
						// Linked turrets without a target fire whenever pilot is firing.
						if( turret->ParentControl )
						{
							turret->Target = parent_ship->Target;
							turret->Firing = parent_ship->Firing;
						}
					}
					else
					{
						// No target, so stop moving and shooting.
						turret->SetYaw( 0. );
						turret->SetPitch( 0. );
						turret->Target = 0;
						turret->Firing = false;
					}
					
					if( turret->Firing && parent_ship && (parent_ship->JumpProgress < 1.) )
						turret->Firing = false;  // Make sure players don't try to fire turrets during hyperspace jump.
					
					if( (turret->Firing || turret->PredictedShots) && (turret->FiringClock.ElapsedSeconds() >= turret->ShotDelay()) )
					{
						GameObject *target = Data.GetObject( turret->Target );
						std::vector<Shot*> shots = turret->NextShots( target );
						turret->JustFired();
						if( turret->PredictedShots )
							turret->PredictedShots --;
						
						for( std::vector<Shot*>::iterator shot_iter = shots.begin(); shot_iter != shots.end(); shot_iter ++ )
						{
							uint32_t shot_id = Data.AddObject( *shot_iter );
							add_object_ids.insert( shot_id );
						}
					}
				}
				else
				{
					remove_object_ids.insert( turret->ID );
					if( turret->PlayerID )
					{
						if( turret->Team == XWing::Team::REBEL )
							dead_rebel_players ++;
						else if( turret->Team == XWing::Team::EMPIRE )
							dead_empire_players ++;
					}
				}
			}
			else if( obj_iter->second->Type() == XWing::Object::DOCKING_BAY )
			{
				DockingBay *dock = (DockingBay*) obj_iter->second;
				if( dock->Radius <= 0. )  // DockingBay::Update sets Radius to 0 when parent ship dies.
					remove_docking_bays.insert( dock );
			}
		}
		
		
		// Tell the clients about any objects we've added, like shots or mission event ships.
		
		SendAddedObjects( &add_object_ids );
		
		
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
		
		
		// Remove docking bays of dead ships.  No need to nofify clients.
		
		for( std::set<DockingBay*>::iterator dock_iter = remove_docking_bays.begin(); dock_iter != remove_docking_bays.end(); dock_iter ++ )
			Data.RemoveObject( (*dock_iter)->ID );
		
		
		// Handle coop respawn in multiplayer missions.
		
		for( std::set<Ship*>::iterator respawn_iter = respawn_waiting.begin(); respawn_iter != respawn_waiting.end(); respawn_iter ++ )
		{
			Ship *ship = *respawn_iter;
			Ship *spawn_at = (ship->Team == XWing::Team::EMPIRE) ? empire_coop_respawn : rebel_coop_respawn;
			if( spawn_at )
			{
				ship->Reset();
				ship->SetPos( spawn_at->X, spawn_at->Y, spawn_at->Z );
				ship->Fwd.Copy( &(spawn_at->Fwd) );
				ship->Up.Copy( &(spawn_at->Up) );
				ship->FixVectors();
				*ship -= (spawn_at->Fwd * ((ship->ID & 4) + 2) + spawn_at->Right * ((ship->ID & 2) - 0.5)) * ship->Radius() * 7.;
				SpawnShipTurrets( ship, &add_object_ids );
				SpawnShipDockingBays( ship );
				
				// Notify player that they are respawning.
				ship->SendUpdate( 127 );
				
				TriggerEvent( MissionEvent::TRIGGER_ON_RESPAWN, ship );
			}
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
				double trench_length = Data.PropertyAsDouble("yavin_dist",15000.);
				Packet explosion( XWing::Packet::EXPLOSION );
				explosion.AddDouble( Rand::Double(-500.,500.) * ended_secs + trench_length ); // X
				explosion.AddDouble( Rand::Double(-500.,500.) * ended_secs                 ); // Y
				explosion.AddDouble( Rand::Double(-2000.,-1000.)                           ); // Z
				explosion.AddFloat( 0.f );                       // dX
				explosion.AddFloat( 0.f );                       // dY
				explosion.AddFloat( Rand::Double(1000.,5000.) ); // dZ
				float size = Rand::Double(500.,2000.);
				explosion.AddFloat( size );       // Size
				explosion.AddFloat( size / 3.f ); // Loudness
				explosion.AddUChar( 30 );         // Sub-Explosions
				explosion.AddFloat( 1.25f );      // Speed Scale
				explosion.AddFloat( 0.5f );       // Speed Scale Sub
				Net.SendAll( &explosion );
			}
		}
		
		
		// If all players are dead with no abiltiy to respawn, we will consider that when checking end-of-round and victory conditions.
		
		bool all_players_dead = (dead_rebel_players || dead_empire_players) && ! (live_rebel_players || live_empire_players);
		
		
		// Check if the round is over.
		
		bool time_limit_reached = TimeLimit && (round_time >= TimeLimit * 60);
		
		if( State < XWing::State::ROUND_WILL_END )
		{
			if( Data.PropertyAsString("victor").length() )
			{
				State = XWing::State::ROUND_WILL_END;
				RoundEndedDelay = 0.;
				if( Data.PropertyAsBool("debug") )
					Console->Print( std::string("Round Will End: victor = ") + Data.PropertyAsString("victor") );
			}
			else if( time_limit_reached )
			{
				State = XWing::State::ROUND_WILL_END;
				if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
					RoundEndedDelay = 7.7;
				else
					RoundEndedDelay = 0.;
				if( Data.PropertyAsBool("debug") )
					Console->Print( "Round Will End: Time Limit Reached" );
			}
			else if( ! Data.PropertyAsBool("endgame",true,true) )
				;
			else if( (GameType == XWing::GameType::TEAM_ELIMINATION) && dead_count )
			{
				if( (rebel_objectives == 0) || (empire_objectives == 0) )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 3.;
					if( Data.PropertyAsBool("debug") )
						Console->Print( "Round Will End: Team Elimination" );
				}
			}
			else if( GameType == XWing::GameType::TEAM_RACE )
			{
				if( ((rebel_count == 0) || (empire_count == 0)) && dead_count && ! Respawn )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 3.;
					if( Data.PropertyAsBool("debug") )
						Console->Print( "Round Will End: Team Race: All Dead" );
				}
				else if( Checkpoints && ((TeamScores[ XWing::Team::REBEL ] >= Checkpoints) || (TeamScores[ XWing::Team::EMPIRE ] >= Checkpoints)) )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 0.;
					if( Data.PropertyAsBool("debug") )
						Console->Print( "Round Will End: Team Race: Team Score" );
				}
			}
			else if( (GameType == XWing::GameType::FFA_ELIMINATION) || (GameType == XWing::GameType::FFA_RACE) )
			{
				if( (ship_count <= 1) && dead_count && ! Respawn )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 3.;
					if( Data.PropertyAsBool("debug") )
						Console->Print( "Round Will End: FFA Elim/Race: All Dead" );
				}
				else if( (GameType == XWing::GameType::FFA_RACE) && Checkpoints )
				{
					Data.Lock.Lock();
					for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
					{
						if( player_iter->second->PropertyAsInt("score") >= Checkpoints )
						{
							State = XWing::State::ROUND_WILL_END;
							RoundEndedDelay = 0.;
							if( Data.PropertyAsBool("debug") )
								Console->Print( "Round Will End: FFA Race: Player Score" );
							break;
						}
					}
					Data.Lock.Unlock();
					
					for( std::map<uint32_t,int>::iterator score_iter = ShipScores.begin(); score_iter != ShipScores.end(); score_iter ++ )
					{
						if( score_iter->second >= Checkpoints )
						{
							State = XWing::State::ROUND_WILL_END;
							RoundEndedDelay = 0.;
							if( Data.PropertyAsBool("debug") )
								Console->Print( "Round Will End: FFA Race: Ship Score" );
							break;
						}
					}
				}
			}
			else if( (GameType == XWing::GameType::TEAM_DEATHMATCH) && KillLimit )
			{
				if( (TeamScores[ XWing::Team::REBEL ] >= KillLimit) || (TeamScores[ XWing::Team::EMPIRE ] >= KillLimit) )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 0.;
					if( Data.PropertyAsBool("debug") )
						Console->Print( "Round Will End: Team Deathmatch: Team Score" );
				}
			}
			else if( (GameType == XWing::GameType::FFA_DEATHMATCH) && KillLimit )
			{
				Data.Lock.Lock();
				for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
				{
					if( player_iter->second->PropertyAsInt("kills") >= KillLimit )
					{
						State = XWing::State::ROUND_WILL_END;
						RoundEndedDelay = 0.;
						if( Data.PropertyAsBool("debug") )
							Console->Print( "Round Will End: FFA Deathmatch: Player Score" );
						break;
					}
				}
				Data.Lock.Unlock();
				
				for( std::map<uint32_t,int>::iterator score_iter = ShipScores.begin(); score_iter != ShipScores.end(); score_iter ++ )
				{
					if( score_iter->second >= KillLimit )
					{
						State = XWing::State::ROUND_WILL_END;
						RoundEndedDelay = 0.;
						if( Data.PropertyAsBool("debug") )
							Console->Print( "Round Will End: FFA Deathmatch: Ship Score" );
						break;
					}
				}
			}
			else if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
			{
				if( (! empire_objectives) || ((rebel_count == 0) && dead_count && ! Respawn) )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 3.;
					if( Data.PropertyAsBool("debug") )
						Console->Print( "Round Will End: Battle of Yavin" );
				}
			}
			else if( GameType == XWing::GameType::CAPITAL_SHIP_HUNT )
			{
				if( DefendingTeam == XWing::Team::EMPIRE )
				{
					if( (! empire_objectives) || ((rebel_count == 0) && dead_count && ! Respawn) )
					{
						State = XWing::State::ROUND_WILL_END;
						RoundEndedDelay = 5.;
						if( Data.PropertyAsBool("debug") )
							Console->Print( "Round Will End: Hunt: Empire Defending" );
					}
				}
				else if( DefendingTeam == XWing::Team::REBEL )
				{
					if( (! rebel_objectives) || ((empire_count == 0) && dead_count && ! Respawn) )
					{
						State = XWing::State::ROUND_WILL_END;
						RoundEndedDelay = 5.;
						if( Data.PropertyAsBool("debug") )
							Console->Print( "Round Will End: Hunt: Rebel Defending" );
					}
				}
			}
			else if( GameType == XWing::GameType::FLEET_BATTLE )
			{
				if( (! empire_objectives) || (! rebel_objectives) || ((rebel_count == 1) && (empire_count == 1) && rebel_flagship && empire_flagship && dead_count && ! Respawn) )
				{
					State = XWing::State::ROUND_WILL_END;
					RoundEndedDelay = 5.;
					if( Data.PropertyAsBool("debug") )
						Console->Print( "Round Will End: Fleet Battle" );
				}
			}
			
			// Allow missions to override round end time.
			if( State == XWing::State::ROUND_WILL_END )
				RoundEndedDelay = Data.PropertyAsDouble( "end_delay", RoundEndedDelay, RoundEndedDelay );
			
			// End the game early if the players are all dead and cannot respawn.
			if( (State != XWing::State::ROUND_WILL_END) && all_players_dead )
			{
				State = XWing::State::ROUND_WILL_END;
				RoundEndedDelay = 1.5;
				if( Data.PropertyAsBool("debug") )
					Console->Print( "Round Will End: All Players Dead" );
				
				/*
				// Wait a little longer to end Team Kessel Run if the race is nearly complete and there are AI ships alive.
				if( (GameType == XWing::GameType::TEAM_RACE) && (rebel_count || empire_count) && Checkpoints
				&&  ((TeamScores[ XWing::Team::REBEL ] * 0.9 >= Checkpoints) || (TeamScores[ XWing::Team::EMPIRE ] * 0.9 >= Checkpoints)) )
					RoundEndedDelay = 15.;
				*/
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
				
				// See if all players are on the same team.
				uint8_t player_team = PlayerTeam;
				if( ffa )
					player_team = XWing::Team::NONE;
				else if( ! player_team )
				{
					Data.Lock.Lock();
					
					for( std::map<uint16_t,Player*>::const_iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
					{
						std::string chosen_team = ChosenTeam( Data.Players.begin()->second );
						if( chosen_team == "Rebel" )
						{
							if( player_team == XWing::Team::EMPIRE )
							{
								// There are players on both teams.
								player_team = XWing::Team::NONE;
								break;
							}
							player_team = XWing::Team::REBEL;
						}
						else if( chosen_team == "Empire" )
						{
							if( player_team == XWing::Team::REBEL )
							{
								// There are players on both teams.
								player_team = XWing::Team::NONE;
								break;
							}
							player_team = XWing::Team::EMPIRE;
						}
					}
					
					Data.Lock.Unlock();
				}
				
				// If the player(s) are all on one team and dead without respawn, treat it like a singleplayer/coop game: they lose.
				uint8_t dead_player_team = all_players_dead ? player_team : (uint8_t) XWing::Team::NONE;
				
				// Determine the victor.
				uint16_t victor = 0;
				std::string victor_name;
				std::string mission_victor = Data.PropertyAsString("victor");
				if( mission_victor.length() )
				{
					if( mission_victor == "player" )
						mission_victor = (player_team && (player_team == XWing::Team::REBEL)) ? "rebel" : "empire";
					else if( mission_victor == "ai" )
						mission_victor = (player_team && (player_team == XWing::Team::EMPIRE)) ? "rebel" : "empire";
					
					if( mission_victor == "rebel" )
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
				else if( dead_player_team && (Data.PropertyAsString("gametype") == "mission") )
				{
					if( dead_player_team == XWing::Team::EMPIRE )
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
				else if( GameType == XWing::GameType::TEAM_ELIMINATION )
				{
					if( rebel_count && empire_count && (dead_player_team == XWing::Team::REBEL) )
					{
						victor = XWing::Team::EMPIRE;
						victor_name = "The Galactic Empire";
					}
					else if( rebel_count && empire_count && (dead_player_team == XWing::Team::EMPIRE) )
					{
						victor = XWing::Team::REBEL;
						victor_name = "The Rebel Alliance";
					}
					else if( rebel_count > empire_count )
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
							Player *player = ship->Owner();
							if( player )
							{
								victor = player->ID;
								victor_name = player->Name;
							}
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
					Data.Lock.Lock();
					for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
					{
						if( player_iter->second->PropertyAsInt("kills") >= KillLimit )
						{
							victor = player_iter->first;
							victor_name = player_iter->second->Name;
							break;
						}
					}
					Data.Lock.Unlock();
					
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
				else if( GameType == XWing::GameType::TEAM_RACE )
				{
					bool score_limit_reached = (TeamScores[ XWing::Team::REBEL ] >= Checkpoints) || (TeamScores[ XWing::Team::EMPIRE ] >= Checkpoints);
					if( ! (score_limit_reached || rebel_count || empire_count || Respawn) )
					{
						// Everyone died before the checkpoint limit was reached.
						victor = XWing::Team::NONE;
					}
					else if( rebel_count && empire_count && ! (score_limit_reached || time_limit_reached) )
					{
						// Round was ended early.
						victor = XWing::Team::NONE;
					}
					else if( rebel_count && ! (empire_count || Respawn || score_limit_reached) )
					{
						victor = XWing::Team::REBEL;
						victor_name = "The Rebel Alliance";
					}
					else if( empire_count && ! (rebel_count || Respawn || score_limit_reached) )
					{
						victor = XWing::Team::EMPIRE;
						victor_name = "The Galactic Empire";
					}
					else if( (dead_player_team == XWing::Team::REBEL) && ! score_limit_reached )
					{
						victor = XWing::Team::EMPIRE;
						victor_name = "The Galactic Empire";
					}
					else if( (dead_player_team == XWing::Team::EMPIRE) && ! score_limit_reached )
					{
						victor = XWing::Team::REBEL;
						victor_name = "The Rebel Alliance";
					}
					else if( TeamScores[ XWing::Team::REBEL ] > TeamScores[ XWing::Team::EMPIRE ] )
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
				else if( GameType == XWing::GameType::FFA_RACE )
				{
					Data.Lock.Lock();
					for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
					{
						if( player_iter->second->PropertyAsInt("score") >= Checkpoints )
						{
							victor = player_iter->first;
							victor_name = player_iter->second->Name;
							break;
						}
					}
					Data.Lock.Unlock();
					
					if( ! victor )
					{
						for( std::map<uint32_t,int>::iterator score_iter = ShipScores.begin(); score_iter != ShipScores.end(); score_iter ++ )
						{
							if( score_iter->second >= Checkpoints )
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
					
					if( last_ship && ! victor )
					{
						Ship *ship = (Ship*) Data.GetObject( last_ship );
						if( ship )
						{
							victor = ship->PlayerID;
							Player *player = ship->Owner();
							if( player )
							{
								victor = player->ID;
								victor_name = player->Name;
							}
							else
								victor_name = ship->Name;
						}
					}
					
					if( time_limit_reached && ! victor )
					{
						int victor_score = 0;
						
						Data.Lock.Lock();
						
						for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
						{
							int player_score = player_iter->second->PropertyAsInt("score");
							if( player_score > victor_score )
							{
								victor = player_iter->first;
								victor_name = player_iter->second->Name;
								victor_score = player_score;
							}
							else if( player_score == victor_score )
							{
								victor = 0;
								victor_name = "";
							}
						}
						
						Data.Lock.Unlock();
						
						for( std::map<uint32_t,int>::iterator score_iter = ShipScores.begin(); score_iter != ShipScores.end(); score_iter ++ )
						{
							if( score_iter->second >= victor_score )
							{
								Ship *ship = (Ship*) Data.GetObject( score_iter->first );
								if( (! ship) || ship->Owner() )
									continue;  // We already checked player scores above.
								
								if( score_iter->second > victor_score )
								{
									victor = 0;
									victor_name = ship->Name;
									victor_score = score_iter->second;
								}
								else if( score_iter->second == victor_score )
								{
									victor = 0;
									victor_name = "";
								}
							}
						}
					}
				}
				else if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
				{
					if( (dead_player_team == XWing::Team::EMPIRE) || ! empire_objectives )
					{
						victor = XWing::Team::REBEL;
						victor_name = "The Rebel Alliance";
					}
					else if( round_time_remaining <= 0. )
					{
						victor = XWing::Team::EMPIRE;
						victor_name = "The Galactic Empire";
						
						double trench_length = Data.PropertyAsDouble("yavin_dist",15000.);
						Shot *superlaser = new Shot();
						superlaser->ShotType = Shot::TYPE_SUPERLASER;
						superlaser->Copy( deathstar );
						superlaser->Pitch( 90. );
						superlaser->Roll( -105. );
						superlaser->Pitch( 5. );
						superlaser->MotionVector = superlaser->Fwd * superlaser->Speed();
						superlaser->MoveAlong( &(deathstar->Fwd), trench_length );
						superlaser->MoveAlong( &(deathstar->Right), -4000. );
						superlaser->MoveAlong( &(deathstar->Up), 1. );
						
						Data.AddObject( superlaser );
						Packet objects_add( Raptor::Packet::OBJECTS_ADD );
						objects_add.AddUInt( 1 );
						objects_add.AddUInt( superlaser->ID );
						objects_add.AddUInt( superlaser->Type() );
						superlaser->AddToInitPacket( &objects_add );
						Net.SendAll( &objects_add );
					}
					else if( (dead_player_team == XWing::Team::REBEL) || ! (rebel_count || Respawn) )
					{
						victor = XWing::Team::EMPIRE;
						victor_name = "The Galactic Empire";
					}
				}
				else if( GameType == XWing::GameType::CAPITAL_SHIP_HUNT )
				{
					if( DefendingTeam == XWing::Team::EMPIRE )
					{
						if( (! empire_objectives) || (dead_player_team == XWing::Team::EMPIRE) )
						{
							victor = XWing::Team::REBEL;
							victor_name = "The Rebel Alliance";
						}
						else if( (round_time_remaining <= 0.) || (dead_player_team == XWing::Team::REBEL) || ! (rebel_count || Respawn) )
						{
							victor = XWing::Team::EMPIRE;
							victor_name = "The Galactic Empire";
						}
					}
					else if( DefendingTeam == XWing::Team::REBEL )
					{
						if( (! rebel_objectives) || (dead_player_team == XWing::Team::REBEL) )
						{
							victor = XWing::Team::EMPIRE;
							victor_name = "The Galactic Empire";
						}
						else if( (round_time_remaining <= 0.) || (dead_player_team == XWing::Team::EMPIRE) || ! (empire_count || Respawn) )
						{
							victor = XWing::Team::REBEL;
							victor_name = "The Rebel Alliance";
						}
					}
				}
				else if( GameType == XWing::GameType::FLEET_BATTLE )
				{
					if( rebel_objectives && empire_objectives && (dead_player_team == XWing::Team::REBEL) )
					{
						victor = XWing::Team::EMPIRE;
						victor_name = "The Galactic Empire";
					}
					else if( rebel_objectives && empire_objectives && (dead_player_team == XWing::Team::EMPIRE) )
					{
						victor = XWing::Team::REBEL;
						victor_name = "The Rebel Alliance";
					}
					else if( rebel_objectives && ! empire_objectives )
					{
						victor = XWing::Team::REBEL;
						victor_name = "The Rebel Alliance";
					}
					else if( empire_objectives && ! rebel_objectives )
					{
						victor = XWing::Team::EMPIRE;
						victor_name = "The Galactic Empire";
					}
				}
				
				// Make sure the final scores are updated.
				SendScores();
				
				// If all players are on the team that lost, play defeat music rather than the other team's victory music.
				uint16_t victor_music = (player_team && (victor != player_team)) ? ((uint32_t)( XWing::Team::NONE )) : victor;
				
				// Send round-ended packet (show scores, play round-ended music) to players.
				Packet round_ended( XWing::Packet::ROUND_ENDED );
				round_ended.AddUInt( GameType );
				round_ended.AddUShort( victor );
				round_ended.AddUShort( victor_music );
				round_ended.AddString( (Data.PropertyAsString("gametype") == "mission") ? Data.PropertyAsString("mission") : "" );
				round_ended.AddString( Data.PropertyAsString("next_mission") );
				Net.SendAll( &round_ended );
				
				// Send message packet to notify players of the victor.
				Packet message( Raptor::Packet::MESSAGE );
				if( victor_name.empty() )
				{
					if( victor )
						message.AddString( "The round is over!" );
					else
						message.AddString( "The round ended in a draw!" );
				}
				else
					message.AddString( (victor_name + " has won the round!").c_str() );
				Net.SendAll( &message );
				
				// Trigger victory/defeat event in missions.
				bool player_victory = ffa ? victor : (victor == player_team);
				TriggerEvent( player_victory ? (MissionEvent::TRIGGER_ON_VICTORY) : (MissionEvent::TRIGGER_ON_DEFEAT), ffa ? (uint8_t)(XWing::Team::NONE) : victor );
				
				if( Data.PropertyAsString("campaign").empty() )
				{
					// Send final player scores.
					bool sent_header = false;
					bool objective = (GameType != XWing::GameType::TEAM_ELIMINATION) && (GameType != XWing::GameType::TEAM_DEATHMATCH) && (GameType != XWing::GameType::TEAM_RACE) && (GameType != XWing::GameType::CTF) && ! ffa;
					for( std::map<uint16_t,Player*>::const_iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
					{
						if( (! player_iter->second->PropertyAsString("team").empty()) && (player_iter->second->PropertyAsString("team") != "Spectator") )
						{
							if( ! sent_header )
							{
								sent_header = true;
								Packet message( Raptor::Packet::MESSAGE );
								message.AddString( "FINAL SCORES:" );
								Net.SendAll( &message );
							}
							
							const Player *p = player_iter->second;
							std::string player_score = p->Name + std::string(": ");
							if( objective )
								player_score += Num::ToString(p->PropertyAsInt("kills_c")) + std::string(" objectives, ")
								              + Num::ToString(p->PropertyAsInt("kills"))   + std::string(" fighters, ")
								              + Num::ToString(p->PropertyAsInt("kills_t")) + std::string(" turrets, ");
							else
							{
								if( (GameType == XWing::GameType::TEAM_RACE) || (GameType == XWing::GameType::FFA_RACE) )
									player_score += Num::ToString(p->PropertyAsInt("score")) + std::string(" checkpoints, ");
								else if( GameType == XWing::GameType::CTF )
									player_score += Num::ToString(p->PropertyAsInt("score")) + std::string(" captures, ");
								player_score += Num::ToString(p->PropertyAsInt("kills")) + std::string(" kills, ");
							}
							player_score += Num::ToString(p->PropertyAsInt("deaths")) + std::string(" deaths");
							
							Packet message( Raptor::Packet::MESSAGE );
							message.AddString( player_score );
							Net.SendAll( &message );
						}
					}
				}
				
				// Determine which team is eligible for positive achievements.
				std::string eligible_team;
				if( dead_player_team || (EventTriggers.find( MissionEvent::TRIGGER_ON_VICTORY ) != EventTriggers.end()) )
					eligible_team = "Nobody";
				else if( (GameType == XWing::GameType::FLEET_BATTLE) || (GameType == XWing::GameType::TEAM_ELIMINATION) )
				{
					if( victor == XWing::Team::REBEL )
						eligible_team = "Rebel";
					else if( victor == XWing::Team::EMPIRE )
						eligible_team = "Empire";
					else
						eligible_team = "Nobody";
				}
				else if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
				{
					if( victor == XWing::Team::REBEL )
						eligible_team = "Rebel";
					else
						eligible_team = "Nobody";
				}
				else if( GameType == XWing::GameType::CAPITAL_SHIP_HUNT )
				{
					if( (victor == DefendingTeam) && ! Respawn )
						eligible_team = "Nobody";
					else if( victor == XWing::Team::REBEL )
						eligible_team = "Rebel";
					else if( victor == XWing::Team::EMPIRE )
						eligible_team = "Empire";
					else
						eligible_team = "Nobody";
				}
				else if( (GameType == XWing::GameType::TEAM_RACE) || (GameType == XWing::GameType::FFA_RACE) )
					eligible_team = "Nobody";
				
				// Check for achievements.
				bool death_star_destroyed = (GameType == XWing::GameType::BATTLE_OF_YAVIN) && (victor == XWing::Team::REBEL);
				int ai_waves = Data.PropertyAsInt("ai_waves");
				
				Data.Lock.Lock();
				
				for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
				{
					if( (ChosenTeam( player_iter->second ) == eligible_team) || eligible_team.empty() )
					{
						int kills   = player_iter->second->PropertyAsInt("kills");
						int kills_c = player_iter->second->PropertyAsInt("kills_c");
						int deaths  = player_iter->second->PropertyAsInt("deaths");
						int impressive_ratio = ai_hard ? 1 : (ai_easy ? (ai_pudu ? 9000 : 10) : 2);
						int impressive_kills = std::max<int>( impressive_ratio, ai_jedi ? 2 : 3 );
						if( death_star_destroyed && (kills_c >= 1) )
						{
							// Hit the Death Star exhaust port.
							Packet great_shot( XWing::Packet::ACHIEVEMENT );
							great_shot.AddString( "great_shot.wav" );
							Net.SendToPlayer( &great_shot, player_iter->first );
						}
						if( ((kills + kills_c) > deaths * impressive_ratio) && (kills >= impressive_kills) && (kills_c >= 0) && (ai_waves >= (((GameType == XWing::GameType::FFA_ELIMINATION) && ! deaths) ? 1 : 3)) && (Cheaters.find( player_iter->first ) == Cheaters.end()) )
						{
							// More kills than deaths vs Ace AI.
							Packet impressive( XWing::Packet::ACHIEVEMENT );
							impressive.AddString( "impressive.wav" );
							Net.SendToPlayer( &impressive, player_iter->first );
						}
						if( (! deaths) && ((kills + kills_c) >= (ai_jedi?1:2)) && (! Respawn) && (ai_skill >= (PlayersTakeEmptyShips?2:1)) && (Cheaters.find( player_iter->first ) == Cheaters.end()) )
						{
							// Survived without respawn.
							Packet powerful( XWing::Packet::ACHIEVEMENT );
							powerful.AddString( "powerful.wav" );
							Net.SendToPlayer( &powerful, player_iter->first );
						}
					}
					else if( (player_iter->second->PropertyAsInt("kills_c") < 0) && (EventTriggers.find( MissionEvent::TRIGGER_ON_DEFEAT ) == EventTriggers.end()) )
					{
						// On the losing team, with more friendly objectives killed than enemy.
						Packet failed( XWing::Packet::ACHIEVEMENT );
						failed.AddString( "failed.wav" );
						Net.SendToPlayer( &failed, player_iter->first );
					}
				}
				
				Data.Lock.Unlock();
			}
		}
		
		// If the round has been over long enough, return to the lobby.
		
		if( (State == XWing::State::ROUND_ENDED) && (RoundEndedTimer.ElapsedSeconds() > 9.) )
		{
			State = XWing::State::LOBBY;
			
			Packet lobby( XWing::Packet::LOBBY );
			Net.SendAll( &lobby );
			
			ResetToStartingObjects();
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
			if( remaining > 0 )
			{
				Packet message( Raptor::Packet::MESSAGE );
				message.AddString( (std::string("Launching in ") + Num::ToString(remaining) + std::string("...")).c_str() );
				Net.SendAll( &message );
				
				if( remaining == (CountdownFrom - 1) )
				{
					Packet rebel_sound( Raptor::Packet::PLAY_SOUND ), empire_sound( Raptor::Packet::PLAY_SOUND );
					if( (Data.PropertyAsInt("ai_skill",1) >= 4) && (Data.PropertyAsInt("ai_waves") >= 3) )
					{
						rebel_sound.AddString("good_luck.wav");
						empire_sound.AddString("good_luck.wav");
					}
					else
					{
						rebel_sound.AddString("rebel_start.wav");
						empire_sound.AddString("empire_start.wav");
					}
					
					uint8_t player_team = XWing::Team::NONE;
					if( Data.PropertyAsString("gametype") == "mission" )
					{
						std::map<std::string,Mission>::const_iterator mission_iter = Missions.find( Data.PropertyAsString("mission") );
						if( mission_iter != Missions.end() )
						{
							std::map<std::string,std::string>::const_iterator property_iter = mission_iter->second.Properties.find("player_team");
							if( property_iter != mission_iter->second.Properties.end() )
							{
								if( Str::EqualsInsensitive( property_iter->second, "rebel" ) )
									player_team = XWing::Team::REBEL;
								else if( Str::EqualsInsensitive( property_iter->second, "empire" ) )
									player_team = XWing::Team::EMPIRE;
							}
						}
					}
					
					Data.Lock.Lock();
					
					for( std::map<uint16_t,Player*>::const_iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
					{
						uint8_t team = player_team;
						if( ! team )
						{
							const ShipClass *ship_class = GetShipClass( ChosenShip( player_iter->second ) );
							if( ship_class )
								team = ship_class->Team;
						}
						if( team == XWing::Team::EMPIRE )
							Net.SendToPlayer( &empire_sound, player_iter->first );
						else
							Net.SendToPlayer( &rebel_sound, player_iter->first );
					}
					
					Data.Lock.Unlock();
				}
			}
			else
				BeginFlying();
		}
	}
}


void XWingServer::SetProperty( std::string name, std::string value )
{
	if( name == "gametype" )
	{
		if( (value != "mission") && (Data.PropertyAsString("gametype") == "mission") )
		{
			// Exiting mission mode.
			
			// Determine which properties to restore.
			std::map<std::string,std::string> properties;
			for( std::map<std::string,std::string>::const_iterator property_iter = Properties.begin(); property_iter != Properties.end(); property_iter ++ )
			{
				if( (property_iter->first != "gametype") && ! Str::BeginsWith( property_iter->first, "mission" ) )
					properties[ property_iter->first ] = property_iter->second;
			}
			properties[ "player_team" ] = "";
			properties[ "player_ship" ] = "";
			properties[ "mission_objs" ] = "";
			Properties.clear();
			
			// Set and send restored properties.
			Packet info( Raptor::Packet::INFO );
			info.AddUShort( properties.size() );
			for( std::map<std::string,std::string>::const_iterator property_iter = properties.begin(); property_iter != properties.end(); property_iter ++ )
			{
				SetProperty( property_iter->first, property_iter->second );
				
				info.AddString( property_iter->first );
				info.AddString( property_iter->second );
			}
			Net.SendAll( &info );
			
			RaptorServer::SetProperty( name, value );
		}
		else
		{
			RaptorServer::SetProperty( name, value );
			
			if( value == "mission" )
				SelectMission();
		}
	}
	else if( name == "mission" )
		SelectMission( value );
	else if( name == "mission_opts" )
	{
		RaptorServer::SetProperty( name, value );
		SelectMission();
	}
	else if( name == "mission_objs" )
	{
		RaptorServer::SetProperty( name, value );
		
		Packet info( Raptor::Packet::INFO );
		info.AddUShort( 1 );
		info.AddString( name );
		info.AddString( value );
		Net.SendAll( &info );
	}
	else if( name == "campaign" )
	{
		RaptorServer::SetProperty( name, value );
		
		std::string mission_id = Data.PropertyAsString("mission");
		if( ! Str::BeginsWith( mission_id, value ) )
		{
			mission_id = value + std::string("1");
			mission_id = Raptor::Game->Cfg.SettingAsString( value + std::string("_mission"), mission_id.c_str(), mission_id.c_str() ); // This reach across threads should be safe because of locking.
			std::string ai_skill = Data.PropertyAsString( "ai_skill", "1", "0" );
			RaptorServer::SetProperty( "ai_skill", Raptor::Game->Cfg.SettingAsString( value + std::string("_difficulty"), ai_skill.c_str(), ai_skill.c_str() ) ); // Ditto here.
		}
		
		if( Missions.find(mission_id) != Missions.end() )
		{
			RaptorServer::SetProperty( "mission", mission_id );
			SetProperty( "gametype", "mission" );  // This calls SelectMission (even if the gametype was already mission).
		}
	}
	else
		RaptorServer::SetProperty( name, value );
}


void XWingServer::ResetToDefaultProperties( void )
{
	std::map<std::string,std::string> defaults = DefaultProperties();
	std::map<std::string,std::string> keep;
	
	Data.Lock.Lock();
	
	if( Data.HasProperty("name") )
		keep["name"]        = Data.PropertyAsString("name");
	if( Data.HasProperty("permissions") )
		keep["permissions"] = Data.PropertyAsString("permissions");
	if( Data.HasProperty("gametype") )
		keep["gametype"]    = Data.PropertyAsString("gametype");
	if( Data.HasProperty("mission") )
		keep["mission"]     = Data.PropertyAsString("mission");
	
	Data.Properties = defaults;
	
	for( std::map<std::string,std::string>::const_iterator keep_iter = keep.begin(); keep_iter != keep.end(); keep_iter ++ )
		Data.Properties[ keep_iter->first ] = keep_iter->second;
	
	if( Data.Properties.size() )
	{
		Packet info( Raptor::Packet::INFO );
		info.AddUShort( Data.Properties.size() );  // Count
		for( std::map<std::string,std::string>::const_iterator property_iter = Data.Properties.begin(); property_iter != Data.Properties.end(); property_iter ++ )
		{
			info.AddString( property_iter->first );
			info.AddString( property_iter->second );
		}
		Net.SendAll( &info );
	}
	
	Data.Lock.Unlock();
}


void XWingServer::ResetToStartingObjects( void )
{
	Data.ClearObjects();
	
	Packet objects_clear( Raptor::Packet::OBJECTS_CLEAR );
	Net.SendAll( &objects_clear );
	
	// Restore ship classes.
	Packet objects_add( Raptor::Packet::OBJECTS_ADD );
	objects_add.AddUInt( ShipClasses.size() );
	for( std::set<ShipClass>::iterator class_iter = ShipClasses.begin(); class_iter != ShipClasses.end(); class_iter ++ )
	{
		ShipClass *copy = new ShipClass( *class_iter );
		Data.AddObject( copy );
		
		objects_add.AddUInt( copy->ID );
		objects_add.AddUInt( copy->Type() );
		copy->AddToInitPacket( &objects_add );
	}
	Net.SendAll( &objects_add );
	
	ClearMissionData();
}


void XWingServer::ClearMissionData( void )
{
	Data.Lock.Lock();
	
	// Clear assigned teams.
	for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
	{
		SetPlayerProperty( player_iter->second, "team", "", true );
		SetPlayerProperty( player_iter->second, "ship", player_iter->second->PropertyAsString("ship") );  // Re-assign team in lobby.
	}
	
	// Restore any settings a mission might have messed with.
	if( Data.PropertyAsString("gametype") == "mission" )
	{
		std::string next_mission = Data.PropertyAsString("next_mission");
		Data.Properties = Properties;
		Properties.clear();
		EventTriggers.clear();
		
		if( ! next_mission.empty() )
			SelectMission( next_mission );
	}
	
	// Restore default time scale.
	Data.Properties["time_scale"] = "1";
	
	// Resend all properties.
	Packet info( Raptor::Packet::INFO );
	info.AddUShort( Data.Properties.size() );
	for( std::map<std::string,std::string>::const_iterator property_iter = Data.Properties.begin(); property_iter != Data.Properties.end(); property_iter ++ )
	{
		info.AddString( property_iter->first );
		info.AddString( property_iter->second );
	}
	Net.SendAll( &info );
	
	Data.Lock.Unlock();
}


bool XWingServer::SelectMission( std::string mission_id, bool load )
{
	if( mission_id.empty() )
		mission_id = Data.PropertyAsString("mission");
	if( mission_id.empty() )
		return false;
	
	// Find or load the selected mission.
	std::map<std::string,Mission>::iterator mission_iter = Missions.find( mission_id );
	const Mission *mission = (mission_iter != Missions.end()) ? &(mission_iter->second) : NULL;
	if( load && (! mission) && mission_id.length() && (mission_id.at(0) != '.') && (mission_id.at(0) != '/') )
	{
		Mission m;
		if( m.Load( std::string("Missions/") + mission_id + std::string(".def") ) )
		{
			Missions[ mission_id ] = m;
			mission_iter = Missions.find( mission_id );
			mission = &(mission_iter->second);
		}
	}
	
	bool mission_mode = mission && (Data.PropertyAsString("gametype") == "mission") && (State < XWing::State::FLYING);
	std::map<std::string,std::string> properties;
	properties[ "mission"      ] = mission_id;
	properties[ "mission_name" ] = mission_id;
	properties[ "mission_desc" ] = "";
	properties[ "mission_opts" ] = "";
	properties[ "mission_objs" ] = "";
	
	if( mission )
	{
		// Send any basic mission info.
		for( std::map<std::string,std::string>::const_iterator mission_property_iter = mission->Properties.begin(); mission_property_iter != mission->Properties.end(); mission_property_iter ++ )
		{
			if( Str::BeginsWith( mission_property_iter->first, "player_" ) || (mission_property_iter->first == "mission_objs") )
				properties[ mission_property_iter->first ] = mission_mode ? mission_property_iter->second : "";
			else if( Str::BeginsWith( mission_property_iter->first, "mission_" ) || (mission_property_iter->first == "bg") )
				properties[ mission_property_iter->first ] = mission_property_iter->second;
		}
		
		if( mission_mode )
		{
			// If in mission mode, send mission default values for customizable options.
			std::vector<std::string> opts = Str::SplitToVector( properties["mission_opts"], " ," );
			for( std::vector<std::string>::const_iterator opt_iter = opts.begin(); opt_iter != opts.end(); opt_iter ++ )
			{
				std::map<std::string,std::string>::const_iterator mission_property_iter = mission->Properties.find( *opt_iter );
				if( mission_property_iter != mission->Properties.end() )
					properties[ *opt_iter ] = mission_property_iter->second;
			}
			
			// If hosting a campaign, match it to the selected mission.
			if( Data.PropertyAsString("campaign").length() )
			{
				if( Str::BeginsWith( mission_id, "rebel" ) )
					properties[ "campaign" ] = "rebel";
				else if( Str::BeginsWith( mission_id, "empire" ) )
					properties[ "campaign" ] = "empire";
				else
					properties[ "campaign" ] = "";  // Exit campaign mode.
			}
		}
	}
	
	if( properties.size() )
	{
		Packet info( Raptor::Packet::INFO );
		info.AddUShort( properties.size() );
		for( std::map<std::string,std::string>::const_iterator property_iter = properties.begin(); property_iter != properties.end(); property_iter ++ )
		{
			// Keep non-mission values tucked away.
			// FIXME: Find someplace else!  The entire Properties structure is overwritten when starting a mission!
			if( Properties.find( property_iter->first ) == Properties.end() )
				Properties[ property_iter->first ] = Data.PropertyAsString( property_iter->first );
			
			Data.SetProperty( property_iter->first, property_iter->second );
			
			info.AddString( property_iter->first );
			info.AddString( property_iter->second );
		}
		Net.SendAll( &info );
	}
	
	return mission;
}


void XWingServer::ToggleCountdown( const Player *player )
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
		message.AddString( player ? (std::string("Launch sequence aborted by ") + player->Name + std::string(".")) : "Launch sequence aborted." );
		Net.SendAll( &message );
		
		State = XWing::State::LOBBY;
	}
}


uint32_t XWingServer::ParseGameType( std::string gametype ) const
{
	if( Str::EqualsInsensitive( gametype, "team_elim" ) )
		return XWing::GameType::TEAM_ELIMINATION;
	else if( Str::EqualsInsensitive( gametype, "ffa_elim" ) )
		return XWing::GameType::FFA_ELIMINATION;
	else if( Str::EqualsInsensitive( gametype, "team_dm" ) )
		return XWing::GameType::TEAM_DEATHMATCH;
	else if( Str::EqualsInsensitive( gametype, "ffa_dm" ) )
		return XWing::GameType::FFA_DEATHMATCH;
	else if( Str::EqualsInsensitive( gametype, "team_race" ) )
		return XWing::GameType::TEAM_RACE;
	else if( Str::EqualsInsensitive( gametype, "ffa_race" ) )
		return XWing::GameType::FFA_RACE;
	else if( Str::EqualsInsensitive( gametype, "yavin" ) )
		return XWing::GameType::BATTLE_OF_YAVIN;
	else if( Str::EqualsInsensitive( gametype, "hunt" ) || Str::EqualsInsensitive( gametype, "defend" ) )
		return XWing::GameType::CAPITAL_SHIP_HUNT;
	else if( Str::EqualsInsensitive( gametype, "fleet" ) )
		return XWing::GameType::FLEET_BATTLE;
	
	return XWing::GameType::UNDEFINED;
}


void XWingServer::BeginFlying( uint16_t player_id, bool respawn )
{
	// Don't allow someone to hit Fly while the round is ending.
	if( State > XWing::State::FLYING )
		return;
	
	if( State < XWing::State::FLYING )
	{
		// Starting a new round.
		
		GameType = XWing::GameType::TEAM_ELIMINATION;
		PlayerTeam = XWing::Team::NONE;
		Respawn = false;
		RespawnDelay = 10.;
		PlayersTakeEmptyShips = false;
		bool ffa = false;
		KillLimit = 0;
		TimeLimit = 0;
		Checkpoints = 0;
		CheckpointFirstTouch = false;
		RespawnClocks.clear();
		Alerts.clear();
		EventTriggers.clear();
		Waypoints.clear();
		Squadrons.clear();
		GroupStagger.clear();
		GroupJumpingIn.clear();
		TeamScores.clear();
		ShipScores.clear();
		Cheaters.clear();
		
		Data.Lock.Lock();
		// Keep a copy of Properties so we can restore anything a mission might change.
		// FIXME: This should restore properties like "ffa_kill_limit" to previous non-mission values, but it doesn't work!
		Properties = Data.Properties;
		Data.Lock.Unlock();
		
		std::string gametype = Data.PropertyAsString("gametype");
		
		if( gametype == "mission" )
		{
			std::map<std::string,Mission>::const_iterator mission_iter = Missions.find( Data.PropertyAsString("mission") );
			if( mission_iter != Missions.end() )
			{
				std::map<std::string,std::string> send_info;
				send_info[ "bg" ] = mission_iter->second.PropertyAsString( "bg", "stars" );
				
				// Keep any customized values.
				std::vector<std::string> opts = Str::SplitToVector( Data.PropertyAsString("mission_opts"), " ," );
				for( std::vector<std::string>::const_iterator opt_iter = opts.begin(); opt_iter != opts.end(); opt_iter ++ )
				{
					if( Data.HasProperty(*opt_iter) )
						send_info[ *opt_iter ] = Data.PropertyAsString(*opt_iter);
				}
				
				// Set defaults for mission mode.
				Data.SetProperty( "respawn",           "coop" );
				Data.SetProperty( "respawn_time",      "" );
				Data.SetProperty( "allow_ship_change", "false" );
				Data.SetProperty( "allow_team_change", "false" );
				Data.SetProperty( "player_group",      "0" );
				Data.SetProperty( "rebel_fighter",     "X/W" );
				Data.SetProperty( "rebel_bomber",      "Y/W" );
				Data.SetProperty( "rebel_cruiser",     "CRV" );
				Data.SetProperty( "rebel_flagship",    "FRG" );
				Data.SetProperty( "empire_fighter",    "T/F" );
				Data.SetProperty( "empire_bomber",     "T/B" );
				Data.SetProperty( "empire_cruiser",    "INT" );
				Data.SetProperty( "empire_flagship",   "ISD" );
				Data.SetProperty( "rebel_cruisers",    "0" );
				Data.SetProperty( "empire_cruisers",   "0" );
				Data.SetProperty( "rebel_frigates",    "0" );
				Data.SetProperty( "empire_frigates",   "0" );
				Data.SetProperty( "hunt_time_limit",   "0" );
				Data.SetProperty( "yavin_time_limit",  "15" );
				Data.SetProperty( "tdm_kill_limit",    "0" );
				Data.SetProperty( "tm_kill_limit",     "0" );
				Data.SetProperty( "ai_flock",          "false" );
				Data.SetProperty( "asteroids",         "0" );
				Data.SetProperty( "bg",                "stars" );
				
				// Make sure special mission variables are clear.
				Data.SetProperty( "victor", "" );
				
				// Load any mission properties.
				for( std::map<std::string,std::string>::const_iterator property_iter = mission_iter->second.Properties.begin(); property_iter != mission_iter->second.Properties.end(); property_iter ++ )
					Data.SetProperty( property_iter->first, property_iter->second );
				
				// Replace mission defaults with any customized options, and send critical need-to-know information to clients.
				Packet info( Raptor::Packet::INFO );
				info.AddUShort( send_info.size() );
				for( std::map<std::string,std::string>::const_iterator info_iter = send_info.begin(); info_iter != send_info.end(); info_iter ++ )
				{
					Data.SetProperty( info_iter->first, info_iter->second );
					info.AddString( info_iter->first );
					info.AddString( info_iter->second );
				}
				Net.SendAll( &info );
			}
			
			// Use the mission's gametype for setup.
			gametype = Data.PropertyAsString("gametype");
			
			// Set triggers for mission events.
			for( std::vector<MissionEvent>::const_iterator event_iter = mission_iter->second.Events.begin(); event_iter != mission_iter->second.Events.end(); event_iter ++ )
				EventTriggers[ event_iter->Trigger ].push_back( *event_iter );
		}
		
		std::string player_team = (gametype == "mission") ? Data.PropertyAsString("player_team") : "";
		if( Str::EqualsInsensitive( player_team, "rebel" ) )
			PlayerTeam = XWing::Team::REBEL;
		else if( Str::EqualsInsensitive( player_team, "empire" ) )
			PlayerTeam = XWing::Team::EMPIRE;
		
		AllowShipChange = Data.PropertyAsBool("allow_ship_change",true);
		AllowTeamChange = Data.PropertyAsBool("allow_team_change",false);
		
		GameType = ParseGameType(gametype);
		
		if( GameType == XWing::GameType::TEAM_ELIMINATION )
		{
			PlayersTakeEmptyShips = Data.PropertyAsBool("respawn");
		}
		else if( GameType == XWing::GameType::FFA_ELIMINATION )
		{
			ffa = true;
			AllowTeamChange = true;
		}
		else if( GameType == XWing::GameType::TEAM_DEATHMATCH )
		{
			Respawn = true;
			KillLimit = Data.PropertyAsInt("tdm_kill_limit");
			RespawnDelay = Data.PropertyAsDouble("dm_respawn",5.);
		}
		else if( GameType == XWing::GameType::FFA_DEATHMATCH )
		{
			ffa = true;
			AllowTeamChange = true;
			Respawn = true;
			KillLimit = Data.PropertyAsInt("dm_kill_limit");
			RespawnDelay = Data.PropertyAsDouble("dm_respawn",5.);
		}
		else if( GameType == XWing::GameType::TEAM_RACE )
		{
			AllowTeamChange = false;
			Checkpoints = Data.PropertyAsInt("team_race_checkpoints",100);
			CheckpointFirstTouch = ! Data.PropertyAsBool("race_circuit");
			TimeLimit = Data.PropertyAsInt("race_time_limit");
			Respawn = Data.PropertyAsBool("respawn");
			//PlayersTakeEmptyShips = ! Respawn;  // FIXME: Should this be a game option?
			RespawnDelay = Data.PropertyAsDouble("race_respawn",10.);
		}
		else if( GameType == XWing::GameType::FFA_RACE )
		{
			ffa = true;
			AllowTeamChange = true;
			Checkpoints = Data.PropertyAsInt("ffa_race_checkpoints",30);
			CheckpointFirstTouch = ! Data.PropertyAsBool("race_circuit");
			TimeLimit = Data.PropertyAsInt("race_time_limit");
			Respawn = Data.PropertyAsBool("respawn");
			RespawnDelay = Data.PropertyAsDouble("race_respawn",10.);
		}
		else if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
		{
			TimeLimit = Data.PropertyAsInt("yavin_time_limit");
			Respawn = Data.PropertyAsBool("respawn");
			RespawnDelay = Data.PropertyAsDouble("yavin_respawn",15.);
			
			if( TimeLimit >= 30 )
				Alerts[ 30. * 60. ] = XWingServerAlert( "deathstar_30min.wav", "The moon with the Rebel base will be in range in 30 minutes." );
			if( TimeLimit >= 15 )
				Alerts[ 15. * 60. ] = XWingServerAlert( "deathstar_15min.wav", "Stand-by alert: Death Star approaching.  Estimated time to firing range: 15 minutes." );
			if( TimeLimit >= 7 )
				Alerts[ 7. * 60. ] = XWingServerAlert( "deathstar_7min.wav", "The Rebel base will be in firing range in 7 minutes." );
			if( TimeLimit >= 5 )
				Alerts[ 5. * 60. ] = XWingServerAlert( "deathstar_5min.wav", "Death Star will be in range in 5 minutes." );
			if( TimeLimit >= 3 )
				Alerts[ 3. * 60. ] = XWingServerAlert( "deathstar_3min.wav", "Rebel base 3 minutes and closing." );
			
			Alerts[ 60. ] = XWingServerAlert( "deathstar_1min.wav", "Rebel base 1 minute and closing." );
			Alerts[ 30. ] = XWingServerAlert( "deathstar_30sec.wav", "Rebel base 30 seconds and closing." );
			Alerts[ 0. ] = XWingServerAlert( "deathstar_0sec.wav", "The Death Star has cleared the planet!  The Death Star has cleared the planet!  Rebel base in range." );
		}
		else if( GameType == XWing::GameType::CAPITAL_SHIP_HUNT )
		{
			TimeLimit = Data.PropertyAsInt("hunt_time_limit");
			Respawn = Data.PropertyAsBool("respawn");
			DefendingTeam = (Data.PropertyAsString("defending_team") == "rebel") ? XWing::Team::REBEL : XWing::Team::EMPIRE;
			RespawnDelay = Data.PropertyAsDouble("hunt_respawn",15.);
		}
		else if( GameType == XWing::GameType::FLEET_BATTLE )
		{
			Respawn = Data.PropertyAsBool("respawn");
			RespawnDelay = Data.PropertyAsDouble("fleet_respawn",15.);
			// FIXME: Should respawn delay for cruisers and frigates be based on something like health/radius?
			RebelCruiserRespawn  = 85. + 5. * Data.PropertyAsInt("rebel_cruisers")  + 7. * Data.PropertyAsInt("rebel_frigates");
			EmpireCruiserRespawn = 85. + 5. * Data.PropertyAsInt("empire_cruisers") + 7. * Data.PropertyAsInt("empire_frigates");
			RebelCruiserRespawn  = Data.PropertyAsDouble( "cruiser_respawn",        RebelCruiserRespawn,  RebelCruiserRespawn );
			EmpireCruiserRespawn = Data.PropertyAsDouble( "cruiser_respawn",        EmpireCruiserRespawn, EmpireCruiserRespawn );
			RebelCruiserRespawn  = Data.PropertyAsDouble( "rebel_cruiser_respawn",  RebelCruiserRespawn,  RebelCruiserRespawn );
			EmpireCruiserRespawn = Data.PropertyAsDouble( "empire_cruiser_respawn", EmpireCruiserRespawn, EmpireCruiserRespawn );
		}
		
		RespawnDelay = Data.PropertyAsDouble( "respawn_time", RespawnDelay, RespawnDelay );
		
		// Restore gametype "mission" if we changed it above.
		if( gametype != Properties["gametype"] )
			Data.SetProperty( "gametype", Properties["gametype"] );
		
		int ai_skill = Data.PropertyAsInt("ai_skill",1);
		
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
			deathstar->TrenchWidth = Data.PropertyAsDouble( "yavin_trench_width", deathstar->TrenchWidth, deathstar->TrenchWidth );
			deathstar->TrenchDepth = Data.PropertyAsDouble( "yavin_trench_depth", deathstar->TrenchDepth, deathstar->TrenchDepth );
			Data.AddObject( deathstar );
			
			// Allow server variable to set distance to exhaust port.
			double trench_length = Data.PropertyAsDouble("yavin_dist",15000.);
			
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
			Ship *exhaust_port = SpawnShip( GetShipClass("Exhaust"), XWing::Team::EMPIRE );
			exhaust_port->Copy( exhaust_box );
			exhaust_port->IsMissionObjective = true;
			exhaust_port->CanRespawn = false;
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
				turret->SingleShotDelay = Rand::Double( 0.4, 0.5 ) - (ai_skill - 1) * 0.05;
				Data.AddObject( turret );
			}
			
			// Surface turrets.
			int yavin_turrets = Data.PropertyAsInt("yavin_turrets");
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
				turret->MinGunPitch = -5.;
				turret->SingleShotDelay = 0.5 - (ai_skill - 1) * 0.05;
				turret->AimAhead = 1.5f;
				Data.AddObject( turret );
			}
			
			// Repair bay.
			if( Data.PropertyAsBool("yavin_repair_bay",true) )
			{
				DockingBay *dock = new DockingBay();
				dock->Copy( deathstar );
				dock->MoveAlong( &(deathstar->Up), 32. );
				dock->MoveAlong( &(deathstar->Fwd), -3979. );
				dock->MoveAlong( &(deathstar->Right), -399. );
				dock->Radius = 36.;
				dock->Team = XWing::Team::EMPIRE;
				Data.AddObject( dock );
				
				DeathStarBox *box1 = new DeathStarBox();
				box1->Copy( dock );
				box1->W = 4.;
				box1->L = 48.;
				box1->H = 64.;
				box1->MoveAlong( &(deathstar->Right), 32. );
				Data.AddObject( box1 );
				
				DeathStarBox *box2 = new DeathStarBox();
				box2->Copy( dock );
				box2->W = 4.;
				box2->L = 48.;
				box2->H = 64.;
				box2->MoveAlong( &(deathstar->Right), -32. );
				Data.AddObject( box2 );
				
				DeathStarBox *box3 = new DeathStarBox();
				box3->Copy( dock );
				box3->W = 60.;
				box3->L = 20.;
				box3->H = 4.;
				box3->MoveAlong( &(deathstar->Up), 30. );
				Data.AddObject( box3 );
				
				Turret *turret = new Turret();
				turret->Team = XWing::Team::EMPIRE;
				turret->Copy( box3 );
				turret->MoveAlong( &(box3->Up), box3->H / 2. );
				turret->MinGunPitch = -10.;
				Data.AddObject( turret );
			}
			
			// Trench bottom turrets.
			for( double fwd = -6450.; fwd < (trench_length - 699.9); fwd += 900. )
			{
				Turret *turret = new Turret();
				turret->Copy( deathstar );
				turret->Team = XWing::Team::EMPIRE;
				turret->Health = 70.;
				turret->SingleShotDelay = Rand::Double( 1., 2. ) - (ai_skill * 0.1);
				turret->FiringMode = 1;
				turret->AimAhead = 3.f;
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
			for( double fwd = -6200.; fwd < (trench_length - 499.9); fwd += 900. )
			{
				Turret *turret = new Turret();
				turret->Copy( deathstar );
				turret->Team = XWing::Team::EMPIRE;
				turret->Health = 70.;
				turret->SingleShotDelay = Rand::Double( 1.5, 2.5 ) - (ai_skill * 0.25);
				turret->FiringMode = 1;
				turret->AimAhead = 2.75f;
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
				
				// Randomize projection direction without any immediate repeats.
				int box_type = Rand::Int( 0, 2 );
				if( box_type >= prev_box_type )
					box_type ++;
				prev_box_type = box_type;
				
				if( box_type == 0 )
				{
					// Across the bottom.
					box->H = (deathstar->TrenchDepth - 20.) * Rand::Double( 0.15, 0.3 );
					box->W = deathstar->TrenchWidth;
					box->L = box->H / 2.;
					box->MoveAlong( &(deathstar->Up), box->H / 2. - deathstar->TrenchDepth );
					
					waypoint.Copy( box );
					waypoint.MoveAlong( &(deathstar->Up), deathstar->TrenchDepth / 2. );
					Waypoints[ 0 ].push_back( waypoint );
				}
				else if( box_type == 1 )
				{
					// Across the top.
					box->H = (deathstar->TrenchDepth - 20.) * Rand::Double( 0.2, 0.4 );
					box->W = deathstar->TrenchWidth;
					box->L = box->H / 2.;
					box->MoveAlong( &(deathstar->Up), box->H / -2. );
					
					waypoint.Copy( box );
					waypoint.MoveAlong( &(deathstar->Up), deathstar->TrenchDepth / -2. );
					Waypoints[ 0 ].push_back( waypoint );
				}
				else if( box_type == 2 )
				{
					// Across the left.
					box->W = (deathstar->TrenchWidth - 20.) * Rand::Double( 0.45, 0.6 );
					box->H = deathstar->TrenchDepth;
					box->L = box->W / 2.;
					box->MoveAlong( &(deathstar->Right), (box->W - deathstar->TrenchWidth) / 2. );
					box->MoveAlong( &(deathstar->Up), box->H / 2. - deathstar->TrenchDepth );
					
					waypoint.Copy( box );
					waypoint.MoveAlong( &(deathstar->Right), deathstar->TrenchWidth / 2. );
					Waypoints[ 0 ].push_back( waypoint );
				}
				else if( box_type == 3 )
				{
					// Across the right.
					box->W = (deathstar->TrenchWidth - 20.) * Rand::Double( 0.45, 0.6 );
					box->H = deathstar->TrenchDepth;
					box->L = box->W / 2.;
					box->MoveAlong( &(deathstar->Right), (deathstar->TrenchWidth - box->W) / 2. );
					box->MoveAlong( &(deathstar->Up), box->H / 2. - deathstar->TrenchDepth );
					
					waypoint.Copy( box );
					waypoint.MoveAlong( &(deathstar->Right), deathstar->TrenchWidth / -2. );
					Waypoints[ 0 ].push_back( waypoint );
				}
				Data.AddObject( box );	
			}
			
			// Attack.
			waypoint.Copy( exhaust_port );
			waypoint.MoveAlong( &(deathstar->Fwd), -200. );
			waypoint.MoveAlong( &(deathstar->Up), 2. );
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
			waypoint.MoveAlong( &(deathstar->Fwd), -1400. );
			waypoint.MoveAlong( &(deathstar->Up), -500. );
			Waypoints[ 0 ].push_back( waypoint );
			waypoint.MoveAlong( &(deathstar->Fwd), -1800. );
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
			
			// Copy course with adjustments to avoid collisions between groups.
			for( size_t i = 0; i < Waypoints[ 0 ].size(); i ++ )
			{
				Waypoints[ 1 ].push_back( Waypoints[ 0 ][ i ] );
				Waypoints[ 2 ].push_back( Waypoints[ 0 ][ i ] );
				Waypoints[ 1 ][ i ].MoveAlong( &(deathstar->Up), 5. );
				Waypoints[ 2 ][ i ].MoveAlong( &(deathstar->Up), -5. );
			}
			
			// Add long distance approach paths for mission mode.
			for( double fwd = -9999.; fwd < 0.; fwd += 1000. )
			{
				waypoint.Copy( deathstar );
				waypoint.MoveAlong( &(deathstar->Fwd), fwd );
				waypoint.MoveAlong( &(deathstar->Up), 150. );
				Waypoints[ 0 ].push_back( waypoint );
				waypoint.MoveAlong( &(deathstar->Up), 100. );
				waypoint.MoveAlong( &(deathstar->Right), (fwd + 1000.) * 0.05 );
				Waypoints[ 1 ].push_back( waypoint );
				waypoint.MoveAlong( &(deathstar->Up), -30. );
				waypoint.MoveAlong( &(deathstar->Right), (fwd + 1000.) * -0.1 );
				Waypoints[ 2 ].push_back( waypoint );
			}
		}
		else
		{
			// Add asteroids to the level.
			int num_asteroids = Data.PropertyAsInt("asteroids");
			if( (GameType == XWing::GameType::TEAM_RACE) || (GameType == XWing::GameType::FFA_RACE) )
				num_asteroids = std::min<int>( num_asteroids * 10, num_asteroids * 2 + 1000 ); // Kessel Run needs more asteroids than other modes.
			std::set<Asteroid*> asteroids;
			for( int i = 0; i < num_asteroids; i ++ )
			{
				Asteroid *asteroid = new Asteroid();
				asteroid->X = Rand::Double( -1000., 1000. );
				asteroid->Y = Rand::Double( -1000., 1000. );
				asteroid->Z = Rand::Double( -1000., 1000. );
				if( (fabs(asteroid->X) > 500.) || (fabs(asteroid->Y) > 500.) )
					asteroid->Z *= 0.5;
				for( std::set<Asteroid*>::iterator asteroid_iter = asteroids.begin(); asteroid_iter != asteroids.end(); asteroid_iter ++ )
				{
					// If two asteroids spawn intersecting, they should not rotate.
					if( (*asteroid_iter)->Dist( asteroid ) < (asteroid->Radius + (*asteroid_iter)->Radius) )
					{
						asteroid->RollRate = asteroid->PitchRate = asteroid->YawRate = 0.;
						(*asteroid_iter)->RollRate = (*asteroid_iter)->PitchRate = (*asteroid_iter)->YawRate = 0.;
					}
				}
				Data.AddObject( asteroid );
				asteroids.insert( asteroid );
			}
			
			// Add hunt target ships.
			if( GameType == XWing::GameType::CAPITAL_SHIP_HUNT )
			{
				if( DefendingTeam == XWing::Team::EMPIRE )
				{
					std::string empire_flagship_class = Data.PropertyAsString("empire_flagship");
					Ship *ship = SpawnShip( GetShipClass(empire_flagship_class), DefendingTeam );
					ship->IsMissionObjective = true;
					ship->CanRespawn = false;
					ship->X = Rand::Double( -3500., -2500. );
					ship->Y = Rand::Double( -500., 500. );
					ship->Z = Rand::Double( -1000., -500. );
					ship->SetFwdVec( 1., 0., 0. );
					ship->SetUpVec( 0., 0., 1. );
					ship->ResetTurrets();
					ship->Health *= Data.PropertyAsDouble( "empire_flagship_hull", 1., 1. );
					double shield_scale = Data.PropertyAsDouble( "empire_flagship_shields", 1., 1. );
					ship->ShieldF *= shield_scale;
					ship->ShieldR *= shield_scale;
					const char *empire_flagship_cstr = empire_flagship_class.c_str();
					ship->Name = Data.PropertyAsString("empire_flagship_name");
					if( ship->Name.length() )
						;
					else if( strcasecmp( empire_flagship_cstr, "ISD" ) == 0 )
						ship->Name = "Devastator";
					else if( strcasecmp( empire_flagship_cstr, "ISD2" ) == 0 )
						ship->Name = "Eviscerator";
					else if( strcasecmp( empire_flagship_cstr, "FRG" ) == 0 )
						ship->Name = "Warspite";
					else if( strcasecmp( empire_flagship_cstr, "INT" ) == 0 )
						ship->Name = "Black Asp";
					else if( strcasecmp( empire_flagship_cstr, "VSD" ) == 0 )
						ship->Name = "Imperator";
					else if( strcasecmp( empire_flagship_cstr, "VSD2" ) == 0 )
						ship->Name = "Warlord";
					else if( strcasecmp( empire_flagship_cstr, "T/A" ) == 0 )
						ship->Name = "Black 1";
					else if( ship->Category() == ShipClass::CATEGORY_TRANSPORT )
						ship->Name = "Imperial VIP";
					else if( ship->Category() != ShipClass::CATEGORY_CAPITAL )
						ship->Name = "Omega Leader";
					else
						ship->Name = "Empire Flagship";
				}
				else
				{
					std::string rebel_flagship_class = Data.PropertyAsString("rebel_flagship");
					Ship *ship = SpawnShip( GetShipClass(rebel_flagship_class), DefendingTeam );
					ship->IsMissionObjective = true;
					ship->CanRespawn = false;
					ship->X = Rand::Double( 2500., 3500. );
					ship->Y = Rand::Double( -500., 500. );
					ship->Z = Rand::Double( -1000., -500. );
					ship->SetFwdVec( -1., 0., 0. );
					ship->SetUpVec( 0., 0., 1. );
					ship->ResetTurrets();
					ship->Health *= Data.PropertyAsDouble( "rebel_flagship_hull", 1., 1. );
					double shield_scale = Data.PropertyAsDouble( "rebel_flagship_shields", 1., 1. );
					ship->ShieldF *= shield_scale;
					ship->ShieldR *= shield_scale;
					const char *rebel_flagship_cstr = rebel_flagship_class.c_str();
					ship->Name = Data.PropertyAsString("rebel_flagship_name");
					if( ship->Name.length() )
						;
					else if( strcasecmp( rebel_flagship_cstr, "CRV" ) == 0 )
						ship->Name = "Tantive IV";
					else if( strcasecmp( rebel_flagship_cstr, "FRG" ) == 0 )
						ship->Name = "Redemption";
					else if( strcasecmp( rebel_flagship_cstr, "CRS" ) == 0 )
						ship->Name = "Independence";
					else if( strcasecmp( rebel_flagship_cstr, "YT1300" ) == 0 )
						ship->Name = "M.Falcon";
					else if( ship->Category() == ShipClass::CATEGORY_TRANSPORT )
						ship->Name = "Rebel VIP";
					else if( ship->Category() != ShipClass::CATEGORY_CAPITAL )
						ship->Name = "Rogue Leader";
					else
						ship->Name = "Rebel Flagship";
				}
			}
			
			// Add Fleet Battle base ships and their circular route.
			else if( GameType == XWing::GameType::FLEET_BATTLE )
			{
				double path_radius = 2500.;
				
				std::string empire_flagship_class = Data.PropertyAsString("empire_flagship");
				Ship *empire_flagship = SpawnShip( GetShipClass(empire_flagship_class), XWing::Team::EMPIRE );
				empire_flagship->IsMissionObjective = true;
				empire_flagship->CanRespawn = false;
				empire_flagship->JumpProgress = 1.;
				empire_flagship->X = -path_radius;
				empire_flagship->Y = 0.;
				empire_flagship->Z = 0.;
				empire_flagship->SetFwdVec( 0., 1., 0. );
				empire_flagship->SetUpVec( 0., 0., 1. );
				empire_flagship->ResetTurrets();
				empire_flagship->Health *= Data.PropertyAsDouble( "empire_flagship_hull", 1., 1. );
				double empire_shields = Data.PropertyAsDouble( "empire_flagship_shields", 1., 1. );
				empire_flagship->ShieldF *= empire_shields;
				empire_flagship->ShieldR *= empire_shields;
				const char *empire_flagship_cstr = empire_flagship_class.c_str();
				empire_flagship->Name = Data.PropertyAsString("empire_flagship_name");
				if( empire_flagship->Name.length() )
					;
				else if( strcasecmp( empire_flagship_cstr, "ISD" ) == 0 )
					empire_flagship->Name = "Devastator";
				else if( strcasecmp( empire_flagship_cstr, "ISD2" ) == 0 )
					empire_flagship->Name = "Eviscerator";
				else if( strcasecmp( empire_flagship_cstr, "FRG" ) == 0 )
					empire_flagship->Name = "Warspite";
				else if( strcasecmp( empire_flagship_cstr, "INT" ) == 0 )
					empire_flagship->Name = "Black Asp";
				else if( strcasecmp( empire_flagship_cstr, "VSD" ) == 0 )
					empire_flagship->Name = "Imperator";
				else if( strcasecmp( empire_flagship_cstr, "VSD2" ) == 0 )
					empire_flagship->Name = "Warlord";
				else if( strcasecmp( empire_flagship_cstr, "T/A" ) == 0 )
					empire_flagship->Name = "Darth Vader";
				else if( empire_flagship->Category() != ShipClass::CATEGORY_CAPITAL )
					empire_flagship->Name = "Omega Leader";
				else
					empire_flagship->Name = "Empire Flagship";
				
				std::string rebel_flagship_class = Data.PropertyAsString("rebel_flagship");
				Ship *rebel_flagship = SpawnShip( GetShipClass(rebel_flagship_class), XWing::Team::REBEL );
				rebel_flagship->IsMissionObjective = true;
				rebel_flagship->CanRespawn = false;
				rebel_flagship->JumpProgress = 1.;
				rebel_flagship->X = path_radius;
				rebel_flagship->Y = 0.;
				rebel_flagship->Z = 0.;
				rebel_flagship->SetFwdVec( 0., -1., 0. );
				rebel_flagship->SetUpVec( 0., 0., 1. );
				rebel_flagship->ResetTurrets();
				rebel_flagship->Health *= Data.PropertyAsDouble( "rebel_flagship_hull", 1., 1. );
				double rebel_shields = Data.PropertyAsDouble( "rebel_flagship_shields", 1., 1. );
				rebel_flagship->ShieldF *= rebel_shields;
				rebel_flagship->ShieldR *= rebel_shields;
				const char *rebel_flagship_cstr = rebel_flagship_class.c_str();
				rebel_flagship->Name = Data.PropertyAsString("rebel_flagship_name");
				if( rebel_flagship->Name.length() )
					;
				else if( strcasecmp( rebel_flagship_cstr, "CRV" ) == 0 )
					rebel_flagship->Name = "Korolev";
				else if( strcasecmp( rebel_flagship_cstr, "FRG" ) == 0 )
					rebel_flagship->Name = "Redemption";
				else if( strcasecmp( rebel_flagship_cstr, "CRS" ) == 0 )
					rebel_flagship->Name = "Independence";
				else if( strcasecmp( rebel_flagship_cstr, "YT1300" ) == 0 )
					rebel_flagship->Name = "M.Falcon";
				else if( rebel_flagship->Category() != ShipClass::CATEGORY_CAPITAL )
					rebel_flagship->Name = "Rogue Leader";
				else
					rebel_flagship->Name = "Rebel Flagship";
				
				for( double theta = 2. * M_PI; theta > 0.01; theta -= M_PI / 8. )
					Waypoints[ 0 ].push_back(Pos3D( path_radius * cos(theta), path_radius * sin(theta), 0. ));
			}
			
			// Add the race course checkpoints.
			else if( (GameType == XWing::GameType::TEAM_RACE) || (GameType == XWing::GameType::FFA_RACE) )
			{
				// Generate the course checkpoints.
				std::vector<Checkpoint*> checkpoints;
				int res = Data.PropertyAsInt("race_lap");
				if( res )
				{
					for( int i = 0; i < res; i ++ )
					{
						double percent = i / (double) res;
						double px = sin( percent * 2. * M_PI );
						double py = cos( percent * 2. * M_PI );
						double x = px * Rand::Double( percent * 1000., 900. );
						double y = py * -1000. + 100.;
						double z = Rand::Double( -500., 500. );
						checkpoints.push_back( new Checkpoint( x, y, z ) );
					}
				}
				else
				{
					for( int i = 0; i < 66; i ++ )  // FIXME: Customizable count?
						checkpoints.push_back( new Checkpoint( Rand::Double(-1111.,1111.), Rand::Double(-1111.,1111.), Rand::Double(-1111.,1111.) ) );
				}
				
				// Add checkpoints to game data and connect in a loop.
				Checkpoint *prev = checkpoints.front();
				Data.AddObject( prev );
				for( std::vector<Checkpoint*>::iterator checkpoint_iter = checkpoints.begin() + 1; checkpoint_iter != checkpoints.end(); checkpoint_iter ++ )
				{
					Data.AddObject( *checkpoint_iter );
					prev->SetNext( *checkpoint_iter );
					prev = *checkpoint_iter;
				}
				prev->SetNext( checkpoints.front() );
				
				// Get asteroids out of the way.
				double race_tunnel = Data.PropertyAsDouble( "race_tunnel", 30. );
				for( std::vector<Checkpoint*>::iterator checkpoint_iter = checkpoints.begin(); checkpoint_iter != checkpoints.end(); checkpoint_iter ++ )
				{
					Checkpoint *checkpoint = *checkpoint_iter;
					GameObject *next = Data.GetObject( checkpoint->Next );
					for( std::set<Asteroid*>::iterator asteroid_iter = asteroids.begin(); asteroid_iter != asteroids.end(); asteroid_iter ++ )
					{
						double dist = (*asteroid_iter)->Dist( checkpoint );
						if( dist < (checkpoint->Radius + 5.) )
							Data.RemoveObject( (*asteroid_iter)->ID );
						else
						{
							if( dist < (checkpoint->Radius + (*asteroid_iter)->Radius) )
								(*asteroid_iter)->SetRadius( dist - checkpoint->Radius );
							if( next )
							{
								dist = Math3D::PointToLineSegDist( *asteroid_iter, checkpoint, next );
								if( dist < (race_tunnel + 5.) )
									Data.RemoveObject( (*asteroid_iter)->ID );
								else if( dist < ((*asteroid_iter)->Radius + race_tunnel) )
									(*asteroid_iter)->SetRadius( dist - race_tunnel );
							}
						}
					}
				}
			}
		}
		
		std::string prefix = (GameType == XWing::GameType::BATTLE_OF_YAVIN) ? "yavin_" : "";
		
		// Add AI cruisers and frigates in Fleet Battle.
		int rebel_cruisers  = (GameType == XWing::GameType::FLEET_BATTLE) ? Data.PropertyAsInt("rebel_cruisers")  : 0;
		int empire_cruisers = (GameType == XWing::GameType::FLEET_BATTLE) ? Data.PropertyAsInt("empire_cruisers") : 0;
		int rebel_frigates  = (GameType == XWing::GameType::FLEET_BATTLE) ? Data.PropertyAsInt("rebel_frigates")  : 0;
		int empire_frigates = (GameType == XWing::GameType::FLEET_BATTLE) ? Data.PropertyAsInt("empire_frigates") : 0;
		
		const ShipClass *rebel_cruiser_class  = rebel_cruisers  ? GetShipClass(Data.PropertyAsString(prefix+std::string("rebel_cruiser")))  : NULL;
		const ShipClass *empire_cruiser_class = empire_cruisers ? GetShipClass(Data.PropertyAsString(prefix+std::string("empire_cruiser"))) : NULL;
		const ShipClass *rebel_frigate_class  = rebel_frigates  ? GetShipClass(Data.PropertyAsString(prefix+std::string("rebel_frigate")))  : NULL;
		const ShipClass *empire_frigate_class = empire_frigates ? GetShipClass(Data.PropertyAsString(prefix+std::string("empire_frigate"))) : NULL;
		
		std::string rebel_cruiser_squadron  = (rebel_cruiser_class  && rebel_cruiser_class->Squadron.length())  ? rebel_cruiser_class->Squadron  : "Rebel";
		std::string empire_cruiser_squadron = (empire_cruiser_class && empire_cruiser_class->Squadron.length()) ? empire_cruiser_class->Squadron : "Imperial";
		std::string rebel_frigate_squadron  = (rebel_frigate_class  && rebel_frigate_class->Squadron.length())  ? rebel_frigate_class->Squadron  : "Rebel";
		std::string empire_frigate_squadron = (empire_frigate_class && empire_frigate_class->Squadron.length()) ? empire_frigate_class->Squadron : "Imperial";
		
		std::vector<std::string> rebel_cruiser_names = Str::SplitToVector( Data.PropertyAsString("rebel_cruiser_names"),   "," );
		std::vector<std::string> empire_cruiser_names = Str::SplitToVector( Data.PropertyAsString("empire_cruiser_names"), "," );
		std::vector<std::string> rebel_frigate_names = Str::SplitToVector( Data.PropertyAsString("rebel_frigate_names"),   "," );
		std::vector<std::string> empire_frigate_names = Str::SplitToVector( Data.PropertyAsString("empire_frigate_names"), "," );
		
		int ai_frigates = std::max<int>( rebel_frigates, empire_frigates );
		for( int frigate = 0; frigate < ai_frigates; frigate ++ )
		{
			for( int i = 0; i < 2; i ++ )
			{
				bool rebel = i;
				if( rebel && (rebel_frigates <= frigate) )
					continue;
				if( (! rebel) && (empire_frigates <= frigate) )
					continue;
				const ShipClass *ship_class = rebel ? rebel_frigate_class : empire_frigate_class;
				std::string squadron = rebel ? rebel_frigate_squadron : empire_frigate_squadron;
				std::vector<std::string> &names = rebel ? rebel_frigate_names : empire_frigate_names;
				bool named = (frigate < (int) names.size()) && ! names.at( frigate ).empty();
				uint8_t team = (rebel ? XWing::Team::REBEL : XWing::Team::EMPIRE);
				Ship *ship = SpawnShip( ship_class, team );
				ship->Group = 255;
				ship->PlayerID = 0;
				ship->CanRespawn = Data.PropertyAsBool( "ai_respawn", Respawn );
				ship->Name = named ? names.at( frigate ) : squadron + std::string(" ") + Num::ToString( (int) Squadrons[ squadron ].size() + 1 );
				ship->SetFwdVec( (rebel ? -1. : 1.), 0., 0. );
				ship->SetUpVec( 0., 0., 1. );
				for( int retry = 0; retry < 10; retry ++ )
				{
					ship->X = Rand::Double( -150.,  150. ) + (rebel ? 2000. : -2000.) + frigate * (rebel ? -200. : 200.);
					ship->Y = Rand::Double( -400.,  400. ) * (frigate + 1.) + retry * ship->Radius() * (((retry + frigate) % 2) ? -0.333 : 0.333);
					ship->Z = Rand::Double(  400.,  300. ) * (frigate + 1.5) + ship->Radius();
					if( ! ship->FindCollision( 5. ) )  // If we are not colliding with another ship at spawn, this position is probably good.
					{
						bool safe_space = true;
						if( retry <= 7 )
						{
							// If we have some retries left, make sure there are no ships too close.
							for( std::map<uint32_t,GameObject*>::const_iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
							{
								if( obj_iter->second->Type() == XWing::Object::SHIP )
								{
									const Ship *other = (const Ship*) obj_iter->second;
									if( ship->Dist( other ) < (ship->Radius() + other->Radius()) * 15. / (retry + 7) )
									{
										safe_space = false;
										break;
									}
								}
								else if( obj_iter->second->Type() == XWing::Object::ASTEROID )
								{
									const Asteroid *asteroid = (const Asteroid*) obj_iter->second;
									if( ship->Dist( asteroid ) < (ship->Radius() + asteroid->Radius) * 15. / (retry + 7) )
									{
										safe_space = false;
										break;
									}
								}
							}
						}
						if( safe_space )
							break;
					}
				}
				ship->ResetTurrets();
				if( ! named )
					Squadrons[ squadron ].insert( ship->ID );
			}
		}
		
		int ai_cruisers = std::max<int>( rebel_cruisers, empire_cruisers );
		for( int cruiser = 0; cruiser < ai_cruisers; cruiser ++ )
		{
			for( int i = 0; i < 2; i ++ )
			{
				bool rebel = i;
				if( rebel && (rebel_cruisers <= cruiser) )
					continue;
				if( (! rebel) && (empire_cruisers <= cruiser) )
					continue;
				const ShipClass *ship_class = rebel ? rebel_cruiser_class : empire_cruiser_class;
				std::string squadron = rebel ? rebel_cruiser_squadron : empire_cruiser_squadron;
				uint8_t team = (rebel ? XWing::Team::REBEL : XWing::Team::EMPIRE);
				std::vector<std::string> &names = rebel ? rebel_cruiser_names : empire_cruiser_names;
				bool named = (cruiser < (int) names.size()) && ! names.at( cruiser ).empty();
				Ship *ship = SpawnShip( ship_class, team );
				ship->Group = 255;
				ship->PlayerID = 0;
				ship->CanRespawn = Data.PropertyAsBool( "ai_respawn", Respawn );
				ship->Name = named ? names.at( cruiser ) : squadron + std::string(" ") + Num::ToString( (int) Squadrons[ squadron ].size() + 1 );
				ship->SetFwdVec( (rebel ? -1. : 1.), 0., 0. );
				ship->SetUpVec( 0., 0., 1. );
				for( int retry = 0; retry < 10; retry ++ )
				{
					ship->X = Rand::Double( -150.,  150. ) + (rebel ? 2000. : -2000.) + cruiser * (rebel ? -200. : 200.);
					ship->Y = Rand::Double( -400.,  400. ) * (cruiser + 1.) + retry * ship->Radius() * (((retry + cruiser) % 2) ? -0.333 : 0.333);
					ship->Z = Rand::Double( -400., -300. ) * (cruiser + 1.5) - ship->Radius();
					if( ! ship->FindCollision( 5. ) )  // If we are not colliding with another ship at spawn, this position is probably good.
					{
						bool safe_space = true;
						if( retry <= 7 )
						{
							// If we have some retries left, make sure there are no ships too close.
							for( std::map<uint32_t,GameObject*>::const_iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
							{
								if( obj_iter->second->Type() == XWing::Object::SHIP )
								{
									const Ship *other = (const Ship*) obj_iter->second;
									if( ship->Dist( other ) < (ship->Radius() + other->Radius()) * 15. / (retry + 7) )
									{
										safe_space = false;
										break;
									}
								}
								else if( obj_iter->second->Type() == XWing::Object::ASTEROID )
								{
									const Asteroid *asteroid = (const Asteroid*) obj_iter->second;
									if( ship->Dist( asteroid ) < (ship->Radius() + asteroid->Radius) * 15. / (retry + 7) )
									{
										safe_space = false;
										break;
									}
								}
							}
						}
						if( safe_space )
							break;
					}
				}
				ship->ResetTurrets();
				if( ! named )
					Squadrons[ squadron ].insert( ship->ID );
			}
		}
		
		// Clear scores and count the players who picked teams.
		std::set<Player*> pilots;
		uint32_t rebel_count = 0;
		uint32_t empire_count = 0;
		
		Data.Lock.Lock();
		
		for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
		{
			SetPlayerProperty( player_iter->second, "score",   "0" );
			SetPlayerProperty( player_iter->second, "kills",   "0" );
			SetPlayerProperty( player_iter->second, "kills_c", "0" );
			SetPlayerProperty( player_iter->second, "kills_t", "0" );
			SetPlayerProperty( player_iter->second, "deaths",  "0" );
			
			std::string player_ship = ChosenShip( player_iter->second );
			if( strcasecmp( player_ship.c_str(), "Spectator" ) == 0 )
				SetPlayerProperty( player_iter->second, "team", "Spectator" );
			else if( strcasecmp( player_ship.c_str(), "Rebel Gunner" ) == 0 )
				SetPlayerProperty( player_iter->second, "team", "Rebel" );
			else if( strcasecmp( player_ship.c_str(), "Imperial Gunner" ) == 0 )
				SetPlayerProperty( player_iter->second, "team", "Empire" );
			else
			{
				pilots.insert( player_iter->second );
				
				const ShipClass *ship_class = player_ship.length() ? GetShipClass(player_ship) : NULL;
				if( ship_class )
				{
					if( ship_class->Team == XWing::Team::EMPIRE )
					{
						SetPlayerProperty( player_iter->second, "team", "Empire" );
						empire_count ++;
					}
					else
					{
						// Corvettes and Frigates are typically Rebel, so that's the default team for any non-Imperial ship.
						SetPlayerProperty( player_iter->second, "team", "Rebel" );
						rebel_count ++;
					}
				}
				else if( ! respawn )
				{
					// Auto-Assign (or invalid ship selected)
					SetPlayerProperty( player_iter->second, "team", "" );
					SetPlayerProperty( player_iter->second, "group", "0" );
				}
			}
		}
		
		Data.Lock.Unlock();
		
		std::map< uint8_t, std::map<uint8_t,Pos3D> > group_spawns;
		std::vector<double> player_spawn;
		if( Data.PropertyAsString("gametype") == "mission" )
			player_spawn = Data.PropertyAsDoubles("player_spawn");
		
		// Build a new list of ships.
		// FIXME: Do auto-assigned players last to better balance teams!
		for( std::set<Player*>::iterator pilot_iter = pilots.begin(); pilot_iter != pilots.end(); pilot_iter ++ )
		{
			bool rebel = Rand::Bool();
			if( pilots.size() == 1 )
			{
				// Only one player flying, so pick the best team for the scenario.
				if( GameType == XWing::GameType::CAPITAL_SHIP_HUNT )
					rebel = (DefendingTeam != XWing::Team::REBEL);
				else
					rebel = true;
			}
			
			// If a ship is selected, use that team by default.
			const ShipClass *ship_class = GetShipClass( ChosenShip( *pilot_iter ) );
			if( ship_class )
				rebel = (ship_class->Team != XWing::Team::EMPIRE);
			
			std::string chosen_team = ChosenTeam( *pilot_iter );
			if( chosen_team == "Rebel" )
				rebel = true;
			else if( chosen_team == "Empire" )
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
			
			SetPlayerProperty( *pilot_iter, "team", rebel ? "Rebel" : "Empire" );
			std::string squadron = rebel ? "Rogue" : "Omega";
			
			uint8_t team = XWing::Team::NONE;
			int group = 0;
			if( ! ffa )
			{
				team = (rebel ? XWing::Team::REBEL : XWing::Team::EMPIRE);
				group = (*pilot_iter)->PropertyAsInt("group");
				if( ! group )
					group = Data.PropertyAsInt("player_group");
			}
			
			if( ! ship_class )
				ship_class = rebel ? GetShipClass(Data.PropertyAsString(prefix+std::string("rebel_fighter"))) : GetShipClass(Data.PropertyAsString(prefix+std::string("empire_fighter")));
			
			// FIXME: Make sure team is valid for ship_class->Team in non-FFA modes?
			Ship *ship = SpawnShip( ship_class, team );
			ship->PlayerID = (*pilot_iter)->ID;
			ship->Group = group;
			ship->CanRespawn = Respawn;
			ship->Name = squadron + std::string(" ") + Num::ToString( (int) Squadrons[ squadron ].size() + 1 );
			if( player_spawn.size() >= 3 )
			{
				ship->X = player_spawn.at(0) + Rand::Double( -100., 100. );
				ship->Y = player_spawn.at(1) + Rand::Double( -100., 100. ) * (ffa ? 10. : 1.);
				ship->Z = player_spawn.at(2) + Rand::Double( -100., 100. ) * (ffa ? 10. : 1.);
			}
			else
			{
				ship->X = Rand::Double( -100., 100. ) + (rebel ? 1500. : -1500.);
				ship->Y = Rand::Double( -100., 100. ) * (ffa ? 10. : 1.);
				ship->Z = Rand::Double( -100., 100. ) * (ffa ? 10. : 1.);
			}
			if( player_spawn.size() >= 6 )
				ship->SetFwdVec( player_spawn.at(3), player_spawn.at(4), player_spawn.at(5) );
			else
				ship->SetFwdVec( (rebel && (GameType != XWing::GameType::BATTLE_OF_YAVIN) ? -1. : 1.), 0., 0. );
			ship->SetUpVec( 0., 0., 1. );
			ship->ResetTurrets();
			if( ship->NextCheckpoint )
			{
				ship->Copy( Data.GetObject( ship->NextCheckpoint ) );
				ship->MoveAlong( &(ship->Fwd), ship->MaxSpeed() * -10. + (ship->ID % 32) * ship->Radius() * -3. );
				ship->MoveAlong( &(ship->Up),    Rand::Double( ship->Radius() * -5., ship->Radius() * 5. ) );
				ship->MoveAlong( &(ship->Right), Rand::Double( ship->Radius() * -5., ship->Radius() * 5. ) );
			}
			else if( GameType == XWing::GameType::FLEET_BATTLE )
			{
				ship->X *= 1.5;
				ship->Z += 100.;
				ship->Pitch( -5. );
			}
			
			if( ship->Group )
			{
				// Spawn flight groups together.
				if( group_spawns[ ship->Team ].find( ship->Group ) != group_spawns[ ship->Team ].end() )
				{
					// Another ship from this group is spawning ahead.
					group_spawns[ ship->Team ][ ship->Group ].MoveAlong( &(group_spawns[ ship->Team ][ ship->Group ].Fwd), -8. * ship->Radius() );
					group_spawns[ ship->Team ][ ship->Group ].MoveAlong( &(group_spawns[ ship->Team ][ ship->Group ].Right), 4. * ship->Radius() );
					ship->Copy( &(group_spawns[ ship->Team ][ ship->Group ]) );
				}
				else
					// This is the lead ship of its flight group.
					group_spawns[ ship->Team ][ ship->Group ].Copy( ship );
			}
			
			Squadrons[ squadron ].insert( ship->ID );
		}
		
		// Add AI ships based on the number of waves selected.
		int ai_in_first_wave = 4;
		int ai_in_other_waves = 4;
		if( (GameType == XWing::GameType::TEAM_RACE) || (GameType == XWing::GameType::FFA_RACE) )
		{
			ai_in_first_wave = Respawn ? 3 : 4;
			ai_in_other_waves = Respawn ? 2 : 4;
		}
		else if( ! Respawn )
		{
			ai_in_first_wave = 8;
			ai_in_other_waves = ffa ? 6 : 8;
		}
		else if( GameType == XWing::GameType::TEAM_DEATHMATCH )
			ai_in_first_wave = 6;
		
		ai_in_first_wave = Data.PropertyAsInt( "ai_wave1", Data.PropertyAsInt( "ai_per_wave", ai_in_first_wave ) );
		ai_in_other_waves = Data.PropertyAsInt( "ai_per_wave", ai_in_other_waves );
		
		int ai_waves = Data.PropertyAsInt("ai_waves");
		int rebel_waves = Data.PropertyAsInt( "rebel_waves", ai_waves );
		int empire_waves = Data.PropertyAsInt( "empire_waves", ai_waves );
		ai_waves = std::max<int>( empire_waves, rebel_waves );
		double rebel_wave_dist = Data.PropertyAsDouble( "rebel_wave_dist", Data.PropertyAsDouble( "wave_dist", 500. ) );
		double empire_wave_dist = Data.PropertyAsDouble( "empire_wave_dist", Data.PropertyAsDouble( "wave_dist", 500. ) );
		bool ai_grouped = Data.PropertyAsBool( "ai_grouped", true );
		
		std::vector<double> rebel_spawn  = Data.PropertyAsDoubles("rebel_spawn");
		std::vector<double> empire_spawn = Data.PropertyAsDoubles("empire_spawn");
		
		double ai_empire_ratio = Data.PropertyAsDouble( "ai_empire_ratio", 1. );
		double ai_fighter_ratio = Data.PropertyAsInt( "ai_fighter_ratio", 2. );
		
		bool rebel_use_bombers  = (GameType == XWing::GameType::FLEET_BATTLE) || ((GameType == XWing::GameType::CAPITAL_SHIP_HUNT) && (DefendingTeam == XWing::Team::EMPIRE)) || (GameType == XWing::GameType::BATTLE_OF_YAVIN);
		bool empire_use_bombers = (GameType == XWing::GameType::FLEET_BATTLE) || ((GameType == XWing::GameType::CAPITAL_SHIP_HUNT) && (DefendingTeam == XWing::Team::REBEL));
		
		const ShipClass *rebel_fighter_class  = GetShipClass(Data.PropertyAsString(prefix+std::string("rebel_fighter")));
		const ShipClass *empire_fighter_class = GetShipClass(Data.PropertyAsString(prefix+std::string("empire_fighter")));
		const ShipClass *rebel_bomber_class   = rebel_use_bombers  ? GetShipClass(Data.PropertyAsString(prefix+std::string("rebel_bomber")))   : NULL;
		const ShipClass *empire_bomber_class  = empire_use_bombers ? GetShipClass(Data.PropertyAsString(prefix+std::string("empire_bomber")))  : NULL;
		
		std::string rebel_fighter_squadron  = (rebel_fighter_class  && rebel_fighter_class->Squadron.length())  ? rebel_fighter_class->Squadron  : "Rogue";
		std::string empire_fighter_squadron = (empire_fighter_class && empire_fighter_class->Squadron.length()) ? empire_fighter_class->Squadron : "Omega";
		std::string rebel_bomber_squadron   = (rebel_bomber_class   && rebel_bomber_class->Squadron.length())   ? rebel_bomber_class->Squadron   : "Echo";
		std::string empire_bomber_squadron  = (empire_bomber_class  && empire_bomber_class->Squadron.length())  ? empire_bomber_class->Squadron  : "Zeta";
		
		rebel_fighter_squadron  = Data.PropertyAsString( "rebel_fighter_squadron",  rebel_fighter_squadron.c_str()  );
		empire_fighter_squadron = Data.PropertyAsString( "empire_fighter_squadron", empire_fighter_squadron.c_str() );
		rebel_bomber_squadron   = Data.PropertyAsString( "rebel_bomber_squadron",   rebel_bomber_squadron.c_str()   );
		empire_bomber_squadron  = Data.PropertyAsString( "empire_bomber_squadron",  empire_bomber_squadron.c_str()  );
		
		int rebel_fighter_group  = Data.PropertyAsInt("rebel_fighter_group");
		int empire_fighter_group = Data.PropertyAsInt("empire_fighter_group");
		int rebel_bomber_group   = Data.PropertyAsInt("rebel_bomber_group");
		int empire_bomber_group  = Data.PropertyAsInt("empire_bomber_group");
		
		for( int wave = 0; wave < ai_waves; wave ++ )
		{
			const ShipClass *rebel_ship_class = rebel_fighter_class;
			std::string rebel_squadron = rebel_fighter_squadron;
			int rebel_group = rebel_fighter_group;
			if( rebel_bomber_class && (Squadrons[ rebel_fighter_squadron ].size() > Squadrons[ rebel_bomber_squadron ].size() * ai_fighter_ratio) )
			{
				rebel_ship_class = rebel_bomber_class;
				rebel_squadron = rebel_bomber_squadron;
				rebel_group = rebel_bomber_group;
			}
			
			const ShipClass *empire_ship_class = empire_fighter_class;
			std::string empire_squadron = empire_fighter_squadron;
			int empire_group = empire_fighter_group;
			if( empire_bomber_class && (Squadrons[ empire_fighter_squadron ].size() > Squadrons[ empire_bomber_squadron ].size() * ai_fighter_ratio) )
			{
				empire_ship_class = empire_bomber_class;
				empire_squadron = empire_bomber_squadron;
				empire_group = empire_bomber_group;
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
				
				const ShipClass *ship_class = rebel ? rebel_ship_class : empire_ship_class;
				std::string squadron = rebel ? rebel_squadron : empire_squadron;
				uint8_t team = (ffa ? XWing::Team::NONE : (rebel ? XWing::Team::REBEL : XWing::Team::EMPIRE));
				
				if( team == (XWing::Team::REBEL) && (wave >= rebel_waves) )
					continue;
				if( team == (XWing::Team::EMPIRE) && (wave >= empire_waves) )
					continue;
				
				Ship *ship = SpawnShip( ship_class, team );
				ship->PlayerID = 0;
				ship->CanRespawn = Data.PropertyAsBool( "ai_respawn", Respawn );
				ship->Name = squadron + std::string(" ") + Num::ToString( (int) Squadrons[ squadron ].size() + 1 );
				
				ship->Group = rebel ? rebel_group : empire_group;
				if( ai_grouped && ! ship->Group )
					ship->Group = 100 + (wave * 2) + (((ship_class == rebel_bomber_class) || (ship_class == empire_bomber_class)) ? 1 : 0);
				
				if( ship->NextCheckpoint )
				{
					ship->Copy( Data.GetObject( ship->NextCheckpoint ) );
					ship->MoveAlong( &(ship->Fwd), ship->MaxSpeed() * -10. + (ship->ID % 32) * ship->Radius() * 4. );
					ship->MoveAlong( &(ship->Up),    Rand::Double( ship->Radius() * -10., ship->Radius() * 10. ) );
					ship->MoveAlong( &(ship->Right), Rand::Double( ship->Radius() * -10., ship->Radius() * 10. ) );
				}
				else if( (team == XWing::Team::REBEL) && (rebel_spawn.size() >= 3) )
				{
					ship->X = rebel_spawn.at(0) + Rand::Double( -150., 150. ) + wave * rebel_wave_dist;
					ship->Y = rebel_spawn.at(1) + Rand::Double( -200., 200. ) * (wave + 1) * (ffa ? 10. : 1.);
					ship->Z = rebel_spawn.at(2) + Rand::Double( -200., 200. ) * (wave + 1) * (ffa ? 10. : 1.);
					if( rebel_spawn.size() >= 6 )
						ship->SetFwdVec( rebel_spawn.at(3), rebel_spawn.at(4), rebel_spawn.at(5) );
					else
						ship->SetFwdVec( -1., 0., 0. );
				}
				else if( (team == XWing::Team::EMPIRE) && (empire_spawn.size() >= 3) )
				{
					ship->X = empire_spawn.at(0) + Rand::Double( -150., 150. ) - wave * empire_wave_dist;
					ship->Y = empire_spawn.at(1) + Rand::Double( -200., 200. ) * (wave + 1) * (ffa ? 10. : 1.);
					ship->Z = empire_spawn.at(2) + Rand::Double( -200., 200. ) * (wave + 1) * (ffa ? 10. : 1.);
					if( empire_spawn.size() >= 6 )
						ship->SetFwdVec( empire_spawn.at(3), empire_spawn.at(4), empire_spawn.at(5) );
					else
						ship->SetFwdVec( 1., 0., 0. );
				}
				else if( GameType == XWing::GameType::BATTLE_OF_YAVIN )
				{
					double wave_dist = rebel ? 100. : (Respawn ? 4000. : 1000.);
					if( Data.HasProperty("wave_dist") || (Data.HasProperty("rebel_wave_dist") && rebel) || (Data.HasProperty("empire_wave_dist") && ! rebel) )
						wave_dist = rebel ? rebel_wave_dist : empire_wave_dist;
					ship->X = Rand::Double( -150., 150. ) + (rebel ? 2000. : -2000.) + wave * wave_dist * (rebel ? 1. : -1.);
					ship->Y = Rand::Double( -200., 200. ) * (wave + 1);
					ship->Z = Rand::Double( 100., 1000. );
					ship->SetFwdVec( 1., 0., 0. );
				}
				else if( GameType == XWing::GameType::FLEET_BATTLE )
				{
					double wave_dist = Respawn ? 500. : 4000.;
					if( Data.HasProperty("wave_dist") || Data.HasProperty("rebel_wave_dist") || Data.HasProperty("empire_wave_dist") )
						wave_dist = rebel ? rebel_wave_dist : empire_wave_dist;
					ship->X = Rand::Double( -150., 150. ) + (rebel ? 2000. : -2000.) + (wave + 1) * wave_dist * (rebel ? 1. : -1.);
					ship->Y = Rand::Double( -200., 200. ) * (wave + 1);
					ship->Z = Rand::Double( -400., -200. ) * (wave + 1);
					ship->SetFwdVec( (rebel ? -1. : 1.), 0., 0. );
				}
				else
				{
					double wave_dist = (Respawn || (GameType == XWing::GameType::FFA_ELIMINATION)) ? 1800. : 4000.;
					if( Data.HasProperty("wave_dist") || Data.HasProperty("rebel_wave_dist") || Data.HasProperty("empire_wave_dist") )
						wave_dist = rebel ? rebel_wave_dist : empire_wave_dist;
					ship->X = Rand::Double( -150., 150. ) + (rebel ? 1200. : -1200.) + wave * wave_dist * (rebel ? 1. : -1.);
					ship->Y = Rand::Double( -200., 200. ) * (wave + 1) * (ffa ? 10. : 1.);
					ship->Z = Rand::Double( -200., 200. ) * (wave + 1) * (ffa ? 10. : 1.);
					ship->SetFwdVec( (rebel ? -1. : 1.), 0., 0. );
				}

				ship->SetUpVec( 0., 0., 1. );
				ship->ResetTurrets();
				
				Squadrons[ squadron ].insert( ship->ID );
			}
		}
		
		
		// Test cases.
		
		std::string spawn = Data.PropertyAsString("spawn");
		if( ! spawn.empty() )
		{
			std::list<std::string> spawn_list = Str::SplitToList( spawn, "," );
			for( std::list<std::string>::iterator spawn_iter = spawn_list.begin(); spawn_iter != spawn_list.end(); spawn_iter ++ )
			{
				const ShipClass *sc = GetShipClass(*spawn_iter);
				if( sc )
				{
					uint8_t team = sc->Team;
					if( !( team || ffa ) )
						team = XWing::Team::REBEL;
					Ship *ship = SpawnShip( sc, team );
					bool rebel = (team != XWing::Team::EMPIRE);
					ship->CanRespawn = Data.PropertyAsBool( "ai_respawn", false, false );
					if( ship->NextCheckpoint )
					{
						ship->Copy( Data.GetObject( ship->NextCheckpoint ) );
						ship->MoveAlong( &(ship->Fwd), ship->MaxSpeed() * -10. + (ship->ID % 32) * ship->Radius() * 4. );
						ship->MoveAlong( &(ship->Up),    Rand::Double( ship->Radius() * -10., ship->Radius() * 10. ) );
						ship->MoveAlong( &(ship->Right), Rand::Double( ship->Radius() * -10., ship->Radius() * 10. ) );
					}
					else
					{
						ship->X = Rand::Double( 1500., 2500. ) * (rebel ? 1. : -1);
						ship->Y = Rand::Double( 1500., 2500. ) * (rebel ? 1. : -1);
						ship->Z = Rand::Double( -1000., 1000. ) + ((GameType == XWing::GameType::BATTLE_OF_YAVIN) ? 2500. : 0.);
						ship->SetFwdVec( 0., rebel ? -1. : 1., 0. );
						ship->SetUpVec( 0., 0., 1. );
					}
					ship->ResetTurrets();
					// FIXME: Add to squadron instead of all this mess below?
					ship->Name = sc->LongName;
					const char *shortname = sc->ShortName.c_str();
					if( strcasecmp( shortname, "CRV" ) == 0 )
						ship->Name = "Korolev";
					else if( strcasecmp( shortname, "FRG" ) == 0 )
						ship->Name = "Redemption";
					else if( strcasecmp( shortname, "CRS" ) == 0 )
						ship->Name = "Stimsenjkat";
					else if( strcasecmp( shortname, "YT1300" ) == 0 )
						ship->Name = "M.Falcon";
					else if( strcasecmp( shortname, "T/A" ) == 0 )
						ship->Name = "Black 1";
					else if( ship->Name.length() > 11 )
					{
						// Trim "Assault Gunboat" to "Gunboat", etc.
						const char *space = strrchr( ship->Name.c_str(), ' ' );
						if( space )
							ship->Name = std::string( space + 1 );
					}
				}
			}
		}
		
		
		// Start the round timer and clear the scores.
		
		RoundTimer.Reset();
		TeamScores[ XWing::Team::REBEL ] = 0;
		TeamScores[ XWing::Team::EMPIRE ] = 0;
		ShipScores.clear();
		
		
		// Send list of objects to all players.
		
		std::vector<GameObject*> objects_to_send;
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHIP_CLASS )
				continue;
			if( ! obj_iter->second->ServerShouldSend() )
				continue;
			objects_to_send.push_back( obj_iter->second );
		}
		
		Packet obj_list = Packet( Raptor::Packet::OBJECTS_ADD );
		obj_list.AddUInt( objects_to_send.size() );
		for( std::vector<GameObject*>::iterator obj_iter = objects_to_send.begin(); obj_iter != objects_to_send.end(); obj_iter ++ )
		{
			obj_list.AddUInt( (*obj_iter)->ID );
			obj_list.AddUInt( (*obj_iter)->Type() );
			(*obj_iter)->AddToInitPacket( &obj_list );
		}
		Net.SendAll( &obj_list );
	}
	else if( State == XWing::State::FLYING )
	{
		// Late joiners want to Fly.
		
		
		// Count players currently on each team.
		
		uint32_t rebel_count = 0, empire_count = 0;
		
		Data.Lock.Lock();
		
		for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
		{
			std::string chosen_team = ChosenTeam( player_iter->second );
			if( chosen_team == "Rebel" )
				rebel_count ++;
			else if( chosen_team == "Empire" )
				empire_count ++;
		}
		
		Data.Lock.Unlock();
		
		
		// Prepare a packet of existing objects to send to new players.
		
		Packet obj_list( Raptor::Packet::OBJECTS_ADD );
		obj_list.AddUInt( Data.GameObjects.size() - ShipClasses.size() );
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHIP_CLASS )
				continue;
			obj_list.AddUInt( obj_iter->second->ID );
			obj_list.AddUInt( obj_iter->second->Type() );
			obj_iter->second->AddToInitPacket( &obj_list );
		}
		
		
		// Assign new players to teams, and give them ships if possible.
		
		std::set<uint16_t> new_players;
		std::set<uint32_t> add_object_ids;
		std::string prefix = (GameType == XWing::GameType::BATTLE_OF_YAVIN) ? "yavin_" : "";
		
		Data.Lock.Lock();
		
		for( std::map<uint16_t,Player*>::iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
		{
			// Only the player that clicked Fly should join, not everyone in lobby.
			if( player_id && (player_iter->first != player_id) )
				continue;

			std::string chosen_ship = ChosenShip( player_iter->second );
			std::string chosen_team = ChosenTeam( player_iter->second );
			
			if( respawn || chosen_team.empty() )
			{
				if( ! respawn )
				{
					new_players.insert( player_iter->first );
					
					SetPlayerProperty( player_iter->second, "score",   "0" );
					SetPlayerProperty( player_iter->second, "kills",   "0" );
					SetPlayerProperty( player_iter->second, "kills_c", "0" );
					SetPlayerProperty( player_iter->second, "kills_t", "0" );
					SetPlayerProperty( player_iter->second, "deaths",  "0" );
				}
				
				bool rebel = (rebel_count < empire_count);
				if( respawn )
					rebel = (chosen_team != "Empire");
				else if( rebel_count == empire_count )
					rebel = Rand::Bool();
				
				if( chosen_ship == "Spectator" )
				{
					SetPlayerProperty( player_iter->second, "team", "Spectator" );
					continue;
				}
				else if( chosen_ship == "Rebel Gunner" )
				{
					SetPlayerProperty( player_iter->second, "team", "Rebel" );
					rebel_count ++;
					continue;
				}
				else if( chosen_ship == "Imperial Gunner" )
				{
					SetPlayerProperty( player_iter->second, "team", "Empire" );
					empire_count ++;
					continue;
				}
				else if( chosen_ship.length() )
				{
					const ShipClass *ship_class = GetShipClass( chosen_ship );
					if( ship_class )
						rebel = (ship_class->Team != XWing::Team::EMPIRE);  // Corvettes and Frigates are typically Rebel, so this should handle all cases well.
				}
				else if( ! respawn )
					// Auto-Assign
					SetPlayerProperty( player_iter->second, "group", "0" );
				
				if( rebel )
					rebel_count ++;
				else
					empire_count ++;
				
				bool ffa = (GameType == XWing::GameType::FFA_DEATHMATCH) || (GameType == XWing::GameType::FFA_RACE);
				uint8_t team = (ffa ? XWing::Team::NONE : (rebel ? XWing::Team::REBEL : XWing::Team::EMPIRE));
				bool player_was_spectator = (ChosenTeam( player_iter->second ) == "Spectator");
				SetPlayerProperty( player_iter->second, "team", rebel ? "Rebel" : "Empire" );
				
				// By default, the global Respawn variable allows respawn, and the position is randomized.
				bool ready_to_spawn = Respawn;
				bool fixed_spawn = false;
				Pos3D spawn_at;
				
				// Fleet Battle mode require fighters to spawn with their capital ship.
				if( ready_to_spawn && (GameType == XWing::GameType::FLEET_BATTLE) )
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
							spawn_at.FixVectors();
							spawn_at.MoveAlong( &(spawn_at.Fwd), spawn_at_ship->Radius() * 1.5 + Rand::Double( 0., 50. ) );
							break;
						}
					}
				}
				
				if( ready_to_spawn )
				{
					const ShipClass *ship_class = GetShipClass( ChosenShip( player_iter->second ) );
					if( ship_class && ! ffa )
					{
						if( rebel && (ship_class->Team == XWing::Team::EMPIRE) )
							ship_class = NULL;
						else if( (! rebel) && (ship_class->Team == XWing::Team::REBEL) )
							ship_class = NULL;
					}
					if( ! ship_class )
						ship_class = rebel ? GetShipClass(Data.PropertyAsString(prefix+std::string("rebel_fighter"))) : GetShipClass(Data.PropertyAsString(prefix+std::string("empire_fighter")));
					
					int group = ffa ? 0 : player_iter->second->PropertyAsInt("group");
					if( ! group )
						group = Data.PropertyAsInt("player_group");
					std::string squadron = rebel ? "Rogue" : "Omega";
					
					Ship *ship = SpawnShip( ship_class, team, &add_object_ids );
					ship->PlayerID = player_iter->second->ID;
					ship->Group = group;
					ship->CanRespawn = true;
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
					ship->ResetTurrets();
					
					Squadrons[ squadron ].insert( ship->ID );
					
					if( player_was_spectator || ! respawn )
					{
						Packet message( Raptor::Packet::MESSAGE );
						std::string joined = " entered the fray";
						if( ! ffa )
							joined = rebel ? " joined the Rebellion" : " joined the Empire";
						std::string ship_class_name = ship->Class ? ship->Class->LongName : ChosenShip( player_iter->second );
						std::string in_a = " in a ";
						if( Str::BeginsWith( ship_class_name, "A" )
						||  Str::BeginsWith( ship_class_name, "E" )
						||  Str::BeginsWith( ship_class_name, "I" )
						||  Str::BeginsWith( ship_class_name, "O" )
						||  Str::BeginsWith( ship_class_name, "F-" )
						||  Str::BeginsWith( ship_class_name, "H-" )
						||  Str::BeginsWith( ship_class_name, "L-" )
						||  Str::BeginsWith( ship_class_name, "M-" )
						||  Str::BeginsWith( ship_class_name, "N-" )
						||  Str::BeginsWith( ship_class_name, "R-" )
						||  Str::BeginsWith( ship_class_name, "S-" )
						||  Str::BeginsWith( ship_class_name, "X-" ) )
							in_a = " in an ";
						message.AddString( player_iter->second->Name + joined + in_a + ship_class_name + std::string(".") );
						message.AddUInt( TextConsole::MSG_CHAT );
						Net.SendAll( &message );
					}
					
					// Players in a live ship are not waiting to respawn.
					std::map<uint16_t,Clock>::iterator clock_iter = RespawnClocks.find( player_iter->first );
					if( clock_iter != RespawnClocks.end() )
						RespawnClocks.erase( clock_iter );
				}
			}
		}
		
		Data.Lock.Unlock();
		
		
		// Send objects that already existed to the new players.
		
		Net.Lock.Lock();
		
		for( std::list<ConnectedClient*>::iterator client = Net.Clients.begin(); client != Net.Clients.end(); client ++ )
		{
			if( new_players.count( (*client)->PlayerID ) )
				(*client)->Send( &obj_list );
		}
		
		Net.Lock.Unlock();
		
		
		// Send any new objects added (spawned ships and turrets) to all players.
		
		SendAddedObjects( &add_object_ids );
	}
	
	// Tell clients to clear/update team scores.
	SendScores();
	
	// Switch to flight mode and tell the clients.
	State = XWing::State::FLYING;
	Packet fly = Packet( XWing::Packet::FLY );
	fly.AddUInt( GameType );
	if( player_id && ! respawn )
		Net.SendToPlayer( &fly, player_id );
	else
		Net.SendAll( &fly );
}


void XWingServer::ShipKilled( Ship *ship, GameObject *killer_obj, Player *killer )
{
	Player *victim = ship->Owner();
	if( killer_obj && ! killer )
		killer = killer_obj->Owner();
	Ship   *killer_ship   = (killer_obj && (killer_obj->Type() == XWing::Object::SHIP))   ? (Ship*)   killer_obj : NULL;
	Turret *killer_turret = (killer_obj && (killer_obj->Type() == XWing::Object::TURRET)) ? (Turret*) killer_obj : NULL;
	if( killer_turret )
	{
		killer_ship = killer_turret->ParentShip();
		if( killer_ship && ! killer )
			killer = killer_ship->Owner();
	}
	
	if( victim && ! ship->PlayerID )
	{
		// Player ship died while AI co-pilot was in control.
		ship->PlayerID = victim->ID;
		ship->SendUpdate( 126 );
	}
	
	// Add to death count.
	if( victim )
		SetPlayerProperty( victim, "deaths", Num::ToString( victim->PropertyAsInt("deaths") + 1 ) );
	
	// Add to turret gunner death counts too.
	std::set<Player*> players_in_turrets = ship->PlayersInTurrets();
	for( std::set<Player*>::iterator victim_gunner = players_in_turrets.begin(); victim_gunner != players_in_turrets.end(); victim_gunner ++ )
	{
		if( *victim_gunner != victim )
			SetPlayerProperty( *victim_gunner, "deaths", Num::ToString( (*victim_gunner)->PropertyAsInt("deaths") + 1 ) );
	}
	
	// If the ship crashed or self-destructed after being hit, credit the kill to whomever most recently shot them.
	if( ship->HitByID && ! (killer || killer_ship) )
	{
		GameObject *hit_by = Data.GetObject( ship->HitByID );
		if( hit_by )
		{
			if( hit_by->Type() == XWing::Object::SHIP )
			{
				Ship *hit_by_ship = (Ship*) hit_by;
				if( (hit_by_ship->Team != ship->Team) || ! ship->Team )  // Don't punish friendly fire if it's not the killing blow.
				{
					killer_ship = hit_by_ship;
					killer = hit_by_ship->Owner();
				}
			}
			else if( hit_by->Type() == XWing::Object::TURRET )
			{
				Turret *hit_by_turret = (Turret*) hit_by;
				if( (hit_by_turret->Team != ship->Team) || ! ship->Team )  // Don't punish friendly fire if it's not the killing blow.
					killer = hit_by_turret->Owner();
			}
			else
				killer = hit_by->Owner();
		}
	}
	
	if( killer || killer_ship )
	{
		// Add to enemy score (or subtract from friendly score) when killed by another ship.
		
		int add_score = 1;
		if( ship->Team && ( (killer_ship && (killer_ship->Team == ship->Team)) || (killer_turret && (killer_turret->Team == ship->Team)) ) )
			add_score = -1;
		
		if( killer )
		{
			std::string score_type = "kills";
			// NOTE: Fighter kills always count as fighters, even when used as flagship or cruiser.
			if( ship->Class && ((ship->Class->Category == ShipClass::CATEGORY_CAPITAL) || (ship->Class->Category == ShipClass::CATEGORY_TARGET)) )
				score_type = "kills_c";
			
			SetPlayerProperty( killer, score_type, Num::ToString( killer->PropertyAsInt(score_type) + add_score ) );
		}
		else if( killer_ship && (GameType != XWing::GameType::TEAM_RACE) && (GameType != XWing::GameType::FFA_RACE) )
			ShipScores[ killer_ship->ID ] += add_score;
		
		if( killer_ship && killer_ship->Team && (add_score > 0) && (GameType != XWing::GameType::TEAM_RACE) )
			TeamScores[ killer_ship->Team ] += add_score;
	}
	else if( (GameType == XWing::GameType::TEAM_DEATHMATCH) || (GameType == XWing::GameType::FFA_DEATHMATCH) )
	{
		// Subtract a point for suicide in deathmatch modes.
		
		if( victim )
		{
			std::string score_type = "kills";
			if( ship->Class && ((ship->Class->Category == ShipClass::CATEGORY_CAPITAL) || (ship->Class->Category == ShipClass::CATEGORY_TARGET)) )
				score_type = "kills_c";
			else if( ship->IsMissionObjective || (ship->Group == 255) )
				score_type = "kills_c";
			
			SetPlayerProperty( victim, score_type, Num::ToString( victim->PropertyAsInt(score_type) - 1 ) );
		}
		else
			ShipScores[ ship->ID ] --;
		
		if( GameType == XWing::GameType::TEAM_DEATHMATCH )
			TeamScores[ ship->Team ] --;
	}
	
	TriggerEvent( MissionEvent::TRIGGER_ON_DESTROYED, ship, killer_obj, victim, killer );
}


void XWingServer::SendScores( void )
{
	Data.Lock.Lock();
	
	Data.Properties["team_score_rebel"]  = Num::ToString( TeamScores[ XWing::Team::REBEL  ] );
	Data.Properties["team_score_empire"] = Num::ToString( TeamScores[ XWing::Team::EMPIRE ] );
	Data.Properties["ai_score_name"]     = "";
	Data.Properties["ai_score_kills"]    = "";
	
	if( ((GameType == XWing::GameType::FFA_DEATHMATCH) || (GameType == XWing::GameType::FFA_ELIMINATION) || (GameType == XWing::GameType::FFA_RACE)) && ShipScores.size() )
	{
		int best_score = -99;
		const Ship *best_ship = NULL;
		for( std::map<uint32_t,int>::const_iterator score_iter = ShipScores.begin(); score_iter != ShipScores.end(); score_iter ++ )
		{
			if( score_iter->second > best_score )
			{
				const Ship *ship = (const Ship*) Data.GetObject( score_iter->first );
				if( ship && (ship->Type() == XWing::Object::SHIP) && ! ship->Owner() )
				{
					if( (GameType == XWing::GameType::FFA_ELIMINATION) && (ship->Health <= 0.) )
						continue;
					best_score = score_iter->second;
					best_ship = ship;
				}
			}
		}
		if( best_ship )
		{
			Data.Properties["ai_score_name"]  = best_ship->Name;
			Data.Properties["ai_score_kills"] = Num::ToString( best_score );
		}
	}
	
	Data.Lock.Unlock();
	
	Packet info( Raptor::Packet::INFO );
	info.AddUShort( 4 );
	info.AddString( "team_score_rebel" );
	info.AddString( Data.PropertyAsString("team_score_rebel") );
	info.AddString( "team_score_empire" );
	info.AddString( Data.PropertyAsString("team_score_empire") );
	info.AddString( "ai_score_name" );
	info.AddString( Data.PropertyAsString("ai_score_name") );
	info.AddString( "ai_score_kills" );
	info.AddString( Data.PropertyAsString("ai_score_kills") );
	Net.SendAll( &info );
	
	// Send time remaining updates if there is (or was) a time limit.
	static bool had_time_limit = true;
	if( TimeLimit || had_time_limit )
	{
		Packet time_remaining( XWing::Packet::TIME_REMAINING );
		time_remaining.AddFloat( TimeLimit ? RoundTimeRemaining() : 0.f );
		Net.SendAll( &time_remaining );
	}
	had_time_limit = TimeLimit;
}


double XWingServer::RoundTimeRemaining( void ) const
{
	if( TimeLimit )
		return TimeLimit * 60. - RoundTimer.ElapsedSeconds();
	
	return 60. * 60.;
}


const ShipClass *XWingServer::GetShipClass( const std::string &name ) const
{
	for( std::map<uint32_t,GameObject*>::const_iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->Type() == XWing::Object::SHIP_CLASS )
		{
			const ShipClass *sc = (const ShipClass*) obj_iter->second;
			if( strcasecmp( name.c_str(), sc->ShortName.c_str() ) == 0 )
				return sc;
			if( strcasecmp( name.c_str(), sc->LongName.c_str() ) == 0 )
				return sc;
		}
	}
	
	for( std::set<ShipClass>::const_iterator class_iter = ShipClasses.begin(); class_iter != ShipClasses.end(); class_iter ++ )
	{
		if( strcasecmp( name.c_str(), class_iter->ShortName.c_str() ) == 0 )
			return &*class_iter;
		if( strcasecmp( name.c_str(), class_iter->LongName.c_str() ) == 0 )
			return &*class_iter;
	}
	
	return NULL;
}


Ship *XWingServer::SpawnShip( const ShipClass *ship_class, uint8_t team, std::set<uint32_t> *add_object_ids )
{
	Ship *ship = new Ship( ship_class );
	ship->Team = team;
	ship->Data = &Data;
	Data.AddObject( ship );
	ShipScores[ ship->ID ] = 0;
	
	if( (GameType == XWing::GameType::TEAM_RACE) || (GameType == XWing::GameType::FFA_RACE) )
	{
		// FIXME: Should late joiners spawn with the pack instead of at first checkpoint?
		for( std::map<uint32_t,GameObject*>::const_iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::CHECKPOINT )
			{
				ship->NextCheckpoint = obj_iter->first;
				ship->Target = ship->NextCheckpoint;
				break;
			}
		}
	}
	
	if( add_object_ids )
		add_object_ids->insert( ship->ID );
	
	SpawnShipTurrets( ship, add_object_ids );
	SpawnShipDockingBays( ship );
	
	TriggerEvent( MissionEvent::TRIGGER_ON_SPAWN, ship );
	
	return ship;
}


void XWingServer::SpawnShipTurrets( const Ship *ship, std::set<uint32_t> *add_object_ids )
{
	if( ship->Class )
	{
		for( std::vector<ShipClassTurret>::const_iterator turret_iter = ship->Class->Turrets.begin(); turret_iter != ship->Class->Turrets.end(); turret_iter ++ )
		{
			Turret *turret = new Turret();
			turret->Data = &Data;
			turret->Visible = turret_iter->Visible;
			turret->CanBeHit = turret_iter->CanBeHit;
			if( turret_iter->Health )
				turret->SetHealth( turret_iter->Health );
			else if( ship->Class->TurretHealth )
				turret->SetHealth( ship->Class->TurretHealth );
			turret->ParentControl = turret_iter->ParentControl;
			turret->Manual = turret_iter->Manual;
			turret->Weapon = turret_iter->Weapon;
			
			if( ship->Team == XWing::Team::EMPIRE )
			{
				if( turret->Weapon == Shot::TYPE_TURBO_LASER_RED )
					turret->Weapon = Shot::TYPE_TURBO_LASER_GREEN;
				else if( turret->Weapon == Shot::TYPE_LASER_RED )
					turret->Weapon = Shot::TYPE_LASER_GREEN;
			}
			else if( ship->Team == XWing::Team::REBEL )
			{
				if( turret->Weapon == Shot::TYPE_TURBO_LASER_GREEN )
					turret->Weapon = Shot::TYPE_TURBO_LASER_RED;
				else if( turret->Weapon == Shot::TYPE_LASER_GREEN )
					turret->Weapon = Shot::TYPE_LASER_RED;
			}
			
			turret->SingleShotDelay = turret_iter->SingleShotDelay;
			turret->FiringMode = turret_iter->FiringMode;
			if( ! turret->FiringMode )
				turret->FiringMode = (turret->Visible && turret->GunWidth && (ship->Team == XWing::Team::EMPIRE)) ? 2 : 1;
			turret->TargetArc = turret_iter->TargetArc;
			turret->MinGunPitch = turret_iter->MinGunPitch;
			turret->MaxGunPitch = turret_iter->MaxGunPitch;
			Vec3D center( turret_iter->X, turret_iter->Y, turret_iter->Z );
			
			// FIXME: Add these to ShipClassTurret and check turret_iter?
			turret->YawSpeed   = ship->Class->TurretYawSpeed;
			turret->PitchSpeed = ship->Class->TurretPitchSpeed;
			turret->BodyModel  = ship->Class->TurretBody;
			turret->GunModel   = ship->Class->TurretGun;
			turret->Radius     = ship->Class->TurretRadius;
			turret->GunWidth   = ship->Class->TurretGunWidth;
			turret->GunUp      = ship->Class->TurretGunUp;
			turret->GunFwd     = ship->Class->TurretGunFwd;
			turret->HeadUp     = ship->Class->TurretHeadUp;
			turret->HeadFwd    = ship->Class->TurretHeadFwd;
			
			turret->Attach( ship, &center, &(turret_iter->Up), &(turret_iter->Fwd), turret_iter->ParentControl );
			Data.AddObject( turret );
			
			if( add_object_ids )
				add_object_ids->insert( turret->ID );
		}
	}
	
	std::map<std::string,ModelObject*>::const_iterator model_turrets = ship->Shape.Objects.find( "Turrets" );
	if( model_turrets != ship->Shape.Objects.end() )
	{
		Vec3D up( 0., 1., 0. );
		Vec3D fwd( 1., 0., 0. );
		for( std::vector<Vec3D>::const_iterator point_iter = model_turrets->second->Points.begin(); point_iter != model_turrets->second->Points.end(); point_iter ++ )
		{
			Turret *turret = new Turret();
			turret->Data = &Data;
			turret->Weapon = (ship->Team == XWing::Team::EMPIRE) ? Shot::TYPE_TURBO_LASER_GREEN : Shot::TYPE_TURBO_LASER_RED;
			if( turret->Weapon == Shot::TYPE_TURBO_LASER_RED )
				turret->FiringMode = 1;
			turret->Attach( ship, &(*point_iter), &up, &fwd, false );
			Data.AddObject( turret );
		}
		for( std::vector< std::vector<Vec3D> >::const_iterator line_iter = model_turrets->second->Lines.begin(); line_iter != model_turrets->second->Lines.end(); line_iter ++ )
		{
			if( line_iter->size() >= 2 )
			{
				up = (*line_iter).at( 1 ) - (*line_iter).at( 0 );
				up.ScaleTo( 1. );
				Turret *turret = new Turret();
				turret->Data = &Data;
				turret->Weapon = (ship->Team == XWing::Team::EMPIRE) ? Shot::TYPE_TURBO_LASER_GREEN : Shot::TYPE_TURBO_LASER_RED;
				if( turret->Weapon == Shot::TYPE_TURBO_LASER_RED )
					turret->FiringMode = 1;
				if( line_iter->size() >= 3 )
				{
					fwd = (*line_iter).at( 2 ) - (*line_iter).at( 0 );
					fwd.ScaleTo( 1. );
					turret->TargetArc = 180.;
				}
				if( ship->Class )
				{
					std::map<uint8_t,double>::const_iterator fire_time_iter = ship->Class->FireTime.find( turret->Weapon );
					if( fire_time_iter != ship->Class->FireTime.end() )
						turret->SingleShotDelay = fire_time_iter->second;
					
					if( ship->Class->TurretHealth )
						turret->SetHealth( ship->Class->TurretHealth );
					
					turret->YawSpeed   = ship->Class->TurretYawSpeed;
					turret->PitchSpeed = ship->Class->TurretPitchSpeed;
					turret->BodyModel  = ship->Class->TurretBody;
					turret->GunModel   = ship->Class->TurretGun;
					turret->GunUp      = ship->Class->TurretGunUp;
					turret->GunFwd     = ship->Class->TurretGunFwd;
					turret->HeadUp     = ship->Class->TurretHeadUp;
					turret->HeadFwd    = ship->Class->TurretHeadFwd;
				}
				
				turret->Attach( ship, &((*line_iter).at( 0 )), &up, &fwd, false );
				Data.AddObject( turret );
				
				if( add_object_ids )
					add_object_ids->insert( turret->ID );
			}
		}
	}
}


void XWingServer::SpawnShipDockingBays( const Ship *ship )
{
	if( ship->Class )
	{
		for( std::vector<ShipClassDockingBay>::const_iterator dock_iter = ship->Class->DockingBays.begin(); dock_iter != ship->Class->DockingBays.end(); dock_iter ++ )
		{
			DockingBay *dock = new DockingBay();
			dock->Radius = dock_iter->Radius;
			dock->Attach( ship, &*dock_iter );
			Data.AddObject( dock );
		}
	}
}


void XWingServer::SendAddedObjects( const std::set<uint32_t> *add_object_ids )
{
	if( ! add_object_ids )
		return;
	
	std::list<GameObject*> add_objects;
	
	for( std::set<uint32_t>::const_iterator id_iter = add_object_ids->begin(); id_iter != add_object_ids->end(); id_iter ++ )
	{
		GameObject *add_object = Data.GetObject( *id_iter );
		if( add_object && add_object->ServerShouldSend() )
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
}


void XWingServer::TriggerEvent( uint8_t trigger, const GameObject *object, const GameObject *by_obj, const Player *player, const Player *by_player )
{
	uint8_t team = XWing::Team::NONE;
	uint8_t group = 0;
	bool objective = false;
	uint16_t player_id = player ? player->ID : (object ? object->PlayerID : 0);
	if( ! player_id )
	{
		if( object && ! player )
			player = object->Owner();
		if( player )
			player_id = player->ID;
	}
	std::string name;
	
	if( object && (object->Type() == XWing::Object::SHIP) )
	{
		const Ship *ship = (const Ship*) object;
		team = ship->Team;
		group = ship->Group;
		objective = ship->IsMissionObjective;
		name = std::string("ship");
		if( ship->Class )
			name += std::string(":") + std::string(ship->Class->ShortName);
		if( ! player_id )
			name += std::string(":") + ship->Name;
	}
	else if( object && (object->Type() == XWing::Object::TURRET) )
	{
		const Turret *turret = (const Turret*) object;
		team = turret->Team;
		name = "turret";
	}
	
	uint32_t flags = 0;
	flags |= player_id ? MissionEvent::TRIGGERFLAG_PLAYER : MissionEvent::TRIGGERFLAG_AI;
	if( team == XWing::Team::REBEL )
		flags |= MissionEvent::TRIGGERFLAG_REBEL;
	else if( team == XWing::Team::EMPIRE )
		flags |= MissionEvent::TRIGGERFLAG_EMPIRE;
	if( objective )
		flags |= MissionEvent::TRIGGERFLAG_OBJECTIVE;
	
	std::string by_name;
	uint8_t by_group = 0;
	if( by_obj )
	{
		uint16_t by_player_id = by_player ? by_player->ID : by_obj->PlayerID;
		Turret *by_turret = (Turret*)( (by_obj->Type() == XWing::Object::TURRET) ? by_obj : NULL );
		Ship *by_ship = by_turret ? by_turret->ParentShip() : (Ship*)( (by_obj->Type() == XWing::Object::SHIP) ? by_obj : NULL );
		uint8_t by_team = (by_turret ? by_turret->Team : (by_ship ? by_ship->Team : (uint8_t)(XWing::Team::NONE)));
		
		if( by_ship )
		{
			by_group = by_ship->Group;
			by_name = "ship";
			if( by_ship->Class )
				by_name += std::string(":") + by_ship->Class->ShortName;
			if( ! by_player_id )
				by_name += std::string(":") + by_ship->Name;
		}
		if( by_turret )
			by_name = "turret";
		
		flags |= by_player_id ? MissionEvent::TRIGGERFLAG_BY_PLAYER : MissionEvent::TRIGGERFLAG_BY_AI;
		if( by_team == XWing::Team::REBEL )
			flags |= MissionEvent::TRIGGERFLAG_BY_REBEL;
		else if( by_team == XWing::Team::EMPIRE )
			flags |= MissionEvent::TRIGGERFLAG_BY_EMPIRE;
		if( by_ship && by_ship->IsMissionObjective )
			flags |= MissionEvent::TRIGGERFLAG_BY_OBJECTIVE;
		if( by_turret )
			flags |= MissionEvent::TRIGGERFLAG_BY_TURRET;
		else if( by_ship )
			flags |= MissionEvent::TRIGGERFLAG_BY_SHIP;
	}
	
	TriggerEvent( trigger, flags, name, group, by_name, by_group );
}


void XWingServer::TriggerEvent( uint8_t trigger, uint32_t flags, std::string target_name, uint8_t target_group, std::string by_name, uint8_t by_group )
{
	std::map< uint8_t, std::vector<MissionEvent> >::iterator trigger_iter = EventTriggers.find( trigger );
	if( trigger_iter != EventTriggers.end() )
	{
		for( std::vector<MissionEvent>::iterator event_iter = trigger_iter->second.begin(); event_iter != trigger_iter->second.end(); event_iter ++ )
			event_iter->Activated( flags, target_name, target_group, by_name, by_group );
	}
}


bool XWingServer::CheckCondition( const std::vector<std::string> &terms )
{
	if( terms.size() && (terms.at(0) == "(") )
	{
		// Allow simple boolean logic, ex: if ( rebel ships alive ) and ( player ships alive ) and ( no empire ships alive )
		
		size_t parens = 1, start = 1, unhandled = 0;
		bool eval = false;
		bool op_or = true;
		
		for( size_t i = 1; i < terms.size(); i ++ )
		{
			if( terms.at(i) == ")" )
			{
				parens --;
				if( (parens == 0) && ! (op_or && eval) )
				{
					std::vector<std::string> subvec( terms.begin() + start, terms.begin() + i );
					eval = CheckCondition( subvec );
					if( ! (op_or || eval) )  // if ( ... ) and ( false )
						return false;
				}
			}
			else if( terms.at(i) == "(" )
			{
				parens ++;
				if( parens == 1 )
					start = i + 1;
			}
			else if( parens )
				;
			else if( Str::EqualsInsensitive( terms.at(i), "and" ) )
			{
				if( ! eval )  // if ( false ) and ...
					return false;
				op_or = false;
			}
			else if( Str::EqualsInsensitive( terms.at(i), "or" ) )
			{
				if( eval )  // if ( true ) or ...
					return true;
				op_or = true;
			}
			else
			{
				unhandled = i;
				break;
			}
		}
		
		if( unhandled )
		{
			if( eval && op_or )  // if ( true ) or ...
				return true;
			if( ! (eval || op_or) )  // if ( false ) and ...
				return false;
			std::vector<std::string> subvec( terms.begin() + unhandled, terms.end() );
			return CheckCondition( subvec );
		}
		else
			return eval;
	}
	
	bool match_ships = false, negative = false;
	for( std::vector<std::string>::const_iterator condition = terms.begin(); condition != terms.end(); condition ++ )
	{
		if( Str::EqualsInsensitive( *condition, "no" ) || Str::EqualsInsensitive( *condition, "not" ) )
			negative = true;
		else if( Str::EqualsInsensitive( *condition, "ships" ) || Str::EqualsInsensitive( *condition, "here" ) || Str::EqualsInsensitive( *condition, "alive" ) || Str::EqualsInsensitive( *condition, "within" ) )
			match_ships = true;
	}
	
	if( match_ships )
		return (MatchingShips( terms ).empty() == negative);
	
	// FIXME: This could definitely be improved, such as supporting "X >= Y" etc.
	if( negative && (terms.size() == 3) )
		return ! Str::EqualsInsensitive( Data.PropertyAsString( terms.at(1) ), terms.at(2) );
	if( terms.size() == 2 )
		return Str::EqualsInsensitive( Data.PropertyAsString( terms.at(0) ), terms.at(1) );
	
	return true;
}


std::set<Ship*> XWingServer::MatchingShips( const std::vector<std::string> &terms )
{
	std::set<Ship*> matches;
	
	uint8_t team = XWing::Team::NONE;
	bool here = false, alive = false, dead = false, player = false, ai = false, objective = false, fighter = false, within_trench = false;
	uint8_t category = ShipClass::CATEGORY_UNKNOWN;
	const ShipClass *ship_class = NULL;
	std::string ship_name;
	double within_radius = 0.;
	std::vector<Pos3D> within_radius_of;
	for( std::vector<std::string>::const_iterator condition = terms.begin(); condition != terms.end(); condition ++ )
	{
		if( Str::EqualsInsensitive( *condition, "here" ) )
		{
			here = true;
			if( ! dead )
				alive = true;
		}
		else if( Str::EqualsInsensitive( *condition, "alive" ) )
			alive = true;
		else if( Str::EqualsInsensitive( *condition, "dead" ) )
			dead = true;
		else if( Str::EqualsInsensitive( *condition, "rebel" ) )
			team = XWing::Team::REBEL;
		else if( Str::EqualsInsensitive( *condition, "empire" ) )
			team = XWing::Team::EMPIRE;
		else if( Str::EqualsInsensitive( *condition, "player" ) || Str::EqualsInsensitive( *condition, "players" ) )
			player = true;
		else if( Str::EqualsInsensitive( *condition, "ai" ) )
			ai = true;
		else if( Str::EqualsInsensitive( *condition, "objective" ) || Str::EqualsInsensitive( *condition, "objectives" ) )
			objective = true;
		else if( Str::EqualsInsensitive( *condition, "fighter" ) || Str::EqualsInsensitive( *condition, "fighters" ) )
			fighter = true;
		else if( Str::EqualsInsensitive( *condition, "capital" ) )
			category = ShipClass::CATEGORY_CAPITAL;
		else if( Str::EqualsInsensitive( *condition, "transport" ) || Str::EqualsInsensitive( *condition, "transports" ) )
			category = ShipClass::CATEGORY_TRANSPORT;
		else if( Str::EqualsInsensitive( *condition, "target" ) )
			category = ShipClass::CATEGORY_TARGET;
		else if( Str::EqualsInsensitive( *condition, "within" ) )
		{
			for( condition ++; condition != terms.end(); condition ++ )
			{
				if( (*condition == "of") || (*condition == "from") )
				{
					condition ++;
					if( *condition == "checkpoint" )
					{
						for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
						{
							if( obj_iter->second->Type() == XWing::Object::CHECKPOINT )
								within_radius_of.push_back( obj_iter->second );
						}
					}
					else if( *condition == "point" )
					{
						Pos3D point;
						for( int i = 0; i < 3; i ++ )
						{
							condition ++;
							if( condition == terms.end() )
								return matches;
							
							const char *condition_cstr = condition->c_str();
							double value = atof( condition_cstr );
							if( strchr( condition_cstr, 'K' ) )
								value *= 1000.;
							
							if( (i%3) == 0 )
								point.X = value;
							else if( (i%3) == 1 )
								point.Y = value;
							else
								point.Z = value;
						}
						within_radius_of.push_back( point );
					}
					else
					{
						std::vector<std::string> subterms( condition, terms.end() );
						std::set<Ship*> matching_ships = MatchingShips( subterms );
						for( std::set<Ship*>::iterator ship_iter = matching_ships.begin(); ship_iter != matching_ships.end(); ship_iter ++ )
							within_radius_of.push_back( *ship_iter );
					}
					
					// If nothing exists matching the second condition, we cannot be near it.
					if( within_radius_of.empty() )
						return matches;
					
					break;  // Prevent double-parsing.
				}
				else if( *condition == "trench" )
					within_trench = true;
				else if( ! within_radius )
				{
					const char *condition_cstr = condition->c_str();
					within_radius = atof( condition_cstr );
					if( strchr( condition_cstr, 'K' ) )
						within_radius *= 1000.;
				}
				else if( *condition == "K" )
					within_radius *= 1000.;
			}
			
			break;  // Prevent double-parsing.
		}
		else if( ship_class && ship_name.empty() && ((*condition)[0] >= 'A') && ((*condition)[0] <= 'Z') )
			ship_name = *condition;
		else if( ! ship_class )
			ship_class = GetShipClass( *condition );
	}
	
	Data.Lock.Lock();
	DeathStar *deathstar = NULL;
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->Type() == XWing::Object::SHIP )
		{
			Ship *ship = (Ship*) obj_iter->second;
			
			if( here && ship->JumpedOut && (ship->JumpProgress >= 1.) )
				continue;
			if( alive && (ship->Health <= 0.) )
				continue;
			if( dead && (ship->Health > 0.) )
				continue;
			if( team && (ship->Team != team) )
				continue;
			if( ai && ship->Owner() )
				continue;
			if( player && ! ship->Owner() )
				continue;
			if( objective && ! ship->IsMissionObjective )
				continue;
			if( fighter && ((ship->Category() == ShipClass::CATEGORY_CAPITAL) || (ship->Category() == ShipClass::CATEGORY_TRANSPORT) || (ship->Category() == ShipClass::CATEGORY_TARGET)) )
				continue;
			if( category && (ship->Category() != category) )
				continue;
			if( within_trench && ! (deathstar && deathstar->WithinTrenchH( ship ) && deathstar->WithinTrenchW( ship )) )
				continue;
			if( ship_name.length() && ! Str::BeginsWith( ship->Name, ship_name ) )
				continue;
			
			bool matched_pos = within_radius_of.empty();
			for( std::vector<Pos3D>::const_iterator pos_iter = within_radius_of.begin(); pos_iter != within_radius_of.end(); pos_iter ++ )
			{
				if( pos_iter->Dist( ship ) <= within_radius )
				{
					matched_pos = true;
					break;
				}
			}
			if( ! matched_pos )
				continue;
			
			matches.insert( ship );
		}
		else if( obj_iter->second->Type() == XWing::Object::DEATH_STAR )
			deathstar = (DeathStar*) obj_iter->second;
	}
	Data.Lock.Unlock();
	
	return matches;
}


std::string XWingServer::ChosenShip( const Player *player, bool allow_spectator )
{
	if( ! player )
		return "";
	
	std::string ship = player->PropertyAsString("ship");
	
	if( Data.PropertyAsBool("darkside") || (Data.PropertyAsString("gametype") != "mission") )
		return ship;
	
	std::string player_ships = Data.PropertyAsString("player_ships");
	if( player_ships.empty() )
		player_ships = Data.PropertyAsString("player_ship");
	if( ! player_ships.empty() )
	{
		// If running a mission, make sure the selected ship is allowed.
		std::vector<std::string> allowed_ships = Str::SplitToVector( player_ships, " " );
		
		if( ship == "Spectator" )
			return allow_spectator ? ship : allowed_ships.at(0);
		if( ship.empty() )
		{
			std::string default_ship = allowed_ships.at(0);
			if( Str::EqualsInsensitive( default_ship, "rebel_gunner" ) )
				return "Rebel Gunner";
			else if( Str::EqualsInsensitive( default_ship, "empire_gunner" ) )
				return "Imperial Gunner";
			return default_ship;
		}
		
		bool gunner = (Str::FindInsensitive( ship, "Gunner" ) >= 0);
		
		for( std::vector<std::string>::const_iterator ship_iter = allowed_ships.begin(); ship_iter != allowed_ships.end(); ship_iter ++ )
		{
			if( Str::EqualsInsensitive( ship, *ship_iter ) )
				return ship;
			if( (ship == "Rebel Gunner") && Str::EqualsInsensitive( *ship_iter, "empire_gunner" ) )
				return "Imperial Gunner";
			if( (ship == "Imperial Gunner") && Str::EqualsInsensitive( *ship_iter, "rebel_gunner" ) )
				return "Rebel Gunner";
			if( gunner && (Str::FindInsensitive( *ship_iter, "gunner" ) >= 0) )
			{
				std::string player_team = Data.PropertyAsString("player_team");
				if( Str::EqualsInsensitive( player_team, "rebel" ) )
					return "Rebel Gunner";
				if( Str::EqualsInsensitive( player_team, "empire" ) )
					return "Imperial Gunner";
				return ship;
			}
		}
		
		return allowed_ships.at(0);
	}
	
	return ship;
}


std::string XWingServer::ChosenTeam( const Player *player, bool allow_spectator )
{
	if( ! player )
		return "";
	
	std::string team = player->PropertyAsString("team");
	if( Data.PropertyAsString("gametype") != "mission" )
		return team;
	
	std::string player_team = Data.PropertyAsString("player_team");
	if( ! player_team.empty() )
	{
		// If running a mission, make sure the team is correct.
		if( (team == "Spectator") && allow_spectator )
			return team;
		if( Str::EqualsInsensitive( player_team, "rebel" ) )
			return "Rebel";
		if( Str::EqualsInsensitive( player_team, "empire" ) )
			return "Empire";
		return player_team;
	}
	
	return team;
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
