/*
 *  FirstLoadScreen.h
 */

#pragma once
class FirstLoadScreen;

#include "platforms.h"

#include "Window.h"
#include "Animation.h"
#include "Font.h"


class FirstLoadScreen : public Window
{
public:
	Animation Background;
	Font *TextFont;
	
	FirstLoadScreen( void );
	virtual ~FirstLoadScreen();
	
	void Draw( void );
};
