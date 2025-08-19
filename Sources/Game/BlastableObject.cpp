/*
 *  BlastableObject.cpp
 */

#include "BlastableObject.h"

#include <cmath>
#include "XWingGame.h"


BlastableObject::BlastableObject( uint32_t id, uint32_t type_code, uint16_t player_id ) : GameObject( id, type_code, player_id )
{
	RecentBlastPoint = NULL;
}


BlastableObject::BlastableObject( const BlastableObject &other ) : GameObject( other )
{
	RecentBlastPoint = NULL;
}


BlastableObject::~BlastableObject()
{
}


void BlastableObject::SetBlastPoint( double x, double y, double z, double radius, double time )
{
	Pos3D shot( x, y, z );
	double fwd   = shot.DistAlong( &Fwd,   this );
	double up    = shot.DistAlong( &Up,    this );
	double right = shot.DistAlong( &Right, this );
	
	SetBlastPointRelative( fwd, up, right, radius, time );
}


void BlastableObject::SetBlastPointRelative( double fwd, double up, double right, double radius, double time )
{
	size_t blastpoints = ((XWingGame*)( Raptor::Game ))->BlastPoints;
	if( ! blastpoints )
		return;
	
	if( BlastPoints.size() < blastpoints )
	{
		BlastPoints.push_back( BlastPoint( fwd, up, right, radius, time ) );
		RecentBlastPoint = &*(BlastPoints.rbegin());
	}
	else
	{
		RecentBlastPoint = LeastImportantBlastPoint();
		RecentBlastPoint->Reset( fwd, up, right, radius, time );
	}
}


BlastPoint* BlastableObject::LeastImportantBlastPoint( void )
{
	if( BlastPoints.empty() )
		return NULL;
	
	BlastPoint *least_important = &*(BlastPoints.begin());
	double least_importance = least_important->Importance();
	size_t blastpoints = std::min<size_t>( BlastPoints.size(), ((XWingGame*)( Raptor::Game ))->BlastPoints );
	for( size_t i = 0; i < blastpoints; i ++ )
	{
		BlastPoint *blastpoint = &(BlastPoints[ i ]);
		double importance = blastpoint->Importance();
		if( importance < least_importance )
		{
			least_important = blastpoint;
			least_importance = importance;
		}
	}
	return least_important;
}


// -----------------------------------------------------------------------------


BlastPoint::BlastPoint( double fwd, double up, double right, double radius, double time )
{
	X = fwd;
	Y = up;
	Z = right;
	Radius = radius;
	Lifetime.CountUpToSecs = time;
}


BlastPoint::~BlastPoint()
{
}


void BlastPoint::Reset( double fwd, double up, double right, double radius, double time )
{
	X = fwd;
	Y = up;
	Z = right;
	Radius = radius;
	Lifetime.Reset( time );
}


double BlastPoint::Importance( void ) const
{
	return Radius * Radius / std::max<double>( 0.01, Lifetime.ElapsedSeconds() );
}


void BlastPoint::AddArea( double da )
{
	Radius = sqrt( std::max<double>( 0., (M_PI * Radius * Radius + da) / M_PI ) );
}
