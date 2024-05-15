/*
 *  DockingBay.cpp
 */

#include "DockingBay.h"

#include <cmath>
#include "XWingDefs.h"
#include "XWingGame.h"


DockingBay::DockingBay( uint32_t id ) : GameObject( id, XWing::Object::DOCKING_BAY )
{
	Radius = 35.;
	ParentID = 0;
	Team = XWing::Team::NONE;
}


DockingBay::~DockingBay()
{
}


void DockingBay::Attach( const GameObject *parent, const Vec3D *offset )
{
	if( offset )
		Offset.Copy( offset );
	
	if( parent )
	{
		ParentID = parent->ID;
		UpdatePos( parent );
		if( parent->Type() == XWing::Object::SHIP )
			Team = ((const Ship*)( parent ))->Team;
		else
			Team = XWing::Team::NONE;
	}
	else
	{
		ParentID = 0;
		Team = XWing::Team::NONE;
	}
}


Ship *DockingBay::ParentShip( void ) const
{
	GameObject *parent = (Data && ParentID) ? Data->GetObject( ParentID ) : NULL;
	return (parent && (parent->Type() == XWing::Object::SHIP)) ? ((Ship*) parent) : NULL;
}


void DockingBay::UpdatePos( const GameObject *parent )
{
	if( ParentID && ! parent )
		parent = Data->GetObject( ParentID );
	if( ! parent )
		return;
	
	X = parent->X;
	Y = parent->Y;
	Z = parent->Z;
	MoveAlong( &(parent->Fwd),   Offset.X );
	MoveAlong( &(parent->Up),    Offset.Y );
	MoveAlong( &(parent->Right), Offset.Z );
	MotionVector.Copy( &(parent->MotionVector) );
	
	// Stop functioning (and indicate ready for removal) when parent ship dies.
	if( (parent->Type() == XWing::Object::SHIP) && (((const Ship*)( parent ))->Health <= 0.) )
		Radius = 0.;
}


bool DockingBay::ServerShouldSend( void ) const
{
	return false;
}


bool DockingBay::CanCollideWithOwnType( void ) const
{
	return false;
}


bool DockingBay::CanCollideWithOtherTypes( void ) const
{
	const Ship *parent = ParentShip();
	if( parent && ((parent->JumpProgress < 1.) || parent->JumpedOut) )
		return false;
	
	return true;
}


bool DockingBay::IsMoving( void ) const
{
	return true;
}


bool DockingBay::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object, Pos3D *loc, double *when ) const
{
	// Can't repair the ship we're attached to.
	if( other->ID == ParentID )
		return false;
	
	if( other->Type() == XWing::Object::SHIP )
	{
		Ship *ship = (Ship*) other;
		
		// Can't undo a kaboom.
		if( ship->Health <= 0. )
			return false;
		
		// Cannot repair capital ships.
		if( ship->ComplexCollisionDetection() )
			return false;
		
		// Cannot repair a ship still jumping in.
		if( ship->JumpProgress < 1. )
			return false;
		
		// Do not repair enemies.
		if( Team && (ship->Team != Team) )
			return false;
		
		// We only care if the ship origin is within our repair radius and the ship is moving with us.
		if( (ship->Dist(this) <= Radius) && ((ship->MotionVector - MotionVector).Length() < 50.) )
			return true;
	}
	
	return false;
}


void DockingBay::Update( double dt )
{
	UpdatePos();
	RepairClock.SetTimeScale( Data->TimeScale );
	RearmClock.SetTimeScale( Data->TimeScale );
}
