/*
 *  IngameMenu.cpp
 */

#include "IngameMenu.h"

#include "RaptorGame.h"
#include "PrefsMenu.h"
#include "TextFileViewer.h"
#include "XWingDefs.h"
#include "XWingGame.h"
#include "Num.h"


IngameMenu::IngameMenu( void )
{
	Background.BecomeInstance( Raptor::Game->Res.GetAnimation("bg_menu.ani") );
	
	TitleFontBig = Raptor::Game->Res.GetFont( "Candara.ttf", 128 );
	TitleFontSmall = Raptor::Game->Res.GetFont( "Candara.ttf", 64 );
	TitleFont = TitleFontBig;
	ButtonFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 40 );
	VersionFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 15 );
	
	SDL_Rect button_rect;
	button_rect.w = 560;
	button_rect.h = ButtonFont->GetHeight() + 2;
	button_rect.x = 0;
	button_rect.y = 0;
	
	Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	std::string player_team = player ? player->PropertyAsString("team") : "Spectator";
	
	AddElement( new IngameMenuResumeButton( &button_rect, ButtonFont ));
	
	if( Raptor::Game->Data.PropertyAsBool("allow_ship_change",true) )
	{
		AddElement( new IngameMenuShipDropDown( &button_rect, ButtonFont ));
		AddElement( new IngameMenuGroupDropDown( &button_rect, ButtonFont ));
	}
	else
	{
		if( player_team != "Spectator" )
			AddElement( new IngameMenuGroupDropDown( &button_rect, ButtonFont ));
	}
	
	if( player->PropertyAsString("ship") == "Spectator" )
		AddElement( new IngameMenuViewDropDown( &button_rect, ButtonFont ));
	
	AddElement( new IngameMenuPrefsButton( &button_rect, ButtonFont ));
	//AddElement( new IngameMenuHelpButton( &button_rect, ButtonFont ));
	AddElement( new IngameMenuLeaveButton( &button_rect, ButtonFont ));
	
	UpdateRects();
}


IngameMenu::~IngameMenu()
{
}


void IngameMenu::UpdateRects( void )
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


void IngameMenu::Draw( void )
{
	UpdateRects();
	
	bool vr = Raptor::Game->Head.VR && Raptor::Game->Gfx.DrawTo;
	if( ! vr )
	{
		if( IsTop() )
			TitleFont->DrawText( Raptor::Game->Game, Rect.w / 2, TitleFont->GetHeight() / 2, Font::ALIGN_TOP_CENTER );
		
		VersionFont->DrawText( "Version " + Raptor::Game->Version, Rect.w - 10, Rect.h - 10, Font::ALIGN_BOTTOM_RIGHT );
	}
}


bool IngameMenu::KeyDown( SDLKey key )
{
	if( (key == SDLK_ESCAPE) || (key == SDLK_F9) ) // FIXME: Check for XWing::Controls::MENU instead?
	{
		Remove();
		Raptor::Game->Mouse.ShowCursor = false;
		return true;
	}
	
	return false;
}


// ---------------------------------------------------------------------------


IngameMenuResumeButton::IngameMenuResumeButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Resume", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


IngameMenuResumeButton::~IngameMenuResumeButton()
{
}


void IngameMenuResumeButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	// Close the ingame menu.
	Container->Remove();
	Raptor::Game->Mouse.ShowCursor = false;
}


// ---------------------------------------------------------------------------


IngameMenuPrefsButton::IngameMenuPrefsButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Preferences", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


IngameMenuPrefsButton::~IngameMenuPrefsButton()
{
}


void IngameMenuPrefsButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	// Close the ingame menu.
	Container->Remove();
	
	// Open the prefs.
	Raptor::Game->Layers.Add( new PrefsMenu() );
}


// ---------------------------------------------------------------------------


IngameMenuHelpButton::IngameMenuHelpButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, "    Help", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


IngameMenuHelpButton::~IngameMenuHelpButton()
{
}


void IngameMenuHelpButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	// Close the ingame menu.
	Container->Remove();
	
	// Show the readme.
	Raptor::Game->Layers.Add( new TextFileViewer( NULL, "README.txt", NULL, "Help and Info" ) );
}


// ---------------------------------------------------------------------------


IngameMenuLeaveButton::IngameMenuLeaveButton( SDL_Rect *rect, Font *button_font, uint8_t align )
: LabelledButton( rect, button_font, (Raptor::Server->IsRunning() ? ((Raptor::Server->State == XWing::State::FLYING) ? "    End Round" : "    Stop Game") : "    Leave"), align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
}


IngameMenuLeaveButton::~IngameMenuLeaveButton()
{
}


void IngameMenuLeaveButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	// Close the ingame menu.
	Container->Remove();
	
	// If we're hosting and the round is in progress, end the round.
	if( Raptor::Server->IsRunning() && (Raptor::Server->State == XWing::State::FLYING) )
	{
		Raptor::Server->State = XWing::State::ROUND_WILL_END;
		Raptor::Game->Mouse.ShowCursor = false;
	}
	// Any other state, leave the game.
	else
		Raptor::Game->Net.DisconnectNice();
}


// ---------------------------------------------------------------------------


IngameMenuShipDropDown::IngameMenuShipDropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size ) : DropDown( rect, font, align, scroll_bar_size, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
	
	uint32_t gametype = ((const XWingGame*)( Raptor::Game ))->GameType;
	std::string player_ship;
	uint8_t player_team = XWing::Team::NONE;
	Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	if( player )
	{
		player_ship = player->PropertyAsString("ship");
		std::string prefix = (gametype == XWing::GameType::BATTLE_OF_YAVIN) ? "yavin_" : "";
		std::string team = player->PropertyAsString("team");
		if( team == "Rebel" )
		{
			player_team = XWing::Team::REBEL;
			if( player_ship.empty() )
				player_ship = Raptor::Game->Data.PropertyAsString(prefix+std::string("rebel_fighter"),"X/W");
		}
		else if( team == "Empire" )
		{
			player_team = XWing::Team::EMPIRE;
			if( player_ship.empty() )
				player_ship = Raptor::Game->Data.PropertyAsString(prefix+std::string("empire_fighter"),"T/F");
		}
		else if( player_ship.empty() )
			player_ship = "Spectator";
	}
	
	bool allow_team_change = (gametype == XWing::GameType::FFA_ELIMINATION) || (gametype == XWing::GameType::FFA_DEATHMATCH) || Raptor::Game->Data.PropertyAsBool("allow_team_change");
	
	if( player_ship == "Spectator" )
		AddItem( "Spectator", "    Select Ship" );
	
	for( std::map<uint32_t,GameObject*>::const_iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->Type() == XWing::Object::SHIP_CLASS )
		{
			const ShipClass *sc = (const ShipClass*) obj_iter->second;
			if( ( (sc->PlayersCanFly() || Raptor::Game->Data.PropertyAsBool("darkside",false)) && ((sc->Team == player_team) || ! sc->Team || ! player_team || allow_team_change) )
			||  (sc->ShortName == player_ship) )
				AddItem( sc->ShortName, std::string("    Ship: ") + sc->LongName );
		}
	}
	
	if( (player_team != XWing::Team::EMPIRE) || allow_team_change )
		AddItem( "Rebel Gunner",    "    Role: Rebel Gunner" );
	
	if( (player_team != XWing::Team::REBEL) || allow_team_change )
		AddItem( "Imperial Gunner", "    Role: Imperial Gunner" );
	
	if( player_ship != "Spectator" )
		AddItem( "Spectator", "    Spectate" );
	
	Value = player_ship;
	Update();
}


IngameMenuShipDropDown::~IngameMenuShipDropDown()
{
}


