/*
 *  Mission.cpp
 */

#include "Mission.h"

#include "XWingServer.h"
#include "Str.h"
#include "Num.h"
#include <fstream>
#include <algorithm>


bool Mission::Load( const std::string &filename )
{
	Properties.clear();
	Events.clear();
	
	std::ifstream input( filename.c_str() );
	if( ! input.is_open() )
		return false;
	
	bool reading_properties = true;
	uint8_t event_trigger = MissionEvent::TRIGGER_NEVER;
	uint16_t event_trigger_flags = 0x0000;
	double event_time = 0.;
	size_t event_number = 0;
	std::string event_target;
	std::vector<std::string> event_if;
	uint8_t event_target_group = 0;
	double event_delay = 0.;
	double event_chance = 1.;
	
	char buffer[ 1024 ] = "";
	while( ! input.eof() )
	{
		buffer[ 0 ] = '\0';
		input.getline( buffer, sizeof(buffer) );
		
		std::list<std::string> parsed = CStr::ParseCommand( buffer );
		if( ! parsed.size() )
			continue;
		
		std::string var = *(parsed.begin());
		std::string unmodified_var = var;
		std::transform( var.begin(), var.end(), var.begin(), tolower );
		parsed.erase( parsed.begin() );
		std::vector<std::string> args( parsed.begin(), parsed.end() );
		
		bool event_trigger_line = (var == "at") || (var == "on") || (var == "when") || (var == "while") || strchr( var.c_str() , ':' );
		
		if( reading_properties && args.size() && ! event_trigger_line )
		{
			// Missions begin with property definitions.
			
			std::string value = args.at(0);
			for( size_t i = 1; i < args.size(); i ++ )
				value += std::string(" ") + args.at(i);
			
			Properties[ var ] = value;
		}
		else if( event_trigger_line )
		{
			// Set triggers for the events that follow.
			
			reading_properties = false;
			event_trigger = MissionEvent::TRIGGER_ALWAYS;
			event_trigger_flags = 0x00;
			event_time = 0.;
			event_number = 0;
			event_target.clear();
			event_target_group = 0;
			event_if.clear();
			event_delay = 0.;
			event_chance = 1.;
			bool conditional = false;
			bool line_first_word = true;
			
			while( var.length() )
			{
				if( strchr( var.c_str() , ':' ) && ! event_time )
				{
					std::vector<std::string> time = Str::SplitToVector( var, ":" );
					for( size_t i = 0; i < time.size(); i ++ )
					{
						event_time *= 60.;
						event_time += atof( time.at(i).c_str() );
					}
				}
				else if( var == "every" )
				{
					event_trigger_flags |= MissionEvent::TRIGGERFLAG_REPEAT;
				}
				else if( (var == "remaining") && ! conditional )
				{
					event_trigger_flags |= MissionEvent::TRIGGERFLAG_TIME_REMAINING;
				}
				else if( var == "on" )
				{
					conditional = true;
					event_trigger = MissionEvent::TRIGGER_NEVER;
				}
				else if( ((var == "when") || (var == "if") || (var == "while")) && args.size() )
				{
					event_if = args;
					args.clear();
					if( var == "while" )
					{
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_RECHECK_IF;
						if( line_first_word )
							event_trigger_flags |= MissionEvent::TRIGGERFLAG_REPEAT;
					}
				}
				else if( conditional )
				{
					if( var == "begin" )
					{
						conditional = false;
						event_trigger = MissionEvent::TRIGGER_ALWAYS;
					}
					else if( var == "rebel" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_ONLY_REBEL;
					else if( var == "empire" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_ONLY_EMPIRE;
					else if( var == "player" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_ONLY_PLAYER;
					else if( var == "ai" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_ONLY_AI;
					else if( var == "objective" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_ONLY_OBJECTIVE;
					else if( var == "victory" )
						event_trigger = MissionEvent::TRIGGER_ON_VICTORY;
					else if( var == "defeat" )
						event_trigger = MissionEvent::TRIGGER_ON_DEFEAT;
					else if( var == "checkpoint" )
						event_trigger = MissionEvent::TRIGGER_ON_CHECKPOINT;
					else if( var == "fire" )
						event_trigger = MissionEvent::TRIGGER_ON_FIRE;
					else if( var == "hit" )
						event_trigger = MissionEvent::TRIGGER_ON_HIT;
					else if( (var == "damage") || (var == "damaged") )
						event_trigger = MissionEvent::TRIGGER_ON_DAMAGE;
					else if( var == "destroyed" )
						event_trigger = MissionEvent::TRIGGER_ON_DESTROYED;
					else if( var.at(0) == '#' )
						event_number = atoi( var.c_str() + 1 );
					else if( (var == "target") && args.size() )
					{
						event_target = args.at(0);
						args.erase( args.begin() );
					}
					else if( (var == "group") && args.size() )
					{
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_ONLY_GROUP;
						event_target_group = atoi( args.at(0).c_str() );
						args.erase( args.begin() );
					}
					else if( event_trigger && event_target.empty() )
						event_target = unmodified_var;
				}
				
				if( args.size() )
				{
					var = *(args.begin());
					unmodified_var = var;
					std::transform( var.begin(), var.end(), var.begin(), tolower );
					args.erase( args.begin() );
				}
				else
					var.clear();
				
				line_first_word = false;
			}
		}
		else if( (var == "wait") && args.size() )
		{
			event_delay += atof( args.at(0).c_str() );
		}
		else if( event_trigger )
		{
			// Handle events listed after a trigger.
			
			Events.push_back( MissionEvent( event_trigger, event_trigger_flags, event_time, event_delay, event_number, event_target, event_target_group, event_if, event_chance ) );
			
			if( (var == "alert") && (args.size() >= 2) )
			{
				Events.back().Sound = args.at(0);
				std::string message = args.at(1);
				for( size_t i = 2; i < args.size(); i ++ )
					message += std::string(" ") + args.at(i);
				Events.back().Message = message;
			}
			else if( ((var == "message") || (var == "chat")) && args.size() )
			{
				std::string message = args.at(0);
				for( size_t i = 1; i < args.size(); i ++ )
					message += std::string(" ") + args.at(i);
				Events.back().Message = message;
				if( var == "chat" )
					Events.back().MessageType = TextConsole::MSG_CHAT;
			}
			else if( (var == "sound") && args.size() )
			{
				Events.back().Sound = args.at(0);
			}
			else if( var == "victory" )
			{
				Events.back().PropertyName = "victor";
				Events.back().PropertyValue = "player";
			}
			else if( var == "defeat" )
			{
				Events.back().PropertyName = "victor";
				Events.back().PropertyValue = "ai";
			}
			else if( (var == "spawn") && args.size() )
			{
				std::string spawn_class, spawn_name, message;
				bool auto_pos = true;
				double x = 0., y = 0., z = 0.;
				double fwd_x = 0., fwd_y = 0., fwd_z = 0.;
				uint8_t spawn_group = 0, spawn_flags = 0x00;
				
				while( args.size() )
				{
					std::string subvar = *(args.begin());
					unmodified_var = subvar;
					std::transform( subvar.begin(), subvar.end(), subvar.begin(), tolower );
					args.erase( args.begin() );
					
					if( subvar == "rebel" )
						spawn_flags |= MissionEvent::SPAWNFLAG_REBEL;
					else if( subvar == "empire" )
						spawn_flags |= MissionEvent::SPAWNFLAG_EMPIRE;
					else if( subvar == "objective" )
						spawn_flags |= MissionEvent::SPAWNFLAG_OBJECTIVE;
					/*
					else if( subvar == "hero" )
						spawn_flags |= MissionEvent::SPAWNFLAG_HERO;
					*/
					else if( (subvar == "class") && args.size() )
					{
						spawn_class = args.at(0);
						args.erase( args.begin() );
					}
					else if( (subvar == "name") && args.size() )
					{
						spawn_name = args.at(0);
						args.erase( args.begin() );
					}
					else if( (subvar == "at") && (args.size() >= 3) )
					{
						// FIXME: Allow random ranges.
						x = atof( args.at(0).c_str() );
						y = atof( args.at(1).c_str() );
						z = atof( args.at(2).c_str() );
						args.erase( args.begin() );
						args.erase( args.begin() );
						args.erase( args.begin() );
						auto_pos = false;
					}
					else if( subvar == "from" && args.size() )
					{
						if( Str::EqualsInsensitive( args.at(0), "player" ) )
							spawn_flags |= MissionEvent::SPAWNFLAG_BY_PLAYER;
						// FIXME: Other targets this position could be relative to?
						args.erase( args.begin() );
					}
					else if( (subvar == "facing") && (args.size() >= 3) )
					{
						fwd_x = atof( args.at(0).c_str() );
						fwd_y = atof( args.at(1).c_str() );
						fwd_z = atof( args.at(2).c_str() );
						args.erase( args.begin() );
						args.erase( args.begin() );
						args.erase( args.begin() );
					}
					else if( (subvar == "group") && args.size() )
					{
						spawn_group = atoi( args.at(0).c_str() );
					}
					else if( (subvar == "message") && args.size() )
					{
						message = args.at(0);
						args.erase( args.begin() );
					}
					else if( spawn_class.empty() && ((XWingServer*)( Raptor::Server ))->GetShipClass(unmodified_var) )
						spawn_class = unmodified_var;
					else if( spawn_name.empty() && ! spawn_class.empty() )
						spawn_name = unmodified_var;
				}
				
				if( auto_pos )
				{
					if( spawn_flags & MissionEvent::SPAWNFLAG_BY_PLAYER )
						y = 100.;
					else
					{
						// FIXME: Automatic position!
					}
				}
				
				if( !(fwd_x || fwd_y || fwd_z) )
				{
					// FIXME: Automatic heading!
				}
				
				Events.back().SpawnClass = spawn_class;
				Events.back().SpawnName  = spawn_name;
				Events.back().SpawnGroup = spawn_group;
				Events.back().SpawnFlags = spawn_flags;
				Events.back().X = x;
				Events.back().Y = y;
				Events.back().Z = z;
				Events.back().FwdX = fwd_x;
				Events.back().FwdY = fwd_y;
				Events.back().FwdZ = fwd_z;
				Events.back().Message = message;
			}
			else if( (var == "jump") && args.size() )
			{
				Events.back().JumpOut = args;
				args.clear();
			}
			else if( (var == "reset") && args.size() )
			{
				Events.back().PropertyName = args.at(0);
				// FIXME: Should there be a better indicator for reset?
			}
			else if( args.size() )
			{
				// Events can change properties too.
				
				Events.back().PropertyName = var;
				std::string value = args.at(0);
				for( size_t i = 1; i < args.size(); i ++ )
					value += std::string(" ") + args.at(i);
				Events.back().PropertyValue = value;
			}
		}
	}
	input.close();
	
	// Return true if we loaded at least the gametype.
	return (Properties.find("gametype") != Properties.end());
}



MissionEvent::MissionEvent( uint8_t trigger, uint16_t trigger_flags, double time, double delay, size_t number, std::string target, uint8_t target_group, std::vector<std::string> trigger_if, double chance )
{
	Trigger = trigger;
	TriggerFlags = trigger_flags;
	Time = time;
	Delay = delay;
	Number = number;
	Target = target;
	TargetGroup = target_group;
	TriggerIf = trigger_if;
	Chance = chance;
	
	MessageType = TextConsole::MSG_NORMAL;
	X = Y = Z = FwdX = FwdY = FwdZ = 0.;
	SpawnGroup = 0;
	SpawnFlags = 0;
	
	Triggered = Used = 0;
	GoTime = 0.;
}


MissionEvent::MissionEvent( const MissionEvent &other )
{
	Trigger = other.Trigger;
	TriggerFlags = other.TriggerFlags;
	Time = other.Time;
	Delay = other.Delay;
	Number = other.Number;
	Target = other.Target;
	TargetGroup = other.TargetGroup;
	TriggerIf = other.TriggerIf;
	Chance = other.Chance;
	
	Message = other.Message;
	MessageType = other.MessageType;
	Sound = other.Sound;
	
	SpawnClass = other.SpawnClass;
	SpawnName  = other.SpawnName;
	SpawnGroup = other.SpawnGroup;
	SpawnFlags = other.SpawnFlags;
	X = other.X;
	Y = other.Y;
	Z = other.Z;
	FwdX = other.FwdX;
	FwdY = other.FwdY;
	FwdZ = other.FwdZ;
	
	JumpOut = other.JumpOut;
	
	PropertyName = other.PropertyName;
	PropertyValue = other.PropertyValue;
	
	Triggered = other.Triggered;
	Used = other.Used;
	GoTime = other.GoTime;
}


MissionEvent::~MissionEvent()
{
}


bool MissionEvent::MatchesConditions( uint8_t team, uint8_t group, bool objective, uint16_t player_id, std::string name ) const
{
	XWingServer *server = (XWingServer*)( Raptor::Server );
	
	if( Triggered && (Triggered >= Number) && ! (TriggerFlags & TRIGGERFLAG_REPEAT) )
		return false;
	
	if( Time && (TriggerFlags & TRIGGERFLAG_TIME_REMAINING) )
	{
		if( ! server->TimeLimit )
			return false;
		double remaining = server->RoundTimeRemaining();
		if( remaining > Time )
			return false;
	}
	else if( Time )
	{
		double elapsed = server->RoundTimer.ElapsedSeconds();
		if( elapsed < Time )
			return false;
	}
	
	if( (TriggerFlags & TRIGGERFLAG_ONLY_PLAYER) && ! player_id )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_ONLY_AI) && player_id )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_ONLY_REBEL) && (team != XWing::Team::REBEL) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_ONLY_EMPIRE) && (team != XWing::Team::EMPIRE) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_ONLY_OBJECTIVE) && ! objective )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_ONLY_GROUP) && (group != TargetGroup) )
		return false;
	
	if( ! (Target.empty() || Str::EqualsInsensitive( Target, name )) )
		return false;
	
	return true;
}


