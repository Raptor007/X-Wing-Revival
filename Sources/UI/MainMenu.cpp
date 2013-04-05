/*
 *  MainMenu.cpp
 */

#include "MainMenu.h"

#include "RaptorGame.h"
#include "JoinMenu.h"
#include "PrefsMenu.h"


MainMenu::MainMenu( void )
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
	AddElement( PlayButton = new MainMenuPlayButton( this, &button_rect, ButtonFont ));
	AddElement( PrefsButton = new MainMenuPrefsButton( this, &button_rect, ButtonFont ));
	AddElement( QuitButton = new MainMenuQuitButton( this, &button_rect, ButtonFont ));
	
	UpdateRects();
}


MainMenu::~MainMenu()
{
}


void MainMenu::UpdateRects( void )
{
	Rect.x = 0;
	Rect.w = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	int top = TitleFont->GetHeight() * 3 / 2;
	int bottom = Rect.h - (VersionFont->GetHeight() + 10);
	int mid = ((bottom - top) / 2) + top;
	PlayButton->Rect.y = mid - PlayButton->Rect.h - PrefsButton->Rect.h;
	PrefsButton->Rect.y = mid - PrefsButton->Rect.h / 2;
	QuitButton->Rect.y = mid + PrefsButton->Rect.h;
}


void MainMenu::Draw( void )
{
	UpdateRects();
	
	Raptor::Game->Gfx.DrawRect2D( Rect.w / 2 - Rect.h, 0, Rect.w / 2 + Rect.h, Rect.h, Background.CurrentFrame(), 1.f, 1.f, 1.f, 1.f );
	
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


void MainMenu::DrawElements( void )
{
	if( IsTop() )
		Layer::DrawElements();
}


bool MainMenu::HandleEvent( SDL_Event *event, bool already_handled )
{
	if( IsTop() )
		return Layer::HandleEvent( event );
	else
		return false;
}


// ---------------------------------------------------------------------------


MainMenuPlayButton::MainMenuPlayButton( MainMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( menu, rect, button_font, "    Play", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


void MainMenuPlayButton::Clicked( Uint8 button )
{
	if( button == SDL_BUTTON_LEFT )
		Raptor::Game->Layers.Add( new JoinMenu() );
}


// ---------------------------------------------------------------------------


MainMenuPrefsButton::MainMenuPrefsButton( MainMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( menu, rect, button_font, "    Preferences", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


void MainMenuPrefsButton::Clicked( Uint8 button )
{
	if( button == SDL_BUTTON_LEFT )
		Raptor::Game->Layers.Add( new PrefsMenu() );
}


// ---------------------------------------------------------------------------


MainMenuQuitButton::MainMenuQuitButton( MainMenu *menu, SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( menu, rect, button_font, "    Quit", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


void MainMenuQuitButton::Clicked( Uint8 button )
{
	if( button == SDL_BUTTON_LEFT )
		Raptor::Game->Quit();
}