void IngameMenuShipDropDown::Changed( void )
{
	if( Raptor::Game->SetPlayerProperty( "ship", Value ) )
	{
		if( (Value == "Spectator") || (Str::FindInsensitive( Value, " Gunner" ) >= 0) )
			Raptor::Game->MessageReceived( std::string("You will be a ") + Value + std::string(" next.") );
		else // FIXME: Don't show this when changing from Spectator.
		{
			const ShipClass *ship_class = ((XWingGame*)( Raptor::Game ))->GetShipClass( Value );
			std::string ship_class_name = ship_class ? ship_class->LongName : Value;
			std::string message = "Your next ship will be a ";
			if( Str::BeginsWith( ship_class_name, "A" )
			||  Str::BeginsWith( ship_class_name, "E" )
			||  Str::BeginsWith( ship_class_name, "I" )
			||  Str::BeginsWith( ship_class_name, "O" )
			||  Str::BeginsWith( ship_class_name, "F-" )
			||  Str::BeginsWith( ship_class_name, "H-" )
			||  Str::BeginsWith( ship_class_name, "L-" )
			||  Str::BeginsWith( ship_class_name, "M-" )
			||  Str::BeginsWith( ship_class_name, "N-" )
			||  Str::BeginsWith( ship_class_name, "R-" )
			||  Str::BeginsWith( ship_class_name, "S-" )
			||  Str::BeginsWith( ship_class_name, "X-" ) )
				message = "Your next ship will be an ";
			Raptor::Game->MessageReceived( message + ship_class_name + std::string(".") );
		}
		
		// Remove the menu after changing ship.
		Container->Remove();
	}
}


// ---------------------------------------------------------------------------


IngameMenuGroupDropDown::IngameMenuGroupDropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size ) : DropDown( rect, font, align, scroll_bar_size, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
	
	std::string player_group = "0";
	Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	if( player )
		player_group = Num::ToString( player->PropertyAsInt("group") );
	
	if( player_group == "0" )
		AddItem( "0", "    Select Flight Group" );
	else
		AddItem( "0", "    Flight Group: None" );
	
	AddItem( "1", "    Flight Group: 1" );
	AddItem( "2", "    Flight Group: 2" );
	AddItem( "3", "    Flight Group: 3" );
	AddItem( "4", "    Flight Group: 4" );
	
	Value = player_group;
	Update();
}


IngameMenuGroupDropDown::~IngameMenuGroupDropDown()
{
}


void IngameMenuGroupDropDown::Changed( void )
{
	if( Raptor::Game->SetPlayerProperty( "group", Value ) )
	{
		if( atoi( Value.c_str() ) )
			Raptor::Game->Msg.Print( std::string("You are now in flight group ") + Value + std::string(".") );
		else
			Raptor::Game->Msg.Print( "You left the flight group." );
		
		// Remove the menu after changing group.
		Container->Remove();
	}
}


// ---------------------------------------------------------------------------


IngameMenuViewDropDown::IngameMenuViewDropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size ) : DropDown( rect, font, align, scroll_bar_size, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
	
	std::string view = Raptor::Game->Cfg.SettingAsString("spectator_view");
	
	if( view == "auto" )
		AddItem( "auto",    "    Change View" );
	else
		AddItem( "auto",    "    View: Auto" );
	
	AddItem( "cinema",      "    View: Cinema" );
	AddItem( "cockpit",     "    View: Cockpit" );
	AddItem( "chase",       "    View: Chase" );
	AddItem( "fixed",       "    View: Fixed" );
	AddItem( "gunner",      "    View: Gunner" );
	AddItem( "cycle",       "    View: Cycle" );
	AddItem( "stationary",  "    View: Stationary" );
	AddItem( "instruments", "    View: Instruments" );
	
	Value = view;
	Update();
}


IngameMenuViewDropDown::~IngameMenuViewDropDown()
{
}


void IngameMenuViewDropDown::Changed( void )
{
	std::string view = Raptor::Game->Cfg.SettingAsString("spectator_view");
	if( Value != view )
	{
		Raptor::Game->Cfg.Settings[ "spectator_view" ] = Value;
		
		// Remove the menu after changing view.
		Container->Remove();
	}
}
