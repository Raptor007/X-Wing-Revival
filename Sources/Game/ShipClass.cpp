/*
 *  ShipClass.cpp
 */

#include "ShipClass.h"

#include "XWingDefs.h"
#include "RaptorGame.h"
#include "Shot.h"
#include "Str.h"
#include <fstream>
#include <algorithm>
#include "Num.h"


ShipClass::ShipClass( uint32_t id ) : GameObject( id, XWing::Object::SHIP_CLASS )
{
	Category = CATEGORY_FIGHTER;
	Team = XWing::Team::NONE;
	Radius = 4.5;
	CollisionDamage = 1000.;
	MaxSpeed = 170.;
	Acceleration = 85.;
	MaxRoll = 180.;
	MaxPitch = 100.;
	MaxYaw = 80.;
	MaxHealth = 100.;
	MaxShield = 0.;
	ShieldRechargeDelay = 5.;
	ShieldRechargeRate = 0.;
	ExplosionRate = 1.;
	ModelScale = 0.022;
}


ShipClass::ShipClass( const ShipClass &other ) : GameObject( 0, XWing::Object::SHIP_CLASS )
{
	ShortName = other.ShortName;
	LongName = other.LongName;
	Squadron = other.Squadron;
	Category = other.Category;
	Team = other.Team;
	Radius = other.Radius;
	CollisionDamage = other.CollisionDamage;
	MaxSpeed = other.MaxSpeed;
	Acceleration = other.Acceleration;
	MaxRoll = other.MaxRoll;
	MaxPitch = other.MaxPitch;
	MaxYaw = other.MaxYaw;
	MaxHealth = other.MaxHealth;
	MaxShield = other.MaxShield;
	ShieldRechargeDelay = other.ShieldRechargeDelay;
	ShieldRechargeRate = other.ShieldRechargeRate;
	ExplosionRate = other.ExplosionRate;
	Subsystems = other.Subsystems;
	Weapons = other.Weapons;
	FireTime = other.FireTime;
	Ammo = other.Ammo;
	Turrets = other.Turrets;
	CollisionModel = other.CollisionModel;
	ExternalModel = other.ExternalModel;
	CockpitModel = other.CockpitModel;
	CockpitPos.Copy( &(other.CockpitPos) );
	ModelScale = other.ModelScale;
	FlybySounds = other.FlybySounds;
}


ShipClass::~ShipClass()
{
}


static uint32_t ShotTypeFromString( std::string type, uint32_t team )
{
	std::transform( type.begin(), type.end(), type.begin(), tolower );
	
	if( type == "red_laser" )
		return Shot::TYPE_LASER_RED;
	else if( type == "green_laser" )
		return Shot::TYPE_LASER_GREEN;
	else if( type == "red_turbolaser" )
		return Shot::TYPE_TURBO_LASER_RED;
	else if( type == "green_turbolaser" )
		return Shot::TYPE_TURBO_LASER_GREEN;
	else if( type == "ion_cannon" )
		return Shot::TYPE_ION_CANNON;
	else if( type == "torpedo" )
		return Shot::TYPE_TORPEDO;
	else if( type == "missile" )
		return Shot::TYPE_MISSILE;
	else if( type == "turbolaser" )
		return (team == XWing::Team::EMPIRE) ? Shot::TYPE_TURBO_LASER_GREEN : Shot::TYPE_TURBO_LASER_RED;
	
	return (team == XWing::Team::EMPIRE) ? Shot::TYPE_LASER_GREEN : Shot::TYPE_LASER_RED;
}


