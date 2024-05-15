/*
 *  Turret.cpp
 */

#include "Turret.h"

#include <cmath>
#include "XWingDefs.h"
#include "XWingGame.h"
#include "Num.h"
#include "Math3D.h"
#include "Ship.h"
#include "Shot.h"


Turret::Turret( uint32_t id ) : BlastableObject( id, XWing::Object::TURRET )
{
	ParentID = 0;
	RelativeUp.Set( 0., 1., 0. );
	RelativeFwd.Set( 1., 0., 0. );
	ParentControl = false;
	Manual = false;
	Team = XWing::Team::NONE;
	
	MinGunPitch = 0.;
	MaxGunPitch = 80.;
	YawSpeed = PitchSpeed = 45.;
	Health = 95.;
	Weapon = Shot::TYPE_TURBO_LASER_GREEN;
	FiringMode = 2;
	SingleShotDelay = 0.5;
	MaxFiringDist = 1500.;
	AimAhead = 1.f;
	TargetDir.Copy(&Fwd);
	TargetArc = 360.;
	SafetyDistance = 0.;
	
	Visible = true;
	CanBeHit = true;
	Radius = 7.7;
	GunWidth = 2.2;
	GunUp = 0.022 * 175.;
	GunFwd = 0.022 * 50.;
	HeadUp = 2.;
	HeadFwd = 5.;
	BodyModel = "turret_body.obj";
	GunModel = "turret_gun.obj";
	BodyShape = NULL;
	GunShape = NULL;
	
	GunPitch = 0.;
	GunPitchRate = 0.;
	
	Firing = false;
	WeaponIndex = 0;
	FiredThisFrame = 0;
	PredictedShots = 0;
	Target = 0;
}


Turret::~Turret()
{
}


void Turret::ClientInit( void )
{
	if( Visible )
	{
		BodyShape = Raptor::Game->Res.GetModel(BodyModel);
		GunShape  = Raptor::Game->Res.GetModel(GunModel);
	}
}


void Turret::Attach( const GameObject *parent, const Vec3D *offset, const Vec3D *relative_up, const Vec3D *relative_fwd, bool parent_control )
{
	if( offset )
		Offset.Copy( offset );
	if( relative_up )
		RelativeUp = relative_up->Unit();
	if( relative_fwd )
		RelativeFwd = relative_fwd->Unit();
	
	ParentControl = parent_control;
	
	if( parent )
	{
		ParentID = parent->ID;
		Fwd.Copy( &(parent->Fwd) );
		UpdatePos( parent );
		Fwd.Copy( &TargetDir );
		FixVectorsKeepUp();
	}
	else
		ParentID = 0;
}


Ship *Turret::ParentShip( void ) const
{
	GameObject *parent = (Data && ParentID) ? Data->GetObject( ParentID ) : NULL;
	return (parent && (parent->Type() == XWing::Object::SHIP)) ? ((Ship*) parent) : NULL;
}


void Turret::UpdatePos( const GameObject *parent )
{
	if( ParentID && ! parent )
		parent = Data->GetObject( ParentID );
	if( ! parent )
		return;
	
	X = parent->X;
	Y = parent->Y;
	Z = parent->Z;
	MoveAlong( &(parent->Fwd), Offset.X );
	MoveAlong( &(parent->Up), Offset.Y );
	MoveAlong( &(parent->Right), Offset.Z );
	MotionVector.Copy( &(parent->MotionVector) );
	Up = (parent->Fwd * RelativeUp.X) + (parent->Up * RelativeUp.Y) + (parent->Right * RelativeUp.Z);
	TargetDir = (parent->Fwd * RelativeFwd.X) + (parent->Up * RelativeFwd.Y) + (parent->Right * RelativeFwd.Z);
	Vec3D prev_fwd( &Fwd );
	FixVectorsKeepUp();
	
	// Correct gun pitch for ship motion.
	double degrees = Fwd.AngleBetween( &prev_fwd );
	if( Up.Dot( &prev_fwd ) < 0. )
		degrees *= -1.;
	PitchGun( degrees );
}


