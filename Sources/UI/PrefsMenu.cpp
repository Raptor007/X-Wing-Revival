/*
 *  PrefsMenu.cpp
 */

#include "PrefsMenu.h"

#include "Label.h"
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
	
	TitleFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 30 );
	ItemFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 18 );
	ButtonFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 32 );
	
	UpdateContents();
	
	PrevFullscreen = Raptor::Game->Cfg.SettingAsBool("g_fullscreen");
	PrevFullscreenX = Raptor::Game->Cfg.SettingAsInt("g_res_fullscreen_x");
	PrevFullscreenY = Raptor::Game->Cfg.SettingAsInt("g_res_fullscreen_y");
	PrevFSAA = Raptor::Game->Cfg.SettingAsInt("g_fsaa");
	PrevAF = Raptor::Game->Cfg.SettingAsInt("g_af");
	PrevShaderFile = Raptor::Game->Cfg.SettingAsString("g_shader_file");
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
	
	for( int i = Elements.size() - 1; i >= 0; i -- )
	{
		try
		{
			Layer *element = Elements.at( i );
			delete element;
			Elements[ i ] = NULL;
		}
		catch( std::out_of_range &exception )
		{
			fprintf( stderr, "Layer::~Layer: std::out_of_range\n" );
		}
	}
	
	Elements.clear();
	
	
	// Add new elements.
	
	SDL_Rect rect;
	
	rect.h = ItemFont ? ItemFont->GetHeight() : 18;
	rect.x = 10;
	rect.y = 60;
	rect.w = 105;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "Fullscreen:", "g_fullscreen" ) );
	rect.x += rect.w + 5;
	rect.w = 60;
	AddElement( new PrefsMenuTextBox( this, &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, "g_res_fullscreen_x" ) );
	rect.x += rect.w + 5;
	rect.w = 15;
	AddElement( new Label( this, &rect, "x", ItemFont, Font::ALIGN_MIDDLE_CENTER ) );
	rect.x += rect.w + 5;
	rect.w = 60;
	AddElement( new PrefsMenuTextBox( this, &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, "g_res_fullscreen_y" ) );
	rect.x = 10;
	
	rect.y += rect.h + 8;
	rect.w = 108;
	AddElement( new Label( this, &rect, "Anti-Aliasing:", ItemFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	PrefsMenuRadioButton *fsaa0 = new PrefsMenuRadioButton( this, &rect, ItemFont, "Off", "g_fsaa", "0" );
	fsaa0->Rect.w = fsaa0->GetWidth();
	AddElement( fsaa0 );
	rect.x += fsaa0->Rect.w + 5;
	PrefsMenuRadioButton *fsaa2 = new PrefsMenuRadioButton( this, &rect, ItemFont, "2x", "g_fsaa", "2" );
	fsaa2->Rect.w = fsaa2->GetWidth();
	AddElement( fsaa2 );
	rect.x += fsaa2->Rect.w + 5;
	PrefsMenuRadioButton *fsaa4 = new PrefsMenuRadioButton( this, &rect, ItemFont, "4x", "g_fsaa", "4" );
	fsaa4->Rect.w = fsaa4->GetWidth();
	AddElement( fsaa4 );
	rect.x += fsaa4->Rect.w + 5;
	PrefsMenuRadioButton *fsaa8 = new PrefsMenuRadioButton( this, &rect, ItemFont, "8x", "g_fsaa", "8" );
	fsaa8->Rect.w = fsaa8->GetWidth();
	AddElement( fsaa8 );
	fsaa0->OtherButtons.push_back( fsaa2 );
	fsaa0->OtherButtons.push_back( fsaa4 );
	fsaa0->OtherButtons.push_back( fsaa8 );
	fsaa2->OtherButtons.push_back( fsaa0 );
	fsaa2->OtherButtons.push_back( fsaa4 );
	fsaa2->OtherButtons.push_back( fsaa8 );
	fsaa4->OtherButtons.push_back( fsaa0 );
	fsaa4->OtherButtons.push_back( fsaa2 );
	fsaa4->OtherButtons.push_back( fsaa8 );
	fsaa8->OtherButtons.push_back( fsaa0 );
	fsaa8->OtherButtons.push_back( fsaa2 );
	fsaa8->OtherButtons.push_back( fsaa4 );
	rect.x = 10;
	rect.w = 300;
	
	rect.y += rect.h + 8;
	rect.w = 160;
	AddElement( new Label( this, &rect, "Anisotropic Filtering:", ItemFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	PrefsMenuRadioButton *af0 = new PrefsMenuRadioButton( this, &rect, ItemFont, "Off", "g_af", "1" );
	af0->Rect.w = af0->GetWidth();
	AddElement( af0 );
	rect.x += af0->Rect.w + 5;
	PrefsMenuRadioButton *af2 = new PrefsMenuRadioButton( this, &rect, ItemFont, "2x", "g_af", "2" );
	af2->Rect.w = af2->GetWidth();
	AddElement( af2 );
	rect.x += af2->Rect.w + 5;
	PrefsMenuRadioButton *af4 = new PrefsMenuRadioButton( this, &rect, ItemFont, "4x", "g_af", "4" );
	af4->Rect.w = af4->GetWidth();
	AddElement( af4 );
	rect.x += af4->Rect.w + 5;
	PrefsMenuRadioButton *af8 = new PrefsMenuRadioButton( this, &rect, ItemFont, "8x", "g_af", "8" );
	af8->Rect.w = af8->GetWidth();
	AddElement( af8 );
	rect.x += af8->Rect.w + 5;
	PrefsMenuRadioButton *af16 = new PrefsMenuRadioButton( this, &rect, ItemFont, "16x", "g_af", "16" );
	af16->Rect.w = af16->GetWidth();
	AddElement( af16 );
	af0->OtherButtons.push_back( af2 );
	af0->OtherButtons.push_back( af4 );
	af0->OtherButtons.push_back( af8 );
	af0->OtherButtons.push_back( af16 );
	af2->OtherButtons.push_back( af0 );
	af2->OtherButtons.push_back( af4 );
	af2->OtherButtons.push_back( af8 );
	af2->OtherButtons.push_back( af16 );
	af4->OtherButtons.push_back( af0 );
	af4->OtherButtons.push_back( af2 );
	af4->OtherButtons.push_back( af8 );
	af4->OtherButtons.push_back( af16 );
	af8->OtherButtons.push_back( af0 );
	af8->OtherButtons.push_back( af2 );
	af8->OtherButtons.push_back( af4 );
	af8->OtherButtons.push_back( af16 );
	af16->OtherButtons.push_back( af0 );
	af16->OtherButtons.push_back( af2 );
	af16->OtherButtons.push_back( af4 );
	af16->OtherButtons.push_back( af8 );
	rect.x = 10;
	rect.w = 300;
	
	rect.y += rect.h + 8;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "Draw With Shaders", "g_shader_enable" ) );
	
	/*
	rect.x += 20;
	rect.y += rect.h + 8;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "Lighting", "g_shader_file", "model", "model_simple" ) );
	*/
	
	/*
	rect.x += 20;
	rect.y += rect.h + 8;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "Dynamic Lights", "g_dynamic_lights", "1", "0" ) );
	rect.x -= 40;
	*/
	
	rect.x += 20;
	rect.y += rect.h + 8;
	rect.w = 210;
	AddElement( new Label( this, &rect, "Dynamic Lights Per Object:", ItemFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	PrefsMenuRadioButton *dynamic0 = new PrefsMenuRadioButton( this, &rect, ItemFont, "Off", "g_dynamic_lights", "0" );
	dynamic0->Rect.w = dynamic0->GetWidth();
	AddElement( dynamic0 );
	rect.x += dynamic0->Rect.w + 5;
	PrefsMenuRadioButton *dynamic1 = new PrefsMenuRadioButton( this, &rect, ItemFont, "1", "g_dynamic_lights", "1" );
	dynamic1->Rect.w = dynamic1->GetWidth();
	AddElement( dynamic1 );
	rect.x += dynamic1->Rect.w + 5;
	PrefsMenuRadioButton *dynamic2 = new PrefsMenuRadioButton( this, &rect, ItemFont, "2", "g_dynamic_lights", "2" );
	dynamic2->Rect.w = dynamic2->GetWidth();
	AddElement( dynamic2 );
	rect.x += dynamic2->Rect.w + 5;
	PrefsMenuRadioButton *dynamic3 = new PrefsMenuRadioButton( this, &rect, ItemFont, "3", "g_dynamic_lights", "3" );
	dynamic3->Rect.w = dynamic3->GetWidth();
	AddElement( dynamic3 );
	rect.x += dynamic3->Rect.w + 5;
	PrefsMenuRadioButton *dynamic4 = new PrefsMenuRadioButton( this, &rect, ItemFont, "4", "g_dynamic_lights", "4" );
	dynamic4->Rect.w = dynamic4->GetWidth();
	AddElement( dynamic4 );
	dynamic0->OtherButtons.push_back( dynamic1 );
	dynamic0->OtherButtons.push_back( dynamic2 );
	dynamic0->OtherButtons.push_back( dynamic3 );
	dynamic0->OtherButtons.push_back( dynamic4 );
	dynamic1->OtherButtons.push_back( dynamic0 );
	dynamic1->OtherButtons.push_back( dynamic2 );
	dynamic1->OtherButtons.push_back( dynamic3 );
	dynamic1->OtherButtons.push_back( dynamic4 );
	dynamic2->OtherButtons.push_back( dynamic0 );
	dynamic2->OtherButtons.push_back( dynamic1 );
	dynamic2->OtherButtons.push_back( dynamic3 );
	dynamic2->OtherButtons.push_back( dynamic4 );
	dynamic3->OtherButtons.push_back( dynamic0 );
	dynamic3->OtherButtons.push_back( dynamic1 );
	dynamic3->OtherButtons.push_back( dynamic2 );
	dynamic3->OtherButtons.push_back( dynamic4 );
	dynamic4->OtherButtons.push_back( dynamic0 );
	dynamic4->OtherButtons.push_back( dynamic1 );
	dynamic4->OtherButtons.push_back( dynamic2 );
	dynamic4->OtherButtons.push_back( dynamic3 );
	rect.x = 10;
	rect.w = 300;
	
	rect.y += rect.h + 8;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "High-Quality Ships", "g_hq_ships" ) );
	
	/*
	rect.y += rect.h + 8;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "3D Cockpit", "g_3d_cockpit" ) );
	rect.y += rect.h + 8;
	rect.x += 10;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "High-Quality 3D Cockpit", "g_hq_cockpit" ) );
	rect.x -= 10;
	*/
	
	/*
	rect.y += rect.h + 8;
	AddElement( new Label( this, &rect, "Spectator View:", ItemFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += 125;
	PrefsMenuRadioButton *spectator_view_cockpit = new PrefsMenuRadioButton( this, &rect, ItemFont, "Cockpit", "spectator_view", "cockpit" );
	spectator_view_cockpit->Rect.w = spectator_view_cockpit->GetWidth();
	AddElement( spectator_view_cockpit );
	rect.x += spectator_view_cockpit->Rect.w + 5;
	PrefsMenuRadioButton *spectator_view_chase = new PrefsMenuRadioButton( this, &rect, ItemFont, "Chase", "spectator_view", "chase" );
	spectator_view_chase->Rect.w = spectator_view_chase->GetWidth();
	AddElement( spectator_view_chase );
	rect.x += spectator_view_chase->Rect.w + 5;
	PrefsMenuRadioButton *spectator_view_cinema = new PrefsMenuRadioButton( this, &rect, ItemFont, "Cinema", "spectator_view", "cinema" );
	spectator_view_cinema->Rect.w = spectator_view_cinema->GetWidth();
	AddElement( spectator_view_cinema );
	rect.x += spectator_view_cinema->Rect.w + 5;
	PrefsMenuRadioButton *spectator_view_cinema2 = new PrefsMenuRadioButton( this, &rect, ItemFont, "Cinema2", "spectator_view", "cinema2" );
	spectator_view_cinema2->Rect.w = spectator_view_cinema2->GetWidth();
	AddElement( spectator_view_cinema2 );
	rect.x += spectator_view_cinema2->Rect.w + 5;
	PrefsMenuRadioButton *spectator_view_cycle = new PrefsMenuRadioButton( this, &rect, ItemFont, "Cycle", "spectator_view", "cycle" );
	spectator_view_cycle->Rect.w = spectator_view_cycle->GetWidth();
	AddElement( spectator_view_cycle );
	spectator_view_cockpit->OtherButtons.push_back( spectator_view_chase );
	spectator_view_cockpit->OtherButtons.push_back( spectator_view_cinema );
	spectator_view_cockpit->OtherButtons.push_back( spectator_view_cinema2 );
	spectator_view_cockpit->OtherButtons.push_back( spectator_view_cycle );
	spectator_view_chase->OtherButtons.push_back( spectator_view_cockpit );
	spectator_view_chase->OtherButtons.push_back( spectator_view_cinema );
	spectator_view_chase->OtherButtons.push_back( spectator_view_cinema2 );
	spectator_view_chase->OtherButtons.push_back( spectator_view_cycle );
	spectator_view_cinema->OtherButtons.push_back( spectator_view_cockpit );
	spectator_view_cinema->OtherButtons.push_back( spectator_view_chase );
	spectator_view_cinema->OtherButtons.push_back( spectator_view_cinema2 );
	spectator_view_cinema->OtherButtons.push_back( spectator_view_cycle );
	spectator_view_cinema2->OtherButtons.push_back( spectator_view_cockpit );
	spectator_view_cinema2->OtherButtons.push_back( spectator_view_chase );
	spectator_view_cinema2->OtherButtons.push_back( spectator_view_cinema );
	spectator_view_cinema2->OtherButtons.push_back( spectator_view_cycle );
	spectator_view_cycle->OtherButtons.push_back( spectator_view_cockpit );
	spectator_view_cycle->OtherButtons.push_back( spectator_view_chase );
	spectator_view_cycle->OtherButtons.push_back( spectator_view_cinema );
	spectator_view_cycle->OtherButtons.push_back( spectator_view_cinema2 );
	rect.x = 10;
	rect.w = 300;
	*/
	
	rect.y += rect.h + 8;
	rect.w = 150;
	AddElement( new Label( this, &rect, "Joystick Deadzone:", ItemFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 60;
	AddElement( new PrefsMenuTextBox( this, &rect, ItemFont, Font::ALIGN_MIDDLE_CENTER, "joy_deadzone" ) );
	rect.x = 10;
	rect.w = 300;
	
	rect.y += rect.h + 8;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "Swap Joystick Yaw/Roll (Classic Controls)", "joy_swap_xz" ) );
	
	rect.y += rect.h + 8;
	rect.w = 190;
	AddElement( new Label( this, &rect, "Joystick Twist Smoothing:", ItemFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 80;
	PrefsMenuDropDown *joy_smooth_dropdown = new PrefsMenuDropDown( this, &rect, ItemFont, Font::ALIGN_MIDDLE_LEFT, 0, "joy_smooth_z" );
	joy_smooth_dropdown->Items.push_back( ListBoxItem("0"," Off ") );
	joy_smooth_dropdown->Items.push_back( ListBoxItem("0.5"," Low ") );
	joy_smooth_dropdown->Items.push_back( ListBoxItem("1"," High ") );
	joy_smooth_dropdown->Update();
	AddElement( joy_smooth_dropdown );
	rect.x = 10;
	rect.w = 300;
	
	rect.y += rect.h + 8;
	AddElement( new Label( this, &rect, "Fly With Mouse (If No Joystick):", ItemFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += 250;
	PrefsMenuRadioButton *mouse_enable_never = new PrefsMenuRadioButton( this, &rect, ItemFont, "Never", "mouse_enable", "false" );
	mouse_enable_never->Rect.w = mouse_enable_never->GetWidth();
	AddElement( mouse_enable_never );
	rect.x += mouse_enable_never->Rect.w + 5;
	PrefsMenuRadioButton *mouse_enable_fullscreen = new PrefsMenuRadioButton( this, &rect, ItemFont, "Fullscreen", "mouse_enable", "fullscreen" );
	mouse_enable_fullscreen->Rect.w = mouse_enable_fullscreen->GetWidth();
	AddElement( mouse_enable_fullscreen );
	rect.x += mouse_enable_fullscreen->Rect.w + 5;
	PrefsMenuRadioButton *mouse_enable_always = new PrefsMenuRadioButton( this, &rect, ItemFont, "Always", "mouse_enable", "true" );
	mouse_enable_always->Rect.w = mouse_enable_always->GetWidth();
	AddElement( mouse_enable_always );
	mouse_enable_never->OtherButtons.push_back( mouse_enable_fullscreen );
	mouse_enable_never->OtherButtons.push_back( mouse_enable_always );
	mouse_enable_fullscreen->OtherButtons.push_back( mouse_enable_never );
	mouse_enable_fullscreen->OtherButtons.push_back( mouse_enable_always );
	mouse_enable_always->OtherButtons.push_back( mouse_enable_never );
	mouse_enable_always->OtherButtons.push_back( mouse_enable_fullscreen );
	rect.x = 10;
	rect.w = 300;
	
	rect.y += rect.h + 8;
	rect.x += 20;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "Invert Mouse", "mouse_invert" ) );
	rect.x -= 20;
	
	rect.y += rect.h + 8;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "Menu Music", "s_menu_music" ) );
	
	rect.y += rect.h + 8;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "Flight Music", "s_game_music" ) );
	
	rect.y += rect.h + 8;
	AddElement( new PrefsMenuCheckBox( this, &rect, ItemFont, "Silly Sounds", "res_sound_dir", "Sounds/Silly", "Sounds" ) );
	
	rect.w = 150;
	rect.h = 50;
	rect.y = Rect.h - rect.h - 10;
	rect.x = Rect.w - rect.w - 10;
	AddElement( new PrefsMenuDoneButton( this, &rect, "Done" ) );
	
	rect.x -= (rect.w + 10);
	AddElement( new PrefsMenuDefaultsButton( this, &rect, "Defaults" ) );
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


