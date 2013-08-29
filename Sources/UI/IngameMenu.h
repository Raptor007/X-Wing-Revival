/*
 *  IngameMenu.h
 */

#pragma once
class IngameMenu;
class IngameMenuResumeButton;
class IngameMenuPrefsButton;
class IngameMenuHelpButton;
class IngameMenuLeaveButton;

#include "PlatformSpecific.h"

#include "Layer.h"
#include "Animation.h"
#include "Font.h"
#include "LabelledButton.h"


class IngameMenu : public Layer
{
public:
	Animation Background;
	Font *TitleFont, *TitleFontBig, *TitleFontSmall;
	Font *VersionFont;
	Font *ButtonFont;
	IngameMenuResumeButton *ResumeButton;
	IngameMenuPrefsButton *PrefsButton;
	IngameMenuHelpButton *HelpButton;
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
	IngameMenuResumeButton( SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~IngameMenuResumeButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class IngameMenuPrefsButton : public LabelledButton
{
public:
	IngameMenuPrefsButton( SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~IngameMenuPrefsButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class IngameMenuHelpButton : public LabelledButton
{
public:
	IngameMenuHelpButton( SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~IngameMenuHelpButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class IngameMenuLeaveButton : public LabelledButton
{
public:
	IngameMenuLeaveButton( SDL_Rect *rect, Font *button_font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~IngameMenuLeaveButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
