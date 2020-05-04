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
	RelativeUp.Set( 0., 1., 0. );
	ParentControl = false;
	Team = XWing::Team::NONE;
	
	MinGunPitch = 0.;
	MaxGunPitch = 80.;
	Health = 100.;
	Weapon = Shot::TYPE_TURBO_LASER_GREEN;
	FiringMode = 2;
	SingleShotDelay = 0.5;
	MaxFiringDist = 1500.;
	AimAhead = 1.f;
	TargetDir.Copy(&Fwd);
	TargetArc = 360.;
	SafetyDistance = 0.;
	
	Visible = true;
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
	if( Visible )
	{
		BodyShape = Raptor::Game->Res.GetModel("turret_body.obj");
		GunShape = Raptor::Game->Res.GetModel("turret_gun.obj");
	}
}


void Turret::Attach( const GameObject *parent, const Vec3D *offset, const Vec3D *relative_up, const Vec3D *relative_fwd, bool parent_control )
{
	if( offset )
		Offset.Copy( offset );
	if( relative_up )
	{
		RelativeUp.Copy( relative_up );
		RelativeUp.ScaleTo( 1. );
	}
	if( relative_fwd )
	{
		RelativeFwd.Copy( relative_fwd );
		RelativeFwd.ScaleTo( 1. );
	}
	
	ParentControl = parent_control;
	
	if( parent )
	{
		ParentID = parent->ID;
		UpdatePos( parent );
	}
	else
		ParentID = 0;
}


void Turret::UpdatePos( const GameObject *parent )
{
	X = parent->X;
	Y = parent->Y;
	Z = parent->Z;
	MoveAlong( &(parent->Fwd), Offset.X );
	MoveAlong( &(parent->Up), Offset.Y );
	MoveAlong( &(parent->Right), Offset.Z );
	MotionVector.Copy( &(parent->MotionVector) );
	Up = (parent->Fwd * RelativeUp.X) + (parent->Up * RelativeUp.Y) + (parent->Right * RelativeUp.Z);
	TargetDir = (parent->Fwd * RelativeFwd.X) + (parent->Up * RelativeFwd.Y) + (parent->Right * RelativeFwd.Z);
	FixVectorsKeepUp();
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
	if( Visible )
	{
		gun.MoveAlong( &(this->Up), 0.022 * 175. );
		gun.MoveAlong( &(this->Fwd), 0.022 * 50. );
	}
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
		
		if( Visible || (FiringMode > 1) )
			right = weapon_index ? -2.2 : 2.2;
		
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
	return Visible;
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
	if( ParentID )
	{
		packet->AddFloat( Offset.X );
		packet->AddFloat( Offset.Y );
		packet->AddFloat( Offset.Z );
		packet->AddFloat( RelativeUp.X );
		packet->AddFloat( RelativeUp.Y );
		packet->AddFloat( RelativeUp.Z );
	}
	
	// Compress GunPitch in range (-180,180) to int16.
	double unit_gun_pitch = GunPitch / 180.;
	while( unit_gun_pitch > 1. )
		unit_gun_pitch -= 2.;
	while( unit_gun_pitch < -1. )
		unit_gun_pitch += 2.;
	packet->AddShort( Num::UnitFloatTo16(unit_gun_pitch) );
	
	packet->AddUChar( Visible ? 1 : 0 );
}


void Turret::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, precision );
	Team = packet->NextUInt();
	ParentID = packet->NextUInt();
	if( ParentID )
	{
		Offset.X = packet->NextFloat();
		Offset.Y = packet->NextFloat();
		Offset.Z = packet->NextFloat();
		RelativeUp.X = packet->NextFloat();
		RelativeUp.Y = packet->NextFloat();
		RelativeUp.Z = packet->NextFloat();
	}
	
	// Extact GunPitch in range (-180,180) from int16.
	int16_t compressed_gun_pitch = packet->NextShort();
	GunPitch = Num::UnitFloatFrom16(compressed_gun_pitch) * 180.;
	
	Visible = packet->NextUChar();
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
	
	if( ParentID && Data )
	{
		GameObject *parent = Data->GetObject( ParentID );
		if( parent )
			UpdatePos( parent );
	}
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


bool Turret::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object ) const
{
	// Can't collide with the ship we're attached to.
	if( other->ID == ParentID )
		return false;
	
	// Hidden embedded turrets don't collide.
	if( ! Visible )
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
		if( ship->ComplexCollisionDetection() )
			return false;
		
		double dist = Math3D::MinimumDistance( this, &(this->MotionVector), other, &(other->MotionVector), dt );
		
		if( dist <= (ship->Radius() + 5.) )
			return true;
	}
	
	/*
	// Let the Death Star determine whether collisions with turrets occur.
	// This could be used to scrape off attached turrets when ships fly too close.
	else if( other->Type() == XWing::Object::DEATH_STAR )
		return other->WillCollide( this, dt, other_object, this_object );
	*/
	
	return false;
}


void Turret::Update( double dt )
{
	GameObject *parent = Data ? Data->GetObject( ParentID ) : NULL;
	if( parent )
	{
		if( parent->Type() == XWing::Object::SHIP )
		{
			Ship *parent_ship = (Ship*) parent;
			Team = parent_ship->Team;
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
		UpdatePos( parent );
}


void Turret::Draw( void )
{
	if( (Health > 0.) && Visible )
	{
		bool change_shader = (! ParentID) && (Raptor::Game->Cfg.SettingAsInt("g_shader_light_quality") >= 2) && Raptor::Game->ShaderMgr.Active();
		Shader *prev_shader = Raptor::Game->ShaderMgr.Selected;
		if( change_shader )
			Raptor::Game->ShaderMgr.SelectAndCopyVars( Raptor::Game->Res.GetShader("deathstar") );
		
		if( BodyShape )
			BodyShape->DrawAt( this, 0.022 );
		
		if( GunShape )
		{
			Pos3D gun = GunPos();
			GunShape->DrawAt( &gun, 0.022 );
		}
		
		if( change_shader )
			Raptor::Game->ShaderMgr.Select( prev_shader );
	}
}
