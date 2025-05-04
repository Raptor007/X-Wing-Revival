/*
 *  XWingGame.h
 */

#pragma once
class XWingGame;

#include "PlatformSpecific.h"

#include "RaptorGame.h"
#include "XWingDefs.h"
#include "ShipClass.h"
#include "Shot.h"


class XWingGame : public RaptorGame
{
public:
	std::map<std::string,std::string> MissionList;
	std::set<uint32_t> ServerFeatures;
	
	uint32_t GameType;
	Clock RoundTimer, RespawnTimer;
	uint8_t CampaignTeam;
	
	uint32_t ObservedShipID;
	uint8_t View;
	double LookYaw, LookPitch;
	bool ThumbstickLook;
	uint8_t TurretFiringMode;
	uint32_t StoredTargets[ 4 ];
	uint8_t StoredSubsystems[ 4 ];
	std::map< uint8_t, std::map< int, std::deque<Shot*> > > ClientShots;
	
	double AsteroidLOD;
	double OverlayScroll;
	Clock EjectHeld;
	size_t BlastPoints;
	
	uint16_t Victor;
	std::queue<std::string> Achievements;
	Clock AchievementClock;
	
	uint8_t Controls[ XWing::Control::COUNT ];
	
	
	XWingGame( std::string version );
	virtual ~XWingGame();
	
	void SetDefaultJoyTypes( void );
	void SetDefaultControls( void );
	void SetDefaults( void );
	void Setup( int argc, char **argv );
	void Precache( void );
	void AddScreensaverLayer( void );
	
	void Update( double dt );
	bool HandleEvent( SDL_Event *event );
	bool HandleCommand( std::string cmd, std::vector<std::string> *params = NULL );
	bool ProcessPacket( Packet *packet );
	void MessageReceived( std::string text, uint32_t type = TextConsole::MSG_NORMAL );
	
	void ChangeState( int state );
	void Disconnected( void );
	void ShowLobby( void );
	void BeginFlying( void );
	
	GameObject *NewObject( uint32_t id, uint32_t type );
	
	const ShipClass *GetShipClass( const std::string &name ) const;
};
