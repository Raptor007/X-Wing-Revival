/*
 *  Turret.h
 */

#pragma once
class Turret;

#include "PlatformSpecific.h"

#include <string>
#include <map>
#include "GameObject.h"
#include "Model.h"
#include "Shot.h"


class Turret : public GameObject
{
public:
	uint32_t ParentID;
	Vec3D Offset;
	Vec3D RelativeUp;
	bool ParentControl;
	double MinGunPitch, MaxGunPitch;
	uint32_t Team;
	Model *BodyShape, *GunShape;
	double GunPitch, GunPitchRate;
	
	double Health;
	
	bool Firing;
	Clock FiringClock;
	uint32_t Weapon;
	int WeaponIndex;
	int FiringMode;
	double SingleShotDelay;
	double MaxFiringDist;
	float AimAhead;
	Vec3D TargetDir;
	double TargetArc;
	double SafetyDistance;
	
	uint32_t Target;
	
	
	Turret( uint32_t id = 0 );
	virtual ~Turret();
	
	void ClientInit( void );
	void Attach( const GameObject *parent, const Vec3D *offset = NULL, const Vec3D *relative_up = NULL, bool parent_control = false );
	void UpdatePos( const GameObject *parent );
	void Reset( void );
	void SetHealth( double health );
	void AddDamage( double damage );
	
	void PitchGun( double degrees );
	void SetPitch( double pitch );
	void SetYaw( double yaw );
	Pos3D GunPos( void );
	
	double MaxHealth( void );
	
	std::map<int,Shot*> NextShots( GameObject *target = NULL );
	void JustFired( void );
	double ShotDelay( void ) const;
	
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
	void AddToUpdatePacketFromClient( Packet *packet, int8_t precision = 0 );
	void ReadFromUpdatePacketFromClient( Packet *packet, int8_t precision = 0 );
	
	bool WillCollide( const GameObject *other, double dt, std::string *this_object = NULL, std::string *other_object = NULL ) const;
	void Update( double dt );
	
	void Draw( void );
};
