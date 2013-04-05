/*
 *  Ship.h
 */

#pragma once
class Ship;

#include "platforms.h"

#include <string>
#include <map>
#include "GameObject.h"
#include "Model.h"
#include "Shot.h"


class Ship : public GameObject
{
public:
	uint32_t Team;
	uint32_t ShipType;
	Model Shape;
	std::string Name;
	bool CanRespawn;
	
	bool SpecialUpdate;
	
	double Health;
	Clock HitClock;
	Clock DeathClock;
	
	double ShieldF, ShieldR;
	uint8_t ShieldPos;
	
	bool Firing;
	uint32_t SelectedWeapon;
	std::map<uint32_t,Clock> FiringClocks;
	std::map<uint32_t,int8_t> Ammo;
	uint8_t FiringMode;
	uint8_t WeaponIndex;
	
	uint32_t Target;
	
	
	Ship( uint32_t id = 0 );
	virtual ~Ship();
	
	void SetType( uint32_t ship_type );
	void Reset( void );
	void SetHealth( double health );
	void AddDamage( double front, double rear );
	void Explode( double dt );
	
	void SetRoll( double roll );
	void SetPitch( double pitch );
	void SetYaw( double yaw );
	void SetThrottle( double throttle, double dt );
	void SetShieldPos( uint8_t pos );
	
	double Radius( void ) const;
	double MaxSpeed( void ) const;
	double Acceleration( void ) const;
	double MaxRoll( void ) const;
	double MaxPitch( void ) const;
	double MaxYaw( void ) const;
	double MaxHealth( void ) const;
	double MaxShield( void ) const;
	double ShieldRechargeDelay( void ) const;
	double ShieldRechargeRate( void ) const;
	bool PlayersCanFly( void ) const;
	
	std::map<int,Shot*> NextShots( GameObject *target = NULL ) const;
	std::map<int,Shot*> AllShots( GameObject *target = NULL );
	void JustFired( void );
	void NextWeapon( void );
	void NextFiringMode( void );
	double ShotDelay( void ) const;
	
	bool PlayerShouldUpdateServer( void ) const;
	bool ServerShouldUpdatePlayer( void ) const;
	bool ServerShouldUpdateOthers( void ) const;
	bool CanCollideWithOwnType( void ) const;
	bool CanCollideWithOtherTypes( void ) const;
	
	void AddToInitPacket( Packet *packet, int8_t precision = 0 );
	void ReadFromInitPacket( Packet *packet, int8_t precision = 0 );
	void AddToUpdatePacketFromServer( Packet *packet, int8_t precision = 0 );
	void ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision = 0 );
	void AddToUpdatePacketFromClient( Packet *packet, int8_t precision = 0 );
	void ReadFromUpdatePacketFromClient( Packet *packet, int8_t precision = 0 );
	
	bool WillCollide( const GameObject *other, double dt ) const;
	void Update( double dt );
	
	void Draw( void );
	
	enum
	{
		TYPE_XWING = 'X/W ',
		TYPE_YWING = 'Y/W ',
		TYPE_TIE_FIGHTER = 'T/F ',
		TYPE_ISD2 = 'ISD2',
		TYPE_EXHAUST_PORT = 'Hole'
	};
	
	enum
	{
		SHIELD_CENTER = 'C',
		SHIELD_FRONT = 'F',
		SHIELD_REAR = 'R'
	};
};
