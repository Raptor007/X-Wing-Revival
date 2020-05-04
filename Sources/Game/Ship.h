/*
 *  Ship.h
 */

#pragma once
class Ship;

#include "PlatformSpecific.h"

#include "GameObject.h"
#include <string>
#include <map>
#include "ShipClass.h"
#include "Model.h"
#include "Shot.h"


class Ship : public GameObject
{
public:
	const ShipClass *Class;
	uint32_t Team;
	Model Shape;
	std::string Name;
	bool CanRespawn;
	uint8_t Group;
	bool IsMissionObjective;
	
	bool SpecialUpdate;
	
	double Health;
	Clock HitClock;
	Vec3D CockpitOffset;
	Clock DeathClock;
	std::map<std::string,double> Subsystems;
	double CollisionPotential;
	double ShieldF, ShieldR;
	uint8_t ShieldPos;
	
	bool Firing;
	uint32_t SelectedWeapon;
	std::map<uint32_t,Clock> FiringClocks;
	std::map<uint32_t,int8_t> Ammo;
	uint8_t FiringMode;
	uint8_t WeaponIndex;
	uint8_t FiredThisFrame;
	
	uint32_t Target;
	float TargetLock;
	
	
	Ship( uint32_t id = 0 );
	Ship( const ShipClass *ship_class );
	virtual ~Ship();
	
	void Clear( void );
	void ClientInit( void );
	bool SetClass( uint32_t ship_class_id );
	void SetClass( const ShipClass *ship_class );
	void Reset( void );
	
	void SetHealth( double health );
	void AddDamage( double front, double rear, const char *subsystem = NULL );
	void KnockCockpit( const Vec3D *dir, double force );
	
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
	double MaxRollChange( void ) const;
	double MaxPitchChange( void ) const;
	double MaxYawChange( void ) const;
	double MaxHealth( void ) const;
	double MaxShield( void ) const;
	double ShieldRechargeDelay( void ) const;
	double ShieldRechargeRate( void ) const;
	double ExplosionRate( void ) const;
	double PiecesDangerousTime( void ) const;
	int WeaponCount( int weapon_type ) const;
	uint8_t Category( void ) const;
	bool PlayersCanFly( void ) const;
	
	Pos3D HeadPos( void ) const;
	double Exploded( void ) const;
	const char *FlybySound( double speed ) const;
	
	std::map<int,Shot*> NextShots( GameObject *target = NULL ) const;
	std::map<int,Shot*> AllShots( GameObject *target = NULL );
	void JustFired( void );
	void JustFired( uint32_t weapon, uint8_t mode );
	bool NextWeapon( void );
	bool NextFiringMode( void );
	double ShotDelay( void ) const;
	float LockingOn( const GameObject *target ) const;
	void UpdateTarget( const GameObject *target, double dt = 0. );
	
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
		TYPE_AWING = 'A/W ',
		TYPE_BWING = 'B/W ',
		TYPE_TIE_FIGHTER = 'T/F ',
		TYPE_TIE_ADVANCED = 'T/A ',
		TYPE_TIE_INTERCEPTOR = 'T/I ',
		TYPE_TIE_BOMBER = 'T/B ',
		TYPE_YT1300 = 'YT13',
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
