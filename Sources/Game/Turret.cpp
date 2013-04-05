/*
 *  Turret.cpp
 */

#include "Turret.h"

#include <cmath>
#include "XWingDefs.h"
#include "RaptorGame.h"
#include "Num.h"
#include "Math3D.h"
#include "Ship.h"
#include "Shot.h"


Turret::Turret( uint32_t id ) : GameObject( id, XWing::Object::TURRET )
{
	ParentID = 0;
	Team = XWing::Team::NONE;
	
	MinGunPitch = 0.;
	MaxGunPitch = 80.;
	Health = 100.;
	Weapon = Shot::TYPE_TURBO_LASER_GREEN;
	FiringMode = 2;
	SingleShotDelay = 0.5;
	
	BodyShape = NULL;
	GunShape = NULL;
	
	GunPitch = 0.;
	GunPitchRate = 0.;
	
	Firing = false;
	WeaponIndex = 0;
	Target = 0;
}


Turret::~Turret()
{
}


void Turret::ClientInit( void )
{
	BodyShape = Raptor::Game->Res.GetModel("turret_body.obj");
	GunShape = Raptor::Game->Res.GetModel("turret_gun.obj");
}


void Turret::Attach( GameObject *parent, Vec3D *offset )
{
	if( offset )
		Offset.Copy( offset );
	else
		Offset.Set( 0., 0., 0. );
	
	if( parent )
	{
		ParentID = parent->ID;
		Copy( parent );
		GunPitch = 0.;
		MotionVector.Copy( &(parent->MotionVector) );
	}
	else
		ParentID = 0;
}


void Turret::SetHealth( double health )
{
	Health = health;
}


void Turret::AddDamage( double damage )
{
	SetHealth( Health - damage );
}


void Turret::PitchGun( double degrees )
{
	// Don't apply pitch to the position, but keep track for the guns.
	GunPitch += degrees;
	if( GunPitch < MinGunPitch )
		GunPitch = MinGunPitch;
	else if( GunPitch > MaxGunPitch )
		GunPitch = MaxGunPitch;
}

void Turret::SetPitch( double pitch )
{
	if( pitch > 1. )
		pitch = 1.;
	else if( pitch < -1. )
		pitch = -1.;
	
	GunPitchRate = pitch * 45.;
}

void Turret::SetYaw( double yaw )
{
	if( yaw > 1. )
		yaw = 1.;
	else if( yaw < -1. )
		yaw = -1.;
	
	YawRate = yaw * 45.;
}


Pos3D Turret::GunPos( void )
{
	GameObject gun;
	gun.Copy( this );
	gun.MoveAlong( &(this->Up), 0.022 * 150. );
	gun.MoveAlong( &(this->Fwd), 0.022 * 50. );
	gun.Pitch( GunPitch );
	return gun;
}


double Turret::MaxHealth( void )
{
	return 200.;
}


std::map<int,Shot*> Turret::NextShots( GameObject *target )
{
	std::map<int,Shot*> shots;
	
	Pos3D gun = GunPos();
	
	for( int num = 0; num < FiringMode; num ++ )
	{
		int weapon_index = (WeaponIndex + num) % 2;
		
		Shot *shot = new Shot();
		shot->Copy( &gun );
		shot->FiredFrom = ParentID ? ParentID : 0;
		shot->PlayerID = PlayerID;
		shot->ShotType = Weapon;
		shot->MotionVector.Set( shot->Fwd.X, shot->Fwd.Y, shot->Fwd.Z );
		shot->MotionVector.ScaleTo( shot->Speed() );
		
		double fwd = 0., up = 0., right = 0.;
		
		if( weapon_index == 0 )
			right = 2.2;
		else
			right = -2.2;
		
		shot->MoveAlong( &Fwd, fwd );
		shot->MoveAlong( &Up, up );
		shot->MoveAlong( &Right, right );
		
		shots[ weapon_index ] = shot;
	}
	
	return shots;
}


void Turret::JustFired( void )
{
	FiringClock.Reset();
	
	WeaponIndex ++;
	if( WeaponIndex >= 2 )
		WeaponIndex = 0;
}


double Turret::ShotDelay( void ) const
{
	return SingleShotDelay * FiringMode;
}


bool Turret::PlayerShouldUpdateServer( void ) const
{
	return false;
}

bool Turret::ServerShouldUpdatePlayer( void ) const
{
	return true;
}

bool Turret::ServerShouldUpdateOthers( void ) const
{
	return true;
}

bool Turret::CanCollideWithOwnType( void ) const
{
	return false;
}

bool Turret::CanCollideWithOtherTypes( void ) const
{
	return true;
}

bool Turret::IsMoving( void ) const
{
	if( ! ParentID )
		return false;
	
	return GameObject::IsMoving();
}


