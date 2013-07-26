/*
 *  Shot.h
 */

#pragma once
class Shot;

#include "PlatformSpecific.h"

#include "GameObject.h"
#include "Model.h"
#include "Color.h"


class Shot : public GameObject
{
public:
	uint32_t ShotType;
	Model Shape;
	Animation Anim;
	
	uint32_t FiredFrom;
	uint32_t Seeking;
	
	Shot( uint32_t id = 0 );
	virtual ~Shot();
	
	void SetType( uint32_t shot_type );
	
	double Damage( void ) const;
	double Speed( void ) const;
	double TurnRate( void ) const;
	Color LightColor( void ) const;
	
	bool PlayerShouldUpdateServer( void ) const;
	bool ServerShouldUpdatePlayer( void ) const;
	bool ServerShouldUpdateOthers( void ) const;
	bool CanCollideWithOwnType( void ) const;
	bool CanCollideWithOtherTypes( void ) const;
	
	void AddToInitPacket( Packet *packet, int8_t precision = 0 );
	void ReadFromInitPacket( Packet *packet, int8_t precision = 0 );
	void AddToUpdatePacket( Packet *packet, int8_t precision = 0 );
	void ReadFromUpdatePacket( Packet *packet, int8_t precision = 0 );
	
	bool WillCollide( const GameObject *other, double dt ) const;
	void Update( double dt );
	
	void Draw( void );
	
	enum
	{
		TYPE_LASER_RED = 'LRed',
		TYPE_LASER_GREEN = 'LGrn',
		TYPE_TURBO_LASER_GREEN = 'TLGr',
		TYPE_ION_CANNON = 'IonC',
		TYPE_TORPEDO = 'Torp',
		TYPE_MISSILE = 'Misl',
		TYPE_MINE = 'Mine'
	};
};
