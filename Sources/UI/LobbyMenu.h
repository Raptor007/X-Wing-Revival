/*
 *  LobbyMenu.h
 */

#pragma once
class LobbyMenu;
class LobbyMenuFlyButton;
class LobbyMenuLeaveButton;
class LobbyMenuTeamButton;
class LobbyMenuShipButton;
class LobbyMenuConfiguration;
class LobbyMenuConfigChangeButton;

#include "PlatformSpecific.h"

#include "Layer.h"
#include "Animation.h"
#include "Font.h"
#include "Label.h"
#include "LabelledButton.h"
#include "ListBox.h"
#include "TextBox.h"


class LobbyMenu : public Layer
{
public:
	Animation Background;
	Font *TitleFont;
	LobbyMenuFlyButton *FlyButton;
	LobbyMenuLeaveButton *LeaveButton;
	LobbyMenuTeamButton *TeamButton;
	LobbyMenuShipButton *ShipButton;
	ListBox *PlayerList, *MessageList;
	TextBox *PlayerName, *MessageInput;
	LobbyMenuConfiguration *GameType, *TDMKillLimit, *DMKillLimit, *AI, *Respawn, *Asteroids, *YavinTimeLimit, *YavinTurrets, *Permissions;
	
	LobbyMenu( void );
	virtual ~LobbyMenu();
	
	void UpdateRects( void );
	void UpdatePlayerName( void );
	void UpdatePlayerList( void );
	void UpdateMessageList( void );
	void UpdateInfoBoxes( void );
	bool KeyDown( SDLKey key );
	void Draw( void );
};


class LobbyMenuFlyButton : public LabelledButton
{
public:
	LobbyMenuFlyButton( void );
	virtual ~LobbyMenuFlyButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class LobbyMenuLeaveButton : public LabelledButton
{
public:
	LobbyMenuLeaveButton( void );
	virtual ~LobbyMenuLeaveButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class LobbyMenuTeamButton : public LabelledButton
{
public:
	LobbyMenuTeamButton( void );
	virtual ~LobbyMenuTeamButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class LobbyMenuShipButton : public LabelledButton
{
public:
	LobbyMenuShipButton( void );
	virtual ~LobbyMenuShipButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class LobbyMenuConfiguration : public Layer
{
public:
	std::string Property;
	Label *Title, *TitleShadow, *Value, *ValueShadow;
	LobbyMenuConfigChangeButton *ChangeButton;
	bool ShowButton;
	
	LobbyMenuConfiguration( std::string property, std::string desc, bool tiny = false );
	virtual ~LobbyMenuConfiguration();
	void Update( void );
};


class LobbyMenuConfigChangeButton : public LabelledButton
{
public:
	LobbyMenuConfigChangeButton( bool tiny = false );
	virtual ~LobbyMenuConfigChangeButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
