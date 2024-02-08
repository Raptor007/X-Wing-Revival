/*
 *  Asteroid.h
 */

#pragma once
class Asteroid;

#include "PlatformSpecific.h"

#include "BlastableObject.h"
#include "Model.h"
#include "Animation.h"
#include "Color.h"


#define ASTEROID_BLASTABLE 1
#if ASTEROID_BLASTABLE
class Asteroid : public BlastableObject
#else
class Asteroid : public GameObject
#endif
{
public:
	double Radius;
	double Health;
	Model *Shape;
	Animation Texture;
	Color Ambient, Diffuse, Specular;
	float Shininess;
	
	Asteroid( uint32_t id = 0 );
	virtual ~Asteroid();
	
	void SetRadius( double radius = 0. );
	
	void ClientInit( void );
	
	bool PlayerShouldUpdateServer( void ) const;
	bool ServerShouldUpdatePlayer( void ) const;
	bool ServerShouldUpdateOthers( void ) const;
	bool CanCollideWithOwnType( void ) const;
	bool CanCollideWithOtherTypes( void ) const;
	bool IsMoving( void ) const;
	
	void AddToInitPacket( Packet *packet, int8_t precision = 0 );
	void ReadFromInitPacket( Packet *packet, int8_t precision = 0 );
	void AddToUpdatePacketFromServer( Packet *packet, int8_t precision = 0 );
	void ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision = 0 );
	
	bool WillCollide( const GameObject *other, double dt, std::string *this_object = NULL, std::string *other_object = NULL ) const;
	
	void Draw( void );
	Shader *WantShader( void ) const;
};
