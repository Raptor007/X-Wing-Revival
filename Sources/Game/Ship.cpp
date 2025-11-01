/*
 *  Ship.cpp
 */

#include "Ship.h"

#include <cmath>
#include <algorithm>
#include "XWingDefs.h"
#include "XWingGame.h"
#include "XWingServer.h"
#include "Num.h"
#include "Rand.h"
#include "Math3D.h"
#include "Shot.h"
#include "Asteroid.h"
#include "Checkpoint.h"
#include "DeathStar.h"


Ship::Ship( uint32_t id ) : BlastableObject( id, XWing::Object::SHIP )
{
	Clear();
}


Ship::Ship( const ShipClass *ship_class ) : BlastableObject( 0, XWing::Object::SHIP )
{
	Clear();
	SetClass( ship_class );
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
	
	Health = 100.;
	CollisionPotential = 0.;
	ShieldF = 0.;
	ShieldR = 0.;
	ShieldPos = SHIELD_CENTER;
	HitByID = 0;
	HitFlags = 0x00;
	BlastPoints.clear();
	JumpProgress = 0.;
	JumpedOut = false;
	
	Firing = false;
	SelectedWeapon = 0;
	FiredThisFrame = 0;
	PredictedShots.clear();
	FiringMode.clear();
	WeaponIndex.clear();
	
	Target = 0;
	TargetLock = 0.f;
	TargetSubsystem = 0;
	NextCheckpoint = 0;
	
	EngineSoundDir = 0;
	EngineSoundClock.Reset();
	EngineSoundPrev = "";
	EngineFlicker = 1.f;
}


void Ship::ClientInit( void )
{
	if( Class && Class->ExternalModel.length() )
	{
		Model *model = NULL;
		
		if( Group && Raptor::Game->Cfg.SettingAsBool("g_group_skins",true) )
		{
			std::map<uint8_t,std::string>::const_iterator skin_iter = Class->GroupSkins.find( Group );
			if( (skin_iter != Class->GroupSkins.end()) && skin_iter->second.length() )
				model = Raptor::Game->Res.GetModel( skin_iter->second );
		}
		
		if( !( model && model->VertexCount() ) )
			model = Raptor::Game->Res.GetModel( Class->ExternalModel );
		
		if( model )
		{
			Shape.BecomeInstance( model );
			Shape.ScaleBy( Class->ModelScale );
		}
	}
}


bool Ship::SetClass( uint32_t ship_class_id )
{
	GameObject *obj = Data->GetObject( ship_class_id );
	if( obj && (obj->Type() == XWing::Object::SHIP_CLASS) )
	{
		SetClass( (const ShipClass*) obj );
		return true;
	}
	return false;
}


void Ship::SetClass( const ShipClass *ship_class )
{
	// Reset now retains firing modes, but we can't assume they are compatible between different ship classes.
	if( Class != ship_class )
		FiringMode.clear();
	
	Class = ship_class;
	
	if( Class )
	{
		if( ClientSide() )
		{
			Engines.clear();
			for( std::vector<ShipClassEngine>::const_iterator engine_iter = ship_class->Engines.begin(); engine_iter != ship_class->Engines.end(); engine_iter ++ )
			{
				Engines.push_back( ShipEngine( &*engine_iter ) );
				Engines.back().Texture.Timer.Sync( &DeathClock );
				Engines.back().Texture.Timer.TimeScale = 1.;
			}
		}
		else if( Class->Shape.VertexCount() )
		{
			Shape.BecomeInstance( &(Class->Shape) );
			Shape.GetMaxRadius();
		}
		else if( ! Class->CollisionModel.empty() )
		{
			Shape.LoadOBJ( std::string("Models/") + Class->CollisionModel, false );
			Shape.ScaleBy( Class->ModelScale );
			Shape.GetMaxRadius();
		}
		else
			Shape.Clear();
	}
	
	Reset();
}


void Ship::Reset( void )
{
	// Reset typically clears JumpProgress, but clients that joined late or resynced should keep value of 1 from init packet.
	// If JumpProgress is negative, we already have a delayed respawn.  Don't do it again because each time queues a jump-in sound!
	if( (JumpProgress >= 0.) && ((JumpProgress != 1.) || ! ClientSide()) )
		JumpProgress = 0.;
	
	JumpedOut = false;
	double jump_time = ((Category() == ShipClass::CATEGORY_CAPITAL) && ! IsMissionObjective) ? 0.7 : 0.5;
	Lifetime.Reset( jump_time );
	Health = MaxHealth();
	ShieldF = MaxShield();
	ShieldR = ShieldF;
	ShieldPos = SHIELD_CENTER;
	Subsystems.clear();
	SubsystemNames.clear();
	CollisionPotential = 500.;
	HitByID = 0;
	BlastPoints.clear();
	
	Target = NextCheckpoint;
	TargetLock = 0.f;
	TargetSubsystem = 0;
	Firing = false;
	WeaponIndex.clear();
	Ammo.clear();
	SelectedWeapon = 0;
	PredictedShots.clear();
	
	std::map<uint8_t,uint8_t> prev_firing_modes = FiringMode;
	FiringMode.clear();
	
	if( Class )
	{
		Subsystems = Class->Subsystems;
		for( std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.begin(); subsystem_iter != Subsystems.end(); subsystem_iter ++ )
		{
			SubsystemNames.push_back( subsystem_iter->first );
			
			if( ! ClientSide() )
			{
				std::map<std::string,ModelObject*>::iterator object_iter = Shape.Objects.find( subsystem_iter->first );
				if( object_iter != Shape.Objects.end() )
				{
					Pos3D offset = object_iter->second->GetCenterPoint();
					SubsystemCenters.push_back( Vec3D( offset.X, offset.Y, offset.Z ) );
				}
				else
					SubsystemCenters.push_back( Vec3D( 0., 0., 0. ) );
			}
		}
		
		CollisionPotential = Class->CollisionDamage;
		
		Ammo = Class->Ammo;
		SelectedWeapon = Class->Weapons.size() ? Class->Weapons.begin()->first : 0;
		for( std::map< uint8_t, std::vector<Pos3D> >::const_iterator weapon_iter = Class->Weapons.begin(); weapon_iter != Class->Weapons.end(); weapon_iter ++ )
		{
			FiringClocks[ weapon_iter->first ].Reset();
			FiringMode[ weapon_iter->first ] = std::max<uint8_t>( 1, prev_firing_modes[ weapon_iter->first ] );
		}
	}
	
	if( (JumpProgress == 0.) && ClientSide() && (Category() != ShipClass::CATEGORY_TARGET) )
	{
		XWingGame *game = (XWingGame*) Raptor::Game;
		if( PlayerID == game->PlayerID )
		{
			double hyperspace = game->Data.PropertyAsDouble("hyperspace");
			DelaySpawn( hyperspace );
			Mix_Chunk *sound = game->Res.GetSound("jump_in_cockpit.wav");
			//if( hyperspace <= 1.25 )
				game->Snd.Play( sound );
			//else
			//	game->Snd.PlayDelayed( sound, hyperspace - 1.25 ); // FIXME: Implement this in SoundOut?
		}
		else if( game->State >= XWing::State::FLYING )
		{
			double loudness = ((ID == game->ObservedShipID) || (Category() == ShipClass::CATEGORY_CAPITAL) || (Category() == ShipClass::CATEGORY_TRANSPORT)) ? 30. : 1.;
			game->Snd.PlayFromObject( game->Res.GetSound("jump_in.wav"),   this, loudness );
			game->Snd.PlayFromObject( game->Res.GetSound("jump_in_2.wav"), this, pow( log(Radius()), 2. ) / 3. );
		}
	}
	else if( PlayerID && ! ClientSide() )
	{
		XWingServer *server = (XWingServer*) Raptor::Server;
		DelaySpawn( server->Data.PropertyAsDouble("hyperspace") );
	}
}


void Ship::DelaySpawn( double secs )
{
	if( secs > 0. )
	{
		Lifetime.Reset();
		Lifetime.Advance( secs );
		JumpProgress = -1.;
	}
}


void Ship::SetHealth( double health )
{
	if( (health <= 0.) && (Health > 0.) )
	{
		DeathClock.Reset();
		ShieldF = 0.;
		ShieldR = 0.;
		if( PlayerID == Raptor::Game->PlayerID )
			Raptor::Game->Snd.AttenuateFor = -1;  // When our own ship explodes, it should be loud.
	}
	else if( (health > 0.) && (Health <= 0.) )
		Reset();
	
	Health = health;
}


