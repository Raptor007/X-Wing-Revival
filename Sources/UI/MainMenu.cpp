/*
 *  MainMenu.cpp
 */

#include "MainMenu.h"

#include "XWingGame.h"
#include "JoinMenu.h"
#include "PrefsMenu.h"
#include "CampaignMenu.h"
#include "TextFileViewer.h"
#include <algorithm>


MainMenu::MainMenu( void )
{
	Name = "MainMenu";
	
	Background.BecomeInstance( Raptor::Game->Res.GetAnimation("bg_menu.ani") );
	Fog.BecomeInstance( Raptor::Game->Res.GetAnimation("fog.ani") );
	FogTime = 0.;
	IdleTime = 0.;
	
	NeedPrecache = false;
	Loading = false;
	LoadingFont = NULL;
	
	TitleFontBig   = Raptor::Game->Res.GetFont( "Candara.ttf", 128 );
	TitleFontSmall = Raptor::Game->Res.GetFont( "Candara.ttf",  64 );
	TitleFont      = TitleFontBig;
	ButtonFont     = Raptor::Game->Res.GetFont( "Verdana.ttf",  40 );
	VersionFont    = Raptor::Game->Res.GetFont( "Verdana.ttf",  16 );
	
	SDL_Rect button_rect;
	
	button_rect.w = 384;
	button_rect.h = ButtonFont->GetHeight() + 2;
	button_rect.x = 0;
	button_rect.y = 0;
	AddElement( StartButton = new MainMenuCampaignButton( &button_rect, ButtonFont ));
	StartButton->Enabled = Raptor::Game->Res.Find("Missions/rebel1.def").length() || Raptor::Game->Res.Find("Missions/empirel1.def").length();
	AddElement( XButton = new MainMenuOnlineButton( &button_rect, ButtonFont ));
	AddElement( AButton = new MainMenuCustomButton( &button_rect, ButtonFont ));
	AddElement( new MainMenuPrefsButton( &button_rect, ButtonFont ));
	AddElement( new MainMenuQuitButton( &button_rect, ButtonFont ));
	YButton = NULL;
	
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
	
	int height = 22 * (Elements.size() - 1);
	for( std::list<Layer*>::const_iterator element = Elements.begin(); element != Elements.end(); element ++ )
		height += (*element)->Rect.h;
	
	int y = mid - height / 2;
	for( std::list<Layer*>::iterator element = Elements.begin(); element != Elements.end(); element ++ )
	{
		(*element)->Rect.y = y;
		y += (*element)->Rect.h + 22;
	}
	
	UpdateCalcRects();
}


void MainMenu::Draw( void )
{
	Raptor::Game->Res.Lock.Lock();
	
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
		title_y = Elements.front()->Rect.y - VersionFont->GetHeight() - 10;
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
	
	bool is_top = IsTop();
	if( is_top )
		TitleFont->DrawText( Raptor::Game->Game, title_x, title_y, title_align );
	if( is_top || ! vr )
		VersionFont->DrawText( std::string("Version ") + Raptor::Game->Version, version_x, version_y, version_align );
	
	Raptor::Game->Res.Lock.Unlock();
	
	Raptor::Game->Mouse.ShowCursor = ! (NeedPrecache || Loading);
	
	if( NeedPrecache && FogTime )
	{
		NeedPrecache = false;
		
		if( Raptor::Game->Cfg.SettingAsInt( "precache", 2 ) == 3 ) // FIXME: This would be ideal, but textures fail to load from another thread!
		{
			#if SDL_VERSION_ATLEAST(2,0,0)
				SDL_CreateThread( MainMenuPrecacheThread, "Precache", this );
			#else
				SDL_CreateThread( MainMenuPrecacheThread, this );
			#endif
		}
		else
			MainMenuPrecacheThread( this );
	}
	else if( is_top && (IdleTime > 3.) && (Raptor::Game->State == Raptor::State::DISCONNECTED) && Raptor::Game->Cfg.SettingAsBool("screensaver") && ! Raptor::Server->IsRunning() )
	{
		// If the screensaver ended up at the main menu somehow, try restarting it.
		// NOTE: Could use similar logic to have a demo play after some amount of idle time.
		IdleTime = 0.;
		Raptor::Game->Host();
	}
	
	double frame_time = std::min<double>( 0.125, Raptor::Game->FrameTime );
	FogTime += frame_time;
	
	if( is_top )
		IdleTime += frame_time;
	else
		IdleTime = 0.;
	
	// Prevent absurd "showfps" output while precaching.
	if( NeedPrecache )
		Raptor::Game->FrameTime = 0.;
}


