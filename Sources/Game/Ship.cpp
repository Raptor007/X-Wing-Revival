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
	Clear();
}


Ship::Ship( const ShipClass *ship_class ) : GameObject( 0, XWing::Object::SHIP )
{
	Clear();
	
	Class = ship_class;
	Reset();
	
	if( Class && Class->CollisionModel.length() )
	{
		Shape.LoadOBJ( std::string("Models/") + Class->CollisionModel, false );
		Shape.ScaleBy( Class->ModelScale );
		Shape.GetMaxRadius();
	}
}


Ship::~Ship()
{
}


void Ship::Clear( void )
{
	Class = NULL;
	Team = XWing::Team::NONE;
	CanRespawn = false;
	Group = 0;
	IsMissionObjective = false;
	
	SpecialUpdate = false;
	
	Health = 100.;
	CollisionPotential = 0.;
	ShieldF = 0.;
	ShieldR = 0.;
	ShieldPos = SHIELD_CENTER;
	
	Firing = false;
	SelectedWeapon = 0;
	FiredThisFrame = 0;
	FiringMode = 1;
	WeaponIndex = 0;
	
	Target = 0;
	TargetLock = 0.f;
}


void Ship::ClientInit( void )
{
	if( Class && Class->ExternalModel.length() )
	{
		Shape.BecomeInstance( Raptor::Game->Res.GetModel( Class->ExternalModel ) );
		Shape.ScaleBy( Class->ModelScale );
	}
}


bool Ship::SetClass( uint32_t ship_class_id )
{
	GameObject *obj = Raptor::Game->Data.GetObject( ship_class_id );
	if( obj && (obj->Type() == XWing::Object::SHIP_CLASS) )
	{
		SetClass( (const ShipClass*) obj );
		return true;
	}
	return false;
}


void Ship::SetClass( const ShipClass *ship_class )
{
	Class = ship_class;
	
	Reset();
}


