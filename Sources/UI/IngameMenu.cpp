/*
 *  IngameMenu.cpp
 */

#include "IngameMenu.h"

#include "RaptorGame.h"
#include "PrefsMenu.h"
#include "TextFileViewer.h"


IngameMenu::IngameMenu( void )
{
	Background.BecomeInstance( Raptor::Game->Res.GetAnimation("bg_menu.ani") );
	
	TitleFontBig = Raptor::Game->Res.GetFont( "Candara.ttf", 128 );
	TitleFontSmall = Raptor::Game->Res.GetFont( "Candara.ttf", 64 );
	TitleFont = TitleFontBig;
	ButtonFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 40 );
	VersionFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 15 );
	
	SDL_Rect button_rect;
	
	button_rect.w = 384;
	button_rect.h = ButtonFont->GetHeight() + 2;
	button_rect.x = 0;
	button_rect.y = 0;
	AddElement( ResumeButton = new IngameMenuResumeButton( &button_rect, ButtonFont ));
	AddElement( PrefsButton = new IngameMenuPrefsButton( &button_rect, ButtonFont ));
	AddElement( HelpButton = new IngameMenuHelpButton( &button_rect, ButtonFont ));
	AddElement( LeaveButton = new IngameMenuLeaveButton( &button_rect, ButtonFont ));
	
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
	
	SDL_Rect title_size;
	TitleFontBig->TextSize( Raptor::Game->Game, &title_size );
	if( (title_size.w <= Raptor::Game->Gfx.W) && (title_size.h <= (Raptor::Game->Gfx.H / 2)) )
		TitleFont = TitleFontBig;
	else
		TitleFont = TitleFontSmall;
	
	int top = TitleFont->GetHeight() * 3 / 2;
	int bottom = Rect.h - (VersionFont->GetHeight() + 10);
	int mid = ((bottom - top) / 2) + top;
	
	ResumeButton->Rect.y = mid - ResumeButton->Rect.h - PrefsButton->Rect.h - 33;
	PrefsButton->Rect.y = mid - PrefsButton->Rect.h - 11;
	HelpButton->Rect.y = mid + 11;
	LeaveButton->Rect.y = mid + HelpButton->Rect.h + 33;
}


void IngameMenu::Draw( void )
{
	UpdateRects();
	
	if( IsTop() )
		TitleFont->DrawText( Raptor::Game->Game, Rect.w / 2, TitleFont->GetHeight() / 2, Font::ALIGN_TOP_CENTER );
	
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


IngameMenuResumeButton::IngameMenuResumeButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Resume", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


IngameMenuResumeButton::~IngameMenuResumeButton()
{
}


void IngameMenuResumeButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Container->Remove();
	Raptor::Game->Mouse.ShowCursor = false;
}


// ---------------------------------------------------------------------------


IngameMenuPrefsButton::IngameMenuPrefsButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Preferences", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


IngameMenuPrefsButton::~IngameMenuPrefsButton()
{
}


void IngameMenuPrefsButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Container->Remove();
	Raptor::Game->Layers.Add( new PrefsMenu() );
}


// ---------------------------------------------------------------------------


IngameMenuHelpButton::IngameMenuHelpButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Help", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


IngameMenuHelpButton::~IngameMenuHelpButton()
{
}


void IngameMenuHelpButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Container->Remove();
	Raptor::Game->Layers.Add( new TextFileViewer( NULL, "README.txt", NULL, "Help and Info" ) );
}


// ---------------------------------------------------------------------------


IngameMenuLeaveButton::IngameMenuLeaveButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Leave", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


IngameMenuLeaveButton::~IngameMenuLeaveButton()
{
}


void IngameMenuLeaveButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Container->Remove();
	Raptor::Game->Net.DisconnectNice();
}

