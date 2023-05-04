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
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("torpedo.ani") );
	else if( ShotType == TYPE_MISSILE )
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("missile.ani") );
	else
		Anim.BecomeInstance( Raptor::Game->Res.GetAnimation("laser_unknown.ani") );
	
	if( Raptor::Game->State >= XWing::State::FLYING )
	{
		if( ShotType == TYPE_LASER_RED )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("laser_red.wav"), X, Y, Z );
		else if( ShotType == TYPE_LASER_GREEN )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("laser_green.wav"), X, Y, Z );
		else if( ShotType == TYPE_TURBO_LASER_RED )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("laser_red.wav"), X, Y, Z, 1.5 );
		else if( ShotType == TYPE_TURBO_LASER_GREEN )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("turbolaser_green.wav"), X, Y, Z, 1.5 );
		else if( ShotType == TYPE_QUAD_LASER_RED )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("laser_turret.wav"), X, Y, Z, 1.125 );
		else if( ShotType == TYPE_TORPEDO )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("torpedo.wav"), X, Y, Z, 0.75 );
		else if( ShotType == TYPE_MISSILE )
			Raptor::Game->Snd.PlayAt( Raptor::Game->Res.GetSound("torpedo.wav"), X, Y, Z, 0.75 );
		
		if( Seeking && (Seeking == ((XWingGame*)( Raptor::Game ))->ObservedShipID) )
		{
			const GameObject *observed_object = Data->GetObject( ((XWingGame*)( Raptor::Game ))->ObservedShipID );
			if( observed_object && (observed_object->PlayerID == Raptor::Game->PlayerID) )
				Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("incoming.wav") );
		}
		
		GameObject *fired_from = Data->GetObject( FiredFrom );
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
		return 21.;
	else if( ShotType == TYPE_TORPEDO )
		return 200.;
	else if( ShotType == TYPE_MISSILE )
		return 95.;
	
	return 30.;
}


double Shot::AsteroidDamage( void ) const
{
	if( ShotType == TYPE_TURBO_LASER_GREEN )
		return 150.;
	else if( ShotType == TYPE_TURBO_LASER_RED )
		return 50.;
	else if( ShotType == TYPE_TORPEDO )
		return 100.;
	else if( ShotType == TYPE_MISSILE )
		return 50.;
	
	return 1.;
}


double Shot::Speed( void ) const
{
	if( ShotType == TYPE_TURBO_LASER_GREEN )
		return 600.;
	else if( ShotType == TYPE_TURBO_LASER_RED )
		return 600.;
	else if( ShotType == TYPE_TORPEDO )
		return 400.;
	else if( ShotType == TYPE_MISSILE )
		return 500.;
	
	return 800.;
}


double Shot::TurnRate( void ) const
{
	if( ShotType == TYPE_TORPEDO )
		return 95.;
	else if( ShotType == TYPE_MISSILE )
		return 120.;
	
	return 0.;
}


double Shot::Intercept( void ) const
{
	if( ShotType == TYPE_TORPEDO )
		return 0.95;
	else if( ShotType == TYPE_MISSILE )
		return 1.;
	
	return 0.;
}


double Shot::MaxLifetime( void ) const
{
	if( ShotType == TYPE_TORPEDO )
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
		return Color( 0.85f, 0.f, 0.f, 15.f );
	else if( ShotType == TYPE_TURBO_LASER_GREEN )
		return Color( 0.f, 0.4f, 0.f, 15.f );
	else if( ShotType == TYPE_QUAD_LASER_RED )
		return Color( 0.8f, 0.f, 0.f, 15.f );
	else if( ShotType == TYPE_TORPEDO )
		return Color( 1.f, 0.7f, 0.4f, 15.f );
	else if( ShotType == TYPE_MISSILE )
		return Color( 0.9f, 0.2f, 0.1f, 15.f );
	
	return Color( 1.f, 1.f, 1.f, 15.f );
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
	// FIXME: This needs to be true to shoot down torpedos and missiles, but it bogs down the server checking laser-on-laser hits.
	return false;
}

bool Shot::CanCollideWithOtherTypes( void ) const
{
	return true;
}


void Shot::AddToInitPacket( Packet *packet, int8_t precision )
{
	GameObject::AddToInitPacket( packet, -127 );
	packet->AddUChar( ShotType );
	packet->AddUInt( FiredFrom );
}


void Shot::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, -127 );
	ShotType = packet->NextUChar();
	FiredFrom = packet->NextUInt();
}


void Shot::AddToUpdatePacket( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacket( packet, -127 );
	packet->AddUInt( Seeking );
}


void Shot::ReadFromUpdatePacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromUpdatePacket( packet, -127 );
	Seeking = packet->NextUInt();
}


bool Shot::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object ) const
{
	if( other->Type() == XWing::Object::SHOT )
	{
		Shot *shot = (Shot*) other;
		if( (shot->FiredFrom != FiredFrom) && ((shot->ShotType == TYPE_TORPEDO) || (ShotType == TYPE_TORPEDO) || (shot->ShotType == TYPE_MISSILE) || (ShotType == TYPE_MISSILE)) )
		{
			double dist = Math3D::MinimumDistance( this, &(this->MotionVector), other, &(other->MotionVector), dt );
			if( dist <= 1. )
				return true;
		}
		
		return false;
	}
	
	// Let other objects determine whether collisions with shots occur.
	return other->WillCollide( this, dt, other_object, this_object );
}


