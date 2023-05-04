/*
 *  XWingGame.h
 */

#pragma once
class XWingGame;

#include "PlatformSpecific.h"

#include "RaptorGame.h"
#include "XWingDefs.h"
#include "ShipClass.h"


class XWingGame : public RaptorGame
{
public:
	Clock RoundTimer;
	uint32_t ObservedShipID;
	double LookYaw, LookPitch;
	bool ThumbstickLook;
	double AsteroidLOD;
	uint8_t Controls[ XWing::Control::COUNT ];
	
	XWingGame( std::string version );
	virtual ~XWingGame();
	
	void SetDefaultControls( void );
	void SetDefaults( void );
	void Setup( int argc, char **argv );
	void Precache( void );
	void AddScreensaverLayer( void );
	
	void Update( double dt );
	bool HandleEvent( SDL_Event *event );
	bool HandleCommand( std::string cmd, std::vector<std::string> *params = NULL );
	bool ProcessPacket( Packet *packet );
	
	void ChangeState( int state );
	void Disconnected( void );
	void ShowLobby( void );
	void BeginFlying( void );
	
	GameObject *NewObject( uint32_t id, uint32_t type );
	
	const ShipClass *GetShipClass( const std::string &name ) const;
};
