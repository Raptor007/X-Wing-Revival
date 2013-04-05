/*
 *  JoinMenu.h
 */

#pragma once
class JoinMenu;
class JoinMenuInputHandler;
class JoinMenuTextBox;
class JoinMenuListBox;
class JoinMenuGoButton;
class JoinMenuHostButton;
class JoinMenuBackButton;

#include "platforms.h"

#include <SDL/SDL.h>
#include <SDL_ttf/SDL_ttf.h>
#include "Window.h"
#include "LabelledButton.h"
#include "TextBox.h"
#include "ListBox.h"
#include "Font.h"
#include "ClientConfig.h"
#include "NetUDP.h"


class JoinMenu : public Window
{
public:
	Font *TitleFont, *ItemFont, *ButtonFont;
	JoinMenuListBox *ServerList;
	NetUDP ServerFinder;
	
	JoinMenu( void );
	virtual ~JoinMenu();
	
	void Draw( void );
};


class JoinMenuInputHandler : public Layer
{
public:
	JoinMenuInputHandler( JoinMenu *menu );
	void Draw( void );
	bool HandleEvent( SDL_Event *event, bool already_handled = false );
};


class JoinMenuTextBox : public TextBox
{
public:
	std::string Variable;
	ListBox *LinkedListBox;
	
	JoinMenuTextBox( Window *wind, SDL_Rect *rect, Font *font, uint8_t align, std::string variable, ListBox *linked_list_box = NULL );
	void Changed( void );
};


class JoinMenuListBox : public ListBox
{
public:
	std::string Variable;
	TextBox *LinkedTextBox;
	
	JoinMenuListBox( Window *wind, SDL_Rect *rect, Font *font, int scroll_bar_size, std::string variable, TextBox *linked_text_box = NULL );
	void Changed( void );
};


class JoinMenuGoButton : public LabelledButton
{
public:
	JoinMenuGoButton( JoinMenu *menu, SDL_Rect *rect, const char *label );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class JoinMenuHostButton : public LabelledButton
{
public:
	JoinMenuHostButton( JoinMenu *menu, SDL_Rect *rect, const char *label );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class JoinMenuBackButton : public LabelledButton
{
public:
	JoinMenuBackButton( JoinMenu *menu, SDL_Rect *rect, const char *label );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
