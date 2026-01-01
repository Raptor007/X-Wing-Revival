/*
 *  ShipClass.cpp
 */

#include "ShipClass.h"

#include "XWingDefs.h"
#include "RaptorGame.h"
#include "Shot.h"
#include "Str.h"
#include "Num.h"
#include <fstream>
#include <algorithm>


ShipClass::ShipClass( uint32_t id ) : GameObject( id, XWing::Object::SHIP_CLASS )
{
	Category = CATEGORY_UNKNOWN;
	Team = XWing::Team::NONE;
	Radius = 4.5;
	CollisionDamage = 1000.;
	MaxSpeed = 170.;
	Acceleration = 85.;
	RollSlow = RollFast = 180.;
	PitchSlow = PitchFast = 100.;
	YawSlow = YawFast = 80.;
	RollExponent = PitchExponent = YawExponent = 1.;
	RollChangeSlow = RollChangeFast = 8. * RollSlow;
	PitchChangeSlow = PitchChangeFast = 8. * PitchSlow;
	YawChangeSlow = YawChangeFast = 8. * YawSlow;
	RollChangeExponent = PitchChangeExponent = YawChangeExponent = 2.;
	MaxHealth = 100.;
	MaxShield = 0.;
	ShieldRechargeDelay = 5.;
	ShieldRechargeRate = 0.;
	ExplosionRate = 1.;
	TurretHealth = 95.;
	TurretYawSpeed = TurretPitchSpeed = 45.;
	TurretBody = "turret_body.obj";
	TurretGun  = "turret_gun.obj";
	TurretRadius = 7.7;
	TurretGunWidth = 2.2;
	TurretGunUp = 0.022 * 175.;
	TurretGunFwd = 0.022 * 50.;
	TurretHeadUp = 2.;
	TurretHeadFwd = 5.;
	Dockable = false;
	GlanceUpFwd = 85.;
	GlanceUpBack = 20.;
	ModelScale = 1.;
	Secret = false;
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
	RollSlow = other.RollSlow;
	PitchSlow = other.PitchSlow;
	YawSlow = other.YawSlow;
	RollFast = other.RollFast;
	PitchFast = other.PitchFast;
	YawFast = other.YawFast;
	RollExponent = other.RollExponent;
	PitchExponent = other.PitchExponent;
	YawExponent = other.YawExponent;
	RollChangeSlow = other.RollChangeSlow;
	PitchChangeSlow = other.PitchChangeSlow;
	YawChangeSlow = other.YawChangeSlow;
	RollChangeFast = other.RollChangeFast;
	PitchChangeFast = other.PitchChangeFast;
	YawChangeFast = other.YawChangeFast;
	RollChangeExponent = other.RollChangeExponent;
	PitchChangeExponent = other.PitchChangeExponent;
	YawChangeExponent = other.YawChangeExponent;
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
	TurretHealth = other.TurretHealth;
	TurretPitchSpeed = other.TurretPitchSpeed;
	TurretYawSpeed = other.TurretYawSpeed;
	TurretBody = other.TurretBody;
	TurretGun = other.TurretGun;
	TurretRadius = other.TurretRadius;
	TurretGunWidth = other.TurretGunWidth;
	TurretGunUp = other.TurretGunUp;
	TurretGunFwd = other.TurretGunFwd;
	TurretHeadUp = other.TurretHeadUp;
	TurretHeadFwd = other.TurretHeadFwd;
	CollisionModel = other.CollisionModel;
	ExternalModel = other.ExternalModel;
	CockpitModel = other.CockpitModel;
	CockpitModelVR = other.CockpitModelVR;
	CockpitPos.Copy( &(other.CockpitPos) );
	CockpitPosVR.Copy( &(other.CockpitPosVR) );
	GlanceUpFwd  = other.GlanceUpFwd;
	GlanceUpBack = other.GlanceUpBack;
	DockingBays = other.DockingBays;
	Dockable = other.Dockable;
	Engines = other.Engines;
	ModelScale = other.ModelScale;
	Shape.BecomeCopy( &(other.Shape) );
	GroupSkins = other.GroupSkins;
	GroupCockpits = other.GroupCockpits;
	FlybySounds = other.FlybySounds;
	Secret = other.Secret;
}


ShipClass::~ShipClass()
{
}


