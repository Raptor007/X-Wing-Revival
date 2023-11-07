/*
 *  Checkpoint.h
 */

#pragma once
class Checkpoint;

#include "PlatformSpecific.h"

#include "GameObject.h"
#include "Model.h"
#include "Animation.h"
#include "Color.h"


class Checkpoint : public GameObject
{
public:
	double Radius;
	uint32_t Prev, Next;
	
	Checkpoint( uint32_t id = 0 );
	Checkpoint( double x, double y, double z );
	virtual ~Checkpoint();
	
	void SetNext( Checkpoint *next );
	
	bool PlayerShouldUpdateServer( void ) const;
	bool ServerShouldUpdatePlayer( void ) const;
	bool ServerShouldUpdateOthers( void ) const;
	bool CanCollideWithOwnType( void ) const;
	bool CanCollideWithOtherTypes( void ) const;
	bool IsMoving( void ) const;
	
	void AddToInitPacket( Packet *packet, int8_t precision = 0 );
	void ReadFromInitPacket( Packet *packet, int8_t precision = 0 );
	
	bool WillCollide( const GameObject *other, double dt, std::string *this_object = NULL, std::string *other_object = NULL ) const;
	
	void Draw( void );
};
