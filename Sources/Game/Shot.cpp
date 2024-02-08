/*
 *  Shot.cpp
 */

#include "Shot.h"

#include <cmath>
#include "XWingDefs.h"
#include "XWingGame.h"
#include "Rand.h"
#include "Math3D.h"
#include "Num.h"
#include "Ship.h"
#include "Turret.h"


Shot::Shot( uint32_t id ) : GameObject( id, XWing::Object::SHOT )
{
	ShotType = TYPE_LASER_RED;
	FiredFrom = 0;
	Seeking = 0;
	SeekingSubsystem = 0;
	Drawn = false;
}


Shot::~Shot()
{
}


void Shot::ClientInit( void )
{
	if( ShotType == TYPE_LASER_RED )
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("laser_red.ani") );
	else if( ShotType == TYPE_LASER_GREEN )
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("laser_green.ani") );
	else if( ShotType == TYPE_TURBO_LASER_RED )
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("laser_red.ani") );
	else if( ShotType == TYPE_TURBO_LASER_GREEN )
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("laser_green.ani") );
	else if( ShotType == TYPE_QUAD_LASER_RED )
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("laser_red.ani") );
	else if( ShotType == TYPE_ION_CANNON )
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("ion_cannon.ani") );
	else if( ShotType == TYPE_TORPEDO )
	{
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("torpedo.ani") );
		Shape.BecomeInstance( Raptor::Game->Res.GetModel("torpedo.obj") );
	}
	else if( ShotType == TYPE_MISSILE )
	{
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("missile.ani") );
		Shape.BecomeInstance( Raptor::Game->Res.GetModel("missile.obj") );
	}
	else if( ShotType == TYPE_SUPERLASER )
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("superlaser.ani") );
	else
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("laser_unknown.ani") );
	
	if( Raptor::Game->State >= XWing::State::FLYING )
	{
		GameObject *fired_from = Data->GetObject( FiredFrom );
		
		if( ShotType == TYPE_LASER_RED )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("laser_red.wav"), X, Y, Z );
		else if( ShotType == TYPE_LASER_GREEN )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("laser_green.wav"), X, Y, Z );
		else if( ShotType == TYPE_ION_CANNON )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("ion_cannon.wav"), X, Y, Z, 0.9 );
		else if( ShotType == TYPE_TURBO_LASER_RED )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("turbolaser_red.wav"), X, Y, Z, 1.5 );
		else if( ShotType == TYPE_TURBO_LASER_GREEN )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("turbolaser_green.wav"), X, Y, Z, 1.5 );
		else if( ShotType == TYPE_QUAD_LASER_RED )
		{
			Turret *fired_from_turret = NULL;
			Ship *fired_from_ship = NULL;
			if( fired_from )
			{
				if( fired_from->Type() == XWing::Object::TURRET )
				{
					fired_from_turret = (Turret*) fired_from;
					fired_from_ship = fired_from_turret->ParentShip();
				}
				else if( fired_from->Type() == XWing::Object::SHIP )
					fired_from_ship = (Ship*) fired_from;
			}
			
			if( fired_from_turret && (fired_from_turret->PlayerID == Raptor::Game->PlayerID) )
			{
				Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("laser_turret.wav"), 0, 66 );
				Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("laser_turret_2.wav") );
			}
			else if( fired_from_ship && (fired_from_ship->PlayerID == Raptor::Game->PlayerID) )
			{
				Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("laser_turret.wav"), X, Y, Z, 1.6 );
				if( ! fired_from_turret )
					Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("laser_turret_2.wav") );
			}
			else
				Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("laser_turret.wav"), X, Y, Z, 1.1 );
		}
		else if( ShotType == TYPE_TORPEDO )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("torpedo.wav"), X, Y, Z, 0.75 );
		else if( ShotType == TYPE_MISSILE )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("missile.wav"), X, Y, Z, 0.875 );
		else if( ShotType == TYPE_SUPERLASER )
		{
			Mix_Chunk *sound = Raptor::Game->Res.GetSound("superlaser.wav");
			Raptor::Game->Snd.PlayAt( sound, X, Y, Z, 1000000. );
			if( ! fired_from )
			{
				Raptor::Game->Snd.PlayAt( sound, X, Y, Z, 100000. );
				Raptor::Game->Snd.PlayAt( sound, X, Y, Z, 10. );
			}
			else if( fired_from->PlayerID == Raptor::Game->PlayerID )
			{
				Raptor::Game->Snd.Play( sound );
				Raptor::Game->Snd.Play( sound );
			}
		}
		
		if( Seeking && (Seeking == ((XWingGame*)( Raptor::Game ))->ObservedShipID) )
		{
			const GameObject *observed_object = Data->GetObject( ((XWingGame*)( Raptor::Game ))->ObservedShipID );
			if( observed_object && (observed_object->PlayerID == Raptor::Game->PlayerID) )
				Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("incoming.wav") );
		}
		
		if( fired_from && (fired_from->Type() == XWing::Object::SHIP) )
		{
			Ship *ship = (Ship*) fired_from;
			if( (ship->PlayerID != Raptor::Game->PlayerID) && (ship->SelectedWeapon != ShotType) && (ship->Ammo.find(ShotType) != ship->Ammo.end()) )
			{
				ship->SelectedWeapon = ShotType;
				ship->WeaponIndex = 0;
			}
			ship->JustFired( ShotType, 1 );
			if( ship->SelectedWeapon && (ship->PlayerID != Raptor::Game->PlayerID) )
				ship->FiringMode[ ship->SelectedWeapon ] = ship->FiredThisFrame;
		}
	}
}


