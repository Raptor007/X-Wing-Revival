/*
 *  DeathStar.cpp
 */

#include "DeathStar.h"

#include <cmath>
#include "XWingDefs.h"
#include "RaptorGame.h"
#include "Num.h"
#include "Math3D.h"
#include "Ship.h"
#include "Shot.h"


DeathStar::DeathStar( uint32_t id ) : GameObject( id, XWing::Object::DEATH_STAR )
{
	TrenchWidth = 40.;
	TrenchDepth = 80.;
	TextureSize = 500.;
	DetailHeight = 2.;
	
	Fwd.Set( 0., 1., 0. );
	Up.Set( 0., 0., 1. );
	FixVectors();
	
	Ambient.Set( 0.f, 0.f, 0.f, 1.f );
	Diffuse.Set( 0.4f, 0.4f, 0.4f, 1.f );
	BottomAmbient.Set( -0.35f, -0.35f, -0.35f, 1.f );
	BottomDiffuse.Set( 0.4f, 0.4f, 0.4f, 1.f );
}


DeathStar::~DeathStar()
{
}


void DeathStar::ClientInit( void )
{
	DetailSide.BecomeInstance( Raptor::Game->Res.GetModel("deathstar_detail.obj") );
	double w = DetailSide.GetWidth();
	double l = DetailSide.GetLength();
	if( w && l )
	{
		DetailSide.ScaleBy( TrenchDepth / w, DetailHeight, TextureSize / l );
		DetailSide.CalculateNormals();
	}
	
	DetailBottom.BecomeInstance( Raptor::Game->Res.GetModel("deathstar_detail_bottom.obj") );
	w = DetailBottom.GetWidth();
	l = DetailBottom.GetLength();
	if( w && l )
	{
		DetailBottom.ScaleBy( TextureSize / l, DetailHeight, TrenchWidth / w );
		DetailBottom.CalculateNormals();
	}
	
	// Get the texture and colors for flat-side rendering.
	int best_vertex_count = -1;
	if( DetailSide.Materials.size() )
	{
		// Use the model's values if available.
		for( std::map<std::string,ModelMaterial>::iterator mtl_iter = DetailSide.Materials.begin(); mtl_iter != DetailSide.Materials.end(); mtl_iter ++ )
		{
			// We want to match the most-used material of the model.
			if( mtl_iter->second.Arrays.VertexCount > best_vertex_count )
			{
				Texture.BecomeInstance( &(mtl_iter->second.Texture) );
				Ambient = mtl_iter->second.Ambient;
				Diffuse = mtl_iter->second.Diffuse;
				best_vertex_count = mtl_iter->second.Arrays.VertexCount;
			}
		}
	}
	if( best_vertex_count < 0 )
	{
		// If we couldn't look up the data from the model, use defaults.
		Texture.BecomeInstance( Raptor::Game->Res.GetAnimation("deathstar.ani") );
	}
	
	// Get the texture and colors for rendering the bottom.
	if( DetailBottom.Materials.size() )
	{
		best_vertex_count = -1;
		
		// Use the model's values if available.
		for( std::map<std::string,ModelMaterial>::iterator mtl_iter = DetailBottom.Materials.begin(); mtl_iter != DetailBottom.Materials.end(); mtl_iter ++ )
		{
			// We want to match the most-used material of the model.
			if( mtl_iter->second.Arrays.VertexCount > best_vertex_count )
			{
				BottomAmbient = mtl_iter->second.Ambient;
				BottomDiffuse = mtl_iter->second.Diffuse;
				best_vertex_count = mtl_iter->second.Arrays.VertexCount;
			}
		}
	}
}


bool DeathStar::PlayerShouldUpdateServer( void ) const
{
	return false;
}

bool DeathStar::ServerShouldUpdatePlayer( void ) const
{
	return false;
}

bool DeathStar::ServerShouldUpdateOthers( void ) const
{
	return false;
}

bool DeathStar::CanCollideWithOwnType( void ) const
{
	return false;
}

bool DeathStar::CanCollideWithOtherTypes( void ) const
{
	return true;
}

bool DeathStar::IsMoving( void ) const
{
	return false;
}


