/*
 *  Ship.cpp
 */

#include "Ship.h"

#include <cmath>
#include "XWingDefs.h"
#include "RaptorGame.h"
#include "Num.h"
#include "Rand.h"
#include "Math3D.h"
#include "Shot.h"
#include "Asteroid.h"
#include <cassert>


Ship::Ship( uint32_t id ) : GameObject( id, XWing::Object::SHIP )
{
	Team = XWing::Team::NONE;
	ShipType = TYPE_XWING;
	CanRespawn = false;
	Group = 0;
	IsMissionObjective = false;
	
	SpecialUpdate = false;
	
	Health = 100.;
	ShieldF = 0.;
	ShieldR = 0.;
	ShieldPos = SHIELD_CENTER;
	CollisionPotential = 0.;
	
	Firing = false;
	SelectedWeapon = Shot::TYPE_LASER_GREEN;
	FiringMode = 1;
	WeaponIndex = 0;
	Target = 0;
	Ammo[ SelectedWeapon ] = -1;
}


Ship::~Ship()
{
}


void Ship::SetType( uint32_t ship_type )
{
	ShipType = ship_type;
	
	if( Data == &(Raptor::Game->Data) )
	{
		// Load client-side view model.
		
		if( ShipType == TYPE_XWING )
		{
			if( Raptor::Game->Cfg.SettingAsBool("g_hq_ships") )
				Shape.BecomeInstance( Raptor::Game->Res.GetModel("x-wing_hq.obj") );
			else
				Shape.BecomeInstance( Raptor::Game->Res.GetModel("x-wing.obj") );
			Shape.ScaleBy( 0.022 );
		}
		else if( ShipType == TYPE_YWING )
		{
			if( Raptor::Game->Cfg.SettingAsBool("g_hq_ships") )
				Shape.BecomeInstance( Raptor::Game->Res.GetModel("y-wing_hq.obj") );
			else
				Shape.BecomeInstance( Raptor::Game->Res.GetModel("y-wing.obj") );
			Shape.ScaleBy( 0.022 );
		}
		else if( ShipType == TYPE_TIE_FIGHTER )
		{
			if( Raptor::Game->Cfg.SettingAsBool("g_hq_ships") )
				Shape.BecomeInstance( Raptor::Game->Res.GetModel("tie-fighter_hq.obj") );
			else
				Shape.BecomeInstance( Raptor::Game->Res.GetModel("tie-fighter.obj") );
			Shape.ScaleBy( 0.022 );
		}
		else if( ShipType == TYPE_ISD2 )
		{
			Shape.BecomeInstance( Raptor::Game->Res.GetModel("isd2.obj") );
			Shape.ScaleBy( 0.022 );
		}
		else if( ShipType == TYPE_CORVETTE )
		{
			Shape.BecomeInstance( Raptor::Game->Res.GetModel("corvette.obj") );
			Shape.ScaleBy( 0.022 );
		}
		else if( ShipType == TYPE_NEBULON_B )
		{
			Shape.BecomeInstance( Raptor::Game->Res.GetModel("nebulon-b.obj") );
			Shape.ScaleBy( 0.022 );
		}
		else if( ShipType == TYPE_CALAMARI_CRUISER )
		{
			Shape.BecomeInstance( Raptor::Game->Res.GetModel("calamari-cruiser.obj") );
			Shape.ScaleBy( 0.022 );
		}
		else if( ShipType == TYPE_EXHAUST_PORT )
		{
			Shape.BecomeInstance( Raptor::Game->Res.GetModel("deathstar_exhaust_port.obj") );
		}
	}
	else
	{
		// Load server-side collision model (if needed).
		
		if( ShipType == TYPE_ISD2 )
		{
			Shape.LoadOBJ( "Models/isd2.obj", false );
			Shape.ScaleBy( 0.022 );
			Shape.GetMaxRadius();
		}
		else if( ShipType == TYPE_CORVETTE )
		{
			Shape.LoadOBJ( "Models/corvette.obj", false );
			Shape.ScaleBy( 0.022 );
			Shape.GetMaxRadius();
		}
		else if( ShipType == TYPE_NEBULON_B )
		{
			Shape.LoadOBJ( "Models/nebulon-b.obj", false );
			Shape.ScaleBy( 0.022 );
			Shape.GetMaxRadius();
		}
		else if( ShipType == TYPE_CALAMARI_CRUISER )
		{
			Shape.LoadOBJ( "Models/calamari-cruiser.obj", false );
			Shape.ScaleBy( 0.022 );
			Shape.GetMaxRadius();
		}
	}
	
	Reset();
}


void Ship::Reset( void )
{
	Shape.Reset();
	
	Health = MaxHealth();
	ShieldF = MaxShield();
	ShieldR = ShieldF;
	ShieldPos = SHIELD_CENTER;
	Subsystems.clear();
	CollisionPotential = 1000.;
	
	Firing = false;
	WeaponIndex = 0;
	FiringMode = 1;
	
	SelectedWeapon = Shot::TYPE_LASER_GREEN;
	if( ShipType == TYPE_XWING )
	{
		SelectedWeapon = Shot::TYPE_LASER_RED;
		Ammo[ Shot::TYPE_TORPEDO ] = 6;
		FiringClocks[ Shot::TYPE_TORPEDO ].ElapsedSeconds();
	}
	else if( ShipType == TYPE_YWING )
	{
		SelectedWeapon = Shot::TYPE_LASER_RED;
		Ammo[ Shot::TYPE_TORPEDO ] = 8;
		FiringClocks[ Shot::TYPE_TORPEDO ].ElapsedSeconds();
	}
	else if( ShipType == TYPE_TIE_FIGHTER )
		SelectedWeapon = Shot::TYPE_LASER_GREEN;
	else if( ShipType == TYPE_ISD2 )
	{
		Shape.SeedExplosionVectors( ID, 0.01 );
		SelectedWeapon = (Team == XWing::Team::EMPIRE) ? Shot::TYPE_TURBO_LASER_GREEN : Shot::TYPE_TURBO_LASER_RED;
		Subsystems[ "ShieldGenerator1" ] = 599.;
		Subsystems[ "ShieldGenerator2" ] = 599.;
		Subsystems[ "CriticalBridge" ] = 999.;
		CollisionPotential = 500000.;
	}
	else if( ShipType == TYPE_CORVETTE )
	{
		Shape.SeedExplosionVectors( ID, 0.05 );
		SelectedWeapon = (Team == XWing::Team::EMPIRE) ? Shot::TYPE_TURBO_LASER_GREEN : Shot::TYPE_TURBO_LASER_RED;
		CollisionPotential = 10000.;
	}
	else if( ShipType == TYPE_NEBULON_B )
	{
		Shape.SeedExplosionVectors( ID, 0.03 );
		SelectedWeapon = (Team == XWing::Team::EMPIRE) ? Shot::TYPE_TURBO_LASER_GREEN : Shot::TYPE_TURBO_LASER_RED;
		Subsystems[ "CriticalHull" ] = 1500.;
		CollisionPotential = 50000.;
	}
	else if( ShipType == TYPE_CALAMARI_CRUISER )
	{
		Shape.SeedExplosionVectors( ID, 0.015 );
		SelectedWeapon = (Team == XWing::Team::EMPIRE) ? Shot::TYPE_TURBO_LASER_GREEN : Shot::TYPE_TURBO_LASER_RED;
		Subsystems[ "ShieldGenerator" ] = 1199.;
		Subsystems[ "CriticalBridge" ] = 999.;
		CollisionPotential = 500000.;
	}
	else if( ShipType == TYPE_EXHAUST_PORT )
	{
		Shape.SeedExplosionVectors( ID, 0. );
		SelectedWeapon = 0;
		CollisionPotential = 0.;
	}
	
	Ammo[ SelectedWeapon ] = -1;
	FiringClocks[ SelectedWeapon ].Reset();
	
	Target = 0;
}