void MissionEvent::Activated( uint8_t team, uint8_t group, bool objective, uint16_t player_id, std::string name )
{
	XWingServer *server = (XWingServer*)( Raptor::Server );
	
	if( ! MatchesConditions( team, group, objective, player_id, name ) )
		return;
	if( ! Ready() )
		return;
	
	if( Triggered == Used )
		GoTime = server->RoundTimer.ElapsedSeconds() + Delay;
	
	Triggered ++;
}


bool MissionEvent::Ready( void )
{
	XWingServer *server = (XWingServer*)( Raptor::Server );
	
	if( Used && (Used >= Number) && ! (TriggerFlags & TRIGGERFLAG_REPEAT) )
		return false;
	
	if( TriggerIf.size() && ! server->CheckCondition( TriggerIf ) )
		return false;
	
	if( Trigger == TRIGGER_ALWAYS )
	{
		// Repeating timed events should not overlap.
		if( Triggered && (Triggered >= Number) && (Triggered > Used) )
			return false;
		
		// Delay for sum of preceeding "wait" and/or repeated trigger such as "at every 0:30".
		if( GoTime && (server->RoundTimer.ElapsedSeconds() < GoTime) )
			return false;
		
		if( Time && (TriggerFlags & TRIGGERFLAG_TIME_REMAINING) )
		{
			if( ! server->TimeLimit )
				return false;
			double remaining = server->RoundTimeRemaining();
			if( remaining > Time )
				return false;
		}
		else if( Time )
		{
			double elapsed = server->RoundTimer.ElapsedSeconds();
			if( elapsed < Time )
				return false;
		}
	}
	else if( (Used < Triggered) && (Triggered >= Number) )
	{
		if( ! Delay )
			return true;
		else if( GoTime )
			return (server->RoundTimer.ElapsedSeconds() >= GoTime);
		else
		{
			GoTime = server->RoundTimer.ElapsedSeconds() + Delay;
			return false;
		}
	}
	
	return true;
}


