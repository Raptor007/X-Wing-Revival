/*
 *  PrefsMenu.h
 */

#pragma once
class PrefsMenu;
class PrefsMenuCheckBox;
class PrefsMenuVRCheckBox;
class PrefsMenuSecretCheckBox;
class PrefsMenuRadioButton;
class PrefsMenuTextBox;
class PrefsMenuListBox;
class PrefsMenuDropDown;
class PrefsMenuFilterDropDown;
class PrefsMenuDoneButton;
class PrefsMenuDefaultsButton;
class PrefsMenuPageButton;
class PrefsMenuSillyButton;
class PrefsMenuRefreshButton;
class PrefsMenuBind;
class PrefsMenuCalibrator;
class PrefsMenuCalClearButton;
class PrefsMenuCalApplyButton;
class PrefsMenuCalDZCheckBox;

#include "PlatformSpecific.h"

#include "Window.h"
#include <list>
#include <map>
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
	Font *LabelFont, *TitleFont, *ItemFont, *ButtonFont, *ControlFont, *BindFont;
	std::map<std::string,std::string> Previous;
	int Page;
	
	PrefsMenu( void );
	virtual ~PrefsMenu();
	
	void WatchSetting( const std::string &name );
	bool WatchedSettingsChanged( void );
	
	void UpdateContents( void );
	bool ChangePage( int page );
	
	void Draw( void );
	bool KeyDown( SDLKey key );
	bool KeyUp( SDLKey key );
	bool MouseDown( Uint8 button = SDL_BUTTON_LEFT );
	bool MouseUp( Uint8 button = SDL_BUTTON_LEFT );
	
	enum
	{
		PAGE_PREFERENCES = 0,
		PAGE_CONTROLS,
		PAGE_CALIBRATION,
		NUM_PAGES
	};
};


class PrefsMenuCheckBox : public CheckBox
{
public:
	std::string Variable, TrueStr, FalseStr;
	
	PrefsMenuCheckBox( SDL_Rect *rect, Font *font, std::string label, std::string variable, std::string true_str = "true", std::string false_str = "false" );
	virtual ~PrefsMenuCheckBox();
	virtual void Changed( void );
};


class PrefsMenuVRCheckBox : public PrefsMenuCheckBox
{
public:
	PrefsMenuVRCheckBox( SDL_Rect *rect, Font *font, std::string label );
	virtual ~PrefsMenuVRCheckBox();
	void Changed( void );
};


class PrefsMenuSecretCheckBox : public PrefsMenuCheckBox
{
public:
	int TimesClicked;
	
	PrefsMenuSecretCheckBox( SDL_Rect *rect, Font *font, std::string label, std::string variable = "darkside", std::string true_str = "true", std::string false_str = "false" );
	virtual ~PrefsMenuSecretCheckBox();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuRadioButton : public CheckBox
{
public:
	std::string Variable, TrueStr;
	std::list<PrefsMenuRadioButton*> OtherButtons;
	
	PrefsMenuRadioButton( SDL_Rect *rect, Font *font, std::string label, std::string variable, std::string true_str );
	virtual ~PrefsMenuRadioButton();
	int GetWidth( void );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuTextBox : public TextBox
{
public:
	std::string Variable;
	ListBox *LinkedListBox;
	
	PrefsMenuTextBox( SDL_Rect *rect, Font *font, uint8_t align, std::string variable );
	virtual ~PrefsMenuTextBox();
	void Deselected( void );
};


class PrefsMenuListBox : public ListBox
{
public:
	std::string Variable;
	TextBox *LinkedTextBox;
	
	PrefsMenuListBox( SDL_Rect *rect, Font *font, int scroll_bar_size, std::string variable, TextBox *linked_text_box );
	virtual ~PrefsMenuListBox();
	void Changed( void );
};


class PrefsMenuDropDown : public DropDown
{
public:
	std::string Variable;
	
	PrefsMenuDropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, std::string variable );
	virtual ~PrefsMenuDropDown();
	void Update( void );
	void Changed( void );
};


class PrefsMenuFilterDropDown : public DropDown
{
public:
	PrefsMenuFilterDropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size );
	virtual ~PrefsMenuFilterDropDown();
	void Changed( void );
};


