/*
 *  Ship.h
 */

#pragma once
class Ship;

#include "PlatformSpecific.h"

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
	uint8_t Group;
	bool IsMissionObjective;
	
	bool SpecialUpdate;
	
	double Health;
	Clock HitClock;
	Vec3D HitDir;
	Clock DeathClock;
	std::map<std::string,double> Subsystems;
	
	double ShieldF, ShieldR;
	uint8_t ShieldPos;
	
	double CollisionPotential;
	
	bool Firing;
	uint32_t SelectedWeapon;
	std::map<uint32_t,Clock> FiringClocks;
	std::map<uint32_t,int8_t> Ammo;
	uint8_t FiringMode;
	uint8_t WeaponIndex;
	uint8_t FiredThisFrame;
	
	uint32_t Target;
	
	
	Ship( uint32_t id = 0 );
	virtual ~Ship();
	
	void SetType( uint32_t ship_type );
	void Reset( void );
	void SetHealth( double health );
	void AddDamage( double front, double rear, const char *subsystem = NULL );
	void Explode( double dt );
	
	void SetRoll( double roll, double dt );
	void SetPitch( double pitch, double dt );
	void SetYaw( double yaw, double dt );
	double GetThrottle( void ) const;
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
	double PiecesDangerousTime( void ) const;
	int WeaponCount( int weapon_type ) const;
	bool PlayersCanFly( void ) const;
	
	std::map<int,Shot*> NextShots( GameObject *target = NULL ) const;
	std::map<int,Shot*> AllShots( GameObject *target = NULL );
	void JustFired( void );
	void JustFired( uint32_t weapon, uint8_t mode );
	void NextWeapon( void );
	void NextFiringMode( void );
	double ShotDelay( void ) const;
	
	bool PlayerShouldUpdateServer( void ) const;
	bool ServerShouldUpdatePlayer( void ) const;
	bool ServerShouldUpdateOthers( void ) const;
	bool CanCollideWithOwnType( void ) const;
	bool CanCollideWithOtherTypes( void ) const;
	bool ComplexCollisionDetection( void ) const;
	
	void AddToInitPacket( Packet *packet, int8_t precision = 0 );
	void ReadFromInitPacket( Packet *packet, int8_t precision = 0 );
	void AddToUpdatePacketFromServer( Packet *packet, int8_t precision = 0 );
	void ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision = 0 );
	void AddToUpdatePacketFromClient( Packet *packet, int8_t precision = 0 );
	void ReadFromUpdatePacketFromClient( Packet *packet, int8_t precision = 0 );
	
	bool WillCollide( const GameObject *other, double dt, std::string *this_object = NULL, std::string *other_object = NULL ) const;
	void Update( double dt );
	
	void Draw( void );
	void DrawWireframe( void );
	
	enum
	{
		TYPE_XWING = 'X/W ',
		TYPE_YWING = 'Y/W ',
		TYPE_TIE_FIGHTER = 'T/F ',
		TYPE_ISD2 = 'ISD2',
		TYPE_CORVETTE = 'CRV ',
		TYPE_NEBULON_B = 'FRG ',
		TYPE_CALAMARI_CRUISER = 'CRS ',
		TYPE_EXHAUST_PORT = 'Hole'
	};
	
	enum
	{
		SHIELD_CENTER = 'C',
		SHIELD_FRONT = 'F',
		SHIELD_REAR = 'R'
	};
};