void Ship::Reset( void )
{
	Health = MaxHealth();
	ShieldF = MaxShield();
	ShieldR = ShieldF;
	ShieldPos = SHIELD_CENTER;
	Subsystems.clear();
	CollisionPotential = 1000.;
	
	Target = 0;
	TargetLock = 0.f;
	Firing = false;
	WeaponIndex = 0;
	FiringMode = 1;
	Ammo.clear();
	SelectedWeapon = 0;
	
	if( Class )
	{
		Subsystems = Class->Subsystems;
		CollisionPotential = Class->CollisionDamage;
		Ammo = Class->Ammo;
		SelectedWeapon = Class->Weapons.size() ? Class->Weapons.begin()->first : 0;
		for( std::map< uint32_t, std::vector<Pos3D> >::const_iterator weapon_iter = Class->Weapons.begin(); weapon_iter != Class->Weapons.end(); weapon_iter ++ )
			FiringClocks[ weapon_iter->first ].Reset();
	}
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


void Ship::KnockCockpit( const Vec3D *dir, double force )
{
	CockpitOffset.X += Fwd.Dot(dir) * force;
	CockpitOffset.Y += Up.Dot(dir) * force;
	CockpitOffset.Z += Right.Dot(dir) * force;
}


void Ship::SetRoll( double roll, double dt )
{
	if( roll > 1. )
		roll = 1.;
	else if( roll < -1. )
		roll = -1.;
	
	double max_change = dt * MaxRollChange();
	double desired = roll * MaxRoll();
	
	if( fabs( desired - RollRate ) > max_change )
		RollRate += max_change * Num::Sign( desired - RollRate );
	else
		RollRate = desired;
}


void Ship::SetPitch( double pitch, double dt )
{
	if( pitch > 1. )
		pitch = 1.;
	else if( pitch < -1. )
		pitch = -1.;
	
	double max_change = dt * MaxPitchChange();
	double desired = pitch * MaxPitch();
	
	if( fabs( desired - PitchRate ) > max_change )
		PitchRate += max_change * Num::Sign( desired - PitchRate );
	else
		PitchRate = desired;
}


void Ship::SetYaw( double yaw, double dt )
{
	if( yaw > 1. )
		yaw = 1.;
	else if( yaw < -1. )
		yaw = -1.;
	
	double max_change = dt * MaxYawChange();
	double desired = yaw * MaxYaw();
	
	if( fabs( desired - YawRate ) > max_change )
		YawRate += max_change * Num::Sign( desired - YawRate );
	else
		YawRate = desired;
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
	
	CockpitOffset.X += (new_speed - old_speed) * 3. * dt;
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
	if( Class )
		return Class->Radius;
	
	return 4.5;
}


double Ship::MaxSpeed( void ) const
{
	if( Class )
		return Class->MaxSpeed;
	
	return 100.;
}


double Ship::Acceleration( void ) const
{
	if( Class )
		return Class->Acceleration;
	
	return MaxSpeed() / 2.;
}


double Ship::MaxRoll( void ) const
{
	if( Class )
		return Class->MaxRoll;
	
	return 180.;
}


double Ship::MaxPitch( void ) const
{
	if( Class )
		return Class->MaxPitch;
	
	return 100.;
}


double Ship::MaxYaw( void ) const
{
	if( Class )
		return Class->MaxYaw;
	
	return 80.;
}


double Ship::MaxRollChange( void ) const
{
	return MaxRoll() * 8.; // FIXME
}


double Ship::MaxPitchChange( void ) const
{
	return MaxYaw() * 8.; // FIXME
}


double Ship::MaxYawChange( void ) const
{
	return MaxYaw() * 8.; // FIXME
}


double Ship::MaxHealth( void ) const
{
	if( Class )
		return Class->MaxHealth;
	
	return 100.;
}


double Ship::MaxShield( void ) const
{
	if( Class )
		return Class->MaxShield;
	
	return 0.;
}


double Ship::ShieldRechargeDelay( void ) const
{
	if( Class )
		return Class->ShieldRechargeDelay;
	
	return 5.;
}


double Ship::ExplosionRate( void ) const
{
	if( Class )
		return Class->ExplosionRate;
	
	return 1.;
}


double Ship::ShieldRechargeRate( void ) const
{
	if( Class )
		return Class->ShieldRechargeRate;
	
	return 0.;
}


double Ship::PiecesDangerousTime( void ) const
{
	if( ComplexCollisionDetection() )
		return 5.;
	
	return 0.5;
}


int Ship::WeaponCount( int weapon_type ) const
{
	if( Class )
	{
		std::map< uint32_t, std::vector<Pos3D> >::const_iterator weapon_iter = Class->Weapons.find( weapon_type );
		return (weapon_iter != Class->Weapons.end()) ? weapon_iter->second.size() : 0;
	}
	
	return 0;
}


uint8_t Ship::Category( void ) const
{
	if( Class )
		return Class->Category;
	
	return ShipClass::CATEGORY_TARGET;
}


bool Ship::PlayersCanFly( void ) const
{
	if( Class )
		return Class->PlayersCanFly();
	
	return false;
}


Pos3D Ship::HeadPos( void ) const
{
	Pos3D head( this );
	if( Class )
	{
		head.MoveAlong( &Fwd,   Class->CockpitPos.X );
		head.MoveAlong( &Up,    Class->CockpitPos.Y );
		head.MoveAlong( &Right, Class->CockpitPos.Z );
	}
	return head;
}


double Ship::Exploded( void ) const
{
	if( Health <= 0. )
		return DeathClock.ElapsedSeconds() * ExplosionRate();
	
	return 0.;
}


const char *Ship::FlybySound( double speed ) const
{
	const char *sound = NULL;
	if( Class )
	{
		for( std::map< double, std::string >::const_iterator flyby_iter = Class->FlybySounds.begin(); flyby_iter != Class->FlybySounds.end(); flyby_iter ++ )
		{
			if( speed >= flyby_iter->first )
				sound = flyby_iter->second.c_str();
		}
	}
	return sound;
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
		
		if( Class )
		{
			std::map< uint32_t, std::vector<Pos3D> >::const_iterator weapon_iter = Class->Weapons.find( SelectedWeapon );
			if( (weapon_iter != Class->Weapons.end()) && weapon_iter->second.size() )
			{
				size_t weapon_count = weapon_iter->second.size();
				if( num && (FiringMode > 1) && (FiringMode < weapon_count) )
					weapon_index += weapon_count - FiringMode - 1;
				
				weapon_index %= weapon_iter->second.size();
				
				fwd   = weapon_iter->second.at( weapon_index ).X;
				up    = weapon_iter->second.at( weapon_index ).Y;
				right = weapon_iter->second.at( weapon_index ).Z;
			}
		}
		
		shot->MoveAlong( &Fwd, fwd );
		shot->MoveAlong( &Up, up );
		shot->MoveAlong( &Right, right );
		
		shots[ weapon_index ] = shot;
		
		// For ships that could be either team, make Empire lasers green.
		if( Team == XWing::Team::EMPIRE )
		{
			if( shot->ShotType == Shot::TYPE_LASER_RED )
				shot->ShotType = Shot::TYPE_LASER_GREEN;
			else if( shot->ShotType == Shot::TYPE_TURBO_LASER_RED )
				shot->ShotType = Shot::TYPE_TURBO_LASER_GREEN;
		}
		
		// If we had a lock for a seeking weapon, give the shot its target.
		if( ((shot->ShotType == Shot::TYPE_TORPEDO) || (shot->ShotType == Shot::TYPE_MISSILE)) && target && (TargetLock >= 1.) )
			shot->Seeking = target->ID;
		
		// Treat turbolasers as omnidirectional turrets.  Aim at the intercept point.
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
	FiringMode = WeaponCount( SelectedWeapon );
	std::map<int,Shot*> shots = NextShots( target );
	FiringMode = firing_mode;
	
	return shots;
}


void Ship::JustFired( void )
{
	JustFired( SelectedWeapon, FiringMode );
}


void Ship::JustFired( uint32_t weapon, uint8_t mode )
{
	FiringClocks[ weapon ].Reset();
	
	if( Ammo[ weapon ] > 0 )
	{
		if( Ammo[ weapon ] > mode )
			Ammo[ weapon ] -= mode;
		else
			Ammo[ weapon ] = 0;
	}
	
	if( (SelectedWeapon == weapon) && ! FiredThisFrame )
	{
		WeaponIndex ++;
		if( WeaponIndex >= WeaponCount(weapon) )
			WeaponIndex = 0;
	}
	
	FiredThisFrame ++;
}


bool Ship::NextWeapon( void )
{
	uint32_t prev = SelectedWeapon;
	
	if( Class )
	{
		std::map< uint32_t, int8_t >::const_iterator ammo_iter = Class->Ammo.find( SelectedWeapon );
		if( ammo_iter != Class->Ammo.end() )
			ammo_iter ++;
		if( ammo_iter == Class->Ammo.end() )
			ammo_iter = Class->Ammo.begin();
		if( ammo_iter != Class->Ammo.end() )
			SelectedWeapon = ammo_iter->first;
	}
	
	FiringMode = 1;
	WeaponIndex = 0;
	
	return (SelectedWeapon != prev);
}


bool Ship::NextFiringMode( void )
{
	uint8_t prev = FiringMode;
	
	uint8_t count = WeaponCount(SelectedWeapon);
	if( FiringMode < count )
	{
		for( uint8_t next_mode = FiringMode + 1; next_mode <= count; next_mode ++ )
		{
			if( count % next_mode == 0 )
			{
				FiringMode = next_mode;
				break;
			}
		}
	}
	else
		FiringMode = 1;
	
	if( (Ammo[ SelectedWeapon ] >= 0) && (Ammo[ SelectedWeapon ] < FiringMode) )
		FiringMode = Ammo[ SelectedWeapon ];
	
	return (FiringMode != prev);
}


double Ship::ShotDelay( void ) const
{
	if( Class )
	{
		std::map<uint32_t,double>::const_iterator firetime_iter = Class->FireTime.find( SelectedWeapon );
		if( firetime_iter != Class->FireTime.end() )
			return firetime_iter->second * FiringMode;
	}
	
	if( (SelectedWeapon == Shot::TYPE_TORPEDO) || (SelectedWeapon == Shot::TYPE_MISSILE) )
		return 1. * FiringMode;
	
	return 0.25 * FiringMode;
}


float Ship::LockingOn( const GameObject *target ) const
{
	if( ! target )
		return 0.f;
	
	if( (SelectedWeapon != Shot::TYPE_TORPEDO) && (SelectedWeapon != Shot::TYPE_MISSILE) )
		return 0.f;
	std::map<uint32_t,int8_t>::const_iterator ammo_iter = Ammo.find( SelectedWeapon );
	if( ammo_iter == Ammo.end() )
		return 0.f;
	if( ammo_iter->second == 0 )
		return 0.f;
	
	Vec3D vec_to_target( target->X - X, target->Y - Y, target->Z - Z );
	double dist_to_target = vec_to_target.Length();
	
	if( dist_to_target > 3000. )
		return 0.f;
	
	vec_to_target.ScaleTo( 1. );
	double t_dot_fwd = vec_to_target.Dot( &Fwd );
	
	double required_dot = 0.9;
	if( target->Type() == XWing::Object::SHIP )
	{
		Ship *t = (Ship*) target;
		if( t->Health <= 0. )
			return 0.f;
		required_dot = std::min<double>( 0.95, dist_to_target / (20. * t->Radius()) );
	}
	
	if( t_dot_fwd >= required_dot )
		// Lock on faster when closer to the target.
		return 300.f / std::max<float>( 1000.f, dist_to_target );
	
	return 0.f;
}


void Ship::UpdateTarget( const GameObject *target, double dt )
{
	if( Health <= 0. )
	{
		TargetLock = 0.f;
		return;
	}
	
	if( target && (target->ID != Target) )
	{
		Target = target->ID;
		
		// Lose lock progress when changing targets.
		TargetLock = 0.f;
	}
	else if( ! target )
		Target = 0;
	
	float locking_on = LockingOn( target );
	if( locking_on )
		TargetLock += locking_on * dt;
	else
		TargetLock -= dt / 2.;
	
	if( TargetLock > 2.f )
		TargetLock = 2.f;
	else if( TargetLock < 0.f )
		TargetLock = 0.f;
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
	if( Class )
		return Class->CollisionModel.length();
	
	return false;
}


void Ship::AddToInitPacket( Packet *packet, int8_t precision )
{
	GameObject::AddToInitPacket( packet, precision );
	packet->AddUInt( Team );
	packet->AddUChar( Group );
	packet->AddUInt( Class ? Class->ID : 0 );
	packet->AddString( Name );
}


void Ship::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, precision );
	Team = packet->NextUInt();
	Group = packet->NextUChar();
	SetClass( packet->NextUInt() );
	Name = packet->NextString();
}


