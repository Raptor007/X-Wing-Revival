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
#include "Mission.h"


class XWingServer : public RaptorServer
{
public:
	std::set<ShipClass> ShipClasses;
	std::map<std::string,Mission> Missions;
	
	uint32_t GameType;
	uint8_t PlayerTeam;
	std::map<std::string,std::string> Properties;
	int KillLimit;
	int TimeLimit;
	int Checkpoints;
	bool CheckpointFirstTouch;
	bool PlayersTakeEmptyShips;
	bool Respawn;
	double RespawnDelay, RebelCruiserRespawn, EmpireCruiserRespawn;
	std::map<uint16_t,Clock> RespawnClocks;
	bool AllowShipChange, AllowTeamChange;
	uint32_t DefendingTeam;
	std::map<double,XWingServerAlert> Alerts;
	std::map< uint8_t, std::vector<MissionEvent> > EventTriggers;
	std::map< uint8_t, std::vector<Pos3D> > Waypoints;
	std::map< std::string, std::set<uint32_t> > Squadrons;
	
	uint32_t CollisionGroup;
	std::map<uint32_t,uint32_t> CollisionWith;
	//std::map<uint32_t,Pos3D> CollisionAt;
	
	Clock RoundTimer;
	Clock RoundEndedTimer;
	double RoundEndedDelay;
	std::map< uint8_t, std::map<uint8_t,Pos3D> > GroupSpawns;
	std::map<uint8_t,Clock> GroupStagger; // FIXME: Separate these by team?
	std::set<uint32_t> GroupJumpingIn;    //
	std::map<uint8_t,int> TeamScores;
	std::map<uint32_t,int> ShipScores;
	std::set<uint16_t> Cheaters;
	
	Clock CountdownTimer;
	int CountdownFrom;
	int CountdownSent;
	
	XWingServer( std::string version );
	virtual ~XWingServer();
	
	std::map<std::string,std::string> DefaultProperties( void ) const;
	void ResetToDefaultProperties( void );
	
	void Started( void );
	void Stopped( void );
	bool HandleCommand( std::string cmd, std::vector<std::string> *params = NULL );
	bool ProcessPacket( Packet *packet, ConnectedClient *from_client );
	bool CompatibleVersion( std::string version ) const;
	void AcceptedClient( ConnectedClient *client );
	void DroppedClient( ConnectedClient *client );
	bool SetPlayerProperty( Player *player, std::string name, std::string value, bool force = false );
	
	void Update( double dt );
	void SetProperty( std::string name, std::string value );
	
	void ResetToStartingObjects( void );
	void ClearMissionData( void );
	bool SelectMission( std::string mission_id = "", bool load = true );
	void ToggleCountdown( const Player *player = NULL );
	uint32_t ParseGameType( std::string gametype ) const;
	void BeginFlying( uint16_t player_id = 0, bool respawn = false );
	
	void ShipKilled( Ship *ship, GameObject *killer_obj = NULL, Player *killer = NULL );
	void SendScores( void );
	
	double RoundTimeRemaining( void ) const;
	
	const ShipClass *GetShipClass( const std::string &name ) const;
	Ship *SpawnShip( const ShipClass *ship_class, uint8_t team, std::set<uint32_t> *add_object_ids = NULL );
	void SpawnShipTurrets( const Ship *ship, std::set<uint32_t> *add_object_ids = NULL );
	void SpawnShipDockingBays( const Ship *ship );
	
	void SendAddedObjects( const std::set<uint32_t> *add_object_ids );
	
	void TriggerEvent( uint8_t trigger, const GameObject *object, const GameObject *by_obj = NULL, const Player *player = NULL, const Player *by_player = NULL );
	void TriggerEvent( uint8_t trigger, uint32_t flags, std::string target_name = "", uint8_t target_group = 0, std::string by_name = "", uint8_t by_group = 0 );
	
	bool CheckCondition( const std::vector<std::string> &terms );
	std::set<Ship*> MatchingShips( const std::vector<std::string> &terms );
	
	std::string ChosenShip( const Player *player, bool allow_spectator = true );
	std::string ChosenTeam( const Player *player, bool allow_spectator = true );
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
