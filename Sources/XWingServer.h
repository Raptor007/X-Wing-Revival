/*
 *  XWingServer.h
 */

#pragma once
class XWingServer;
class XWingServerAlert;

#include "platforms.h"

#include "RaptorServer.h"


class XWingServer : public RaptorServer
{
public:
	uint32_t GameType;
	int KillLimit;
	int TimeLimit;
	bool PlayersTakeEmptyShips;
	bool Respawn;
	bool AIFlock;
	std::map<double,XWingServerAlert> Alerts;
	std::map< uint8_t, std::vector<Pos3D> > Waypoints;
	std::map< std::string, std::set<uint32_t> > Squadrons;
	
	Clock RoundTimer;
	Clock RoundEndedTimer;
	double RoundEndedDelay;
	std::map<uint32_t,int> TeamScores;
	std::map<uint32_t,int> ShipScores;
	
	Clock CountdownTimer;
	int CountdownFrom;
	int CountdownSent;
	
	XWingServer( std::string version );
	virtual ~XWingServer();
	
	void Started( void );
	bool ProcessPacket( Packet *packet );
	void AcceptedClient( ConnectedClient *client );
	void DroppedClient( ConnectedClient *client );
	
	void Update( double dt );
	
	void ToggleCountdown( void );
	void BeginFlying( void );
	void SendScores( void );

	double RoundTimeRemaining( void );
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