void MainMenu::DrawElements( void )
{
	if( Loading || NeedPrecache )
	{
		if( ! LoadingFont )
			LoadingFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 30 );
		
		Raptor::Game->Res.Lock.Lock();
		
		LoadingFont->DrawText( "Loading...", Rect.w / 2 + 2, Rect.h / 2 + 2, Font::ALIGN_MIDDLE_CENTER, 0.f,0.f,0.f,0.8f );
		LoadingFont->DrawText( "Loading...", Rect.w / 2,     Rect.h / 2,     Font::ALIGN_MIDDLE_CENTER );
		
		Raptor::Game->Res.Lock.Unlock();
	}
	else if( IsTop() )
		Layer::DrawElements();
}


bool MainMenu::HandleEvent( SDL_Event *event )
{
	IdleTime = 0.;
	
	if( Loading || NeedPrecache )
		return false;
	
	if( IsTop() )
	{
		if( (event->type == SDL_JOYBUTTONDOWN)
		&&  Raptor::Game->Cfg.SettingAsBool("joy_enable")
		&&  (Str::FindInsensitive( Raptor::Game->Joy.Joysticks[ event->jbutton.which ].Name, "Xbox" ) >= 0)
		&&  (event->jbutton.button == 7) // Start
		&&  StartButton && StartButton->Enabled && StartButton->Visible )
		{
			StartButton->Clicked();
			return true;
		}
		
		if( (event->type == SDL_JOYBUTTONDOWN)
		&&  Raptor::Game->Cfg.SettingAsBool("joy_enable")
		&&  (Str::FindInsensitive( Raptor::Game->Joy.Joysticks[ event->jbutton.which ].Name, "Xbox" ) >= 0)
		&&  (event->jbutton.button == 0) // A
		&&  AButton && AButton->Enabled && AButton->Visible )
		{
			AButton->Clicked();
			return true;
		}
		
		if( (event->type == SDL_JOYBUTTONDOWN)
		&&  Raptor::Game->Cfg.SettingAsBool("joy_enable")
		&&  (Str::FindInsensitive( Raptor::Game->Joy.Joysticks[ event->jbutton.which ].Name, "Xbox" ) >= 0)
		&&  (event->jbutton.button == 2) // X
		&&  XButton && XButton->Enabled && XButton->Visible )
		{
			XButton->Clicked();
			return true;
		}
		
		if( (event->type == SDL_JOYBUTTONDOWN)
		&&  Raptor::Game->Cfg.SettingAsBool("joy_enable")
		&&  (Str::FindInsensitive( Raptor::Game->Joy.Joysticks[ event->jbutton.which ].Name, "Xbox" ) >= 0)
		&&  (event->jbutton.button == 3) // Y
		&&  YButton && YButton->Enabled && YButton->Visible )
		{
			YButton->Clicked();
			return true;
		}
		
		return Layer::HandleEvent( event );
	}
	else
		return false;
}


// ---------------------------------------------------------------------------


MainMenuCampaignButton::MainMenuCampaignButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Campaign", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


MainMenuCampaignButton::~MainMenuCampaignButton()
{
}


void MainMenuCampaignButton::Clicked( Uint8 button )
{
	XWingGame *game = (XWingGame*) Raptor::Game;
	bool rebel_campaign_installed  = (game->Res.Find("Missions/rebel1.def")[0] == '.');
	bool empire_campaign_installed = (game->Res.Find("Missions/empire1.def")[0] == '.');
	if( (rebel_campaign_installed == empire_campaign_installed) || game->Cfg.SettingAsBool("campaign_menu",true) )
		game->Layers.Add( new CampaignMenu() );
	else
	{
		game->CampaignTeam = empire_campaign_installed ? XWing::Team::EMPIRE : XWing::Team::REBEL;
		game->Host();
	}
}


// ---------------------------------------------------------------------------


MainMenuOnlineButton::MainMenuOnlineButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Fly Online", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


MainMenuOnlineButton::~MainMenuOnlineButton()
{
}


void MainMenuOnlineButton::Clicked( Uint8 button )
{
	if( button == SDL_BUTTON_LEFT )
		Raptor::Game->Cfg.Command( std::string("connect ") + Raptor::Game->Cfg.SettingAsString( "online_server", "www.raptor007.com:7000", "www.raptor007.com:7000" ) );
}


// ---------------------------------------------------------------------------


MainMenuCustomButton::MainMenuCustomButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Custom / LAN", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


MainMenuCustomButton::~MainMenuCustomButton()
{
}


void MainMenuCustomButton::Clicked( Uint8 button )
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
	
	Raptor::Game->Layers.Add( new TextFileViewer( NULL, "README.txt", Raptor::Game->Res.GetFont( "ProFont.ttf", 12 ), "Help and Info" ) );
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


// -----------------------------------------------------------------------------


int MainMenuPrecacheThread( void *main_menu )
{
	((MainMenu*)( main_menu ))->Loading = true;
	SDL_Delay( 1 );
	((XWingGame*)( Raptor::Game ))->Precache();
	((MainMenu*)( main_menu ))->Loading = false;
	return 0;
}