void DeathStar::AddToInitPacket( Packet *packet, int8_t precision )
{
	GameObject::AddToInitPacket( packet, precision );
	packet->AddDouble( TrenchWidth );
	packet->AddDouble( TrenchDepth );
}


void DeathStar::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, precision );
	TrenchWidth = packet->NextDouble();
	TrenchDepth = packet->NextDouble();
}


void DeathStar::AddToUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacketFromServer( packet, precision );
}


void DeathStar::ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::ReadFromUpdatePacketFromServer( packet, precision );
}


bool DeathStar::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object ) const
{
	if( other->Type() == XWing::Object::SHOT )
	{
		// The Death Star can't shoot itself.
		Shot *shot = (Shot*) other;
		if( shot->FiredFrom == ID )
			return false;
		
		// Adjust for the trench depth.
		double bottom = 0.;
		if( fabs( other->DistAlong(&Right,this) ) < TrenchWidth / 2. )
			bottom -= TrenchDepth;
		
		// See if it hit bottom.
		if( other->DistAlong(&Up,this) < bottom )
			return true;
	}
	
	else if( other->Type() == XWing::Object::SHIP )
	{
		// We don't care about dead ships hitting the Death Star.
		Ship *ship = (Ship*) other;
		if( ship->Health <= 0. )
			return false;
		
		// Don't destroy the exhaust port.
		if( ship->ShipType == Ship::TYPE_EXHAUST_PORT )
			return false;
		
		// Use per-vertex checking for capital ship near Death Star.
		if( ship->ComplexCollisionDetection() )
		{
			if( ship->DistAlong(&Up,this) > ship->Shape.PrecalculatedMaxRadius() )
				return false;
			
			ModelArrays array_inst;
			for( std::map<std::string,ModelObject>::const_iterator obj_iter = ship->Shape.Objects.begin(); obj_iter != ship->Shape.Objects.end(); obj_iter ++ )
			{
				// Don't detect collisions with destroyed subsystems.
				std::map<std::string,double>::const_iterator subsystem_iter = ship->Subsystems.find( obj_iter->first );
				if( (subsystem_iter != ship->Subsystems.end()) && (subsystem_iter->second <= 0.) )
					continue;
				
				for( std::map<std::string,ModelArrays>::const_iterator array_iter = obj_iter->second.Arrays.begin(); array_iter != obj_iter->second.Arrays.end(); array_iter ++ )
				{
					array_inst.BecomeInstance( &(array_iter->second), false );
					array_inst.MakeWorldSpace( ship );
					for( int i = 0; i < array_inst.VertexCount; i ++ )
					{
						Pos3D vertex( array_inst.WorldSpaceVertexArray[ i*3 ], array_inst.WorldSpaceVertexArray[ i*3 + 1 ], array_inst.WorldSpaceVertexArray[ i*3 + 2 ] );
						double bottom = 0.;
						/*
						if( fabs( vertex.DistAlong(&Right,this) ) < TrenchWidth / 2. )
							bottom -= TrenchDepth;
						*/
						if( vertex.DistAlong(&Up,this) <= bottom )
						{
							if( other_object )
								*other_object = obj_iter->first;
							return true;
						}
					}
				}
			}
			return false;
		}
		
		// Adjust for the trench depth.
		double bottom = 0.;
		if( fabs( other->DistAlong(&Right,this) ) + ship->Radius() < TrenchWidth / 2. )
			bottom -= TrenchDepth;
		
		// See if it hit bottom.
		if( other->DistAlong(&Up,this) - ship->Radius() < bottom )
			return true;
	}
	
	return false;
}


void DeathStar::Update( double dt )
{
	GameObject::Update( dt );
}


