/*
 *  FleetMenu.h
 */

#pragma once
class FleetMenu;
class FleetMenuDropDown;
class FleetMenuDoneButton;
class FleetMenuDefaultsButton;

#include "PlatformSpecific.h"

#include "Window.h"
#include "Font.h"
#include "LabelledButton.h"
#include "DropDown.h"
#include <set>


class FleetMenu : public Window
{
public:
	Font *LabelFont, *TitleFont, *ItemFont, *ButtonFont;
	std::set<FleetMenuDropDown*> DropDowns;
	FleetMenuDefaultsButton *DefaultsButton;
	bool Admin;
	
	FleetMenu( void );
	virtual ~FleetMenu();
	
	void UpdateContents( void );
	
	void Draw( void );
	bool KeyDown( SDLKey key );
	bool KeyUp( SDLKey key );
};


class FleetMenuDropDown : public DropDown
{
public:
	FleetMenuDropDown( SDL_Rect *rect, Font *font, std::string variable );
	virtual ~FleetMenuDropDown();
	void Changed( void );
};


class FleetMenuDoneButton : public LabelledButton
{
public:
	FleetMenuDoneButton( SDL_Rect *rect, Font *button_font, const char *label );
	virtual ~FleetMenuDoneButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class FleetMenuDefaultsButton : public LabelledButton
{
public:
	FleetMenuDefaultsButton( SDL_Rect *rect, Font *button_font, const char *label );
	virtual ~FleetMenuDefaultsButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