void Ship::SetHealth( double health )
{
	if( (health <= 0.) && (Health > 0.) )
	{
		DeathClock.Reset();
		ShieldF = 0.;
		ShieldR = 0.;
	}
	else if( (health > 0.) && (Health <= 0.) )
		Reset();
	
	Health = health;
}


void Ship::AddDamage( double front, double rear, const char *subsystem )
{
	// Damage to shield towers can't be blocked by shields, and doesn't carry to the hull.
	if( subsystem && (strncmp( subsystem, "ShieldGenerator", strlen("ShieldGenerator") ) == 0) && (Subsystems.find(subsystem) != Subsystems.end()) )
	{
		Subsystems[ subsystem ] -= (front + rear);
		
		if( Subsystems[ subsystem ] <= 0. )
		{
			Subsystems[ subsystem ] = 0.;
			
			int num_shield_towers = 0;
			for( std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.begin(); subsystem_iter != Subsystems.end(); subsystem_iter ++ )
			{
				if( strncmp( subsystem_iter->first.c_str(), "ShieldGenerator", strlen("ShieldGenerator") ) == 0 )
					num_shield_towers ++;
			}
			
			ShieldF -= MaxShield() / num_shield_towers;
			ShieldR -= MaxShield() / num_shield_towers;
			
			if( ShieldF < 0. )
				ShieldF = 0.;
			if( ShieldR < 0. )
				ShieldR = 0.;
		}
		
		front = 0.;
		rear = 0.;
	}
	
	double hull_damage = front + rear;
	
	if( ShieldF > front )
	{
		hull_damage -= front;
		ShieldF -= front;
	}
	else
	{
		hull_damage -= ShieldF;
		ShieldF = 0.;
	}
	
	if( ShieldR > rear )
	{
		hull_damage -= rear;
		ShieldR -= rear;
	}
	else
	{
		hull_damage -= ShieldR;
		ShieldR = 0.;
	}
	
	// Damage to most subsystems is carried on to the hull too.
	if( subsystem && (Subsystems.find(subsystem) != Subsystems.end()) )
	{
		Subsystems[ subsystem ] -= hull_damage;
		
		if( Subsystems[ subsystem ] <= 0. )
		{
			Subsystems[ subsystem ] = 0.;
			
			// Destroying the bridge destroys the whole ship.
			if( strncmp( subsystem, "Critical", strlen("Critical") ) == 0 )
				hull_damage = Health + 1.;
		}
	}
	
	SetHealth( Health - hull_damage );
	
	HitClock.Reset();
}


void Ship::Explode( double dt )
{
	if( ShipType == TYPE_EXHAUST_PORT )
		return;
	
	Shape.Explode( dt );
}


void Ship::SetRoll( double roll )
{
	if( roll > 1. )
		roll = 1.;
	else if( roll < -1. )
		roll = -1.;
	
	RollRate = roll * MaxRoll();
}


void Ship::SetPitch( double pitch )
{
	if( pitch > 1. )
		pitch = 1.;
	else if( pitch < -1. )
		pitch = -1.;
	
	PitchRate = pitch * MaxPitch();
}


void Ship::SetYaw( double yaw )
{
	if( yaw > 1. )
		yaw = 1.;
	else if( yaw < -1. )
		yaw = -1.;
	
	YawRate = yaw * MaxYaw();
}


double Ship::GetThrottle( void ) const
{
	double max_speed = MaxSpeed();
	return max_speed ? MotionVector.Length() / max_speed : 0.;
}


void Ship::SetThrottle( double throttle, double dt )
{
	if( throttle > 1. )
		throttle = 1.;
	else if( throttle < 0. )
		throttle = 0.;
	
	double old_speed = MotionVector.Length();
	double new_speed = throttle * MaxSpeed();
	double max_change = dt * Acceleration();
	
	if( fabs( new_speed - old_speed ) > max_change )
		new_speed = old_speed + max_change * Num::Sign( new_speed - old_speed );
	
	MotionVector = Fwd;
	MotionVector.ScaleTo( new_speed );
}


void Ship::SetShieldPos( uint8_t pos )
{
	if( pos == ShieldPos )
		return;
	
	ShieldPos = pos;
	double total_shield = ShieldF + ShieldR;
	double max_shield = MaxShield();
	
	if( ShieldPos == Ship::SHIELD_FRONT )
	{
		if( total_shield > max_shield )
		{
			ShieldF = max_shield;
			ShieldR = total_shield - ShieldF;
		}
		else
		{
			ShieldF = total_shield;
			ShieldR = 0.;
		}
	}
	else if( ShieldPos == Ship::SHIELD_REAR )
	{
		if( total_shield > max_shield )
		{
			ShieldR = max_shield;
			ShieldF = total_shield - ShieldR;
		}
		else
		{
			ShieldR = total_shield;
			ShieldF = 0.;
		}
	}
	else
	{
		ShieldF = total_shield / 2.;
		ShieldR = ShieldF;
	}
}


double Ship::Radius( void ) const
{
	if( ShipType == TYPE_ISD2 )
		return 900.;
	else if( ShipType == TYPE_CORVETTE )
		return 80.;
	else if( ShipType == TYPE_NEBULON_B )
		return 200.;
	else if( ShipType == TYPE_CALAMARI_CRUISER )
		return 650.;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 1.;
	
	return 4.5;
}


