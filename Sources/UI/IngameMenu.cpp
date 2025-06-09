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
	button_rect.w = 700;
	button_rect.h = ButtonFont->GetHeight() + 2;
	button_rect.x = 0;
	button_rect.y = 0;
	
	Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	std::string player_team = player ? player->PropertyAsString("team") : "Spectator";
	
	AddElement( new IngameMenuResumeButton( &button_rect, ButtonFont ));
	
	bool respawn = Raptor::Game->Data.PropertyAsBool("respawn");
	bool campaign = false;
	if( Raptor::Game->Data.PropertyAsString("gametype") == "mission" )
	{
		// Campaign missions do not allow respawn.
		// FIXME: Server should set mission_respawn property and we should check that instead.
		std::string mission = Raptor::Game->Data.PropertyAsString("mission");
		if( Str::BeginsWith( mission, "rebel" ) || Str::BeginsWith( mission, "empire" ) )
		{
			respawn = false;
			campaign = true;
		}
	}
	
	if( respawn
	&&  Raptor::Game->Data.PropertyAsBool("allow_ship_change",true)
	&& (Raptor::Game->Data.PropertyAsString("player_ship").empty()  || Str::Count(Raptor::Game->Data.PropertyAsString("player_ship"), " "))
	&& (Raptor::Game->Data.PropertyAsString("player_ships").empty() || Str::Count(Raptor::Game->Data.PropertyAsString("player_ships")," ")) )
	{
		AddElement( new IngameMenuShipDropDown( &button_rect, ButtonFont ));
		AddElement( new IngameMenuGroupDropDown( &button_rect, ButtonFont ));
	}
	else if( (player_team != "Spectator") && ! campaign )
		AddElement( new IngameMenuGroupDropDown( &button_rect, ButtonFont ));
	AddElement( new IngameMenuPrefsButton( &button_rect, ButtonFont ));
	AddElement( new IngameMenuLeaveButton( &button_rect, ButtonFont ));
	
	UpdateRects();
	
	XWingGame *game = (XWingGame*) Raptor::Game;
	Paused = (game->State >= XWing::State::FLYING)
		&& (game->Data.TimeScale == 1.)
		&& (game->Data.Players.size() == 1)
		&& game->Cfg.SettingAsBool("ui_pause",true)
		&& Raptor::Server->IsRunning()
		&& game->ControlPressed( game->Controls[ XWing::Control::PAUSE ] );
}


