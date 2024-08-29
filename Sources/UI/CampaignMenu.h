/*
 *  CampaignMenu.h
 */

#pragma once
class CampaignMenu;
class CampaignMenuTeamButton;
class CampaignMenuCloseButton;

#include "PlatformSpecific.h"

#include "Window.h"
#include "Font.h"
#include "LabelledButton.h"
#include "Model.h"


class CampaignMenu : public Window
{
public:
	Font *TitleFont;
	
	CampaignMenu( void );
	virtual ~CampaignMenu();
	bool KeyDown( SDLKey key );
	void Draw( void );
};


class CampaignMenuTeamButton : public Button
{
public:
	uint8_t Team;
	Model Shape;
	double Rotation;
	
	CampaignMenuTeamButton( SDL_Rect *rect, uint8_t team );
	virtual ~CampaignMenuTeamButton();
	void Draw( void );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class CampaignMenuCloseButton : public LabelledButton
{
public:
	CampaignMenuCloseButton( SDL_Rect *rect, Font *button_font, const char *label );
	virtual ~CampaignMenuCloseButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