double Ship::MaxSpeed( void ) const
{
	if( ShipType == TYPE_XWING )
		return 170.;
	else if( ShipType == TYPE_YWING )
		return 150.;
	else if( ShipType == TYPE_TIE_FIGHTER )
		return 200.;
	else if( ShipType == TYPE_ISD2 )
		return 25.;
	else if( ShipType == TYPE_CORVETTE )
		return 25.;
	else if( ShipType == TYPE_NEBULON_B )
		return 25.;
	else if( ShipType == TYPE_CALAMARI_CRUISER )
		return 25.;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 0.;
	
	return 100.;
}


double Ship::Acceleration( void ) const
{
	if( ShipType == TYPE_YWING )
		return MaxSpeed() / 3.;
	
	return MaxSpeed() / 2.;
}


double Ship::MaxRoll( void ) const
{
	if( ShipType == TYPE_XWING )
		return 180.;
	else if( ShipType == TYPE_YWING )
		return 120.;
	else if( ShipType == TYPE_TIE_FIGHTER )
		return 240.;
	else if( ShipType == TYPE_ISD2 )
		return 0.5;
	else if( ShipType == TYPE_CORVETTE )
		return 5.;
	else if( ShipType == TYPE_NEBULON_B )
		return 1.;
	else if( ShipType == TYPE_CALAMARI_CRUISER )
		return 0.5;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 0.;
	
	return 180.;
}


double Ship::MaxPitch( void ) const
{
	if( ShipType == TYPE_XWING )
		return 100.;
	else if( ShipType == TYPE_YWING )
		return 70.;
	else if( ShipType == TYPE_TIE_FIGHTER )
		return 120.;
	else if( ShipType == TYPE_ISD2 )
		return 0.5;
	else if( ShipType == TYPE_CORVETTE )
		return 3.;
	else if( ShipType == TYPE_NEBULON_B )
		return 1.;
	else if( ShipType == TYPE_CALAMARI_CRUISER )
		return 0.5;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 0.;
	
	return 90.;
}


double Ship::MaxYaw( void ) const
{
	if( ShipType == TYPE_XWING )
		return 80.;
	else if( ShipType == TYPE_YWING )
		return 60.;
	else if( ShipType == TYPE_TIE_FIGHTER )
		return 90.;
	else if( ShipType == TYPE_ISD2 )
		return 0.5;
	else if( ShipType == TYPE_CORVETTE )
		return 3.;
	else if( ShipType == TYPE_NEBULON_B )
		return 1.5;
	else if( ShipType == TYPE_CALAMARI_CRUISER )
		return 0.5;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 0.;
	
	return 90.;
}


double Ship::MaxHealth( void ) const
{
	if( ShipType == TYPE_YWING )
		return 115.;
	else if( ShipType == TYPE_ISD2 )
		return 25000.;
	else if( ShipType == TYPE_CORVETTE )
		return 2500.;
	else if( ShipType == TYPE_NEBULON_B )
		return 7500.;
	else if( ShipType == TYPE_CALAMARI_CRUISER )
		return 15000.;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 0.1;
	
	return 100.;
}


double Ship::MaxShield( void ) const
{
	if( ShipType == TYPE_XWING )
		return 100.;
	else if( ShipType == TYPE_YWING )
		return 150.;
	else if( ShipType == TYPE_TIE_FIGHTER )
		return 0.;
	else if( ShipType == TYPE_ISD2 )
		return 100000.;
	else if( ShipType == TYPE_CORVETTE )
		return 1500.;
	else if( ShipType == TYPE_NEBULON_B )
		return 3000.;
	else if( ShipType == TYPE_CALAMARI_CRUISER )
		return 50000.;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 149.8;
	
	return 0.;
}


double Ship::ShieldRechargeDelay( void ) const
{
	if( ShipType == TYPE_XWING )
		return 5.;
	else if( ShipType == TYPE_YWING )
		return 5.;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 0.;
	
	return 60.;
}


double Ship::ShieldRechargeRate( void ) const
{
	if( ShipType == TYPE_XWING )
		return 3.;
	else if( ShipType == TYPE_YWING )
		return 4.;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 9999.;
	
	return 0.;
}


double Ship::PiecesDangerousTime( void ) const
{
	if( ComplexCollisionDetection() )
		return 5.;
	
	return 0.5;
}


bool Ship::PlayersCanFly( void ) const
{
	if( ShipType == TYPE_ISD2 )
		return false;
	else if( ShipType == TYPE_CORVETTE )
		return false;
	else if( ShipType == TYPE_NEBULON_B )
		return false;
	else if( ShipType == TYPE_CALAMARI_CRUISER )
		return false;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return false;
	
	return true;
}


