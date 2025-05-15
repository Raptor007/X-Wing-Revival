/*
 *  CampaignMenu.cpp
 */

#include "CampaignMenu.h"

#include "GroupBox.h"
#include "Label.h"
#include "Num.h"
#include "XWingDefs.h"
#include "XWingGame.h"
#include "ShipClass.h"


CampaignMenu::CampaignMenu( void )
{
	Name = "CampaignMenu";
	
	Rect.w = 640;
	Rect.h = 420;
	Rect.x = Raptor::Game->Gfx.W/2 - Rect.w/2;
	Rect.y = Raptor::Game->Gfx.H/2 - Rect.h/2;
	
	Red = 0.f;
	Green = 0.f;
	Blue = 1.f;
	Alpha = 0.5f;
	
	TitleFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 30 );
	
	SDL_Rect rect;
	
	rect.w = std::min<int>( (Rect.w - 30) / 2, Rect.h - 20 );
	rect.h = rect.w;
	rect.x = 10;
	rect.y = (Rect.h - rect.h) / 2;
	
	CampaignMenuTeamButton *rebel_button = new CampaignMenuTeamButton( &rect, XWing::Team::REBEL );
	rebel_button->Enabled = (Raptor::Game->Res.Find("Missions/rebel1.def")[0] == '.');
	AddElement( rebel_button );
	if( ! rebel_button->Enabled )
		AddElement( new Label( &rect, "MISSION\nDATA\nMISSING", TitleFont, Font::ALIGN_MIDDLE_CENTER ) );
	
	rect.x += rect.w + 10;
	
	CampaignMenuTeamButton *empire_button = new CampaignMenuTeamButton( &rect, XWing::Team::EMPIRE );
	empire_button->Enabled = (Raptor::Game->Res.Find("Missions/empire1.def")[0] == '.');
	AddElement( empire_button );
	if( ! empire_button->Enabled )
		AddElement( new Label( &rect, "MISSION\nDATA\nMISSING", TitleFont, Font::ALIGN_MIDDLE_CENTER ) );
	
	rect.w = 150;
	rect.h = 50;
	rect.y = Rect.h - rect.h - 10;
	rect.x = 10;
	AddElement( new CampaignMenuCloseButton( &rect, TitleFont, "Cancel" ) );
}


CampaignMenu::~CampaignMenu()
{
}


void CampaignMenu::Draw( void )
{
	// Keep the position up-to-date.
	Rect.x = Raptor::Game->Gfx.W/2 - Rect.w/2;
	Rect.y = Raptor::Game->Gfx.H/2 - Rect.h/2;
	
	Window::Draw();
	TitleFont->DrawText( "Select Faction", Rect.w/2 + 2, 12, Font::ALIGN_TOP_CENTER, 0,0,0,0.8f );
	TitleFont->DrawText( "Select Faction", Rect.w/2, 10, Font::ALIGN_TOP_CENTER );
}


bool CampaignMenu::KeyDown( SDLKey key )
{
	if( key == SDLK_ESCAPE )
	{
		Remove();
		return true;
	}
	
	return false;
}


// ---------------------------------------------------------------------------


CampaignMenuTeamButton::CampaignMenuTeamButton( SDL_Rect *rect, uint8_t team ) : Button( rect, NULL )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
	
	Team = team;
	Rotation = 0.;
	
	if( team == XWing::Team::REBEL )
		Shape.BecomeInstance( Raptor::Game->Res.GetModel("logo_rebel.obj") );
	else if( team == XWing::Team::EMPIRE )
		Shape.BecomeInstance( Raptor::Game->Res.GetModel("logo_empire.obj") );
}


CampaignMenuTeamButton::~CampaignMenuTeamButton()
{
}