bool ShipClass::Load( const std::string &filename )
{
	std::ifstream input( filename.c_str() );
	if( ! input.is_open() )
		return false;
	
	char buffer[ 1024 ] = "";
	while( ! input.eof() )
	{
		buffer[ 0 ] = '\0';
		input.getline( buffer, sizeof(buffer) );
		
		std::list<std::string> parsed = CStr::ParseCommand( buffer );
		if( ! parsed.size() )
			continue;
		
		std::string var = *(parsed.begin());
		std::transform( var.begin(), var.end(), var.begin(), tolower );
		parsed.erase( parsed.begin() );
		std::vector<std::string> args( parsed.begin(), parsed.end() );
		
		if( (var == "id") && args.size() )
		{
			ShortName = args.at(0);
			std::transform( ShortName.begin(), ShortName.end(), ShortName.begin(), toupper );
		}
		else if( (var == "name") && args.size() )
		{
			LongName = Str::Join( args, " " );
		}
		else if( (var == "squadron") && args.size() )
		{
			Squadron = Str::Join( args, " " );
		}
		else if( (var == "category") && args.size() )
		{
			std::string category = args.at(0);
			std::transform( category.begin(), category.end(), category.begin(), tolower );
			if( category == "fighter" )
				Category = CATEGORY_FIGHTER;
			else if( category == "bomber" )
				Category = CATEGORY_BOMBER;
			else if( category == "capital" )
				Category = CATEGORY_CAPITAL;
			else if( category == "target" )
				Category = CATEGORY_TARGET;
		}
		else if( (var == "team") && args.size() )
		{
			std::string team = args.at(0);
			std::transform( team.begin(), team.end(), team.begin(), tolower );
			if( team == "rebel" )
				Team = XWing::Team::REBEL;
			else if( team == "empire" )
				Team = XWing::Team::EMPIRE;
			else
				Team = XWing::Team::NONE;
		}
		else if( (var == "radius") && args.size() )
		{
			Radius = atof( args.at(0).c_str() );
		}
		else if( (var == "ramming") && args.size() )
		{
			CollisionDamage = atof( args.at(0).c_str() );
		}
		else if( (var == "speed") && args.size() )
		{
			MaxSpeed = atof( args.at(0).c_str() );
		}
		else if( (var == "accel") && args.size() )
		{
			Acceleration = atof( args.at(0).c_str() );
		}
		else if( (var == "roll") && args.size() )
		{
			MaxRoll = atof( args.at(0).c_str() );
		}
		else if( (var == "pitch") && args.size() )
		{
			MaxPitch = atof( args.at(0).c_str() );
		}
		else if( (var == "yaw") && args.size() )
		{
			MaxYaw = atof( args.at(0).c_str() );
		}
		else if( (var == "health") && args.size() )
		{
			MaxHealth = atof( args.at(0).c_str() );
		}
		else if( (var == "shield") && args.size() )
		{
			MaxShield = atof( args.at(0).c_str() );
		}
		else if( (var == "recover") && args.size() )
		{
			ShieldRechargeDelay = atof( args.at(0).c_str() );
		}
		else if( (var == "recharge") && args.size() )
		{
			ShieldRechargeRate = atof( args.at(0).c_str() );
		}
		else if( (var == "explode") && args.size() )
		{
			ExplosionRate = atof( args.at(0).c_str() );
		}
		else if( (var == "subsystem") && (args.size() >= 2) )
		{
			Subsystems[ args.at(0) ] = atof( args.at(1).c_str() );
		}
		else if( (var == "weapon") && (args.size() >= 4) )
		{
			uint32_t type = ShotTypeFromString( args.at(0), Team );
			double right = atof( args.at(1).c_str() );
			double up    = atof( args.at(2).c_str() );
			double fwd   = atof( args.at(3).c_str() );
			Weapons[ type ].push_back( Pos3D(fwd,up,right) );
		}
		else if( (var == "firetime") && (args.size() >= 2) )
		{
			uint32_t type = ShotTypeFromString( args.at(0), Team );
			FireTime[ type ] = atof( args.at(1).c_str() );
		}
		else if( (var == "ammo") && (args.size() >= 2) )
		{
			uint32_t type = ShotTypeFromString( args.at(0), Team );
			Ammo[ type ] = atoi( args.at(1).c_str() );
		}
		else if( (var == "turret") && (args.size() >= 4) )
		{
			uint32_t type = ShotTypeFromString( args.at(0), Team );
			double right = atof( args.at(1).c_str() );
			double up    = atof( args.at(2).c_str() );
			double fwd   = atof( args.at(3).c_str() );
			args.erase( args.begin() );
			args.erase( args.begin() );
			args.erase( args.begin() );
			args.erase( args.begin() );
			Turrets.push_back( ShipClassTurret( fwd,up,right, type ) );
			
			while( args.size() )
			{
				std::string subvar = *(args.begin());
				std::transform( subvar.begin(), subvar.end(), subvar.begin(), tolower );
				args.erase( args.begin() );
				
				if( (subvar == "up") && (args.size() >= 3) )
				{
					double right = atof( args.at(0).c_str() );
					double up    = atof( args.at(1).c_str() );
					double fwd   = atof( args.at(2).c_str() );
					args.erase( args.begin() );
					args.erase( args.begin() );
					args.erase( args.begin() );
					Turrets.back().Up.Set( right, up, fwd );
				}
				else if( (subvar == "dir") && (args.size() >= 3) )
				{
					double right = atof( args.at(0).c_str() );
					double up    = atof( args.at(1).c_str() );
					double fwd   = atof( args.at(2).c_str() );
					args.erase( args.begin() );
					args.erase( args.begin() );
					args.erase( args.begin() );
					Turrets.back().Fwd.Set( right, up, fwd );
				}
				else if( (subvar == "arc") && args.size() )
				{
					Turrets.back().TargetArc = atof( args.at(0).c_str() );
					args.erase( args.begin() );
				}
				else if( (subvar == "pitch") && (args.size() >= 2) )
				{
					Turrets.back().MinGunPitch = atof( args.at(0).c_str() );
					Turrets.back().MaxGunPitch = atof( args.at(1).c_str() );
					args.erase( args.begin() );
					args.erase( args.begin() );
				}
				else if( subvar == "visible" )
				{
					Turrets.back().Visible = true;
				}
				else if( subvar == "hidden" )
				{
					Turrets.back().Visible = false;
				}
				else if( subvar == "independent" )
				{
					Turrets.back().ParentControl = false;
				}
				else if( subvar == "linked" )
				{
					Turrets.back().ParentControl = true;
				}
				else if( subvar == "dual" )
				{
					Turrets.back().FiringMode = 2;
				}
				else if( (subvar == "firetime") && args.size() )
				{
					Turrets.back().SingleShotDelay = atof( args.at(0).c_str() );
					args.erase( args.begin() );
				}
			}
		}
		else if( (var == "cockpit") && (args.size() >= 4) )
		{
			CockpitModel = args.at(0);
			CockpitPos.Z = atof( args.at(1).c_str() ); // Right
			CockpitPos.Y = atof( args.at(2).c_str() ); // Up
			CockpitPos.X = atof( args.at(3).c_str() ); // Fwd
		}
		else if( (var == "model") && args.size() )
		{
			ExternalModel = args.at(0);
		}
		else if( (var == "model_collision") && args.size() )
		{
			CollisionModel = args.at(0);
		}
		else if( (var == "model_scale") && args.size() )
		{
			ModelScale = atof( args.at(0).c_str() );
		}
		else if( (var == "flyby") && (args.size() >= 2) )
		{
			FlybySounds[ atof( args.at(0).c_str() ) ] = args.at(1);
		}
	}
	input.close();
	
	for( std::map< uint32_t, std::vector<Pos3D> >::const_iterator weapon_iter = Weapons.begin(); weapon_iter != Weapons.end(); weapon_iter ++ )
	{
		if( Ammo.find( weapon_iter->first ) == Ammo.end() )
			Ammo[ weapon_iter->first ] = -1;
		if( FireTime.find( weapon_iter->first ) == FireTime.end() )
			FireTime[ weapon_iter->first ] = 0.25;
	}
	
	// Return true if we loaded at least some basic ship data from the file.
	return (ShortName.length() && LongName.length());
}


