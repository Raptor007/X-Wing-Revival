/*
 *  DeathStarBox.cpp
 */

#include "DeathStarBox.h"

#include <cmath>
#include "XWingDefs.h"
#include "RaptorGame.h"
#include "Ship.h"


DeathStarBox::DeathStarBox( uint32_t id ) : GameObject( id, XWing::Object::DEATH_STAR_BOX )
{
	L = 1.;
	H = 1.;
	W = 1.;
	
	Shape = NULL;
}


DeathStarBox::~DeathStarBox()
{
}


void DeathStarBox::ClientInit( void )
{
	Shape = Raptor::Game->Res.GetModel("deathstar_box.obj");
}


bool DeathStarBox::PlayerShouldUpdateServer( void ) const
{
	return false;
}

bool DeathStarBox::ServerShouldUpdatePlayer( void ) const
{
	return false;
}

bool DeathStarBox::ServerShouldUpdateOthers( void ) const
{
	return false;
}

bool DeathStarBox::CanCollideWithOwnType( void ) const
{
	return false;
}

bool DeathStarBox::CanCollideWithOtherTypes( void ) const
{
	return true;
}

bool DeathStarBox::IsMoving( void ) const
{
	return false;
}


void DeathStarBox::AddToInitPacket( Packet *packet, int8_t precision )
{
	GameObject::AddToInitPacket( packet, -127 );
	packet->AddFloat( L );
	packet->AddFloat( H );
	packet->AddFloat( W );
}


void DeathStarBox::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, -127 );
	L = packet->NextFloat();
	H = packet->NextFloat();
	W = packet->NextFloat();
}


bool DeathStarBox::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object ) const
{
	if( other->Type() == XWing::Object::SHOT )
	{
		if( (fabs(other->DistAlong(&Fwd,this)) <= L/2.) && (fabs(other->DistAlong(&Up,this)) <= H/2.) && (fabs(other->DistAlong(&Right,this)) <= W/2.) )
			return true;
	}
	
	else if( other->Type() == XWing::Object::SHIP )
	{
		// We don't care about dead ships hitting a box.
		Ship *ship = (Ship*) other;
		if( ship->Health <= 0. )
			return false;
		
		// Don't destroy the exhaust port.
		if( ship->ShipType == Ship::TYPE_EXHAUST_PORT )
			return false;
		
		// Don't worry about capital ship hitting these.
		if( ship->ComplexCollisionDetection() )
			return false;
		
		if( (fabs(other->DistAlong(&Fwd,this)) <= L/2. + ship->Radius()) && (fabs(other->DistAlong(&Up,this)) <= H/2. + ship->Radius()) && (fabs(other->DistAlong(&Right,this)) <= W/2. + ship->Radius()) )
			return true;
	}
	
	return false;
}


void DeathStarBox::Update( double dt )
{
}


void DeathStarBox::Draw( void )
{
	if( Shape )
		Shape->DrawAt( this, 1., L, H, W );
}
