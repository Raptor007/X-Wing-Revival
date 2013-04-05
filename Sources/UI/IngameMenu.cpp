/*
 *  IngameMenu.cpp
 */

#include "IngameMenu.h"

#include "RaptorGame.h"
#include "PrefsMenu.h"


IngameMenu::IngameMenu( void )
{
	Background.BecomeInstance( Raptor::Game->Res.GetAnimation("bg_menu.ani") );
	
	TitleFont = Raptor::Game->Res.GetFont( "Candara.ttf", 128 );
	TitleFontSmall = Raptor::Game->Res.GetFont( "Candara.ttf", 64 );
	ButtonFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 48 );
	VersionFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 16 );
	
	SDL_Rect button_rect;
	
	button_rect.w = 384;
	button_rect.h = ButtonFont->GetHeight() + 2;
	button_rect.x = 0;
	button_rect.y = 0;
	AddElement( ResumeButton = new IngameMenuResumeButton( this, &button_rect, ButtonFont ));
	AddElement( PrefsButton = new IngameMenuPrefsButton( this, &button_rect, ButtonFont ));
	AddElement( LeaveButton = new IngameMenuLeaveButton( this, &button_rect, ButtonFont ));
	
	UpdateRects();
}


IngameMenu::~IngameMenu()
{
}


void IngameMenu::UpdateRects( void )
{
	Rect.x = 0;
	Rect.w = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	int top = TitleFont->GetHeight() * 3 / 2;
	int bottom = Rect.h - (VersionFont->GetHeight() + 10);
	int mid = ((bottom - top) / 2) + top;
	ResumeButton->Rect.y = mid - ResumeButton->Rect.h - PrefsButton->Rect.h;
	PrefsButton->Rect.y = mid - PrefsButton->Rect.h / 2;
	LeaveButton->Rect.y = mid + PrefsButton->Rect.h;
}


void IngameMenu::Draw( void )
{
	UpdateRects();
	
	if( IsTop() )
	{
		SDL_Rect title_size;
		TitleFont->TextSize( Raptor::Game->Game, &title_size );
		if( (title_size.w <= Raptor::Game->Gfx.W) && (title_size.h <= Raptor::Game->Gfx.H) )
			TitleFont->DrawText( Raptor::Game->Game, Rect.w / 2, TitleFont->GetHeight() / 2, Font::ALIGN_TOP_CENTER );
		else
			TitleFontSmall->DrawText( Raptor::Game->Game, Rect.w / 2, TitleFontSmall->GetHeight() / 2, Font::ALIGN_TOP_CENTER );
	}
	
	VersionFont->DrawText( "Version " + Raptor::Game->Version, Rect.w - 10, Rect.h - 10, Font::ALIGN_BOTTOM_RIGHT );
}


bool IngameMenu::KeyDown( SDLKey key )
{
	if( key == SDLK_ESCAPE )
	{
		Remove();
		Raptor::Game->Mouse.ShowCursor = false;
		return true;
	}
	
	return false;
}


// ---------------------------------------------------------------------------


IngameMenuResumeButton::IngameMenuResumeButton( IngameMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( menu, rect, button_font, "    Resume", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


void IngameMenuResumeButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Container->Remove();
	Raptor::Game->Mouse.ShowCursor = false;
}


// ---------------------------------------------------------------------------


IngameMenuPrefsButton::IngameMenuPrefsButton( IngameMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( menu, rect, button_font, "    Preferences", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


void IngameMenuPrefsButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Container->Remove();
	Raptor::Game->Layers.Add( new PrefsMenu() );
}


// ---------------------------------------------------------------------------


IngameMenuLeaveButton::IngameMenuLeaveButton( IngameMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( menu, rect, button_font, "    Leave", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


void IngameMenuLeaveButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Container->Remove();
	Raptor::Game->Net.DisconnectNice();
}