void CampaignMenuTeamButton::Draw( void )
{
	glPushAttrib( GL_VIEWPORT_BIT );
	Raptor::Game->Gfx.SetViewport( CalcRect.x, CalcRect.y, CalcRect.w, CalcRect.h );
	
	bool hovering = MouseIsWithin && Enabled;
	Camera cam;
	cam.FOV = 30;
	Pos3D pos = cam + cam.Fwd * (hovering ? 2. : 2.25);
	if( Raptor::Game->Gfx.DrawTo )
	{
		if( Raptor::Game->Gfx.DrawTo == Raptor::Game->Head.EyeL )
			pos.MoveAlong( &(cam.Right), Raptor::Game->Cfg.SettingAsDouble("vr_logo_sep",0.01) );
		else if( Raptor::Game->Gfx.DrawTo == Raptor::Game->Head.EyeR )
			pos.MoveAlong( &(cam.Right), Raptor::Game->Cfg.SettingAsDouble("vr_logo_sep",0.01) * -1. );
	}
	pos.RotateAround( &(pos.Up), Rotation * -1. );
	Raptor::Game->Gfx.Setup3D( &cam, CalcRect.h ? (CalcRect.w / (double) CalcRect.h) : 1. );
	
	bool use_shaders = Raptor::Game->Cfg.SettingAsBool("g_shader_enable");
	if( use_shaders )
	{
		Raptor::Game->ShaderMgr.ResumeShaders();
		Raptor::Game->ShaderMgr.Set3f( "CamPos", cam.X, cam.Y, cam.Z );
		Vec3D dir;
		dir.Set( -0.1, 0.8, 0.3 ); // Right Fwd Up
		dir.ScaleTo( 1. );
		Raptor::Game->ShaderMgr.Set3f( "AmbientLight", 0.125, 0.125, 0.125 );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight0Dir", dir.X, dir.Y, dir.Z );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight0Color", 0.75, 0.5, 1. );
		Raptor::Game->ShaderMgr.Set1f( "DirectionalLight0WrapAround", 0.5 );
		dir.Set( 0., 0.8, -0.3 ); // Right Fwd Up
		dir.ScaleTo( 1. );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight1Dir", dir.X, dir.Y, dir.Z );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight1Color", 0.5, 0.375, 0.75 );
		Raptor::Game->ShaderMgr.Set1f( "DirectionalLight1WrapAround", 0.5 );
		bool bright = Enabled;
		if( ! MouseIsWithin )
		{
			if( (Team == XWing::Team::EMPIRE) && (Raptor::Game->Cfg.SettingAsString("empire_mission") == "empire0") )
				bright = false;
			else if( (Team == XWing::Team::REBEL) && (Raptor::Game->Cfg.SettingAsString("rebel_mission") == "rebel0") )
				bright = false;
		}
		if( bright )
		{
			dir.Set( -0.2, -0.8, 0.2 ); // Right Fwd Up
			dir.ScaleTo( 1. );
			Raptor::Game->ShaderMgr.Set3f( "DirectionalLight2Dir", dir.X, dir.Y, dir.Z );
			Raptor::Game->ShaderMgr.Set3f( "DirectionalLight2Color", 1., 1., 1. );
			Raptor::Game->ShaderMgr.Set1f( "DirectionalLight2WrapAround", 0.5 );
			dir.Set( -0.2, -0.8, 0.3 ); // Right Fwd Up
			dir.ScaleTo( 1. );
			Raptor::Game->ShaderMgr.Set3f( "DirectionalLight3Dir", dir.X, dir.Y, dir.Z );
			Raptor::Game->ShaderMgr.Set3f( "DirectionalLight3Color", 0.5, 0.75, 1. );
			Raptor::Game->ShaderMgr.Set1f( "DirectionalLight3WrapAround", 0.25 );
		}
		else
		{
			Raptor::Game->ShaderMgr.Set3f( "DirectionalLight2Color", 0., 0., 0. );
			Raptor::Game->ShaderMgr.Set3f( "DirectionalLight3Color", 0., 0., 0. );
		}
	}
	
	Shape.DrawAt( &pos );
	
	if( use_shaders )
		Raptor::Game->ShaderMgr.StopShaders();
	
	double frame_time = std::min<double>( 0.125, Raptor::Game->FrameTime );
	if( hovering )
	{
		Rotation += frame_time * 90.;
		if( Rotation > 360. )
			Rotation -= 360.;
	}
	else if( Rotation && (Rotation != 180.) )
	{
		double old_rotation = Rotation;
		
		if( ((Rotation > 90.) && (Rotation < 180.)) || (Rotation > 270.) )
			Rotation += frame_time * 120.;
		else
			Rotation -= frame_time * 120.;
		
		if( ((old_rotation > 0.) != (Rotation > 0.)) || (Rotation > 360.) )
			Rotation = 0.;
		else if( (old_rotation > 180.) != (Rotation > 180.) )
			Rotation = 180.;
	}
	
	glPopAttrib();
}


void CampaignMenuTeamButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	XWingGame *game = (XWingGame*) Raptor::Game;
	game->CampaignTeam = Team;
	game->Host();
	
	if( game->Cfg.SettingAsBool("darkside") && Raptor::Server->IsRunning() )
	{
		Packet info = Packet( Raptor::Packet::INFO );
		info.AddUShort( 1 );
		info.AddString( "darkside" );
		info.AddString( "true" );
		game->Net.Send( &info );
	}
}


// ---------------------------------------------------------------------------


CampaignMenuCloseButton::CampaignMenuCloseButton( SDL_Rect *rect, Font *button_font, const char *label ) : LabelledButton( rect, button_font, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


CampaignMenuCloseButton::~CampaignMenuCloseButton()
{
}


void CampaignMenuCloseButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	Container->Remove();
}
