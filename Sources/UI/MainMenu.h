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
	double FogTime, IdleTime;
	
	bool NeedPrecache;
	volatile bool Loading;
	Font *LoadingFont;
	
	Font *TitleFont, *TitleFontBig, *TitleFontSmall;
	Font *VersionFont;
	Font *ButtonFont;
	LabelledButton *StartButton, *AButton, *XButton, *YButton;
	
	MainMenu( void );
	virtual ~MainMenu();
	
	void UpdateRects( void );
	void Draw( void );
	void DrawElements( void );
	bool HandleEvent( SDL_Event *event );
};


class MainMenuCampaignButton : public LabelledButton
{
public:
	MainMenuCampaignButton( SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~MainMenuCampaignButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class MainMenuOnlineButton : public LabelledButton
{
public:
	MainMenuOnlineButton( SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~MainMenuOnlineButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class MainMenuCustomButton : public LabelledButton
{
public:
	MainMenuCustomButton( SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~MainMenuCustomButton();
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
