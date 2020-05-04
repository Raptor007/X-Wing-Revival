/*
 *  ShipClass.h
 */

#pragma once
class ShipClass;

#include "PlatformSpecific.h"

#include <string>
#include <map>
#include <vector>
#include "GameObject.h"
#include "Model.h"


class ShipClassTurret;


class ShipClass : public GameObject
{
public:
	std::string ShortName;
	std::string LongName;
	std::string Squadron;
	uint8_t Category;
	uint32_t Team;
	double Radius;
	double CollisionDamage;
	double MaxSpeed;
	double Acceleration;
	double MaxRoll;
	double MaxPitch;
	double MaxYaw;
	double MaxHealth;
	double MaxShield;
	double ShieldRechargeDelay;
	double ShieldRechargeRate;
	double ExplosionRate;
	std::map< std::string, double > Subsystems;
	std::map< uint32_t, std::vector<Pos3D> > Weapons;
	std::map< uint32_t, double > FireTime;
	std::map< uint32_t, int8_t > Ammo;
	std::vector<ShipClassTurret> Turrets;
	std::string CollisionModel;
	std::string ExternalModel;
	std::string CockpitModel;
	Vec3D CockpitPos;
	double ModelScale;
	std::map< double, std::string > FlybySounds;
	
	ShipClass( uint32_t id = 0 );
	ShipClass( const ShipClass &other );
	virtual ~ShipClass();
	
	bool Load( const std::string &filename );
	
	void AddToInitPacket( Packet *packet, int8_t precision = 0 );
	void ReadFromInitPacket( Packet *packet, int8_t precision = 0 );
	
	bool ServerShouldUpdateOthers( void ) const;
	bool IsMoving( void ) const;
	void Update( double dt );
	
	bool PlayersCanFly( void ) const;
	
	bool operator < ( const ShipClass &other ) const;
	
	enum
	{
		CATEGORY_FIGHTER = 'F',
		CATEGORY_BOMBER = 'B',
		CATEGORY_CAPITAL = 'C',
		CATEGORY_TARGET = 'T'
	};
};


class ShipClassTurret : public Pos3D
{
public:
	bool Visible;
	bool ParentControl;
	uint32_t Weapon;
	uint8_t FiringMode;
	double SingleShotDelay;
	double TargetArc;
	double MinGunPitch;
	double MaxGunPitch;
	
	ShipClassTurret( double fwd, double up, double right, uint32_t weapon );
};