double Shot::Damage( void ) const
{
	if( ShotType == TYPE_TURBO_LASER_GREEN )
		return 33.;
	else if( ShotType == TYPE_TURBO_LASER_RED )
		return 32.;
	else if( ShotType == TYPE_QUAD_LASER_RED )
		return 29.;
	else if( ShotType == TYPE_ION_CANNON )
		return 50.;
	else if( ShotType == TYPE_TORPEDO )
		return 200.;
	else if( ShotType == TYPE_MISSILE )
		return 95.;
	else if( ShotType == TYPE_SUPERLASER )
		return 1000000.;
	
	return 30.;
}


double Shot::HullDamage( void ) const
{
	if( ShotType == TYPE_ION_CANNON )
		return 5.;
	
	return Damage();
}


double Shot::AsteroidDamage( void ) const
{
	if( ShotType == TYPE_TURBO_LASER_GREEN )
		return 150.;
	else if( ShotType == TYPE_TURBO_LASER_RED )
		return 50.;
	else if( ShotType == TYPE_QUAD_LASER_RED )
		return 0.5;
	else if( ShotType == TYPE_ION_CANNON )
		return 0.;
	else if( ShotType == TYPE_TORPEDO )
		return 100.;
	else if( ShotType == TYPE_MISSILE )
		return 50.;
	else if( ShotType == TYPE_SUPERLASER )
		return 1000000.;
	
	return 1.;
}


double Shot::Speed( void ) const
{
	if( ShotType == TYPE_TURBO_LASER_GREEN )
		return 600.;
	else if( ShotType == TYPE_TURBO_LASER_RED )
		return 600.;
	else if( ShotType == TYPE_QUAD_LASER_RED )
		return 750.;
	else if( ShotType == TYPE_ION_CANNON )
		return 550.;
	else if( ShotType == TYPE_TORPEDO )
		return 400.;
	else if( ShotType == TYPE_MISSILE )
		return 500.;
	else if( ShotType == TYPE_SUPERLASER )
		return 10000.;
	
	return 800.;
}


double Shot::TurnRate( void ) const
{
	if( ShotType == TYPE_TORPEDO )
		return 90.;
	else if( ShotType == TYPE_MISSILE )
		return 120.;
	
	return 0.;
}


double Shot::Intercept( void ) const
{
	if( ShotType == TYPE_TORPEDO )
		return 0.95;
	else if( ShotType == TYPE_MISSILE )
		return 0.99;
	
	return 0.;
}