std::map<int,Shot*> Ship::NextShots( GameObject *target ) const
{
	std::map<int,Shot*> shots;
	
	if( ! SelectedWeapon )
		return shots;
	
	// If ammo is limited, don't fire more than the ammo we have.
	int firing_mode = FiringMode;
	std::map<uint32_t,int8_t>::const_iterator ammo_iter = Ammo.find(SelectedWeapon);
	if( (ammo_iter != Ammo.end()) && (ammo_iter->second >= 0) && (ammo_iter->second < firing_mode) )
		firing_mode = ammo_iter->second;
	
	for( int num = 0; num < firing_mode; num ++ )
	{
		int weapon_index = WeaponIndex + num;
		
		Shot *shot = new Shot();
		shot->Copy( this );
		shot->FiredFrom = ID;
		shot->PlayerID = PlayerID;
		shot->ShotType = SelectedWeapon;
		shot->MotionVector.Set( Fwd.X, Fwd.Y, Fwd.Z );
		shot->MotionVector.ScaleTo( shot->Speed() );
		
		double fwd = 0., up = 0., right = 0.;
		
		if( ShipType == TYPE_XWING )
		{
			if( SelectedWeapon == Shot::TYPE_TORPEDO )
			{
				weapon_index %= 2;
				
				if( weapon_index == 0 )
					right = 0.5;
				else
					right = -0.5;
				up = -0.2;
				
				if( target )
					shot->Seeking = target->ID;
			}
			else
			{
				if( (firing_mode == 2) && num )
					weapon_index ++;
				
				weapon_index %= 4;
				
				if( weapon_index == 0 )
				{
					right = 4.8;
					up = 1.3;
				}
				else if( weapon_index == 1 )
				{
					right = -4.8;
					up = 1.3;
				}
				else if( weapon_index == 2 )
				{
					right = -4.8;
					up = -1.4;
				}
				else
				{
					right = 4.8;
					up = -1.4;
				}
			}
		}
		else if( ShipType == TYPE_YWING )
		{
			if( SelectedWeapon == Shot::TYPE_TORPEDO )
			{
				weapon_index %= 2;
				
				if( weapon_index == 0 )
					right = 0.5;
				else
					right = -0.5;
				up = -0.2;
				fwd = 4.;
				
				if( target )
					shot->Seeking = target->ID;
			}
			else
			{
				weapon_index %= 2;
				
				if( weapon_index == 0 )
					right = 0.4;
				else
					right = -0.4;
				fwd = 7.;
			}
		}
		else if( ShipType == TYPE_TIE_FIGHTER )
		{
			weapon_index %= 2;
			
			up = -0.8;
			if( weapon_index == 0 )
				right = 0.4;
			else
				right = -0.4;
		}
		else if( ShipType == TYPE_ISD2 )
		{
			fwd = Rand::Double( -400., 400. );
			right = Rand::Double( -50., 50. );
			up = Rand::Double( -120., -80. );
		}
		else if( ShipType == TYPE_CALAMARI_CRUISER )
		{
			fwd = Rand::Double( -200., 200. );
			right = Rand::Double( -50., 50. );
			up = Rand::Double( -50., 50. );
		}
		else if( ShipType == TYPE_NEBULON_B )
		{
			if( Rand::Bool(0.65) )
			{
				fwd = Rand::Double( 120., 140. );
				up = Rand::Double( -100., -20. );
			}
			else
			{
				fwd = Rand::Double( -60., -80. );
				up = Rand::Double( -10., 10. );
			}
		}
		else if( ShipType == TYPE_CORVETTE )
		{
			bool top = Rand::Bool();
			if( target )
			{
				double dist_up = target->DistAlong( &Up, this );
				if( fabs(dist_up) > target->Dist(this) * 0.5 )
					top = (dist_up >= 0.);
			}
			
			fwd = 37.5;
			up = (top ? 12. : -12.);
		}
		
		shot->MoveAlong( &Fwd, fwd );
		shot->MoveAlong( &Up, up );
		shot->MoveAlong( &Right, right );
		
		shots[ weapon_index ] = shot;
		
		// Aim turrets at the intercept point.
		if( target && (shot->ShotType == Shot::TYPE_TURBO_LASER_GREEN || shot->ShotType == Shot::TYPE_TURBO_LASER_RED) )
		{
			// First aim at the ship.
			Vec3D vec_to_target( target->X - shot->X, target->Y - shot->Y, target->Z - shot->Z );
			shot->Fwd.Copy( &vec_to_target );
			shot->Fwd.ScaleTo( 1. );
			shot->FixVectors();
			shot->MotionVector.Copy( &(shot->Fwd) );
			shot->MotionVector.ScaleTo( shot->Speed() );
			
			// Adjust for the intercept point.
			double dist_to_target = vec_to_target.Length();
			Vec3D shot_vec = shot->MotionVector;
			shot_vec -= target->MotionVector;
			double time_to_target = dist_to_target / shot_vec.Length();
			Vec3D vec_to_intercept = vec_to_target;
			vec_to_intercept.X += target->MotionVector.X * time_to_target;
			vec_to_intercept.Y += target->MotionVector.Y * time_to_target;
			vec_to_intercept.Z += target->MotionVector.Z * time_to_target;
			shot->Fwd.Copy( &vec_to_intercept );
			shot->Fwd.ScaleTo( 1. );
			shot->FixVectors();
			shot->MotionVector.Copy( &(shot->Fwd) );
			shot->MotionVector.ScaleTo( shot->Speed() );
		}
	}
	
	return shots;
}


std::map<int,Shot*> Ship::AllShots( GameObject *target )
{
	uint8_t firing_mode = FiringMode;
	
	if( (ShipType == TYPE_XWING) && (SelectedWeapon == Shot::TYPE_LASER_RED) )
		FiringMode = 4;
	else
		FiringMode = 2;
	
	std::map<int,Shot*> shots = NextShots( target );
	
	FiringMode = firing_mode;
	
	return shots;
}


void Ship::JustFired( void )
{
	FiringClocks[ SelectedWeapon ].Reset();
	
	if( Ammo[ SelectedWeapon ] > 0 )
	{
		if( Ammo[ SelectedWeapon ] > FiringMode )
			Ammo[ SelectedWeapon ] -= FiringMode;
		else
			Ammo[ SelectedWeapon ] = 0;
	}
	
	int weapon_count = 1;
	if( ShipType == TYPE_XWING )
	{
		if( SelectedWeapon == Shot::TYPE_TORPEDO )
			weapon_count = 2;
		else
			weapon_count = 4;
	}
	else if( ShipType == TYPE_YWING )
		weapon_count = 2;
	else if( ShipType == TYPE_TIE_FIGHTER )
		weapon_count = 2;
	
	WeaponIndex ++;
	if( WeaponIndex >= weapon_count )
		WeaponIndex = 0;
}


void Ship::NextWeapon( void )
{
	if( ShipType == TYPE_XWING )
	{
		if( SelectedWeapon == Shot::TYPE_LASER_RED )
			SelectedWeapon = Shot::TYPE_TORPEDO;
		else
			SelectedWeapon = Shot::TYPE_LASER_RED;
	}
	else if( ShipType == TYPE_YWING )
	{
		if( SelectedWeapon == Shot::TYPE_LASER_RED )
			SelectedWeapon = Shot::TYPE_TORPEDO;
		else
			SelectedWeapon = Shot::TYPE_LASER_RED;
	}
	
	FiringMode = 1;
	WeaponIndex = 0;
}


double Ship::ShotDelay( void ) const
{
	if( ShipType == TYPE_CORVETTE )
		return 0.4;
	else if( ShipType == TYPE_NEBULON_B )
		return 0.8;
	else if( ShipType == TYPE_ISD2 )
		return 0.67;
	else if( ShipType == TYPE_CALAMARI_CRUISER )
		return 0.75;
	
	if( SelectedWeapon == Shot::TYPE_TORPEDO )
		return 1. * FiringMode;
	
	return 0.25 * FiringMode;
}


void Ship::NextFiringMode( void )
{
	FiringMode *= 2;
	
	if( ShipType == TYPE_XWING )
	{
		if( (SelectedWeapon == Shot::TYPE_LASER_RED) && (FiringMode > 4) )
			FiringMode = 1;
		else if( (SelectedWeapon == Shot::TYPE_TORPEDO) && (FiringMode > 2) )
			FiringMode = 1;
	}
	else if( ShipType == TYPE_YWING )
	{
		if( (SelectedWeapon == Shot::TYPE_LASER_RED) && (FiringMode > 2) )
			FiringMode = 1;
		else if( (SelectedWeapon == Shot::TYPE_TORPEDO) && (FiringMode > 2) )
			FiringMode = 1;
	}
	else if( ShipType == TYPE_TIE_FIGHTER )
	{
		if( (SelectedWeapon == Shot::TYPE_LASER_GREEN) && (FiringMode > 2) )
			FiringMode = 1;
	}
	
	if( (Ammo[ SelectedWeapon ] >= 0) && (Ammo[ SelectedWeapon ] < FiringMode) )
		FiringMode = Ammo[ SelectedWeapon ];
}