void DeathStar::Draw( void )
{
	bool use_shaders = Raptor::Game->ShaderMgr.Active();
	
	double cam_up = Raptor::Game->Cam.DistAlong(&Up,this);
	double cam_right = Raptor::Game->Cam.DistAlong(&Right,this);
	double cam_fwd = Raptor::Game->Cam.DistAlong(&Fwd,this);
	
	double trench_fwd = 0.7 * Raptor::Game->Cfg.SettingAsDouble( "g_bg_dist", 50000. );
	double textures_fwd = trench_fwd * 2. / TextureSize;
	double texture_fwd_offset = -0.5 - Num::FPart( cam_fwd / TextureSize );
	if( texture_fwd_offset < -0.5 )
		texture_fwd_offset += 1.;
	Pos3D center( this );
	center.MoveAlong( &Fwd, cam_fwd );
	
	int deathstar_detail = Raptor::Game->Cfg.SettingAsInt( "g_deathstar_detail", 3 );
	if( deathstar_detail && ( Raptor::Game->Cam.Dist(&center) < Raptor::Game->Cfg.SettingAsDouble( "g_deathstar_detail_dist", 1000. ) ) )
	{
		double sign = Num::Sign( Raptor::Game->Cam.Fwd.Dot(&Fwd) );
		Pos3D detail_center( &center );
		detail_center.MoveAlong( &Fwd, (texture_fwd_offset - 0.5 * sign) * TextureSize );
		detail_center.MoveAlong( &Up, -TrenchDepth );
		Pos3D detail_left( &detail_center );
		detail_left.MoveAlong( &Right, TrenchWidth / -2. );
		detail_left.MoveAlong( &Up, TrenchDepth / 2. );
		detail_left.Fwd.Copy( &Up );
		detail_left.Up.Copy( &Right );
		detail_left.FixVectors();
		Pos3D detail_right( &detail_left );
		detail_right.MoveAlong( &Right, TrenchWidth );
		detail_right.Up.ScaleBy( -1. );
		detail_right.Right.ScaleBy( -1. );
		detail_right.FixVectors();
		
		if( cam_up >= -TrenchDepth )
			DetailBottom.DrawAt( &detail_center );
		if( (cam_up >= 0.) || (cam_right >= TrenchWidth / -2.) )
			DetailSide.DrawAt( &detail_left );
		if( (cam_up >= 0.) || (cam_right <= TrenchWidth / 2.) )
			DetailSide.DrawAt( &detail_right );
		
		for( int i = 1; i < deathstar_detail; i ++ )
		{
			detail_center.MoveAlong( &Fwd, TextureSize * sign );
			detail_left.MoveAlong( &Fwd, TextureSize * sign );
			detail_right.MoveAlong( &Fwd, TextureSize * sign );
			
			if( cam_up >= -TrenchDepth )
				DetailBottom.DrawAt( &detail_center );
			if( (cam_up >= 0.) || (cam_right >= TrenchWidth / -2.) )
				DetailSide.DrawAt( &detail_left );
			if( (cam_up >= 0.) || (cam_right <= TrenchWidth / 2.) )
				DetailSide.DrawAt( &detail_right );
		}
		
		detail_center.MoveAlong( &Fwd, TextureSize * sign );
		detail_left.MoveAlong( &Fwd, TextureSize * sign );
		detail_right.MoveAlong( &Fwd, TextureSize * sign );
		
		//double height_scale = 0.5 - sign * texture_fwd_offset;
		double height_scale = 1.;
		
		if( cam_up >= -TrenchDepth )
			DetailBottom.DrawAt( &detail_center, 1., 1., height_scale, 1. );
		if( (cam_up >= 0.) || (cam_right >= TrenchWidth / -2.) )
			DetailSide.DrawAt( &detail_left, 1., 1., height_scale, 1. );
		if( (cam_up >= 0.) || (cam_right <= TrenchWidth / 2.) )
			DetailSide.DrawAt( &detail_right, 1., 1., height_scale, 1. );
	}
	
	Pos3D slice[ 4 ];
	slice[ 0 ].Copy( &center );
	slice[ 0 ].MoveAlong( &Right, TrenchWidth / -2. );
	slice[ 1 ].Copy( &(slice[ 0 ]) );
	slice[ 1 ].MoveAlong( &Right, TrenchWidth );
	slice[ 2 ].Copy( &(slice[ 0 ]) );
	slice[ 2 ].MoveAlong( &Up, TrenchDepth * -1. );
	slice[ 3 ].Copy( &(slice[ 1 ]) );
	slice[ 3 ].MoveAlong( &Up, TrenchDepth * -1. );
	
	if( use_shaders )
	{
		Raptor::Game->ShaderMgr.Set3f( "AmbientColor", Ambient.Red, Ambient.Green, Ambient.Blue );
		Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", Diffuse.Red, Diffuse.Green, Diffuse.Blue );
	}
	
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, Texture.CurrentFrame() );
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	
	// NOTE: This assumes we've run glGenerateMipmap(GL_TEXTURE_2D) when loading the texture!
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	
	int tx_width = Num::NearestWhole( TrenchWidth / TextureSize ) + 0.5;
	if( tx_width < 1 )
		tx_width = 1.;
	int tx_depth = Num::NearestWhole( TrenchDepth / TextureSize ) + 0.5;
	if( tx_depth < 1 )
		tx_depth = 1.;
	
	glBegin( GL_QUADS );
		
		if( cam_up >= 0. )
		{
			// Surface Left
			glNormal3d( Up.X, Up.Y, Up.Z );
			glTexCoord2d( 0., texture_fwd_offset );
			glVertex3d( slice[ 0 ].X + Fwd.X * trench_fwd - Right.X * trench_fwd, slice[ 0 ].Y + Fwd.Y * trench_fwd - Right.Y * trench_fwd, slice[ 0 ].Z + Fwd.Z * trench_fwd - Right.Z * trench_fwd );
			glTexCoord2d( 0., texture_fwd_offset + textures_fwd );
			glVertex3d( slice[ 0 ].X - Fwd.X * trench_fwd - Right.X * trench_fwd, slice[ 0 ].Y - Fwd.Y * trench_fwd - Right.Y * trench_fwd, slice[ 0 ].Z - Fwd.Z * trench_fwd - Right.Z * trench_fwd );
			glTexCoord2d( textures_fwd / 2., textures_fwd + texture_fwd_offset );
			glVertex3d( slice[ 0 ].X - Fwd.X * trench_fwd, slice[ 0 ].Y - Fwd.Y * trench_fwd, slice[ 0 ].Z - Fwd.Z * trench_fwd );
			glTexCoord2d( textures_fwd / 2., texture_fwd_offset );
			glVertex3d( slice[ 0 ].X + Fwd.X * trench_fwd, slice[ 0 ].Y + Fwd.Y * trench_fwd, slice[ 0 ].Z + Fwd.Z * trench_fwd );
			
			// Surface Right
			glNormal3d( Up.X, Up.Y, Up.Z );
			glTexCoord2d( 0., texture_fwd_offset );
			glVertex3d( slice[ 1 ].X + Fwd.X * trench_fwd, slice[ 1 ].Y + Fwd.Y * trench_fwd, slice[ 1 ].Z + Fwd.Z * trench_fwd );
			glTexCoord2d( 0., texture_fwd_offset + textures_fwd );
			glVertex3d( slice[ 1 ].X - Fwd.X * trench_fwd, slice[ 1 ].Y - Fwd.Y * trench_fwd, slice[ 1 ].Z - Fwd.Z * trench_fwd );
			glTexCoord2d( textures_fwd / 2., textures_fwd + texture_fwd_offset );
			glVertex3d( slice[ 1 ].X - Fwd.X * trench_fwd + Right.X * trench_fwd, slice[ 1 ].Y - Fwd.Y * trench_fwd + Right.Y * trench_fwd, slice[ 1 ].Z - Fwd.Z * trench_fwd + Right.Z * trench_fwd );
			glTexCoord2d( textures_fwd / 2., texture_fwd_offset );
			glVertex3d( slice[ 1 ].X + Fwd.X * trench_fwd + Right.X * trench_fwd, slice[ 1 ].Y + Fwd.Y * trench_fwd + Right.Y * trench_fwd, slice[ 1 ].Z + Fwd.Z * trench_fwd + Right.Z * trench_fwd );
		}
		
		if( cam_right >= TrenchWidth / -2. )
		{
			// Wall Left
			glNormal3d( Right.X, Right.Y, Right.Z );
			glTexCoord2d( -texture_fwd_offset, 0. );
			glVertex3d( slice[ 0 ].X - Fwd.X * trench_fwd, slice[ 0 ].Y - Fwd.Y * trench_fwd, slice[ 0 ].Z - Fwd.Z * trench_fwd );
			glTexCoord2d( -texture_fwd_offset, tx_depth );
			glVertex3d( slice[ 2 ].X - Fwd.X * trench_fwd, slice[ 2 ].Y - Fwd.Y * trench_fwd, slice[ 2 ].Z - Fwd.Z * trench_fwd );
			glTexCoord2d( -texture_fwd_offset + textures_fwd, tx_depth );
			glVertex3d( slice[ 2 ].X + Fwd.X * trench_fwd, slice[ 2 ].Y + Fwd.Y * trench_fwd, slice[ 2 ].Z + Fwd.Z * trench_fwd );
			glTexCoord2d( -texture_fwd_offset + textures_fwd, 0. );
			glVertex3d( slice[ 0 ].X + Fwd.X * trench_fwd, slice[ 0 ].Y + Fwd.Y * trench_fwd, slice[ 0 ].Z + Fwd.Z * trench_fwd );
		}
		
		if( cam_right <= TrenchWidth / 2. )
		{
			// Wall Right
			glNormal3d( -(Right.X), -(Right.Y), -(Right.Z) );
			glTexCoord2d( texture_fwd_offset, 0. );
			glVertex3d( slice[ 1 ].X + Fwd.X * trench_fwd, slice[ 1 ].Y + Fwd.Y * trench_fwd, slice[ 1 ].Z + Fwd.Z * trench_fwd );
			glTexCoord2d( texture_fwd_offset, tx_depth );
			glVertex3d( slice[ 3 ].X + Fwd.X * trench_fwd, slice[ 3 ].Y + Fwd.Y * trench_fwd, slice[ 3 ].Z + Fwd.Z * trench_fwd );
			glTexCoord2d( texture_fwd_offset + textures_fwd, tx_depth );
			glVertex3d( slice[ 3 ].X - Fwd.X * trench_fwd, slice[ 3 ].Y - Fwd.Y * trench_fwd, slice[ 3 ].Z - Fwd.Z * trench_fwd );
			glTexCoord2d( texture_fwd_offset + textures_fwd, 0. );
			glVertex3d( slice[ 1 ].X - Fwd.X * trench_fwd, slice[ 1 ].Y - Fwd.Y * trench_fwd, slice[ 1 ].Z - Fwd.Z * trench_fwd );
		}
	
		if( use_shaders )
		{
			glEnd();
			Raptor::Game->ShaderMgr.Set3f( "AmbientColor", BottomAmbient.Red, BottomAmbient.Green, BottomAmbient.Blue );
			Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", BottomDiffuse.Red, BottomDiffuse.Green, BottomDiffuse.Blue );
			glBegin( GL_QUADS );
		}
		
		if( cam_up >= -TrenchDepth )
		{
			// Bottom
			glNormal3d( Up.X, Up.Y, Up.Z );
			glTexCoord2d( 0., texture_fwd_offset );
			glVertex3d( slice[ 2 ].X + Fwd.X * trench_fwd, slice[ 2 ].Y + Fwd.Y * trench_fwd, slice[ 2 ].Z + Fwd.Z * trench_fwd );
			glTexCoord2d( 0., textures_fwd + texture_fwd_offset );
			glVertex3d( slice[ 2 ].X - Fwd.X * trench_fwd, slice[ 2 ].Y - Fwd.Y * trench_fwd, slice[ 2 ].Z - Fwd.Z * trench_fwd );
			glTexCoord2d( tx_width, textures_fwd + texture_fwd_offset );
			glVertex3d( slice[ 3 ].X - Fwd.X * trench_fwd, slice[ 3 ].Y - Fwd.Y * trench_fwd, slice[ 3 ].Z - Fwd.Z * trench_fwd );
			glTexCoord2d( tx_width, texture_fwd_offset );
			glVertex3d( slice[ 3 ].X + Fwd.X * trench_fwd, slice[ 3 ].Y + Fwd.Y * trench_fwd, slice[ 3 ].Z + Fwd.Z * trench_fwd );
		}
		
	glEnd();
	
	if( Raptor::Game->ShaderMgr.Active() )
	{
		Raptor::Game->ShaderMgr.Set3f( "AmbientColor", 1.f, 1.f, 1.f );
		Raptor::Game->ShaderMgr.Set3f( "DiffuseColor", 0.f, 0.f, 0.f );
	}
}