static uint8_t ShotTypeFromString( std::string type, uint8_t team )
{
	type = Str::LowercaseCopy( type );
	
	if( (type == "red_laser") || (type == "laser_red") )
		return Shot::TYPE_LASER_RED;
	else if( (type == "green_laser") || (type == "laser_green") )
		return Shot::TYPE_LASER_GREEN;
	else if( (type == "red_turbolaser") || (type == "turbolaser_red") )
		return Shot::TYPE_TURBO_LASER_RED;
	else if( (type == "green_turbolaser") || (type == "turbolaser_green") )
		return Shot::TYPE_TURBO_LASER_GREEN;
	else if( type == "quad_laser" )
		return Shot::TYPE_QUAD_LASER_RED;
	else if( type == "ion_cannon" )
		return Shot::TYPE_ION_CANNON;
	else if( type == "torpedo" )
		return Shot::TYPE_TORPEDO;
	else if( type == "missile" )
		return Shot::TYPE_MISSILE;
	else if( type == "turbolaser" )
		return (team == XWing::Team::EMPIRE) ? Shot::TYPE_TURBO_LASER_GREEN : Shot::TYPE_TURBO_LASER_RED;
	else if( type == "superlaser" )
		return Shot::TYPE_SUPERLASER;
	
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
			else if( category == "gunboat" )
				Category = CATEGORY_GUNBOAT;
			else if( category == "bomber" )
				Category = CATEGORY_BOMBER;
			else if( category == "transport" )
				Category = CATEGORY_TRANSPORT;
			else if( category == "capital" )
				Category = CATEGORY_CAPITAL;
			else if( category == "target" )
				Category = CATEGORY_TARGET;
			else
				Category = CATEGORY_UNKNOWN;
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
		else if( var == "secret" )
			Secret = true;
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
			RollSlow = atof( args.at(0).c_str() );
			if( args.size() >= 2 )
				RollFast = atof( args.at(1).c_str() );
			else
				RollFast = RollSlow;
			if( args.size() >= 3 )
				RollExponent = atof( args.at(2).c_str() );
			else
				RollExponent = 2.;
			RollChangeSlow = 8. * RollSlow;
			RollChangeFast = 8. * RollFast;
		}
		else if( (var == "pitch") && args.size() )
		{
			PitchSlow = atof( args.at(0).c_str() );
			if( args.size() >= 2 )
				PitchFast = atof( args.at(1).c_str() );
			else
				PitchFast = PitchSlow;
			if( args.size() >= 3 )
				PitchExponent = atof( args.at(2).c_str() );
			else
				PitchExponent = 2.;
			PitchChangeSlow = 8. * PitchSlow;
			PitchChangeFast = 8. * PitchFast;
		}
		else if( (var == "yaw") && args.size() )
		{
			YawSlow = atof( args.at(0).c_str() );
			if( args.size() >= 2 )
				YawFast = atof( args.at(1).c_str() );
			else
				YawFast = YawSlow;
			if( args.size() >= 3 )
				YawExponent = atof( args.at(2).c_str() );
			else
				YawExponent = 2.;
			YawChangeSlow = 8. * YawSlow;
			YawChangeFast = 8. * YawFast;
		}
		else if( (var == "roll_change") && args.size() )
		{
			RollChangeSlow = atof( args.at(0).c_str() );
			if( args.size() >= 2 )
				RollChangeFast = atof( args.at(1).c_str() );
			else
				RollChangeFast = RollChangeSlow;
			if( args.size() >= 3 )
				RollChangeExponent = atof( args.at(2).c_str() );
			else
				RollChangeExponent = 2.;
		}
		else if( (var == "pitch_change") && args.size() )
		{
			PitchChangeSlow = atof( args.at(0).c_str() );
			if( args.size() >= 2 )
				PitchChangeFast = atof( args.at(1).c_str() );
			else
				PitchChangeFast = PitchChangeSlow;
			if( args.size() >= 3 )
				PitchChangeExponent = atof( args.at(2).c_str() );
			else
				PitchChangeExponent = 2.;
		}
		else if( (var == "yaw_change") && args.size() )
		{
			YawChangeSlow = atof( args.at(0).c_str() );
			if( args.size() >= 2 )
				YawChangeFast = atof( args.at(1).c_str() );
			else
				YawChangeFast = YawChangeSlow;
			if( args.size() >= 3 )
				YawChangeExponent = atof( args.at(2).c_str() );
			else
				YawChangeExponent = 2.;
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
			uint8_t type = ShotTypeFromString( args.at(0), Team );
			double right = atof( args.at(1).c_str() );
			double up    = atof( args.at(2).c_str() );
			double fwd   = atof( args.at(3).c_str() );
			if( type == Shot::TYPE_MISSILE )
				fwd -= 2.75;  // Missiles spawn with just the nose ahead of weapon position, not the whole model.
			Weapons[ type ].push_back( Pos3D( fwd, up, right ) );
		}
		else if( (var == "firetime") && (args.size() >= 2) )
		{
			uint8_t type = ShotTypeFromString( args.at(0), Team );
			FireTime[ type ] = atof( args.at(1).c_str() );
		}
		else if( (var == "ammo") && (args.size() >= 2) )
		{
			uint8_t type = ShotTypeFromString( args.at(0), Team );
			Ammo[ type ] = atoi( args.at(1).c_str() );
		}
		else if( (var == "turret") && (args.size() >= 4) )
		{
			uint8_t type = ShotTypeFromString( args.at(0), Team );
			double right = atof( args.at(1).c_str() );
			double up    = atof( args.at(2).c_str() );
			double fwd   = atof( args.at(3).c_str() );
			args.erase( args.begin() );
			args.erase( args.begin() );
			args.erase( args.begin() );
			args.erase( args.begin() );
			Turrets.push_back( ShipClassTurret( fwd, up, right, type ) );
			
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
					Turrets.back().Up.Set( fwd, up, right );
				}
				else if( (subvar == "dir") && (args.size() >= 3) )
				{
					double right = atof( args.at(0).c_str() );
					double up    = atof( args.at(1).c_str() );
					double fwd   = atof( args.at(2).c_str() );
					args.erase( args.begin() );
					args.erase( args.begin() );
					args.erase( args.begin() );
					Turrets.back().Fwd.Set( fwd, up, right );
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
				else if( subvar == "invincible" )
				{
					Turrets.back().CanBeHit = false;
				}
				else if( (subvar == "health") && args.size() )
				{
					Turrets.back().Health = atof( args.at(0).c_str() );
					args.erase( args.begin() );
				}
				else if( subvar == "independent" )
				{
					Turrets.back().ParentControl = false;
				}
				else if( subvar == "linked" )
				{
					Turrets.back().ParentControl = true;
				}
				else if( subvar == "manual" )
				{
					Turrets.back().Manual = true;
				}
				else if( subvar == "single" )
				{
					Turrets.back().FiringMode = 1;
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
		else if( (var == "turret_health") && args.size() )
		{
			TurretHealth = atof( args.at(0).c_str() );
		}
		else if( (var == "turret_model") && args.size() )
		{
			TurretBody = (args.size() >= 2) ? args.at(0) : "";
			TurretGun  = (args.size() >= 2) ? args.at(1) : args.at(0);
		}
		else if( (var == "turret_gunpos") && args.size() )
		{
			TurretGunUp = atof( args.at(0).c_str() );
			TurretGunFwd = (args.size() >= 2) ? atof( args.at(1).c_str() ) : 0.;
		}
		else if( (var == "turret_radius") && args.size() )
		{
			TurretRadius = atof( args.at(0).c_str() );
		}
		else if( (var == "turret_separation") && args.size() )
		{
			TurretGunWidth = atof( args.at(0).c_str() );
		}
		else if( (var == "turret_headpos") && args.size() )
		{
			TurretHeadUp = atof( args.at(0).c_str() );
			TurretHeadFwd = (args.size() >= 2) ? atof( args.at(1).c_str() ) : 0.;
		}
		else if( (var == "turret_speed") && args.size() )
		{
			TurretYawSpeed = atof( args.at(0).c_str() );
			TurretPitchSpeed = (args.size() >= 2) ? atof( args.at(1).c_str() ) : TurretYawSpeed;
		}
		else if( (var == "docking_bay") && (args.size() >= 3) )
		{
			double right = atof( args.at(0).c_str() );
			double up    = atof( args.at(1).c_str() );
			double fwd   = atof( args.at(2).c_str() );
			DockingBays.push_back( ShipClassDockingBay( fwd, up, right ) );
			Dockable = true;
			if( args.size() >= 4 )
				DockingBays.back().Radius = atof( args.at(3).c_str() );
		}
		else if( (var == "engine") && (args.size() >= 5) )
		{
			double right  = atof( args.at(0).c_str() );
			double up     = atof( args.at(1).c_str() );
			double fwd    = atof( args.at(2).c_str() );
			std::string texture = args.at(3);
			double radius = atof( args.at(4).c_str() );
			Engines.push_back( ShipClassEngine( fwd, up, right, texture, radius ) );
			if( args.size() >= 8 )
			{
				Engines.back().DrawColor.Red   = atof( args.at(5).c_str() );
				Engines.back().DrawColor.Green = atof( args.at(6).c_str() );
				Engines.back().DrawColor.Blue  = atof( args.at(7).c_str() );
				if( args.size() >= 9 )
					Engines.back().DrawColor.Alpha = atof( args.at(8).c_str() );
			}
		}
		else if( (var == "cockpit") && (args.size() >= 4) )
		{
			CockpitModel = args.at(0);
			CockpitPos.Z = atof( args.at(1).c_str() ); // Right
			CockpitPos.Y = atof( args.at(2).c_str() ); // Up
			CockpitPos.X = atof( args.at(3).c_str() ); // Fwd
		}
		else if( (var == "cockpit_vr") && (args.size() >= 4) )
		{
			CockpitModelVR = args.at(0);
			CockpitPosVR.Z = atof( args.at(1).c_str() ); // Right
			CockpitPosVR.Y = atof( args.at(2).c_str() ); // Up
			CockpitPosVR.X = atof( args.at(3).c_str() ); // Fwd
		}
		else if( (var == "glance") && args.size() )
		{
			GlanceUpFwd = atof( args.at(0).c_str() );
			if( args.size() >= 2 )
				GlanceUpBack = atof( args.at(1).c_str() );
		}
		else if( (var == "model") && args.size() )
		{
			ExternalModel = args.at(0);
		}
		else if( (var == "model_collision") && args.size() )
		{
			CollisionModel = args.at(0);
			Shape.LoadOBJ( std::string("Models/") + CollisionModel, false );
			Shape.ScaleBy( ModelScale );  // This only has any effect if model_scale has already been set (default is 1).
		}
		else if( (var == "group_skin") && (args.size() >= 2) )
		{
			uint8_t group = atoi( args.at(0).c_str() );
			GroupSkins[ group ] = args.at(1);
			if( args.size() >= 3 )
				GroupCockpits[ group ] = args.at(2);
		}
		else if( (var == "model_scale") && args.size() )
		{
			ModelScale = atof( args.at(0).c_str() );
			Shape.ScaleBy( ModelScale );  // This only has any effect if model_collision has already been loaded.
		}
		else if( (var == "flyby") && (args.size() >= 2) )
		{
			FlybySounds[ atof( args.at(0).c_str() ) ] = args.at(1);
		}
	}
	input.close();
	
	for( std::map< uint8_t, std::vector<Pos3D> >::const_iterator weapon_iter = Weapons.begin(); weapon_iter != Weapons.end(); weapon_iter ++ )
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
	uint8_t category_and_flags = Category;
	if( Secret )
		category_and_flags |= 0x80;
	if( Dockable )
		category_and_flags |= 0x40;
	packet->AddUChar( category_and_flags );
	packet->AddUChar( Team );
	packet->AddFloat( Radius );
	packet->AddFloat( MaxSpeed );
	packet->AddFloat( Acceleration );
	packet->AddFloat( RollSlow );
	packet->AddFloat( PitchSlow );
	packet->AddFloat( YawSlow );
	packet->AddFloat( RollFast );
	packet->AddFloat( PitchFast );
	packet->AddFloat( YawFast );
	packet->AddFloat( RollExponent );
	packet->AddFloat( PitchExponent );
	packet->AddFloat( YawExponent );
	packet->AddFloat( RollChangeSlow );
	packet->AddFloat( PitchChangeSlow );
	packet->AddFloat( YawChangeSlow );
	packet->AddFloat( RollChangeFast );
	packet->AddFloat( PitchChangeFast );
	packet->AddFloat( YawChangeFast );
	packet->AddFloat( RollChangeExponent );
	packet->AddFloat( PitchChangeExponent );
	packet->AddFloat( YawChangeExponent );
	packet->AddFloat( MaxHealth );
	
	packet->AddFloat( MaxShield );
	if( MaxShield )
	{
		packet->AddFloat( ShieldRechargeDelay );
		packet->AddFloat( ShieldRechargeRate );
	}
	
	packet->AddFloat( ExplosionRate );
	
	packet->AddUChar( Weapons.size() );
	for( std::map< uint8_t, std::vector<Pos3D> >::const_iterator weapon_iter = Weapons.begin(); weapon_iter != Weapons.end(); weapon_iter ++ )
	{
		packet->AddUInt( weapon_iter->first );
		
		std::map<uint8_t,int8_t>::const_iterator ammo_iter = Ammo.find( weapon_iter->first );
		packet->AddChar( (ammo_iter != Ammo.end()) ? ammo_iter->second : -1 );
		
		std::map<uint8_t,double>::const_iterator rate_iter = FireTime.find( weapon_iter->first );
		packet->AddFloat( (rate_iter != FireTime.end()) ? rate_iter->second : 0.f );
		
		packet->AddUChar( weapon_iter->second.size() );
		for( size_t i = 0; i < weapon_iter->second.size(); i ++ )
		{
			packet->AddFloat( weapon_iter->second[ i ].X );
			packet->AddFloat( weapon_iter->second[ i ].Y );
			packet->AddFloat( weapon_iter->second[ i ].Z );
		}
	}
	
	packet->AddUChar( Subsystems.size() );
	for( std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.begin(); subsystem_iter != Subsystems.end(); subsystem_iter ++ )
	{
		packet->AddString( subsystem_iter->first );
		packet->AddDouble( subsystem_iter->second );
	}
	
	packet->AddUChar( Engines.size() );
	for( std::vector<ShipClassEngine>::const_iterator engine_iter = Engines.begin(); engine_iter != Engines.end(); engine_iter ++ )
	{
		packet->AddFloat( engine_iter->X );
		packet->AddFloat( engine_iter->Y );
		packet->AddFloat( engine_iter->Z );
		packet->AddString( engine_iter->Texture );
		packet->AddFloat( engine_iter->Radius );
		packet->AddUChar( Num::UnitFloatTo8( engine_iter->DrawColor.Red   * 2. - 1. ) );
		packet->AddUChar( Num::UnitFloatTo8( engine_iter->DrawColor.Green * 2. - 1. ) );
		packet->AddUChar( Num::UnitFloatTo8( engine_iter->DrawColor.Blue  * 2. - 1. ) );
		packet->AddUChar( Num::UnitFloatTo8( engine_iter->DrawColor.Alpha * 2. - 1. ) );
	}
	
	packet->AddString( CollisionModel );
	packet->AddString( ExternalModel );
	
	packet->AddString( CockpitModel );
	packet->AddFloat( CockpitPos.X );
	packet->AddFloat( CockpitPos.Y );
	packet->AddFloat( CockpitPos.Z );
	
	packet->AddString( CockpitModelVR );
	if( ! CockpitModelVR.empty() )
	{
		packet->AddFloat( CockpitPosVR.X );
		packet->AddFloat( CockpitPosVR.Y );
		packet->AddFloat( CockpitPosVR.Z );
	}
	
	packet->AddUChar( GlanceUpFwd  + 0.5 * Num::Sign(GlanceUpFwd)  );
	packet->AddUChar( GlanceUpBack + 0.5 * Num::Sign(GlanceUpBack) );
	
	packet->AddFloat( ModelScale );
	
	packet->AddUChar( GroupSkins.size() );
	for( std::map<uint8_t,std::string>::const_iterator skin_iter = GroupSkins.begin(); skin_iter != GroupSkins.end(); skin_iter ++ )
	{
		packet->AddUChar( skin_iter->first );
		packet->AddString( skin_iter->second );
		std::map<uint8_t,std::string>::const_iterator cockpit_iter = GroupCockpits.find( skin_iter->first );
		packet->AddString( (cockpit_iter != GroupCockpits.end()) ? cockpit_iter->second : "" );
	}
	
	packet->AddUChar( FlybySounds.size() );
	for( std::map< double, std::string >::const_iterator flyby_iter = FlybySounds.begin(); flyby_iter != FlybySounds.end(); flyby_iter ++ )
	{
		packet->AddFloat( flyby_iter->first );
		packet->AddString( flyby_iter->second );
	}
}


