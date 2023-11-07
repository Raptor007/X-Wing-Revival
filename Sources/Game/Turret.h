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
#include "Ship.h"


class Turret : public GameObject
{
public:
	uint32_t ParentID;
	Vec3D Offset;
	Vec3D RelativeUp, RelativeFwd;
	bool ParentControl;
	double MinGunPitch, MaxGunPitch;
	double YawSpeed, PitchSpeed;
	uint8_t Team;
	bool Visible, CanBeHit;
	double GunWidth;
	double GunUp, GunFwd, HeadUp, HeadFwd;
	std::string BodyModel, GunModel;
	Model *BodyShape, *GunShape;
	
	double GunPitch, GunPitchRate;
	
	double Health;
	
	bool Firing;
	Clock FiringClock;
	uint8_t Weapon;
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
	void Attach( const GameObject *parent, const Vec3D *offset = NULL, const Vec3D *relative_up = NULL, const Vec3D *relative_fwd = NULL, bool parent_control = false );
	Ship *ParentShip( void ) const;
	void UpdatePos( const GameObject *parent = NULL );
	void Reset( void );
	void SetHealth( double health );
	void AddDamage( double damage );
	
	void PitchGun( double degrees );
	void SetPitch( double pitch );
	void SetYaw( double yaw );
	Pos3D GunPos( const Pos3D *body_pos = NULL ) const;
	Pos3D HeadPos( void ) const;
	
	std::map<int,Shot*> NextShots( GameObject *target = NULL, uint8_t firing_mode = 0 ) const;
	std::map<int,Shot*> AllShots( GameObject *target = NULL ) const;
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
	void Draw( bool allow_shader_change );
	void DrawWireframe( Color *color = NULL, double scale = 0.022 );
};