double Shot::MaxLifetime( void ) const
{
	if( ShotType == TYPE_QUAD_LASER_RED )
		return 1.5;
	else if( ShotType == TYPE_TORPEDO )
		return 8.;
	else if( ShotType == TYPE_MISSILE )
		return 6.;
	
	return 4.;
}


Color Shot::LightColor( void ) const
{
	// Return the color of the dynamic light for this shot.
	// We store the radius in the "alpha" variable.
	
	if( ShotType == TYPE_LASER_RED )
		return Color( 0.85f, 0.f, 0.f, 15.f );
	else if( ShotType == TYPE_LASER_GREEN )
		return Color( 0.f, 0.4f, 0.f, 15.f );
	else if( ShotType == TYPE_TURBO_LASER_RED )
		return Color( 0.85f, 0.f, 0.f, 16.f );
	else if( ShotType == TYPE_TURBO_LASER_GREEN )
		return Color( 0.f, 0.4f, 0.f, 16.f );
	else if( ShotType == TYPE_QUAD_LASER_RED )
		return Color( 0.8f, 0.f, 0.f, 14.f );
	else if( ShotType == TYPE_ION_CANNON )
		return Color( 0.f, 0.3f, 1.f, 20.f );
	else if( ShotType == TYPE_TORPEDO )
		return Color( 1.f, 0.7f, 0.4f, 17.f );
	else if( ShotType == TYPE_MISSILE )
		return Color( 0.9f, 0.5f, 0.1f, 17.f );
	else if( ShotType == TYPE_SUPERLASER )
		return Color( 0.f, 0.9f, 0.01f, 10000.f );
	
	return Color( 1.f, 1.f, 1.f, 15.f );
}


Player* Shot::Owner( void ) const
{
	Player *owner = GameObject::Owner();
	
	if( Data && ! owner )
	{
		GameObject *fired_from = Data->GetObject( FiredFrom );
		if( fired_from )
		{
			owner = fired_from->Owner();
			if( (! owner) && (fired_from->Type() == XWing::Object::TURRET) )
			{
				Turret *turret = (Turret*) fired_from;
				fired_from = Data->GetObject( turret->ParentID );
				if( fired_from )
					owner = fired_from->Owner();
			}
		}
	}
	
	return owner;
}


bool Shot::PlayerShouldUpdateServer( void ) const
{
	return false;
}

bool Shot::ServerShouldUpdatePlayer( void ) const
{
	return Seeking;
}

bool Shot::ServerShouldUpdateOthers( void ) const
{
	return Seeking;
}

bool Shot::CanCollideWithOwnType( void ) const
{
	return false;
}

bool Shot::CanCollideWithOtherTypes( void ) const
{
	return true;
}


void Shot::AddToInitPacket( Packet *packet, int8_t precision )
{
	// FIXME: Dirty hacks to transmit relative weapon position without changing packet contents!  Remove in v0.4?
	const GameObject *fired_from = FiredFrom ? Data->GetObject( FiredFrom ) : NULL;
	Vec3D up( &Up );
	double dist_along_fwd = 0.;
	if( fired_from )
	{
		dist_along_fwd = DistAlong( &Fwd, fired_from );
		if( fired_from->Type() == XWing::Object::SHIP )
		{
			Pos3D weapon( this );
			weapon.MoveAlong( &(fired_from->Fwd), dist_along_fwd * -1. );
			if( weapon.Dist(fired_from) )
			{
				Up = (weapon - *fired_from).Unit();
				Up.RotateAround( &Fwd, -90. );
			}
		}
		else if( (fired_from->Type() == XWing::Object::TURRET) && ((const Turret*)( fired_from ))->GunWidth && (DistAlong( &Right, fired_from ) < 0.) )
			Up.ScaleBy( -1. );
	}
	
	GameObject::AddToInitPacket( packet, -127 );
	
	packet->AddUChar( ShotType );
	packet->AddUInt( FiredFrom );
	
	if( FiredFrom )
	{
		packet->AddFloat( dist_along_fwd );  // FIXME: Replace with weapon index in v0.4 netcode?
		Up.Copy( &up );
	}
}


