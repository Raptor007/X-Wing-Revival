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


Ship::Ship( uint32_t id ) : GameObject( id, XWing::Object::SHIP )
{
	Team = XWing::Team::NONE;
	ShipType = TYPE_XWING;
	CanRespawn = false;
	
	SpecialUpdate = false;
	
	Health = 100.;
	ShieldF = 0.;
	ShieldR = 0.;
	ShieldPos = SHIELD_CENTER;
	
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
		else if( ShipType == TYPE_EXHAUST_PORT )
		{
			Shape.BecomeInstance( Raptor::Game->Res.GetModel("deathstar_exhaust_port.obj") );
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
		SelectedWeapon = Shot::TYPE_TURBO_LASER_GREEN;
	else if( ShipType == TYPE_EXHAUST_PORT )
		SelectedWeapon = 0;
	
	Ammo[ SelectedWeapon ] = -1;
	FiringClocks[ SelectedWeapon ].ElapsedSeconds();
	
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


void Ship::AddDamage( double front, double rear )
{
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
	
	SetHealth( Health - hull_damage );
	
	HitClock.Reset();
}


void Ship::Explode( double dt )
{
	if( ShipType == TYPE_EXHAUST_PORT )
		return;
	
	if( ShipType == TYPE_ISD2 )
		dt *= 0.01;
	
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
		return 100.;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 0.5;
	
	return 4.5;
}


double Ship::MaxSpeed( void ) const
{
	if( ShipType == TYPE_XWING )
		return 150.;
	else if( ShipType == TYPE_YWING )
		return 125.;
	else if( ShipType == TYPE_TIE_FIGHTER )
		return 180.;
	else if( ShipType == TYPE_ISD2 )
		return 10.;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 0.;
	
	return 100.;
}


double Ship::Acceleration( void ) const
{
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
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 0.;
	
	return 180.;
}


double Ship::MaxPitch( void ) const
{
	if( ShipType == TYPE_XWING )
		return 90.;
	else if( ShipType == TYPE_YWING )
		return 60.;
	else if( ShipType == TYPE_TIE_FIGHTER )
		return 120.;
	else if( ShipType == TYPE_ISD2 )
		return 0.5;
	else if( ShipType == TYPE_EXHAUST_PORT )
		return 0.;
	
	return 90.;
}


double Ship::MaxYaw( void ) const
{
	if( ShipType == TYPE_XWING )
		return 90.;
	else if( ShipType == TYPE_YWING )
		return 60.;
	else if( ShipType == TYPE_TIE_FIGHTER )
		return 120.;
	else if( ShipType == TYPE_ISD2 )
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
		return 1000.;
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
		return 1000.;
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


bool Ship::PlayersCanFly( void ) const
{
	if( ShipType == TYPE_ISD2 )
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
		
		shot->MoveAlong( &Fwd, fwd );
		shot->MoveAlong( &Up, up );
		shot->MoveAlong( &Right, right );
		
		shots[ weapon_index ] = shot;
		
		// Aim turrets at the intercept point.
		if( target && (shot->ShotType == Shot::TYPE_TURBO_LASER_GREEN) )
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


void Ship::AddToInitPacket( Packet *packet, int8_t precision )
{
	GameObject::AddToInitPacket( packet, precision );
	packet->AddUInt( Team );
	packet->AddUInt( ShipType );
	packet->AddString( Name );
}


void Ship::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, precision );
	Team = packet->NextUInt();
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


bool Ship::WillCollide( const GameObject *other, double dt ) const
{
	// Dead ships don't collide after a bit.
	if( (Health <= 0.) && (DeathClock.ElapsedSeconds() > 0.5) )
		return false;
	
	if( other->Type() == XWing::Object::SHOT )
	{
		// Ships can't shoot themselves.
		Shot *shot = (Shot*) other;
		if( shot->FiredFrom == ID )
			return false;
		
		double dist = Math3D::MinimumDistance( this, &(this->MotionVector), other, &(other->MotionVector), dt );
		
		if( dist <= (Radius() * 2.) )
			return true;
	}
	
	else if( other->Type() == XWing::Object::SHIP )
	{
		// Dead ships don't collide after a bit, and they never hit other dead ships.
		Ship *ship = (Ship*) other;
		if( (ship->Health <= 0.) && ((Health <= 0.) || (ship->DeathClock.ElapsedSeconds() > 0.5)) )
			return false;
		
		// Let capitol ships handle collisions.
		if( (ShipType != TYPE_ISD2) && (ship->ShipType == TYPE_ISD2) )
			return other->WillCollide( this, dt );
		
		// Don't let ships hit the Death Star exhaust port.
		if( (ShipType == TYPE_EXHAUST_PORT) || (ship->ShipType == TYPE_EXHAUST_PORT) )
			return false;
		
		double dist = Math3D::MinimumDistance( this, &(this->MotionVector), other, &(other->MotionVector), dt );
		
		if( dist <= (Radius() + ship->Radius()) )
			return true;
	}
	
	else if( other->Type() == XWing::Object::ASTEROID )
	{
		double dist = Math3D::MinimumDistance( this, &(this->MotionVector), other, &(other->MotionVector), dt );
		
		Asteroid *asteroid = (Asteroid*) other;
		if( dist <= (Radius() + asteroid->Radius) )
			return true;
	}
	
	// Let the Death Star determine whether collisions with ships occur.
	else if( (other->Type() == XWing::Object::DEATH_STAR_BOX) || (other->Type() == XWing::Object::DEATH_STAR) || (other->Type() == XWing::Object::TURRET) )
		return other->WillCollide( this, dt );
	
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
	Shape.DrawAt( this );
}