PrefsMenuCheckBox::PrefsMenuCheckBox( Window *Container, SDL_Rect *rect, Font *font, std::string label, std::string variable, std::string true_str, std::string false_str ) : CheckBox( Container, rect, font, label, Raptor::Game->Cfg.SettingAsString( variable ) == true_str, Raptor::Game->Res.GetAnimation("box_unchecked.ani"), Raptor::Game->Res.GetAnimation("box_unchecked_mdown.ani"), NULL, Raptor::Game->Res.GetAnimation("box_checked.ani"), Raptor::Game->Res.GetAnimation("box_checked_mdown.ani"), NULL )
{
	Variable = variable;
	TrueStr = true_str;
	FalseStr = false_str;
}


void PrefsMenuCheckBox::Changed( void )
{
	Raptor::Game->Cfg.Settings[ Variable ] = Checked ? TrueStr : FalseStr;
}


// ---------------------------------------------------------------------------


PrefsMenuRadioButton::PrefsMenuRadioButton( Window *Container, SDL_Rect *rect, Font *font, std::string label, std::string variable, std::string true_str ) : CheckBox( Container, rect, font, label, Raptor::Game->Cfg.SettingAsString( variable ) == true_str, Raptor::Game->Res.GetAnimation("box_unchecked.ani"), Raptor::Game->Res.GetAnimation("box_unchecked_mdown.ani"), NULL, Raptor::Game->Res.GetAnimation("box_checked.ani"), Raptor::Game->Res.GetAnimation("box_checked_mdown.ani"), NULL )
{
	Variable = variable;
	TrueStr = true_str;
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


PrefsMenuTextBox::PrefsMenuTextBox( Window *Container, SDL_Rect *rect, Font *font, uint8_t align, std::string variable ) : TextBox( Container, rect, font, align, Raptor::Game->Cfg.SettingAsString( variable ) )
{
	Variable = variable;
	LinkedListBox = NULL;
	ReturnDeselects = true;
}


void PrefsMenuTextBox::Changed( void )
{
	Raptor::Game->Cfg.Settings[ Variable ] = Text;
}


// ---------------------------------------------------------------------------


PrefsMenuListBox::PrefsMenuListBox( Window *Container, SDL_Rect *rect, Font *font, int scroll_bar_size, std::string variable, TextBox *linked_text_box ) : ListBox( Container, rect, font, scroll_bar_size )
{
	Variable = variable;
	LinkedTextBox = linked_text_box;
}


void PrefsMenuListBox::Changed( void )
{
	if( LinkedTextBox )
		LinkedTextBox->Text = SelectedValue();
	
	Raptor::Game->Cfg.Settings[ Variable ] = SelectedValue();
}


// ---------------------------------------------------------------------------


PrefsMenuDropDown::PrefsMenuDropDown( Layer *container, SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, std::string variable ) : DropDown( container, rect, font, align, scroll_bar_size, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Variable = variable;
	Value = Raptor::Game->Cfg.SettingAsString( Variable );
}


void PrefsMenuDropDown::Changed( void )
{
	Raptor::Game->Cfg.Settings[ Variable ] = Value;
}


// ---------------------------------------------------------------------------


PrefsMenuDoneButton::PrefsMenuDoneButton( PrefsMenu *menu, SDL_Rect *rect, const char *label ) : LabelledButton( menu, rect, menu->ButtonFont, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
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
	else if( menu->PrevShaderFile != Raptor::Game->Cfg.SettingAsString("g_shader_file") )
		Raptor::Game->ShaderMgr.LoadShaders( Raptor::Game->Cfg.SettingAsString("g_shader_file") );
	
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


PrefsMenuDefaultsButton::PrefsMenuDefaultsButton( PrefsMenu *menu, SDL_Rect *rect, const char *label ) : LabelledButton( menu, rect, menu->ButtonFont, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
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
