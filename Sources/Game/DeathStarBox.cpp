/*
 *  DeathStarBox.cpp
 */

#include "DeathStarBox.h"

#include <cmath>
#include "XWingDefs.h"
#include "RaptorGame.h"
#include "Ship.h"
#include "Num.h"


DeathStarBox::DeathStarBox( uint32_t id ) : GameObject( id, XWing::Object::DEATH_STAR_BOX )
{
	L = 1.;
	H = 1.;
	W = 1.;
	
	Shape = NULL;
}


DeathStarBox::~DeathStarBox()
{
}


void DeathStarBox::ClientInit( void )
{
	Shape = Raptor::Game->Res.GetModel("deathstar_box.obj");
}


bool DeathStarBox::PlayerShouldUpdateServer( void ) const
{
	return false;
}

bool DeathStarBox::ServerShouldUpdatePlayer( void ) const
{
	return false;
}

bool DeathStarBox::ServerShouldUpdateOthers( void ) const
{
	return false;
}

bool DeathStarBox::CanCollideWithOwnType( void ) const
{
	return false;
}

bool DeathStarBox::CanCollideWithOtherTypes( void ) const
{
	return true;
}

bool DeathStarBox::IsMoving( void ) const
{
	return false;
}


void DeathStarBox::AddToInitPacket( Packet *packet, int8_t precision )
{
	if( precision > 0 )
	{
		packet->AddDouble( X );
		packet->AddDouble( Y );
		packet->AddDouble( Z );
	}
	else
	{
		packet->AddFloat( X );
		packet->AddFloat( Y );
		packet->AddFloat( Z );
	}
	packet->AddShort( Num::UnitFloatTo16(Fwd.X) );
	packet->AddShort( Num::UnitFloatTo16(Fwd.Y) );
	packet->AddShort( Num::UnitFloatTo16(Fwd.Z) );
	packet->AddChar( Num::UnitFloatTo8(Up.X) );
	packet->AddChar( Num::UnitFloatTo8(Up.Y) );
	packet->AddChar( Num::UnitFloatTo8(Up.Z) );
	packet->AddFloat( L );
	packet->AddFloat( H );
	packet->AddFloat( W );
}


void DeathStarBox::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	if( precision > 0 )
	{
		X = packet->NextDouble();
		Y = packet->NextDouble();
		Z = packet->NextDouble();
	}
	else
	{
		X = packet->NextFloat();
		Y = packet->NextFloat();
		Z = packet->NextFloat();
	}
	Fwd.X = Num::UnitFloatFrom16( packet->NextShort() );
	Fwd.Y = Num::UnitFloatFrom16( packet->NextShort() );
	Fwd.Z = Num::UnitFloatFrom16( packet->NextShort() );
	Up.X = Num::UnitFloatFrom8( packet->NextChar() );
	Up.Y = Num::UnitFloatFrom8( packet->NextChar() );
	Up.Z = Num::UnitFloatFrom8( packet->NextChar() );
	PrevPos.Copy( this );
	L = packet->NextFloat();
	H = packet->NextFloat();
	W = packet->NextFloat();
}


bool DeathStarBox::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object, Pos3D *loc, double *when ) const
{
	if( other->Type() == XWing::Object::SHOT )
	{
		if( (fabs(other->DistAlong(&Fwd,this)) <= L/2.) && (fabs(other->DistAlong(&Up,this)) <= H/2.) && (fabs(other->DistAlong(&Right,this)) <= W/2.) )
			return true;
	}
	
	else if( other->Type() == XWing::Object::SHIP )
	{
		// We don't care about dead ships hitting a box.
		Ship *ship = (Ship*) other;
		if( ship->Health <= 0. )
			return false;
		
		// Don't destroy the exhaust port.
		if( ship->Category() == ShipClass::CATEGORY_TARGET )
			return false;
		
		// Don't worry about capital ship hitting these.
		if( ship->ComplexCollisionDetection() )
			return false;
		
		if( (fabs(other->DistAlong(&Fwd,this)) <= L/2. + ship->Radius()) && (fabs(other->DistAlong(&Up,this)) <= H/2. + ship->Radius()) && (fabs(other->DistAlong(&Right,this)) <= W/2. + ship->Radius()) )
			return true;
	}
	
	return false;
}


void DeathStarBox::Update( double dt )
{
}


void DeathStarBox::Draw( void )
{
	bool change_shaders = (Raptor::Game->Cfg.SettingAsInt("g_shader_light_quality") >= 2) && Raptor::Game->ShaderMgr.Active();
	Shader *prev_shader = Raptor::Game->ShaderMgr.Selected;
	if( change_shaders )
		Raptor::Game->ShaderMgr.SelectAndCopyVars( Raptor::Game->Res.GetShader("deathstar") );
	
	if( Shape )
		Shape->DrawAt( this, 1., L, H, W );
	
	if( change_shaders )
		Raptor::Game->ShaderMgr.Select( prev_shader );
}
