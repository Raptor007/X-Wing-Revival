/*
 *  PrefsMenu.cpp
 */

#include "PrefsMenu.h"

#include "Label.h"
#include "GroupBox.h"
#include "ScrollArea.h"
#include "RaptorGame.h"
#include "XWingDefs.h"
#include "XWingGame.h"
#include "Num.h"
#include <algorithm>


PrefsMenu::PrefsMenu( void )
{
	Name = "PrefsMenu";
	
	Rect.x = Raptor::Game->Gfx.W/2 - 640/2;
	Rect.y = Raptor::Game->Gfx.H/2 - 480/2;
	Rect.w = 640;
	Rect.h = 480;
	
	Red = 0.f;
	Green = 0.f;
	Blue = 1.f;
	Alpha = 0.5f;
	
	TitleFont   = Raptor::Game->Res.GetFont( "Verdana.ttf", 30 );
	LabelFont   = Raptor::Game->Res.GetFont( "Verdana.ttf", 16 );
	ItemFont    = LabelFont;
	ButtonFont  = TitleFont;
	ControlFont = Raptor::Game->Res.GetFont( "Geneva.ttf",  12 );
	BindFont    = Raptor::Game->Res.GetFont( "Verdana.ttf", 11 );
	
	Page = PAGE_VIDEO;
	UpdateContents();
	
	WatchSetting( "g_fullscreen" );
	WatchSetting( "g_res_fullscreen_x" );
	WatchSetting( "g_res_fullscreen_y" );
	WatchSetting( "g_vsync" );
	WatchSetting( "g_bpp" );
	WatchSetting( "g_zbits" );
	WatchSetting( "g_fsaa" );
	WatchSetting( "g_mipmap" );
	WatchSetting( "g_af" );
	WatchSetting( "g_texture_maxres" );
	WatchSetting( "g_framebuffers" );
	WatchSetting( "g_shader_enable" );
	WatchSetting( "g_shader_version" );
	WatchSetting( "g_shader_light_quality" );
	WatchSetting( "g_shader_point_lights" );
	WatchSetting( "g_shader_blastpoints" );
	WatchSetting( "g_shader_blastpoint_quality" );
#if SDL_VERSION_ATLEAST(2,0,0)
	WatchSetting( "s_channels" );
	WatchSetting( "s_depth" );
	WatchSetting( "s_rate" );
#endif
	WatchSetting( "s_buffer" );
	WatchSetting( "s_mix_channels" );
	WatchSetting( "s_mic_buffer" );
	WatchSetting( "s_mic_device" );
	if( ! Raptor::Game->Mic.Device )
		WatchSetting( "s_mic_init" );
	
	XWingGame *game = (XWingGame*) Raptor::Game;
	Paused = (game->State >= XWing::State::FLYING)
		&& (game->Data.TimeScale == 1.)
		&& (game->Data.Players.size() == 1)
		&& game->Cfg.SettingAsBool("ui_pause",true)
		&& Raptor::Server->IsRunning()
		&& game->ControlPressed( game->Controls[ XWing::Control::PAUSE ] );
}


PrefsMenu::~PrefsMenu()
{
	XWingGame *game = (XWingGame*) Raptor::Game;
	if( Paused && (game->State >= XWing::State::FLYING) && (game->Data.TimeScale < 0.0000011) )
		game->ControlPressed( game->Controls[ XWing::Control::PAUSE ] );  // Unpause
}


void PrefsMenu::WatchSetting( const std::string &name )
{
	if( Str::BeginsWith( name, "s_" ) )
		PrevSnd[ name ] = Raptor::Game->Cfg.SettingAsString( name );
	else
		PrevGfx[ name ] = Raptor::Game->Cfg.SettingAsString( name );
}


bool PrefsMenu::GfxSettingsChanged( void )
{
	// Toggling shaders only requires restart when enabling if any Shader::Load occurred while they were disabled.
	bool shader_enable = Raptor::Game->Cfg.SettingAsBool( "g_shader_enable" );
	if( shader_enable && (PrevGfx[ "g_shader_enable" ] != Raptor::Game->Cfg.SettingAsString( "g_shader_enable" )) && Raptor::Game->Res.ShadersNeedReload() )
	{
		//Raptor::Game->Console.Print( "PrefsMenu::WatchedSettingsChanged: g_shader_enable" );
		return true;
	}
	
	for( std::map<std::string,std::string>::const_iterator prev_iter = PrevGfx.begin(); prev_iter != PrevGfx.end(); prev_iter ++ )
	{
		// We already decided above if g_shader_enable requires a restart.
		if( prev_iter->first == "g_shader_enable" )
			continue;
		
		// Don't restart graphics for changes to shader variables if shaders are disabled.
		if( Str::BeginsWith( prev_iter->first, "g_shader_" ) && ! shader_enable )
			continue;
		
		// Don't restart graphics for changing fullscreen resolution if we're in windowed mode.
		if( Str::BeginsWith( prev_iter->first, "g_res_fullscreen_" ) && ! Raptor::Game->Cfg.SettingAsBool("g_fullscreen") )
			continue;
		
		// Restart if any other watched variable was changed.
		if( prev_iter->second != Raptor::Game->Cfg.SettingAsString( prev_iter->first ) )
		{
			if( Raptor::Game->Cfg.SettingAsBool("debug") )
				Raptor::Game->Console.Print( std::string("PrefsMenu::GfxSettingsChanged: ") + prev_iter->first );
			return true;
		}
	}
	
	return false;
}


bool PrefsMenu::SndSettingsChanged( void )
{
	for( std::map<std::string,std::string>::const_iterator prev_iter = PrevSnd.begin(); prev_iter != PrevSnd.end(); prev_iter ++ )
	{
		if( prev_iter->second != Raptor::Game->Cfg.SettingAsString( prev_iter->first ) )
		{
			if( Raptor::Game->Cfg.SettingAsBool("debug") )
				Raptor::Game->Console.Print( std::string("PrefsMenu::SndSettingsChanged: ") + prev_iter->first );
			return true;
		}
	}
	
	return false;
}


