/*
 *  PrefsMenu.h
 */

#pragma once
class PrefsMenu;
class PrefsMenuCheckBox;
class PrefsMenuRadioButton;
class PrefsMenuTextBox;
class PrefsMenuListBox;
class PrefsMenuDropDown;
class PrefsMenuDoneButton;
class PrefsMenuDefaultsButton;

#include "PlatformSpecific.h"

#include "Window.h"
#include <list>
#include "Font.h"
#include "Layer.h"
#include "CheckBox.h"
#include "TextBox.h"
#include "ListBox.h"
#include "LabelledButton.h"
#include "DropDown.h"


class PrefsMenu : public Window
{
public:
	Font *TitleFont, *ItemFont, *ButtonFont;
	int PrevFullscreenX, PrevFullscreenY, PrevFSAA, PrevAF;
	bool PrevFullscreen;
	std::string PrevShaderFile, PrevSoundDir, PrevMusicDir;
	
	PrefsMenu( void );
	virtual ~PrefsMenu();
	
	void UpdateContents( void );
	void Draw( void );
	bool KeyUp( SDLKey key );
};


class PrefsMenuCheckBox : public CheckBox
{
public:
	std::string Variable, TrueStr, FalseStr;
	
	PrefsMenuCheckBox( Window *wind, SDL_Rect *rect, Font *font, std::string label, std::string variable, std::string true_str = "true", std::string false_str = "false" );
	void Changed( void );
};


class PrefsMenuRadioButton : public CheckBox
{
public:
	std::string Variable, TrueStr;
	std::list<PrefsMenuRadioButton*> OtherButtons;
	
	PrefsMenuRadioButton( Window *wind, SDL_Rect *rect, Font *font, std::string label, std::string variable, std::string true_str );
	int GetWidth( void );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuTextBox : public TextBox
{
public:
	std::string Variable;
	ListBox *LinkedListBox;
	
	PrefsMenuTextBox( Window *wind, SDL_Rect *rect, Font *font, uint8_t align, std::string variable );
	void Changed( void );
};


class PrefsMenuListBox : public ListBox
{
public:
	std::string Variable;
	TextBox *LinkedTextBox;
	
	PrefsMenuListBox( Window *wind, SDL_Rect *rect, Font *font, int scroll_bar_size, std::string variable, TextBox *linked_text_box );
	void Changed( void );
};


class PrefsMenuDropDown : public DropDown
{
public:
	std::string Variable;
	
	PrefsMenuDropDown( Layer *container, SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, std::string variable );
	void Changed( void );
};


class PrefsMenuDoneButton : public LabelledButton
{
public:
	PrefsMenuDoneButton( PrefsMenu *menu, SDL_Rect *rect, const char *label );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuDefaultsButton : public LabelledButton
{
public:
	PrefsMenuDefaultsButton( PrefsMenu *menu, SDL_Rect *rect, const char *label );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