bool Ship::PlayerShouldUpdateServer( void ) const
{
	return (Health > 0.);
}

bool Ship::ServerShouldUpdatePlayer( void ) const
{
	return (Health <= 0.) || SpecialUpdate;
}

bool Ship::ServerShouldUpdateOthers( void ) const
{
	return true;
}

bool Ship::CanCollideWithOwnType( void ) const
{
	return true;
}

bool Ship::CanCollideWithOtherTypes( void ) const
{
	return true;
}

bool Ship::ComplexCollisionDetection( void ) const
{
	if( ShipType == TYPE_ISD2 || ShipType == TYPE_CORVETTE || ShipType == TYPE_NEBULON_B || ShipType == TYPE_CALAMARI_CRUISER )
		return true;
	
	return false;
}


void Ship::AddToInitPacket( Packet *packet, int8_t precision )
{
	GameObject::AddToInitPacket( packet, precision );
	packet->AddUInt( Team );
	packet->AddUChar( Group );
	packet->AddUInt( ShipType );
	packet->AddString( Name );
}


void Ship::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, precision );
	Team = packet->NextUInt();
	Group = packet->NextUChar();
	SetType( packet->NextUInt() );
	Name = packet->NextString();
}


void Ship::AddToUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacketFromServer( packet, precision );
	packet->AddFloat( Health );
	packet->AddUChar( ShieldPos );
	packet->AddUInt( Target );
	//packet->AddUInt( SelectedWeapon );
	//packet->AddUChar( FiringMode );
	//packet->AddUChar( WeaponIndex );
	
	SpecialUpdate = false;
}


void Ship::ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::ReadFromUpdatePacketFromServer( packet, precision );
	SetHealth( packet->NextFloat() );
	SetShieldPos( packet->NextUChar() );
	Target = packet->NextUInt();
	//SelectedWeapon = packet->NextInt();
	//FiringMode = packet->NextUChar();
	//WeaponIndex = packet->NextUChar();
}


void Ship::AddToUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacketFromClient( packet, precision );
	packet->AddUChar( Firing );
	packet->AddUChar( ShieldPos );
	packet->AddUInt( Target );
	packet->AddUInt( SelectedWeapon );
	packet->AddUChar( FiringMode );
}


void Ship::ReadFromUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	// Dirty hack so we don't get client position data for a dead ship.
	if( Health <= 0. )
	{
		Ship temp_ship;
		temp_ship.Health = 100.;
		temp_ship.ReadFromUpdatePacketFromClient( packet, precision );
		return;
	}
	
	uint32_t prev_selected_weapon = SelectedWeapon;
	
	GameObject::ReadFromUpdatePacketFromClient( packet, precision );
	Firing = packet->NextUChar();
	SetShieldPos( packet->NextUChar() );
	Target = packet->NextUInt();
	SelectedWeapon = packet->NextUInt();
	FiringMode = packet->NextUChar();
	
	if( SelectedWeapon != prev_selected_weapon )
		WeaponIndex = 0;
}