void Turret::SetHealth( double health )
{
	if( (health <= 0.) && (Health > 0.) && ParentID && Data )
	{
		// Find any shots owned by this turret and change their ownership to the parent ship.
		for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data->GameObjects.begin(); obj_iter != Data->GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHOT )
			{
				Shot *shot = (Shot*) obj_iter->second;
				if( shot->FiredFrom == ID )
					shot->FiredFrom = ParentID;
			}
		}
	}
	
	Health = health;
}


void Turret::AddDamage( double damage )
{
	SetHealth( Health - damage );
}


void Turret::SetBlastPoint( double x, double y, double z, double radius, double time, const ModelObject *object )
{
	size_t blastpoints = ((XWingGame*)( Raptor::Game ))->BlastPoints;
	if( ! blastpoints )
		return;
	
	Pos3D shot( x, y, z );
	double fwd   = shot.DistAlong( &Fwd,   this );
	double up    = shot.DistAlong( &Up,    this );
	double right = shot.DistAlong( &Right, this );
	
	// FIXME: Why does this have to be scaled here?  (Is it related to cockpit-view blastpoints not always showing up?)
	fwd   *= std::min<double>( 1., BodyShape->GetLength() * 0.001 );
	up    *= std::min<double>( 1., BodyShape->GetHeight() * 0.001 );
	right *= std::min<double>( 1., BodyShape->GetWidth()  * 0.001 );
	
	if( BlastPoints.size() < blastpoints )
		BlastPoints.push_back( BlastPoint( fwd, up, right, radius, time, object ) );
	else
		LeastImportantBlastPoint()->Reset( fwd, up, right, radius, time, object );
}


void Turret::PitchGun( double degrees )
{
	// Don't apply pitch to the position, but keep track for the guns.
	if( Num::Valid(degrees) )
		GunPitch += degrees;
	
	if( GunPitch < MinGunPitch )
	{
		GunPitch = MinGunPitch;
		if( (GunPitchRate < 0.) && ClientSide() && (PlayerID == Raptor::Game->PlayerID) )
			GunPitchRate = 0.;
	}
	else if( GunPitch > MaxGunPitch )
	{
		GunPitch = MaxGunPitch;
		if( (GunPitchRate > 0.) && ClientSide() && (PlayerID == Raptor::Game->PlayerID) )
			GunPitchRate = 0.;
	}
}

void Turret::SetPitch( double pitch )
{
	if( pitch > 1. )
		pitch = 1.;
	else if( pitch < -1. )
		pitch = -1.;
	
	GunPitchRate = pitch * PitchSpeed;
}

void Turret::SetYaw( double yaw )
{
	if( yaw > 1. )
		yaw = 1.;
	else if( yaw < -1. )
		yaw = -1.;
	
	YawRate = yaw * YawSpeed;
}


Pos3D Turret::GunPos( const Pos3D *body_pos ) const
{
	if( ! body_pos )
		body_pos = this;
	GameObject gun;
	gun.Copy( body_pos );
	gun.MoveAlong( &(body_pos->Up),  GunUp  );
	gun.MoveAlong( &(body_pos->Fwd), GunFwd );
	gun.Pitch( GunPitch );
	return gun;
}


Pos3D Turret::HeadPos( void ) const
{
	Pos3D head = GunPos();
	head.MoveAlong( &(head.Up),  HeadUp  );
	head.MoveAlong( &(head.Fwd), HeadFwd );
	return head;
}


std::map<int,Shot*> Turret::NextShots( GameObject *target, uint8_t firing_mode ) const
{
	std::map<int,Shot*> shots;
	
	Pos3D gun = GunPos();
	
	if( ! firing_mode )
		firing_mode = FiringMode;
	
	uint16_t player_id = PlayerID;
	if( ParentID && ! PlayerID )
	{
		Ship *parent = ParentShip();
		if( parent )
			player_id = parent->PlayerID;
	}
	
	for( int num = 0; num < (int) firing_mode; num ++ )
	{
		int weapon_index = (WeaponIndex + num) % 2;
		
		Shot *shot = new Shot();
		shot->Copy( &gun );
		shot->FiredFrom = ID;
		shot->WeaponIndex = weapon_index;
		shot->PlayerID = player_id;
		shot->ShotType = Weapon;
		shot->MotionVector.Set( shot->Fwd.X, shot->Fwd.Y, shot->Fwd.Z );
		shot->MotionVector.ScaleTo( shot->Speed() );
		
		// Apply ship motion to attached turrets.
		double speed = MotionVector.Length();
		if( speed && (speed < shot->Speed()) )
		{
			shot->MotionVector += MotionVector;
			shot->MotionVector.ScaleTo( shot->Speed() );
			shot->Fwd = shot->Fwd.Unit();
			shot->FixVectors();
		}
		
		shot->MoveAlong( &Right, weapon_index ? -GunWidth : GunWidth ); // FIXME: Make turret weapon positions customizable?
		
		shots[ weapon_index ] = shot;
	}
	
	return shots;
}


