/*
 *  IngameMenu.h
 */

#pragma once
class IngameMenu;
class IngameMenuResumeButton;
class IngameMenuPrefsButton;
class IngameMenuHelpButton;
class IngameMenuLeaveButton;
class IngameMenuShipDropDown;
class IngameMenuGroupDropDown;

#include "PlatformSpecific.h"

#include "Layer.h"
#include "Animation.h"
#include "Font.h"
#include "LabelledButton.h"
#include "DropDown.h"


class IngameMenu : public Layer
{
public:
	Animation Background;
	Font *TitleFont, *TitleFontBig, *TitleFontSmall;
	Font *VersionFont;
	Font *ButtonFont;
	
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


class IngameMenuShipDropDown : public DropDown
{
public:
	IngameMenuShipDropDown( SDL_Rect *rect, Font *font, uint8_t align = Font::ALIGN_MIDDLE_LEFT, int scroll_bar_size = 0 );
	virtual ~IngameMenuShipDropDown();
	void Changed( void );
};


class IngameMenuGroupDropDown : public DropDown
{
public:
	IngameMenuGroupDropDown( SDL_Rect *rect, Font *font, uint8_t align = Font::ALIGN_MIDDLE_LEFT, int scroll_bar_size = 0 );
	virtual ~IngameMenuGroupDropDown();
	void Changed( void );
};


class IngameMenuViewDropDown : public DropDown
{
public:
	IngameMenuViewDropDown( SDL_Rect *rect, Font *font, uint8_t align = Font::ALIGN_MIDDLE_LEFT, int scroll_bar_size = 0 );
	virtual ~IngameMenuViewDropDown();
	void Changed( void );
};
