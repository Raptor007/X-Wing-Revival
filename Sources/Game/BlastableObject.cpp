/*
 *  BlastableObject.cpp
 */

#include "BlastableObject.h"

#include <cmath>
#include "XWingGame.h"


BlastableObject::BlastableObject( uint32_t id, uint32_t type_code, uint16_t player_id ) : GameObject( id, type_code, player_id )
{
}


BlastableObject::BlastableObject( const BlastableObject &other ) : GameObject( other )
{
}


BlastableObject::~BlastableObject()
{
}


void BlastableObject::SetBlastPoint( double x, double y, double z, double radius, double time, const ModelObject *object )
{
	size_t blastpoints = ((XWingGame*)( Raptor::Game ))->BlastPoints;
	if( ! blastpoints )
		return;
	
	Pos3D shot( x, y, z );
	double fwd   = shot.DistAlong( &Fwd,   this );
	double up    = shot.DistAlong( &Up,    this );
	double right = shot.DistAlong( &Right, this );
	
	if( BlastPoints.size() < blastpoints )
		BlastPoints.push_back( BlastPoint( fwd, up, right, radius, time, object ) );
	else
		LeastImportantBlastPoint()->Reset( fwd, up, right, radius, time, object );
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


BlastPoint::BlastPoint( double fwd, double up, double right, double radius, double time, const ModelObject *object )
{
	X = fwd;
	Y = up;
	Z = right;
	Radius = radius;
	Object = object;
	Lifetime.CountUpToSecs = time;
}


BlastPoint::~BlastPoint()
{
}


void BlastPoint::Reset( double fwd, double up, double right, double radius, double time, const ModelObject *object )
{
	X = fwd;
	Y = up;
	Z = right;
	Radius = radius;
	Object = object;
	Lifetime.Reset( time );
}


double BlastPoint::Importance( void ) const
{
	return Radius * Radius / std::max<double>( 0.01, Lifetime.ElapsedSeconds() );
}


Pos3D BlastPoint::Worldspace( const Pos3D *pos ) const
{
	return *pos + pos->Fwd * X + pos->Up * Y + pos->Right * Z;
}


void BlastPoint::AddArea( double da )
{
	Radius = sqrt( std::max<double>( 0., (M_PI * Radius * Radius + da) / M_PI ) );
}