std::map<int,Shot*> Turret::AllShots( GameObject *target ) const
{
	return NextShots( target, GunWidth ? 2 : 1 ); // FIXME: Make turret weapon positions customizable?
}


void Turret::JustFired( void )
{
	FiringClock.Reset();
	
	if( ! FiredThisFrame )
	{
		WeaponIndex ++;
		if( WeaponIndex >= 2 )
			WeaponIndex = 0;
	}
	
	FiredThisFrame ++;
}


double Turret::ShotDelay( void ) const
{
	return SingleShotDelay * FiringMode;
}


bool Turret::PlayerShouldUpdateServer( void ) const
{
	return (Health > 0.);  // FIXME: Not when parent ship has jumped out?
}

bool Turret::ServerShouldUpdatePlayer( void ) const
{
	return false;
}

bool Turret::ServerShouldUpdateOthers( void ) const
{
	return true;  // FIXME: Not when parent ship has jumped out?
}

bool Turret::CanCollideWithOwnType( void ) const
{
	return false;
}

bool Turret::CanCollideWithOtherTypes( void ) const
{
	const Ship *parent = ParentShip();
	if( parent && (parent->JumpedOut || (parent->JumpProgress < 1.)) )
		return false;
	
	return CanBeHit;
}

bool Turret::IsMoving( void ) const
{
	if( ! ParentID )
		return false;
	
	return GameObject::IsMoving();
}


void Turret::AddToInitPacket( Packet *packet, int8_t precision )
{
	packet->AddUChar( Team );
	packet->AddUChar( Weapon );
	packet->AddFloat( SingleShotDelay );
	packet->AddUInt( ParentID );
	if( ParentID )
	{
		packet->AddFloat( Offset.X );
		packet->AddFloat( Offset.Y );
		packet->AddFloat( Offset.Z );
		packet->AddFloat( RelativeUp.X );
		packet->AddFloat( RelativeUp.Y );
		packet->AddFloat( RelativeUp.Z );
		packet->AddShort( Num::UnitFloatTo16(Fwd.X) );
		packet->AddShort( Num::UnitFloatTo16(Fwd.Y) );
		packet->AddShort( Num::UnitFloatTo16(Fwd.Z) );
	}
	else
		GameObject::AddToUpdatePacketFromServer( packet, -127 );
	
	// Compress GunPitch in range (-180,180) to int16.
	double unit_gun_pitch = GunPitch / 180.;
	while( unit_gun_pitch > 1. )
		unit_gun_pitch -= 2.;
	while( unit_gun_pitch < -1. )
		unit_gun_pitch += 2.;
	packet->AddShort( Num::UnitFloatTo16(unit_gun_pitch) );
	
	packet->AddChar( Num::Clamp( MinGunPitch, -127, 128 ) );
	packet->AddChar( Num::Clamp( MaxGunPitch, -127, 128 ) );
	
	packet->AddFloat( Health );
	
	uint8_t flags = (Visible ? 0x01 : 0x00) | (ParentControl ? 0x02 : 0x00) | (CanBeHit ? 0x04 : 0x00);
	packet->AddUChar( flags );
	
	Vec3D target_dir = TargetDir.Unit();
	packet->AddChar( Num::UnitFloatTo8( target_dir.X ) );
	packet->AddChar( Num::UnitFloatTo8( target_dir.Y ) );
	packet->AddChar( Num::UnitFloatTo8( target_dir.Z ) );
	packet->AddUChar( Num::Clamp( TargetArc / 2. + 0.5, 0, 255 ) );
	
	packet->AddUChar( YawSpeed   );
	packet->AddUChar( PitchSpeed );
	
	packet->AddFloat( GunWidth );
	packet->AddFloat( GunUp    );
	packet->AddFloat( GunFwd   );
	packet->AddFloat( HeadUp   );
	packet->AddFloat( HeadFwd  );
	
	if( (BodyModel == "turret_body.obj") && (GunModel == "turret_body.obj") )
	{
		packet->AddString( "" );
		packet->AddString( "" );
	}
	else
	{
		packet->AddString( BodyModel );
		packet->AddString( GunModel  );
	}
}