void Shot::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, -127 );
	
	ShotType = packet->NextUChar();
	FiredFrom = packet->NextUInt();
	
	if( FiredFrom )
	{
		double dist_along_fwd = packet->NextFloat();  // FIXME: Replace with weapon index in v0.4 netcode?
		
		const GameObject *fired_from = Data->GetObject( FiredFrom );
		if( fired_from )
		{
			if( fired_from->Type() == XWing::Object::TURRET )
			{
				// Make sure turret shots always appear to come from the turret, even if its position may be slightly mis-predicted.
				const Turret *turret = (const Turret*) fired_from;
				Pos3D gun = turret->GunPos();
				
				if( turret->PlayerID == Raptor::Game->PlayerID )
				{
					Copy( &gun );
					MoveAlong( &(gun.Fwd), 2.2 );
				}
				else
				{
					// Shots fired from someone else's turret should copy its position but not forward direction.
					SetPos( gun.X, gun.Y, gun.Z );
					MoveAlong( &(gun.Fwd), 2. );
				}
				
				double right_dot = gun.Right.Dot( &Right ); // FIXME: Dirty hack to determine which side fired.
				MoveAlong( &(gun.Right), turret->GunWidth * right_dot );
			}
			else
			{
				if( fired_from->Type() == XWing::Object::SHIP )
				{
					const Ship *ship = (const Ship*) fired_from;
					if( ship->Class )
					{
						// Make sure ship shots appear to come from the ship's weapons by matching Up/Right offset to shot Right vector.
						std::map< uint8_t, std::vector<Pos3D> >::const_iterator weapon_iter = ship->Class->Weapons.find( ShotType );
						if( weapon_iter != ship->Class->Weapons.end() )
						{
							double best_dot = 0.5;  // Make sure ship's predicted rotation isn't too far off to guess the weapon index.
							uint8_t weapon_index = 255;
							for( uint8_t i = 0; i < (uint8_t) weapon_iter->second.size(); i ++ )
							{
								Vec3D to_weapon = ship->Up * weapon_iter->second.at( i ).Y + ship->Right * weapon_iter->second.at( i ).Z;
								double dot = Right.Dot( &to_weapon );
								if( dot > best_dot )
								{
									weapon_index = i;
									best_dot = dot;
								}
							}
							if( weapon_index != 255 )
							{
								SetPos( ship->X, ship->Y, ship->Z );
								MoveAlong( &(ship->Up),    weapon_iter->second.at( weapon_index ).Y );
								MoveAlong( &(ship->Right), weapon_iter->second.at( weapon_index ).Z );
							}
						}
					}
				}
				
				double current_dist_fwd = DistAlong( &Fwd, fired_from );
				dist_along_fwd += 1.7;  // FIXME: Temporary hack to correct for sloppy gun placements in v0.3.x ship defs!
				MoveAlong( &Fwd, dist_along_fwd - current_dist_fwd );
			}
		}
	}
}


void Shot::AddToUpdatePacket( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacket( packet, -127 );
	
	packet->AddUInt( Seeking );
	if( Seeking )
		packet->AddUChar( SeekingSubsystem );
}


void Shot::ReadFromUpdatePacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromUpdatePacket( packet, -127 );
	
	Seeking = packet->NextUInt();
	if( Seeking )
		SeekingSubsystem = packet->NextUChar();
}


bool Shot::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object ) const
{
	if( other->Type() == XWing::Object::SHOT )
	{
		/*
		// FIXME: Commented-out because Shot::CanCollideWithOwnType() is always false.
		// If we ever want to be able to shoot down missiles and torpedoes, they'll need to be their own object type that can collide with shots.
		Shot *shot = (Shot*) other;
		if( (shot->FiredFrom != FiredFrom) && ((shot->ShotType == TYPE_TORPEDO) || (ShotType == TYPE_TORPEDO) || (shot->ShotType == TYPE_MISSILE) || (ShotType == TYPE_MISSILE)) )
		{
			double dist = Math3D::MinimumDistance( this, &(this->MotionVector), other, &(other->MotionVector), dt );
			if( dist <= 1. )
				return true;
		}
		*/
		return false;
	}
	
	// Let other objects determine whether collisions with shots occur.
	return other->WillCollide( this, dt, other_object, this_object );
}


