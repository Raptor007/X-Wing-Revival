/*
 *  DeathStarBox.h
 */

#pragma once
class DeathStarBox;

#include "platforms.h"

#include <string>
#include <map>
#include "GameObject.h"
#include "Model.h"
#include "Shot.h"


class DeathStarBox : public GameObject
{
public:
	double L,H,W;
	Model *Shape;
	
	
	DeathStarBox( uint32_t id = 0 );
	virtual ~DeathStarBox();
	
	void ClientInit( void );
	
	bool PlayerShouldUpdateServer( void ) const;
	bool ServerShouldUpdatePlayer( void ) const;
	bool ServerShouldUpdateOthers( void ) const;
	bool CanCollideWithOwnType( void ) const;
	bool CanCollideWithOtherTypes( void ) const;
	bool IsMoving( void ) const;
	
	void AddToInitPacket( Packet *packet, int8_t precision = 0 );
	void ReadFromInitPacket( Packet *packet, int8_t precision = 0 );
	
	bool WillCollide( const GameObject *other, double dt ) const;
	void Update( double dt );
	
	void Draw( void );
};