void Turret::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	Team = packet->NextUChar();
	Weapon = packet->NextUChar();
	SingleShotDelay = packet->NextFloat();
	ParentID = packet->NextUInt();
	if( ParentID )
	{
		Offset.X = packet->NextFloat();
		Offset.Y = packet->NextFloat();
		Offset.Z = packet->NextFloat();
		RelativeUp.X = packet->NextFloat();
		RelativeUp.Y = packet->NextFloat();
		RelativeUp.Z = packet->NextFloat();
		Fwd.X = Num::UnitFloatFrom16( packet->NextShort() );
		Fwd.Y = Num::UnitFloatFrom16( packet->NextShort() );
		Fwd.Z = Num::UnitFloatFrom16( packet->NextShort() );
		UpdatePos();
	}
	else
	{
		bool smooth_pos = SmoothPos;
		SmoothPos = false;
		GameObject::ReadFromUpdatePacketFromServer( packet, -127 );
		SmoothPos = smooth_pos;
	}
	
	// Extact GunPitch in range (-180,180) from int16.
	int16_t compressed_gun_pitch = packet->NextShort();
	GunPitch = Num::UnitFloatFrom16(compressed_gun_pitch) * 180.;
	
	MinGunPitch = packet->NextChar();
	MaxGunPitch = packet->NextChar();
	
	SetHealth( packet->NextFloat() );
	
	uint8_t flags = packet->NextUChar();
	Visible       = flags & 0x01;
	ParentControl = flags & 0x02;
	CanBeHit      = flags & 0x04;
	
	TargetDir.X = Num::UnitFloatFrom8( packet->NextChar() );
	TargetDir.Y = Num::UnitFloatFrom8( packet->NextChar() );
	TargetDir.Z = Num::UnitFloatFrom8( packet->NextChar() );
	TargetArc = packet->NextUChar() * 2.;
	
	YawSpeed   = packet->NextUChar();
	PitchSpeed = packet->NextUChar();
	
	GunWidth   = packet->NextFloat();
	GunUp      = packet->NextFloat();
	GunFwd     = packet->NextFloat();
	HeadUp     = packet->NextFloat();
	HeadFwd    = packet->NextFloat();
	
	BodyModel = packet->NextString();
	GunModel  = packet->NextString();
	if( BodyModel.empty() && GunModel.empty() )
	{
		BodyModel = "turret_body.obj";
		GunModel  = "turret_gun.obj";
	}
}


void Turret::AddToUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	packet->AddShort( Num::UnitFloatTo16(Fwd.X) );
	packet->AddShort( Num::UnitFloatTo16(Fwd.Y) );
	packet->AddShort( Num::UnitFloatTo16(Fwd.Z) );
	
	// Compress GunPitch in range (-180,180) to int16.
	double unit_gun_pitch = GunPitch / 180.;
	while( unit_gun_pitch > 1. )
		unit_gun_pitch -= 2.;
	while( unit_gun_pitch < -1. )
		unit_gun_pitch += 2.;
	packet->AddShort( Num::UnitFloatTo16(unit_gun_pitch) );
	
	if( precision >= 0 )
	{
		packet->AddFloat( YawRate );
		packet->AddFloat( GunPitchRate );
	}
	
	packet->AddFloat( Health );
	packet->AddUInt( Target );
	
	if( precision >= 126 )  // FIXME: Maybe this should be its own packet type?
	{
		packet->AddUShort( PlayerID );
		packet->AddUChar( FiringMode );
	}
}


