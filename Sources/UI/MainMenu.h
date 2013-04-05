/*
 *  MainMenu.h
 */

#pragma once
class MainMenu;
class MainMenuPlayButton;
class MainMenuPrefsButton;
class MainMenuQuitButton;

#include "platforms.h"

#include "Layer.h"
#include "Animation.h"
#include "Font.h"
#include "LabelledButton.h"


class MainMenu : public Layer
{
public:
	Animation Background;
	Font *TitleFont, *TitleFontSmall;
	Font *VersionFont;
	Font *ButtonFont;
	MainMenuPlayButton *PlayButton;
	MainMenuPrefsButton *PrefsButton;
	MainMenuQuitButton *QuitButton;
	
	MainMenu( void );
	virtual ~MainMenu();
	
	void UpdateRects( void );
	void Draw( void );
	void DrawElements( void );
	bool HandleEvent( SDL_Event *event, bool already_handled = false );
};


class MainMenuPlayButton : public LabelledButton
{
public:
	MainMenuPlayButton( MainMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class MainMenuPrefsButton : public LabelledButton
{
public:
	MainMenuPrefsButton( MainMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class MainMenuQuitButton : public LabelledButton
{
public:
	MainMenuQuitButton( MainMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