bool Ship::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object ) const
{
	// Dead ships don't collide after a bit (except capital ship chunks).
	if( (Health <= 0.) && (DeathClock.ElapsedSeconds() > PiecesDangerousTime()) )
		return false;
	
	if( other->Type() == XWing::Object::SHOT )
	{
		Shot *shot = (Shot*) other;
		
		// Ships can't shoot themselves.
		if( shot->FiredFrom == ID )
			return false;
		
		// Use face hit detection for capital ships.
		if( ComplexCollisionDetection() )
		{
			if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius * (1. + Shape.ExplodedSeconds * 0.25)) )
				return false;
			
			Pos3D end( other );
			end += (other->MotionVector * dt) - (MotionVector * dt);
			ModelArrays array_inst;
			for( std::map<std::string,ModelObject>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
			{
				// Don't detect collisions with destroyed subsystems.
				std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find( obj_iter->first );
				if( (subsystem_iter != Subsystems.end()) && (subsystem_iter->second <= 0.) )
					continue;
				
				// Get the worldspace center of object.
				Pos3D modelspace_center = obj_iter->second.CenterPoint;
				Vec3D offset = Fwd * modelspace_center.X + Up * modelspace_center.Y + Right * modelspace_center.Z;
				if( Shape.ExplodedSeconds )
					offset += obj_iter->second.ExplosionMotion * Shape.ExplodedSeconds;
				Pos3D center = *this + offset;
				
				// If these two objects don't pass near each other, don't bother checking faces.
				// FIXME: Apply worldspace_explosion_motion to MotionVector.
				if( Math3D::MinimumDistance( &center, &MotionVector, other, &(other->MotionVector), dt ) > obj_iter->second.MaxRadius )
					continue;
				
				for( std::map<std::string,ModelArrays>::const_iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
				{
					array_inst.BecomeInstance( &(array_iter->second), false );
					
					if( Shape.ExplodedSeconds )
					{
						Pos3D draw_pos( this );
						
						// Convert explosion vectors to worldspace.
						Vec3D explosion_motion = obj_iter->second.ExplosionMotion * Shape.ExplodedSeconds;
						Vec3D explosion_rotation_axis = (Fwd * obj_iter->second.ExplosionRotationAxis.X) + (Up * obj_iter->second.ExplosionRotationAxis.Y) + (Right * obj_iter->second.ExplosionRotationAxis.Z);
						
						draw_pos.MoveAlong( &Fwd, explosion_motion.X );
						draw_pos.MoveAlong( &Up, explosion_motion.Y );
						draw_pos.MoveAlong( &Right, explosion_motion.Z );
						
						draw_pos.Fwd.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
						draw_pos.Up.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
						draw_pos.Right.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
						
						array_inst.MakeWorldSpace( &draw_pos );
					}
					else
						array_inst.MakeWorldSpace( this );
					
					for( int i = 0; i + 2 < array_inst.VertexCount; i += 3 )
					{
						double dist = Math3D::LineSegDistFromFace( other, &end, array_inst.WorldSpaceVertexArray + i*3, 3 );
						if( dist < 0.1 )
						{
							if( this_object )
								*this_object = obj_iter->first;
							return true;
						}
					}
				}
			}
			return false;
		}
		
		double dist = Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt );
		
		if( dist <= (Radius() * 2.) )
			return true;
	}
	
	else if( other->Type() == XWing::Object::SHIP )
	{
		Ship *ship = (Ship*) other;
		
		// Let capitol ships handle collisions.
		if( ship->ComplexCollisionDetection() && ! ComplexCollisionDetection() )
			return other->WillCollide( this, dt, other_object, this_object );
		
		// Don't let ships hit the Death Star exhaust port.
		if( (ShipType == TYPE_EXHAUST_PORT) || (ship->ShipType == TYPE_EXHAUST_PORT) )
			return false;
		
		// Dead ships don't collide after a bit, and they never hit other dead ships.
		if( (ship->Health <= 0.) && ((Health <= 0.) || (ship->DeathClock.ElapsedSeconds() > ship->PiecesDangerousTime())) )
			return false;
		
		// Use face hit detection for capital ships.
		if( ComplexCollisionDetection() )
		{
			if( ship->ComplexCollisionDetection() )
			{
				// The other ship also has complex collision model!
				
				// If they're nowhere near each other, don't bother getting fancy.
				if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius * (1. + Shape.ExplodedSeconds * 0.25) + ship->Shape.MaxRadius * (1. + ship->Shape.ExplodedSeconds * 0.25)) )
					return false;
				
				// Let the bigger ship handle the collision.
				if( ship->Shape.MaxRadius > Shape.MaxRadius )
					return ship->WillCollide( this, dt, other_object, this_object );
				
				ModelArrays array_inst;
				std::map<const ModelArrays*,ModelArrays> other_arrays;
				Vec3D motion = (MotionVector * dt) - (other->MotionVector * dt);
				
				for( std::map<std::string,ModelObject>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
				{
					// Don't detect collisions with destroyed subsystems.
					std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find( obj_iter->first );
					if( (subsystem_iter != Subsystems.end()) && (subsystem_iter->second <= 0.) )
						continue;
					
					Pos3D center = obj_iter->second.CenterPoint;
					center.X += X;
					center.Y += Y;
					center.Z += Z;
					
					if( Shape.ExplodedSeconds )
					{
						Vec3D worldspace_explosion_motion = Fwd * obj_iter->second.ExplosionMotion.X + Up * obj_iter->second.ExplosionMotion.Y + Right * obj_iter->second.ExplosionMotion.Z;
						center += worldspace_explosion_motion * Shape.ExplodedSeconds;
					}
					
					std::vector<const ModelObject*> other_objects;
					
					for( std::map<std::string,ModelObject>::const_iterator other_obj_iter = ship->Shape.Objects.begin(); other_obj_iter != ship->Shape.Objects.end(); other_obj_iter ++ )
					{
						// Don't detect collisions with destroyed subsystems.
						std::map<std::string,double>::const_iterator subsystem_iter = ship->Subsystems.find( other_obj_iter->first );
						if( (subsystem_iter != ship->Subsystems.end()) && (subsystem_iter->second <= 0.) )
							continue;
						
						Pos3D other_center = other_obj_iter->second.CenterPoint;
						other_center.X += other->X;
						other_center.Y += other->Y;
						other_center.Z += other->Z;

						if( ship->Shape.ExplodedSeconds )
						{
							Vec3D worldspace_explosion_motion = other->Fwd * other_obj_iter->second.ExplosionMotion.X + other->Up * other_obj_iter->second.ExplosionMotion.Y + other->Right * other_obj_iter->second.ExplosionMotion.Z;
							other_center += worldspace_explosion_motion * ship->Shape.ExplodedSeconds;
						}
						
						// If these two objects don't pass near each other, don't bother checking faces.
						// FIXME: Apply worldspace_explosion_motion to MotionVector.
						if( Math3D::MinimumDistance( &center, &MotionVector, &other_center, &(other->MotionVector), dt ) > (obj_iter->second.MaxRadius + other_obj_iter->second.MaxRadius) )
							continue;
						
						// This object is worth checking.
						other_objects.push_back( &(other_obj_iter->second) );
					}
					
					if( other_objects.size() )
					{
						for( std::map<std::string,ModelArrays>::const_iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
						{
							// Make the worldspace arrays for this.
							array_inst.BecomeInstance( &(array_iter->second), false );
							
							if( Shape.ExplodedSeconds )
							{
								Pos3D draw_pos( this );
								
								// Convert explosion vectors to worldspace.
								Vec3D explosion_motion = obj_iter->second.ExplosionMotion * Shape.ExplodedSeconds;
								Vec3D explosion_rotation_axis = (Fwd * obj_iter->second.ExplosionRotationAxis.X) + (Up * obj_iter->second.ExplosionRotationAxis.Y) + (Right * obj_iter->second.ExplosionRotationAxis.Z);
								
								draw_pos.MoveAlong( &Fwd, explosion_motion.X );
								draw_pos.MoveAlong( &Up, explosion_motion.Y );
								draw_pos.MoveAlong( &Right, explosion_motion.Z );
								
								draw_pos.Fwd.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
								draw_pos.Up.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
								draw_pos.Right.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
								
								array_inst.MakeWorldSpace( &draw_pos );
							}
							else
								array_inst.MakeWorldSpace( this );
							
							GLdouble *worldspace = array_inst.WorldSpaceVertexArray;
							int vertex_count = array_inst.VertexCount;
							
							// Find objects of the other ship worth considering for point-by-point with this array.
							std::vector<const ModelObject*> other_close_objects;
							for( std::vector<const ModelObject*>::const_iterator other_obj_iter = other_objects.begin(); other_obj_iter != other_objects.end(); other_obj_iter ++ )
							{
								Pos3D other_start = (*other_obj_iter)->CenterPoint;
								other_start.X += other->X;
								other_start.Y += other->Y;
								other_start.Z += other->Z;
								Pos3D other_end( &other_start );
								other_end -= motion;
								
								if( ship->Shape.ExplodedSeconds )
								{
									Vec3D worldspace_explosion_motion = other->Fwd * (*other_obj_iter)->ExplosionMotion.X + other->Up * (*other_obj_iter)->ExplosionMotion.Y + other->Right * (*other_obj_iter)->ExplosionMotion.Z;
									other_start += worldspace_explosion_motion * ship->Shape.ExplodedSeconds;
									other_end += worldspace_explosion_motion * (ship->Shape.ExplodedSeconds + dt);
								}
								
								for( int i = 0; i + 2 < array_inst.VertexCount; i += 3 )
								{
									double dist = Math3D::LineSegDistFromFace( &other_start, &other_end, array_inst.WorldSpaceVertexArray + i*3, 3 );
									if( dist <= ship->Radius() )
									{
										other_close_objects.push_back( *other_obj_iter );
										break;
									}
								}
							}
							
							// All filters have passed, so it's time to do the hard work.
							for( std::vector<const ModelObject*>::const_iterator other_obj_iter = other_close_objects.begin(); other_obj_iter != other_close_objects.end(); other_obj_iter ++ )
							{
								for( std::map<std::string,ModelArrays>::const_iterator other_array_iter = (*other_obj_iter)->Arrays.begin(); other_array_iter != (*other_obj_iter)->Arrays.end(); other_array_iter ++ )
								{
									if( other_arrays.find( &(other_array_iter->second) ) == other_arrays.end() )
									{
										other_arrays[ &(other_array_iter->second) ].BecomeInstance( &(other_array_iter->second), false );

										if( ship->Shape.ExplodedSeconds )
										{
											Pos3D draw_pos( other );
											
											// Convert explosion vectors to worldspace.
											Vec3D explosion_motion = (*other_obj_iter)->ExplosionMotion * ship->Shape.ExplodedSeconds;
											Vec3D explosion_rotation_axis = (other->Fwd * (*other_obj_iter)->ExplosionRotationAxis.X) + (other->Up * (*other_obj_iter)->ExplosionRotationAxis.Y) + (other->Right * (*other_obj_iter)->ExplosionRotationAxis.Z);
											
											draw_pos.MoveAlong( &(other->Fwd), explosion_motion.X );
											draw_pos.MoveAlong( &(other->Up), explosion_motion.Y );
											draw_pos.MoveAlong( &(other->Right), explosion_motion.Z );
											
											draw_pos.Fwd.RotateAround( &explosion_rotation_axis, ship->Shape.ExplodedSeconds * (*other_obj_iter)->ExplosionRotationRate );
											draw_pos.Up.RotateAround( &explosion_rotation_axis, ship->Shape.ExplodedSeconds * (*other_obj_iter)->ExplosionRotationRate );
											draw_pos.Right.RotateAround( &explosion_rotation_axis, ship->Shape.ExplodedSeconds * (*other_obj_iter)->ExplosionRotationRate );
											
											other_arrays[ &(other_array_iter->second) ].MakeWorldSpace( &draw_pos );
										}
										else
											other_arrays[ &(other_array_iter->second) ].MakeWorldSpace( other );
									}
									
									GLdouble *other_worldspace = other_arrays[ &(other_array_iter->second) ].WorldSpaceVertexArray;
									int other_vertex_count = other_arrays[ &(other_array_iter->second) ].VertexCount;
									
									//std::vector<Pos3D> vertices1;
									std::vector<Pos3D> vertices2;
									for( int i = 0; i < other_vertex_count; i ++ )
									{
										//vertices1.push_back( Pos3D( other_worldspace[ i*3 ], other_worldspace[ i*3 + 1 ], other_worldspace[ i*3 + 2 ] ) );
										vertices2.push_back( Pos3D( other_worldspace[ i*3 ] - motion.X, other_worldspace[ i*3 + 1 ] - motion.Y, other_worldspace[ i*3 + 2 ] - motion.Z ) );
									}
									
									if( ship->Shape.ExplodedSeconds )
									{
										Vec3D worldspace_explosion_motion = other->Fwd * (*other_obj_iter)->ExplosionMotion.X + other->Up * (*other_obj_iter)->ExplosionMotion.Y + other->Right * (*other_obj_iter)->ExplosionMotion.Z;
										
										for( int i = 0; i < other_vertex_count; i ++ )
										{
											vertices2[ i ] += (*other_obj_iter)->ExplosionMotion * dt;
											// FIXME: Apply explosion rotation too!
										}
									}
									
									// See if any of the edges will be crossing their faces.
									for( int i = 0; i + 2 < vertex_count; i += 3 )
									{
										for( int j = 0; j + 2 < other_vertex_count; j += 3 )
										{
											if( Math3D::LineIntersectsFace( &(vertices2[ j ]), &(vertices2[ j + 1 ]), worldspace + i*3, 3 ) )
											{
												if( this_object )
													*this_object = obj_iter->first;
												if( other_object )
													*other_object = (*other_obj_iter)->Name;
												return true;
											}
											if( Math3D::LineIntersectsFace( &(vertices2[ j + 1 ]), &(vertices2[ j + 2 ]), worldspace + i*3, 3 ) )
											{
												if( this_object )
													*this_object = obj_iter->first;
												if( other_object )
													*other_object = (*other_obj_iter)->Name;
												return true;
											}
											if( Math3D::LineIntersectsFace( &(vertices2[ j + 2 ]), &(vertices2[ j ]), worldspace + i*3, 3 ) )
											{
												if( this_object )
													*this_object = obj_iter->first;
												if( other_object )
													*other_object = (*other_obj_iter)->Name;
												return true;
											}
										}
									}
									
									/*
									// See if any of the edges already cross their faces.
									for( int i = 0; i + 2 < vertex_count; i += 3 )
									{
										for( int j = 0; j + 2 < other_vertex_count; j += 3 )
										{
											if( Math3D::LineIntersectsFace( &(vertices1[ j ]), &(vertices1[ j + 1 ]), worldspace + i*3, 3 ) )
												return true;
											if( Math3D::LineIntersectsFace( &(vertices1[ j + 1 ]), &(vertices1[ j + 2 ]), worldspace + i*3, 3 ) )
												return true;
											if( Math3D::LineIntersectsFace( &(vertices1[ j + 2 ]), &(vertices1[ j + 1 ]), worldspace + i*3, 3 ) )
												return true;
										}
									}
									
									// See if any of the vertices will cross their faces while moving.
									for( int i = 0; i + 2 < vertex_count; i += 3 )
									{
										for( int j = 0; j < other_vertex_count; j ++ )
										{
											if( Math3D::LineIntersectsFace( &(vertices1[ j ]), &(vertices2[ j ]), worldspace + i*3, 3 ) )
												return true;
										}
									}
									*/
								}
							}
						}
					}
				}
			}
			else
			{
				// The other ship uses a simple spherical collision model.
				
				if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius * (1. + Shape.ExplodedSeconds * 0.25) + ship->Radius()) )
					return false;
				
				Pos3D end( other );
				end += (other->MotionVector * dt) - (MotionVector * dt);
				ModelArrays array_inst;
				for( std::map<std::string,ModelObject>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
				{
					// Don't detect collisions with destroyed subsystems.
					std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find( obj_iter->first );
					if( (subsystem_iter != Subsystems.end()) && (subsystem_iter->second <= 0.) )
						continue;
					
					// Get the worldspace center of object.
					Pos3D modelspace_center = obj_iter->second.CenterPoint;
					Vec3D offset = Fwd * modelspace_center.X + Up * modelspace_center.Y + Right * modelspace_center.Z;
					if( Shape.ExplodedSeconds )
						offset += obj_iter->second.ExplosionMotion * Shape.ExplodedSeconds;
					Pos3D center = *this + offset;
					
					// If these two objects don't pass near each other, don't bother checking faces.
					// FIXME: Apply worldspace_explosion_motion to MotionVector.
					if( Math3D::MinimumDistance( &center, &MotionVector, other, &(other->MotionVector), dt ) > (obj_iter->second.MaxRadius + ship->Radius()) )
						continue;
					
					for( std::map<std::string,ModelArrays>::const_iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
					{
						array_inst.BecomeInstance( &(array_iter->second), false );
						
						if( Shape.ExplodedSeconds )
						{
							Pos3D draw_pos( this );
							
							// Convert explosion vectors to worldspace.
							Vec3D explosion_motion = obj_iter->second.ExplosionMotion * Shape.ExplodedSeconds;
							Vec3D explosion_rotation_axis = (Fwd * obj_iter->second.ExplosionRotationAxis.X) + (Up * obj_iter->second.ExplosionRotationAxis.Y) + (Right * obj_iter->second.ExplosionRotationAxis.Z);
							
							draw_pos.MoveAlong( &Fwd, explosion_motion.X );
							draw_pos.MoveAlong( &Up, explosion_motion.Y );
							draw_pos.MoveAlong( &Right, explosion_motion.Z );
							
							draw_pos.Fwd.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
							draw_pos.Up.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
							draw_pos.Right.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
							
							array_inst.MakeWorldSpace( &draw_pos );
						}
						else
							array_inst.MakeWorldSpace( this );
						
						for( int i = 0; i + 2 < array_inst.VertexCount; i += 3 )
						{
							double dist = Math3D::LineSegDistFromFace( other, &end, array_inst.WorldSpaceVertexArray + i*3, 3 );
							if( dist <= ship->Radius() )
							{
								if( this_object )
									*this_object = obj_iter->first;
								return true;
							}
						}
					}
				}
			}
			return false;
		}
		
		double dist = Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt );
		
		if( dist <= (Radius() + ship->Radius()) )
			return true;
	}
	
	else if( other->Type() == XWing::Object::ASTEROID )
	{
		Asteroid *asteroid = (Asteroid*) other;
		
		// Use face hit detection for capital ships.
		if( ComplexCollisionDetection() )
		{
			if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius * (1. + Shape.ExplodedSeconds * 0.25) + asteroid->Radius) )
				return false;
			
			Pos3D end( other );
			end += (other->MotionVector * dt) - (MotionVector * dt);
			ModelArrays array_inst;
			for( std::map<std::string,ModelObject>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
			{
				// Don't detect collisions with destroyed subsystems.
				std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find( obj_iter->first );
				if( (subsystem_iter != Subsystems.end()) && (subsystem_iter->second <= 0.) )
					continue;
				
				for( std::map<std::string,ModelArrays>::const_iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
				{
					array_inst.BecomeInstance( &(array_iter->second), false );
					
					if( Shape.ExplodedSeconds )
					{
						Pos3D draw_pos( this );
						
						// Convert explosion vectors to worldspace.
						Vec3D explosion_motion = obj_iter->second.ExplosionMotion * Shape.ExplodedSeconds;
						Vec3D explosion_rotation_axis = (Fwd * obj_iter->second.ExplosionRotationAxis.X) + (Up * obj_iter->second.ExplosionRotationAxis.Y) + (Right * obj_iter->second.ExplosionRotationAxis.Z);
						
						draw_pos.MoveAlong( &Fwd, explosion_motion.X );
						draw_pos.MoveAlong( &Up, explosion_motion.Y );
						draw_pos.MoveAlong( &Right, explosion_motion.Z );
						
						draw_pos.Fwd.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
						draw_pos.Up.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
						draw_pos.Right.RotateAround( &explosion_rotation_axis, Shape.ExplodedSeconds * obj_iter->second.ExplosionRotationRate );
						
						array_inst.MakeWorldSpace( &draw_pos );
					}
					else
						array_inst.MakeWorldSpace( this );
					
					for( int i = 0; i + 2 < array_inst.VertexCount; i += 3 )
					{
						double dist = Math3D::LineSegDistFromFace( other, &end, array_inst.WorldSpaceVertexArray + i*3, 3 );
						if( dist <= asteroid->Radius )
						{
							if( this_object )
								*this_object = obj_iter->first;
							return true;
						}
					}
				}
			}
			return false;
		}
		
		double dist = Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt );
		
		if( dist <= (Radius() + asteroid->Radius) )
			return true;
	}
	
	// Let the Death Star determine whether collisions with ships occur.
	else if( (other->Type() == XWing::Object::DEATH_STAR_BOX) || (other->Type() == XWing::Object::DEATH_STAR) || (other->Type() == XWing::Object::TURRET) )
		return other->WillCollide( this, dt, other_object, this_object );
	
	return false;
}


