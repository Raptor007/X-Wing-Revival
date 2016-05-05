/*
 *  Asteroid.cpp
 */

#include "Asteroid.h"

#include <cmath>

#include "XWingDefs.h"
#include "XWingGame.h"
#include "Rand.h"
#include "Math3D.h"


Asteroid::Asteroid( uint32_t id ) : GameObject( id, XWing::Object::ASTEROID )
{
	Shape = NULL;
	
	Radius = Rand::Double( 20., 50. );
	
	Fwd.Set( Rand::Double( -1., 1 ), Rand::Double( -1., 1 ), Rand::Double( -1., 1 ) );
	Up.Set( Rand::Double( -1., 1 ), Rand::Double( -1., 1 ), Rand::Double( -1., 1 ) );
	FixVectors();
	
	RollRate = Rand::Double( -10., 10 );
	PitchRate = Rand::Double( -10., 10 );
	YawRate = Rand::Double( -10., 10 );

	Shininess = 1.f;
}


Asteroid::~Asteroid()
{
	Shape = NULL;
}


void Asteroid::ClientInit( void )
{
	// Just use a pointer to a shape in the resource manager.
	Shape = Raptor::Game->Res.GetModel("asteroid.obj");
	if( Shape && (! Shape->Objects.size()) )
		Shape = NULL;
	
	// Get the texture and colors for sphere rendering.
	int best_vertex_count = -1;
	if( Shape && Shape->Materials.size() )
	{
		// Use the model's values if available.
		for( std::map<std::string,ModelMaterial>::iterator mtl_iter = Shape->Materials.begin(); mtl_iter != Shape->Materials.end(); mtl_iter ++ )
		{
			// We want to match the most-used material of the model.
			if( mtl_iter->second.Arrays.VertexCount > best_vertex_count )
			{
				Texture.BecomeInstance( &(mtl_iter->second.Texture) );
				Ambient = mtl_iter->second.Ambient;
				Diffuse = mtl_iter->second.Diffuse;
				Specular = mtl_iter->second.Specular;
				Shininess = mtl_iter->second.Shininess;
				best_vertex_count = mtl_iter->second.Arrays.VertexCount;
			}
		}
	}
	if( best_vertex_count < 0 )
	{
		// If we couldn't look up the data from the model, use these defaults.
		Texture.BecomeInstance( Raptor::Game->Res.GetAnimation("dirt.ani") );
		Ambient.Set( 0.f, 0.f, 0.f, 1.f );
		Diffuse.Set( 0.5f, 0.5f, 0.5f, 1.f );
		Specular.Set( 0.f, 0.f, 0.f, 1.f );
		Shininess = 1.f;
	}
}


bool Asteroid::PlayerShouldUpdateServer( void ) const
{
	return false;
}

bool Asteroid::ServerShouldUpdatePlayer( void ) const
{
	return false;
}

bool Asteroid::ServerShouldUpdateOthers( void ) const
{
	return false;
}

bool Asteroid::CanCollideWithOwnType( void ) const
{
	return false;
}

bool Asteroid::CanCollideWithOtherTypes( void ) const
{
	return true;
}

bool Asteroid::IsMoving( void ) const
{
	return false;
}


void Asteroid::AddToInitPacket( Packet *packet, int8_t precision )
{
	GameObject::AddToInitPacket( packet, -127 );
	packet->AddUChar( Radius + 0.5 );
	packet->AddChar( RollRate + 0.5 );
	packet->AddChar( PitchRate + 0.5 );
	packet->AddChar( YawRate + 0.5 );
}


void Asteroid::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, -127 );
	Radius = packet->NextUChar();
	RollRate = packet->NextChar();
	PitchRate = packet->NextChar();
	YawRate = packet->NextChar();
}


bool Asteroid::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object ) const
{
	if( other->Type() == XWing::Object::SHOT )
	{
		double dist = Math3D::MinimumDistance( this, &(this->MotionVector), other, &(other->MotionVector), dt );
		if( dist <= Radius )
			return true;
	}
	
	// Let ships determine whether collisions with asteroids occur.
	else if( other->Type() == XWing::Object::SHIP )
		return other->WillCollide( this, dt, other_object, this_object );
	
	return false;
}


void Asteroid::Draw( void )
{
	// Don't draw if the camera is within the asteroid.
	double dist = Raptor::Game->Cam.Dist( this );
	if( dist < Radius )
		return;
	
	// Don't draw asteroids behind us.
	double size = Radius;
	if( Shape )
		size = Shape->GetTriagonal();
	if( DistAlong( &(Raptor::Game->Cam.Fwd), &(Raptor::Game->Cam) ) < -size )
		return;
	
	// Calculate asteroid detail based on distance and screen resolution.
	int detail = Raptor::Game->Gfx.H * Radius * 0.5 / dist;
	
	// If we decided to draw the model, draw it.
	if( (detail >= 5) && Shape )
		Shape->DrawAt( this, Radius * 3.4 / Shape->GetTriagonal() );
	else
	{
		// We're not drawing the model, so draw a sphere instead.
		
		#ifdef APPLE_POWERPC
			int max_detail = 6;
		#else
			int max_detail = 8;
		#endif
		if( detail > max_detail )
			detail = max_detail;
		else if( detail < 3 )
			detail = 3;
		
		if( Raptor::Game->ShaderMgr.Active() )
		{
			// Match the color values in asteroid.mtl for shader lighting.
			Raptor::Game->ShaderMgr.Set3f( "AmbientColor", Ambient.Red, Ambient.Green, Ambient.Blue );
			Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", Diffuse.Red, Diffuse.Green, Diffuse.Blue );
			Raptor::Game->ShaderMgr.Set3f( "SpecularColor", Specular.Red, Specular.Green, Specular.Blue );
			Raptor::Game->ShaderMgr.Set1f( "Shininess", Shininess );
		}
		
		Raptor::Game->Gfx.DrawSphere3D( X, Y, Z, Radius, detail, Texture.CurrentFrame(), Graphics::TEXTURE_MODE_X_DIV_R );
	}
}
