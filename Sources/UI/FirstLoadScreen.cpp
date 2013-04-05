/*
 *  FirstLoadScreen.cpp
 */

#include "FirstLoadScreen.h"

#include "RaptorGame.h"


FirstLoadScreen::FirstLoadScreen( void )
{
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	TextFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 32 );
	
	Background.BecomeInstance( Raptor::Game->Res.GetAnimation("bg_menu_small.ani") );
}


FirstLoadScreen::~FirstLoadScreen()
{
}


void FirstLoadScreen::Draw( void )
{
	Window::Draw();

	Raptor::Game->Gfx.DrawRect2D( Rect.w / 2 - Rect.h, 0, Rect.w / 2 + Rect.h, Rect.h, Background.CurrentFrame(), 1.f, 1.f, 1.f, 1.f );
	
	TextFont->DrawText( "Loading...", Rect.w / 2, Rect.h / 2, Font::ALIGN_MIDDLE_CENTER );
}