void Turret::ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	Fwd.X = Num::UnitFloatFrom16( packet->NextShort() );
	Fwd.Y = Num::UnitFloatFrom16( packet->NextShort() );
	Fwd.Z = Num::UnitFloatFrom16( packet->NextShort() );
	FixVectorsKeepUp();
	
	// Extact GunPitch in range (-180,180) from int16.
	int16_t compressed_gun_pitch = packet->NextShort();
	GunPitch = Num::UnitFloatFrom16(compressed_gun_pitch) * 180.;
	
	if( precision >= 0 )
	{
		YawRate      = packet->NextFloat();
		GunPitchRate = packet->NextFloat();
	}
	
	SetHealth( packet->NextFloat() );
	Target = packet->NextUInt();
	
	UpdatePos();
	
	if( precision >= 126 )  // FIXME: Maybe this should be its own packet type?
	{
		PlayerID = packet->NextUShort();
		FiringMode = packet->NextUChar();
		
		if( PlayerID == Raptor::Game->PlayerID )
		{
			const Ship *parent = ParentShip();
			if( parent && (parent->JumpProgress < 1.) && (parent->PlayerID != PlayerID) )
				Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("jump_in_cockpit.wav") );
		}
	}
}


void Turret::AddToUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	packet->AddFloat( Fwd.X );
	packet->AddFloat( Fwd.Y );
	packet->AddFloat( Fwd.Z );
	
	packet->AddFloat( GunPitch );
	
	if( precision >= 0 )
	{
		packet->AddFloat( YawRate );
		packet->AddFloat( GunPitchRate );
	}
	
	uint8_t firing_mode = FiringMode;
	if( Firing )
		firing_mode |= 0x80;
	if( ((XWingGame*)( Raptor::Game ))->ZeroLagServer )  // v0.4 server would be confused by ZeroLag data.
	{
		if( PredictedShots )
		{
			firing_mode |= 0x40;  // ZeroLag Shot(s) Fired
			PredictedShots --;
		}
		else if( Firing && Raptor::Game->Cfg.SettingAsInt("net_zerolag",2) )
			firing_mode &= ~0x80;  // If we are using ZeroLag mode for this weapon, only send Firing with each shot.
	}
	packet->AddUChar( firing_mode );
	
	packet->AddUInt( Target );
}


void Turret::ReadFromUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	Fwd.X = packet->NextFloat();
	Fwd.Y = packet->NextFloat();
	Fwd.Z = packet->NextFloat();
	FixVectorsKeepUp();
	
	GunPitch = packet->NextFloat();
	
	if( precision >= 0 )
	{
		YawRate      = packet->NextFloat();
		GunPitchRate = packet->NextFloat();
	}
	
	uint8_t firing_mode = packet->NextUChar();
	Firing            = firing_mode & 0x80;
	bool zerolag_shot = firing_mode & 0x40;
	if( ! PredictedShots )
		FiringMode    = firing_mode & 0x0F;
	if( zerolag_shot )
		PredictedShots ++;
	
	Target = packet->NextUInt();
}


bool Turret::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object, Pos3D *loc, double *when ) const
{
	// Can't collide with the ship we're attached to.
	if( other->ID == ParentID )
		return false;
	
	// Hidden embedded turrets don't collide.
	if( ! CanBeHit )
		return false;
	
	// Dead turrets don't collide.
	if( Health <= 0. )
		return false;
	
	if( other->Type() == XWing::Object::SHOT )
	{
		// Turrets can't shoot themselves, and neither can fixed guns on their ship.
		// NOTE: Because shots are now owned by turrets, they can commit friendly fire to other turrets on their ship!
		Shot *shot = (Shot*) other;
		if( (! shot->FiredFrom) || (shot->FiredFrom == ID) || (shot->FiredFrom == ParentID) )
			return false;
		
		double dist = Math3D::MinimumDistance( this, &(this->MotionVector), other, &(other->MotionVector), dt );
		
		if( dist <= Radius )
			return true;
	}
	
	else if( other->Type() == XWing::Object::SHIP )
	{
		// Dead ships don't collide after a bit, and they never hit other dead ships.
		Ship *ship = (Ship*) other;
		if( (ship->Health <= 0.) && ((Health <= 0.) || (ship->DeathClock.ElapsedSeconds() > 0.5)) )
			return false;
		
		// Don't let turrets hit capitol ships.
		if( ship->ComplexCollisionDetection() )
			return false;
		
		double dist = Math3D::MinimumDistance( this, &(this->MotionVector), other, &(other->MotionVector), dt );
		
		if( dist <= (ship->Radius() + Radius) )
			return true;
	}
	
	/*
	// Let the Death Star determine whether collisions with turrets occur.
	// This could be used to scrape off attached turrets when ships fly too close.
	else if( other->Type() == XWing::Object::DEATH_STAR )
		return other->WillCollide( this, dt, other_object, this_object, loc, when );
	*/
	
	return false;
}


