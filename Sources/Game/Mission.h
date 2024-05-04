/*
 *  Mission.h
 */

#pragma once
class Mission;
class MissionEvent;

#include "PlatformSpecific.h"

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "XWingDefs.h"
#include "Ship.h"


class Mission
{
public:
	std::map<std::string,std::string> Properties;
	std::vector<MissionEvent> Events;
	
	bool Load( const std::string &filename );
};


class MissionEvent
{
public:
	uint8_t Trigger, TargetGroup;
	uint16_t TriggerFlags;
	size_t Number;
	double Time, Delay, Chance;
	std::string Target;
	std::vector<std::string> TriggerIf;
	
	std::string Message, Sound;
	uint32_t MessageType;
	
	std::string SpawnClass, SpawnName;
	double X, Y, Z, FwdX, FwdY, FwdZ;
	uint8_t SpawnGroup, SpawnFlags;
	
	std::vector<std::string> JumpOut;
	
	std::string PropertyName, PropertyValue;
	
	//std::string ID;
	//bool Enabled;
	size_t Triggered, Used;
	double GoTime;
	
	
	MissionEvent( uint8_t trigger, uint16_t flags = 0x0000, double time = 0., double delay = 0., size_t number = 0, std::string target = "", uint8_t target_group = 0, std::vector<std::string> trigger_if = std::vector<std::string>(), double chance = 1. );
	MissionEvent( const MissionEvent &other );
	virtual ~MissionEvent();
	
	bool MatchesConditions( uint8_t team = XWing::Team::NONE, uint8_t group = 0, bool objective = false, uint16_t player_id = 0, std::string name = "" ) const;
	void Activated( uint8_t team = XWing::Team::NONE, uint8_t group = 0, bool objective = false, uint16_t player_id = 0, std::string name = "" );
	
	bool Ready( void );
	void FireWhenReady( std::set<uint32_t> *add_object_ids );
	
	enum
	{
		TRIGGER_NEVER = 0,
		TRIGGER_ALWAYS,
		TRIGGER_ON_VICTORY,
		TRIGGER_ON_DEFEAT,
		TRIGGER_ON_CHECKPOINT,
		TRIGGER_ON_FIRE,
		TRIGGER_ON_HIT,
		TRIGGER_ON_DAMAGE,
		TRIGGER_ON_DESTROYED
	};
	
	enum
	{
		TRIGGERFLAG_ONLY_REBEL     = 0x0001,
		TRIGGERFLAG_ONLY_EMPIRE    = 0x0002,
		TRIGGERFLAG_ONLY_PLAYER    = 0x0004,
		TRIGGERFLAG_ONLY_AI        = 0x0008,
		TRIGGERFLAG_ONLY_OBJECTIVE = 0x0010,
		TRIGGERFLAG_ONLY_GROUP     = 0x0020,
		TRIGGERFLAG_TIME_REMAINING = 0x0040,
		TRIGGERFLAG_REPEAT         = 0x0080,
		TRIGGERFLAG_RECHECK_IF     = 0x0100
	};
	
	enum
	{
		SPAWNFLAG_REBEL     = 0x01,
		SPAWNFLAG_EMPIRE    = 0x02,
		SPAWNFLAG_OBJECTIVE = 0x04,
		//SPAWNFLAG_HERO      = 0x08,
		SPAWNFLAG_BY_PLAYER = 0x10
	};
};