void Ship::Update( double dt )
{
	if( Health <= 0. )
	{
		// Dead ships stay on their old course.
		RollRate = 0.;
		PitchRate = 0.;
		YawRate = 0.;
	}
	else
	{
		// Shield recharge.
		if( HitClock.ElapsedSeconds() >= ShieldRechargeDelay() )
		{
			double recharge = ShieldRechargeRate() * dt;
			ShieldF += recharge;
			ShieldR += recharge;
		}
		
		// Shield position.
		if( ShieldPos == Ship::SHIELD_FRONT )
		{
			ShieldF += ShieldR;
			ShieldR = 0.;
		}
		else if( ShieldPos == Ship::SHIELD_REAR )
		{
			ShieldR += ShieldF;
			ShieldF = 0.;
		}
		
		// Keep shields legal.
		double max_shield = MaxShield();
		if( ShieldF > max_shield )
		{
			ShieldR += (ShieldF - max_shield);
			ShieldF = max_shield;
		}
		if( ShieldR > max_shield )
		{
			ShieldF += (ShieldR - max_shield);
			ShieldR = max_shield;
		}
		if( ShieldF > max_shield )
			ShieldF = max_shield;
	}
	
	GameObject::Update( dt );
	
	if( ShipType == TYPE_EXHAUST_PORT )
		Target = 0;
}


void Ship::Draw( void )
{
	if( Subsystems.size() )
	{
		// Build a list of all objects that don't have destroyed subsystems.
		std::list<std::string> objects;
		for( std::map<std::string,ModelObject>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
		{
			std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find(obj_iter->first);
			if( (subsystem_iter == Subsystems.end()) || (subsystem_iter->second > 0.) )
				objects.push_back( obj_iter->first );
		}
		
		// If all objects are valid to draw, use the standard draw routine.
		if( objects.size() == Shape.Objects.size() )
			Shape.DrawAt( this );
		// If we're avoiding drawing some objects due to subsystem damage, use special per-object draw routine.
		else
			Shape.DrawObjectsAt( &objects, this );
	}
	else
		Shape.DrawAt( this );
}


void Ship::DrawWireframe( void )
{
	// FIXME: Don't draw missing subsystems.
	Shape.DrawWireframeAt( this, Color(0.5,0.5,1,1) );
}
