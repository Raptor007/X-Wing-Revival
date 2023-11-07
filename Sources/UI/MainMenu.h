/*
 *  MainMenu.h
 */

#pragma once
class MainMenu;
class MainMenuPlayButton;
class MainMenuPrefsButton;
class MainMenuHelpButton;
class MainMenuQuitButton;

#include "PlatformSpecific.h"

#include "Layer.h"
#include "Animation.h"
#include "Font.h"
#include "LabelledButton.h"


class MainMenu : public Layer
{
public:
	Animation Background, Fog;
	double FogTime;
	
	bool NeedPrecache;
	volatile bool Loading;
	Font *LoadingFont;
	
	Font *TitleFont, *TitleFontBig, *TitleFontSmall;
	Font *VersionFont;
	Font *ButtonFont;
	MainMenuPlayButton *PlayButton;
	MainMenuPrefsButton *PrefsButton;
	MainMenuHelpButton *HelpButton;
	MainMenuQuitButton *QuitButton;
	
	MainMenu( void );
	virtual ~MainMenu();
	
	void UpdateRects( void );
	void Draw( void );
	void DrawElements( void );
	bool HandleEvent( SDL_Event *event );
};


class MainMenuPlayButton : public LabelledButton
{
public:
	MainMenuPlayButton( SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~MainMenuPlayButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class MainMenuPrefsButton : public LabelledButton
{
public:
	MainMenuPrefsButton( SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~MainMenuPrefsButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class MainMenuHelpButton : public LabelledButton
{
public:
	MainMenuHelpButton( SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~MainMenuHelpButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class MainMenuQuitButton : public LabelledButton
{
public:
	MainMenuQuitButton( SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~MainMenuQuitButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


int MainMenuPrecacheThread( void *main_menu );
