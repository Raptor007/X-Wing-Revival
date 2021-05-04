/*
 *  PrefsMenu.cpp
 */

#include "PrefsMenu.h"

#include "Label.h"
#include "GroupBox.h"
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
	
	TitleFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 30 );
	LabelFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 16 );
	ItemFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 17 );
	ButtonFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 30 );
	
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
	WatchSetting( "g_shader_light_quality" );
	WatchSetting( "g_shader_point_lights" );
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
	// Restart graphics for turning shaders on, but not for turning them off.
	bool shader_enable = Raptor::Game->Cfg.SettingAsBool( "g_shader_enable" );
	if( shader_enable && (Previous[ "g_shader_enable" ] != Raptor::Game->Cfg.SettingAsString( "g_shader_enable" )) )
		return true;
	
	for( std::map<std::string,std::string>::const_iterator prev_iter = Previous.begin(); prev_iter != Previous.end(); prev_iter ++ )
	{
		// Don't restart graphics for changes to shader variables if shaders are disabled.
		if( (! shader_enable) && (strncmp( prev_iter->first.c_str(), "g_shader_", 9 ) == 0) )
			continue;
		
		// Restart if any other watched variable was changed.
		if( prev_iter->second != Raptor::Game->Cfg.SettingAsString( prev_iter->first ) )
			return true;
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
	
	// --------------------------------------------------------------------------------------------------------------------
	// Graphics
	
	group_rect.x = 10;
	group_rect.y = 28;
	group_rect.w = 400;
	group_rect.h = 269;
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
	rect.x += rect.w + 15;
	rect.w = 85;
	PrefsMenuDropDown *fsaa_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_fsaa" );
	fsaa_dropdown->AddItem( "0", "No AA" );
	fsaa_dropdown->AddItem( "2", "2xFSAA" );
	fsaa_dropdown->AddItem( "4", "4xFSAA" );
	fsaa_dropdown->AddItem( "8", "8xFSAA" );
	fsaa_dropdown->AddItem( "16", "16xFSAA" );
	fsaa_dropdown->Update();
	group->AddElement( fsaa_dropdown );
	
	rect.y += rect.h + 8;
	rect.x = 10;
	rect.w = 285;
	PrefsMenuCheckBox *vsync_checkbox = new PrefsMenuCheckBox( &rect, LabelFont, "Vertical Synchronization (VSync)", "g_vsync" );
	group->AddElement( vsync_checkbox );
	
	rect.y += rect.h + 8;
	rect.w = 145;
	group->AddElement( new Label( &rect, "Texture Quality:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 85;
	PrefsMenuDropDown *texture_res_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_texture_maxres" );
	texture_res_dropdown->AddItem( "128", "Low" );
	texture_res_dropdown->AddItem( "256", "Medium" );
	texture_res_dropdown->AddItem( "0", "High" );
	texture_res_dropdown->Update();
	group->AddElement( texture_res_dropdown );
	rect.x += rect.w + 10;
	rect.w = 85;
	PrefsMenuFilterDropDown *af_dropdown = new PrefsMenuFilterDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0 );
	af_dropdown->AddItem( "1", "Trilinear" );
	af_dropdown->AddItem( "2", "2xAF" );
	af_dropdown->AddItem( "4", "4xAF" );
	af_dropdown->AddItem( "8", "8xAF" );
	af_dropdown->AddItem( "16", "16xAF" );
	af_dropdown->AddItem( "-1", "Linear" );
	af_dropdown->Update();
	group->AddElement( af_dropdown );
	
	rect.x = 10;
	rect.y += rect.h + 8;
	rect.w = 345;
	PrefsMenuCheckBox *framebuffers_checkbox = new PrefsMenuCheckBox( &rect, LabelFont, "Framebuffer Textures (Required for VR)", "g_framebuffers" );
	group->AddElement( framebuffers_checkbox );
	
	rect.x = 10;
	rect.y += rect.h + 8;
	rect.w = 185;
	group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Draw With Shaders", "g_shader_enable" ) );
	
	rect.x = 10;
	rect.y += rect.h + 8;
	rect.w = 115;
	group->AddElement( new Label( &rect, "Light Quality:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 125;
	PrefsMenuDropDown *light_quality_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_shader_light_quality" );
	light_quality_dropdown->AddItem( "0", "Off" );
	light_quality_dropdown->AddItem( "1", "Low (Vertex)" );
	light_quality_dropdown->AddItem( "2", "High (Pixel)" );
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
	rect.w = 240;
	group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Death Star Trench Details", "g_deathstar_detail", "3", "0" ) );
	
	rect.x = 10;
	rect.y += rect.h + 8;
	rect.w = 305;
	group->AddElement( new PrefsMenuVRCheckBox( &rect, LabelFont, "Virtual Reality (OpenVR/SteamVR)", framebuffers_checkbox, vsync_checkbox, fsaa_dropdown ) );
	
	// --------------------------------------------------------------------------------------------------------------------
	// Sound
	
	group_rect.x += group_rect.w + 10;
	group_rect.w = Rect.w - group_rect.x - 10;
	group_rect.h = 237;
	group = new GroupBox( &group_rect, "Sound", ItemFont );
	AddElement( group );
	rect.x = 10;
	rect.y = 10 + group->TitleFont->GetAscent();
	
	rect.h = ItemFont ? ItemFont->GetHeight() : 18;
	rect.w = 70;
	group->AddElement( new Label( &rect, "Volume:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 100;
	PrefsMenuDropDown *s_volume_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_volume" );
	s_volume_dropdown->AddItem( "0", "Off" );
	s_volume_dropdown->AddItem( "0.075", "Very Quiet" );
	s_volume_dropdown->AddItem( "0.125", "Quiet" );
	s_volume_dropdown->AddItem( "0.25", "Medium" );
	s_volume_dropdown->AddItem( "0.5", "Loud" );
	s_volume_dropdown->AddItem( "0.75", "Louder" );
	s_volume_dropdown->AddItem( "1", "Loudest" );
	s_volume_dropdown->Update();
	group->AddElement( s_volume_dropdown );
	
	rect.y += rect.h + 8;
	rect.x = 10;
	rect.w = 70;
	group->AddElement( new Label( &rect, "Effects:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 100;
	PrefsMenuDropDown *s_effect_volume_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_effect_volume" );
	s_effect_volume_dropdown->AddItem( "0", "Off" );
	s_effect_volume_dropdown->AddItem( "0.125", "Very Quiet" );
	s_effect_volume_dropdown->AddItem( "0.25", "Quiet" );
	s_effect_volume_dropdown->AddItem( "0.5", "Medium" );
	s_effect_volume_dropdown->AddItem( "0.75", "Loud" );
	s_effect_volume_dropdown->AddItem( "1", "Loudest" );
	s_effect_volume_dropdown->Update();
	group->AddElement( s_effect_volume_dropdown );
	
	rect.x = 10;
	rect.y += rect.h + 8;
	rect.w = 70;
	group->AddElement( new Label( &rect, "Music:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 100;
	PrefsMenuDropDown *s_music_volume_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "s_music_volume" );
	s_music_volume_dropdown->AddItem( "0", "Off" );
	s_music_volume_dropdown->AddItem( "0.125", "Very Quiet" );
	s_music_volume_dropdown->AddItem( "0.25", "Quiet" );
	s_music_volume_dropdown->AddItem( "0.5", "Medium" );
	s_music_volume_dropdown->AddItem( "0.75", "Loud" );
	s_music_volume_dropdown->AddItem( "1", "Loudest" );
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
	group->AddElement( new PrefsMenuSillyButton( &rect ) );
	
	rect.h = s_music_volume_dropdown->Rect.h;
	
	// --------------------------------------------------------------------------------------------------------------------
	// Joystick
	
	group_rect.x = 10;
	group_rect.y = 301;
	group_rect.w = 195;
	group_rect.h = 168;
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
	rect.x = 10;
	rect.w = 150;
	group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Swap Yaw/Roll", "joy_swap_xz" ) );
	
	rect.y += rect.h + 8;
	rect.x = 20;
	rect.w = 20;
	group->AddElement( new Label( &rect, "X:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 105;
	PrefsMenuDropDown *joy_smooth_x_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_x" );
	joy_smooth_x_dropdown->AddItem( "0", "Linear" );
	joy_smooth_x_dropdown->AddItem( "0.125", "Twitchy" );
	joy_smooth_x_dropdown->AddItem( "0.25", "Responsive" );
	joy_smooth_x_dropdown->AddItem( "0.5", "Medium" );
	joy_smooth_x_dropdown->AddItem( "0.75", "Smooth" );
	joy_smooth_x_dropdown->AddItem( "1", "Smoothest" );
	joy_smooth_x_dropdown->Update();
	group->AddElement( joy_smooth_x_dropdown );
	
	rect.y += rect.h + 3;
	rect.x = 20;
	rect.w = 20;
	group->AddElement( new Label( &rect, "Y:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 105;
	PrefsMenuDropDown *joy_smooth_y_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_y" );
	joy_smooth_y_dropdown->AddItem( "0", "Linear" );
	joy_smooth_y_dropdown->AddItem( "0.125", "Twitchy" );
	joy_smooth_y_dropdown->AddItem( "0.25", "Responsive" );
	joy_smooth_y_dropdown->AddItem( "0.5", "Medium" );
	joy_smooth_y_dropdown->AddItem( "0.75", "Smooth" );
	joy_smooth_y_dropdown->AddItem( "1", "Smoothest" );
	joy_smooth_y_dropdown->Update();
	group->AddElement( joy_smooth_y_dropdown );
	
	rect.y += rect.h + 3;
	rect.x = 20;
	rect.w = 55;
	group->AddElement( new Label( &rect, "Twist:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 105;
	PrefsMenuDropDown *joy_smooth_z_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_z" );
	joy_smooth_z_dropdown->AddItem( "0", "Linear" );
	joy_smooth_z_dropdown->AddItem( "0.125", "Twitchy" );
	joy_smooth_z_dropdown->AddItem( "0.25", "Responsive" );
	joy_smooth_z_dropdown->AddItem( "0.5", "Medium" );
	joy_smooth_z_dropdown->AddItem( "0.75", "Smooth" );
	joy_smooth_z_dropdown->AddItem( "1", "Smoothest" );
	joy_smooth_z_dropdown->Update();
	group->AddElement( joy_smooth_z_dropdown );
	
	// --------------------------------------------------------------------------------------------------------------------
	// Controller
	
	group_rect.x += group_rect.w + 10;
	group_rect.w = 195;
	group_rect.h = 90;
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
	rect.w = 65;
	group->AddElement( new Label( &rect, "Sticks:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 105;
	PrefsMenuDropDown *joy_smooth_thumbsticks_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_thumbsticks" );
	joy_smooth_thumbsticks_dropdown->AddItem( "0", "Linear" );
	joy_smooth_thumbsticks_dropdown->AddItem( "0.125", "Twitchy" );
	joy_smooth_thumbsticks_dropdown->AddItem( "0.25", "Responsive" );
	joy_smooth_thumbsticks_dropdown->AddItem( "0.5", "Medium" );
	joy_smooth_thumbsticks_dropdown->AddItem( "0.75", "Smooth" );
	joy_smooth_thumbsticks_dropdown->AddItem( "1", "Smoothest" );
	joy_smooth_thumbsticks_dropdown->Update();
	group->AddElement( joy_smooth_thumbsticks_dropdown );
	
	// --------------------------------------------------------------------------------------------------------------------
	// Mouse
	
	group_rect.x += group_rect.w + 10;
	group_rect.w = Rect.w - group_rect.x - 10;
	group_rect.y -= 120 - group_rect.h;
	group_rect.h = 120;
	group = new GroupBox( &group_rect, "Mouse", ItemFont );
	AddElement( group );
	rect.x = 10;
	rect.y = 10 + group->TitleFont->GetAscent();
	
	rect.w = 55;
	group->AddElement( new Label( &rect, "Mode:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 110;
	PrefsMenuDropDown *mouse_mode_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "mouse_mode" );
	mouse_mode_dropdown->AddItem( "disabled", "Disabled" );
	mouse_mode_dropdown->AddItem( "fly", "Yaw/Pitch" );
	mouse_mode_dropdown->AddItem( "fly2", "Roll/Pitch" );
	mouse_mode_dropdown->AddItem( "look", "Freelook" );
	mouse_mode_dropdown->Update();
	group->AddElement( mouse_mode_dropdown );
	
	rect.y += rect.h + 8;
	rect.x = 10;
	rect.w = 80;
	group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Invert", "mouse_invert" ) );
	
	rect.y += rect.h + 8;
	rect.x = 10;
	rect.w = 65;
	group->AddElement( new Label( &rect, "Input:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 105;
	PrefsMenuDropDown *mouse_smooth_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "mouse_smooth" );
	mouse_smooth_dropdown->AddItem( "0", "Linear" );
	mouse_smooth_dropdown->AddItem( "0.125", "Twitchy" );
	mouse_smooth_dropdown->AddItem( "0.25", "Responsive" );
	mouse_smooth_dropdown->AddItem( "0.5", "Medium" );
	mouse_smooth_dropdown->AddItem( "0.75", "Smooth" );
	mouse_smooth_dropdown->AddItem( "1", "Smoothest" );
	mouse_smooth_dropdown->Update();
	group->AddElement( mouse_smooth_dropdown );
}


void PrefsMenu::Draw( void )
{
	// Keep the size and position up-to-date.
	Rect.x = Raptor::Game->Gfx.W/2 - 640/2;
	Rect.y = Raptor::Game->Gfx.H/2 - 480/2;
	Rect.w = 640;
	Rect.h = 480;
	
	Window::Draw();
	TitleFont->DrawText( "Preferences", Rect.w/2 + 2, 3, Font::ALIGN_TOP_CENTER, 0,0,0,0.8f );
	TitleFont->DrawText( "Preferences", Rect.w/2,     1, Font::ALIGN_TOP_CENTER );
}


bool PrefsMenu::KeyUp( SDLKey key )
{
	if( key == SDLK_ESCAPE )
	{
		Remove();
		return true;
	}
	
	return false;
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


PrefsMenuVRCheckBox::PrefsMenuVRCheckBox( SDL_Rect *rect, Font *font, std::string label, PrefsMenuCheckBox *framebuffers_checkbox, PrefsMenuCheckBox *vsync_checkbox, PrefsMenuDropDown *fsaa_dropdown ) : PrefsMenuCheckBox( rect, font, label, "vr_enable" )
{
	FramebuffersCheckbox = framebuffers_checkbox;
	VSyncCheckbox = vsync_checkbox;
	FSAADropDown = fsaa_dropdown;
}


PrefsMenuVRCheckBox::~PrefsMenuVRCheckBox()
{
}


void PrefsMenuVRCheckBox::Changed( void )
{
	PrefsMenuCheckBox::Changed();
	
	int maxfps = Raptor::Game->Cfg.SettingAsInt( "maxfps", 60 );
	
	if( Checked )
	{
		Raptor::Game->Cfg.Settings[ "g_framebuffers" ] = "true";
		Raptor::Game->Cfg.Settings[ "g_zbits" ] = "32";
		Raptor::Game->Cfg.Settings[ "g_fsaa" ] = "0";
		Raptor::Game->Cfg.Settings[ "g_vsync" ] = "false";
		
		if( maxfps && (maxfps < 90) )
			Raptor::Game->Cfg.Settings[ "maxfps" ] = "90";
		
		if( FramebuffersCheckbox )
		{
			FramebuffersCheckbox->Checked = true;
			FramebuffersCheckbox->Image.BecomeInstance( FramebuffersCheckbox->ImageNormalChecked );
		}
		if( VSyncCheckbox )
		{
			VSyncCheckbox->Checked = false;
			VSyncCheckbox->Image.BecomeInstance( VSyncCheckbox->ImageNormal );
		}
		if( FSAADropDown )
		{
			FSAADropDown->Value = FSAADropDown->Items[0].Value;
			FSAADropDown->LabelText = FSAADropDown->Items[0].Text;
		}
	}
	else
	{
		Raptor::Game->Cfg.Settings[ "g_zbits" ] = "24";
		
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


void PrefsMenuTextBox::Changed( void )
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
	
	std::string old_name = Raptor::Game->Cfg.SettingAsString("name");
	
	Raptor::Game->Cfg.Settings.clear();
	Raptor::Game->SetDefaults();
	
	if( old_name.size() )
		Raptor::Game->Cfg.Settings["name"] = old_name;
	
	((PrefsMenu*)( Container ))->UpdateContents();
	
	if( Raptor::Game->Res.SearchPath.front() == "Sounds/Silly" )
	{
		Raptor::Game->Res.SearchPath.pop_front();
		Raptor::Game->Snd.StopSounds();
		Raptor::Game->Res.DeleteSounds();
	}
}


// ---------------------------------------------------------------------------


PrefsMenuSillyButton::PrefsMenuSillyButton( SDL_Rect *rect ) : Button( rect, NULL )
{
	TimesClicked = 0;
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
		PrefsMenu *prefs = (PrefsMenu*) Container->Container;
		prefs->TitleFont->DrawText( "PEW PEW", Rect.w/2, Rect.h/2, Font::ALIGN_MIDDLE_CENTER );
	}
}


void PrefsMenuSillyButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	else if( Raptor::Game->Res.SearchPath.front() == "Sounds/Silly" )
		TimesClicked = 7;
	else if( !( Raptor::Game->Keys.KeyDown(SDLK_RSHIFT) || Raptor::Game->Keys.KeyDown(SDLK_LSHIFT) ) )
		return;
	else
		TimesClicked ++;
	
	if( TimesClicked == 7 )
	{
		TimesClicked = 0;
		
		Raptor::Game->HandleCommand( "pew" );
		
		Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("laser_green.wav") );
	}
}
