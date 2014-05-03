/*
 *  JoinMenu.h
 */

#pragma once
class JoinMenu;
class JoinMenuTextBox;
class JoinMenuListBox;
class JoinMenuGoButton;
class JoinMenuHostButton;
class JoinMenuBackButton;

#include "PlatformSpecific.h"

#include <SDL/SDL.h>

#ifdef __APPLE__
	#include <SDL_ttf/SDL_ttf.h>
#else
	#include <SDL/SDL_ttf.h>
#endif

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
	Font *TitleFont, *LabelFont, *ItemFont, *ButtonFont;
	JoinMenuListBox *ServerList;
	NetUDP ServerFinder;
	
	JoinMenu( void );
	virtual ~JoinMenu();
	
	void Draw( void );
	bool KeyUp( SDLKey key );
};


class JoinMenuTextBox : public TextBox
{
public:
	std::string Variable;
	ListBox *LinkedListBox;
	
	JoinMenuTextBox( SDL_Rect *rect, Font *font, uint8_t align, std::string variable, ListBox *linked_list_box = NULL );
	virtual ~JoinMenuTextBox();
	void Changed( void );
};


class JoinMenuListBox : public ListBox
{
public:
	std::string Variable;
	TextBox *LinkedTextBox;
	
	JoinMenuListBox( SDL_Rect *rect, Font *font, int scroll_bar_size, std::string variable, TextBox *linked_text_box = NULL );
	virtual ~JoinMenuListBox();
	void Changed( void );
};


class JoinMenuGoButton : public LabelledButton
{
public:
	JoinMenuGoButton( SDL_Rect *rect, Font *button_font );
	virtual ~JoinMenuGoButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class JoinMenuHostButton : public LabelledButton
{
public:
	JoinMenuHostButton( SDL_Rect *rect, Font *button_font );
	virtual ~JoinMenuHostButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class JoinMenuBackButton : public LabelledButton
{
public:
	JoinMenuBackButton( SDL_Rect *rect, Font *button_font );
	virtual ~JoinMenuBackButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