void Shot::Update( double dt )
{
	Lifetime.SetTimeScale( Data->TimeScale );
	//Anim.Timer.SetTimeScale( Data->TimeScale );  // Pause and slow-motion actually look better without this.
	
	if( Seeking )
	{
		GameObject *target = Data ? Data->GetObject( Seeking ) : NULL;
		
		if( ! target )
		{
			Seeking = 0;
			SeekingSubsystem = 0;
		}
		else if( target->Type() == XWing::Object::SHIP )
		{
			Ship *ship = (Ship*) target;
			if( ship->Health <= 0. )
			{
				target = NULL;
				Seeking = 0;
				SeekingSubsystem = 0;
			}
		}
		else if( target->Type() == XWing::Object::TURRET )
		{
			Turret *turret = (Turret*) target;
			if( turret->Health <= 0. )
			{
				target = NULL;
				Seeking = 0;
				SeekingSubsystem = 0;
			}
		}
		
		if( target )
		{
			// Point somewhere between the target and intercept point.
			
			Vec3D vec_to_target( target->X - X, target->Y - Y, target->Z - Z );
			Ship *target_ship = (Ship*)( (target->Type() == XWing::Object::SHIP) ? target : NULL );
			if( target_ship && SeekingSubsystem )
				vec_to_target = target_ship->TargetCenter( SeekingSubsystem ) - *this;
			else if( target_ship && target_ship->ComplexCollisionDetection() )
			{
				// If a ship's origin is in empty space (ex: Star Destroyer) the model object Hull defines the point to aim at.
				std::map<std::string,ModelObject>::const_iterator object_iter = target_ship->Shape.Objects.find("Hull");
				if( (object_iter != target_ship->Shape.Objects.end()) && (object_iter->second.Points.size()) )
				{
					Vec3D point = object_iter->second.Points.front();
					vec_to_target += target_ship->Fwd   * point.X;
					vec_to_target += target_ship->Up    * point.Y;
					vec_to_target += target_ship->Right * point.Z;
				}
			}
			double time_to_target = vec_to_target.Length() / Speed();
			vec_to_target += target->MotionVector * (time_to_target * Intercept());
			vec_to_target.ScaleTo( 1. );
			Up.Copy( &vec_to_target );
			FixVectors();
			
			double fwd_dot = Fwd.Dot(&vec_to_target);
			if( fwd_dot < 0.95 )
				PitchRate = TurnRate();
			else
				PitchRate = TurnRate() * (1. - fwd_dot) * 20.;
			
			MotionVector.Copy( &Fwd );
			MotionVector.ScaleTo( Speed() );
		}
	}
	
	if( ! Seeking )
		PitchRate = 0.;
	
	GameObject::Update( dt );
}