IngameMenu::~IngameMenu()
{
	XWingGame *game = (XWingGame*) Raptor::Game;
	if( Paused && (game->State >= XWing::State::FLYING) && (game->Data.TimeScale < 0.0000011) )
		game->ControlPressed( game->Controls[ XWing::Control::PAUSE ] );  // Unpause
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
	IngameMenu *ingame_menu = (IngameMenu*) Container;
	ingame_menu->Remove();
	
	// Open the prefs and give it control of automatic unpause.
	PrefsMenu *prefs_menu = new PrefsMenu();
	Raptor::Game->Layers.Add( prefs_menu );
	prefs_menu->Paused = ingame_menu->Paused;
	ingame_menu->Paused = false;
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
: LabelledButton( rect, button_font, "    Return to Lobby", align, Raptor::Game->Res.GetAnimation("fade.ani") )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
	
	LobbyAttempted = false;
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
	
	if( ! LobbyAttempted )
	{
		// Tell the server we want to go back to the lobby.
		Packet lobby( XWing::Packet::LOBBY );
		Raptor::Game->Net.Send( &lobby );
		
		LobbyAttempted = true;
	}
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
	bool allow_team_change = (gametype == XWing::GameType::FFA_ELIMINATION) || (gametype == XWing::GameType::FFA_DEATHMATCH) || Raptor::Game->Data.PropertyAsBool("allow_team_change");
	bool darkside = (Raptor::Game->Cfg.SettingAsBool("darkside",false) || Raptor::Game->Data.PropertyAsBool("darkside",false)) && ! Raptor::Game->Data.PropertyAsBool("lightside",false);
	
	std::vector<std::string> allowed_ships;
	if( Raptor::Game->Data.PropertyAsString("gametype") == "mission" )
	{
		std::string player_ships = Raptor::Game->Data.PropertyAsString("player_ships");
		if( player_ships.empty() )
			player_ships = Raptor::Game->Data.PropertyAsString("player_ship");
		if( ! player_ships.empty() )
			allowed_ships = Str::SplitToVector( player_ships, " " );
	}
	
	std::string player_ship;
	uint8_t player_team = XWing::Team::NONE;
	Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	if( player )
	{
		player_ship = player->PropertyAsString("ship");
		if( player_ship.empty() && allowed_ships.size() )
			player_ship = allowed_ships.at(0);
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
	
	if( player_ship == "Spectator" )
		AddItem( "Spectator", "    Select Ship" );
	
	if( allowed_ships.size() && ! darkside )
	{
		for( std::vector<std::string>::const_iterator allowed_iter = allowed_ships.begin(); allowed_iter != allowed_ships.end(); allowed_iter ++ )
		{
			const ShipClass *sc = ((const XWingGame*)( Raptor::Game ))->GetShipClass( *allowed_iter );
			if( sc )
				AddItem( sc->ShortName, std::string("    Ship: ") + sc->LongName );
			else if( *allowed_iter == "rebel_gunner" )
				AddItem( "Rebel Gunner", "    Role: Rebel Gunner" );
			else if( *allowed_iter == "empire_gunner" )
				AddItem( "Imperial Gunner", "    Role: Imperial Gunner" );
			else if( *allowed_iter == "spectator" )
			{
				if( player_ship != "Spectator" )
					AddItem( "Spectator", "    Spectate" );
			}
			else
				AddItem( *allowed_iter, std::string("    Ship: ") + *allowed_iter );
		}
	}
	else
	{
		for( std::map<uint32_t,GameObject*>::const_iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHIP_CLASS )
			{
				const ShipClass *sc = (const ShipClass*) obj_iter->second;
				bool allowed = (darkside || sc->PlayersCanFly()) && (allow_team_change || (! sc->Team) || (sc->Team == player_team));
				
				if( allowed_ships.size() && ! darkside )
				{
					allowed = false;
					for( std::vector<std::string>::const_iterator ship_iter = allowed_ships.begin(); ship_iter != allowed_ships.end(); ship_iter ++ )
					{
						if( Str::EqualsInsensitive( sc->ShortName, *ship_iter ) )
						{
							allowed = true;
							break;
						}
					}
				}
				
				if( allowed )
					AddItem( sc->ShortName, std::string("    Ship: ") + sc->LongName );
			}
		}
		
		if( (player_team != XWing::Team::EMPIRE) || allow_team_change )
			AddItem( "Rebel Gunner",    "    Role: Rebel Gunner" );
		
		if( (player_team != XWing::Team::REBEL) || allow_team_change )
			AddItem( "Imperial Gunner", "    Role: Imperial Gunner" );
		
		if( player_ship != "Spectator" )
			AddItem( "Spectator", "    Spectate" );
	}
	
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
		AddItem( "0", "    Select Squadron" );
	else
		AddItem( "0", "    Squadron: None" );
	
	AddItem( "1", "    Squadron: Red" );
	AddItem( "2", "    Squadron: Gold" );
	AddItem( "3", "    Squadron: Green" );
	AddItem( "4", "    Squadron: Blue" );
	
	if( atoi( player_group.c_str() ) > 4 )
		AddItem( player_group, std::string("    Squadron: ") + player_group );
	
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
		int group_num = atoi( Value.c_str() );
		if( group_num == 1 )
			Raptor::Game->Msg.Print( std::string("You are now in Red Squadron.") );
		else if( group_num == 2 )
			Raptor::Game->Msg.Print( std::string("You are now in Gold Squadron.") );
		else if( group_num == 2 )
			Raptor::Game->Msg.Print( std::string("You are now in Green Squadron.") );
		else if( group_num == 2 )
			Raptor::Game->Msg.Print( std::string("You are now in Blue Squadron.") );
		else if( group_num )
			Raptor::Game->Msg.Print( std::string("You are now in squadron ") + Value + std::string(".") );
		else
			Raptor::Game->Msg.Print( "You left the squadron." );
		
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
	
	AddItem( "cycle",       "    View: Cycle" );
	AddItem( "cinema",      "    View: Cinema" );
	AddItem( "fixed",       "    View: Fixed" );
	AddItem( "chase",       "    View: Chase" );
	AddItem( "cockpit",     "    View: Cockpit" );
	AddItem( "crosshair",   "    View: Crosshair" );
	AddItem( "padlock",     "    View: Padlock" );
	AddItem( "gunner",      "    View: Gunner" );
	AddItem( "selfie",      "    View: Selfie" );
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
