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
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	bool vr = Raptor::Game->Head.VR && Raptor::Game->Gfx.DrawTo;
	if( vr )
	{
		Rect.x = Raptor::Game->Gfx.W/2 - 640/2;
		Rect.y = Raptor::Game->Gfx.H/2 - 480/2;
		Rect.w = 640;
		Rect.h = 480;
	}
	
	SDL_Rect title_size = {0,0,0,0};
	TitleFontBig->TextSize( Raptor::Game->Game, &title_size );
	if( (title_size.w <= Rect.w) && (title_size.h <= (Rect.h / 2)) )
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
	
	UpdateCalcRects();
}


void MainMenu::Draw( void )
{
	UpdateRects();
	
	int title_x   = Rect.w / 2;
	int title_y   = TitleFont->GetHeight() / 2;
	int version_x = Rect.w - 10;
	int version_y = Rect.h - 10;
	uint8_t title_align   = Font::ALIGN_TOP_CENTER;
	uint8_t version_align = Font::ALIGN_BOTTOM_RIGHT;
	bool vr = Raptor::Game->Head.VR && Raptor::Game->Gfx.DrawTo;
	
	if( vr )
	{
		// In VR, show 3D background behind the menu.
		glPushMatrix();
		Pos3D origin;
		Raptor::Game->Cam.Copy( &origin );
		Raptor::Game->Cam.FOV = Raptor::Game->Cfg.SettingAsDouble("vr_fov");
		Raptor::Game->Gfx.Setup3D( &(Raptor::Game->Cam) );
		Animation bg;
		bg.BecomeInstance( Raptor::Game->Res.GetAnimation("nebula.ani") );
		double bg_dist = Raptor::Game->Cfg.SettingAsDouble( "g_bg_dist", std::min<double>( 50000., Raptor::Game->Gfx.ZFar * 0.875 ) );
		Raptor::Game->Gfx.DrawSphere3D( 0,0,0, bg_dist, 32, bg.CurrentFrame(), Graphics::TEXTURE_MODE_Y_ASIN );
		glPopMatrix();
		DrawSetup();
		
		// Move title and version positions for VR.
		title_y = PlayButton->Rect.y - VersionFont->GetHeight() - 10;
		version_x = title_x;
		version_y = title_y;
		title_align   = Font::ALIGN_BOTTOM_CENTER;
		version_align = Font::ALIGN_TOP_CENTER;
	}
	else
	{
		Raptor::Game->Gfx.DrawRect2D( Rect.w / 2 - Rect.h, 0, Rect.w / 2 + Rect.h, Rect.h, Background.CurrentFrame(), 1.f, 1.f, 1.f, 1.f );
		
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, Fog.CurrentFrame() );
		
		// Make sure the fog texture is allowed to repeat.
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		
		int fog_h1 = Rect.h / 5;
		int fog_h2 = Rect.h / 6;
		int fog_x1 = Rect.w / 2 - Rect.h;
		int fog_x2 = fog_x1 + Rect.h * 2;
		float fog_alpha = std::min<float>( FogTime / 12.f, 0.25f );
		
		glBegin( GL_QUADS );
			
			glColor4f( 0.41f, 0.56f, 0.58f, fog_alpha );
			
			// Top-left
			glTexCoord2d( FogTime * -0.03, 0 );
			glVertex2i( fog_x1, Rect.h - fog_h1 );
			
			// Bottom-left
			glTexCoord2d( FogTime * -0.03, 1 );
			glVertex2i( fog_x1, Rect.h );
			
			// Bottom-right
			glTexCoord2d( (FogTime * -0.03) + (0.5 * Rect.h / fog_h1), 1 );
			glVertex2i( fog_x2, Rect.h );
			
			// Top-right
			glTexCoord2d( (FogTime * -0.03) + (0.5 * Rect.h / fog_h1), 0 );
			glVertex2i( fog_x2, Rect.h - fog_h1 );
			
			glColor4f( 0.41f, 0.54f, 0.55f, fog_alpha );
			
			// Top-left
			glTexCoord2d( FogTime * -0.07, 0 );
			glVertex2i( fog_x1, Rect.h - fog_h2 );
			
			// Bottom-left
			glTexCoord2d( FogTime * -0.07, 1 );
			glVertex2i( fog_x1, Rect.h );
			
			// Bottom-right
			glTexCoord2d( (FogTime * -0.07) + (0.5 * Rect.h / fog_h2), 1 );
			glVertex2i( fog_x2, Rect.h );
			
			// Top-right
			glTexCoord2d( (FogTime * -0.07) + (0.5 * Rect.h / fog_h2), 0 );
			glVertex2i( fog_x2, Rect.h - fog_h2 );
			
		glEnd();
		glDisable( GL_TEXTURE_2D );
	}
	
	FogTime += std::min<double>( Raptor::Game->FrameTime, 0.25 );
	
	bool is_top = IsTop();
	if( is_top )
		TitleFont->DrawText( Raptor::Game->Game, title_x, title_y, title_align );
	if( is_top || ! vr )
		VersionFont->DrawText( std::string("Version ") + Raptor::Game->Version, version_x, version_y, version_align );
	
	// If the screensaver ended up at the main menu somehow, try restarting it.
	// NOTE: Could use similar logic to have a demo play after some amount of idle time.
	if( (FogTime > 5.) && (Raptor::Game->State == Raptor::State::DISCONNECTED) && Raptor::Game->Cfg.SettingAsBool("screensaver") && IsTop() && ! Raptor::Server->IsRunning() )
	{
		Raptor::Game->Host();
		FogTime = 0.;
	}
}


void MainMenu::DrawElements( void )
{
	if( IsTop() )
		Layer::DrawElements();
}


bool MainMenu::HandleEvent( SDL_Event *event )
{
	if( IsTop() )
	{
		if( (event->type == SDL_JOYBUTTONDOWN)
		&&  Raptor::Game->Cfg.SettingAsBool("joy_enable")
		&&  (Str::FindInsensitive( Raptor::Game->Joy.Joysticks[ event->jbutton.which ].Name, "Xbox" ) >= 0)
		&&  (event->jbutton.button == 7) // Start
		&&  PlayButton && PlayButton->Enabled && PlayButton->Visible )
		{
			PlayButton->Clicked();
			return true;
		}
		
		return Layer::HandleEvent( event );
	}
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