void Ship::AddToUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacketFromServer( packet, precision );
	packet->AddFloat( Health );
	packet->AddUChar( ShieldPos );
	packet->AddUInt( Target );
	packet->AddChar( Num::UnitFloatTo8( (Target && LockingOn(Data->GetObject(Target))) ? (TargetLock / 2.f) : 0.f ) );
	
	SpecialUpdate = false;
}


void Ship::ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::ReadFromUpdatePacketFromServer( packet, precision );
	SetHealth( packet->NextFloat() );
	SetShieldPos( packet->NextUChar() );
	Target = packet->NextUInt();
	TargetLock = Num::UnitFloatFrom8( packet->NextChar() ) * 2.f;
}


void Ship::AddToUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacketFromClient( packet, precision );
	packet->AddUChar( Firing );
	packet->AddUChar( ShieldPos );
	packet->AddUInt( Target );
	packet->AddUInt( SelectedWeapon );
	packet->AddUChar( FiringMode );
	packet->AddChar( Num::UnitFloatTo8( TargetLock / 2.f ) );
}


void Ship::ReadFromUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	// Dirty hack so we don't get client position data for a dead ship.
	if( Health <= 0. )
	{
		Ship temp_ship;
		temp_ship.Health = 100.;
		temp_ship.Class = Class;
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
	TargetLock = Num::UnitFloatFrom8( packet->NextChar() ) * 2.f;
	
	if( SelectedWeapon != prev_selected_weapon )
		WeaponIndex = 0;
}


