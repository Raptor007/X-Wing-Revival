/*
 *  MissionUploader.h
 */

#pragma once
class MissionUploader;
class MissionUploaderDropDown;
class MissionUploaderUploadButton;
class MissionUploaderCancelButton;

#include "PlatformSpecific.h"

#include "Window.h"
#include "Font.h"
#include "LabelledButton.h"
#include "DropDown.h"
#include <set>


class MissionUploader : public Window
{
public:
	Font *TitleFont, *ItemFont, *ButtonFont;
	MissionUploaderUploadButton *UploadButton;
	std::string MissionID;
	
	MissionUploader( void );
	virtual ~MissionUploader();
	
	void UpdateContents( void );
	
	void Draw( void );
	bool KeyDown( SDLKey key );
	bool KeyUp( SDLKey key );
	bool MouseDown( Uint8 button = SDL_BUTTON_LEFT );
	bool MouseUp( Uint8 button = SDL_BUTTON_LEFT );
};


class MissionUploaderDropDown : public DropDown
{
public:
	MissionUploaderDropDown( SDL_Rect *rect, Font *font, uint8_t align = Font::ALIGN_MIDDLE_LEFT );
	virtual ~MissionUploaderDropDown();
	void Changed( void );
};


class MissionUploaderUploadButton : public LabelledButton
{
public:
	MissionUploaderUploadButton( SDL_Rect *rect, Font *button_font, const char *label );
	virtual ~MissionUploaderUploadButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};


class MissionUploaderCancelButton : public LabelledButton
{
public:
	MissionUploaderCancelButton( SDL_Rect *rect, Font *button_font, const char *label );
	virtual ~MissionUploaderCancelButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