void Ship::AddDamage( double front, double rear, const char *subsystem, uint32_t hit_by_id, double max_hull_damage )
{
	// Damage to shield towers can't be blocked by shields, and doesn't carry to the hull.
	if( subsystem && (strncmp( subsystem, "ShieldGen", strlen("ShieldGen") ) == 0) && (Subsystems.find(subsystem) != Subsystems.end()) )
	{
		Subsystems[ subsystem ] -= (front + rear);
		
		if( Subsystems[ subsystem ] <= 0. )
		{
			Subsystems[ subsystem ] = 0.;
			
			int num_shield_towers = 0, intact_shield_towers = 0;
			for( std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.begin(); subsystem_iter != Subsystems.end(); subsystem_iter ++ )
			{
				if( strncmp( subsystem_iter->first.c_str(), "ShieldGen", strlen("ShieldGen") ) == 0 )
				{
					num_shield_towers ++;
					if( subsystem_iter->second > 0. )
						intact_shield_towers ++;
				}
			}
			
			if( intact_shield_towers )
			{
				ShieldF *= intact_shield_towers / (double) num_shield_towers;
				ShieldR *= intact_shield_towers / (double) num_shield_towers;
			}
			else
			{
				ShieldF = 0.;
				ShieldR = 0.;
			}
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
	
	if( hull_damage > max_hull_damage )
		hull_damage = max_hull_damage;
	
	// Damage to most subsystems is carried on to the hull too.
	if( subsystem && (Subsystems.find(subsystem) != Subsystems.end()) )
	{
		Subsystems[ subsystem ] -= hull_damage;
		
		if( Subsystems[ subsystem ] <= 0. )
		{
			Subsystems[ subsystem ] = 0.;
			
			if( strncmp( subsystem, "Critical", strlen("Critical") ) == 0 )
			{
				int num_critical = 0, intact_critical = 0;
				for( std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.begin(); subsystem_iter != Subsystems.end(); subsystem_iter ++ )
				{
					if( strncmp( subsystem_iter->first.c_str(), "Critical", strlen("Critical") ) == 0 )
					{
						num_critical ++;
						if( subsystem_iter->second > 0. )
							intact_critical ++;
					}
				}
				
				// Destroying all critical components destroys the whole ship.
				if( ! intact_critical )
					hull_damage += Health;
			}
		}
	}
	
	if( hit_by_id )
		HitByID = hit_by_id;
	
	SetHealth( std::min<double>( Health - hull_damage, MaxHealth() ) );
	
	HitClock.Reset();
	HitFlags = 0x00;
	if( front > 0. )
		HitFlags |= HIT_FRONT;
	if( rear > 0. )
		HitFlags |= HIT_REAR;
	if( hull_damage )
		HitFlags |= HIT_HULL;
	if( hull_damage < 0. )
		HitFlags |= HIT_REPAIR;
}


bool Ship::Repair( double heal )
{
	if( Health < MaxHealth() )
	{
		ShieldF = ShieldR = 0.;  // Must drop shields to repair hull damage.
		
		AddDamage( 0., 0., NULL, 0, heal * -1. );  // NOTE: This handles HitFlags and HitClock.
		
		return true;
	}
	return false;
}


bool Ship::Rearm( int count )
{
	for( std::map<uint8_t,int8_t>::iterator ammo_iter = Ammo.begin(); ammo_iter != Ammo.end(); ammo_iter ++ )
	{
		int8_t max_ammo = MaxAmmo( ammo_iter->first );  // NOTE: MaxAmmo returns -1 for lasers (or weapon not found).
		if( ammo_iter->second < max_ammo )
		{
			ShieldF = ShieldR = 0.;  // Must drop shields to rearm.
			HitFlags = 0x00;
			HitClock.Reset();
			
			if( (count > 0) && ! ammo_iter->second )
				FiringMode[ ammo_iter->first ] = 1;
			
			Ammo[ ammo_iter->first ] = std::max<int8_t>( 0, std::min<int8_t>( ammo_iter->second + count, max_ammo ) );
			
			FiringClocks[ ammo_iter->first ].Reset();  // Can't fire a weapon being rearmed.
			
			return true;
		}
	}
	return false;
}


void Ship::KnockCockpit( const Vec3D *dir, double force )
{
	CockpitOffset.X += Fwd.Dot(dir) * force;
	CockpitOffset.Y += Up.Dot(dir) * force;
	CockpitOffset.Z += Right.Dot(dir) * force;
}


void Ship::SetBlastPoint( double x, double y, double z, double radius, double time )
{
	Pos3D shot( x, y, z );
	double fwd   = shot.DistAlong( &Fwd,   this );
	double up    = shot.DistAlong( &Up,    this );
	double right = shot.DistAlong( &Right, this );
	
	if( Category() == ShipClass::CATEGORY_CAPITAL )
		radius *= 3.;
	else
	{
		radius *= Radius() / 10.;
		double front = Shape.GetLength() * 0.5;
		double top   = Shape.GetHeight() * 0.5;
		double side  = Shape.GetWidth()  * 0.5;
		if( fabs(fwd) > front )
			fwd = front * Num::Sign(fwd);
		if( fabs(up) > top )
			up = top * Num::Sign(up);
		if( fabs(right) > side )
			right = side * Num::Sign(right);
	}
	
	SetBlastPointRelative( fwd, up, right, radius, time );
}


void Ship::SetRoll( double roll, double dt )
{
	if( JumpedOut )
	{
		RollRate = 0.;
		return;
	}
	
	if( ! Num::Valid(roll) )
		roll = 0.;
	else if( roll > 1. )
		roll = 1.;
	else if( roll < -1. )
		roll = -1.;
	
	double max_change = dt * MaxRollChange();
	double desired = roll * MaxRoll();
	
	int8_t sign1 = Num::Sign(desired), sign2 = Num::Sign(RollRate);
	if( sign1 && sign2 && (sign1 != sign2) )
		max_change *= 2.;
	
	if( fabs( desired - RollRate ) > max_change )
		RollRate += max_change * Num::Sign( desired - RollRate );
	else
		RollRate = desired;
}


void Ship::SetPitch( double pitch, double dt )
{
	if( JumpedOut )
	{
		PitchRate = 0.;
		return;
	}
	
	if( ! Num::Valid(pitch) )
		pitch = 0.;
	else if( pitch > 1. )
		pitch = 1.;
	else if( pitch < -1. )
		pitch = -1.;
	
	double max_change = dt * MaxPitchChange();
	double desired = pitch * MaxPitch();
	
	int8_t sign1 = Num::Sign(desired), sign2 = Num::Sign(PitchRate);
	if( sign1 && sign2 && (sign1 != sign2) )
		max_change *= 2.;
	
	if( fabs( desired - PitchRate ) > max_change )
		PitchRate += max_change * Num::Sign( desired - PitchRate );
	else
		PitchRate = desired;
}


void Ship::SetYaw( double yaw, double dt )
{
	if( JumpedOut )
	{
		YawRate = 0.;
		return;
	}
	
	if( ! Num::Valid(yaw) )
		yaw = 0.;
	else if( yaw > 1. )
		yaw = 1.;
	else if( yaw < -1. )
		yaw = -1.;
	
	double max_change = dt * MaxYawChange();
	double desired = yaw * MaxYaw();
	
	int8_t sign1 = Num::Sign(desired), sign2 = Num::Sign(YawRate);
	if( sign1 && sign2 && (sign1 != sign2) )
		max_change *= 2.;
	
	if( fabs( desired - YawRate ) > max_change )
		YawRate += max_change * Num::Sign( desired - YawRate );
	else
		YawRate = desired;
}


double Ship::GetThrottle( void ) const
{
	double max_speed = MaxSpeed();
	return max_speed ? (MotionVector.Length() / max_speed) : 0.;
}


void Ship::SetThrottle( double throttle, double dt )
{
	if( JumpedOut || (throttle > 1.) || ! Num::Valid(throttle) )
		throttle = 1.;
	else if( throttle < 0. )
		throttle = 0.;
	
	double old_speed = MotionVector.Length();
	double new_speed = throttle * MaxSpeed();
	double max_change = dt * Acceleration();
	
	if( fabs( new_speed - old_speed ) > max_change )
		new_speed = old_speed + max_change * Num::Sign( new_speed - old_speed );
	
	MotionVector.Copy( &Fwd );
	MotionVector.ScaleTo( new_speed );
	
	CockpitOffset.X += (new_speed - old_speed) * 3. * dt;
}


std::string Ship::SetThrottleGetSound( double throttle, double dt )
{
	double prev_throttle = GetThrottle();
	SetThrottle( throttle, dt );
	
	std::string engine_sound;
	
	if( Class && Class->FlybySounds.size() )
	{
		// Play flyby sounds for sudden acceleration/deceleration.
		
		double engine_sound_elapsed = EngineSoundClock.ElapsedSeconds();
		if( engine_sound_elapsed > 0.5 )
			EngineSoundDir = 0;
		
		if( (throttle > prev_throttle) && (prev_throttle < 0.75) && (GetThrottle() >= 0.75) && (EngineSoundDir >= 0) )
		{
			engine_sound = Class->FlybySounds.rbegin()->second;
			EngineSoundDir = 1;
		}
		else if( (Class->FlybySounds.size() >= 2) && (throttle < prev_throttle) && (prev_throttle > 0.875) && (GetThrottle() <= 0.875) && (EngineSoundDir <= 0) )
		{
			engine_sound = Class->FlybySounds.begin()->second;
			EngineSoundDir = -1;
		}
		else if( (Class->FlybySounds.size() >= 3) && (throttle > prev_throttle) && (prev_throttle < 0.5) && (GetThrottle() >= 0.5) && (EngineSoundDir >= 0) )
		{
			std::map<double,std::string>::const_reverse_iterator sound_iter = Class->FlybySounds.rbegin();
			sound_iter ++;
			engine_sound = sound_iter->second;
			EngineSoundDir = 1;
		}
		
		if( ! engine_sound.empty() )
		{
			if( (engine_sound_elapsed > 1.5) || (engine_sound != EngineSoundPrev) )
			{
				EngineSoundPrev = engine_sound;
				EngineSoundClock.Reset();
			}
			else
				engine_sound = "";
		}
	}
	
	return engine_sound;
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


double Ship::MaxGeneric( double slow, double fast, double exponent ) const
{
	double speed = pow( GetThrottle(), exponent );
	return slow * (1. - speed) + fast * speed;
}


double Ship::MaxRoll( void ) const
{
	if( Class )
		return MaxGeneric( Class->RollSlow, Class->RollFast, Class->RollExponent );
	
	return 180.;
}


double Ship::MaxPitch( void ) const
{
	if( Class )
		return MaxGeneric( Class->PitchSlow, Class->PitchFast, Class->PitchExponent );
	
	return 100.;
}


double Ship::MaxYaw( void ) const
{
	if( Class )
		return MaxGeneric( Class->YawSlow, Class->YawFast, Class->YawExponent );
	
	return 80.;
}


double Ship::MaxRollChange( void ) const
{
	if( Class )
		return MaxGeneric( Class->RollChangeSlow, Class->RollChangeFast, Class->RollChangeExponent );
	
	return MaxRoll() * 8.;
}


double Ship::MaxPitchChange( void ) const
{
	if( Class )
		return MaxGeneric( Class->PitchChangeSlow, Class->PitchChangeFast, Class->PitchChangeExponent );
	
	return MaxPitch() * 8.;
}


double Ship::MaxYawChange( void ) const
{
	if( Class )
		return MaxGeneric( Class->YawChangeSlow, Class->YawChangeFast, Class->YawChangeExponent );
	
	return MaxYaw() * 8.;
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
	if( ComplexCollisionDetection() && (Category() == ShipClass::CATEGORY_CAPITAL) )
		return 3.;
	
	return 0.25;
}


int Ship::WeaponCount( int weapon_type ) const
{
	if( Class )
	{
		std::map< uint8_t, std::vector<Pos3D> >::const_iterator weapon_iter = Class->Weapons.find( weapon_type );
		return (weapon_iter != Class->Weapons.end()) ? weapon_iter->second.size() : 0;
	}
	
	return 0;
}


uint8_t Ship::Category( void ) const
{
	return Class ? Class->Category : (uint8_t) ShipClass::CATEGORY_UNKNOWN;
}


bool Ship::HasDockingBays( void ) const
{
	return (Class && Class->Dockable);
}


bool Ship::PlayersCanFly( void ) const
{
	return (Class && Class->PlayersCanFly());
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


Pos3D Ship::HeadPosVR( void ) const
{
	Pos3D head( this );
	if( Class )
	{
		head.MoveAlong( &Fwd,   Class->CockpitPosVR.X );
		head.MoveAlong( &Up,    Class->CockpitPosVR.Y );
		head.MoveAlong( &Right, Class->CockpitPosVR.Z );
	}
	return head;
}


double Ship::Exploded( void ) const
{
	if( Health <= 0. )
		return DeathClock.ElapsedSeconds() * ExplosionRate();
	
	return 0.;
}


int Ship::ExplosionSeed( void ) const
{
	// Only use synchronized data that will not change as the dead ship continues forward!
	// If this ship has complex collision detection, other ships can collide with the pieces.
	return (ID + (int)(Fwd.X * 1024.)) % 32;
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


std::vector<Shot*> Ship::NextShots( GameObject *target, uint8_t firing_mode ) const
{
	std::vector<Shot*> shots;
	
	if( ! SelectedWeapon )
		return shots;
	
	if( ! firing_mode )
		firing_mode = CurrentFiringMode();
	
	// If ammo is limited, don't fire more than the ammo we have.
	std::map<uint8_t,int8_t>::const_iterator ammo_iter = Ammo.find(SelectedWeapon);
	if( (ammo_iter != Ammo.end()) && (ammo_iter->second >= 0) && (ammo_iter->second < (int8_t)firing_mode) )
		firing_mode = ammo_iter->second;
	
	for( int num = 0; num < (int) firing_mode; num ++ )
	{
		int weapon_index = CurrentWeaponIndex() + num;
		
		Shot *shot = new Shot();
		shot->Copy( this );
		shot->FiredFrom = ID;
		shot->PlayerID = PlayerID;
		shot->ShotType = SelectedWeapon;
		
		// Imperial capital ships always use green turbolasers.
		if( (Team == XWing::Team::EMPIRE) && (shot->ShotType == Shot::TYPE_TURBO_LASER_RED) )
			shot->ShotType = Shot::TYPE_TURBO_LASER_GREEN;
		
		shot->MotionVector.Set( Fwd.X, Fwd.Y, Fwd.Z );
		shot->MotionVector.ScaleTo( shot->Speed() );
		
		double fwd = 0., up = 0., right = 0.;
		
		if( Class )
		{
			std::map< uint8_t, std::vector<Pos3D> >::const_iterator weapon_iter = Class->Weapons.find( SelectedWeapon );
			if( (weapon_iter != Class->Weapons.end()) && weapon_iter->second.size() )
			{
				size_t weapon_count = weapon_iter->second.size();
				if( num && (firing_mode > 1) && (firing_mode < weapon_count) )
					weapon_index += weapon_count - firing_mode - 1;
				
				weapon_index %= weapon_iter->second.size();
				shot->WeaponIndex = weapon_index;
				
				fwd   = weapon_iter->second.at( weapon_index ).X;
				up    = weapon_iter->second.at( weapon_index ).Y;
				right = weapon_iter->second.at( weapon_index ).Z;
			}
		}
		
		shot->MoveAlong( &Fwd, fwd );
		shot->MoveAlong( &Up, up );
		shot->MoveAlong( &Right, right );
		
		shots.push_back( shot );
		
		// If we had a lock for a seeking weapon, give the shot its target.
		if( ((shot->ShotType == Shot::TYPE_TORPEDO) || (shot->ShotType == Shot::TYPE_MISSILE)) && target && (TargetLock >= 1.) )
		{
			shot->Seeking = target->ID;
			shot->SeekingSubsystem = TargetSubsystem;
		}
	}
	
	return shots;
}


std::vector<Shot*> Ship::AllShots( GameObject *target ) const
{
	return NextShots( target, WeaponCount(SelectedWeapon) );
}


void Ship::JustFired( void )
{
	JustFired( SelectedWeapon, CurrentFiringMode() );
}


void Ship::JustFired( uint8_t weapon, uint8_t mode )
{
	FiringClocks[ weapon ].Reset();
	
	std::map<uint8_t,int8_t>::const_iterator ammo_iter = Ammo.find( weapon );
	if( (ammo_iter != Ammo.end()) && (ammo_iter->second > 0) )
	{
		if( ammo_iter->second > mode )
			Ammo[ weapon ] -= mode;
		else
			Ammo[ weapon ] = 0;
	}
	
	if( (SelectedWeapon == weapon) && ! FiredThisFrame )
	{
		WeaponIndex[ weapon ] ++;
		if( WeaponIndex[ weapon ] >= WeaponCount(weapon) )
			WeaponIndex[ weapon ] = 0;
	}
	
	FiredThisFrame ++;
}


bool Ship::NextWeapon( void )
{
	uint8_t prev = SelectedWeapon;
	
	if( Class )
	{
		std::map< uint8_t, int8_t >::const_iterator ammo_iter = Class->Ammo.find( SelectedWeapon );
		if( ammo_iter != Class->Ammo.end() )
			ammo_iter ++;
		if( ammo_iter == Class->Ammo.end() )
			ammo_iter = Class->Ammo.begin();
		if( ammo_iter != Class->Ammo.end() )
			SelectedWeapon = ammo_iter->first;
	}
	
	if( SelectedWeapon != prev )
	{
		FiringClocks[ SelectedWeapon ].Reset();
		return true;
	}
	return false;
}


bool Ship::NextFiringMode( void )
{
	uint8_t firing_mode = CurrentFiringMode();
	uint8_t prev = firing_mode;
	
	uint8_t count = WeaponCount(SelectedWeapon);
	if( firing_mode < count )
	{
		for( uint8_t next_mode = firing_mode + 1; next_mode <= count; next_mode ++ )
		{
			if( count % next_mode == 0 )
			{
				firing_mode = next_mode;
				break;
			}
		}
	}
	else
		firing_mode = 1;
	
	int ammo = AmmoForWeapon( SelectedWeapon );
	if( (ammo >= 0) && (ammo < firing_mode) )
		firing_mode = ammo;
	
	FiringMode[ SelectedWeapon ] = firing_mode;
	
	return (firing_mode != prev);
}


uint8_t Ship::CurrentFiringMode( void ) const
{
	std::map<uint8_t,uint8_t>::const_iterator mode_iter = FiringMode.find( SelectedWeapon );
	uint8_t firing_mode = (mode_iter != FiringMode.end()) ? mode_iter->second : 1;
	std::map<uint8_t,int8_t>::const_iterator ammo_iter = Ammo.find( SelectedWeapon );
	if( (ammo_iter != Ammo.end()) && (ammo_iter->second >= 0) && (ammo_iter->second < (int8_t)firing_mode) )
		return ammo_iter->second;
	return firing_mode;
}


uint8_t Ship::CurrentWeaponIndex( void ) const
{
	std::map<uint8_t,uint8_t>::const_iterator index_iter = WeaponIndex.find( SelectedWeapon );
	return (index_iter != WeaponIndex.end()) ? index_iter->second : 0;
}


double Ship::ShotDelay( uint8_t weapon ) const
{
	if( ! weapon )
		weapon = SelectedWeapon;
	
	uint8_t firing_mode = CurrentFiringMode();
	
	if( Class )
	{
		std::map<uint8_t,double>::const_iterator firetime_iter = Class->FireTime.find( weapon );
		if( firetime_iter != Class->FireTime.end() )
			return firetime_iter->second * firing_mode;
	}
	
	if( (SelectedWeapon == Shot::TYPE_TORPEDO) || (SelectedWeapon == Shot::TYPE_MISSILE) )
		return 1. * firing_mode;
	
	return 0.25 * firing_mode;
}


double Ship::LastFired( uint8_t weapon ) const
{
	if( ! weapon )
		weapon = SelectedWeapon;
	
	std::map<uint8_t,Clock>::const_iterator clock_iter = FiringClocks.find( weapon );
	return (clock_iter != FiringClocks.end()) ? clock_iter->second.ElapsedSeconds() : 0.;
}


int8_t Ship::AmmoForWeapon( uint8_t weapon ) const
{
	if( ! weapon )
		weapon = SelectedWeapon;
	if( ! weapon )
		return -1;
	
	std::map<uint8_t,int8_t>::const_iterator ammo_iter = Ammo.find( weapon );
	return (ammo_iter != Ammo.end()) ? ammo_iter->second : 0;
}


int8_t Ship::MaxAmmo( uint8_t weapon ) const
{
	if( ! weapon )
		weapon = SelectedWeapon;
	if( ! weapon )
		return -1;
	if( ! Class )
		return -1;
	
	std::map<uint8_t,int8_t>::const_iterator ammo_iter = Class->Ammo.find( weapon );
	return (ammo_iter != Class->Ammo.end()) ? ammo_iter->second : -1;
}


float Ship::LockingOn( const GameObject *target ) const
{
	if( ! target )
		return 0.f;
	if( target->Type() == XWing::Object::CHECKPOINT )
		return 0.f;
	
	if( (SelectedWeapon != Shot::TYPE_TORPEDO) && (SelectedWeapon != Shot::TYPE_MISSILE) )
		return 0.f;
	std::map<uint8_t,int8_t>::const_iterator ammo_iter = Ammo.find( SelectedWeapon );
	if( ammo_iter == Ammo.end() )
		return 0.f;
	if( ammo_iter->second == 0 )
		return 0.f;
	
	Vec3D vec_to_target( target->X - X, target->Y - Y, target->Z - Z );
	double subsystem_radius = 0.;
	if( TargetSubsystem && (target->Type() == XWing::Object::SHIP) )
	{
		const Ship *target_ship = (const Ship*) target;
		vec_to_target = target_ship->TargetCenter( TargetSubsystem ) - *this;
		std::string subsystem_name = target_ship->SubsystemName( TargetSubsystem );
		
		// Can't lock onto destroyed subsystems.
		std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find( subsystem_name );
		if( (subsystem_iter != Subsystems.end()) && (subsystem_iter->second <= 0.) )
			return 0.f;
		
		// FIXME: Should probably not use client-side model to determine subsystem radius.
		std::map<std::string,ModelObject*>::const_iterator object_iter = target_ship->Shape.Objects.find( subsystem_name );
		if( object_iter != target_ship->Shape.Objects.end() )
			subsystem_radius = object_iter->second->MaxRadius;
		if( subsystem_radius < 1. )
			subsystem_radius = 1.;
	}
	
	double dist_to_target = vec_to_target.Length();
	
	// Locking onto a subsystem requires shorter range.
	if( TargetSubsystem )
		dist_to_target *= 2.;
	
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
		if( t->Category() == ShipClass::CATEGORY_TARGET )
		{
			// Find the Death Star trench.
			for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data->GameObjects.begin(); obj_iter != Data->GameObjects.end(); obj_iter ++ )
			{
				if( obj_iter->second->Type() == XWing::Object::DEATH_STAR )
				{
					DeathStar *ds = (DeathStar*) obj_iter->second;
					if( !(ds->WithinTrenchW(this) && ds->WithinTrenchH(this)) )  // Don't allow locking onto exhaust port from outside the trench.
						return 0.f;
					break;
				}
			}
		}
		double target_radius = subsystem_radius ? subsystem_radius : t->Radius();
		required_dot = std::min<double>( 0.95, dist_to_target / (20. * target_radius) );
	}
	
	if( t_dot_fwd >= required_dot )
		// Lock on faster when closer to the target.
		return 300.f / std::max<float>( 1000.f, dist_to_target );
	
	return 0.f;
}


void Ship::UpdateTarget( const GameObject *target, uint8_t subsystem, double dt )
{
	if( Health <= 0. )
	{
		Target = 0;
		TargetLock = 0.f;
		TargetSubsystem = 0;
		return;
	}
	
	if( target && ((target->ID != Target) || (subsystem != TargetSubsystem)) )
	{
		Target = target->ID;
		TargetSubsystem = subsystem;
		
		// Lose lock progress when changing targets.
		TargetLock = 0.f;
	}
	else if( ! target )
	{
		Target = 0;
		TargetSubsystem = 0;
	}
	
	float locking_on = LockingOn( target );
	if( locking_on )
		TargetLock += locking_on * dt;
	else
		TargetLock -= dt / 2.;
	
	if( TargetLock < 0.f )
		TargetLock = 0.f;
	else while( TargetLock > 2.f ) // FIXME: The loop here is bad form, but not a major concern since it should typically only iterate once.
		TargetLock -= 0.02f; // Allow blinking to continue with maximum lock-on.
}


Pos3D Ship::TargetCenter( uint8_t subsystem ) const
{
	Pos3D pos( this );
	if( subsystem && (subsystem <= SubsystemCenters.size()) )
	{
		Vec3D offset = SubsystemCenters.at( subsystem - 1 );
		pos.MoveAlong( &Fwd,   offset.X );
		pos.MoveAlong( &Up,    offset.Y );
		pos.MoveAlong( &Right, offset.Z );
	}
	return pos;
}


std::string Ship::SubsystemName( uint8_t subsystem ) const
{
	if( subsystem && (subsystem <= SubsystemNames.size()) )
		return SubsystemNames.at( subsystem - 1 );
	return "";
}


uint8_t Ship::SubsystemID( std::string subsystem ) const
{
	std::vector<std::string>::const_iterator name_iter = std::find( SubsystemNames.begin(), SubsystemNames.end(), subsystem );
	if( name_iter == SubsystemNames.end() )
		return 0;
	uint8_t subsystem_num = 1;
	while( name_iter != SubsystemNames.begin() )
	{
		subsystem_num ++;
		name_iter --;
	}
	return subsystem_num;
}


double Ship::SubsystemMaxHealth( uint8_t subsystem ) const
{
	if( Class )
	{
		std::map<std::string,double>::const_iterator subsystem_iter = Class->Subsystems.find( SubsystemName(subsystem) );
		if( subsystem_iter != Class->Subsystems.end() )
			return subsystem_iter->second;
	}
	return 0.;
}


void Ship::ResetTurrets( void ) const
{
	std::list<Turret*> turrets = AttachedTurrets();
	for( std::list<Turret*>::iterator turret_iter = turrets.begin(); turret_iter != turrets.end(); turret_iter ++ )
	{
		Turret *turret = *turret_iter;
		turret->Attach( this, &(turret->Offset), &(turret->RelativeUp), &(turret->RelativeFwd), turret->ParentControl );
	}
}


Turret *Ship::AttachedTurret( uint8_t index ) const
{
	// Attached turrets are defined in the ship class defs and collision models.
	if( (Class && Class->Turrets.size()) || (ComplexCollisionDetection() && (Category() == ShipClass::CATEGORY_CAPITAL)) || ClientSide() )
	{
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data->GameObjects.begin(); obj_iter != Data->GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::TURRET )
			{
				Turret *turret = (Turret*) obj_iter->second;
				if( turret->ParentID == ID )
				{
					if( index )
						index --;
					else
						return turret;
				}
			}
		}
	}
	
	return NULL;
}


std::list<Turret*> Ship::AttachedTurrets( void ) const
{
	std::list<Turret*> turrets;
	
	// Attached turrets are defined in the ship class defs and collision models.
	if( (Class && Class->Turrets.size()) || (ComplexCollisionDetection() && (Category() == ShipClass::CATEGORY_CAPITAL)) || ClientSide() )
	{
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data->GameObjects.begin(); obj_iter != Data->GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::TURRET )
			{
				Turret *turret = (Turret*) obj_iter->second;
				if( turret->ParentID == ID )
					turrets.push_back( turret );
			}
		}
	}
	
	return turrets;
}