void Shot::Update( double dt )
{
	if( Seeking )
	{
		GameObject *target = Data ? Data->GetObject( Seeking ) : NULL;
		
		if( ! target )
		{
			Seeking = 0;
		}
		else if( target->Type() == XWing::Object::SHIP )
		{
			Ship *ship = (Ship*) target;
			if( ship->Health <= 0. )
			{
				target = NULL;
				Seeking = 0;
			}
		}
		else if( target->Type() == XWing::Object::TURRET )
		{
			Turret *turret = (Turret*) target;
			if( turret->Health <= 0. )
			{
				target = NULL;
				Seeking = 0;
			}
		}
		
		if( target )
		{
			// Point somewhere between the target and intercept point.
			
			Vec3D vec_to_target( target->X - X, target->Y - Y, target->Z - Z );
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
	if( Anim.Frames.size() )
	{
		// Draw a laser.
		
		// Size parameters.
		double ahead = 15.;
		double behind = 30.;
		double width = 1.;
		if( ShotType == TYPE_TURBO_LASER_GREEN )
		{
			ahead = 25.;
			behind = 50.;
			width = 1.5;
		}
		else if( ShotType == TYPE_TORPEDO )
		{
			ahead = 5.;
			behind = 25.;
			width = 1.5;
		}
		double moved = Lifetime.ElapsedSeconds() * MotionVector.Length() + width;
		if( moved < behind )
			behind = moved;
		
		// If the shot is behind us, don't draw it.
		Vec3D vec_to_front( X + Fwd.X * ahead - Raptor::Game->Cam.X, Y + Fwd.Y * ahead - Raptor::Game->Cam.Y, Z + Fwd.Z * ahead - Raptor::Game->Cam.Z );
		Vec3D vec_to_rear( X - Fwd.X * behind - Raptor::Game->Cam.X, Y - Fwd.Y * behind - Raptor::Game->Cam.Y, Z - Fwd.Z * behind - Raptor::Game->Cam.Z );
		if( (Raptor::Game->Cam.Fwd.Dot( &vec_to_front ) < 0.) && (Raptor::Game->Cam.Fwd.Dot( &vec_to_rear ) < 0.) )
			return;
		
		// Calculate corners.
		Vec3D vec_to_shot( X - Raptor::Game->Cam.X, Y - Raptor::Game->Cam.Y, Z - Raptor::Game->Cam.Z );
		vec_to_shot.ScaleTo( 1. );
		Vec2D vec( vec_to_shot.Dot(&(Raptor::Game->Cam.Right)), vec_to_shot.Dot(&(Raptor::Game->Cam.Up)) );
		double rotation = Num::RadToDeg( -1. * atan2( -vec.X, vec.Y ) );
		Vec3D out = Raptor::Game->Cam.Up;
		out.ScaleTo( width );
		out.RotateAround( &(Raptor::Game->Cam.Fwd), rotation );
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
		
		glBegin( GL_TRIANGLES );
			
			if( going_away )
			{
				// Clockwise
				glTexCoord2i( 0, 1 );
				glVertex3d( X + cw.X, Y + cw.Y, Z + cw.Z );
				
				// Front
				glTexCoord2i( 1, 1 );
				glVertex3d( X + Fwd.X * ahead, Y + Fwd.Y * ahead, Z + Fwd.Z * ahead );
				
				// Counter-clockwise
				glTexCoord2i( 1, 0 );
				glVertex3d( X + ccw.X, Y + ccw.Y, Z + ccw.Z );
			}
			else
			{
				// Counter-clockwise
				glTexCoord2i( 1, 0 );
				glVertex3d( X + ccw.X, Y + ccw.Y, Z + ccw.Z );
				
				// Rear
				glTexCoord2i( 0, 0 );
				glVertex3d( X + Fwd.X * behind, Y + Fwd.Y * behind, Z + Fwd.Z * behind );
				
				// Clockwise
				glTexCoord2i( 0, 1 );
				glVertex3d( X + cw.X, Y + cw.Y, Z + cw.Z );
			}
			
		glEnd();
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
		glBegin( GL_TRIANGLES );
			
			if( ! going_away )
			{
				// Clockwise
				glTexCoord2i( 0, 1 );
				glVertex3d( X + cw.X, Y + cw.Y, Z + cw.Z );
				
				// Front
				glTexCoord2i( 1, 1 );
				glVertex3d( X + Fwd.X * ahead, Y + Fwd.Y * ahead, Z + Fwd.Z * ahead );
				
				// Counter-clockwise
				glTexCoord2i( 1, 0 );
				glVertex3d( X + ccw.X, Y + ccw.Y, Z + ccw.Z );
			}
			else
			{
				// Counter-clockwise
				glTexCoord2i( 1, 0 );
				glVertex3d( X + ccw.X, Y + ccw.Y, Z + ccw.Z );
				
				// Rear
				glTexCoord2i( 0, 0 );
				glVertex3d( X + Fwd.X * behind, Y + Fwd.Y * behind, Z + Fwd.Z * behind );
				
				// Clockwise
				glTexCoord2i( 0, 1 );
				glVertex3d( X + cw.X, Y + cw.Y, Z + cw.Z );
			}
			
		glEnd();
		
		glDisable( GL_TEXTURE_2D );
	}
	else
		// Not a laser, so draw the model.
		Shape.DrawAt( this );
}