void Shot::Draw( void )
{
	// Size parameters.
	double ahead  = DrawAhead();
	double behind = DrawBehind();
	double width  = DrawWidth();
	
	// If the shot is behind us, don't draw it.
	Vec3D vec_to_front( X + Fwd.X * ahead  - Raptor::Game->Cam.X, Y + Fwd.Y * ahead  - Raptor::Game->Cam.Y, Z + Fwd.Z * ahead  - Raptor::Game->Cam.Z );
	Vec3D vec_to_rear(  X - Fwd.X * behind - Raptor::Game->Cam.X, Y - Fwd.Y * behind - Raptor::Game->Cam.Y, Z - Fwd.Z * behind - Raptor::Game->Cam.Z );
	if( (Raptor::Game->Cam.Fwd.Dot( &vec_to_front ) < 0.) && (Raptor::Game->Cam.Fwd.Dot( &vec_to_rear ) < 0.) )
	{
		Drawn = true;
		return;
	}
	
	if( Shape.Objects.size() )
	{
		// Draw the missile model.
		
		bool use_shaders = Raptor::Game->Cfg.SettingAsBool("g_shader_enable");
		if( use_shaders )
			Raptor::Game->ShaderMgr.ResumeShaders();
		
		Shape.DrawAt( this );
		
		if( use_shaders )
			Raptor::Game->ShaderMgr.StopShaders();
	}
	
	if( Anim.Frames.size() )
	{
		// Draw a laser, torpedo, or missile trail.
		
		// Calculate corners.
		Vec3D vec_to_shot( X - Raptor::Game->Cam.X, Y - Raptor::Game->Cam.Y, Z - Raptor::Game->Cam.Z );
		vec_to_shot.ScaleTo( 1. );
		Vec3D out = Raptor::Game->Cam.Right * Raptor::Game->Cam.Right.Dot(&Fwd) + Raptor::Game->Cam.Up * Raptor::Game->Cam.Up.Dot(&Fwd);
		if( out.Length() < 0.1 ) 
		{
			Vec2D vec( vec_to_shot.Dot(&(Raptor::Game->Cam.Right)), vec_to_shot.Dot(&(Raptor::Game->Cam.Up)) );
			double rotation = -1. * Num::RadToDeg( atan2( vec.Y, vec.X ) );
			out = Raptor::Game->Cam.Right;
			out.RotateAround( &(Raptor::Game->Cam.Fwd), rotation );
		}
		out.ScaleTo( width );
		Vec3D cw = out;
		cw.RotateAround( &(Raptor::Game->Cam.Fwd), 90. );
		Vec3D in = out;
		in.RotateAround( &(Raptor::Game->Cam.Fwd), 180. );
		Vec3D ccw = out;
		ccw.RotateAround( &(Raptor::Game->Cam.Fwd), 270. );
		
		// Draw order is based on whether the shot is heading towards or away from the camera.
		bool going_away = (Raptor::Game->Cam.Fwd.Dot(&Fwd) > 0.);
		behind *= -1.;
		
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, Anim.CurrentFrame() );
		glColor4f( 1.f, 1.f, 1.f, 1.f );
		
		// Don't mipmap shots; they should look bright in the distance.
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		
		if( going_away )
		{
			glBegin( GL_TRIANGLES );
				
				// Clockwise
				glTexCoord2i( 0, 1 );
				glVertex3d( X + cw.X, Y + cw.Y, Z + cw.Z );
				
				// Front
				glTexCoord2i( 1, 1 );
				glVertex3d( X + Fwd.X * ahead, Y + Fwd.Y * ahead, Z + Fwd.Z * ahead );
				
				// Counter-clockwise
				glTexCoord2i( 1, 0 );
				glVertex3d( X + ccw.X, Y + ccw.Y, Z + ccw.Z );
				
			glEnd();
		}
		else
		{
			if( ShotType == TYPE_SUPERLASER )
			{
				glBegin( GL_QUADS );
					
					// Rear CW
					glTexCoord2d( 0., 0.4 );
					glVertex3d( X + cw.X + Fwd.X * behind, Y + cw.Y + Fwd.Y * behind, Z + cw.Z + Fwd.Z * behind );
					
					// Clockwise
					glTexCoord2i( 0., 0.6 );
					glVertex3d( X + cw.X, Y + cw.Y, Z + cw.Z );
					
					// Counter-clockwise
					glTexCoord2i( 1., 0.6 );
					glVertex3d( X + ccw.X, Y + ccw.Y, Z + ccw.Z );
					
					// Rear CCW
					glTexCoord2d( 1., 0.4 );
					glVertex3d( X + ccw.X + Fwd.X * behind, Y + ccw.Y + Fwd.Y * behind, Z + ccw.Z + Fwd.Z * behind );
					
				glEnd();
				glBegin( GL_TRIANGLES );
					
					// Rear CCW
					glTexCoord2d( 1., 0.4 );
					glVertex3d( X + ccw.X + Fwd.X * behind, Y + ccw.Y + Fwd.Y * behind, Z + ccw.Z + Fwd.Z * behind );
					
					// Rear
					glTexCoord2i( 0.5, 0. );
					glVertex3d( X + Fwd.X * (behind - ahead), Y + Fwd.Y * (behind - ahead), Z + Fwd.Z * (behind - ahead) );
					
					// Rear CW
					glTexCoord2d( 0., 0.4 );
					glVertex3d( X + cw.X + Fwd.X * behind, Y + cw.Y + Fwd.Y * behind, Z + cw.Z + Fwd.Z * behind );
					
				glEnd();
			}
			else
			{
				glBegin( GL_TRIANGLES );
					
					// Counter-clockwise
					glTexCoord2i( 1, 0 );
					glVertex3d( X + ccw.X, Y + ccw.Y, Z + ccw.Z );
					
					// Rear
					glTexCoord2i( 0, 0 );
					glVertex3d( X + Fwd.X * behind, Y + Fwd.Y * behind, Z + Fwd.Z * behind );
					
					// Clockwise
					glTexCoord2i( 0, 1 );
					glVertex3d( X + cw.X, Y + cw.Y, Z + cw.Z );
					
				glEnd();
			}
		}
		
		glBegin( GL_QUADS );
			
			// Outside
			glTexCoord2i( 0, 0 );
			glVertex3d( X + out.X, Y + out.Y, Z + out.Z );
			
			// Clockwise
			glTexCoord2i( 0, 1 );
			glVertex3d( X + cw.X, Y + cw.Y, Z + cw.Z );
			
			// Inside
			glTexCoord2i( 1, 1 );
			glVertex3d( X + in.X, Y + in.Y, Z + in.Z );
			
			// Counter-clockwise
			glTexCoord2i( 1, 0 );
			glVertex3d( X + ccw.X, Y + ccw.Y, Z + ccw.Z );
			
		glEnd();
		
		if( ! going_away )
		{
			glBegin( GL_TRIANGLES );
				
				// Clockwise
				glTexCoord2i( 0, 1 );
				glVertex3d( X + cw.X, Y + cw.Y, Z + cw.Z );
				
				// Front
				glTexCoord2i( 1, 1 );
				glVertex3d( X + Fwd.X * ahead, Y + Fwd.Y * ahead, Z + Fwd.Z * ahead );
				
				// Counter-clockwise
				glTexCoord2i( 1, 0 );
				glVertex3d( X + ccw.X, Y + ccw.Y, Z + ccw.Z );
			
			glEnd();
		}
		else
		{
			if( ShotType == TYPE_SUPERLASER )
			{
				glBegin( GL_QUADS );
					
					// Rear CW
					glTexCoord2d( 0., 0.4 );
					glVertex3d( X + cw.X + Fwd.X * behind, Y + cw.Y + Fwd.Y * behind, Z + cw.Z + Fwd.Z * behind );
					
					// Clockwise
					glTexCoord2i( 0., 0.6 );
					glVertex3d( X + cw.X, Y + cw.Y, Z + cw.Z );
					
					// Counter-clockwise
					glTexCoord2i( 1., 0.6 );
					glVertex3d( X + ccw.X, Y + ccw.Y, Z + ccw.Z );
					
					// Rear CCW
					glTexCoord2d( 1., 0.4 );
					glVertex3d( X + ccw.X + Fwd.X * behind, Y + ccw.Y + Fwd.Y * behind, Z + ccw.Z + Fwd.Z * behind );
					
				if( Raptor::Game->Cam.Fwd.Dot(&Fwd) > 0.9 )
				{
						// Rear Outside
						glTexCoord2i( 0, 0 );
						glVertex3d( X + out.X + Fwd.X * behind, Y + out.Y + Fwd.Y * behind, Z + out.Z + Fwd.Z * behind );
						
						// Rear Clockwise
						glTexCoord2i( 0, 1 );
						glVertex3d( X + cw.X + Fwd.X * behind, Y + cw.Y + Fwd.Y * behind, Z + cw.Z + Fwd.Z * behind );
						
						// Rear Inside
						glTexCoord2i( 1, 1 );
						glVertex3d( X + in.X + Fwd.X * behind, Y + in.Y + Fwd.Y * behind, Z + in.Z + Fwd.Z * behind );
						
						// Rear Counter-clockwise
						glTexCoord2i( 1, 0 );
						glVertex3d( X + ccw.X + Fwd.X * behind, Y + ccw.Y + Fwd.Y * behind, Z + ccw.Z + Fwd.Z * behind );
						
					glEnd(); // GL_QUADS
				}
				else
				{
					glEnd(); // GL_QUADS
					glBegin( GL_TRIANGLES );
						
						// Rear CCW
						glTexCoord2d( 1., 0.4 );
						glVertex3d( X + ccw.X + Fwd.X * behind, Y + ccw.Y + Fwd.Y * behind, Z + ccw.Z + Fwd.Z * behind );
						
						// Rear
						glTexCoord2i( 0.5, 0. );
						glVertex3d( X + Fwd.X * (behind - ahead), Y + Fwd.Y * (behind - ahead), Z + Fwd.Z * (behind - ahead) );
						
						// Rear CW
						glTexCoord2d( 0., 0.4 );
						glVertex3d( X + cw.X + Fwd.X * behind, Y + cw.Y + Fwd.Y * behind, Z + cw.Z + Fwd.Z * behind );
						
					glEnd();
				}
			}
			else
			{
				glBegin( GL_TRIANGLES );
					
					// Counter-clockwise
					glTexCoord2i( 1, 0 );
					glVertex3d( X + ccw.X, Y + ccw.Y, Z + ccw.Z );
					
					// Rear
					glTexCoord2i( 0, 0 );
					glVertex3d( X + Fwd.X * behind, Y + Fwd.Y * behind, Z + Fwd.Z * behind );
					
					// Clockwise
					glTexCoord2i( 0, 1 );
					glVertex3d( X + cw.X, Y + cw.Y, Z + cw.Z );
					
				glEnd();
			}
		}
		
		glDisable( GL_TEXTURE_2D );
	}
	
	Drawn = true;
}