void PrefsMenu::UpdateContents( void )
{
	// Remove existing elements.
	
	Selected = NULL;
	RemoveAllElements();
	
	
	// Add new elements.
	
	SDL_Rect group_rect, rect;
	GroupBox *group = NULL;
	
	rect.w = 150;
	rect.h = 50;
	rect.y = Rect.h - rect.h - 10;
	rect.x = Rect.w - rect.w - 10;
	AddElement( new PrefsMenuDoneButton( &rect, ButtonFont, "Done" ) );
	
	rect.x -= (rect.w + 10);
	AddElement( new PrefsMenuDefaultsButton( &rect, ButtonFont, "Defaults" ) );
	
	rect.x = Rect.w/2 - 310;
	rect.y = 10;
	rect.w = 125;
	rect.h = 40;
	AddElement( new PrefsMenuPageButton( &rect, TitleFont, "Video", PAGE_VIDEO, (Page == PAGE_VIDEO) ) );
	
	rect.x += rect.w + 5;
	AddElement( new PrefsMenuPageButton( &rect, TitleFont, "Audio", PAGE_AUDIO, (Page == PAGE_AUDIO) ) );
	
	rect.x += rect.w + 5;
	rect.w = 155;
	AddElement( new PrefsMenuPageButton( &rect, TitleFont, "Controls", PAGE_CONTROLS, (Page == PAGE_CONTROLS) ) );
	
	rect.x += rect.w + 5;
	rect.w = 200;
	AddElement( new PrefsMenuPageButton( &rect, TitleFont, "Calibration", PAGE_CALIBRATION, (Page == PAGE_CALIBRATION) ) );
	
	if( Page == PrefsMenu::PAGE_VIDEO )
	{
		// --------------------------------------------------------------------------------------------------------------------
		// Graphics
		
		group_rect.x = 10;
		group_rect.y = 50;
		group_rect.w = 420;
		group_rect.h = 299;
		group = new GroupBox( &group_rect, "Graphics", ItemFont );
		AddElement( group );
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		rect.h = ItemFont ? ItemFont->GetHeight() : 18;
		
		rect.w = 115;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Fullscreen:", "g_fullscreen" ) );
		rect.x += rect.w + 5;
		rect.w = 60;
		group->AddElement( new PrefsMenuTextBox( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, "g_res_fullscreen_x" ) );
		rect.x += rect.w + 5;
		rect.w = 10;
		group->AddElement( new Label( &rect, "x", LabelFont, Font::ALIGN_MIDDLE_CENTER ) );
		rect.x += rect.w + 5;
		rect.w = 60;
		group->AddElement( new PrefsMenuTextBox( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, "g_res_fullscreen_y" ) );
		rect.w = 90;
		rect.x = group_rect.w - rect.w - 10;
		PrefsMenuDropDown *fov_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_fov" );
		fov_dropdown->AddItem( "auto", "FOV Auto" );
		std::set<int> fov_list;
		int current_fov = Raptor::Game->Cfg.SettingAsInt("g_fov");
		if( current_fov )
			fov_list.insert( current_fov );
		for( int i = 80; i <= 140; i += 10 )
			fov_list.insert( i );
		for( std::set<int>::const_iterator fov_iter = fov_list.begin(); fov_iter != fov_list.end(); fov_iter ++ )
		{
			int fov = *fov_iter;
			fov_dropdown->AddItem( Num::ToString(fov), ((fov >= 0) ? std::string("FOV ") : std::string("vFOV ")) + Num::ToString(abs(fov)) );
		}
		fov_dropdown->Update();
		group->AddElement( fov_dropdown );
		
		rect.y += rect.h + 8;
		rect.x = 10;
		rect.w = 80;
		PrefsMenuCheckBox *vsync_checkbox = new PrefsMenuCheckBox( &rect, LabelFont, "VSync", "g_vsync" );
		group->AddElement( vsync_checkbox );
		rect.x += rect.w + 5;
		rect.w = 210;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Framebuffer Textures", "g_framebuffers" ) );
		rect.x += rect.w + 20;
		rect.w = 85;
		PrefsMenuDropDown *fsaa_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_fsaa" );
		fsaa_dropdown->AddItem(  "0", "No AA" );
		fsaa_dropdown->AddItem(  "2", "2xMSAA" );
		fsaa_dropdown->AddItem(  "3", "3xMSAA" );
		fsaa_dropdown->AddItem(  "4", "4xMSAA" );
		fsaa_dropdown->AddItem(  "6", "6xMSAA" );
		fsaa_dropdown->AddItem(  "8", "8xMSAA" );
		fsaa_dropdown->AddItem( "16", "16xMSAA" );
		fsaa_dropdown->Update();
		group->AddElement( fsaa_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 140;
		group->AddElement( new Label( &rect, "Texture Quality:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 85;
		PrefsMenuDropDown *texture_res_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_texture_maxres" );
		texture_res_dropdown->AddItem(   "64", "Awful" );
		texture_res_dropdown->AddItem(  "128", "Low" );
		texture_res_dropdown->AddItem(  "256", "Medium" );
		texture_res_dropdown->AddItem( "1024", "Med-Hi" );
		texture_res_dropdown->AddItem(    "0", "High" );
		texture_res_dropdown->Update();
		group->AddElement( texture_res_dropdown );
		rect.x += rect.w + 10;
		rect.w = 85;
		PrefsMenuFilterDropDown *af_dropdown = new PrefsMenuFilterDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0 );
		af_dropdown->AddItem(  "1", "Trilinear" );
		af_dropdown->AddItem(  "2", "2xAF" );
		af_dropdown->AddItem(  "4", "4xAF" );
		af_dropdown->AddItem(  "8", "8xAF" );
		af_dropdown->AddItem( "16", "16xAF" );
		af_dropdown->AddItem( "-1", "Linear" );
		af_dropdown->Update();
		group->AddElement( af_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 90;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Shaders", "g_shader_enable" ) );
		rect.x += rect.w + 10;
		rect.w = 170;
		PrefsMenuDropDown *light_quality_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_shader_light_quality" );
		light_quality_dropdown->AddItem( "0", "No Lighting" );
		light_quality_dropdown->AddItem( "1", "Per-Vertex Lighting" );
		light_quality_dropdown->AddItem( "2", "Per-Pixel Lighting" );
		light_quality_dropdown->AddItem( "3", "Partial BumpMap" );
		light_quality_dropdown->AddItem( "4", "BumpMap Lighting" );
		light_quality_dropdown->Update();
		group->AddElement( light_quality_dropdown );
		rect.x += rect.w + 10;
		rect.w = 120;
		PrefsMenuDropDown *dynamic_lights_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_shader_point_lights" );
		dynamic_lights_dropdown->AddItem( "0", "No Dynamic" );
		dynamic_lights_dropdown->AddItem( "1", "1 Dynamic" );
		dynamic_lights_dropdown->AddItem( "2", "2 Dynamic" );
		dynamic_lights_dropdown->AddItem( "3", "3 Dynamic" );
		dynamic_lights_dropdown->AddItem( "4", "4 Dynamic" );
		dynamic_lights_dropdown->Update();
		group->AddElement( dynamic_lights_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 97;
		group->AddElement( new Label( &rect, "Blastpoints:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 55;
		PrefsMenuDropDown *blastpoints_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_shader_blastpoints" );
		int blastpoints = Raptor::Game->Cfg.SettingAsInt("g_shader_blastpoints");
		blastpoints_dropdown->AddItem( "0", "Off" );
		if( (blastpoints > 0) && (blastpoints < 10) )
			blastpoints_dropdown->AddItem( Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints"), Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints") );
		blastpoints_dropdown->AddItem( "10", "10" );
		if( (blastpoints > 10) && (blastpoints < 20) )
			blastpoints_dropdown->AddItem( Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints"), Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints") );
		blastpoints_dropdown->AddItem( "20", "20" );
		if( (blastpoints > 20) && (blastpoints < 30) )
			blastpoints_dropdown->AddItem( Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints"), Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints") );
		blastpoints_dropdown->AddItem( "30", "30" );
		if( (blastpoints > 30) && (blastpoints < 40) )
			blastpoints_dropdown->AddItem( Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints"), Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints") );
		blastpoints_dropdown->AddItem( "40", "40" );
		if( (blastpoints > 40) && (blastpoints < 50) )
			blastpoints_dropdown->AddItem( Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints"), Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints") );
		blastpoints_dropdown->AddItem( "50", "50" );
		if( (blastpoints > 50) )
			blastpoints_dropdown->AddItem( Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints"), Raptor::Game->Cfg.SettingAsString("g_shader_blastpoints") );
		blastpoints_dropdown->Update();
		group->AddElement( blastpoints_dropdown );
		rect.x += rect.w + 12;
		rect.w = 151;
		group->AddElement( new Label( &rect, "Blastpoint Quality:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 75;
		PrefsMenuDropDown *blastpoint_quality_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_shader_blastpoint_quality" );
		blastpoint_quality_dropdown->AddItem( "-1", "Lowest" );
		blastpoint_quality_dropdown->AddItem( "0", "Low" );
		blastpoint_quality_dropdown->AddItem( "1", "Medium" );
		blastpoint_quality_dropdown->AddItem( "2", "High" );
		int blastpoint_quality = Raptor::Game->Cfg.SettingAsInt("g_shader_blastpoint_quality",2);
		if( blastpoint_quality > 2 )
			blastpoint_quality_dropdown->AddItem( Raptor::Game->Cfg.SettingAsString("g_shader_blastpoint_quality"), "Ultra" );
		blastpoint_quality_dropdown->Update();
		group->AddElement( blastpoint_quality_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 64;
		group->AddElement( new Label( &rect, "Effects:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 75;
		PrefsMenuDropDown *effects_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_effects" );
		double effects_quality = Raptor::Game->Cfg.SettingAsDouble("g_effects",1.);
		if( effects_quality < 0. )
			effects_dropdown->AddItem( Raptor::Game->Cfg.SettingAsString("g_effects"), "Awful" );
		effects_dropdown->AddItem( "0",   "Lowest" );
		effects_dropdown->AddItem( "0.1", "Low" );
		effects_dropdown->AddItem( "0.5", "Medium" );
		effects_dropdown->AddItem( "1",   "High" );
		if( effects_quality > 1. )
			effects_dropdown->AddItem( Raptor::Game->Cfg.SettingAsString("g_effects"), "Ultra" );
		effects_dropdown->Update();
		group->AddElement( effects_dropdown );
		rect.x += rect.w + 20;
		rect.w = 125;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Engine Glow", "g_engine_glow" ) );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 200;
		group->AddElement( new Label( &rect, "Asteroid Level of Detail:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 75;
		PrefsMenuDropDown *asteroid_lod_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_asteroid_lod" );
		asteroid_lod_dropdown->AddItem( "0",    "Awful" );
		asteroid_lod_dropdown->AddItem( "0.15", "Bad" );
		asteroid_lod_dropdown->AddItem( "0.25", "Low" );
		asteroid_lod_dropdown->AddItem( "0.5",  "Medium" );
		asteroid_lod_dropdown->AddItem( "1",    "High" );
		asteroid_lod_dropdown->AddItem( "2",    "Ultra" );
		asteroid_lod_dropdown->Update();
		group->AddElement( asteroid_lod_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 160;
		group->AddElement( new Label( &rect, "Death Star Trench:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 75;
		PrefsMenuDropDown *deathstar_trench_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_deathstar_trench" );
		deathstar_trench_dropdown->AddItem( "0", "Flat" );
		deathstar_trench_dropdown->AddItem( "1", "Awful" );
		deathstar_trench_dropdown->AddItem( "2", "Low" );
		deathstar_trench_dropdown->AddItem( "3", "Medium" );
		deathstar_trench_dropdown->AddItem( "4", "High" );
		deathstar_trench_dropdown->AddItem( "5", "Ultra" );
		deathstar_trench_dropdown->Update();
		group->AddElement( deathstar_trench_dropdown );
		rect.x += rect.w + 9;
		rect.w = 71;
		group->AddElement( new Label( &rect, "Surface:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 75;
		PrefsMenuDropDown *deathstar_surface_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_deathstar_surface" );
		deathstar_surface_dropdown->AddItem( "0", "Flat" );
		deathstar_surface_dropdown->AddItem( "1", "Awful" );
		deathstar_surface_dropdown->AddItem( "2", "Low" );
		deathstar_surface_dropdown->AddItem( "3", "Medium" );
		deathstar_surface_dropdown->AddItem( "4", "High" );
		deathstar_surface_dropdown->AddItem( "5", "Ultra" );
		deathstar_surface_dropdown->Update();
		group->AddElement( deathstar_surface_dropdown );
		
		group->Rect.h = rect.y + rect.h + 13;
		group_rect.h = group->Rect.h;
		
		rect.w = 75;
		rect.x = group_rect.w - rect.w - 10;
		rect.y = effects_dropdown->Rect.y;
		PrefsMenuCheckBox *g_debris_checkbox = new PrefsMenuCheckBox( &rect, LabelFont, "Debris", "g_debris", "500", "0" );
		#ifdef APPLE_POWERPC
			g_debris_checkbox->TrueStr = "200";
		#endif
		if( Raptor::Game->Cfg.SettingAsInt("g_debris") )
		{
			g_debris_checkbox->Checked = true;
			g_debris_checkbox->Image.BecomeInstance( g_debris_checkbox->ImageNormalChecked );  // FIXME: This should probably be moved to RaptorEngine.
		}
		group->AddElement( g_debris_checkbox );
		
		rect.w = 65;
		rect.x = group_rect.w - rect.w - 10;
		rect.y = asteroid_lod_dropdown->Rect.y;
		PrefsMenuCheckBox *g_sway_checkbox = new PrefsMenuCheckBox( &rect, LabelFont, "Sway", "g_sway", "1", "0" );
		if( Raptor::Game->Cfg.SettingAsDouble("g_sway") )
		{
			g_sway_checkbox->Checked = true;
			g_sway_checkbox->Image.BecomeInstance( g_sway_checkbox->ImageNormalChecked );  // FIXME: This should probably be moved to RaptorEngine.
		}
		group->AddElement( g_sway_checkbox );
		
		// --------------------------------------------------------------------------------------------------------------------
		// Virtual Reality
		
		group_rect.y += group_rect.h + 5;
		group = new GroupBox( &group_rect, "Virtual Reality", ItemFont );
		AddElement( group );
		
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		rect.w = 90;
		PrefsMenuVRCheckBox *vr_enable_checkbox = new PrefsMenuVRCheckBox( &rect, LabelFont, "VR Mode" );
		group->AddElement( vr_enable_checkbox );
		#ifdef NO_VR
			vr_enable_checkbox->Enabled = false;
		#endif
		
		rect.x += rect.w + 5;
		rect.w = 110;
		PrefsMenuCheckBox *vr_always_checkbox = new PrefsMenuCheckBox( &rect, LabelFont, "Start in VR", "vr_always" );
		group->AddElement( vr_always_checkbox );
		#ifdef NO_VR
			vr_always_checkbox->Enabled = false;
		#endif
		
		rect.x += rect.w + 5;
		rect.w = 65;
		PrefsMenuCheckBox *vr_sway_checkbox = new PrefsMenuCheckBox( &rect, LabelFont, "Sway", "vr_sway", "1", "0" );
		if( Raptor::Game->Cfg.SettingAsDouble("vr_sway") )
		{
			vr_sway_checkbox->Checked = true;
			vr_sway_checkbox->Image.BecomeInstance( vr_sway_checkbox->ImageNormalChecked );  // FIXME: This should probably be moved to RaptorEngine.
		}
		group->AddElement( vr_sway_checkbox );
		
		rect.y += rect.h + 8;
		rect.x = 10;
		rect.w = 165;
		group->AddElement( new Label( &rect, "FOV / vFOV(-):", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 105;
		group->AddElement( new PrefsMenuTextBox( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, "vr_fov" ) );
		
		rect.y += rect.h + 8;
		rect.x = 10;
		rect.w = 165;
		group->AddElement( new Label( &rect, "Eye Separation (m):", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 105;
		group->AddElement( new PrefsMenuTextBox( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, "vr_separation" ) );
		
		rect.y += rect.h + 8;
		rect.x = 10;
		rect.w = 165;
		group->AddElement( new Label( &rect, "Center Offset (px):", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 105;
		group->AddElement( new PrefsMenuTextBox( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, "vr_offset" ) );
		
		group->Rect.w = rect.x + rect.w + 10;
		group->Rect.h = rect.y + rect.h + 10;
		vr_sway_checkbox->Rect.x = group->Rect.w - vr_sway_checkbox->Rect.w - 10;
		
		// --------------------------------------------------------------------------------------------------------------------
		// Networking
		
		group = new GroupBox( &(group->Rect), "Networking", ItemFont );
		group->Rect.x += group->Rect.w + 10;
		group->Rect.w = Rect.w - group->Rect.x - 10;
		AddElement( group );
		
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		
		rect.w = 45;
		group->AddElement( new Label( &rect, "Rate:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 45;
		PrefsMenuDropDown *netrate_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "netrate" );
		std::set<int> rate_list;
		int current_rate = Raptor::Game->Cfg.SettingAsInt("netrate");
		if( current_rate )
			rate_list.insert( current_rate );
		for( int i = 10; i <= 30; i += 10 )
			rate_list.insert( i );
		for( std::set<int>::const_iterator rate_iter = rate_list.begin(); rate_iter != rate_list.end(); rate_iter ++ )
		{
			int rate = *rate_iter;
			netrate_dropdown->AddItem( Num::ToString(rate), Num::ToString(rate) );
		}
		netrate_dropdown->Update();
		group->AddElement( netrate_dropdown );
		
		rect.x += rect.w + 15;
		rect.w = 65;
		group->AddElement( new Label( &rect, "Predict:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 115;
		PrefsMenuDropDown *zerolag_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "net_zerolag" );
		zerolag_dropdown->AddItem( "0", "Nothing" );
		zerolag_dropdown->AddItem( "1", "My Lasers" );
		zerolag_dropdown->AddItem( "2", "All My Shots" );
		zerolag_dropdown->Update();
		group->AddElement( zerolag_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 195;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Screensaver Connect", "screensaver_connect" ) );
		
		rect.x += rect.w + 5;
		group->AddElement( new PrefsMenuSecretCheckBox( &rect, LabelFont, "Dark Side" ) );
		
		group->Rect.h = rect.y + rect.h + 10;
		
		// --------------------------------------------------------------------------------------------------------------------
		// Dark Side
		/*
		rect.x = group->Rect.x;
		rect.y = group->Rect.y + group->Rect.h + 10;
		rect.w = 100;
		rect.h = ItemFont ? ItemFont->GetHeight() : 18;
		PrefsMenuSecretCheckBox *darkside_checkbox = new PrefsMenuSecretCheckBox( &rect, LabelFont, "Dark Side" );
		AddElement( darkside_checkbox );
		*/
		
		// --------------------------------------------------------------------------------------------------------------------
		// UI
		
		group_rect.x += group_rect.w + 10;
		group_rect.y = 50;
		group_rect.w = Rect.w - group_rect.x - 10;
		group = new GroupBox( &group_rect, "UI", ItemFont );
		AddElement( group );
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		
		rect.w = 90;
		group->AddElement( new Label( &rect, "Target Box:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 75;
		PrefsMenuDropDown *color1_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "ui_box_color" );
		color1_dropdown->InvertMouseWheel = false;
		color1_dropdown->AddItem( "1,1,1,1",   "White" );
		color1_dropdown->AddItem( "1,0.1,0,1", "Red" );
		color1_dropdown->AddItem( "0,1,0,1",   "Green" );
		color1_dropdown->AddItem( "0,0.4,1,1", "Blue" );
		color1_dropdown->AddItem( "0,1,1,1",   "Cyan" );
		color1_dropdown->AddItem( "1,0,1,1",   "Magenta" );
		color1_dropdown->AddItem( "1,1,0,1",   "Yellow" );
		color1_dropdown->AddItem( "1,0.5,0,1", "Orange" );
		color1_dropdown->AddItem( "0.5,0,1,1", "Purple" );
		color1_dropdown->AddItem( "1,0.5,1,1", "Pink" );
		color1_dropdown->AddItem( "team",      "Team" );
		color1_dropdown->Update();
		group->AddElement( color1_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 90;
		group->AddElement( new Label( &rect, "Friendly:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 75;
		PrefsMenuDropDown *color2_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "ui_box_color2" );
		color2_dropdown->InvertMouseWheel = false;
		color2_dropdown->AddItem( "1,1,1,1",   "White" );
		color2_dropdown->AddItem( "1,0.1,0,1", "Red" );
		color2_dropdown->AddItem( "0,1,0,1",   "Green" );
		color2_dropdown->AddItem( "0,0.4,1,1", "Blue" );
		color2_dropdown->AddItem( "0,1,1,1",   "Cyan" );
		color2_dropdown->AddItem( "1,0,1,1",   "Magenta" );
		color2_dropdown->AddItem( "1,1,0,1",   "Yellow" );
		color2_dropdown->AddItem( "1,0.5,0,1", "Orange" );
		color2_dropdown->AddItem( "0.5,0,1,1", "Purple" );
		color2_dropdown->AddItem( "1,0.5,1,1", "Pink" );
		color2_dropdown->AddItem( "team",      "Team" );
		color2_dropdown->Update();
		group->AddElement( color2_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 90;
		group->AddElement( new Label( &rect, "Thickness:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 75;
		PrefsMenuDropDown *thickness_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "ui_box_line" );
		thickness_dropdown->AddItem( "1",   "Slim" );
		thickness_dropdown->AddItem( "1.5", "Medium" );
		thickness_dropdown->AddItem( "2",   "Thic" );
		thickness_dropdown->AddItem( "2.5", "Thicc" );
		thickness_dropdown->AddItem( "3",   "Thiccc" );
		thickness_dropdown->AddItem( "3.5", "Thicccc" );
		thickness_dropdown->AddItem( "4",   "Thiccccc" );
		thickness_dropdown->Update();
		group->AddElement( thickness_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 170;
		rect.w = 50;
		group->AddElement( new Label( &rect, "Style:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 115;
		PrefsMenuDropDown *ui_box_style = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "ui_box_style" );
		ui_box_style->InvertMouseWheel = false;
		ui_box_style->AddItem( "0", "Classic Rect" );
		ui_box_style->AddItem( "1", "Modern" );
		ui_box_style->AddItem( "2", "Squared" );
		ui_box_style->Update();
		group->AddElement( ui_box_style );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 175;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Classic Target Info", "ui_classic" ) );
		
		/*
		rect.y += rect.h + 8;
		rect.w = 170;
		PrefsMenuCheckBox *ui_ship_rotate = new PrefsMenuCheckBox( &rect, LabelFont, "Rotate Lobby Ship", "ui_ship_rotate", "20", "0" );
		group->AddElement( ui_ship_rotate );
		if( Raptor::Game->Cfg.SettingAsDouble("ui_ship_rotate") )
		{
			ui_ship_rotate->Checked = true;
			ui_ship_rotate->Image.BecomeInstance( ui_ship_rotate->ImageNormalChecked );  // FIXME: This should probably be moved to RaptorEngine.
		}
		*/
		
		rect.y += rect.h + 8;
		rect.w = 160;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Menu Auto-Pause", "ui_pause" ) );  // FIXME: Make new class that pauses/unpauses when clicked.
		
		rect.y += rect.h + 8;
		rect.w = 155;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Show Framerate", "showfps" ) );
		
		rect.y += rect.h + 8;
		rect.w = 150;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Cinematic Mode", "cinematic" ) );
	}
	else if( Page == PrefsMenu::PAGE_AUDIO )
	{
		// --------------------------------------------------------------------------------------------------------------------
		// Sound Volume
		
		group_rect.x = 10;
		group_rect.y = 50;
		group_rect.w = 240;
		group_rect.h = 30;
		GroupBox *volume_group = group = new GroupBox( &group_rect, "Sound Volume", ItemFont );
		AddElement( group );
		
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		rect.h = ItemFont ? ItemFont->GetHeight() : 18;
		
		rect.w = 65;
		group->AddElement( new Label( &rect, "Master:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 90;
		PrefsMenuDropDown *s_volume_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_volume" );
		s_volume_dropdown->AddItem( "0",   "Mute" );
		s_volume_dropdown->AddItem( "0.1", "10%" );
		s_volume_dropdown->AddItem( "0.2", "20%" );
		s_volume_dropdown->AddItem( "0.3", "30%" );
		s_volume_dropdown->AddItem( "0.4", "40%" );
		s_volume_dropdown->AddItem( "0.5", "50%" );
		s_volume_dropdown->AddItem( "0.6", "60%" );
		s_volume_dropdown->AddItem( "0.7", "70%" );
		s_volume_dropdown->AddItem( "0.8", "80%" );
		s_volume_dropdown->AddItem( "0.9", "90%" );
		s_volume_dropdown->AddItem( "1",   "100%" );
		s_volume_dropdown->Update();
		group->AddElement( s_volume_dropdown );
		
		rect.y += rect.h + 8;
		rect.x = 20;
		rect.w = 65;
		group->AddElement( new Label( &rect, "Effects:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 80;
		PrefsMenuDropDown *s_effect_volume_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_effect_volume" );
		s_effect_volume_dropdown->AddItem( "0",   "Mute" );
		s_effect_volume_dropdown->AddItem( "0.1", "10%" );
		s_effect_volume_dropdown->AddItem( "0.2", "20%" );
		s_effect_volume_dropdown->AddItem( "0.3", "30%" );
		s_effect_volume_dropdown->AddItem( "0.4", "40%" );
		s_effect_volume_dropdown->AddItem( "0.5", "50%" );
		s_effect_volume_dropdown->AddItem( "0.6", "60%" );
		s_effect_volume_dropdown->AddItem( "0.7", "70%" );
		s_effect_volume_dropdown->AddItem( "0.8", "80%" );
		s_effect_volume_dropdown->AddItem( "0.9", "90%" );
		s_effect_volume_dropdown->AddItem( "1",   "100%" );
		s_effect_volume_dropdown->Update();
		group->AddElement( s_effect_volume_dropdown );
		
		rect.y += rect.h + 8;
		rect.x = 30;
		rect.w = 65;
		group->AddElement( new Label( &rect, "Engine:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 70;
		PrefsMenuDropDown *s_engine_volume_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_engine_volume" );
		s_engine_volume_dropdown->AddItem( "0",   "Mute" );
		s_engine_volume_dropdown->AddItem( "0.1", "10%" );
		s_engine_volume_dropdown->AddItem( "0.2", "20%" );
		s_engine_volume_dropdown->AddItem( "0.3", "30%" );
		s_engine_volume_dropdown->AddItem( "0.4", "40%" );
		s_engine_volume_dropdown->AddItem( "0.5", "50%" );
		s_engine_volume_dropdown->AddItem( "0.6", "60%" );
		s_engine_volume_dropdown->AddItem( "0.7", "70%" );
		s_engine_volume_dropdown->AddItem( "0.8", "80%" );
		s_engine_volume_dropdown->AddItem( "0.9", "90%" );
		s_engine_volume_dropdown->AddItem( "1",   "100%" );
		s_engine_volume_dropdown->Update();
		group->AddElement( s_engine_volume_dropdown );
		
		rect.y += rect.h + 8;
		rect.x = 20;
		rect.w = 65;
		group->AddElement( new Label( &rect, "Comm:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 80;
		PrefsMenuDropDown *s_voice_volume_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_voice_volume" );
		s_voice_volume_dropdown->AddItem( "0",   "Mute" );
		s_voice_volume_dropdown->AddItem( "0.1", "10%" );
		s_voice_volume_dropdown->AddItem( "0.2", "20%" );
		s_voice_volume_dropdown->AddItem( "0.3", "30%" );
		s_voice_volume_dropdown->AddItem( "0.4", "40%" );
		s_voice_volume_dropdown->AddItem( "0.5", "50%" );
		s_voice_volume_dropdown->AddItem( "0.6", "60%" );
		s_voice_volume_dropdown->AddItem( "0.7", "70%" );
		s_voice_volume_dropdown->AddItem( "0.8", "80%" );
		s_voice_volume_dropdown->AddItem( "0.9", "90%" );
		s_voice_volume_dropdown->AddItem( "1",   "100%" );
		s_voice_volume_dropdown->Update();
		group->AddElement( s_voice_volume_dropdown );
		
		rect.x = 20;
		rect.y += rect.h + 8;
		rect.w = 65;
		group->AddElement( new Label( &rect, "Music:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 80;
		PrefsMenuDropDown *s_music_volume_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_music_volume" );
		s_music_volume_dropdown->AddItem( "0",   "Mute" );
		s_music_volume_dropdown->AddItem( "0.1", "10%" );
		s_music_volume_dropdown->AddItem( "0.2", "20%" );
		s_music_volume_dropdown->AddItem( "0.3", "30%" );
		s_music_volume_dropdown->AddItem( "0.4", "40%" );
		s_music_volume_dropdown->AddItem( "0.5", "50%" );
		s_music_volume_dropdown->AddItem( "0.6", "60%" );
		s_music_volume_dropdown->AddItem( "0.7", "70%" );
		s_music_volume_dropdown->AddItem( "0.8", "80%" );
		s_music_volume_dropdown->AddItem( "0.9", "90%" );
		s_music_volume_dropdown->AddItem( "1",   "100%" );
		s_music_volume_dropdown->Update();
		group->AddElement( s_music_volume_dropdown );
		
		rect.x = 30;
		rect.y += rect.h + 8;
		rect.w = 140;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Music in Menus", "s_menu_music" ) );
		
		rect.y += rect.h + 8;
		rect.w = 170;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Music During Flight", "s_game_music" ) );
		
		rect.x = 10;
		rect.y += rect.h + 19;
		rect.w = 115;
		group->AddElement( new Label( &rect, "Shield Alarm:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 100;
		PrefsMenuDropDown *s_shield_alarm_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_shield_alarm_radius" );
		s_shield_alarm_dropdown->AddItem( "0",  "Off" );
		s_shield_alarm_dropdown->AddItem( "10", "YT-1300" );
		s_shield_alarm_dropdown->AddItem( "1",  "All Ships" );
		s_shield_alarm_dropdown->Update();
		group->AddElement( s_shield_alarm_dropdown );
		
		group->Rect.h = rect.y + rect.h + 10;
		
		// --------------------------------------------------------------------------------------------------------------------
		// Mixing Quality
		
		group_rect.x += group_rect.w + 10;
		group_rect.y = 50;
		group_rect.w = Rect.w - group_rect.x - 10;
		group = new GroupBox( &group_rect, "Mixing Quality", ItemFont );
		AddElement( group );
		
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		rect.h = ItemFont ? ItemFont->GetHeight() : 18;
		
		rect.w = 145;
		group->AddElement( new Label( &rect, "Output Channels:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 200;
		PrefsMenuDropDown *s_channels_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_channels" );
		s_channels_dropdown->AddItem( "2", "Stereo (2.0)" );
		s_channels_dropdown->AddItem( "4", "Quadraphonic (4.0)" );
		s_channels_dropdown->AddItem( "6", "Surround 5.1" );
		s_channels_dropdown->AddItem( "8", "Surround 7.1" );
#if SDL_VERSION_ATLEAST(2,0,0)
		s_channels_dropdown->Update();
#else
		s_channels_dropdown->LabelText = "Stereo (2.0)";
		s_channels_dropdown->Enabled = false;
#endif
		group->AddElement( s_channels_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 125;
		group->AddElement( new Label( &rect, "Sample Depth:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 65;
		PrefsMenuDropDown *s_depth_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_depth" );
		s_depth_dropdown->AddItem( "16", "16-Bit" );
		s_depth_dropdown->AddItem( "24", "24-Bit" );
		s_depth_dropdown->AddItem( "32", "Float" );
#if SDL_VERSION_ATLEAST(2,0,0)
		s_depth_dropdown->Update();
#else
		s_depth_dropdown->LabelText = "16-Bit";
		s_depth_dropdown->Enabled = false;
#endif
		group->AddElement( s_depth_dropdown );
		
		rect.x += rect.w + 20;
		rect.w = 45;
		group->AddElement( new Label( &rect, "Rate:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 85;
		PrefsMenuDropDown *s_rate_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_rate" );
		s_rate_dropdown->AddItem(  "44100", "44.1KHz" );
		s_rate_dropdown->AddItem(  "48000", "48KHz" );
		s_rate_dropdown->AddItem(  "88200", "88.2KHz" );
		s_rate_dropdown->AddItem(  "96000", "96KHz" );
		s_rate_dropdown->AddItem( "192000", "192KHz" );
		// FIXME: Allow custom value to appear, preferrably in correct order.
#if SDL_VERSION_ATLEAST(2,0,0)
		s_rate_dropdown->Update();
#else
		s_rate_dropdown->LabelText = "44.1KHz";
		s_rate_dropdown->Enabled = false;
#endif
		group->AddElement( s_rate_dropdown );
		
		group->Rect.h = rect.y + rect.h + 10;
		
		// --------------------------------------------------------------------------------------------------------------------
		// Commlink
		
		group_rect.y += group->Rect.h + 10;
		group_rect.w = Rect.w - group_rect.x - 10;
		group = new GroupBox( &group_rect, "Commlink", ItemFont );
		AddElement( group );
		
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		rect.h = ItemFont ? ItemFont->GetHeight() : 18;
		
		rect.w = 205;
		group->AddElement( new Label( &rect, "Microphone Input Gain:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 85;
		PrefsMenuDropDown *s_mic_volume_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_mic_volume" );
		s_mic_volume_dropdown->AddItem( "0.5", "-6 dB" );
		s_mic_volume_dropdown->AddItem( "0.7", "-3 dB" );
		s_mic_volume_dropdown->AddItem( "1",   "+0 dB" );
		s_mic_volume_dropdown->AddItem( "1.4", "+3 dB" );
		s_mic_volume_dropdown->AddItem( "2",   "+6 dB" );
		s_mic_volume_dropdown->AddItem( "2.8", "+9 dB" );
		s_mic_volume_dropdown->AddItem( "auto","Auto" );
		// FIXME: Allow custom value to appear, preferrably in correct order.
		s_mic_volume_dropdown->Update();
		group->AddElement( s_mic_volume_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 205;
		group->AddElement( new Label( &rect, "Maximum Playback Gain:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 85;
		PrefsMenuDropDown *s_autogain_volume_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_voice_autogain_volume" );
		s_autogain_volume_dropdown->AddItem(   "1", " +0 dB" );
		s_autogain_volume_dropdown->AddItem(   "2", " +6 dB" );
		s_autogain_volume_dropdown->AddItem(   "4", "+12 dB" );
		s_autogain_volume_dropdown->AddItem(   "8", "+18 dB" );
		s_autogain_volume_dropdown->AddItem(  "16", "+24 dB" );
		s_autogain_volume_dropdown->AddItem(  "32", "+30 dB" );
		s_autogain_volume_dropdown->AddItem(  "64", "+36 dB" );
		s_autogain_volume_dropdown->AddItem( "128", "+40 dB" );
		// FIXME: Allow custom value to appear, preferrably in correct order.
		s_autogain_volume_dropdown->Update();
		group->AddElement( s_autogain_volume_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 240;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Always Use Automatic Gain", "s_voice_autogain_always" ) );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 200;
		PrefsMenuCheckBox *positional_checkbox = new PrefsMenuCheckBox( &rect, LabelFont, "Positional Team Voice", "s_voice_positional" );
		group->AddElement( positional_checkbox );
		if( Raptor::Game->Cfg.SettingAsBool("s_voice_positional") )
		{
			positional_checkbox->Checked = true;
			positional_checkbox->Image.BecomeInstance( positional_checkbox->ImageNormalChecked );  // FIXME: This should probably be moved to RaptorEngine.
			if( Raptor::Game->Cfg.SettingAsInt("s_voice_positional") >= 2 )
			{
				positional_checkbox->LabelText = "Positional Voice (All Channels)";
				positional_checkbox->Rect.w = 250;
			}
		}
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 205;
		group->AddElement( new Label( &rect, "Music Scale for Comms:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 85;
		PrefsMenuDropDown *s_music_scale_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_voice_scale_music" );
		s_music_scale_dropdown->AddItem( "0",   "Mute" );
		s_music_scale_dropdown->AddItem( "0.1", "10%" );
		s_music_scale_dropdown->AddItem( "0.2", "20%" );
		s_music_scale_dropdown->AddItem( "0.3", "30%" );
		s_music_scale_dropdown->AddItem( "0.4", "40%" );
		s_music_scale_dropdown->AddItem( "0.5", "50%" );
		s_music_scale_dropdown->AddItem( "0.6", "60%" );
		s_music_scale_dropdown->AddItem( "0.7", "70%" );
		s_music_scale_dropdown->AddItem( "0.8", "80%" );
		s_music_scale_dropdown->AddItem( "0.9", "90%" );
		s_music_scale_dropdown->AddItem( "1",   "100%" );
		s_music_scale_dropdown->Update();
		group->AddElement( s_music_scale_dropdown );
		
		group->Rect.h = rect.y + rect.h + 10;
		
		rect.x = volume_group->Rect.x;
		rect.y = volume_group->Rect.y + volume_group->Rect.h + 10;
		rect.w = Rect.w - rect.x * 2;
		rect.h = 85;
		AddElement( new PrefsMenuSillyButton( &rect, TitleFont ) );
	}
	else if( Page == PrefsMenu::PAGE_CONTROLS )
	{
		// --------------------------------------------------------------------------------------------------------------------
		// Joystick
		
		group_rect.x = 10;
		group_rect.y = 50;
		group_rect.w = 200;
		group_rect.h = 135;
		group = new GroupBox( &group_rect, "Joystick", ItemFont );
		AddElement( group );
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		
		bool joy_calibrated = false, xbox_calibrated = false;
		for( std::map<std::string,std::string>::const_iterator setting_iter = Raptor::Game->Cfg.Settings.begin(); setting_iter != Raptor::Game->Cfg.Settings.end(); setting_iter ++ )
		{
			if( Str::BeginsWith( setting_iter->first, "joy_cal_xbox_" ) )
				xbox_calibrated = true;
			else if( Str::BeginsWith( setting_iter->first, "joy_cal_" ) )
				joy_calibrated = true;
		}
		
		rect.h = ItemFont ? ItemFont->GetHeight() : 18;
		rect.w = 90;
		group->AddElement( new Label( &rect, "Deadzone:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 55;
		PrefsMenuDropDown *joy_deadzone_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_deadzone" );
		joy_deadzone_dropdown->AddItem( "0", "None" );
		joy_deadzone_dropdown->AddItem( "0.01", "1%" );
		joy_deadzone_dropdown->AddItem( "0.02", "2%" );
		joy_deadzone_dropdown->AddItem( "0.03", "3%" );
		joy_deadzone_dropdown->AddItem( "0.04", "4%" );
		joy_deadzone_dropdown->AddItem( "0.05", "5%" );
		joy_deadzone_dropdown->AddItem( "0.06", "6%" );
		joy_deadzone_dropdown->AddItem( "0.07", "7%" );
		joy_deadzone_dropdown->AddItem( "0.08", "8%" );
		joy_deadzone_dropdown->AddItem( "0.09", "9%" );
		joy_deadzone_dropdown->AddItem( "0.1", "10%" );
		joy_deadzone_dropdown->AddItem( "0.11", "11%" );
		joy_deadzone_dropdown->AddItem( "0.12", "12%" );
		joy_deadzone_dropdown->AddItem( "0.13", "13%" );
		joy_deadzone_dropdown->AddItem( "0.14", "14%" );
		joy_deadzone_dropdown->AddItem( "0.15", "15%" );
		joy_deadzone_dropdown->AddItem( "0.16", "16%" );
		joy_deadzone_dropdown->AddItem( "0.17", "17%" );
		joy_deadzone_dropdown->AddItem( "0.18", "18%" );
		joy_deadzone_dropdown->AddItem( "0.19", "19%" );
		joy_deadzone_dropdown->AddItem( "0.2", "20%" );
		joy_deadzone_dropdown->Update();
		group->AddElement( joy_deadzone_dropdown );
		if( joy_calibrated )
		{
			rect.x += rect.w + 1;
			rect.w = 20;
			group->AddElement( new Label( &rect, "*", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		}
		
		rect.y += rect.h + 8;
		rect.x = 20;
		rect.w = 20;
		group->AddElement( new Label( &rect, "X:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 105;
		PrefsMenuDropDown *joy_smooth_x_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_x" );
		joy_smooth_x_dropdown->AddItem( "-1", "Digital" );
		joy_smooth_x_dropdown->AddItem( "-0.75", "Sharpest" );
		joy_smooth_x_dropdown->AddItem( "-0.5", "Sharper" );
		joy_smooth_x_dropdown->AddItem( "-0.25", "Sharp" );
		joy_smooth_x_dropdown->AddItem( "0", "Linear" );
		joy_smooth_x_dropdown->AddItem( "0.125", "Twitchy" );
		joy_smooth_x_dropdown->AddItem( "0.25", "Responsive" );
		joy_smooth_x_dropdown->AddItem( "0.5", "Medium" );
		joy_smooth_x_dropdown->AddItem( "0.75", "Smooth" );
		joy_smooth_x_dropdown->AddItem( "1", "Smoother" );
		joy_smooth_x_dropdown->AddItem( "2", "Smoothest" );
		joy_smooth_x_dropdown->AddItem( "3", "Buttery" );
		joy_smooth_x_dropdown->Update();
		group->AddElement( joy_smooth_x_dropdown );
		
		rect.y += rect.h + 3;
		rect.x = 20;
		rect.w = 20;
		group->AddElement( new Label( &rect, "Y:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 105;
		PrefsMenuDropDown *joy_smooth_y_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_y" );
		joy_smooth_y_dropdown->AddItem( "-1", "Digital" );
		joy_smooth_y_dropdown->AddItem( "-0.75", "Sharpest" );
		joy_smooth_y_dropdown->AddItem( "-0.5", "Sharper" );
		joy_smooth_y_dropdown->AddItem( "-0.25", "Sharp" );
		joy_smooth_y_dropdown->AddItem( "0", "Linear" );
		joy_smooth_y_dropdown->AddItem( "0.125", "Twitchy" );
		joy_smooth_y_dropdown->AddItem( "0.25", "Responsive" );
		joy_smooth_y_dropdown->AddItem( "0.5", "Medium" );
		joy_smooth_y_dropdown->AddItem( "0.75", "Smooth" );
		joy_smooth_y_dropdown->AddItem( "1", "Smoother" );
		joy_smooth_y_dropdown->AddItem( "2", "Smoothest" );
		joy_smooth_y_dropdown->AddItem( "3", "Buttery" );
		joy_smooth_y_dropdown->Update();
		group->AddElement( joy_smooth_y_dropdown );
		
		rect.y += rect.h + 3;
		rect.x = 20;
		rect.w = 55;
		group->AddElement( new Label( &rect, "Twist:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 105;
		PrefsMenuDropDown *joy_smooth_z_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_z" );
		joy_smooth_z_dropdown->AddItem( "-1", "Digital" );
		joy_smooth_z_dropdown->AddItem( "-0.75", "Sharpest" );
		joy_smooth_z_dropdown->AddItem( "-0.5", "Sharper" );
		joy_smooth_z_dropdown->AddItem( "-0.25", "Sharp" );
		joy_smooth_z_dropdown->AddItem( "0", "Linear" );
		joy_smooth_z_dropdown->AddItem( "0.125", "Twitchy" );
		joy_smooth_z_dropdown->AddItem( "0.25", "Responsive" );
		joy_smooth_z_dropdown->AddItem( "0.5", "Medium" );
		joy_smooth_z_dropdown->AddItem( "0.75", "Smooth" );
		joy_smooth_z_dropdown->AddItem( "1", "Smoother" );
		joy_smooth_z_dropdown->AddItem( "2", "Smoothest" );
		joy_smooth_z_dropdown->AddItem( "3", "Buttery" );
		joy_smooth_z_dropdown->Update();
		group->AddElement( joy_smooth_z_dropdown );
		
		// --------------------------------------------------------------------------------------------------------------------
		// Controller
		
		group_rect.y += group_rect.h + 5;
		group_rect.h = 116;
		group = new GroupBox( &group_rect, "Controller", ItemFont );
		AddElement( group );
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		
		rect.w = 90;
		group->AddElement( new Label( &rect, "Deadzone:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 55;
		PrefsMenuDropDown *joy_deadzone_thumbsticks_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_deadzone_thumbsticks" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0", "None" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.01", "1%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.02", "2%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.03", "3%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.04", "4%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.05", "5%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.06", "6%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.07", "7%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.08", "8%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.09", "9%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.1", "10%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.11", "11%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.12", "12%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.13", "13%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.14", "14%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.15", "15%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.16", "16%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.17", "17%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.18", "18%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.19", "19%" );
		joy_deadzone_thumbsticks_dropdown->AddItem( "0.2", "20%" );
		joy_deadzone_thumbsticks_dropdown->Update();
		group->AddElement( joy_deadzone_thumbsticks_dropdown );
		if( xbox_calibrated )
		{
			rect.x += rect.w + 1;
			rect.w = 20;
			group->AddElement( new Label( &rect, "*", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		}
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 70;
		group->AddElement( new Label( &rect, "Sticks:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 105;
		PrefsMenuDropDown *joy_smooth_thumbsticks_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_thumbsticks" );
		joy_smooth_thumbsticks_dropdown->AddItem( "-1", "Digital" );
		joy_smooth_thumbsticks_dropdown->AddItem( "-0.75", "Sharpest" );
		joy_smooth_thumbsticks_dropdown->AddItem( "-0.5", "Sharper" );
		joy_smooth_thumbsticks_dropdown->AddItem( "-0.25", "Sharp" );
		joy_smooth_thumbsticks_dropdown->AddItem( "0", "Linear" );
		joy_smooth_thumbsticks_dropdown->AddItem( "0.125", "Twitchy" );
		joy_smooth_thumbsticks_dropdown->AddItem( "0.25", "Responsive" );
		joy_smooth_thumbsticks_dropdown->AddItem( "0.5", "Medium" );
		joy_smooth_thumbsticks_dropdown->AddItem( "0.75", "Smooth" );
		joy_smooth_thumbsticks_dropdown->AddItem( "1", "Smoother" );
		joy_smooth_thumbsticks_dropdown->AddItem( "2", "Smoothest" );
		joy_smooth_thumbsticks_dropdown->AddItem( "3", "Buttery" );
		joy_smooth_thumbsticks_dropdown->Update();
		group->AddElement( joy_smooth_thumbsticks_dropdown );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 70;
		group->AddElement( new Label( &rect, "Triggers:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 105;
		PrefsMenuDropDown *joy_smooth_triggers_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_triggers" );
		joy_smooth_triggers_dropdown->AddItem( "-1", "Digital" );
		joy_smooth_triggers_dropdown->AddItem( "-0.75", "Sharpest" );
		joy_smooth_triggers_dropdown->AddItem( "-0.5", "Sharper" );
		joy_smooth_triggers_dropdown->AddItem( "-0.25", "Sharp" );
		joy_smooth_triggers_dropdown->AddItem( "0", "Linear" );
		joy_smooth_triggers_dropdown->AddItem( "0.125", "Twitchy" );
		joy_smooth_triggers_dropdown->AddItem( "0.25", "Responsive" );
		joy_smooth_triggers_dropdown->AddItem( "0.5", "Medium" );
		joy_smooth_triggers_dropdown->AddItem( "0.75", "Smooth" );
		joy_smooth_triggers_dropdown->AddItem( "1", "Smoother" );
		joy_smooth_triggers_dropdown->AddItem( "2", "Smoothest" );
		joy_smooth_triggers_dropdown->AddItem( "3", "Buttery" );
		joy_smooth_triggers_dropdown->Update();
		group->AddElement( joy_smooth_triggers_dropdown );
		
		// --------------------------------------------------------------------------------------------------------------------
		// Mouse
		
		group_rect.y += group_rect.h + 5;
		group_rect.h = 116;
		group = new GroupBox( &group_rect, "Mouse", ItemFont );
		AddElement( group );
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		
		rect.w = 60;
		group->AddElement( new Label( &rect, "Mode:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 110;
		PrefsMenuDropDown *mouse_mode_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "mouse_mode" );
		mouse_mode_dropdown->AddItem( "disabled", "Disabled" );
		mouse_mode_dropdown->AddItem( "gunner", "Turret Aim" );
		mouse_mode_dropdown->AddItem( "fly", "Yaw/Pitch" );
		mouse_mode_dropdown->AddItem( "fly2", "Roll/Pitch" );
		mouse_mode_dropdown->AddItem( "look", "Freelook" );
		mouse_mode_dropdown->Update();
		group->AddElement( mouse_mode_dropdown );
		
		rect.y += rect.h + 8;
		rect.x = 10;
		rect.w = 120;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Invert Pitch", "mouse_invert" ) );
		
		rect.y += rect.h + 8;
		rect.x = 10;
		rect.w = 60;
		group->AddElement( new Label( &rect, "Input:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 105;
		PrefsMenuDropDown *mouse_smooth_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "mouse_smooth" );
		mouse_smooth_dropdown->AddItem( "-0.75", "Sharpest" );
		mouse_smooth_dropdown->AddItem( "-0.5", "Sharper" );
		mouse_smooth_dropdown->AddItem( "-0.25", "Sharp" );
		mouse_smooth_dropdown->AddItem( "0", "Linear" );
		mouse_smooth_dropdown->AddItem( "0.125", "Twitchy" );
		mouse_smooth_dropdown->AddItem( "0.25", "Responsive" );
		mouse_smooth_dropdown->AddItem( "0.5", "Medium" );
		mouse_smooth_dropdown->AddItem( "0.75", "Smooth" );
		mouse_smooth_dropdown->AddItem( "1", "Smoother" );
		mouse_smooth_dropdown->AddItem( "2", "Smoothest" );
		mouse_smooth_dropdown->AddItem( "3", "Buttery" );
		mouse_smooth_dropdown->Update();
		group->AddElement( mouse_smooth_dropdown );
		
		// --------------------------------------------------------------------------------------------------------------------
		// Global Toggles
		
		rect.x = 10;
		rect.y = group_rect.y + group_rect.h + 15;
		rect.w = 150;
		AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Swap Yaw/Roll", "swap_yaw_roll" ) );
		
		rect.x += rect.w + 10;
		rect.w = 145;
		AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Invert Turrets", "turret_invert" ) );
		
		// --------------------------------------------------------------------------------------------------------------------
		// Binds
		
		group_rect.x += group_rect.w + 10;
		group_rect.y = 50;
		group_rect.w = Rect.w - group_rect.x - 10;
		group_rect.h = Rect.h - group_rect.y - 70;
		group = new GroupBox( &group_rect, "Binds", ItemFont );
		AddElement( group );
		
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		rect.w = 270;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Show Only Connected Devices", "joy_hide_binds" ) );
		
		rect.w = 90;
		rect.x = group->Rect.w - rect.w - 10;
		group->AddElement( new PrefsMenuRefreshButton( &rect, ItemFont, "Refresh" ) );
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = group_rect.w - 20;
		rect.h = group_rect.h - rect.y - 10;
		
		ScrollArea *scroll_area = new ScrollArea( &rect, 12 );
		group->AddElement( scroll_area );
		
		rect.x = 0;
		rect.y = 0;
		rect.w = scroll_area->Rect.w - scroll_area->ScrollBarSize - 5;
		rect.h = 15;
		
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::ROLL ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::ROLL_INVERTED ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::PITCH ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::PITCH_INVERTED ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::YAW ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::YAW_INVERTED ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::THROTTLE ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::THROTTLE_INVERTED ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_X ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_X_INVERTED ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_Y ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_Y_INVERTED ], ControlFont, BindFont ) );
		
		rect.y += rect.h + 10;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::ROLL_LEFT ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::ROLL_LEFT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::ROLL_RIGHT ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::ROLL_RIGHT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::PITCH_UP ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::PITCH_UP ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::PITCH_DOWN ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::PITCH_DOWN ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::YAW_LEFT ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::YAW_LEFT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::YAW_RIGHT ], ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::YAW_RIGHT ], ControlFont, BindFont ) );
		
		rect.y += rect.h + 10;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::THROTTLE_UP ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::THROTTLE_DOWN ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::THROTTLE_0 ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::THROTTLE_33 ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::THROTTLE_66 ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::THROTTLE_100 ], ControlFont, BindFont ) );
		
		rect.y += rect.h + 10;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::FIRE ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::WEAPON ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::MODE ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::SHIELD_DIR ], ControlFont, BindFont ) );
		
		rect.y += rect.h + 10;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_CROSSHAIR ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_NOTHING ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_NEAREST_ENEMY ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_NEAREST_ATTACKER ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_NEWEST_INCOMING ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_OBJECTIVE ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_DOCKABLE ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_TARGET_ATTACKER ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_NEXT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_PREV ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_NEXT_ENEMY ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_PREV_ENEMY ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_NEXT_FRIENDLY ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_PREV_FRIENDLY ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_NEXT_PLAYER ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_PREV_PLAYER ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_NEXT_SUBSYSTEM ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_PREV_SUBSYSTEM ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_GROUPMATE ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_SYNC ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_NEWEST ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET1 ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET2 ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET3 ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET4 ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::TARGET_STORE ], ControlFont, BindFont ) );
		
		rect.y += rect.h + 10;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::SEAT_COCKPIT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::SEAT_GUNNER1 ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::SEAT_GUNNER2 ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::CHEWIE_TAKE_THE_WHEEL ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::EJECT ], ControlFont, BindFont ) );
		
		rect.y += rect.h + 10;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_CENTER ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_UP ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_DOWN ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_LEFT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_RIGHT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_UP_LEFT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_UP_RIGHT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_DOWN_LEFT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::LOOK_DOWN_RIGHT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::GLANCE_UP ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::GLANCE_BACK ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::GLANCE_LEFT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::GLANCE_RIGHT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::GLANCE_UP_LEFT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::GLANCE_UP_RIGHT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::GLANCE_BACK_LEFT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::GLANCE_BACK_RIGHT ], ControlFont, BindFont ) );
		
		rect.y += rect.h + 10;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VIEW_COCKPIT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VIEW_CROSSHAIR ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VIEW_CHASE ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VIEW_PADLOCK ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VIEW_STATIONARY ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VIEW_CINEMA ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VIEW_FIXED ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VIEW_GUNNER ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VIEW_CYCLE ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VIEW_INSTRUMENTS ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VIEW_SELFIE ], ControlFont, BindFont ) );
		
		rect.y += rect.h + 10;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::CHAT ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::CHAT_TEAM ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VOICE_TEAM ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::VOICE_ALL ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::SCORES ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::PAUSE ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::MENU ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::PREFS ], ControlFont, BindFont ) );
	}
	else if( Page == PrefsMenu::PAGE_CALIBRATION )
	{
		group_rect.x = 10;
		group_rect.y = 50;
		group_rect.w = Rect.w - group_rect.x - 10;
		group_rect.h = Rect.h - group_rect.y - 70;
		group = new GroupBox( &group_rect, "Device:                 ", ItemFont );
		AddElement( group );
		
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		rect.w = group->Rect.w - rect.x - 10;
		rect.h = group->Rect.h - rect.y - 10;
		PrefsMenuCalibrator *calibrator = new PrefsMenuCalibrator( &rect );
		group->AddElement( calibrator );
		
		rect.x = 75;
		rect.y = 1;
		rect.w = 98;
		rect.h = 21;
		group->AddElement( new PrefsMenuCalDevDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, calibrator ) );
		
		rect.x = 10;
		rect.w = 100;
		rect.h = 29;
		rect.y = Rect.h - rect.h - 10;
		AddElement( new PrefsMenuCalClearButton( &rect, ItemFont, "Clear", calibrator ) );
		
		rect.x += rect.w + 10;
		AddElement( new PrefsMenuCalApplyButton( &rect, ItemFont, "Apply", calibrator ) );
		
		rect.x = 10;
		rect.y -= 25;
		rect.w = 240;
		rect.h = ItemFont ? ItemFont->GetHeight() : 18;
		AddElement( new PrefsMenuCalDZCheckBox( &rect, ItemFont, "Deadzone (Gently Wiggle)", calibrator ) );
	}
}


bool PrefsMenu::ChangePage( int page )
{
	if( Page != page )
	{
		Page = page;
		UpdateContents();
		return true;
	}
	return false;
}


void PrefsMenu::Draw( void )
{
	// Keep the size and position up-to-date.
	if( ! Draggable )
	{
		Rect.x = Raptor::Game->Gfx.W/2 - 640/2;
		Rect.y = Raptor::Game->Gfx.H/2 - 480/2;
	}
	Rect.w = 640;
	Rect.h = 480;
	
	Window::Draw();
}


bool PrefsMenu::KeyDown( SDLKey key )
{
	if( key == SDLK_F10 ) // FIXME: Check for XWing::Controls::PREFS instead?
	{
		Remove();
		return true;
	}
	
	return false;
}


bool PrefsMenu::KeyUp( SDLKey key )
{
	if( key == SDLK_ESCAPE )
	{
		if( Selected )
			Selected = NULL;
		else
		{
			Remove();
			
			XWingGame *game = (XWingGame*) Raptor::Game;
			if( Paused && (game->Cfg.KeyBind( SDLK_ESCAPE ) == game->Controls[ XWing::Control::MENU ]) )  // FIXME: Dirty hack to keep it paused for the IngameMenu.
				Paused = false;
		}
		return true;
	}
	
	return false;
}


bool PrefsMenu::MouseDown( Uint8 button )
{
	return true;
}


bool PrefsMenu::MouseUp( Uint8 button )
{
	return true;
}


// ---------------------------------------------------------------------------


PrefsMenuCheckBox::PrefsMenuCheckBox( SDL_Rect *rect, Font *font, std::string label, std::string variable, std::string true_str, std::string false_str ) : CheckBox( rect, font, label, Raptor::Game->Cfg.SettingAsString( variable ) == true_str, Raptor::Game->Res.GetAnimation("box_unchecked.ani"), Raptor::Game->Res.GetAnimation("box_unchecked_mdown.ani"), NULL, Raptor::Game->Res.GetAnimation("box_checked.ani"), Raptor::Game->Res.GetAnimation("box_checked_mdown.ani"), NULL )
{
	Variable = variable;
	TrueStr = true_str;
	FalseStr = false_str;
}


PrefsMenuCheckBox::~PrefsMenuCheckBox()
{
}


void PrefsMenuCheckBox::Changed( void )
{
	Raptor::Game->Cfg.Settings[ Variable ] = Checked ? TrueStr : FalseStr;
}


// ---------------------------------------------------------------------------


PrefsMenuVRCheckBox::PrefsMenuVRCheckBox( SDL_Rect *rect, Font *font, std::string label ) : PrefsMenuCheckBox( rect, font, label, "vr_enable" )
{
}


PrefsMenuVRCheckBox::~PrefsMenuVRCheckBox()
{
}


void PrefsMenuVRCheckBox::Changed( void )
{
	PrefsMenuCheckBox::Changed();
	
	int maxfps = Raptor::Game->Cfg.SettingAsInt( "maxfps", 60 );
	PrefsMenu *prefs_menu = (PrefsMenu*) Container->Container;
	
	if( Checked )
	{
		if( maxfps && (maxfps < 90) )
			Raptor::Game->Cfg.Settings[ "maxfps" ] = "90";
		
		double asteroid_lod = Raptor::Game->Cfg.SettingAsDouble( "g_asteroid_lod", 1. );
		if( asteroid_lod > 0.5 )
			Raptor::Game->Cfg.Settings[ "g_asteroid_lod" ] = "0.5";
		
		// StartVR will set these and call Gfx.Restart if required; update UI and prevent the Done button from restarting again.
		prefs_menu->PrevGfx[ "g_framebuffers" ] = Raptor::Game->Cfg.Settings[ "g_framebuffers" ] = "true";
		prefs_menu->PrevGfx[ "g_vsync"        ] = Raptor::Game->Cfg.Settings[ "g_vsync"        ] = "false";
	}
	else
	{
		if( maxfps == 90 )
		{
			Raptor::Game->Cfg.Settings[ "maxfps" ] = "60";
			
			#ifdef WIN32
				// Ideal maxfps is the display's refresh rate, especially when using vsync.
				DEVMODE dm;
				memset( &dm, 0, sizeof(dm) );
				dm.dmSize = sizeof(DEVMODE);
				if( EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &dm )
				&& (dm.dmFields & DM_DISPLAYFREQUENCY)
				&& (dm.dmDisplayFrequency >= 60) )
					Raptor::Game->Cfg.Settings[ "maxfps" ] = Num::ToString( (int) dm.dmDisplayFrequency );
			#endif
		}
	}
	
	prefs_menu->UpdateContents();
}


// ---------------------------------------------------------------------------


PrefsMenuSecretCheckBox::PrefsMenuSecretCheckBox( SDL_Rect *rect, Font *font, std::string label, std::string variable, std::string true_str, std::string false_str ) : PrefsMenuCheckBox( rect, font, label, variable, true_str, false_str )
{
	TimesClicked = 0;
	Visible = Raptor::Game->Cfg.SettingAsBool( variable );
	
	RedNormal = RedOver = 1.;
	GreenNormal = BlueNormal = GreenOver = BlueOver = 0.;
	GreenDown = 0.5;
}


PrefsMenuSecretCheckBox::~PrefsMenuSecretCheckBox()
{
}


void PrefsMenuSecretCheckBox::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	TimesClicked ++;
	if( (TimesClicked == 7) && ! Visible )
	{
		Visible = true;
		int channel = Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound( Variable + std::string(".wav") ) );
		if( (channel >= 0) && (Raptor::Game->Snd.AttenuateFor < 0) )
		{
			Raptor::Game->Snd.AttenuateFor = channel;
			Raptor::Game->Snd.MusicAttenuate = 0.75f;
			Raptor::Game->Snd.SoundAttenuate = 0.5f;
		}
	}
	
	if( Visible )
	{
		TimesClicked = 0;
		PrefsMenuCheckBox::Clicked( button );
	}
	
	if( (Variable == "darkside") && Raptor::Server->IsRunning() )
		Raptor::Game->Cfg.Command( std::string("sv set darkside ") + (Checked ? TrueStr : FalseStr) );
}


// ---------------------------------------------------------------------------


PrefsMenuRadioButton::PrefsMenuRadioButton( SDL_Rect *rect, Font *font, std::string label, std::string variable, std::string true_str ) : CheckBox( rect, font, label, Raptor::Game->Cfg.SettingAsString( variable ) == true_str, Raptor::Game->Res.GetAnimation("box_unchecked.ani"), Raptor::Game->Res.GetAnimation("box_unchecked_mdown.ani"), NULL, Raptor::Game->Res.GetAnimation("box_checked.ani"), Raptor::Game->Res.GetAnimation("box_checked_mdown.ani"), NULL )
{
	Variable = variable;
	TrueStr = true_str;
}


PrefsMenuRadioButton::~PrefsMenuRadioButton()
{
}


int PrefsMenuRadioButton::GetWidth( void )
{
	SDL_Rect rect;
	LabelFont->TextSize( LabelText, &rect );
	return rect.w + Rect.h;
}


void PrefsMenuRadioButton::Clicked( Uint8 button )
{
	Checked = (button == 0);
	CheckBox::Clicked( button );
	
	if( button )
	{
		for( std::list<PrefsMenuRadioButton*>::iterator other_iter = OtherButtons.begin(); other_iter != OtherButtons.end(); other_iter ++ )
			(*other_iter)->Clicked(0);
		
		Raptor::Game->Cfg.Settings[ Variable ] = TrueStr;
	}
}


// ---------------------------------------------------------------------------


PrefsMenuTextBox::PrefsMenuTextBox( SDL_Rect *rect, Font *font, uint8_t align, std::string variable ) : TextBox( rect, font, align, Raptor::Game->Cfg.SettingAsString( variable ) )
{
	Variable = variable;
	LinkedListBox = NULL;
	ReturnDeselects = true;
}


PrefsMenuTextBox::~PrefsMenuTextBox()
{
}


void PrefsMenuTextBox::Deselected( void )
{
	Raptor::Game->Cfg.Settings[ Variable ] = Text;
}


// ---------------------------------------------------------------------------


PrefsMenuListBox::PrefsMenuListBox( SDL_Rect *rect, Font *font, int scroll_bar_size, std::string variable, TextBox *linked_text_box ) : ListBox( rect, font, scroll_bar_size )
{
	Variable = variable;
	LinkedTextBox = linked_text_box;
}


PrefsMenuListBox::~PrefsMenuListBox()
{
}


void PrefsMenuListBox::Changed( void )
{
	if( LinkedTextBox )
		LinkedTextBox->Text = SelectedValue();
	
	Raptor::Game->Cfg.Settings[ Variable ] = SelectedValue();
}


// ---------------------------------------------------------------------------


PrefsMenuDropDown::PrefsMenuDropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, std::string variable ) : DropDown( rect, font, align, scroll_bar_size, NULL, NULL )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
	InvertMouseWheel = true;
	
	Variable = variable;
	Value = Raptor::Game->Cfg.SettingAsString( Variable );
}


PrefsMenuDropDown::~PrefsMenuDropDown()
{
}


void PrefsMenuDropDown::Update( void )
{
	// If we don't have an item matching the current setting, make a new item called "Custom".
	if( FindItem(Value) < 0 )
	{
		// There can be only one.
		if( Items.size() && (Items.rbegin()->Text == "Custom") )
			Items.erase( Items.end() - 1 );
		
		AddItem( Value, "Custom" );
	}
	
	DropDown::Update();
}


void PrefsMenuDropDown::Changed( void )
{
	Raptor::Game->Cfg.Settings[ Variable ] = Value;
}


// ---------------------------------------------------------------------------


PrefsMenuFilterDropDown::PrefsMenuFilterDropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size ) : DropDown( rect, font, align, scroll_bar_size, NULL, NULL )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
	InvertMouseWheel = true;
	
	if( Raptor::Game->Cfg.SettingAsBool( "g_mipmap", true ) )
		Value = Raptor::Game->Cfg.SettingAsString( "g_af", "1" );
	else
		Value = "-1";
}


PrefsMenuFilterDropDown::~PrefsMenuFilterDropDown()
{
}


void PrefsMenuFilterDropDown::Changed( void )
{
	if( Value == "-1" )
	{
		Raptor::Game->Cfg.Settings[ "g_mipmap" ] = "false";
		Raptor::Game->Cfg.Settings[ "g_af" ] = "1";
	}
	else
	{
		Raptor::Game->Cfg.Settings[ "g_mipmap" ] = "true";
		Raptor::Game->Cfg.Settings[ "g_af" ] = Value;
	}
}


// ---------------------------------------------------------------------------


PrefsMenuDoneButton::PrefsMenuDoneButton( SDL_Rect *rect, Font *button_font, const char *label ) : LabelledButton( rect, button_font, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


PrefsMenuDoneButton::~PrefsMenuDoneButton()
{
}


void PrefsMenuDoneButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	PrefsMenu *menu = (PrefsMenu*) Container;
	
	// Apply joystick calibration.
	PrefsMenuCalibrator *calibrator = (PrefsMenuCalibrator*) menu->FindElement("Calibrator");
	if( calibrator && calibrator->AxisMax.size() )
		calibrator->Apply();
	
	// Restart graphics/shaders if necessary.
	if( menu->GfxSettingsChanged() )
		Raptor::Game->Gfx.Restart();
	
	// Restart VR if it's checked but not working.
	if( (! Raptor::Game->Head.VR) && Raptor::Game->Cfg.SettingAsBool("vr_enable") )
		Raptor::Game->Head.RestartVR();
	
	// Restart sound if necessary.
	if( menu->SndSettingsChanged() )
		Raptor::Game->Snd.Initialize();
	
	// Start/stop the music.
	if( Raptor::Game->State >= XWing::State::FLYING )
	{
		bool s_game_music = Raptor::Game->Cfg.SettingAsBool( "s_game_music" );
		if( s_game_music != Raptor::Game->Snd.PlayMusic )
		{
			if( s_game_music )
				Raptor::Game->Snd.PlayMusicSubdir( "Flight" );
			else
				Raptor::Game->Snd.StopMusic();
		}
	}
	else
	{
		bool s_menu_music = Raptor::Game->Cfg.SettingAsBool( "s_menu_music" );
		if( s_menu_music != Raptor::Game->Snd.PlayMusic )
		{
			if( s_menu_music )
				Raptor::Game->Snd.PlayMusicSubdir( Raptor::Game->Layers.Find("LobbyMenu") ? "Lobby" : "Menu" );
			else
				Raptor::Game->Snd.StopMusic();
		}
	}
	
#ifndef _DEBUG
	// Make sure all resources that need to be loaded for these settings have been loaded.
	if( Raptor::Game->Cfg.SettingAsBool( "precache", true, true ) )
		((XWingGame*)( Raptor::Game ))->Precache();
#endif
	
	Container->Remove();
}


// ---------------------------------------------------------------------------


PrefsMenuDefaultsButton::PrefsMenuDefaultsButton( SDL_Rect *rect, Font *button_font, const char *label ) : LabelledButton( rect, button_font, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


PrefsMenuDefaultsButton::~PrefsMenuDefaultsButton()
{
}


void PrefsMenuDefaultsButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	PrefsMenu *menu = (PrefsMenu*) Container;
	
	if( menu->Page == PrefsMenu::PAGE_VIDEO )
	{
		std::map<std::string,std::string> prev_settings = Raptor::Game->Cfg.Settings;
		Raptor::Game->Cfg.Settings.clear();
		Raptor::Game->SetDefaults();
		
		for( std::map<std::string,std::string>::const_iterator setting = prev_settings.begin(); setting != prev_settings.end(); setting ++ )
		{
			if( (setting->first == "name")
			||  (setting->first == "host_address")
			||  (setting->first == "vr_enable")
			||  (setting->first == "vr_always")
			||  (setting->first == "swap_yaw_roll")
			||  (setting->first == "turret_invert")
			||  Str::BeginsWith( setting->first, "s_" )
			||  Str::BeginsWith( setting->first, "rebel_" )
			||  Str::BeginsWith( setting->first, "empire_" )
			||  Str::BeginsWith( setting->first, "joy_" )
			||  Str::BeginsWith( setting->first, "mouse_" )
			||  Str::BeginsWith( setting->first, "g_res_" ) )
				Raptor::Game->Cfg.Settings[ setting->first ] = setting->second;
		}
		
		if( Raptor::Game->Cfg.SettingAsBool("vr_enable") )
		{
			Raptor::Game->Cfg.Settings[ "g_fullscreen" ] = prev_settings[ "g_fullscreen" ];
			Raptor::Game->Cfg.Settings[ "g_framebuffers" ] = "true";
			Raptor::Game->Cfg.Settings[ "g_vsync" ] = "false";
			Raptor::Game->Cfg.Settings[ "g_asteroid_lod" ] = "0.5";
			Raptor::Game->Cfg.Settings[ "g_deathstar_trench" ] = "3";
			Raptor::Game->Cfg.Settings[ "g_deathstar_surface" ] = "0";
			Raptor::Game->Cfg.Settings[ "g_fsaa" ] = "0";
			
			int blastpoint_quality = Raptor::Game->Cfg.SettingAsInt( "g_shader_blastpoint_quality", 2 );
			if( blastpoint_quality > 2 )
				Raptor::Game->Cfg.Settings[ "g_shader_blastpoint_quality" ] = "2";
			
			int maxfps = Raptor::Game->Cfg.SettingAsInt( "maxfps", 60 );
			if( maxfps && (maxfps < 90) )
				Raptor::Game->Cfg.Settings[ "maxfps" ] = "90";
		}
	}
	else if( menu->Page == PrefsMenu::PAGE_AUDIO )
	{
		std::map<std::string,std::string> prev_settings = Raptor::Game->Cfg.Settings;
		Raptor::Game->Cfg.Settings.clear();
		Raptor::Game->SetDefaults();
		
		for( std::map<std::string,std::string>::const_iterator setting = prev_settings.begin(); setting != prev_settings.end(); setting ++ )
		{
			if( ! Str::BeginsWith( setting->first, "s_" ) )
				Raptor::Game->Cfg.Settings[ setting->first ] = setting->second;
		}
		
		if( Raptor::Game->Res.SearchPath.front() == "Sounds/Silly" )
		{
			Raptor::Game->Res.SearchPath.pop_front();
			Raptor::Game->Snd.StopSounds();
			Raptor::Game->Res.DeleteSounds();
		}
	}
	else if( menu->Page == PrefsMenu::PAGE_CONTROLS )
	{
		Raptor::Game->SetDefaultJoyTypes();
		Raptor::Game->SetDefaultControls();
		
		std::map<std::string,std::string> prev_settings = Raptor::Game->Cfg.Settings;
		Raptor::Game->Cfg.Settings.clear();
		Raptor::Game->SetDefaults();
		std::map<std::string,std::string> defaults = Raptor::Game->Cfg.Settings;
		Raptor::Game->Cfg.Settings = prev_settings;
		
		for( std::map<std::string,std::string>::const_iterator setting = defaults.begin(); setting != defaults.end(); setting ++ )
		{
			if( (setting->first == "swap_yaw_roll")
			||  (setting->first == "turret_invert")
			||  (Str::BeginsWith( setting->first, "joy_" ) && ! Str::BeginsWith( setting->first, "joy_cal_" ))
			||  Str::BeginsWith( setting->first, "mouse_" ) )
				Raptor::Game->Cfg.Settings[ setting->first ] = setting->second;
		}
	}
	else if( menu->Page == PrefsMenu::PAGE_CALIBRATION )
	{
		for( std::map<std::string,std::string>::iterator setting_iter = Raptor::Game->Cfg.Settings.begin(); setting_iter != Raptor::Game->Cfg.Settings.end(); )
		{
			std::map<std::string,std::string>::iterator next_setting = setting_iter;
			next_setting ++;
			
			if( Str::BeginsWith( setting_iter->first, "joy_cal_" ) )
				Raptor::Game->Cfg.Settings.erase( setting_iter );
			
			setting_iter = next_setting;
		}
	}
	
	menu->UpdateContents();
}


// ---------------------------------------------------------------------------


PrefsMenuPageButton::PrefsMenuPageButton( SDL_Rect *rect, Font *button_font, const char *label, int page, bool selected ) : LabelledButton( rect, button_font, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
	
	Page = page;
	
	if( selected )
	{
		Alpha = 0.f;
		AlphaNormal = AlphaOver = AlphaDown = 1.f;
		BlueDown = 1.f;
	}
}


PrefsMenuPageButton::~PrefsMenuPageButton()
{
}


void PrefsMenuPageButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	((PrefsMenu*)( Container ))->ChangePage( Page );
}


// ---------------------------------------------------------------------------


PrefsMenuSillyButton::PrefsMenuSillyButton( SDL_Rect *rect, Font *pew_font ) : Button( rect, NULL )
{
	TimesClicked = 0;
	PewFont = pew_font;
	
	Red = 1.f;
	Green = 1.f;
	Blue = 0.f;
	Alpha = 1.f;
}


PrefsMenuSillyButton::~PrefsMenuSillyButton()
{
}


void PrefsMenuSillyButton::Draw( void )
{
	if( Raptor::Game->Res.SearchPath.front() == "Sounds/Silly" )
	{
		Button::Draw();
		PewFont->DrawText( "PEW PEW", Rect.w/2, Rect.h/2, Font::ALIGN_MIDDLE_CENTER, 0.f,0.f,1.f,1.f );
	}
}


void PrefsMenuSillyButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	else if( Raptor::Game->Res.SearchPath.front() == "Sounds/Silly" )
		TimesClicked = 7;
	else
		TimesClicked ++;
	
	if( TimesClicked == 7 )
	{
		TimesClicked = 6;
		
		Raptor::Game->Cfg.Command( "pew" );
		Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("laser_green.wav") );
	}
}


// ---------------------------------------------------------------------------


PrefsMenuRefreshButton::PrefsMenuRefreshButton( SDL_Rect *rect, Font *button_font, const char *label ) : LabelledButton( rect, button_font, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


PrefsMenuRefreshButton::~PrefsMenuRefreshButton()
{
}


void PrefsMenuRefreshButton::Clicked( Uint8 button )
{
	if( button == SDL_BUTTON_LEFT )
		Raptor::Game->Joy.Refresh();
}


// ---------------------------------------------------------------------------


PrefsMenuBind::PrefsMenuBind( SDL_Rect *rect, uint8_t analog_control, uint8_t inverse, Font *name_font, Font *bind_font ) : Button( rect, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Control = analog_control;
	Inverse = inverse;
	Analog = true;
	Digital = (Control == Inverse);
	
	NameFont = name_font;
	BindFont = bind_font;
	
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


PrefsMenuBind::PrefsMenuBind( SDL_Rect *rect, uint8_t digital_control, Font *name_font, Font *bind_font ) : Button( rect, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Control = digital_control;
	Inverse = 0;
	Analog = false;
	Digital = true;
	
	NameFont = name_font;
	BindFont = bind_font;
	
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


PrefsMenuBind::~PrefsMenuBind()
{
}


void PrefsMenuBind::Draw( void )
{
	std::string control_name = Raptor::Game->Input.ControlName(Control);
	if( Raptor::Game->Cfg.SettingAsBool("swap_yaw_roll") )
	{
		if( Str::BeginsWith( control_name, "Roll" ) )
			control_name = std::string("Yaw") + control_name.substr( 4 );
		else if( Str::BeginsWith( control_name, "Yaw" ) )
			control_name = std::string("Roll") + control_name.substr( 3 );
	}
	if( Analog && ! Digital )
		control_name += std::string(" Axis");
	
	std::string bound_string;
	std::set<std::string> hidden_binds;
	
	std::vector<std::string> bound_to = Raptor::Game->Cfg.ControlBoundTo( Control );
	for( std::vector<std::string>::const_iterator bound_iter = bound_to.begin(); bound_iter != bound_to.end(); bound_iter ++ )
	{
		// Don't display unusual joystick types unless one is connected.
		std::string dev;
		Uint8 unused = 0, unused2 = 0;
		if( (Raptor::Game->Input.ParseJoyAxis( *bound_iter, &dev, &unused ) || Raptor::Game->Input.ParseJoyButton( *bound_iter, &dev, &unused ) || Raptor::Game->Input.ParseJoyHat( *bound_iter, &dev, &unused, &unused2 ))
		&&  Raptor::Game->Cfg.SettingAsBool("joy_hide_binds",true) && ! Raptor::Game->Joy.DeviceTypeFound(dev) )
		{
			hidden_binds.insert( dev );
			continue;
		}
		
		if( ! bound_string.empty() )
			bound_string += std::string(", ");
		bound_string += *bound_iter;
	}
	
	if( Inverse && ! Digital )
	{
		std::vector<std::string> inverted = Raptor::Game->Cfg.ControlBoundTo( Inverse );
		for( std::vector<std::string>::const_iterator bound_iter = inverted.begin(); bound_iter != inverted.end(); bound_iter ++ )
		{
			// Don't display unusual joystick types unless one is connected.
			std::string dev;
			Uint8 unused = 0, unused2 = 0;
			if( (Raptor::Game->Input.ParseJoyAxis( *bound_iter, &dev, &unused ) || Raptor::Game->Input.ParseJoyButton( *bound_iter, &dev, &unused ) || Raptor::Game->Input.ParseJoyHat( *bound_iter, &dev, &unused, &unused2 ))
			&&  Raptor::Game->Cfg.SettingAsBool("joy_hide_binds",true) && ! Raptor::Game->Joy.DeviceTypeFound(dev) )
			{
				hidden_binds.insert( dev );
				continue;
			}
			
			if( ! bound_string.empty() )
				bound_string += std::string(", ");
			bound_string += *bound_iter + std::string("-");
		}
	}
	
	if( bound_string.empty() )
	{
		bound_string = Raptor::Game->Input.ControlName( 0 );  // "Unbound"
		if( hidden_binds.size() )
		{
			bound_string += std::string("* (bound to ");
			for( std::set<std::string>::const_iterator dev_iter = hidden_binds.begin(); dev_iter != hidden_binds.end(); dev_iter ++ )
			{
				if( dev_iter != hidden_binds.begin() )
					bound_string += std::string(", ");
				bound_string += *dev_iter;
			}
			bound_string += std::string(")");
		}
	}
	
	Button::Draw();
	NameFont->DrawText( control_name + std::string(":"), 10, Rect.h/2, Font::ALIGN_MIDDLE_LEFT );
	BindFont->DrawText( bound_string, 120, Rect.h/2, Font::ALIGN_MIDDLE_LEFT );
}


bool PrefsMenuBind::HandleEvent( SDL_Event *event )
{
	if( Container->Selected && (Container->Selected != this) )
		return false;
	
	if( IsSelected() )
	{
		bool handled = false;
		
		if( (event->type == SDL_KEYDOWN) && (event->key.keysym.sym == SDLK_ESCAPE) )
			return true;
		else if( (event->type == SDL_KEYUP) && (event->key.keysym.sym == SDLK_ESCAPE) )
			handled = true;
		else if( Analog && ((event->type == SDL_JOYAXISMOTION) || (event->type == SDL_JOYBALLMOTION)) )
		{
			int8_t dir = Raptor::Game->Cfg.Bind( event, Control );
			handled = dir;
			if( Inverse && (dir < 0) )
				Raptor::Game->Cfg.Bind( event, Inverse );
		}
		else if( Digital && (event->type != SDL_JOYAXISMOTION) && (event->type != SDL_JOYBALLMOTION) )
			handled = Raptor::Game->Cfg.Bind( event, Control );
		
		if( handled )
		{
			Container->Selected = NULL;
			Image.BecomeInstance( ImageNormal );
			Alpha = 0.75f;
			return true;
		}
	}
	else if( ((event->type == SDL_MOUSEBUTTONDOWN) || (event->type == SDL_MOUSEBUTTONUP))
	&&       ((event->button.button == SDL_BUTTON_WHEELUP) || (event->button.button == SDL_BUTTON_WHEELDOWN)) )
		return false;
#if SDL_VERSION_ATLEAST(2,0,0)
	else if( event->type == SDL_MOUSEWHEEL )
		return false;
#endif
	
	return Button::HandleEvent( event );
}


void PrefsMenuBind::Clicked( Uint8 button )
{
	if( button == SDL_BUTTON_RIGHT )
	{
		Raptor::Game->Cfg.Unbind( Control );
		if( Inverse )
			Raptor::Game->Cfg.Unbind( Inverse );
		return;
	}
	else if( button != SDL_BUTTON_LEFT )
		return;
	
	Container->Selected = this;
	Image.BecomeInstance( ImageMouseDown );
	Alpha = 1.f;
}


// ---------------------------------------------------------------------------


PrefsMenuCalibrator::PrefsMenuCalibrator( SDL_Rect *rect ) : Layer( rect )
{
	Name = "Calibrator";
	ReadingDeadzone = false;
	// FIXME: Populate RecentJoyID with connected devices!
}


PrefsMenuCalibrator::~PrefsMenuCalibrator()
{
}


void PrefsMenuCalibrator::Reset( void )
{
	AxisMin.clear();
	AxisMax.clear();
	AxisDeadMin.clear();
	AxisDeadMax.clear();
}


void PrefsMenuCalibrator::Clear( void )
{
	Reset();
	
	if( ! DeviceType.length() )
		return;
	
	std::string setting_prefix = "joy_cal_";
	if( DeviceType == "Joy" )
		setting_prefix += std::string("axis");
	else
	{
		setting_prefix += DeviceType + std::string("_axis");
		std::transform( setting_prefix.begin(), setting_prefix.end(), setting_prefix.begin(), tolower );
		std::replace( setting_prefix.begin(), setting_prefix.end(), ' ', '_' );
	}
	
	for( std::map<std::string,std::string>::iterator setting_iter = Raptor::Game->Cfg.Settings.begin(); setting_iter != Raptor::Game->Cfg.Settings.end(); )
	{
		std::map<std::string,std::string>::iterator next_setting = setting_iter;
		next_setting ++;
		
		if( Str::BeginsWith( setting_iter->first, setting_prefix ) )
			Raptor::Game->Cfg.Settings.erase( setting_iter );
		
		setting_iter = next_setting;
	}
}


void PrefsMenuCalibrator::Apply( void )
{
	if( ! DeviceType.length() )
		return;
	
	std::string setting_prefix = "joy_cal_";
	if( DeviceType == "Joy" )
		setting_prefix += std::string("axis");
	else
	{
		setting_prefix += DeviceType + std::string("_axis");
		std::transform( setting_prefix.begin(), setting_prefix.end(), setting_prefix.begin(), tolower );
		std::replace( setting_prefix.begin(), setting_prefix.end(), ' ', '_' );
	}
	
	for( std::map<Uint8,double>::const_iterator axis_iter = AxisMin.begin(); axis_iter != AxisMin.end(); axis_iter ++ )
	{
		Uint8 axis = axis_iter->first;
		double axis_min = axis_iter->second;
		double axis_max = AxisMax[ axis ];
		double deadzone_min = AxisDeadMin[ axis ];
		double deadzone_max = AxisDeadMax[ axis ];
		Raptor::Game->Cfg.Settings[ setting_prefix + Num::ToString( axis + 1 ) ] = Num::ToString(axis_min) + std::string(",") + Num::ToString(deadzone_min) + std::string(",") + Num::ToString(deadzone_max) + std::string(",") + Num::ToString(axis_max);
	}
	
	if( AxisMin.empty() )
		Clear();
}


void PrefsMenuCalibrator::TrackEvent( SDL_Event *event )
{
	if( event->type == SDL_JOYAXISMOTION )
	{
		Sint32 joy_id = event->jaxis.which;
		std::string device_type = Raptor::Game->Joy.Joysticks[ joy_id ].DeviceType();
		RecentJoyID[ device_type ] = joy_id;
		
		if( device_type != DeviceType )
			return;
		
		Uint8 axis = event->jaxis.axis;
		double value = Raptor::Game->Joy.Axis( joy_id, axis );
		
		if( ReadingDeadzone )
		{
			if( value > 0.9 )
			{
				if( (AxisMax.find(axis) == AxisMax.end()) || (value < AxisMax[ axis ]) )
					AxisMax[ axis ] = value;
				return;
			}
			else if( value < -0.9 )
			{
				if( (AxisMin.find(axis) == AxisMin.end()) || (value > AxisMin[ axis ]) )
					AxisMin[ axis ] = value;
				return;
			}
			
			if( (AxisDeadMin.find(axis) == AxisDeadMin.end()) || (value < AxisDeadMin[ axis ]) )
				AxisDeadMin[ axis ] = value;
			if( (AxisDeadMax.find(axis) == AxisDeadMax.end()) || (value > AxisDeadMax[ axis ]) )
				AxisDeadMax[ axis ] = value;
		}
		
		if( (AxisMin.find(axis) == AxisMin.end()) || (value < AxisMin[ axis ]) )
			AxisMin[ axis ] = value;
		if( (AxisMax.find(axis) == AxisMax.end()) || (value > AxisMax[ axis ]) )
			AxisMax[ axis ] = value;
	}
}


void PrefsMenuCalibrator::Draw( void )
{
	Sint32 joy_id = (RecentJoyID.find(DeviceType) == RecentJoyID.end()) ? -1 : RecentJoyID[ DeviceType ];
	double w = std::min<double>( Rect.h, (Rect.w - 10.) / 2. );
	
	bool range_x = AxisMin.find( 0 ) != AxisMin.end();
	bool range_y = AxisMin.find( 1 ) != AxisMin.end();
	if( range_x || range_y )
	{
		// Draw the full XY motion range.
		double min_x = -1., max_x = 1., min_y = -1., max_y = 1.;
		if( range_x )
		{
			min_x = AxisMin[ 0 ];
			max_x = AxisMax[ 0 ];
		}
		if( range_y )
		{
			min_y = AxisMin[ 1 ];
			max_y = AxisMax[ 1 ];
		}
		double x1 = w * (min_x + 1.) / 2.;
		double y1 = w * (min_y + 1.) / 2.;
		double x2 = w * (max_x + 1.) / 2.;
		double y2 = w * (max_y + 1.) / 2.;
		Raptor::Game->Gfx.DrawRect2D( x1, y1, x2, y2, 0, 0.5f,0.5f,0.5f,0.75f );
		
		bool deadzone_x = AxisDeadMin.find( 0 ) != AxisDeadMin.end();
		bool deadzone_y = AxisDeadMin.find( 1 ) != AxisDeadMin.end();
		if( deadzone_x || deadzone_y )
		{
			// Draw the XY deadzone.
			min_x = max_x = min_y = max_y = 0.;
			if( deadzone_x )
			{
				min_x = AxisDeadMin[ 0 ];
				max_x = AxisDeadMax[ 0 ];
			}
			if( deadzone_y )
			{
				min_y = AxisDeadMin[ 1 ];
				max_y = AxisDeadMax[ 1 ];
			}
			x1 = w * (min_x + 1.) / 2.;
			y1 = w * (min_y + 1.) / 2.;
			x2 = w * (max_x + 1.) / 2.;
			y2 = w * (max_y + 1.) / 2.;
			Raptor::Game->Gfx.DrawRect2D( x1, y1, x2, y2, 0, 1.f,0.f,0.f,0.75f );
		}
	}
	
	// Draw center point.
	Raptor::Game->Gfx.DrawLine2D( w * 0.5 - 3., w * 0.5, w * 0.5 + 3., w * 0.5, 1.f, 0.f,0.f,0.f,0.75f );
	Raptor::Game->Gfx.DrawLine2D( w * 0.5, w * 0.5 - 3., w * 0.5, w * 0.5 + 3., 1.f, 0.f,0.f,0.f,0.75f );
	
	// Draw outline of XY range.
	Raptor::Game->Gfx.DrawLine2D( 0.,0., 0.,w,  1.f, 1.f,1.f,1.f,1.f );
	Raptor::Game->Gfx.DrawLine2D( 0.,w,  w, w,  1.f, 1.f,1.f,1.f,1.f );
	Raptor::Game->Gfx.DrawLine2D( w, w,  w, 0., 1.f, 1.f,1.f,1.f,1.f );
	Raptor::Game->Gfx.DrawLine2D( w, 0., 0.,0., 1.f, 1.f,1.f,1.f,1.f );
	
	if( (joy_id >= 0) && (range_x || range_y) )
	{
		// Draw the current position.
		double curr_x = w * (Raptor::Game->Joy.Axis( joy_id, 0 ) + 1.) / 2.;
		double curr_y = w * (Raptor::Game->Joy.Axis( joy_id, 1 ) + 1.) / 2.;
		Raptor::Game->Gfx.DrawLine2D( curr_x - 5., curr_y, curr_x + 5., curr_y, 1.f, 1.f,1.f,0.f,1.f );
		Raptor::Game->Gfx.DrawLine2D( curr_x, curr_y - 5., curr_x, curr_y + 5., 1.f, 1.f,1.f,0.f,1.f );
	}
	
	// Find all non-XY axes.
	// FIXME: Also search for uncalibrated axes detected.
	std::vector<Uint8> axes;
	for( std::map<Uint8,double>::const_iterator axis_iter = AxisMin.begin(); axis_iter != AxisMin.end(); axis_iter ++ )
	{
		Uint8 axis = axis_iter->first;
		if( axis >= 2 )
			axes.push_back( axis );
	}
	
	double x = Rect.w - w;
	double y = 0.;
	double h = std::max<double>( 10., std::min<double>( (w - 30.) / 7., (Rect.h - (axes.size() - 1) * 5.) / axes.size() ) );
	
	for( std::vector<Uint8>::const_iterator axis_iter = axes.begin(); axis_iter != axes.end(); axis_iter ++ )
	{
		Uint8 axis = *axis_iter;
		
		// Draw the full axis motion range.
		double lx = x + w * (AxisMin[ axis ] + 1.) / 2.;
		double rx = x + w * (AxisMax[ axis ] + 1.) / 2.;
		Raptor::Game->Gfx.DrawRect2D( lx, y, rx, y + h, 0, 0.5f,0.5f,0.5f,0.75f );
		
		if( AxisDeadMin.find( axis ) != AxisDeadMin.end() )
		{
			// Draw the axis deadzone.
			lx = x + w * (AxisDeadMin[ axis ] + 1.) / 2.;
			rx = x + w * (AxisDeadMax[ axis ] + 1.) / 2.;
			Raptor::Game->Gfx.DrawRect2D( lx, y, rx, y + h, 0, 1.f,0.f,0.f,0.75f );
		}
		
		// Draw center point.
		Raptor::Game->Gfx.DrawLine2D( x + w * 0.5, y + h * 0.25, x + w * 0.5, y + h * 0.75, 1.f, 0.f,0.f,0.f,0.75f );
		
		// Draw outline of range.
		Raptor::Game->Gfx.DrawLine2D( x,  y,   x,  y+h,  1.f, 1.f,1.f,1.f,1.f );
		Raptor::Game->Gfx.DrawLine2D( x,  y+h, x+w,y+h,  1.f, 1.f,1.f,1.f,1.f );
		Raptor::Game->Gfx.DrawLine2D( x+w,y+h, x+w,y,    1.f, 1.f,1.f,1.f,1.f );
		Raptor::Game->Gfx.DrawLine2D( x+w,y,   x,  y,    1.f, 1.f,1.f,1.f,1.f );
		
		if( joy_id >= 0 )
		{
			// Draw the current position.
			double curr_x = x + w * (Raptor::Game->Joy.Axis( joy_id, axis ) + 1.) / 2.;
			Raptor::Game->Gfx.DrawLine2D( curr_x, y + 1, curr_x, y + h - 1, 1.f, 1.f,1.f,0.f,1.f );
		}
		
		y += h + 5;
	}
}


// ---------------------------------------------------------------------------


PrefsMenuCalDevDropDown::PrefsMenuCalDevDropDown( SDL_Rect *rect, Font *font, uint8_t align, PrefsMenuCalibrator *calibrator ) : DropDown( rect, font, align, 0, NULL, NULL )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 1.f;
	
	Calibrator = calibrator;
	
	std::set<std::string> device_types;
	for( std::map<Sint32,JoystickState>::const_iterator joy_iter = Raptor::Game->Joy.Joysticks.begin(); joy_iter != Raptor::Game->Joy.Joysticks.end(); joy_iter ++ )
		device_types.insert( joy_iter->second.DeviceType() );
	for( std::set<std::string>::const_iterator dev_iter = device_types.begin(); dev_iter != device_types.end(); dev_iter ++ )
	{
		AddItem( *dev_iter, *dev_iter );
		if( Value.empty() || (*dev_iter == "Joy") )
			Select( *dev_iter );
	}
}


PrefsMenuCalDevDropDown::~PrefsMenuCalDevDropDown()
{
}


void PrefsMenuCalDevDropDown::Changed( void )
{
	Calibrator->DeviceType = Value;
	Calibrator->Reset();
	
	if( Value.empty() )
		return;
	
	std::string setting_prefix = "joy_cal_";
	if( Value == "Joy" )
		setting_prefix += std::string("axis");
	else
	{
		setting_prefix += Value + std::string("_axis");
		std::transform( setting_prefix.begin(), setting_prefix.end(), setting_prefix.begin(), tolower );
		std::replace( setting_prefix.begin(), setting_prefix.end(), ' ', '_' );
	}
	
	for( std::map<std::string,std::string>::const_iterator setting_iter = Raptor::Game->Cfg.Settings.begin(); setting_iter != Raptor::Game->Cfg.Settings.end(); setting_iter ++ )
	{
		if( Str::BeginsWith( setting_iter->first, setting_prefix ) )
		{
			const char *axis_ptr = strstr( setting_iter->first.c_str(), "_axis" );
			Uint8 axis = atoi( axis_ptr + strlen("_axis") );
			if( axis )
			{
				axis --;
				std::vector<double> calibration = Raptor::Game->Cfg.SettingAsDoubles( setting_iter->first );
				if( calibration.size() >= 4 )
				{
					Calibrator->AxisMin[ axis ]     = calibration[ 0 ];
					Calibrator->AxisDeadMin[ axis ] = calibration[ 1 ];
					Calibrator->AxisDeadMax[ axis ] = calibration[ 2 ];
					Calibrator->AxisMax[ axis ]     = calibration[ 3 ];
				}
			}
		}
	}
}


// ---------------------------------------------------------------------------


PrefsMenuCalClearButton::PrefsMenuCalClearButton( SDL_Rect *rect, Font *button_font, const char *label, PrefsMenuCalibrator *calibrator ) : LabelledButton( rect, button_font, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
	
	Calibrator = calibrator;
}


PrefsMenuCalClearButton::~PrefsMenuCalClearButton()
{
}


void PrefsMenuCalClearButton::Clicked( Uint8 button )
{
	if( (button == SDL_BUTTON_LEFT) && Calibrator )
		Calibrator->Clear();
}


// ---------------------------------------------------------------------------


PrefsMenuCalApplyButton::PrefsMenuCalApplyButton( SDL_Rect *rect, Font *button_font, const char *label, PrefsMenuCalibrator *calibrator ) : LabelledButton( rect, button_font, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
	
	Calibrator = calibrator;
}


PrefsMenuCalApplyButton::~PrefsMenuCalApplyButton()
{
}


void PrefsMenuCalApplyButton::Clicked( Uint8 button )
{
	if( (button == SDL_BUTTON_LEFT) && Calibrator )
	{
		Calibrator->Apply();
		PrefsMenuCalDZCheckBox *dz_checkbox = (PrefsMenuCalDZCheckBox*) Raptor::Game->Layers.Find( "DZCheckBox", true );
		if( dz_checkbox )
		{
			Calibrator->ReadingDeadzone = false;
			dz_checkbox->Checked = false;
		}
	}
}


// ---------------------------------------------------------------------------


PrefsMenuCalDZCheckBox::PrefsMenuCalDZCheckBox( SDL_Rect *rect, Font *font, std::string label, PrefsMenuCalibrator *calibrator ) : CheckBox( rect, font, label, false, Raptor::Game->Res.GetAnimation("box_unchecked.ani"), Raptor::Game->Res.GetAnimation("box_unchecked_mdown.ani"), NULL, Raptor::Game->Res.GetAnimation("box_checked.ani"), Raptor::Game->Res.GetAnimation("box_checked_mdown.ani"), NULL )
{
	Name = "DZCheckBox";
	Calibrator = calibrator;
}


PrefsMenuCalDZCheckBox::~PrefsMenuCalDZCheckBox()
{
}


void PrefsMenuCalDZCheckBox::Changed( void )
{
	if( Calibrator )
		Calibrator->ReadingDeadzone = Checked;
}
