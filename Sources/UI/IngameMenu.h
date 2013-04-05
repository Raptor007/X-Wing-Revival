/*
 *  IngameMenu.h
 */

#pragma once
class IngameMenu;
class IngameMenuResumeButton;
class IngameMenuPrefsButton;
class IngameMenuLeaveButton;

#include "platforms.h"

#include "Layer.h"
#include "Animation.h"
#include "Font.h"
#include "LabelledButton.h"


class IngameMenu : public Layer
{
public:
	Animation Background;
	Font *TitleFont, *TitleFontSmall;
	Font *VersionFont;
	Font *ButtonFont;
	IngameMenuResumeButton *ResumeButton;
	IngameMenuPrefsButton *PrefsButton;
	IngameMenuLeaveButton *LeaveButton;
	
	IngameMenu( void );
	virtual ~IngameMenu();
	
	void UpdateRects( void );
	void Draw( void );
	
	bool KeyDown( SDLKey key );
};


class IngameMenuResumeButton : public LabelledButton
{
public:
	IngameMenuResumeButton( IngameMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class IngameMenuPrefsButton : public LabelledButton
{
public:
	IngameMenuPrefsButton( IngameMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class IngameMenuLeaveButton : public LabelledButton
{
public:
	IngameMenuLeaveButton( IngameMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
