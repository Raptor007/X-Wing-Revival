/*
 *  Checkpoint.cpp
 */

#include "Checkpoint.h"

#include <cmath>
#include "XWingDefs.h"
#include "XWingGame.h"
#include "Ship.h"
#include "Num.h"


Checkpoint::Checkpoint( uint32_t id ) : GameObject( id, XWing::Object::CHECKPOINT )
{
	Radius = 100.;
	Prev = Next = 0;
}


Checkpoint::Checkpoint( double x, double y, double z ) : GameObject( 0, XWing::Object::CHECKPOINT )
{
	Radius = 100.;
	Prev = Next = 0;
	X = x;
	Y = y;
	Z = z;
}


Checkpoint::~Checkpoint()
{
}


void Checkpoint::SetNext( Checkpoint *next )
{
	if( next && (next != this) )
	{
		Next = next->ID;
		next->Prev = ID;
		
		if( Dist(next) )
		{
			Fwd = (*next - *this).Unit();
			FixVectors();
		}
	}
	else
		Prev = Next = 0;
}


bool Checkpoint::PlayerShouldUpdateServer( void ) const
{
	return false;
}

bool Checkpoint::ServerShouldUpdatePlayer( void ) const
{
	return false;
}

bool Checkpoint::ServerShouldUpdateOthers( void ) const
{
	return false;
}

bool Checkpoint::CanCollideWithOwnType( void ) const
{
	return false;
}

bool Checkpoint::CanCollideWithOtherTypes( void ) const
{
	return true;
}

bool Checkpoint::IsMoving( void ) const
{
	return false;
}


void Checkpoint::AddToInitPacket( Packet *packet, int8_t precision )
{
	packet->AddDouble( X );
	packet->AddDouble( Y );
	packet->AddDouble( Z );
	packet->AddFloat( Radius );
	/*
	packet->AddFloat( Fwd.X );
	packet->AddFloat( Fwd.Y );
	packet->AddFloat( Fwd.Z );
	packet->AddUInt( Next );
	packet->AddUInt( Prev );
	*/
}


void Checkpoint::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	X = packet->NextDouble();
	Y = packet->NextDouble();
	Z = packet->NextDouble();
	Radius = packet->NextFloat();
	/*
	Fwd.X = packet->NextFloat();
	Fwd.Y = packet->NextFloat();
	Fwd.Z = packet->NextFloat();
	FixVectors();
	Next = packet->NextUInt();
	Prev = packet->NextUInt();
	*/
}


bool Checkpoint::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object, Pos3D *loc, double *when ) const
{
	if( other->Type() == XWing::Object::SHIP )
		return ((const Ship*)other)->WillCollideWithSphere( this, Radius, dt, other_object, loc, when );
	
	return false;
}


void Checkpoint::Draw( void )
{
}
