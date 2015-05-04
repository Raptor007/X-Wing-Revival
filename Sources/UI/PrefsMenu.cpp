/*
 *  PrefsMenu.cpp
 */

#include "PrefsMenu.h"

#include "Label.h"
#include "GroupBox.h"
#include "RaptorGame.h"
#include "XWingDefs.h"
#include "XWingGame.h"


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
	
	PrevFullscreen = Raptor::Game->Cfg.SettingAsBool("g_fullscreen");
	PrevFullscreenX = Raptor::Game->Cfg.SettingAsInt("g_res_fullscreen_x");
	PrevFullscreenY = Raptor::Game->Cfg.SettingAsInt("g_res_fullscreen_y");
	PrevFSAA = Raptor::Game->Cfg.SettingAsInt("g_fsaa");
	PrevAF = Raptor::Game->Cfg.SettingAsInt("g_af");
	PrevSoundDir = Raptor::Game->Cfg.SettingAsString("res_sound_dir");
	PrevMusicDir = Raptor::Game->Cfg.SettingAsString("res_music_dir");
}


PrefsMenu::~PrefsMenu()
{
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
	group_rect.y = 55;
	group_rect.w = 305;
	group_rect.h = 180;
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
	
	rect.y += rect.h + 8;
	rect.x = 10;
	rect.w = 80;
	group->AddElement( new Label( &rect, "Filtering:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 85;
	PrefsMenuDropDown *fsaa_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_fsaa" );
	fsaa_dropdown->AddItem( "0", "No AA" );
	fsaa_dropdown->AddItem( "2", "2xFSAA" );
	fsaa_dropdown->AddItem( "4", "4xFSAA" );
	fsaa_dropdown->AddItem( "8", "8xFSAA" );
	fsaa_dropdown->AddItem( "16", "16xFSAA" );
	fsaa_dropdown->Update();
	group->AddElement( fsaa_dropdown );
	rect.x += rect.w + 5;
	rect.w = 85;
	PrefsMenuDropDown *af_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_af" );
	af_dropdown->AddItem( "1", "No AF" );
	af_dropdown->AddItem( "2", "2xAF" );
	af_dropdown->AddItem( "4", "4xAF" );
	af_dropdown->AddItem( "8", "8xAF" );
	af_dropdown->AddItem( "16", "16xAF" );
	af_dropdown->Update();
	group->AddElement( af_dropdown );
	
	rect.y += rect.h + 8;
	rect.x = 10;
	rect.w = group_rect.w - 20;
	group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Draw With Shaders", "g_shader_enable" ) );
	
	rect.y += rect.h + 8;
	rect.w = 225;
	group->AddElement( new Label( &rect, "Dynamic Lights Per Object:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 50;
	PrefsMenuDropDown *dynamic_lights_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "g_dynamic_lights" );
	dynamic_lights_dropdown->AddItem( "0", "Off" );
	dynamic_lights_dropdown->AddItem( "1", "1" );
	dynamic_lights_dropdown->AddItem( "2", "2" );
	dynamic_lights_dropdown->AddItem( "3", "3" );
	dynamic_lights_dropdown->AddItem( "4", "4" );
	dynamic_lights_dropdown->Update();
	group->AddElement( dynamic_lights_dropdown );
	
	rect.y += rect.h + 8;
	rect.x = 10;
	rect.w = 120;
	group->AddElement( new Label( &rect, "High Quality:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w;
	rect.w = 80;
	group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Ships", "g_hq_ships" ) );
	rect.x += rect.w;
	rect.w = 90;
	group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Cockpit", "g_hq_cockpit" ) );
	
	// --------------------------------------------------------------------------------------------------------------------
	// Sound
	
	group_rect.x += group_rect.w + 10;
	group_rect.w = Rect.w - group_rect.x - 10;
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
	s_volume_dropdown->AddItem( "0.125", "Very Quiet" );
	s_volume_dropdown->AddItem( "0.25", "Quiet" );
	s_volume_dropdown->AddItem( "0.5", "Medium" );
	s_volume_dropdown->AddItem( "0.75", "Loud" );
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
	rect.w = group_rect.w - 20;
	group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Menu Music", "s_menu_music" ) );
	
	rect.y += rect.h + 8;
	group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Flight Music", "s_game_music" ) );
	
	// --------------------------------------------------------------------------------------------------------------------
	// Joystick
	
	group_rect.x = 10;
	group_rect.y += group_rect.h + 10;
	group_rect.w = 195;
	group_rect.h = 195;
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
	group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Swap Yaw/Roll", "joy_swap_xz" ) );
	
	rect.y += rect.h + 8;
	rect.w = 80;
	group->AddElement( new Label( &rect, "Smooth:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	
	rect.y += rect.h + 3;
	rect.x = 20;
	rect.w = 20;
	group->AddElement( new Label( &rect, "X:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 95;
	PrefsMenuDropDown *joy_smooth_x_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_x" );
	joy_smooth_x_dropdown->AddItem( "0", "Off" );
	joy_smooth_x_dropdown->AddItem( "0.125", "Very Low" );
	joy_smooth_x_dropdown->AddItem( "0.25", "Low" );
	joy_smooth_x_dropdown->AddItem( "0.5", "Medium" );
	joy_smooth_x_dropdown->AddItem( "0.75", "High" );
	joy_smooth_x_dropdown->AddItem( "1", "Very High" );
	joy_smooth_x_dropdown->Update();
	group->AddElement( joy_smooth_x_dropdown );
	
	rect.y += rect.h + 3;
	rect.x = 20;
	rect.w = 20;
	group->AddElement( new Label( &rect, "Y:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 95;
	PrefsMenuDropDown *joy_smooth_y_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_y" );
	joy_smooth_y_dropdown->AddItem( "0", "Off" );
	joy_smooth_y_dropdown->AddItem( "0.125", "Very Low" );
	joy_smooth_y_dropdown->AddItem( "0.25", "Low" );
	joy_smooth_y_dropdown->AddItem( "0.5", "Medium" );
	joy_smooth_y_dropdown->AddItem( "0.75", "High" );
	joy_smooth_y_dropdown->AddItem( "1", "Very High" );
	joy_smooth_y_dropdown->Update();
	group->AddElement( joy_smooth_y_dropdown );
	
	rect.y += rect.h + 3;
	rect.x = 20;
	rect.w = 55;
	group->AddElement( new Label( &rect, "Twist:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 95;
	PrefsMenuDropDown *joy_smooth_z_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_z" );
	joy_smooth_z_dropdown->AddItem( "0", "Off" );
	joy_smooth_z_dropdown->AddItem( "0.125", "Very Low" );
	joy_smooth_z_dropdown->AddItem( "0.25", "Low" );
	joy_smooth_z_dropdown->AddItem( "0.5", "Medium" );
	joy_smooth_z_dropdown->AddItem( "0.75", "High" );
	joy_smooth_z_dropdown->AddItem( "1", "Very High" );
	joy_smooth_z_dropdown->Update();
	group->AddElement( joy_smooth_z_dropdown );
	
	// --------------------------------------------------------------------------------------------------------------------
	// Controller
	
	group_rect.x += group_rect.w + 10;
	group_rect.w = 195;
	group_rect.h = 120;
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
	group->AddElement( new Label( &rect, "Smooth:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 95;
	PrefsMenuDropDown *joy_smooth_thumbsticks_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "joy_smooth_thumbsticks" );
	joy_smooth_thumbsticks_dropdown->AddItem( "0", "Off" );
	joy_smooth_thumbsticks_dropdown->AddItem( "0.125", "Very Low" );
	joy_smooth_thumbsticks_dropdown->AddItem( "0.25", "Low" );
	joy_smooth_thumbsticks_dropdown->AddItem( "0.5", "Medium" );
	joy_smooth_thumbsticks_dropdown->AddItem( "0.75", "High" );
	joy_smooth_thumbsticks_dropdown->AddItem( "1", "Very High" );
	joy_smooth_thumbsticks_dropdown->Update();
	group->AddElement( joy_smooth_thumbsticks_dropdown );
	
	// --------------------------------------------------------------------------------------------------------------------
	// Mouse
	
	group_rect.x += group_rect.w + 10;
	group_rect.w = Rect.w - group_rect.x - 10;
	group = new GroupBox( &group_rect, "Mouse", ItemFont );
	AddElement( group );
	rect.x = 10;
	rect.y = 10 + group->TitleFont->GetAscent();
	
	rect.w = 185;
	PrefsMenuDropDown *mouse_enable_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "mouse_enable" );
	mouse_enable_dropdown->AddItem( "false", "Never Fly By Mouse" );
	mouse_enable_dropdown->AddItem( "fullscreen", "Fullscreen Only" );
	mouse_enable_dropdown->AddItem( "true", "Anytime No Joystick" );
	mouse_enable_dropdown->Update();
	group->AddElement( mouse_enable_dropdown );
	
	rect.y += rect.h + 8;
	rect.x = 10;
	rect.w = group_rect.w - 20;
	group->AddElement( new PrefsMenuCheckBox( &rect, LabelFont, "Invert", "mouse_invert" ) );
	
	rect.y += rect.h + 8;
	rect.x = 10;
	rect.w = 70;
	group->AddElement( new Label( &rect, "Smooth:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 95;
	PrefsMenuDropDown *mouse_smooth_dropdown = new PrefsMenuDropDown( &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, 0, "mouse_smooth" );
	mouse_smooth_dropdown->AddItem( "0", "Off" );
	mouse_smooth_dropdown->AddItem( "0.125", "Very Low" );
	mouse_smooth_dropdown->AddItem( "0.25", "Low" );
	mouse_smooth_dropdown->AddItem( "0.5", "Medium" );
	mouse_smooth_dropdown->AddItem( "0.75", "High" );
	mouse_smooth_dropdown->AddItem( "1", "Very High" );
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
	TitleFont->DrawText( "Preferences", Rect.w/2 + 2, 12, Font::ALIGN_TOP_CENTER, 0,0,0,0.8f );
	TitleFont->DrawText( "Preferences", Rect.w/2, 10, Font::ALIGN_TOP_CENTER );
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
	if( (menu->PrevFullscreen != Raptor::Game->Cfg.SettingAsBool("g_fullscreen"))
	 || (menu->PrevFullscreen && (menu->PrevFullscreenX != Raptor::Game->Cfg.SettingAsInt("g_res_fullscreen_x")))
	 || (menu->PrevFullscreen && (menu->PrevFullscreenY != Raptor::Game->Cfg.SettingAsInt("g_res_fullscreen_y")))
	 || (menu->PrevFSAA != Raptor::Game->Cfg.SettingAsInt("g_fsaa"))
	 || (menu->PrevAF != Raptor::Game->Cfg.SettingAsInt("g_af")) )
		Raptor::Game->Gfx.Restart();
	
	// If we changed directories, clear old resources.
	if( menu->PrevSoundDir != Raptor::Game->Cfg.SettingAsString("res_sound_dir") )
	{
		Raptor::Game->Snd.StopSounds();
		Raptor::Game->Res.DeleteSounds();
	}
	if( menu->PrevMusicDir != Raptor::Game->Cfg.SettingAsString("res_music_dir") )
	{
		Raptor::Game->Snd.StopMusic();
		Raptor::Game->Res.DeleteMusic();
	}
	
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
}