void ShipClass::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	ShortName           = packet->NextString();
	LongName            = packet->NextString();
	Category            = packet->NextUChar();
	Secret   = Category & 0x80;
	Dockable = Category & 0x40;
	Category &= 0x3F;
	Team                = packet->NextUChar();
	Radius              = packet->NextFloat();
	MaxSpeed            = packet->NextFloat();
	Acceleration        = packet->NextFloat();
	RollSlow            = packet->NextFloat();
	PitchSlow           = packet->NextFloat();
	YawSlow             = packet->NextFloat();
	RollFast            = packet->NextFloat();
	PitchFast           = packet->NextFloat();
	YawFast             = packet->NextFloat();
	RollExponent        = packet->NextFloat();
	PitchExponent       = packet->NextFloat();
	YawExponent         = packet->NextFloat();
	RollChangeSlow      = packet->NextFloat();
	PitchChangeSlow     = packet->NextFloat();
	YawChangeSlow       = packet->NextFloat();
	RollChangeFast      = packet->NextFloat();
	PitchChangeFast     = packet->NextFloat();
	YawChangeFast       = packet->NextFloat();
	RollChangeExponent  = packet->NextFloat();
	PitchChangeExponent = packet->NextFloat();
	YawChangeExponent   = packet->NextFloat();
	MaxHealth           = packet->NextFloat();
	
	MaxShield           = packet->NextFloat();
	if( MaxShield )
	{
		ShieldRechargeDelay = packet->NextFloat();
		ShieldRechargeRate  = packet->NextFloat();
	}
	
	ExplosionRate = packet->NextFloat();
	
	size_t num_weapons = packet->NextUChar();
	for( size_t i = 0; i < num_weapons; i ++ )
	{
		uint8_t weapon_id = packet->NextUInt();
		
		Ammo[ weapon_id ] = packet->NextChar();
		
		double fire_time = packet->NextFloat();
		if( fire_time )
			FireTime[ weapon_id ] = fire_time;
		
		size_t weapon_count = packet->NextUChar();
		Weapons[ weapon_id ] = std::vector<Pos3D>();
		for( size_t j = 0; j < weapon_count; j ++ )
		{
			double x = packet->NextFloat();
			double y = packet->NextFloat();
			double z = packet->NextFloat();
			Weapons[ weapon_id ].push_back( Pos3D(x,y,z) );
		}
	}
	
	size_t num_subsystems = packet->NextUChar();
	for( size_t i = 0; i < num_subsystems; i ++ )
	{
		std::string subsystem_name = packet->NextString();
		double subsystem_health = packet->NextDouble();
		Subsystems[ subsystem_name ] = subsystem_health;
	}
	
	size_t num_engines = packet->NextUChar();
	for( size_t i = 0; i < num_engines; i ++ )
	{
		double fwd    = packet->NextFloat();
		double up     = packet->NextFloat();
		double right  = packet->NextFloat();
		std::string texture = packet->NextString();
		double radius = packet->NextFloat();
		float r = (Num::UnitFloatFrom8( packet->NextUChar() ) + 1.) / 2.;
		float g = (Num::UnitFloatFrom8( packet->NextUChar() ) + 1.) / 2.;
		float b = (Num::UnitFloatFrom8( packet->NextUChar() ) + 1.) / 2.;
		float a = (Num::UnitFloatFrom8( packet->NextUChar() ) + 1.) / 2.;
		Engines.push_back( ShipClassEngine( fwd, up, right, texture, radius, r, g, b, a ) );
	}
	
	CollisionModel = packet->NextString();
	ExternalModel = packet->NextString();
	
	CockpitModel = packet->NextString();
	CockpitPos.X = packet->NextFloat();
	CockpitPos.Y = packet->NextFloat();
	CockpitPos.Z = packet->NextFloat();
	
	CockpitModelVR = packet->NextString();
	if( ! CockpitModelVR.empty() )
	{
		CockpitPosVR.X = packet->NextFloat();
		CockpitPosVR.Y = packet->NextFloat();
		CockpitPosVR.Z = packet->NextFloat();
	}
	
	GlanceUpFwd  = packet->NextUChar();
	GlanceUpBack = packet->NextUChar();
	
	ModelScale = packet->NextFloat();
	
	size_t num_skins = packet->NextUChar();
	for( size_t i = 0; i < num_skins; i ++ )
	{
		uint8_t group = packet->NextUChar();
		GroupSkins[ group ] = packet->NextString();
		std::string cockpit = packet->NextString();
		if( cockpit.length() )
			GroupCockpits[ group ] = cockpit;
	}
	
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
	return ((Category == CATEGORY_FIGHTER) || (Category == CATEGORY_BOMBER) || (Category == CATEGORY_GUNBOAT)) && ! Secret;
}


