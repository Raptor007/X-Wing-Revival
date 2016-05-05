/*
 *  LobbyMenu.h
 */

#pragma once
class LobbyMenu;
class LobbyMenuFlyButton;
class LobbyMenuLeaveButton;
class LobbyMenuTeamButton;
class LobbyMenuGroupButton;
class LobbyMenuShipButton;
class LobbyMenuConfiguration;
class LobbyMenuConfigChangeButton;

#include "PlatformSpecific.h"

#include <map>
#include <vector>
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
	LobbyMenuGroupButton *GroupButton;
	LobbyMenuShipButton *ShipButton;
	ListBox *PlayerList, *MessageList;
	TextBox *PlayerName, *MessageInput;
	std::map<std::string,LobbyMenuConfiguration*> Configs;
	std::vector<LobbyMenuConfiguration*> ConfigOrder;
	
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
	LobbyMenuTeamButton( int font_size = 17 );
	virtual ~LobbyMenuTeamButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class LobbyMenuGroupButton : public LabelledButton
{
public:
	LobbyMenuGroupButton( int font_size = 17 );
	virtual ~LobbyMenuGroupButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class LobbyMenuShipButton : public LabelledButton
{
public:
	LobbyMenuShipButton( int font_size = 17 );
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
	
	LobbyMenuConfiguration( std::string property, std::string desc, int label_size, int value_size );
	virtual ~LobbyMenuConfiguration();
	void Update( void );
};


class LobbyMenuConfigChangeButton : public LabelledButton
{
public:
	LobbyMenuConfigChangeButton( Font *font, uint8_t align );
	virtual ~LobbyMenuConfigChangeButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
