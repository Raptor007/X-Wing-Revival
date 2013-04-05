/*
 *  WaitScreen.cpp
 */

#include "WaitScreen.h"

#include "RaptorGame.h"


WaitScreen::WaitScreen( std::string text, Font *font, SDL_Rect *rect )
{
	Text = text;
	
	if( rect )
	{
		Rect.w = rect->w;
		Rect.h = rect->h;
		Rect.x = rect->x;
		Rect.y = rect->y;
	}
	else
	{
		Rect.w = 300;
		Rect.h = 100;
		Rect.x = Raptor::Game->Gfx.W / 2 - Rect.w / 2;
		Rect.y = Raptor::Game->Gfx.H / 2 - Rect.h / 2;
	}
	
	if( font )
		TextFont = font;
	else
		TextFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 14 );
}


WaitScreen::~WaitScreen()
{
}


void WaitScreen::Draw( void )
{
	Window::Draw();
	
	TextFont->DrawText( Text, Rect.w / 2, Rect.h / 2, Font::ALIGN_MIDDLE_CENTER );
}
