/*
 *  WaitScreen.h
 */

#pragma once
class WaitScreen;

#include "platforms.h"

#include "Window.h"
#include "Font.h"


class WaitScreen : public Window
{
public:
	Font *TextFont;
	std::string Text;
	
	WaitScreen( std::string text, Font *font = NULL, SDL_Rect *rect = NULL );
	virtual ~WaitScreen();
	
	void Draw( void );
};