void Turret::Update( double dt )
{
	FiringClock.SetTimeScale( Data->TimeScale );
	
	FiredThisFrame = 0;
	
	const Ship *parent = ParentShip();
	if( parent )
	{
		Team = parent->Team;
		if( parent->Health <= 0. ) // Parent is dead.
			parent = NULL;
	}
	if( ParentID && (! parent) && ! ClientSide() )
	{
		SetHealth( 0. ); // Commit sudoku.
		ParentID = 0;    // Forget dead parent.
	}
	
	if( (Health <= 0.) || (parent && ((parent->JumpProgress < 1.) || parent->JumpedOut)) )
	{
		// Dead turrets don't turn.
		RollRate = 0.;
		PitchRate = 0.;
		YawRate = 0.;
		GunPitchRate = 0.;
		
		PredictedShots = 0;
	}
	
	if( GunPitchRate )
		PitchGun( GunPitchRate * dt );
	
	GameObject::Update( dt );
	UpdatePos();
}


void Turret::Draw( void )
{
	Draw( true );
}


void Turret::Draw( bool allow_shader_change )
{
	UpdatePos();
	
	if( (Health > 0.) && Visible )
	{
		XWingGame *game = (XWingGame*) Raptor::Game;
		Pos3D pos = *this;
		
		// If attached to a ship jumping in from hyperspace, the turrets jump in with it.
		const Ship *parent = ParentShip();
		if( parent && ((parent->JumpProgress < 1.) || parent->JumpedOut) )
		{
			if( parent->JumpedOut && (parent->JumpProgress > 1.) )
				return;
			
			if( (parent->PlayerID == Raptor::Game->PlayerID) && (game->View != XWing::View::GUNNER) )
				pos.MoveAlong( &(parent->Fwd), parent->CockpitDrawOffset() );
			else
				pos.MoveAlong( &(parent->Fwd), parent->DrawOffset() );
		}
		
		bool change_shader = allow_shader_change && (! ParentID) && (Raptor::Game->Cfg.SettingAsInt("g_shader_light_quality") >= 2) && Raptor::Game->ShaderMgr.Active();
		Shader *prev_shader = Raptor::Game->ShaderMgr.Selected;
		if( change_shader )
			Raptor::Game->ShaderMgr.SelectAndCopyVars( Raptor::Game->Res.GetShader("deathstar") );
		
		if( game->View == XWing::View::GUNNER )
			pos.Yaw( YawRate * -0.004 );
		
		if( BodyShape )
			BodyShape->DrawAt( &pos, 0.022 );
		
		if( GunShape )
		{
			Pos3D gun = GunPos( &pos );
			
			if( game->View == XWing::View::GUNNER )
				gun.Pitch( GunPitchRate * -0.004 );
			
			GunShape->DrawAt( &gun, 0.022 );
		}
		
		if( change_shader )
			Raptor::Game->ShaderMgr.Select( prev_shader );
	}
}


void Turret::DrawWireframe( Color *color, double scale )
{
	if( (Health > 0.) && Visible )
	{
		Color default_color(0.5,0.5,1,1);
		if( ! color )
			color = &default_color;
		
		Pos3D pos( this );
		//pos.MoveAlong( &Up, GunUp * -1. );  // FIXME: RenderLayer needs to call something like DrawWireframeAt with corresponding DrawAt.
		
		if( BodyShape )
			BodyShape->Draw( &pos, NULL, color, 0., 0, scale, scale, scale );
		
		if( GunShape )
		{
			Pos3D gun = GunPos( &pos );
			GunShape->Draw( &gun, NULL, color, 0., 0, scale, scale, scale );
		}
	}
}
