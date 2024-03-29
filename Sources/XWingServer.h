/*
 *  XWingServer.h
 */

#pragma once
class XWingServer;
class XWingServerAlert;

#include "PlatformSpecific.h"

#include "RaptorServer.h"
#include "Ship.h"
#include "ShipClass.h"


class XWingServer : public RaptorServer
{
public:
	std::set<ShipClass> ShipClasses;
	
	uint32_t GameType;
	int KillLimit;
	int TimeLimit;
	int Checkpoints;
	bool PlayersTakeEmptyShips;
	bool Respawn;
	double RespawnDelay, RebelCruiserRespawn, EmpireCruiserRespawn;
	std::map<uint16_t,Clock> RespawnClocks;
	bool AllowShipChange, AllowTeamChange;
	uint32_t DefendingTeam;
	std::map<double,XWingServerAlert> Alerts;
	std::map< uint8_t, std::vector<Pos3D> > Waypoints;
	std::map< std::string, std::set<uint32_t> > Squadrons;
	
	Clock RoundTimer;
	Clock RoundEndedTimer;
	double RoundEndedDelay;
	std::map<uint8_t,int> TeamScores;
	std::map<uint32_t,int> ShipScores;
	std::set<uint16_t> Cheaters;
	
	Clock CountdownTimer;
	int CountdownFrom;
	int CountdownSent;
	
	XWingServer( std::string version );
	virtual ~XWingServer();
	
	void Started( void );
	void Stopped( void );
	bool ProcessPacket( Packet *packet, ConnectedClient *from_client );
	bool CompatibleVersion( std::string version ) const;
	void AcceptedClient( ConnectedClient *client );
	void DroppedClient( ConnectedClient *client );
	
	void Update( double dt );
	
	void ResetToStartingObjects( void );
	void ToggleCountdown( void );
	void BeginFlying( uint16_t player_id = 0, bool respawn = false );
	
	void ShipKilled( Ship *ship, GameObject *killer_obj = NULL, Player *killer = NULL );
	void SendScores( void );
	
	double RoundTimeRemaining( void ) const;
	
	const ShipClass *GetShipClass( const std::string &name ) const;
	Ship *SpawnShip( const ShipClass *ship_class, uint8_t team, std::set<uint32_t> *add_object_ids = NULL );
	void SpawnShipTurrets( const Ship *ship, std::set<uint32_t> *add_object_ids = NULL );
	void SpawnShipDockingBays( const Ship *ship );
};


class XWingServerAlert
{
public:
	std::string Sound;
	std::string Message;
	
	XWingServerAlert( void );
	XWingServerAlert( std::string sound, std::string message );
	~XWingServerAlert();
};