class PrefsMenuDoneButton : public LabelledButton
{
public:
	PrefsMenuDoneButton( SDL_Rect *rect, Font *button_font, const char *label );
	virtual ~PrefsMenuDoneButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuDefaultsButton : public LabelledButton
{
public:
	PrefsMenuDefaultsButton( SDL_Rect *rect, Font *button_font, const char *label );
	virtual ~PrefsMenuDefaultsButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuPageButton : public LabelledButton
{
public:
	int Page;
	PrefsMenuPageButton( SDL_Rect *rect, Font *button_font, const char *label, int page, bool selected );
	virtual ~PrefsMenuPageButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuSillyButton : public Button
{
public:
	int TimesClicked;
	Font *PewFont;
	
	PrefsMenuSillyButton( SDL_Rect *rect, Font *pew_font );
	virtual ~PrefsMenuSillyButton();
	void Draw( void );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuRefreshButton : public LabelledButton
{
public:
	PrefsMenuRefreshButton( SDL_Rect *rect, Font *button_font, const char *label );
	virtual ~PrefsMenuRefreshButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuBind : public Button
{
public:
	uint8_t Control, Inverse;
	bool Analog, Digital;
	Font *NameFont, *BindFont;
	
	PrefsMenuBind( SDL_Rect *rect, uint8_t analog_control, uint8_t inverse, Font *name_font, Font *bind_font );
	PrefsMenuBind( SDL_Rect *rect, uint8_t digital_control, Font *name_font, Font *bind_font );
	virtual ~PrefsMenuBind();
	void Draw( void );
	bool HandleEvent( SDL_Event *event );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuCalibrator : public Layer
{
public:
	std::string DeviceType;
	std::map<std::string,Sint32> RecentJoyID;
	std::map<Uint8,double> AxisMin, AxisMax, AxisDeadMin, AxisDeadMax;
	bool ReadingDeadzone;
	
	PrefsMenuCalibrator( SDL_Rect *rect );
	virtual ~PrefsMenuCalibrator();
	
	void Reset( void );
	void Clear( void );
	void Apply( void );
	void TrackEvent( SDL_Event *event );
	void Draw( void );
};


class PrefsMenuCalDevDropDown : public DropDown
{
public:
	PrefsMenuCalibrator *Calibrator;
	
	PrefsMenuCalDevDropDown( SDL_Rect *rect, Font *font, uint8_t align, PrefsMenuCalibrator *calibrator );
	virtual ~PrefsMenuCalDevDropDown();
	void Changed( void );
};


class PrefsMenuCalClearButton : public LabelledButton
{
public:
	PrefsMenuCalibrator *Calibrator;
	
	PrefsMenuCalClearButton( SDL_Rect *rect, Font *button_font, const char *label, PrefsMenuCalibrator *calibrator );
	virtual ~PrefsMenuCalClearButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuCalApplyButton : public LabelledButton
{
public:
	PrefsMenuCalibrator *Calibrator;
	
	PrefsMenuCalApplyButton( SDL_Rect *rect, Font *button_font, const char *label, PrefsMenuCalibrator *calibrator );
	virtual ~PrefsMenuCalApplyButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class PrefsMenuCalDZCheckBox : public CheckBox
{
public:
	PrefsMenuCalibrator *Calibrator;
	
	PrefsMenuCalDZCheckBox( SDL_Rect *rect, Font *font, std::string label, PrefsMenuCalibrator *calibrator );
	virtual ~PrefsMenuCalDZCheckBox();
	virtual void Changed( void );
};