void MissionEvent::FireWhenReady( std::set<uint32_t> *add_object_ids )
{
	XWingServer *server = (XWingServer*)( Raptor::Server );
	
	if( ! Ready() )
		return;
	
	if( Trigger == TRIGGER_ALWAYS )
	{
		if( Used && (Used >= Number) && ! (TriggerFlags & TRIGGERFLAG_REPEAT) )
			return;
		
		if( Delay && ! (GoTime || Used) )
		{
			GoTime = server->RoundTimer.ElapsedSeconds() + Delay;
			return;
		}
		
		// Because no event fires this trigger, we do it here (via XWingServer::Update).
		Triggered ++;
	}
	else if( Used >= Triggered )
		return;
	
	Used ++;
	
	if( TriggerFlags & TRIGGERFLAG_REPEAT )
		GoTime = server->RoundTimer.ElapsedSeconds() + Delay + Time;
	
	if( Used < Number )
		return;
	
	// FIXME: This implementation currently gives separate chances to each event in a chain!
	if( (Chance < 1.) && ! Rand::Bool(Chance) )
		return;
	
	if( (TriggerFlags & TRIGGERFLAG_RECHECK_IF) && ! server->CheckCondition( TriggerIf ) )
		return;
	
	if( Sound.length() )
	{
		Packet play_sound( Raptor::Packet::PLAY_SOUND );
		play_sound.AddString( Sound );
		play_sound.AddChar( Num::UnitFloatTo8( 0.25 ) );
		play_sound.AddChar( Num::UnitFloatTo8( 0.5 ) );
		server->Net.SendAll( &play_sound );
	}
	
	if( Message.length() )
	{
		Packet message( Raptor::Packet::MESSAGE );
		message.AddString( Message );
		message.AddUInt( MessageType );
		server->Net.SendAll( &message );
	}
	
	if( PropertyName.length() )
	{
		std::string value = PropertyValue;
		if( value.empty() )  // FIXME: Should there be a better indicator for reset?
		{
			std::map<std::string,std::string>::const_iterator property_iter = server->Properties.find( PropertyName );
			if( property_iter != server->Properties.end() )
				value = property_iter->second;
		}
		
		if( Str::EqualsInsensitive( PropertyName, "gametype" ) )
		{
			// NOTE: Property "gametype" remains "mission".
			uint32_t gametype = server->ParseGameType( value );
			if( gametype )
			{
				server->GameType = server->ParseGameType( value );
				
				Packet change_gametype( XWing::Packet::GAMETYPE );
				change_gametype.AddUInt( server->GameType );
				server->Net.SendAll( &change_gametype );
			}
		}
		else if( Str::EqualsInsensitive( PropertyName, "time_limit" ) )
		{
			server->TimeLimit = atof( value.c_str() );
			if( server->TimeLimit )
				server->TimeLimit += server->RoundTimer.ElapsedSeconds();
			
			server->SendScores();  // This sends TIME_REMAINING packet.
		}
		else if( Str::EqualsInsensitive( PropertyName, "kill_limit" ) )
		{
			server->KillLimit = atoi( value.c_str() );
		}
		else if( Str::EqualsInsensitive( PropertyName, "end_delay" ) )
		{
			server->Data.SetProperty( PropertyName, value );
			server->RoundEndedDelay = atof( value.c_str() );
		}
		else if( Str::EqualsInsensitive( PropertyName, "team_score_rebel" ) )
		{
			server->TeamScores[ XWing::Team::REBEL ] = atoi( value.c_str() );
			server->SendScores();
		}
		else if( Str::EqualsInsensitive( PropertyName, "team_score_empire" ) )
		{
			server->TeamScores[ XWing::Team::EMPIRE ] = atoi( value.c_str() );
			server->SendScores();
		}
		else if( Str::EqualsInsensitive( PropertyName, "next_mission" ) )
		{
			server->Data.SetProperty( PropertyName, value );
			
			Packet mission_complete( XWing::Packet::MISSION_COMPLETE );
			mission_complete.AddString( server->Data.PropertyAsString("mission") );
			mission_complete.AddString( PropertyValue );
			server->Net.SendAll( &mission_complete );
		}
		else
			server->Data.SetProperty( PropertyName, value );
	}
	
	if( SpawnClass.length() )
	{
		const ShipClass *sc = server->GetShipClass( SpawnClass );
		if( sc )
		{
			uint8_t team = sc->Team;
			if( (SpawnFlags & (SPAWNFLAG_REBEL | SPAWNFLAG_EMPIRE)) == SPAWNFLAG_REBEL )
				team = XWing::Team::REBEL;
			else if( (SpawnFlags & (SPAWNFLAG_REBEL | SPAWNFLAG_EMPIRE)) == SPAWNFLAG_EMPIRE )
				team = XWing::Team::EMPIRE;
			
			Ship *ship = server->SpawnShip( sc, team, add_object_ids );
			ship->Group = SpawnGroup;
			ship->IsMissionObjective = SpawnFlags & SPAWNFLAG_OBJECTIVE;
			// FIXME: SpawnFlags & SPAWNFLAG_HERO
			
			if( SpawnName.length() )
				ship->Name = SpawnName;
			else if( sc->Squadron.length() )
			{
				ship->Name = sc->Squadron + std::string(" ") + Num::ToString( (int) server->Squadrons[ sc->Squadron ].size() + 1 );
				server->Squadrons[ sc->Squadron ].insert( ship->ID );
			}
			else
				ship->Name = sc->LongName;
			
			if( SpawnFlags & SPAWNFLAG_BY_PLAYER )
			{
				server->Data.Lock.Lock();
				
				const GameObject *player_object = NULL;
				for( std::map<uint32_t,GameObject*>::const_iterator obj_iter = server->Data.GameObjects.begin(); obj_iter != server->Data.GameObjects.end(); obj_iter ++ )
				{
					if( obj_iter->second->PlayerID || obj_iter->second->Owner() )
					{
						player_object = obj_iter->second;
						if( player_object->Type() == XWing::Object::SHIP )
						{
							const Ship *player_ship = (const Ship*) player_object;
							if( player_ship->Health > 0. )
								break;  // Spawn relative to the first alive player ship.
						}
					}
				}
				
				if( player_object )
				{
					X += player_object->X;
					Y += player_object->Y;
					Z += player_object->Z;
				}
				
				server->Data.Lock.Unlock();
			}
			
			ship->SetPos( X, Y, Z );
			if( FwdX || FwdY || FwdZ )
				ship->SetFwdVec( FwdX, FwdY, FwdZ );
		}
	}
	
	std::set<Ship*> jumping_out, jumping_in;
	
	if( JumpOut.size() == 1 )
	{
		// Match ships by name.
		std::string match_name = JumpOut.at(0);
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = server->Data.GameObjects.begin(); obj_iter != server->Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHIP )
			{
				Ship *ship = (Ship*) obj_iter->second;
				if( Str::BeginsWith( ship->Name, match_name ) )
					jumping_out.insert( ship );
			}
		}
	}
	else if( JumpOut.size() )
	{
		if( std::find( JumpOut.begin(), JumpOut.end(), "here" ) == JumpOut.end() )
			JumpOut.push_back("here");
		
		jumping_out = server->MatchingShips( JumpOut );
	}
	
	// FIXME: Stagger?
	for( std::set<Ship*>::iterator ship_iter = jumping_out.begin(); ship_iter != jumping_out.end(); ship_iter ++ )
	{
		(*ship_iter)->JumpedOut = true;
		(*ship_iter)->JumpProgress = 0.;
		(*ship_iter)->Lifetime.Reset();
		(*ship_iter)->MotionVector = (*ship_iter)->Fwd * (*ship_iter)->MaxSpeed();
		
		Packet jump_out( XWing::Packet::JUMP_OUT );
		jump_out.AddUInt( (*ship_iter)->ID );
		server->Net.SendAll( &jump_out );
	}
	
	for( std::set<Ship*>::iterator ship_iter = jumping_in.begin(); ship_iter != jumping_in.end(); ship_iter ++ )
	{
		(*ship_iter)->JumpedOut = false;
		(*ship_iter)->JumpProgress = 0.;
		(*ship_iter)->Lifetime.Reset();
		(*ship_iter)->MotionVector = (*ship_iter)->Fwd * (*ship_iter)->MaxSpeed();
		
		(*ship_iter)->SendUpdate( 126 );  // FIXME: Does this do the needful?
	}
}