uint8_t ShipClass::DefaultSkinGroup( void ) const
{
	if( GroupSkins.empty() )
		return 0;
	
	for( uint8_t i = 1; i <= GroupSkins.size(); i ++ )
	{
		if( GroupSkins.find( i ) == GroupSkins.end() )
			return i;
	}
	
	return GroupSkins.size() + 1;  // If we have alternate skins for groups 1-3, the default skin matches group 4.
}


bool ShipClass::operator < ( const ShipClass &other ) const
{
	if( (Category == CATEGORY_TARGET) && (other.Category != CATEGORY_TARGET) )
		return false;
	if( (Category != CATEGORY_TARGET) && (other.Category == CATEGORY_TARGET) )
		return true;
	if( (Category == CATEGORY_CAPITAL) && (other.Category != CATEGORY_CAPITAL) )
		return false;
	if( (Category != CATEGORY_CAPITAL) && (other.Category == CATEGORY_CAPITAL) )
		return true;
	
	if( (Team == XWing::Team::NONE) && (other.Team != XWing::Team::NONE) )
		return true;
	if( (Team != XWing::Team::NONE) && (other.Team == XWing::Team::NONE) )
		return false;
	if( (Team == XWing::Team::REBEL) && (other.Team != XWing::Team::REBEL) )
		return true;
	if( (Team != XWing::Team::REBEL) && (other.Team == XWing::Team::REBEL) )
		return false;
	
	if( Radius < other.Radius )
		return true;
	if( Radius > other.Radius )
		return false;
	if( CollisionDamage < other.CollisionDamage )
		return true;
	if( CollisionDamage > other.CollisionDamage )
		return false;
	
	return (strcasecmp( ShortName.c_str(), other.ShortName.c_str() ) < 0);
}