void ShipClass::AddToInitPacket( Packet *packet, int8_t precision )
{
	packet->AddString( ShortName );
	packet->AddString( LongName );
	packet->AddChar( Category );
	packet->AddUInt( Team );
	packet->AddDouble( Radius );
	packet->AddDouble( MaxSpeed );
	packet->AddDouble( Acceleration );
	packet->AddDouble( MaxRoll );
	packet->AddDouble( MaxPitch );
	packet->AddDouble( MaxYaw );
	packet->AddDouble( MaxHealth );
	
	packet->AddDouble( MaxShield );
	if( MaxShield )
	{
		packet->AddDouble( ShieldRechargeDelay );
		packet->AddDouble( ShieldRechargeRate );
	}
	
	packet->AddFloat( ExplosionRate );
	
	packet->AddUChar( Weapons.size() );
	for( std::map< uint32_t, std::vector<Pos3D> >::const_iterator weapon_iter = Weapons.begin(); weapon_iter != Weapons.end(); weapon_iter ++ )
	{
		packet->AddUInt( weapon_iter->first );
		
		std::map<uint32_t,int8_t>::const_iterator ammo_iter = Ammo.find( weapon_iter->first );
		packet->AddChar( (ammo_iter != Ammo.end()) ? ammo_iter->second : -1 );
		
		packet->AddUChar( weapon_iter->second.size() );
		for( size_t i = 0; i < weapon_iter->second.size(); i ++ )
		{
			packet->AddDouble( weapon_iter->second[ i ].X );
			packet->AddDouble( weapon_iter->second[ i ].Y );
			packet->AddDouble( weapon_iter->second[ i ].Z );
		}
	}
	
	packet->AddString( CollisionModel );
	packet->AddString( ExternalModel );
	
	packet->AddString( CockpitModel );
	packet->AddDouble( CockpitPos.X );
	packet->AddDouble( CockpitPos.Y );
	packet->AddDouble( CockpitPos.Z );
	
	packet->AddDouble( ModelScale );
	
	packet->AddUChar( FlybySounds.size() );
	for( std::map< double, std::string >::const_iterator flyby_iter = FlybySounds.begin(); flyby_iter != FlybySounds.end(); flyby_iter ++ )
	{
		packet->AddFloat( flyby_iter->first );
		packet->AddString( flyby_iter->second );
	}
}


