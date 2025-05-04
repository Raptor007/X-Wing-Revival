/*
 *  DeathStar.h
 */

#pragma once
class DeathStar;

#include "PlatformSpecific.h"

#include "GameObject.h"
#include "Model.h"


class DeathStar : public GameObject
{
public:
	double TrenchWidth, TrenchDepth, TextureSize, DetailHeight, SurfaceDetailHeight;
	Animation Texture, BumpMap;
	Color Ambient, Diffuse, Specular, BottomAmbient, BottomDiffuse, BottomSpecular;
	float Shininess, BottomShininess, BumpScale, BottomBumpScale;
	Model DetailBottom, DetailSide, DetailSurface;
	
	
	DeathStar( uint32_t id = 0 );
	virtual ~DeathStar();
	
	void ClientInit( void );
	
	bool PlayerShouldUpdateServer( void ) const;
	bool ServerShouldUpdatePlayer( void ) const;
	bool ServerShouldUpdateOthers( void ) const;
	bool CanCollideWithOwnType( void ) const;
	bool CanCollideWithOtherTypes( void ) const;
	bool IsMoving( void ) const;
	
	void AddToInitPacket( Packet *packet, int8_t precision = 0 );
	void ReadFromInitPacket( Packet *packet, int8_t precision = 0 );
	
	bool WillCollide( const GameObject *other, double dt, std::string *this_object = NULL, std::string *other_object = NULL, Pos3D *loc = NULL, double *when = NULL ) const;
	void Update( double dt );
	
	void Draw( void );
	
	bool WithinTrenchH( const Pos3D *pos ) const;
	bool WithinTrenchW( const Pos3D *pos ) const;
};