bool Ship::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object ) const
{
	// Dead ships don't collide after a bit (except capital ship chunks).
	if( (Health <= 0.) && (DeathClock.ElapsedSeconds() > PiecesDangerousTime()) )
		return false;
	
	double exploded = Exploded();
	
	if( other->Type() == XWing::Object::SHOT )
	{
		Shot *shot = (Shot*) other;
		
		// Ships can't shoot themselves.
		if( shot->FiredFrom == ID )
			return false;
		
		// Use face hit detection for capital ships.
		if( ComplexCollisionDetection() )
		{
			if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius * (1. + exploded * 0.25)) )
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
				if( exploded )
					offset += obj_iter->second.GetExplosionMotion( ID ) * exploded;
				Pos3D center = *this + offset;
				
				// If these two objects don't pass near each other, don't bother checking faces.
				// FIXME: Apply worldspace_explosion_motion to MotionVector.
				if( Math3D::MinimumDistance( &center, &MotionVector, other, &(other->MotionVector), dt ) > obj_iter->second.MaxRadius )
					continue;
				
				for( std::map<std::string,ModelArrays>::const_iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
				{
					array_inst.BecomeInstance( &(array_iter->second), false );
					
					if( exploded )
					{
						Pos3D draw_pos( this );
						
						// Convert explosion vectors to worldspace.
						Vec3D explosion_motion = obj_iter->second.GetExplosionMotion( ID ) * exploded;
						Vec3D modelspace_rotation_axis = obj_iter->second.GetExplosionRotationAxis( ID );
						Vec3D explosion_rotation_axis = (Fwd * modelspace_rotation_axis.X) + (Up * modelspace_rotation_axis.Y) + (Right * modelspace_rotation_axis.Z);
						
						draw_pos.MoveAlong( &Fwd, explosion_motion.X );
						draw_pos.MoveAlong( &Up, explosion_motion.Y );
						draw_pos.MoveAlong( &Right, explosion_motion.Z );
						
						double explosion_rotation_rate = obj_iter->second.GetExplosionRotationRate( ID );
						draw_pos.Fwd.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
						draw_pos.Up.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
						draw_pos.Right.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
						
						array_inst.MakeWorldSpace( &draw_pos );
					}
					else
						array_inst.MakeWorldSpace( this );
					
					for( size_t i = 0; i + 2 < array_inst.VertexCount; i += 3 )
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
		if( (Category() == ShipClass::CATEGORY_TARGET) || (ship->Category() == ShipClass::CATEGORY_TARGET) )
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
				
				double other_exploded = ship->Exploded();
				
				// If they're nowhere near each other, don't bother getting fancy.
				if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius * (1. + exploded * 0.25) + ship->Shape.MaxRadius * (1. + other_exploded * 0.25)) )
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
					
					if( exploded )
					{
						Vec3D modelspace_explosion_motion = obj_iter->second.GetExplosionMotion( ID );
						Vec3D worldspace_explosion_motion = Fwd * modelspace_explosion_motion.X + Up * modelspace_explosion_motion.Y + Right * modelspace_explosion_motion.Z;
						center += worldspace_explosion_motion * exploded;
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
						
						if( other_exploded )
						{
							Vec3D modelspace_explosion_motion = other_obj_iter->second.GetExplosionMotion( ID );
							Vec3D worldspace_explosion_motion = other->Fwd * modelspace_explosion_motion.X + other->Up * modelspace_explosion_motion.Y + other->Right * modelspace_explosion_motion.Z;
							other_center += worldspace_explosion_motion * other_exploded;
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
							
							if( exploded )
							{
								Pos3D draw_pos( this );
								
								// Convert explosion vectors to worldspace.
								Vec3D explosion_motion = obj_iter->second.GetExplosionMotion( ID ) * exploded;
								Vec3D modelspace_rotation_axis = obj_iter->second.GetExplosionRotationAxis( ID );
								Vec3D explosion_rotation_axis = (Fwd * modelspace_rotation_axis.X) + (Up * modelspace_rotation_axis.Y) + (Right * modelspace_rotation_axis.Z);
								
								draw_pos.MoveAlong( &Fwd, explosion_motion.X );
								draw_pos.MoveAlong( &Up, explosion_motion.Y );
								draw_pos.MoveAlong( &Right, explosion_motion.Z );
								
								double explosion_rotation_rate = obj_iter->second.GetExplosionRotationRate( ID );
								draw_pos.Fwd.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
								draw_pos.Up.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
								draw_pos.Right.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
								
								array_inst.MakeWorldSpace( &draw_pos );
							}
							else
								array_inst.MakeWorldSpace( this );
							
							GLdouble *worldspace = array_inst.WorldSpaceVertexArray;
							size_t vertex_count = array_inst.VertexCount;
							
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
								
								if( other_exploded )
								{
									Vec3D modelspace_explosion_motion = (*other_obj_iter)->GetExplosionMotion( other->ID );
									Vec3D worldspace_explosion_motion = other->Fwd * modelspace_explosion_motion.X + other->Up * modelspace_explosion_motion.Y + other->Right * modelspace_explosion_motion.Z;
									other_start += worldspace_explosion_motion * other_exploded;
									other_end += worldspace_explosion_motion * (other_exploded + dt);
								}
								
								for( size_t i = 0; i + 2 < array_inst.VertexCount; i += 3 )
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

										if( other_exploded )
										{
											Pos3D draw_pos( other );
											
											// Convert explosion vectors to worldspace.
											Vec3D explosion_motion = (*other_obj_iter)->GetExplosionMotion( other->ID ) * other_exploded;
											Vec3D modelspace_rotation_axis = (*other_obj_iter)->GetExplosionRotationAxis( other->ID );
											Vec3D explosion_rotation_axis = (other->Fwd * modelspace_rotation_axis.X) + (other->Up * modelspace_rotation_axis.Y) + (other->Right * modelspace_rotation_axis.Z);
											
											draw_pos.MoveAlong( &(other->Fwd), explosion_motion.X );
											draw_pos.MoveAlong( &(other->Up), explosion_motion.Y );
											draw_pos.MoveAlong( &(other->Right), explosion_motion.Z );
											
											double explosion_rotation_rate = (*other_obj_iter)->GetExplosionRotationRate( other->ID );
											draw_pos.Fwd.RotateAround( &explosion_rotation_axis, other_exploded * explosion_rotation_rate );
											draw_pos.Up.RotateAround( &explosion_rotation_axis, other_exploded * explosion_rotation_rate );
											draw_pos.Right.RotateAround( &explosion_rotation_axis, other_exploded * explosion_rotation_rate );
											
											other_arrays[ &(other_array_iter->second) ].MakeWorldSpace( &draw_pos );
										}
										else
											other_arrays[ &(other_array_iter->second) ].MakeWorldSpace( other );
									}
									
									GLdouble *other_worldspace = other_arrays[ &(other_array_iter->second) ].WorldSpaceVertexArray;
									size_t other_vertex_count = other_arrays[ &(other_array_iter->second) ].VertexCount;
									
									//std::vector<Pos3D> vertices1;
									std::vector<Pos3D> vertices2;
									for( size_t i = 0; i < other_vertex_count; i ++ )
									{
										//vertices1.push_back( Pos3D( other_worldspace[ i*3 ], other_worldspace[ i*3 + 1 ], other_worldspace[ i*3 + 2 ] ) );
										vertices2.push_back( Pos3D( other_worldspace[ i*3 ] - motion.X, other_worldspace[ i*3 + 1 ] - motion.Y, other_worldspace[ i*3 + 2 ] - motion.Z ) );
									}
									
									if( other_exploded )
									{
										Vec3D modelspace_explosion_motion = (*other_obj_iter)->GetExplosionMotion( other->ID );
										Vec3D worldspace_explosion_motion = other->Fwd * modelspace_explosion_motion.X + other->Up * modelspace_explosion_motion.Y + other->Right * modelspace_explosion_motion.Z;
										
										for( size_t i = 0; i < other_vertex_count; i ++ )
										{
											vertices2[ i ] += (*other_obj_iter)->GetExplosionMotion( other->ID ) * ship->ExplosionRate() * dt;
											// FIXME: Apply explosion rotation too!
										}
									}
									
									// See if any of the edges will be crossing their faces.
									for( size_t i = 0; i + 2 < vertex_count; i += 3 )
									{
										for( size_t j = 0; j + 2 < other_vertex_count; j += 3 )
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
									for( size_t i = 0; i + 2 < vertex_count; i += 3 )
									{
										for( size_t j = 0; j + 2 < other_vertex_count; j += 3 )
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
									for( size_t i = 0; i + 2 < vertex_count; i += 3 )
									{
										for( size_t j = 0; j < other_vertex_count; j ++ )
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
				
				if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius * (1. + exploded * 0.25) + ship->Radius()) )
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
					if( exploded )
						offset += obj_iter->second.GetExplosionMotion( ID ) * exploded;
					Pos3D center = *this + offset;
					
					// If these two objects don't pass near each other, don't bother checking faces.
					// FIXME: Apply worldspace_explosion_motion to MotionVector.
					if( Math3D::MinimumDistance( &center, &MotionVector, other, &(other->MotionVector), dt ) > (obj_iter->second.MaxRadius + ship->Radius()) )
						continue;
					
					for( std::map<std::string,ModelArrays>::const_iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
					{
						array_inst.BecomeInstance( &(array_iter->second), false );
						
						if( exploded )
						{
							Pos3D draw_pos( this );
							
							// Convert explosion vectors to worldspace.
							Vec3D explosion_motion = obj_iter->second.GetExplosionMotion( ID ) * exploded;
							Vec3D modelspace_rotation_axis = obj_iter->second.GetExplosionRotationAxis( ID );
							Vec3D explosion_rotation_axis = (Fwd * modelspace_rotation_axis.X) + (Up * modelspace_rotation_axis.Y) + (Right * modelspace_rotation_axis.Z);
							
							draw_pos.MoveAlong( &Fwd, explosion_motion.X );
							draw_pos.MoveAlong( &Up, explosion_motion.Y );
							draw_pos.MoveAlong( &Right, explosion_motion.Z );
							
							double explosion_rotation_rate = obj_iter->second.GetExplosionRotationRate( ID );
							draw_pos.Fwd.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
							draw_pos.Up.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
							draw_pos.Right.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
							
							array_inst.MakeWorldSpace( &draw_pos );
						}
						else
							array_inst.MakeWorldSpace( this );
						
						for( size_t i = 0; i + 2 < array_inst.VertexCount; i += 3 )
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
			if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius * (1. + exploded * 0.25) + asteroid->Radius) )
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
					
					if( exploded )
					{
						Pos3D draw_pos( this );
						
						// Convert explosion vectors to worldspace.
						Vec3D explosion_motion = obj_iter->second.GetExplosionMotion( ID ) * exploded;
						Vec3D modelspace_rotation_axis = obj_iter->second.GetExplosionRotationAxis( ID );
						Vec3D explosion_rotation_axis = (Fwd * modelspace_rotation_axis.X) + (Up * modelspace_rotation_axis.Y) + (Right * modelspace_rotation_axis.Z);
						
						draw_pos.MoveAlong( &Fwd, explosion_motion.X );
						draw_pos.MoveAlong( &Up, explosion_motion.Y );
						draw_pos.MoveAlong( &Right, explosion_motion.Z );
						
						double explosion_rotation_rate = obj_iter->second.GetExplosionRotationRate( ID );
						draw_pos.Fwd.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
						draw_pos.Up.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
						draw_pos.Right.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
						
						array_inst.MakeWorldSpace( &draw_pos );
					}
					else
						array_inst.MakeWorldSpace( this );
					
					for( size_t i = 0; i + 2 < array_inst.VertexCount; i += 3 )
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
	FiredThisFrame = 0;
	
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
	
	if( CockpitOffset.Length() > 1. )
		CockpitOffset.ScaleTo( 1. );
	CockpitOffset.ScaleBy( pow( 0.5, dt * 5. ) );
	
	GameObject::Update( dt );
	
	if( Category() == ShipClass::CATEGORY_TARGET )
		Target = 0;
}


void Ship::Draw( void )
{
	if( Subsystems.size() )
	{
		// Build a list of all objects that don't have destroyed subsystems.
		std::set<std::string> objects;
		for( std::map<std::string,ModelObject>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
		{
			std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find(obj_iter->first);
			if( (subsystem_iter == Subsystems.end()) || (subsystem_iter->second > 0.) )
				objects.insert( obj_iter->first );
		}
		
		Shape.Draw( this, (objects.size() < Shape.Objects.size()) ? &objects : NULL, NULL, Exploded() );
	}
	else
		Shape.Draw( this, NULL, NULL, Exploded() );
}


void Ship::DrawWireframe( void )
{
	Color color(0.5,0.5,1,1);
	
	if( Subsystems.size() )
	{
		// Build a set of all objects that don't have destroyed subsystems.
		std::set<std::string> objects;
		for( std::map<std::string,ModelObject>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
		{
			std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find(obj_iter->first);
			if( (subsystem_iter == Subsystems.end()) || (subsystem_iter->second > 0.) )
				objects.insert( obj_iter->first );
		}
		
		Shape.Draw( this, (objects.size() < Shape.Objects.size()) ? &objects : NULL, &color, Exploded(), ID );
	}
	else
		Shape.Draw( this, NULL, &color, Exploded(), ID );
}
