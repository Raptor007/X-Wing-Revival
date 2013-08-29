/*
 *  MainMenu.cpp
 */

#include "MainMenu.h"

#include "RaptorGame.h"
#include "JoinMenu.h"
#include "PrefsMenu.h"
#include "TextFileViewer.h"
#include <algorithm>


MainMenu::MainMenu( void )
{
	Background.BecomeInstance( Raptor::Game->Res.GetAnimation("bg_menu.ani") );
	Fog.BecomeInstance( Raptor::Game->Res.GetAnimation("fog.ani") );
	FogTime = 0.;
	
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
	AddElement( PlayButton = new MainMenuPlayButton( &button_rect, ButtonFont ));
	AddElement( PrefsButton = new MainMenuPrefsButton( &button_rect, ButtonFont ));
	AddElement( HelpButton = new MainMenuHelpButton( &button_rect, ButtonFont ));
	AddElement( QuitButton = new MainMenuQuitButton( &button_rect, ButtonFont ));
	
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
	
	SDL_Rect title_size;
	TitleFontBig->TextSize( Raptor::Game->Game, &title_size );
	if( (title_size.w <= Raptor::Game->Gfx.W) && (title_size.h <= (Raptor::Game->Gfx.H / 2)) )
		TitleFont = TitleFontBig;
	else
		TitleFont = TitleFontSmall;
	
	int top = TitleFont->GetHeight() * 3 / 2;
	int bottom = Rect.h - (VersionFont->GetHeight() + 10);
	int mid = ((bottom - top) / 2) + top;
	
	PlayButton->Rect.y = mid - PlayButton->Rect.h - PrefsButton->Rect.h - 33;
	PrefsButton->Rect.y = mid - PrefsButton->Rect.h - 11;
	HelpButton->Rect.y = mid + 11;
	QuitButton->Rect.y = mid + HelpButton->Rect.h + 33;
}


void MainMenu::Draw( void )
{
	UpdateRects();
	
	Raptor::Game->Gfx.DrawRect2D( Rect.w / 2 - Rect.h, 0, Rect.w / 2 + Rect.h, Rect.h, Background.CurrentFrame(), 1.f, 1.f, 1.f, 1.f );
	
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, Fog.CurrentFrame() );
	
	double fog_h1 = Rect.h / 5.;
	double fog_h2 = Rect.h / 6.;
	float fog_alpha = std::min<float>( FogTime / 12.f, 0.25f );
	
	glBegin( GL_QUADS );
		
		glColor4f( 0.41f, 0.56f, 0.58f, fog_alpha );
		
		// Top-left
		glTexCoord2d( FogTime * -0.03, 0 );
		glVertex2i( 0., Rect.h - fog_h1 );
		
		// Bottom-left
		glTexCoord2d( FogTime * -0.03, 1 );
		glVertex2i( 0., Rect.h );
		
		// Bottom-right
		glTexCoord2d( (FogTime * -0.03) + (0.25 * Rect.w / fog_h1), 1 );
		glVertex2i( Rect.w, Rect.h );
		
		// Top-right
		glTexCoord2d( (FogTime * -0.03) + (0.25 * Rect.w / fog_h1), 0 );
		glVertex2i( Rect.w, Rect.h - fog_h1 );
		
		glColor4f( 0.41f, 0.54f, 0.55f, fog_alpha );
		
		// Top-left
		glTexCoord2d( FogTime * -0.07, 0 );
		glVertex2i( 0., Rect.h - fog_h2 );
		
		// Bottom-left
		glTexCoord2d( FogTime * -0.07, 1 );
		glVertex2i( 0., Rect.h );
		
		// Bottom-right
		glTexCoord2d( (FogTime * -0.07) + (0.25 * Rect.w / fog_h2), 1 );
		glVertex2i( Rect.w, Rect.h );
		
		// Top-right
		glTexCoord2d( (FogTime * -0.07) + (0.25 * Rect.w / fog_h2), 0 );
		glVertex2i( Rect.w, Rect.h - fog_h2 );
		
	glEnd();
	glDisable( GL_TEXTURE_2D );
	
	FogTime += std::min<double>( Raptor::Game->FrameTime, 0.25 );
	
	if( IsTop() )
		TitleFont->DrawText( Raptor::Game->Game, Rect.w / 2, TitleFont->GetHeight() / 2, Font::ALIGN_TOP_CENTER );
	
	VersionFont->DrawText( "Version " + Raptor::Game->Version, Rect.w - 10, Rect.h - 10, Font::ALIGN_BOTTOM_RIGHT );
}


void MainMenu::DrawElements( void )
{
	if( IsTop() )
		Layer::DrawElements();
}


bool MainMenu::HandleEvent( SDL_Event *event )
{
	if( IsTop() )
		return Layer::HandleEvent( event );
	else
		return false;
}


// ---------------------------------------------------------------------------


MainMenuPlayButton::MainMenuPlayButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Play", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


MainMenuPlayButton::~MainMenuPlayButton()
{
}


void MainMenuPlayButton::Clicked( Uint8 button )
{
	if( button == SDL_BUTTON_LEFT )
		Raptor::Game->Layers.Add( new JoinMenu() );
}


// ---------------------------------------------------------------------------


MainMenuPrefsButton::MainMenuPrefsButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Preferences", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


MainMenuPrefsButton::~MainMenuPrefsButton()
{
}


void MainMenuPrefsButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Raptor::Game->Layers.Add( new PrefsMenu() );
}


// ---------------------------------------------------------------------------


MainMenuHelpButton::MainMenuHelpButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Help", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


MainMenuHelpButton::~MainMenuHelpButton()
{
}


void MainMenuHelpButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Raptor::Game->Layers.Add( new TextFileViewer( NULL, "README.txt", NULL, "Help and Info" ) );
}


// ---------------------------------------------------------------------------


MainMenuQuitButton::MainMenuQuitButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Quit", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


MainMenuQuitButton::~MainMenuQuitButton()
{
}


void MainMenuQuitButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Raptor::Game->Quit();
}