std::set<Player*> Ship::PlayersInTurrets( void ) const
{
	std::set<Player*> players;
	
	// Attached turrets are defined in the ship class defs and collision models.
	if( (Class && Class->Turrets.size()) || (ComplexCollisionDetection() && (Category() == ShipClass::CATEGORY_CAPITAL)) || ClientSide() )
	{
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data->GameObjects.begin(); obj_iter != Data->GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::TURRET )
			{
				Turret *turret = (Turret*) obj_iter->second;
				if( turret->ParentID == ID )
				{
					Player *player = Data->GetPlayer( turret->PlayerID );
					if( player )
						players.insert( player );
				}
			}
		}
	}
	
	return players;
}


Player *Ship::Owner( void ) const
{
	Player *owner = Data->GetPlayer( PlayerID );
	
	if( (! owner) && (! IsMissionObjective) && (Group != 255) )
	{
		std::set<Player*> players_in_ship = PlayersInTurrets();
		for( std::set<Player*>::iterator player_iter = players_in_ship.begin(); player_iter != players_in_ship.end(); player_iter ++ )
		{
			owner = *player_iter;
			break;
		}
	}
	
	return owner;
}


bool Ship::PlayerShouldUpdateServer( void ) const
{
	return (Health > 0.) && ! JumpedOut;
}

