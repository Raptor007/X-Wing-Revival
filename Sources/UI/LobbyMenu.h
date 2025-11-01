/*
 *  LobbyMenu.h
 */

#pragma once
class LobbyMenu;
class LobbyMenuFlyButton;
class LobbyMenuLeaveButton;
class LobbyMenuPrefsButton;
class LobbyMenuPlayerTextBox;
class LobbyMenuPlayerDropDown;
class LobbyMenuShipDropDown;
class LobbyMenuShipView;
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
#include "DropDown.h"
#include "Model.h"


class LobbyMenu : public Layer
{
public:
	Animation Background;
	Font *TitleFont;
	LobbyMenuFlyButton *FlyButton;
	LobbyMenuLeaveButton *LeaveButton;
	LobbyMenuPrefsButton *PrefsButton;
	LobbyMenuPlayerTextBox *PlayerName;
	LobbyMenuPlayerDropDown *ShipDropDown;
	LobbyMenuPlayerDropDown *GroupDropDown;
	ListBox *PlayerList, *MessageList;
	TextBox *MessageInput;
	LobbyMenuShipView *ShipView;
	std::map<std::string,LobbyMenuConfiguration*> Configs;
	std::vector<LobbyMenuConfiguration*> ConfigOrder;
	
	LobbyMenu( void );
	virtual ~LobbyMenu();
	
	void UpdateRects( void );
	void UpdatePlayerName( void );
	void UpdatePlayerList( void );
	void UpdateMessageList( void );
	void UpdateInfoBoxes( void );
	bool HandleEvent( SDL_Event *event );
	bool KeyDown( SDLKey key );
	bool ControlDown( uint8_t control );
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


class LobbyMenuPrefsButton : public LabelledButton
{
public:
	LobbyMenuPrefsButton( std::string title = "Preferences" );
	virtual ~LobbyMenuPrefsButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class LobbyMenuPlayerTextBox : public TextBox
{
public:
	std::string Variable;
	
	LobbyMenuPlayerTextBox( SDL_Rect *rect, Font *font, uint8_t align, std::string variable );
	virtual ~LobbyMenuPlayerTextBox();
	void Deselected( void );
};


class LobbyMenuPlayerDropDown : public DropDown
{
public:
	std::string Variable;
	
	LobbyMenuPlayerDropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, std::string variable );
	virtual ~LobbyMenuPlayerDropDown();
	void Changed( void );
};


class LobbyMenuShipDropDown : public LobbyMenuPlayerDropDown
{
public:
	LobbyMenuShipDropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, std::string variable );
	virtual ~LobbyMenuShipDropDown();
	void Update( void );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class LobbyMenuShipView : public Layer
{
public:
	Model Shape;
	double Rotation;
	std::string CurrentShip;
	uint8_t CurrentGroup;
	
	LobbyMenuShipView( SDL_Rect *rect = NULL );
	virtual ~LobbyMenuShipView();
	void Update( void );
	void Draw( void );
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
