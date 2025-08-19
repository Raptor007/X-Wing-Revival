/*
 *  BlastableObject.h
 */

#pragma once
class BlastableObject;
class BlastPoint;

#include "PlatformSpecific.h"

#include "GameObject.h"
#include "Model.h"


class BlastableObject : public GameObject
{
public:
	std::vector<BlastPoint> BlastPoints;
	BlastPoint *RecentBlastPoint;
	
	BlastableObject( uint32_t id = 0, uint32_t type_code = '    ', uint16_t player_id = 0 );
	BlastableObject( const BlastableObject &other );
	virtual ~BlastableObject();
	
	virtual void SetBlastPoint( double x, double y, double z, double radius, double time = 0. );
	virtual void SetBlastPointRelative( double fwd, double up, double right, double radius, double time = 0. );
	virtual BlastPoint *LeastImportantBlastPoint( void );
};


class BlastPoint : public Vec3D
{
public:
	double Radius;
	Clock Lifetime;
	
	BlastPoint( double fwd, double up, double right, double radius, double time = 0. );
	virtual ~BlastPoint();
	
	void Reset( double fwd, double up, double right, double radius, double time = 0. );
	double Importance( void ) const;
	void AddArea( double da );
};
