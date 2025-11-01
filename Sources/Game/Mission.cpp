/*
 *  Mission.cpp
 */

#include "Mission.h"

#include "XWingServer.h"
#include "Str.h"
#include "Num.h"
#include "File.h"
#include <fstream>
#include <algorithm>


bool Mission::Load( const std::string &filename )
{
	std::vector<std::string> lines = File::AsLines( filename.c_str() );
	return Parse( lines );
}


bool Mission::Parse( const std::string &content )
{
	std::vector<std::string> lines = Str::SplitToVector( content, "\n" );
	return Parse( lines );
}


bool Mission::Parse( std::vector<std::string> &lines )
{
	Properties.clear();
	Events.clear();
	
	bool reading_properties = true;
	uint8_t event_trigger = MissionEvent::TRIGGER_NEVER;
	uint32_t event_trigger_flags = 0x00000000;
	double event_time = 0.;
	size_t event_number = 0;
	std::string event_target;
	std::string event_by_name;
	std::vector<std::string> event_if;
	uint8_t event_target_group = 0;
	uint8_t event_by_group = 0;
	double event_delay = 0.;
	double event_chance = 1.;
	
	for( size_t i = 0; i < lines.size(); i ++ )
	{
		std::list<std::string> parsed = Str::ParseCommand( lines.at( i ) );
		if( ! parsed.size() )
			continue;
		
		std::string var = *(parsed.begin());
		std::string unmodified_var = var;
		std::transform( var.begin(), var.end(), var.begin(), tolower );
		parsed.erase( parsed.begin() );
		std::vector<std::string> args( parsed.begin(), parsed.end() );
		
		/*
		// Separate any grouped parentheses into their own tokens.
		// FIXME: Commented-out to avoid possible infinite loop.
		//        This means for now, missions must have parens ( spaced out ) !
		for( size_t i = 0; i < args.size(); i ++ )
		{
			while( (args[ i ].length() > 1) && (args[ i ][ 0 ] == '(') )
			{
				args.insert( args.begin() + i, "(" );
				args[ i + 1 ].erase( args[ i + 1 ].begin() );
				i ++;
			}
			while( (args[ i ].length() > 1) && (args[ i ][ args[ i ].length() - 1 ] == ')') )
			{
				args[ i ].erase( args[ i ].rbegin().base() );
				args.insert( args.begin() + i + 1, ")" );
			}
		}
		*/
		
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
			event_by_name.clear();
			event_target_group = 0;
			event_by_group = 0;
			event_if.clear();
			event_delay = 0.;
			event_chance = 1.;
			bool conditional = false;
			bool trigger_by = false;
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
					if( (args.size() > 1) && (args.at(0) == "always") )
					{
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_RECHECK_IF;
						args.erase( args.begin() );
					}
					
					event_if = args;
					args.clear();
					
					if( var == "while" )
					{
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_RECHECK_IF;
						if( line_first_word )
							event_trigger_flags |= MissionEvent::TRIGGERFLAG_REPEAT;
					}
				}
				else if( trigger_by )
				{
					if( var == "rebel" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_BY_REBEL;
					else if( var == "empire" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_BY_EMPIRE;
					else if( var == "player" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_BY_PLAYER;
					else if( var == "ai" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_BY_AI;
					else if( var == "objective" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_BY_OBJECTIVE;
					else if( var == "turret" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_BY_TURRET;
					else if( var == "ship" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_BY_SHIP;
					else if( (var == "group") && args.size() )
					{
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_BY_GROUP;
						event_by_group = atoi( args.at(0).c_str() );
						args.erase( args.begin() );
					}
					else if( (var.at(0) == '#') && ! event_number )
						event_number = atoi( var.c_str() + 1 );
					else if( event_trigger && event_by_name.empty() )
						event_by_name = unmodified_var;
				}
				else if( conditional )
				{
					if( var == "begin" )
					{
						conditional = false;
						event_trigger = MissionEvent::TRIGGER_ALWAYS;
					}
					else if( var == "by" )
						trigger_by = true;
					else if( var == "rebel" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_REBEL;
					else if( var == "empire" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_EMPIRE;
					else if( var == "player" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_PLAYER;
					else if( var == "ai" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_AI;
					else if( var == "objective" )
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_OBJECTIVE;
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
					else if( var == "spawn" )
						event_trigger = MissionEvent::TRIGGER_ON_SPAWN;
					else if( var == "respawn" )
						event_trigger = MissionEvent::TRIGGER_ON_RESPAWN;
					else if( var.at(0) == '#' )
						event_number = atoi( var.c_str() + 1 );
					else if( (var == "target") && args.size() )
					{
						event_target = args.at(0);
						args.erase( args.begin() );
					}
					else if( (var == "group") && args.size() )
					{
						event_trigger_flags |= MissionEvent::TRIGGERFLAG_GROUP;
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
			
			Events.push_back( MissionEvent( event_trigger, event_trigger_flags, event_time, event_delay, event_number, event_target, event_target_group, event_if, event_chance, event_by_name, event_by_group ) );
			
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
					else if( subvar == "silent" )
						spawn_flags |= MissionEvent::SPAWNFLAG_SILENT;
					else if( subvar == "respawn" )
						spawn_flags |= MissionEvent::SPAWNFLAG_RESPAWN;
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
						args.erase( args.begin() );
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
	
	// Return true if we loaded at least the gametype.
	return (Properties.find("gametype") != Properties.end());
}


std::string Mission::PropertyAsString( const std::string &name, const std::string &ifndef ) const
{
	std::map<std::string,std::string>::const_iterator property_iter = Properties.find( name );
	if( property_iter != Properties.end() )
		return property_iter->second;
	return ifndef;
}


// ---------------------------------------------------------------------------


MissionEvent::MissionEvent( uint8_t trigger, uint32_t trigger_flags, double time, double delay, size_t number, std::string target, uint8_t target_group, std::vector<std::string> trigger_if, double chance, std::string by_name, uint8_t by_group )
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
	ByName = by_name;
	ByGroup = by_group;
	
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
	ByName = other.ByName;
	ByGroup = other.ByGroup;
	
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


bool MissionEvent::MatchesConditions( uint32_t flags, std::string target_name, uint8_t target_group, std::string by_name, uint8_t by_group ) const
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
	
	if( (TriggerFlags & TRIGGERFLAG_PLAYER      ) && ! (flags & TRIGGERFLAG_PLAYER) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_AI          ) && ! (flags & TRIGGERFLAG_AI) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_REBEL       ) && ! (flags & TRIGGERFLAG_REBEL) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_EMPIRE      ) && ! (flags & TRIGGERFLAG_EMPIRE) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_OBJECTIVE   ) && ! (flags & TRIGGERFLAG_OBJECTIVE) )
		return false;
	
	if( (TriggerFlags & TRIGGERFLAG_BY_PLAYER   ) && ! (flags & TRIGGERFLAG_BY_PLAYER) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_BY_AI       ) && ! (flags & TRIGGERFLAG_BY_AI) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_BY_REBEL    ) && ! (flags & TRIGGERFLAG_BY_REBEL) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_BY_EMPIRE   ) && ! (flags & TRIGGERFLAG_BY_EMPIRE) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_BY_OBJECTIVE) && ! (flags & TRIGGERFLAG_BY_OBJECTIVE) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_BY_TURRET   ) && ! (flags & TRIGGERFLAG_BY_TURRET) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_BY_SHIP     ) && ! (flags & TRIGGERFLAG_BY_SHIP) )
		return false;
	
	if( (TriggerFlags & TRIGGERFLAG_GROUP       ) && (target_group != TargetGroup) )
		return false;
	if( (TriggerFlags & TRIGGERFLAG_BY_GROUP    ) && (by_group != ByGroup) )
		return false;
	
	if( ! (Target.empty() || Str::ContainsInsensitive( target_name, Target )) )
		return false;
	if( ! (ByName.empty() || Str::ContainsInsensitive( by_name, ByName )) )
		return false;
	
	return true;
}


void MissionEvent::Activated( uint32_t flags, std::string target_name, uint8_t target_group, std::string by_name, uint8_t by_group )
{
	XWingServer *server = (XWingServer*)( Raptor::Server );
	
	if( ! MatchesConditions( flags, target_name, target_group, by_name, by_group ) )
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
		else if( Str::EqualsInsensitive( PropertyName, "mission_objs" ) )
			server->SetProperty( PropertyName, PropertyValue );  // XWingServer::SetProperty sends mission_objs to clients.
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
			ship->IsMissionObjective =   SpawnFlags & SPAWNFLAG_OBJECTIVE;
			ship->CanRespawn         =   SpawnFlags & SPAWNFLAG_RESPAWN;
			ship->JumpProgress       = ((SpawnFlags & SPAWNFLAG_SILENT) || ((Trigger == TRIGGER_ALWAYS) && ! (Time || Delay || TriggerIf.size()))) ? 1. : 0.;  // Jump in unless spawning at mission start.
			ship->SetThrottle( 1., 999. );
			
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