double Shot::DrawAhead( void ) const
{
	if( (ShotType == TYPE_TURBO_LASER_GREEN) || (ShotType == TYPE_TURBO_LASER_RED) )
		return 25.;
	else if( ShotType == TYPE_ION_CANNON )
		return 20.;
	else if( ShotType == TYPE_TORPEDO )
		return 3.;
	else if( ShotType == TYPE_MISSILE )
		return 1.5;
	else if( ShotType == TYPE_SUPERLASER )
		return 20.;
	
	return 15.;
}


double Shot::DrawBehind( void ) const
{
	double behind = 30.;
	
	if( (ShotType == TYPE_TURBO_LASER_GREEN) || (ShotType == TYPE_TURBO_LASER_RED) )
		behind = 50.;
	else if( ShotType == TYPE_ION_CANNON )
		behind = 40.;
	else if( ShotType == TYPE_TORPEDO )
		behind = 25.;
	else if( ShotType == TYPE_MISSILE )
		behind = 20.;
	else if( ShotType == TYPE_SUPERLASER )
		behind = Speed() * 1.5;
	
	double lifetime = Drawn ? Lifetime.ElapsedSeconds() : 0.;
	double moved = lifetime * Speed() + DrawWidth();
	if( moved < behind )
		behind = moved;
	
	return behind;
}


double Shot::DrawWidth( void ) const
{
	if( ShotType == TYPE_TORPEDO )
		return 1.25;
	else if( ShotType == TYPE_MISSILE )
		return 1.125;
	
	double lifetime = Drawn ? Lifetime.ElapsedSeconds() : 0.;
	
	if( (ShotType == TYPE_TURBO_LASER_GREEN) || (ShotType == TYPE_TURBO_LASER_RED) )
		return std::max<double>( 2. - lifetime, 1.5 );
	else if( ShotType == TYPE_LASER_GREEN )
		return std::min<double>( 0.5 + lifetime, 1. );
	else if( ShotType == TYPE_ION_CANNON )
		return std::min<double>( 0.5 + lifetime, 1.5 );
	else if( ShotType == TYPE_QUAD_LASER_RED )
		return std::max<double>( 1.25 - lifetime, 0.875 );
	else if( ShotType == TYPE_SUPERLASER )
		return std::min<double>( 10. + lifetime * 30., 40. );
	
	return std::max<double>( 1.125 - lifetime, 1. );
}
