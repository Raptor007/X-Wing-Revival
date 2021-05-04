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
	uint8_t ShotType;
	Model Shape;
	Animation Anim;
	
	uint32_t FiredFrom;
	uint32_t Seeking;
	
	Shot( uint32_t id = 0 );
	virtual ~Shot();
	
	void ClientInit( void );
	
	double Damage( void ) const;
	double AsteroidDamage( void ) const;
	double Speed( void ) const;
	double TurnRate( void ) const;
	double Intercept( void ) const;
	double MaxLifetime( void ) const;
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
	
	bool WillCollide( const GameObject *other, double dt, std::string *this_object = NULL, std::string *other_object = NULL ) const;
	void Update( double dt );
	
	void Draw( void );
	
	enum
	{
		TYPE_LASER_RED = 1,
		TYPE_LASER_GREEN,
		TYPE_TURBO_LASER_RED,
		TYPE_TURBO_LASER_GREEN,
		TYPE_ION_CANNON,
		TYPE_TORPEDO,
		TYPE_MISSILE
	};
};
