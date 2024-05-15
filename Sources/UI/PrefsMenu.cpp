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


PrefsMenu::PrefsMenu( void )
{
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
	BindFont    = Raptor::Game->Res.GetFont( "Verdana.ttf", 10 );
	
	Page = PAGE_PREFERENCES;
	UpdateContents();
	
	WatchSetting( "g_fullscreen" );
#if ! SDL_VERSION_ATLEAST(2,0,0)
	WatchSetting( "g_res_fullscreen_x" );
	WatchSetting( "g_res_fullscreen_y" );
#endif
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
}


PrefsMenu::~PrefsMenu()
{
}


void PrefsMenu::WatchSetting( const std::string &name )
{
	Previous[ name ] = Raptor::Game->Cfg.SettingAsString( name );
}


bool PrefsMenu::WatchedSettingsChanged( void )
{
	// Toggling shaders only requires restart when enabling if any Shader::Load occurred while they were disabled.
	bool shader_enable = Raptor::Game->Cfg.SettingAsBool( "g_shader_enable" );
	if( shader_enable && (Previous[ "g_shader_enable" ] != Raptor::Game->Cfg.SettingAsString( "g_shader_enable" )) && Raptor::Game->Res.ShadersNeedReload() )
	{
		//Raptor::Game->Console.Print( "PrefsMenu::WatchedSettingsChanged: g_shader_enable" );
		return true;
	}
	
	for( std::map<std::string,std::string>::const_iterator prev_iter = Previous.begin(); prev_iter != Previous.end(); prev_iter ++ )
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
			//Raptor::Game->Console.Print( std::string("PrefsMenu::WatchedSettingsChanged: ") + prev_iter->first );
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
	
	rect.x = Rect.w / 2 - 225;
	rect.y = 10;
	rect.w = 250;
	rect.h = 40;
	AddElement( new PrefsMenuPageButton( &rect, TitleFont, "Preferences", PAGE_PREFERENCES, (Page == PAGE_PREFERENCES) ) );
	
	rect.x += rect.w;
	rect.y = 10;
	rect.w = 200;
	rect.h = 40;
	AddElement( new PrefsMenuPageButton( &rect, TitleFont, "Controls", PAGE_CONTROLS, (Page == PAGE_CONTROLS) ) );
	
	if( Page == PrefsMenu::PAGE_PREFERENCES )
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
		
#if SDL_VERSION_ATLEAST(2,0,0)
		rect.w = 110;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Fullscreen", "g_fullscreen" ) );
#else
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
#endif
		rect.x += rect.w + 15;
		rect.w = 90;
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
		rect.w = 165;
		PrefsMenuDropDown *light_quality_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_shader_light_quality" );
		light_quality_dropdown->AddItem( "0", "No Lighting" );
		light_quality_dropdown->AddItem( "1", "Per-Vertex Lighting" );
		light_quality_dropdown->AddItem( "2", "Per-Pixel Lighting" );
		light_quality_dropdown->Update();
		group->AddElement( light_quality_dropdown );
		rect.x += rect.w + 10;
		rect.w = 125;
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
		
		// --------------------------------------------------------------------------------------------------------------------
		// Virtual Reality
		
		group_rect.y += group_rect.h + 5;
		group = new GroupBox( &group_rect, "Virtual Reality", ItemFont );
		AddElement( group );
		
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		rect.w = 160;
		group->AddElement( new PrefsMenuVRCheckBox( &rect, LabelFont, "Enable VR Mode" ) );
		
		rect.x += rect.w + 20;
		rect.w = 70;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Sway", "vr_sway" ) );
		
		rect.y += rect.h + 8;
		rect.x = 10;
		rect.w = 165;
		group->AddElement( new Label( &rect, "FOV / vFOV(-):", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 95;
		group->AddElement( new PrefsMenuTextBox( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, "vr_fov" ) );
		
		rect.y += rect.h + 8;
		rect.x = 10;
		rect.w = 165;
		group->AddElement( new Label( &rect, "Eye Separation (m):", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 95;
		group->AddElement( new PrefsMenuTextBox( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, "vr_separation" ) );
		
		rect.y += rect.h + 8;
		rect.x = 10;
		rect.w = 165;
		group->AddElement( new Label( &rect, "Center Offset (px):", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
		rect.x += rect.w + 5;
		rect.w = 95;
		group->AddElement( new PrefsMenuTextBox( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, "vr_offset" ) );
		
		group->Rect.w = rect.x + rect.w + 10;
		group->Rect.h = rect.y + rect.h + 10;
		
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
		rect.w = 50;
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
		
		group->Rect.h = rect.y + rect.h + 10;
		
		// --------------------------------------------------------------------------------------------------------------------
		// Dark Side
		
		rect.x = group->Rect.x;
		rect.y = group->Rect.y + group->Rect.h + 10;
		rect.w = 100;
		rect.h = ItemFont ? ItemFont->GetHeight() : 18;
		AddElement( new PrefsMenuSecretCheckBox( &rect, LabelFont, "Dark Side" ) );
		
		// --------------------------------------------------------------------------------------------------------------------
		// Sound
		
		group_rect.x += group_rect.w + 10;
		group_rect.y = 50;
		group_rect.w = Rect.w - group_rect.x - 10;
		group = new GroupBox( &group_rect, "Sound", ItemFont );
		AddElement( group );
		rect.x = 10;
		rect.y = 10 + group->TitleFont->GetAscent();
		
		rect.h = ItemFont ? ItemFont->GetHeight() : 18;
		rect.w = 70;
		group->AddElement( new Label( &rect, "Volume:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
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
		rect.w = 70;
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
		rect.w = 70;
		group->AddElement( new Label( &rect, "Engines:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
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
		
		rect.x = 20;
		rect.y += rect.h + 8;
		rect.w = 70;
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
		
		rect.x = 10;
		rect.y += rect.h + 8;
		rect.w = 120;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Menu Music", "s_menu_music" ) );
		
		rect.y += rect.h + 8;
		group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Flight Music", "s_game_music" ) );
		
		rect.y += rect.h + 8;
		rect.w = group_rect.w - 20;
		rect.h = group_rect.h - rect.y - rect.x;
		group->AddElement( new PrefsMenuSillyButton( &rect, TitleFont ) );
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
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::SCORES ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::PAUSE ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::MENU ], ControlFont, BindFont ) );
		rect.y += rect.h + 3;
		scroll_area->AddElement( new PrefsMenuBind( &rect, ((XWingGame*)( Raptor::Game ))->Controls[ XWing::Control::PREFS ], ControlFont, BindFont ) );
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
			Remove();
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
		prefs_menu->Previous[ "g_framebuffers" ] = Raptor::Game->Cfg.Settings[ "g_framebuffers" ] = "true";
		prefs_menu->Previous[ "g_vsync"        ] = Raptor::Game->Cfg.Settings[ "g_vsync"        ] = "false";
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
	
	// Restart graphics/shaders if necessary.
	PrefsMenu *menu = (PrefsMenu*) Container;
	if( menu->WatchedSettingsChanged() )
		Raptor::Game->Gfx.Restart();
	
	// Restart VR if it's checked but not working.
	if( (! Raptor::Game->Head.VR) && Raptor::Game->Cfg.SettingAsBool("vr_enable") )
		Raptor::Game->Head.RestartVR();
	
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
				Raptor::Game->Snd.PlayMusicSubdir( "Menu" );
			else
				Raptor::Game->Snd.StopMusic();
		}
	}
	
	// Make sure all resources that need to be loaded for these settings have been loaded.
	((XWingGame*)( Raptor::Game ))->Precache();
	
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
	
	if( menu->Page == PrefsMenu::PAGE_PREFERENCES )
	{
		std::map<std::string,std::string> prev_settings = Raptor::Game->Cfg.Settings;
		Raptor::Game->Cfg.Settings.clear();
		Raptor::Game->SetDefaults();
		
		for( std::map<std::string,std::string>::const_iterator setting = prev_settings.begin(); setting != prev_settings.end(); setting ++ )
		{
			if( (setting->first == "name")
			||  (setting->first == "host_address")
			||  (setting->first == "rebel_mission")
			||  (setting->first == "empire_mission")
			||  (setting->first == "vr_enable")
			||  (setting->first == "swap_yaw_roll")
			||  (setting->first == "turret_invert")
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
			Raptor::Game->Cfg.Settings[ "g_deathstar_surface" ] = "2";
			
			int fsaa = Raptor::Game->Cfg.SettingAsInt( "g_fsaa", 4 );
			if( fsaa > 2 )
				Raptor::Game->Cfg.Settings[ "g_fsaa" ] = "2";
			
			int blastpoint_quality = Raptor::Game->Cfg.SettingAsInt( "g_shader_blastpoint_quality", 2 );
			if( blastpoint_quality > 2 )
				Raptor::Game->Cfg.Settings[ "g_shader_blastpoint_quality" ] = "2";
			
			int maxfps = Raptor::Game->Cfg.SettingAsInt( "maxfps", 60 );
			if( maxfps && (maxfps < 90) )
				Raptor::Game->Cfg.Settings[ "maxfps" ] = "90";
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
			||  Str::BeginsWith( setting->first, "joy_" )
			||  Str::BeginsWith( setting->first, "mouse_" ) )
				Raptor::Game->Cfg.Settings[ setting->first ] = setting->second;
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
	Green = 0.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


PrefsMenuSillyButton::~PrefsMenuSillyButton()
{
}


void PrefsMenuSillyButton::Draw( void )
{
	if( Raptor::Game->Res.SearchPath.front() == "Sounds/Silly" )
	{
		Button::Draw();
		PewFont->DrawText( "PEW PEW", Rect.w/2, Rect.h/2, Font::ALIGN_MIDDLE_CENTER );
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