bool Ship::ServerShouldUpdatePlayer( void ) const
{
	return (Health <= 0.);
}

bool Ship::ServerShouldUpdateOthers( void ) const
{
	return (! JumpedOut);
}

bool Ship::CanCollideWithOwnType( void ) const
{
	return (Category() != ShipClass::CATEGORY_TARGET) && (JumpProgress >= 1.) && ! JumpedOut;
}

bool Ship::CanCollideWithOtherTypes( void ) const
{
	return (JumpProgress >= 1.) && ! JumpedOut;
}

bool Ship::ComplexCollisionDetection( void ) const
{
	return Class && Class->CollisionModel.length() && (! JumpedOut) && ((Category() == ShipClass::CATEGORY_CAPITAL) || (Health > 0.));
}


void Ship::AddToInitPacket( Packet *packet, int8_t precision )
{
	GameObject::AddToInitPacket( packet, precision );
	
	packet->AddUChar( (Team & 0x0F) | (IsMissionObjective ? 0x80 : 0) | ((JumpProgress >= 1.) ? 0x40 : 0) | (JumpedOut ? 0x20 : 0) );
	
	packet->AddUChar( Group );
	packet->AddUInt( Class ? Class->ID : 0 );
	packet->AddString( Name );
	
	packet->AddFloat( Health );
	packet->AddFloat( ShieldF );
	packet->AddFloat( ShieldR );
	
	packet->AddUChar( SelectedWeapon | (ShieldPos << 6) );
	packet->AddUChar( CurrentFiringMode() | (CurrentWeaponIndex() << 4) );
	packet->AddUInt( Target );
	packet->AddUChar( TargetSubsystem );
	packet->AddChar( Num::UnitFloatTo8( (Target && LockingOn(Data->GetObject(Target))) ? (TargetLock / 2.f) : 0.f ) );
	packet->AddUInt( NextCheckpoint );
	
	packet->AddUChar( Subsystems.size() );
	size_t subsystem_index = 0;
	for( std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.begin(); subsystem_iter != Subsystems.end(); subsystem_iter ++ )
	{
		packet->AddString( subsystem_iter->first );
		packet->AddDouble( subsystem_iter->second );
		
		Vec3D subsystem_center;
		if( subsystem_index < SubsystemCenters.size() )
			subsystem_center = SubsystemCenters.at( subsystem_index );
		packet->AddDouble( subsystem_center.X );
		packet->AddDouble( subsystem_center.Y );
		packet->AddDouble( subsystem_center.Z );
		subsystem_index ++;
	}
}


void Ship::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, precision );
	
	uint8_t team_and_flags = packet->NextUChar();
	Team               =  team_and_flags & 0x0F;
	IsMissionObjective =  team_and_flags & 0x80;
	JumpProgress       = (team_and_flags & 0x40) ? 1. : 0.;
	JumpedOut          =  team_and_flags & 0x20;
	
	Group = packet->NextUChar();
	SetClass( packet->NextUInt() );
	Name = packet->NextString();
	
	SetHealth( packet->NextFloat() );
	ShieldF = packet->NextFloat();
	ShieldR = packet->NextFloat();
	
	uint8_t weapon_shield = packet->NextUChar();
	SetShieldPos(   (weapon_shield & 0xC0) >> 6 );
	SelectedWeapon = weapon_shield & 0x3F;
	uint8_t firing_mode = packet->NextUChar();
	FiringMode[ SelectedWeapon ]  =  firing_mode & 0x0F;
	WeaponIndex[ SelectedWeapon ] = (firing_mode & 0xF0) >> 4;
	Target = packet->NextUInt();
	TargetSubsystem = packet->NextUChar();
	TargetLock = Num::UnitFloatFrom8( packet->NextChar() ) * 2.f;
	NextCheckpoint = packet->NextUInt();
	
	size_t subsystem_count = packet->NextUChar();
	for( size_t i = 0; i < subsystem_count; i ++ )
	{
		std::string subsystem   = packet->NextString();
		Subsystems[ subsystem ] = packet->NextDouble();
		double fwd   = packet->NextDouble();
		double up    = packet->NextDouble();
		double right = packet->NextDouble();
		SubsystemCenters.push_back( Vec3D( fwd, up, right ) );
	}
}


