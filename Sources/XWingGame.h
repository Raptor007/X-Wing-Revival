/*
 *  XWingGame.h
 */

#pragma once
class XWingGame;

#include "PlatformSpecific.h"

#include "RaptorGame.h"


class XWingGame : public RaptorGame
{
public:
	Layer *Overlay;
	bool ReadKeyboard, ReadMouse;
	Clock RoundTimer;
	uint32_t ObservedShipID;
	double LookYaw, LookPitch;
	
	XWingGame( std::string version );
	virtual ~XWingGame();
	
	void SetDefaults( void );
	void Setup( int argc, char **argv );
	void Precache( void );
	void AddScreensaverLayer( void );
	
	void Update( double dt );
	bool HandleEvent( SDL_Event *event );
	bool ProcessPacket( Packet *packet );
	
	void ChangeState( int state );
	void AddedObject( GameObject *obj );
	
	void Disconnected( void );
	void Connecting( void );
	void Connected( void );
	void ShowLobby( void );
	void BeginFlying( void );
	
	GameObject *NewObject( uint32_t id, uint32_t type );
};
