/*
 *  ShipClass.h
 */

#pragma once
class ShipClass;
class ShipClassTurret;
class ShipClassDockingBay;
class ShipClassEngine;

#include "PlatformSpecific.h"

#include <string>
#include <map>
#include <vector>
#include "GameObject.h"
#include "Model.h"
#include "Color.h"


class ShipClassTurret;


class ShipClass : public GameObject
{
public:
	std::string ShortName;
	std::string LongName;
	std::string Squadron;
	uint8_t Category;
	uint8_t Team;
	double Radius;
	double CollisionDamage;
	double MaxSpeed;
	double Acceleration;
	double RollSlow, RollFast, RollExponent;
	double PitchSlow, PitchFast, PitchExponent;
	double YawSlow, YawFast, YawExponent;
	double RollChangeSlow, RollChangeFast, RollChangeExponent;
	double PitchChangeSlow, PitchChangeFast, PitchChangeExponent;
	double YawChangeSlow, YawChangeFast, YawChangeExponent;
	double MaxHealth;
	double MaxShield;
	double ShieldRechargeDelay;
	double ShieldRechargeRate;
	double ExplosionRate;
	std::map< std::string, double > Subsystems;
	//std::vector<Vec3D> SubsystemCenters;
	//std::vector<std::string> SubsystemNames;
	std::map< uint8_t, std::vector<Pos3D> > Weapons;
	std::map< uint8_t, double > FireTime;
	std::map< uint8_t, int8_t > Ammo;
	std::vector<ShipClassTurret> Turrets;
	double TurretHealth;
	double TurretYawSpeed, TurretPitchSpeed;
	std::string TurretBody, TurretGun;
	double TurretGunWidth;
	double TurretGunUp, TurretGunFwd, TurretHeadUp, TurretHeadFwd;
	std::vector<ShipClassDockingBay> DockingBays;
	std::vector<ShipClassEngine> Engines;
	std::string CollisionModel;
	std::string ExternalModel;
	std::string CockpitModel, CockpitModelVR;
	std::map<uint8_t,std::string> GroupSkins;
	std::map<uint8_t,std::string> GroupCockpits;
	Vec3D CockpitPos, CockpitPosVR;
	double GlanceUpFwd, GlanceUpBack;
	double ModelScale;
	Model Shape;
	std::map< double, std::string > FlybySounds;
	bool Secret;
	
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
		CATEGORY_UNKNOWN = 0,
		CATEGORY_FIGHTER,
		CATEGORY_BOMBER,
		CATEGORY_GUNBOAT,
		CATEGORY_CAPITAL,
		CATEGORY_TARGET
	};
};


class ShipClassTurret : public Pos3D
{
public:
	bool Visible, CanBeHit;
	double Health;
	bool ParentControl, Manual;
	uint8_t Weapon;
	uint8_t FiringMode;
	double SingleShotDelay;
	double TargetArc;
	double MinGunPitch;
	double MaxGunPitch;
	
	ShipClassTurret( double fwd, double up, double right, uint8_t weapon );
	ShipClassTurret &operator = ( const ShipClassTurret &other );
	virtual ~ShipClassTurret();
};


class ShipClassDockingBay : public Vec3D
{
public:
	double Radius;
	
	ShipClassDockingBay( double fwd, double up, double right, double radius = 35. );
	ShipClassDockingBay &operator = ( const ShipClassDockingBay &other );
	virtual ~ShipClassDockingBay();
};


class ShipClassEngine : public Vec3D
{
public:
	std::string Texture;
	double Radius;
	Color DrawColor;
	
	ShipClassEngine( double fwd, double up, double right, std::string texture, double radius, float r = 1.f, float g = 1.f, float b = 1.f, float a = 1.f );
	ShipClassEngine &operator = ( const ShipClassEngine &other );
	virtual ~ShipClassEngine();
};