// ---------------------------------------------------------------------------


ShipClassTurret::ShipClassTurret( double fwd, double up, double right, uint8_t weapon ) : Pos3D( fwd, up, right )
{
	Weapon = weapon;
	
	Visible = true;
	CanBeHit = true;
	Health = 95.;
	ParentControl = true;
	Manual = false;
	FiringMode = 0;  // Automatically determine mode in XWingServer::SpawnShipTurrets.
	SingleShotDelay = 0.5;
	TargetArc = 360.;
	MinGunPitch = -10.;
	MaxGunPitch = 90.;
}


ShipClassTurret &ShipClassTurret::operator = ( const ShipClassTurret &other )
{
	Pos3D::Copy( &other );
	Weapon = other.Weapon;
	
	Visible = other.Visible;
	CanBeHit = other.CanBeHit;
	Health = other.Health;
	ParentControl = other.ParentControl;
	Manual = other.Manual;
	FiringMode = other.FiringMode;
	SingleShotDelay = other.SingleShotDelay;
	TargetArc = other.TargetArc;
	MinGunPitch = other.MinGunPitch;
	MaxGunPitch = other.MaxGunPitch;
	
	return *this;
}


ShipClassTurret::~ShipClassTurret()
{
}

// ---------------------------------------------------------------------------


ShipClassDockingBay::ShipClassDockingBay( double fwd, double up, double right, double radius ) : Vec3D( fwd, up, right )
{
	Radius = radius;
}


ShipClassDockingBay &ShipClassDockingBay::operator = ( const ShipClassDockingBay &other )
{
	Vec3D::Copy( &other );
	Radius = other.Radius;
	
	return *this;
}


ShipClassDockingBay::~ShipClassDockingBay()
{
}


// ---------------------------------------------------------------------------


ShipClassEngine::ShipClassEngine( double fwd, double up, double right, std::string texture, double radius, float r, float g, float b, float a ) : Vec3D( fwd, up, right )
{
	Texture = texture;
	Radius = radius;
	DrawColor.Red   = r;
	DrawColor.Green = g;
	DrawColor.Blue  = b;
	DrawColor.Alpha = a;
}


ShipClassEngine &ShipClassEngine::operator = ( const ShipClassEngine &other )
{
	Vec3D::Copy( &other );
	Texture   = other.Texture;
	Radius    = other.Radius;
	DrawColor = other.DrawColor;
	
	return *this;
}


ShipClassEngine::~ShipClassEngine()
{
}
