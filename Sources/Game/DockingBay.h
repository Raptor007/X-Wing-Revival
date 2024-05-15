/*
 *  DockingBay.h
 */

#pragma once
class DockingBay;

#include "PlatformSpecific.h"

#include "GameObject.h"
#include "Ship.h"


class DockingBay : public GameObject
{
public:
	double Radius;
	uint8_t Team;
	uint32_t ParentID;
	Vec3D Offset;
	Clock RepairClock, RearmClock;
	
	DockingBay( uint32_t id = 0 );
	virtual ~DockingBay();
	
	void Attach( const GameObject *parent, const Vec3D *offset );
	Ship *ParentShip( void ) const;
	void UpdatePos( const GameObject *parent = NULL );
	
	bool ServerShouldSend( void ) const;
	bool CanCollideWithOwnType( void ) const;
	bool CanCollideWithOtherTypes( void ) const;
	bool IsMoving( void ) const;
	
	bool WillCollide( const GameObject *other, double dt, std::string *this_object = NULL, std::string *other_object = NULL, Pos3D *loc = NULL, double *when = NULL ) const;
	void Update( double dt );
};