void ShipClass::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	ShortName    = packet->NextString();
	LongName     = packet->NextString();
	Category     = packet->NextChar();
	Team         = packet->NextUInt();
	Radius       = packet->NextDouble();
	MaxSpeed     = packet->NextDouble();
	Acceleration = packet->NextDouble();
	MaxRoll      = packet->NextDouble();
	MaxPitch     = packet->NextDouble();
	MaxYaw       = packet->NextDouble();
	MaxHealth    = packet->NextDouble();
	
	MaxShield    = packet->NextDouble();
	if( MaxShield )
	{
		ShieldRechargeDelay = packet->NextDouble();
		ShieldRechargeRate  = packet->NextDouble();
	}
	
	ExplosionRate = packet->NextFloat();
	
	size_t num_weapons = packet->NextUChar();
	for( size_t i = 0; i < num_weapons; i ++ )
	{
		uint32_t weapon_id = packet->NextUInt();
		
		Ammo[ weapon_id ] = packet->NextChar();
		
		size_t weapon_count = packet->NextUChar();
		Weapons[ weapon_id ] = std::vector<Pos3D>();
		for( size_t j = 0; j < weapon_count; j ++ )
		{
			double x = packet->NextDouble();
			double y = packet->NextDouble();
			double z = packet->NextDouble();
			Weapons[ weapon_id ].push_back( Pos3D(x,y,z) );
		}
	}
	
	CollisionModel = packet->NextString();
	ExternalModel = packet->NextString();
	
	CockpitModel = packet->NextString();
	CockpitPos.X = packet->NextDouble();
	CockpitPos.Y = packet->NextDouble();
	CockpitPos.Z = packet->NextDouble();
	
	ModelScale = packet->NextDouble();
	
	size_t num_flyby = packet->NextUChar();
	for( size_t i = 0; i < num_flyby; i ++ )
	{
		double flyby_speed = packet->NextFloat();
		FlybySounds[ flyby_speed ] = packet->NextString();
	}
}


bool ShipClass::ServerShouldUpdateOthers( void ) const
{
	return false;
}

bool ShipClass::IsMoving( void ) const
{
	return false;
}


void ShipClass::Update( double dt )
{
}


bool ShipClass::PlayersCanFly( void ) const
{
	return (Category == CATEGORY_FIGHTER) || (Category == CATEGORY_BOMBER);
}


bool ShipClass::operator < ( const ShipClass &other ) const
{
	if( (Team == XWing::Team::NONE) && (other.Team != XWing::Team::NONE) )
		return true;
	if( (Team != XWing::Team::NONE) && (other.Team == XWing::Team::NONE) )
		return false;
	if( (Team == XWing::Team::REBEL) && (other.Team != XWing::Team::REBEL) )
		return true;
	if( (Team != XWing::Team::REBEL) && (other.Team == XWing::Team::REBEL) )
		return false;
	
	/*
	if( (Category == CATEGORY_FIGHTER) && (other.Category != CATEGORY_FIGHTER) )
		return true;
	if( (Category != CATEGORY_FIGHTER) && (other.Category == CATEGORY_FIGHTER) )
		return false;
	if( (Category == CATEGORY_BOMBER) && (other.Category != CATEGORY_BOMBER) )
		return true;
	if( (Category != CATEGORY_BOMBER) && (other.Category == CATEGORY_BOMBER) )
		return false;
	*/
	
	if( Radius < other.Radius )
		return true;
	if( CollisionDamage < other.CollisionDamage )
		return true;
	
	return (strcasecmp( ShortName.c_str(), other.ShortName.c_str() ) < 0);
}


// ---------------------------------------------------------------------------


ShipClassTurret::ShipClassTurret( double fwd, double up, double right, uint32_t weapon ) : Pos3D( fwd, up, right )
{
	Weapon = weapon;
	
	Visible = false;
	ParentControl = true;
	FiringMode = 1;
	SingleShotDelay = 0.5;
	TargetArc = 360.;
	MinGunPitch = -10.;
	MaxGunPitch = 90.;
}