void Turret::AddToInitPacket( Packet *packet, int8_t precision )
{
	GameObject::AddToInitPacket( packet, precision );
	packet->AddUInt( Team );
	packet->AddUInt( ParentID );
	packet->AddFloat( GunPitch );
}


void Turret::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, precision );
	Team = packet->NextUInt();
	ParentID = packet->NextUInt();
	GunPitch = packet->NextFloat();
}


void Turret::AddToUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacketFromServer( packet, -127 );
	
	// Compress GunPitch in range (-180,180) to int16.
	double unit_gun_pitch = GunPitch / 180.;
	while( unit_gun_pitch > 1. )
		unit_gun_pitch -= 2.;
	while( unit_gun_pitch < -1. )
		unit_gun_pitch += 2.;
	packet->AddShort( Num::UnitFloatTo16(unit_gun_pitch) );
	
	packet->AddFloat( Health );
	packet->AddUInt( Target );
}


void Turret::ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::ReadFromUpdatePacketFromServer( packet, -127 );
	
	// Extact GunPitch in range (-180,180) from int16.
	int16_t compressed_gun_pitch = packet->NextShort();
	GunPitch = Num::UnitFloatFrom16(compressed_gun_pitch) * 180.;
	
	SetHealth( packet->NextFloat() );
	Target = packet->NextUInt();
}


void Turret::AddToUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacketFromClient( packet, precision );
	packet->AddDouble( GunPitch );
	packet->AddUChar( Firing );
	packet->AddUInt( Target );
}


void Turret::ReadFromUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	GameObject::ReadFromUpdatePacketFromClient( packet, precision );
	GunPitch = packet->NextDouble();
	Firing = packet->NextUChar();
	Target = packet->NextUInt();
}


bool Turret::WillCollide( const GameObject *other, double dt ) const
{
	// Can't collide with the ship we're attached to.
	if( other->ID == ParentID )
		return false;
	
	// Dead turrets don't collide.
	if( Health <= 0. )
		return false;
	
	if( other->Type() == XWing::Object::SHOT )
	{
		// Turrets can't shoot themselves, and neither can other turrets on this ship.
		Shot *shot = (Shot*) other;
		if( (! shot->FiredFrom) || (shot->FiredFrom == ID) || (shot->FiredFrom == ParentID) )
			return false;
		
		double dist = Math3D::MinimumDistance( this, &(this->MotionVector), other, &(other->MotionVector), dt );
		
		if( dist <= 10. )
			return true;
	}
	
	else if( other->Type() == XWing::Object::SHIP )
	{
		// Dead ships don't collide after a bit, and they never hit other dead ships.
		Ship *ship = (Ship*) other;
		if( (ship->Health <= 0.) && ((Health <= 0.) || (ship->DeathClock.ElapsedSeconds() > 0.5)) )
			return false;
		
		// Don't let turrets hit capitol ships.
		if( ship->ShipType == Ship::TYPE_ISD2 )
			return false;
		
		double dist = Math3D::MinimumDistance( this, &(this->MotionVector), other, &(other->MotionVector), dt );
		
		if( dist <= (ship->Radius() + 5.) )
			return true;
	}
	
	// Let the Death Star determine whether collisions with ships occur.
	else if( other->Type() == XWing::Object::DEATH_STAR )
		return other->WillCollide( this, dt );
	
	return false;
}


void Turret::Update( double dt )
{
	// FIXME: This should not use Raptor::Game because it might be on the server!
	GameObject *parent = Raptor::Game->Data.GetObject( ParentID );
	if( parent )
	{
		if( parent->Type() == XWing::Object::SHIP )
		{
			Ship *parent_ship = (Ship*) parent;
			if( parent_ship->Health <= 0. )
				parent = NULL;
		}
	}
	if( ParentID && (! parent) )
	{
		ParentID = 0;
		Health = 0.;
	}
	
	if( Health <= 0. )
	{
		// Dead turrets don't turn.
		RollRate = 0.;
		PitchRate = 0.;
		YawRate = 0.;
		GunPitchRate = 0.;
	}
	
	if( GunPitchRate )
		PitchGun( GunPitchRate * dt );
	
	GameObject::Update( dt );
	
	if( parent )
	{
		X = parent->X;
		Y = parent->Y;
		Z = parent->Z;
		MoveAlong( &(parent->Fwd), Offset.Z );
		MoveAlong( &(parent->Up), Offset.Y );
		MoveAlong( &(parent->Right), Offset.X );
		MotionVector.Copy( &(parent->MotionVector) );
	}
}


void Turret::Draw( void )
{
	if( Health > 0. )
	{
		if( BodyShape )
			BodyShape->DrawAt( this, 0.022 );
		
		if( GunShape )
		{
			Pos3D gun = GunPos();
			GunShape->DrawAt( &gun, 0.022 );
		}
	}
}