void Ship::AddToUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacketFromServer( packet, precision );
	
	packet->AddFloat( Health );
	packet->AddUChar( SelectedWeapon | (ShieldPos << 6) );
	packet->AddUChar( CurrentFiringMode() | (CurrentWeaponIndex() << 4) );
	packet->AddUInt( Target );
	packet->AddUChar( TargetSubsystem );
	packet->AddChar( Num::UnitFloatTo8( (Target && LockingOn(Data->GetObject(Target))) ? (TargetLock / 2.f) : 0.f ) );
	
	if( precision == 127 ) // Respawn  FIXME: Maybe this should be its own packet type?
	{
		packet->AddUChar( Team );
		packet->AddUChar( Group );
		packet->AddUInt( Class ? Class->ID : 0 );
		
		packet->AddUChar( SubsystemCenters.size() );
		for( size_t subsystem_index = 0; subsystem_index < SubsystemCenters.size(); subsystem_index ++ )
		{
			Vec3D subsystem_center = SubsystemCenters.at( subsystem_index );
			packet->AddDouble( subsystem_center.X );
			packet->AddDouble( subsystem_center.Y );
			packet->AddDouble( subsystem_center.Z );
		}
	}
}


void Ship::ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::ReadFromUpdatePacketFromServer( packet, precision );
	
	double prev_health = Health;
	SetHealth( packet->NextFloat() );
	uint8_t weapon_shield = packet->NextUChar();
	SetShieldPos(   (weapon_shield & 0xC0) >> 6 );
	SelectedWeapon = weapon_shield & 0x3F;
	uint8_t firing_mode = packet->NextUChar();
	FiringMode[ SelectedWeapon ]  =  firing_mode & 0x0F;
	WeaponIndex[ SelectedWeapon ] = (firing_mode & 0xF0) >> 4;
	Target = packet->NextUInt();
	TargetSubsystem = packet->NextUChar();
	TargetLock = Num::UnitFloatFrom8( packet->NextChar() ) * 2.f;
	
	if( precision == 127 ) // Respawn  FIXME: Maybe this should be its own packet type?
	{
		Team = packet->NextUChar();
		uint8_t prev_group = Group;
		Group = packet->NextUChar();
		const ShipClass *prev_class = Class;
		SetClass( packet->NextUInt() ); // This also resets the ship and starts jump-in animation.
		if( (Class != prev_class) || ((Group != prev_group) && Raptor::Game->Cfg.SettingAsBool("g_group_skins",true)) || ((Category() == ShipClass::CATEGORY_CAPITAL) && (prev_health <= 0.)) )
			ClientInit(); // Load model for new ship class (or refresh capital ship model).
		
		SubsystemCenters.clear();
		size_t subsystem_count = packet->NextUChar();
		for( size_t i = 0; i < subsystem_count; i ++ )
		{
			double fwd   = packet->NextDouble();
			double up    = packet->NextDouble();
			double right = packet->NextDouble();
			SubsystemCenters.push_back( Vec3D( fwd, up, right ) );
		}
	}
	else if( (Health < 0.) && (prev_health < 0.) && (Category() != ShipClass::CATEGORY_CAPITAL) )
	{
		// Attempting to fix explosion jitter on fighters by preventing Fwd vector from being modified after death (effecting ExplosionSeed).
		Fwd.Copy(   &(PrevPos.Fwd) );
		Up.Copy(    &(PrevPos.Up) );
		Right.Copy( &(PrevPos.Right) );
	}
}


void Ship::AddToUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacketFromClient( packet, precision );
	
	uint8_t selected_weapon = SelectedWeapon;
	uint8_t firing_and_mode = CurrentFiringMode();
	
	if( PredictedShots.size() )
	{
		uint8_t predicted = PredictedShots.front();
		PredictedShots.pop_front();
		firing_and_mode = (predicted & 0x0F) | 0xE0;  // ZeroLag Shot(s) Fired
		selected_weapon = (predicted & 0xF0) >> 4;
	}
	else if( Firing )
	{
		if( (! SelectedWeapon) || Raptor::Game->Cfg.SettingAsInt("net_zerolag",2) < ((MaxAmmo() > 0) ? 2 : 1) )
			firing_and_mode |= 0x80;
	}
	
	packet->AddUChar( selected_weapon | (ShieldPos << 6) );
	packet->AddUChar( firing_and_mode );
	packet->AddUInt( Target );
	packet->AddUChar( TargetSubsystem );
	packet->AddChar( Num::UnitFloatTo8( TargetLock / 2.f ) );
}


void Ship::ReadFromUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	// Ignore incoming position data from the client after their ship died or jumped out.
	if( (Health <= 0.) || JumpedOut )
	{
		Ship temp_ship;
		temp_ship.Health = 100.;
		temp_ship.JumpedOut = false;
		temp_ship.Class = Class;
		temp_ship.ReadFromUpdatePacketFromClient( packet, precision );
		return;
	}
	
	uint8_t prev_selected_weapon = SelectedWeapon;
	
	GameObject::ReadFromUpdatePacketFromClient( packet, precision );
	
	uint8_t weapon_shield = packet->NextUChar();
	SetShieldPos(            (weapon_shield & 0xC0) >> 6 );
	uint8_t selected_weapon = weapon_shield & 0x3F;
	
	uint8_t firing_and_mode = packet->NextUChar();
	uint8_t firing_mode = firing_and_mode & 0x0F;
	bool zerolag_shot   = firing_and_mode & 0x40;
	
	if( ! PredictedShots.size() )
	{
		Firing = firing_and_mode & 0x80;
		if( ((firing_and_mode & 0x60) != 0x40) || (MaxAmmo(selected_weapon) <= 0) )  // Bug fix for v0.4.1 clients.
			FiringMode[ selected_weapon ] = firing_mode;
		SelectedWeapon = selected_weapon;
	}
	
	// FIXME: Should these also be stored with predicted shots?
	Target = packet->NextUInt();
	TargetSubsystem = packet->NextUChar();
	TargetLock = Num::UnitFloatFrom8( packet->NextChar() ) * 2.f;
	
	if( zerolag_shot )
	{
		if( ((firing_and_mode & 0x60) == 0x40) && (MaxAmmo(selected_weapon) > 0) )  // Bug fix for v0.4.1 clients.
			firing_mode = FiringMode[ selected_weapon ];
		PredictedShots.push_back( firing_mode | (selected_weapon << 4) );
	}
	
	if( SelectedWeapon != prev_selected_weapon )
	{
		if( ! zerolag_shot )
			FiringClocks[ SelectedWeapon ].Reset();
	}
}


bool Ship::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object, Pos3D *loc, double *when ) const
{	
	// Dead ships don't collide after a bit (except capital ship chunks).
	if( (Health <= 0.) && (DeathClock.ElapsedSeconds() > PiecesDangerousTime()) )
		return false;
	
	// No sequel nonsense.
	if( JumpedOut )
		return false;
	
#define BLOCKMAP_USAGE 3
#ifdef BLOCKMAP_USAGE
	double block_size = 0.;  // Default to automatic (use longest triangle edge as block size).
#endif
	
	bool this_complex = ComplexCollisionDetection();
	uint8_t this_category = Category();
	bool other_complex = other->ComplexCollisionDetection();
	uint32_t other_type = other->Type();
	
#ifdef SV_LAZY
	// Allow "sv lazy" to reduce CPU load by disabling complex collision models on fighters (revert to old sphere behavior).
	if( (this_complex || other_complex) && Data->PropertyAsBool("lazy") )  // FIXME: Read this property ONCE and store it somewhere!
	{
		if( this_category != ShipClass::CATEGORY_CAPITAL )
			this_complex = false;
		if( (other_type == XWing::Object::SHIP) && (((const Ship*) other)->Category() != ShipClass::CATEGORY_CAPITAL) )
			other_complex = false;
	}
#endif
	
	// Let the more complex ship handle collisions.
	if( other_complex && (! this_complex) && (other_type == XWing::Object::SHIP) )
		return other->WillCollide( this, dt, other_object, this_object, loc, when );
	if( this_complex && other_complex && (other_type == XWing::Object::SHIP) && (Shape.Objects.size() < ((const Ship*) other)->Shape.Objects.size()) )
		return other->WillCollide( this, dt, other_object, this_object, loc, when );
	
	std::set<std::string> objects;
	if( this_complex )
	{
		// Don't detect collisions with destroyed subsystems.
		for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
		{
			std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find( obj_iter->first );
			if( (subsystem_iter == Subsystems.end()) || (subsystem_iter->second > 0.) )
				objects.insert( obj_iter->first );
		}
	}
	
	double exploded = Exploded();
	int explosion_seed = ExplosionSeed();
	
	if( other_type == XWing::Object::SHOT )
	{
		Shot *shot = (Shot*) other;
		
		// Ships can't shoot themselves.
		if( shot->FiredFrom == ID )
		{
			// Lasers never hit their own ship.
			if( (shot->ShotType != Shot::TYPE_MISSILE) && (shot->ShotType != Shot::TYPE_TORPEDO) )
				return false;
			
			// Allow missiles and torpedoes to hit their own ship when looping back around.
			if( shot->Lifetime.ElapsedSeconds() < 2. )
				return false;
		}
		
		// Ship turrets can't shoot the ship they're attached to.
		// FIXME: Maybe the shots should hit the ship and disappear, but not deal damage?
		GameObject *fired_from = Data->GetObject( shot->FiredFrom );
		if( fired_from && (fired_from->Type() == XWing::Object::TURRET) && (((Turret*)( fired_from ))->ParentID == ID) )
			return false;
		
		// Use face hit detection for shots vs capital ships (and fighters when BLOCKMAP_USAGE >= 5).
#if BLOCKMAP_USAGE >= 6
		if( this_complex )
#elif BLOCKMAP_USAGE >= 5
		if( this_complex && ((this_category == ShipClass::CATEGORY_CAPITAL) || ! ClientSide()) )
#else
		if( this_complex && (this_category == ShipClass::CATEGORY_CAPITAL) )
#endif
		{
			if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius * (1. + exploded * 0.25)) )
				return false;
			
			Vec3D relative_motion = (other->MotionVector - MotionVector) * dt;
			Pos3D end( other );
			end += relative_motion;
			
#if BLOCKMAP_USAGE >= 4
			// NOTE: This is currently unused because it is noticeably slower!
			if( this_category == ShipClass::CATEGORY_CAPITAL )
				return (Shape.DistanceFromLine( this, loc, &objects, this_object, exploded, explosion_seed, other, &end, 64. ) < 0.01);
			else  // if (BLOCKMAP_USAGE >= 5) && (this_category != ShipClass::CATEGORY_CAPITAL)
				return Shape.CollidesWithSphere( this, loc, &objects, this_object, exploded, explosion_seed, other, &relative_motion, 3., block_size );
#endif
			// ===== Beginning of section ignored when BLOCKMAP_USAGE >= 4 =====
			
			Randomizer rando(explosion_seed);
			ModelArrays array_inst;
			Pos3D this_loc;
			for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
			{
				// Don't detect collisions with destroyed subsystems.
				std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find( obj_iter->first );
				if( (subsystem_iter != Subsystems.end()) && (subsystem_iter->second <= 0.) )
					continue;
				
				// Get the worldspace center of object.
				Pos3D modelspace_center = obj_iter->second->CenterPoint;
				Vec3D offset = Fwd * modelspace_center.X + Up * modelspace_center.Y + Right * modelspace_center.Z;
				if( exploded )
					offset += obj_iter->second->GetExplosionMotion( ExplosionSeed(), &rando ) * exploded;
				Pos3D center = *this + offset;
				
				// If these two objects don't pass near each other, don't bother checking faces.
				// FIXME: Apply worldspace_explosion_motion to MotionVector.
				if( Math3D::MinimumDistance( &center, &MotionVector, other, &(other->MotionVector), dt ) > obj_iter->second->MaxRadius )
					continue;
				
				for( std::map<std::string,ModelArrays*>::const_iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
				{
					array_inst.BecomeInstance( array_iter->second );
					
					if( exploded )
					{
						Pos3D draw_pos( this );
						
						// Convert explosion vectors to worldspace.
						Vec3D explosion_motion = obj_iter->second->GetExplosionMotion( ExplosionSeed(), &rando ) * exploded;
						Vec3D modelspace_rotation_axis = obj_iter->second->GetExplosionRotationAxis( ExplosionSeed(), &rando );
						Vec3D explosion_rotation_axis = (Fwd * modelspace_rotation_axis.X) + (Up * modelspace_rotation_axis.Y) + (Right * modelspace_rotation_axis.Z);
						
						draw_pos.MoveAlong( &Fwd, explosion_motion.X );
						draw_pos.MoveAlong( &Up, explosion_motion.Y );
						draw_pos.MoveAlong( &Right, explosion_motion.Z );
						
						double explosion_rotation_rate = obj_iter->second->GetExplosionRotationRate( ExplosionSeed(), &rando );
						draw_pos.Fwd.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
						draw_pos.Up.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
						draw_pos.Right.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
						
						array_inst.MakeWorldSpace( &draw_pos );
					}
					else
						array_inst.MakeWorldSpace( this );
					
					for( size_t i = 0; i + 2 < array_inst.VertexCount; i += 3 )
					{
						double dist = Math3D::LineSegDistFromFace( other, &end, array_inst.WorldSpaceVertexArray + i*3, 3, &this_loc );
						if( dist < 0.1 )
						{
							if( loc )
								loc->Copy( &this_loc );
							if( this_object )
								*this_object = obj_iter->first;
							return true;
						}
					}
				}
			}
			return false;
		}
		
		// ===== End of section ignored when BLOCKMAP_USAGE >= 4 =====
		
		double dist = Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt );
		
		if( dist <= (Radius() * 2.) )
		{
			/*
			if( loc )
			{
				// FIXME
			}
			*/
			return true;
		}
	}
	
	else if( other_type == XWing::Object::SHIP )
	{
		Ship *ship = (Ship*) other;
		uint8_t other_category = ship->Category();
		
		// No sequel nonsense.
		if( ship->JumpedOut )
			return false;
		
		// Don't let ships hit the Death Star exhaust port.
		if( (this_category == ShipClass::CATEGORY_TARGET) || (other_category == ShipClass::CATEGORY_TARGET) )
			return false;
		
		// Dead ships don't collide after a bit, and they never hit other dead ships.
		if( (ship->Health <= 0.) && ((Health <= 0.) || (ship->DeathClock.ElapsedSeconds() > ship->PiecesDangerousTime())) )
			return false;
		
		// Use face hit detection for capital ships.
		if( this_complex )
		{
			Randomizer rando(explosion_seed);
			
			if( other_complex )
			{
				// The other ship also has complex collision model!
				
				double other_exploded = ship->Exploded();
				int other_explosion_seed = ship->ExplosionSeed();
				
				// If they're nowhere near each other, don't bother getting fancy.
				if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius * (1. + exploded * 0.25) + ship->Shape.MaxRadius * (1. + other_exploded * 0.25)) )
					return false;
				
				// Don't detect collisions with destroyed subsystems.
				std::set<std::string> objects2;
				for( std::map<std::string,ModelObject*>::const_iterator other_obj_iter = ship->Shape.Objects.begin(); other_obj_iter != ship->Shape.Objects.end(); other_obj_iter ++ )
				{
					std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find( other_obj_iter->first );
					if( (subsystem_iter == Subsystems.end()) || (subsystem_iter->second > 0.) )
						objects2.insert( other_obj_iter->first );
				}
				
#if BLOCKMAP_USAGE
				Vec3D relative_motion = (other->MotionVector - MotionVector) * dt;
				Vec3D *motion2 = &relative_motion;
				if( (this_category == ShipClass::CATEGORY_CAPITAL) && (other_category == ShipClass::CATEGORY_CAPITAL) )
				{
					// Performance tweaks for Fleet Battle.
					motion2 = NULL;
					block_size = 64.;
				}
#endif
#if BLOCKMAP_USAGE >= 2
				return Shape.CollidesWithModel( this, loc, &objects, this_object, exploded, explosion_seed, &(ship->Shape), other, motion2, &objects2, other_object, other_exploded, other_explosion_seed, block_size );
#else  // ===== Beginning of section ignored when BLOCKMAP_USAGE >= 2 =====
				
#if BLOCKMAP_USAGE
				// Do a quick vertex blockmap overlap check.
				if( ! Shape.CollidesWithModel( this, loc, &objects, NULL, exploded, explosion_seed, &(ship->Shape), other, motion2, &objects2, NULL, other_exploded, other_explosion_seed, block_size, false ) )
					return false;
#endif
				ModelArrays array_inst;
				std::map<const ModelArrays*,ModelArrays> other_arrays;
				Vec3D motion = (MotionVector * dt) - (other->MotionVector * dt);
				
				for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
				{
					// Don't detect collisions with destroyed subsystems.
					std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find( obj_iter->first );
					if( (subsystem_iter != Subsystems.end()) && (subsystem_iter->second <= 0.) )
						continue;
					
					Pos3D center = obj_iter->second->CenterPoint;
					center.X += X;
					center.Y += Y;
					center.Z += Z;
					
					if( exploded )
					{
						Vec3D modelspace_explosion_motion = obj_iter->second->GetExplosionMotion( explosion_seed, &rando );
						Vec3D worldspace_explosion_motion = Fwd * modelspace_explosion_motion.X + Up * modelspace_explosion_motion.Y + Right * modelspace_explosion_motion.Z;
						center += worldspace_explosion_motion * exploded;
					}
					
					std::vector<const ModelObject*> other_objects;
					
					for( std::map<std::string,ModelObject*>::const_iterator other_obj_iter = ship->Shape.Objects.begin(); other_obj_iter != ship->Shape.Objects.end(); other_obj_iter ++ )
					{
						// Don't detect collisions with destroyed subsystems.
						std::map<std::string,double>::const_iterator subsystem_iter = ship->Subsystems.find( other_obj_iter->first );
						if( (subsystem_iter != ship->Subsystems.end()) && (subsystem_iter->second <= 0.) )
							continue;
						
						Pos3D other_center = other_obj_iter->second->CenterPoint;
						other_center.X += other->X;
						other_center.Y += other->Y;
						other_center.Z += other->Z;
						
						if( other_exploded )
						{
							Vec3D modelspace_explosion_motion = other_obj_iter->second->GetExplosionMotion( other_explosion_seed, &rando );
							Vec3D worldspace_explosion_motion = other->Fwd * modelspace_explosion_motion.X + other->Up * modelspace_explosion_motion.Y + other->Right * modelspace_explosion_motion.Z;
							other_center += worldspace_explosion_motion * other_exploded;
						}
						
						// If these two objects don't pass near each other, don't bother checking faces.
						// FIXME: Apply worldspace_explosion_motion to MotionVector.
						if( Math3D::MinimumDistance( &center, &MotionVector, &other_center, &(other->MotionVector), dt ) > (obj_iter->second->MaxRadius + other_obj_iter->second->MaxRadius) )
							continue;
						
						// This object is worth checking.
						other_objects.push_back( other_obj_iter->second );
					}
					
					if( other_objects.size() )
					{
						for( std::map<std::string,ModelArrays*>::const_iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
						{
							// Make the worldspace arrays for this.
							array_inst.BecomeInstance( array_iter->second );
							
							if( exploded )
							{
								Pos3D draw_pos( this );
								
								// Convert explosion vectors to worldspace.
								Vec3D explosion_motion = obj_iter->second->GetExplosionMotion( explosion_seed, &rando ) * exploded;
								Vec3D modelspace_rotation_axis = obj_iter->second->GetExplosionRotationAxis( explosion_seed, &rando );
								Vec3D explosion_rotation_axis = (Fwd * modelspace_rotation_axis.X) + (Up * modelspace_rotation_axis.Y) + (Right * modelspace_rotation_axis.Z);
								
								draw_pos.MoveAlong( &Fwd, explosion_motion.X );
								draw_pos.MoveAlong( &Up, explosion_motion.Y );
								draw_pos.MoveAlong( &Right, explosion_motion.Z );
								
								double explosion_rotation_rate = obj_iter->second->GetExplosionRotationRate( explosion_seed, &rando );
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
									Vec3D modelspace_explosion_motion = (*other_obj_iter)->GetExplosionMotion( other_explosion_seed, &rando );
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
								for( std::map<std::string,ModelArrays*>::const_iterator other_array_iter = (*other_obj_iter)->Arrays.begin(); other_array_iter != (*other_obj_iter)->Arrays.end(); other_array_iter ++ )
								{
									if( other_arrays.find( other_array_iter->second ) == other_arrays.end() )
									{
										other_arrays[ other_array_iter->second ].BecomeInstance( other_array_iter->second );
										
										if( other_exploded )
										{
											Pos3D draw_pos( other );
											
											// Convert explosion vectors to worldspace.
											Vec3D explosion_motion = (*other_obj_iter)->GetExplosionMotion( other_explosion_seed, &rando ) * other_exploded;
											Vec3D modelspace_rotation_axis = (*other_obj_iter)->GetExplosionRotationAxis( other_explosion_seed, &rando );
											Vec3D explosion_rotation_axis = (other->Fwd * modelspace_rotation_axis.X) + (other->Up * modelspace_rotation_axis.Y) + (other->Right * modelspace_rotation_axis.Z);
											
											draw_pos.MoveAlong( &(other->Fwd), explosion_motion.X );
											draw_pos.MoveAlong( &(other->Up), explosion_motion.Y );
											draw_pos.MoveAlong( &(other->Right), explosion_motion.Z );
											
											double explosion_rotation_rate = (*other_obj_iter)->GetExplosionRotationRate( other_explosion_seed, &rando );
											draw_pos.Fwd.RotateAround( &explosion_rotation_axis, other_exploded * explosion_rotation_rate );
											draw_pos.Up.RotateAround( &explosion_rotation_axis, other_exploded * explosion_rotation_rate );
											draw_pos.Right.RotateAround( &explosion_rotation_axis, other_exploded * explosion_rotation_rate );
											
											other_arrays[ other_array_iter->second ].MakeWorldSpace( &draw_pos );
										}
										else
											other_arrays[ other_array_iter->second ].MakeWorldSpace( other );
									}
									
									GLdouble *other_worldspace = other_arrays[ other_array_iter->second ].WorldSpaceVertexArray;
									size_t other_vertex_count = other_arrays[ other_array_iter->second ].VertexCount;
									
									//std::vector<Pos3D> vertices1;
									std::vector<Pos3D> vertices2;
									for( size_t i = 0; i < other_vertex_count; i ++ )
									{
										//vertices1.push_back( Pos3D( other_worldspace[ i*3 ], other_worldspace[ i*3 + 1 ], other_worldspace[ i*3 + 2 ] ) );
										vertices2.push_back( Pos3D( other_worldspace[ i*3 ] - motion.X, other_worldspace[ i*3 + 1 ] - motion.Y, other_worldspace[ i*3 + 2 ] - motion.Z ) );
									}
									
									if( other_exploded )
									{
										Vec3D modelspace_explosion_motion = (*other_obj_iter)->GetExplosionMotion( other_explosion_seed, &rando );
										Vec3D worldspace_explosion_motion = other->Fwd * modelspace_explosion_motion.X + other->Up * modelspace_explosion_motion.Y + other->Right * modelspace_explosion_motion.Z;
										
										for( size_t i = 0; i < other_vertex_count; i ++ )
										{
											vertices2[ i ] += (*other_obj_iter)->GetExplosionMotion( other_explosion_seed, &rando ) * ship->ExplosionRate() * dt;
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
				
				// Complex collision detection found no hits.
				return false;
				
#endif  // ===== End of section ignored when BLOCKMAP_USAGE >= 2 =====
			}
			
			// The other ship uses a simple spherical collision model.
			return WillCollideWithSphere( other, ((Ship*)other)->Radius(), dt, this_object, loc, when );
		}
		
		
		double dist = Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt );
		
		if( dist <= (Radius() + ship->Radius()) )
		{
			/*
			if( loc )
			{
				// FIXME
			}
			*/
			return true;
		}
	}
	
	else if( other_type == XWing::Object::ASTEROID )
		return WillCollideWithSphere( other, ((Asteroid*)other)->Radius, dt, this_object, loc, when );
	
	// Let the Death Star and other misc objects determine whether collisions with ships occur.
	else if( (other_type == XWing::Object::DEATH_STAR_BOX) || (other_type == XWing::Object::DEATH_STAR)
	||       (other_type == XWing::Object::TURRET)         || (other_type == XWing::Object::CHECKPOINT)
	||       (other_type == XWing::Object::DOCKING_BAY) )
		return other->WillCollide( this, dt, other_object, this_object, loc, when );
	
	return false;
}


bool Ship::WillCollideWithSphere( const GameObject *other, double other_radius, double dt, std::string *this_object, Pos3D *loc, double *when ) const
{
	// Dead ships don't collide after a bit (except capital ship chunks).
	if( (Health <= 0.) && (DeathClock.ElapsedSeconds() > PiecesDangerousTime()) )
		return false;
	
	// FIXME: Set *when = ?
	
	double exploded = Exploded();
	int explosion_seed = ExplosionSeed();
	
	std::set<std::string> objects;
	if( ComplexCollisionDetection() )
	{
		// Don't detect collisions with destroyed subsystems.
		for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
		{
			std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find( obj_iter->first );
			if( (subsystem_iter == Subsystems.end()) || (subsystem_iter->second > 0.) )
				objects.insert( obj_iter->first );
		}
		
		if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius * (1. + exploded * 0.25) + other_radius) )
			return false;
		
		Vec3D relative_motion = (other->MotionVector - MotionVector) * dt;
		
#if BLOCKMAP_USAGE >= 3
		return Shape.CollidesWithSphere( this, loc, &objects, this_object, exploded, explosion_seed, other, &relative_motion, other_radius );
#endif
		// ===== Beginning of section ignored when BLOCKMAP_USAGE >= 3 =====
		
		Pos3D end( other );
		end += relative_motion;
		
		Randomizer rando(explosion_seed);
		ModelArrays array_inst;
		Pos3D intersection;
		for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
		{
			// Don't detect collisions with destroyed subsystems.
			std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find( obj_iter->first );
			if( (subsystem_iter != Subsystems.end()) && (subsystem_iter->second <= 0.) )
				continue;
			
			if( ! exploded )
			{
				// Get the worldspace center of object.
				Pos3D modelspace_center = obj_iter->second->CenterPoint;
				Vec3D offset = Fwd * modelspace_center.X + Up * modelspace_center.Y + Right * modelspace_center.Z;
				/*
				if( exploded )
					offset += obj_iter->second->GetExplosionMotion( explosion_seed, &rando ) * exploded;
				*/
				Pos3D center = *this + offset;
				
				// If these two objects don't pass near each other, don't bother checking faces.
				// FIXME: Apply worldspace_explosion_motion to MotionVector.
				if( Math3D::MinimumDistance( &center, &MotionVector, other, &(other->MotionVector), dt ) > (obj_iter->second->MaxRadius + other_radius) )
					continue;
			}
			
			for( std::map<std::string,ModelArrays*>::const_iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
			{
				array_inst.BecomeInstance( array_iter->second );
				
				if( exploded )
				{
					Pos3D draw_pos( this );
					
					// Convert explosion vectors to worldspace.
					Vec3D explosion_motion = obj_iter->second->GetExplosionMotion( explosion_seed, &rando ) * exploded;
					Vec3D modelspace_rotation_axis = obj_iter->second->GetExplosionRotationAxis( explosion_seed, &rando );
					Vec3D explosion_rotation_axis = (Fwd * modelspace_rotation_axis.X) + (Up * modelspace_rotation_axis.Y) + (Right * modelspace_rotation_axis.Z);
					
					draw_pos.MoveAlong( &Fwd, explosion_motion.X );
					draw_pos.MoveAlong( &Up, explosion_motion.Y );
					draw_pos.MoveAlong( &Right, explosion_motion.Z );
					
					double explosion_rotation_rate = obj_iter->second->GetExplosionRotationRate( explosion_seed, &rando );
					draw_pos.Fwd.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
					draw_pos.Up.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
					draw_pos.Right.RotateAround( &explosion_rotation_axis, exploded * explosion_rotation_rate );
					
					array_inst.MakeWorldSpace( &draw_pos );
				}
				else
					array_inst.MakeWorldSpace( this );
				
				for( size_t i = 0; i + 2 < array_inst.VertexCount; i += 3 )
				{
					double dist = Math3D::LineSegDistFromFace( other, &end, array_inst.WorldSpaceVertexArray + i*3, 3, &intersection );
					if( dist <= other_radius )
					{
						if( loc )
							loc->Copy( &intersection );
						if( this_object )
							*this_object = obj_iter->first;
						return true;
					}
				}
			}
		}
		
		// Complex collision detection found no hits.
		return false;
	}
	
	// ===== End of section ignored when BLOCKMAP_USAGE >= 3 =====
	
	// Simple check if two spheres collide.
	if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) <= (Radius() + other_radius) )
	{
		if( loc )
		{
			loc->X = (X + other->X) * 0.5;
			loc->Y = (Y + other->Y) * 0.5;
			loc->Z = (Z + other->Z) * 0.5;
		}
		return true;
	}
	return false;
}


void Ship::Update( double dt )
{
	Lifetime.SetTimeScale( Data->TimeScale );
	HitClock.SetTimeScale( Data->TimeScale );
	DeathClock.SetTimeScale( Data->TimeScale );
	if( SelectedWeapon )
		FiringClocks[ SelectedWeapon ].SetTimeScale( Data->TimeScale );
	
	FiredThisFrame = 0;
	
	if( Health <= 0. )
	{
		// Dead ships stay on their old course.
		RollRate = 0.;
		PitchRate = 0.;
		YawRate = 0.;
		
		PredictedShots.clear();
		
		// Long-dead capital ship pieces fade away.
		if( ClientSide() && (Category() == ShipClass::CATEGORY_CAPITAL) && (DeathClock.ElapsedSeconds() >= PiecesDangerousTime()) )
		{
			for( std::map<std::string,ModelMaterial*>::iterator mtl_iter = Shape.Materials.begin(); mtl_iter != Shape.Materials.end(); mtl_iter ++ )
				mtl_iter->second->Ambient.Alpha = std::max<float>( 0.f, mtl_iter->second->Ambient.Alpha - dt * 0.667 );
		}
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
		
		// Shield position, and keep shields legal.
		double max_shield = MaxShield();
		if( ShieldPos == Ship::SHIELD_FRONT )
		{
			ShieldF += ShieldR;
			ShieldR = 0.;
			if( ShieldF > max_shield * 2. )
				ShieldF = max_shield * 2.;
		}
		else if( ShieldPos == Ship::SHIELD_REAR )
		{
			ShieldR += ShieldF;
			ShieldF = 0.;
			if( ShieldR > max_shield * 2. )
				ShieldR = max_shield * 2.;
		}
		else
		{
			if( ShieldF > max_shield )
				ShieldF = max_shield;
			if( ShieldR > max_shield )
				ShieldR = max_shield;
		}
	}
	
	CockpitOffset.ScaleBy( pow( 0.5, dt * 5. ) );
	if( CockpitOffset.Length() > 1. )
		CockpitOffset.ScaleTo( 1. );
	
	if( ClientSide() && (BlastPoints.size() == ((XWingGame*)( Raptor::Game ))->BlastPoints) )
	{
		BlastPoint *shrink_blastpoint = LeastImportantBlastPoint();
		if( shrink_blastpoint )
			shrink_blastpoint->Radius *= pow( 0.9, dt );
	}
	
	double jump_progress = Lifetime.Progress();
	if( jump_progress > JumpProgress )
		JumpProgress = jump_progress;
	
	if( JumpedOut )
	{
		// Jumping into hyperspace.
		MotionVector.Copy( &Fwd );
		MotionVector.ScaleTo( std::max<double>( MaxSpeed(), 1000. * cos(JumpProgress * M_PI * 0.5) ) );  // Move away quickly so we stop hearing the sounds.
	}
	else if( JumpProgress < 1. )
	{
		// Arriving from hyperspace.
		CockpitOffset.X = -1. * sin( JumpProgress * M_PI / 2. );
		MotionVector.Copy( &Fwd );
		MotionVector.ScaleTo( MaxSpeed() );
	}
	
	if( Category() == ShipClass::CATEGORY_TARGET )
	{
		Target = 0;
		TargetSubsystem = 0;
	}
	
	EngineFlicker = Rand::Double( 0.875, 1.125 );
	
	GameObject::Update( dt );
	
	// After applying rotations in GameObject::Update, make sure MotionVector matches Fwd direction.
	double speed = MotionVector.Length();
	MotionVector.Copy( &Fwd );
	MotionVector.ScaleTo( speed );
}


double Ship::DrawOffset( void ) const
{
	if( JumpedOut )
		return pow( JumpProgress, (Category() == ShipClass::CATEGORY_CAPITAL) ? 1.2 : 1.1 ) * 200. * Radius();
	
	return pow( 1. - JumpProgress, (Category() == ShipClass::CATEGORY_CAPITAL) ? 1.2 : 1.1 ) * -200. * Radius();
}


double Ship::CockpitDrawOffset( void ) const
{
	if( JumpedOut )
		return pow( JumpProgress, (Category() == ShipClass::CATEGORY_CAPITAL) ? 1.2 : 1.1 ) * 200. * Radius();
	
	return (1. - sin( JumpProgress * M_PI / 2. )) * -5000.;
}


void Ship::Draw( void )
{
	Pos3D *pos = this;
	
	if( JumpProgress < 0. )
		return;
	if( JumpedOut && (JumpProgress > 1.) )
		return;
	if( (Health <= 0.) && (Category() == ShipClass::CATEGORY_CAPITAL) && (DeathClock.ElapsedSeconds() > (PiecesDangerousTime() + 2.)) )
		return;
	
	if( (JumpProgress < 1.) && (Category() != ShipClass::CATEGORY_TARGET) )
	{
		pos = new Pos3D( this );
		pos->MoveAlong( &(pos->Fwd), DrawOffset() );
	}
	
	// Engine glow is based on throttle level.
	float engine_power = ((Health > 0.) && MaxSpeed()) ? ((MotionVector.Length() + PrevMotionVector.Length()) / (MaxSpeed() * 2.)) : std::max<float>( 0.f, 1. - DeathClock.ElapsedSeconds() );
	float r = std::min<float>( 1.f, powf( engine_power, 0.2f ) * 1.01f );
	float g = std::min<float>( 1.f, powf( engine_power, 1.3f ) * 1.01f );
	float b = std::min<float>( 1.f, powf( engine_power, 1.5f ) * 1.01f );
	for( std::map<std::string,ModelMaterial*>::iterator mtl_iter = Shape.Materials.begin(); mtl_iter != Shape.Materials.end(); mtl_iter ++ )
	{
		if( Str::FindInsensitive( mtl_iter->first, "Engine" ) >= 0 )
		{
			mtl_iter->second->Ambient.Red   = r;
			mtl_iter->second->Ambient.Green = g;
			mtl_iter->second->Ambient.Blue  = b;
		}
	}
	
	// Cull model back faces to reduce z-fighting and improve performance.
	if( Health > 0. )
		glEnable( GL_CULL_FACE );
	
	if( Subsystems.size() )
	{
		// Build a list of all objects that don't have destroyed subsystems.
		std::set<std::string> objects;
		for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
		{
			std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find(obj_iter->first);
			if( (subsystem_iter == Subsystems.end()) || (subsystem_iter->second > 0.) )
				objects.insert( obj_iter->first );
		}
		
		Shape.Draw( pos, (objects.size() < Shape.Objects.size()) ? &objects : NULL, NULL, Exploded(), ExplosionSeed() );
	}
	else
		Shape.Draw( pos, NULL, NULL, Exploded(), ExplosionSeed() );
	
	glDisable( GL_CULL_FACE );
	
	if( pos != this )
		delete pos;
}


void Ship::DrawWireframe( const Color *color, double scale )
{
	DrawWireframeAt( this, color, scale );
}


void Ship::DrawWireframeAt( const Pos3D *pos, const Color *color, double scale )
{
	Color default_color(0.5,0.5,1,1);
	if( ! color )
		color = &default_color;
	
	if( Subsystems.size() )
	{
		// Build a set of all objects that don't have destroyed subsystems.
		std::set<std::string> objects;
		for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
		{
			std::map<std::string,double>::const_iterator subsystem_iter = Subsystems.find(obj_iter->first);
			if( (subsystem_iter == Subsystems.end()) || (subsystem_iter->second > 0.) )
				objects.insert( obj_iter->first );
		}
		
		Shape.Draw( pos, (objects.size() < Shape.Objects.size()) ? &objects : NULL, color, Exploded(), ExplosionSeed(), scale, scale, scale );
	}
	else
		Shape.Draw( pos, NULL, color, Exploded(), ExplosionSeed(), scale, scale, scale );
}


Shader *Ship::WantShader( void ) const
{
	return Raptor::Game->Res.GetShader("model");
}


std::map<ShipEngine*,Pos3D> Ship::EnginePositions( void )
{
	if( (JumpProgress < 0.) || (JumpedOut && (JumpProgress > 1.)) )
		return std::map<ShipEngine*,Pos3D>();
	
	if( (JumpProgress < 1.) && (Category() != ShipClass::CATEGORY_TARGET) )
	{
		Pos3D pos( this );
		pos.MoveAlong( &(pos.Fwd), DrawOffset() );
		return EnginePositions( &pos );
	}
	
	return EnginePositions( this );
}


std::map<ShipEngine*,Pos3D> Ship::EnginePositions( const Pos3D *pos )
{
	std::map<ShipEngine*,Pos3D> positions;
	
	for( std::vector<ShipEngine>::iterator engine_iter = Engines.begin(); engine_iter != Engines.end(); engine_iter ++ )
	{
		Pos3D engine_pos( pos );
		engine_pos.MoveAlong( &(pos->Fwd),   engine_iter->X );
		engine_pos.MoveAlong( &(pos->Up),    engine_iter->Y );
		engine_pos.MoveAlong( &(pos->Right), engine_iter->Z );
		positions[ &*engine_iter ] = engine_pos;
	}
	
	return positions;
}


// ---------------------------------------------------------------------------


ShipEngine::ShipEngine( const ShipClassEngine *engine ) : Vec3D( engine )
{
	Texture.BecomeInstance( Raptor::Game->Res.GetAnimation( engine->Texture ) );
	
	Radius = engine->Radius;
	DrawColor = engine->DrawColor;
}


ShipEngine &ShipEngine::operator = ( const ShipEngine &other )
{
	Vec3D::Copy( &other );
	
	Texture.BecomeInstance( &(other.Texture) );
	Texture.Timer.Sync( &(other.Texture.Timer) );
	
	Radius = other.Radius;
	DrawColor = other.DrawColor;
	
	return *this;
}


void ShipEngine::DrawAt( const Pos3D *pos, float alpha, double scale )
{
	bool use_shaders = Raptor::Game->ShaderMgr.Active();
	if( use_shaders )
		Raptor::Game->ShaderMgr.StopShaders();
	
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, Texture.CurrentFrame() );
	glColor4f( DrawColor.Red, DrawColor.Green, DrawColor.Blue, DrawColor.Alpha * alpha );
	
	// Calculate corners.
	double radius = Radius * scale;
	Vec3D tl = Raptor::Game->Cam.Up * radius - Raptor::Game->Cam.Right * radius;
	Vec3D tr = tl + Raptor::Game->Cam.Right * radius * 2.;
	Vec3D bl = tr;
	Vec3D br = tl;
	br.RotateAround( &(Raptor::Game->Cam.Fwd), 180. );
	bl.RotateAround( &(Raptor::Game->Cam.Fwd), 180. );
	
	glBegin( GL_QUADS );
		
		// Top-left
		glTexCoord2i( 0, 0 );
		glVertex3d( pos->X + tl.X, pos->Y + tl.Y, pos->Z + tl.Z );
		
		// Bottom-left
		glTexCoord2i( 0, 1 );
		glVertex3d( pos->X + bl.X, pos->Y + bl.Y, pos->Z + bl.Z );
		
		// Bottom-right
		glTexCoord2i( 1, 1 );
		glVertex3d( pos->X + br.X, pos->Y + br.Y, pos->Z + br.Z );
		
		// Top-right
		glTexCoord2i( 1, 0 );
		glVertex3d( pos->X + tr.X, pos->Y + tr.Y, pos->Z + tr.Z );
		
	glEnd();
	
	glDisable( GL_TEXTURE_2D );
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	
	if( use_shaders )
		Raptor::Game->ShaderMgr.ResumeShaders();
}
