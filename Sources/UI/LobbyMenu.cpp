/*
 *  LobbyMenu.cpp
 */

#include "LobbyMenu.h"

#include "XWingDefs.h"
#include "XWingGame.h"
#include "Num.h"
#include "PrefsMenu.h"
#include "FleetMenu.h"
#include "MissionUploader.h"
#include <algorithm>


LobbyMenu::LobbyMenu( void )
{
	Name = "LobbyMenu";
	
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	if( Raptor::Game->Head.VR )
	{
		Rect.x = Raptor::Game->Gfx.W/2 - 640/2;
		Rect.y = Raptor::Game->Gfx.H/2 - 480/2;
		Rect.w = 640;
		Rect.h = 480;
	}
	
	bool empire_campaign = (((const XWingGame*)( Raptor::Game ))->CampaignTeam == XWing::Team::EMPIRE);
	Background.BecomeInstance( Raptor::Game->Res.GetAnimation( empire_campaign ? "bg_lobby2.ani" : "bg_lobby.ani" ) );
	
	bool tiny = (Rect.h < 720) || (Rect.w < 800);
	TitleFont = Raptor::Game->Res.GetFont( "Verdana.ttf", tiny ? 24 : 30 );
	
	AddElement( ShipView = new LobbyMenuShipView() );
	AddElement( PrefsButton = new LobbyMenuPrefsButton( tiny ? "Prefs" : "Preferences" ) );
	AddElement( LeaveButton = new LobbyMenuLeaveButton() );
	AddElement( FlyButton = new LobbyMenuFlyButton() );
	
	ShipDropDown = new LobbyMenuShipDropDown( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", tiny ? 13 : 19 ), Font::ALIGN_TOP_LEFT, 0, "ship" );
	AddElement( ShipDropDown );
	
	GroupDropDown = new LobbyMenuPlayerDropDown( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", tiny ? 13 : 19 ), Font::ALIGN_TOP_LEFT, 0, "group" );
	GroupDropDown->AddItem( "0", "Solo" );
	GroupDropDown->AddItem( "1", "Red" );
	GroupDropDown->AddItem( "2", "Gold" );
	GroupDropDown->AddItem( "3", "Green" );
	GroupDropDown->AddItem( "4", "Blue" );
	GroupDropDown->Value = "0";
	const Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	if( player )
		GroupDropDown->Value = player->PropertyAsString("group","0");
	GroupDropDown->Update();
	AddElement( GroupDropDown );
	
	AddElement( PlayerName = new LobbyMenuPlayerTextBox( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", tiny ? 13 : 19 ), Font::ALIGN_MIDDLE_LEFT, "name" ) );
	PlayerName->ReturnDeselects = false;
	PlayerName->EscDeselects = true;
	PlayerName->PassReturn = true;
	PlayerName->PassEsc = true;
	PlayerName->SelectedRed = 1.f;
	PlayerName->SelectedGreen = 1.f;
	PlayerName->SelectedBlue = 0.f;
	PlayerName->SelectedAlpha = 1.f;
	
	AddElement( PlayerList = new ListBox( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", tiny ? 13 : 19 ), 16 ) );
	PlayerList->SelectedRed = PlayerList->TextRed;
	PlayerList->SelectedGreen = PlayerList->TextGreen;
	PlayerList->SelectedBlue = PlayerList->TextBlue;
	PlayerList->SelectedAlpha = PlayerList->TextAlpha;
	
	AddElement( MessageList = new ListBox( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", tiny ? 12 : 16 ), 16 ) );
	MessageList->SelectedRed = MessageList->TextRed;
	MessageList->SelectedGreen = MessageList->TextGreen;
	MessageList->SelectedBlue = MessageList->TextBlue;
	MessageList->SelectedAlpha = MessageList->TextAlpha;
	
	AddElement( MessageInput = new TextBox( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", tiny ? 12 : 16 ), Font::ALIGN_TOP_LEFT ) );
	MessageInput->ReturnDeselects = false;
	MessageInput->EscDeselects = false;
	MessageInput->PassReturn = true;
	MessageInput->PassEsc = true;
	MessageInput->TextRed = 1.f;
	MessageInput->TextGreen = 1.f;
	MessageInput->TextBlue = 1.f;
	MessageInput->TextAlpha = 0.5f;
	MessageInput->SelectedTextRed = 1.f;
	MessageInput->SelectedTextGreen = 1.f;
	MessageInput->SelectedTextBlue = 0.f;
	MessageInput->SelectedTextAlpha = 1.f;
	MessageInput->Red = MessageList->Red;
	MessageInput->Green = MessageList->Green;
	MessageInput->Blue = MessageList->Blue;
	MessageInput->Alpha = MessageList->Alpha;
	MessageInput->SelectedRed = MessageInput->Red;
	MessageInput->SelectedGreen = MessageInput->Green;
	MessageInput->SelectedBlue = MessageInput->Blue;
	MessageInput->SelectedAlpha = MessageInput->Alpha;
	
	int label_size = tiny ? 12 : 17;
	int value_size = tiny ? 15 : 22;
	ConfigOrder.push_back( new LobbyMenuConfiguration( "permissions", "", 0, tiny ? 14 : 17 ) );
	ConfigOrder.push_back( NULL );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "gametype", "Game Type", tiny ? 12 : 17, tiny ? 22 : 27 ) );
	ConfigOrder.push_back( NULL );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "mission", "", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "customize_fleet", "", label_size, value_size ) );
	ConfigOrder.push_back( NULL );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "defending_team", "Defending Team", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "rebel_frigates", "Rebel Battleships", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "rebel_cruisers", "Rebel Cruisers", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "empire_frigates", "Imperial Battleships", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "empire_cruisers", "Imperial Cruisers", label_size, value_size ) );
	ConfigOrder.push_back( NULL );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "race_circuit", "Race Mode", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "race_lap", "Lap Length", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "team_race_checkpoints", "Race Length", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "ffa_race_checkpoints", "Race Length", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "tdm_kill_limit", "Team Kill Limit", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "dm_kill_limit", "Kill Limit", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "hunt_time_limit", "Time Limit", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "yavin_time_limit", "Time Limit", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "race_time_limit", "Time Limit", label_size, value_size ) );
	ConfigOrder.push_back( NULL );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "ai_waves", "AI Ships", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "ai_skill", "AI Pilot Skill", label_size, value_size ) );
	ConfigOrder.push_back( NULL );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "yavin_turrets", "Surface Turrets", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "asteroids", "Asteroids", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "bg", "Environment", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "allow_team_change", "Allow Team Change", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "respawn", "Respawn", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "respawn_time", "Respawn Time", label_size, value_size ) );
	ConfigOrder.push_back( NULL );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "defaults", "", label_size, value_size ) );
	ConfigOrder.push_back( NULL );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "upload_mission", "", label_size, value_size ) );
	
	for( std::vector<LobbyMenuConfiguration*>::iterator config_iter = ConfigOrder.begin(); config_iter != ConfigOrder.end(); config_iter ++ )
	{
		if( *config_iter )
		{
			AddElement( *config_iter );
			Configs[ (*config_iter)->Property ] = *config_iter;
		}
	}
}


LobbyMenu::~LobbyMenu()
{
}


void LobbyMenu::UpdateRects( void )
{
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	if( Raptor::Game->Head.VR && Raptor::Game->Gfx.DrawTo )
	{
		Rect.x = Raptor::Game->Gfx.W/2 - 640/2;
		Rect.y = Raptor::Game->Gfx.H/2 - 480/2;
		Rect.w = 640;
		Rect.h = 480;
	}
	
	bool tiny = (Rect.h < 720) || (Rect.w < 800);
	
	LeaveButton->Rect.h = LeaveButton->LabelFont->GetHeight() + 4;
	FlyButton->Rect.h = PrefsButton->Rect.h = LeaveButton->Rect.h;
	
	LeaveButton->Rect.x = 32;
	
	if( Rect.w >= 1024 )
		LeaveButton->Rect.w = 384;
	else
		LeaveButton->Rect.w = Rect.w / 3 - LeaveButton->Rect.x * 2;
	FlyButton->Rect.w = PrefsButton->Rect.w = LeaveButton->Rect.w;
	
	LeaveButton->Rect.y = Rect.h - LeaveButton->Rect.h - 32;
	FlyButton->Rect.y = PrefsButton->Rect.y = LeaveButton->Rect.y;
	
	FlyButton->Rect.x = Rect.w - FlyButton->Rect.w - LeaveButton->Rect.x;
	PrefsButton->Rect.x = (Rect.w - PrefsButton->Rect.w) / 2;
	
	PlayerName->Rect.x = LeaveButton->Rect.x;
	PlayerName->Rect.w = (FlyButton->Rect.x + FlyButton->Rect.w - PlayerList->Rect.x) / 2 - 5;
	PlayerName->Rect.y = TitleFont->GetHeight() + 20;
	
	ShipDropDown->Rect.x = LeaveButton->Rect.x;
	ShipDropDown->Rect.y = PlayerName->Rect.y + PlayerName->Rect.h + 10;
	ShipDropDown->Rect.w = tiny ? 200 : 300;
	ShipDropDown->Rect.h = ShipDropDown->LabelFont->GetHeight() + 6;
	
	GroupDropDown->Rect.x = ShipDropDown->Rect.x + ShipDropDown->Rect.w + 10;
	GroupDropDown->Rect.y = ShipDropDown->Rect.y;
	GroupDropDown->Rect.w = tiny ? 80 : 100;
	GroupDropDown->Rect.h = GroupDropDown->LabelFont->GetHeight() + 6;
	
	if( ShipDropDown->Value.length() && (ShipDropDown->Value != "Spectator") && (Raptor::Game->Data.PropertyAsString("gametype").find("ffa_") != 0) )
	{
		GroupDropDown->Visible = true;
		GroupDropDown->Enabled = true;
	}
	else
	{
		GroupDropDown->Visible = false;
		GroupDropDown->Enabled = false;
	}
	
	PlayerList->Rect.x = LeaveButton->Rect.x;
	PlayerList->Rect.w = (FlyButton->Rect.x + FlyButton->Rect.w - PlayerList->Rect.x) / 2 - 5;
	PlayerList->Rect.y = ShipDropDown->Rect.y + ShipDropDown->Rect.h + 10;
	PlayerList->Rect.h = 400 - (PlayerName->Rect.h + ShipDropDown->Rect.h + 20);
	if( PlayerList->Rect.h > Rect.h / 3 )
		PlayerList->Rect.h = Rect.h / 3;
	
	MessageInput->Rect.x = PlayerList->Rect.x;
	MessageInput->Rect.w = Rect.w - (MessageList->Rect.x * 2);
	MessageInput->Rect.h = MessageInput->TextFont->GetHeight();
	MessageInput->Rect.y = FlyButton->Rect.y - MessageInput->Rect.h - MessageInput->Rect.x;
	
	MessageList->Rect.x = MessageInput->Rect.x;
	MessageList->Rect.w = MessageInput->Rect.w;
	MessageList->Rect.y = PlayerList->Rect.y + PlayerList->Rect.h + MessageList->Rect.x;
	MessageList->Rect.h = MessageInput->Rect.y - (PlayerList->Rect.y + PlayerList->Rect.h) - MessageList->Rect.x;
	
	if( MessageList->Rect.h * 2 > PlayerList->Rect.h )
	{
		int h = MessageList->Rect.h + PlayerList->Rect.h;
		MessageList->Rect.h = h / 3;
		PlayerList->Rect.h = h - MessageList->Rect.h;
		MessageList->Rect.y = PlayerList->Rect.y + PlayerList->Rect.h + MessageList->Rect.x;
	}
	
	MessageList->Selected = NULL;
	
	Layer *prev = NULL;
	double x = PlayerList->Rect.x + PlayerList->Rect.w + 10;
	double y = PlayerName->Rect.y + TitleFont->PointSize;
	double w = PlayerList->Rect.w;
	
	for( std::vector<LobbyMenuConfiguration*>::iterator config_iter = ConfigOrder.begin(); config_iter != ConfigOrder.end(); config_iter ++ )
	{
		if( *config_iter )
		{
			if( (*config_iter)->Enabled )
			{
				(*config_iter)->Rect.x = x;
				(*config_iter)->Rect.y = y;
				(*config_iter)->Rect.w = w;
				(*config_iter)->Update();
				y += (*config_iter)->Rect.h + (tiny ? 0 : 1);
				prev = *config_iter;
			}
		}
		else if( prev )
		{
			y += (tiny ? 4 : 16);
			prev = NULL;
		}
	}
	
	if( y >= MessageList->Rect.y )
	{
		MessageList->Rect.w = PlayerList->Rect.w;
		MessageInput->Rect.w = MessageList->Rect.w;
	}
	
	if( Raptor::Game->Console.IsActive() )
		Selected = NULL;
	else if( PlayerName->IsSelected() )
		;
	else
		Selected = MessageInput;
	
	if( Raptor::Game->Cfg.SettingAsInt("ui_ship_preview") == 2 )
	{
		ShipView->Rect.x = ShipView->Rect.y = 0;
		ShipView->Rect.w = Raptor::Game->Gfx.W;
		ShipView->Rect.h = Raptor::Game->Gfx.H;
	}
	else
	{
		memcpy( &(ShipView->Rect), &(PlayerList->Rect), sizeof(ShipView->Rect) );
		ShipView->Rect.w -= PlayerList->ScrollBarSize;
		if( ShipView->Rect.h > ShipView->Rect.w )
		{
			ShipView->Rect.y += ShipView->Rect.h - ShipView->Rect.w;
			ShipView->Rect.h = ShipView->Rect.w;
		}
	}
	
	UpdateCalcRects();
}


void LobbyMenu::UpdatePlayerName( void )
{
	if( ! PlayerName->IsSelected() )
	{
		const Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
		if( player )
			PlayerName->Text = player->Name;
	}
}


void LobbyMenu::UpdatePlayerList( void )
{
	std::string prev_selected = PlayerList->SelectedValue();
	PlayerList->Clear();
	
	XWingGame *game = (XWingGame*) Raptor::Game;
	std::string gametype = game->Data.PropertyAsString("gametype");
	bool ffa = (gametype.find("ffa_") == 0);
	std::string player_team = game->Data.PropertyAsString("player_team");
	if( player_team.length() )
		player_team[ 0 ] = toupper( player_team[ 0 ] );
	else if( gametype == "mission" )
	{
		std::string mission_id = game->Data.PropertyAsString("mission");
		if( Str::BeginsWith( mission_id, "rebel" ) )
			player_team = "Rebel";
		else if( Str::BeginsWith( mission_id, "empire" ) )
			player_team = "Empire";
	}
	
	for( std::map<uint16_t,Player*>::iterator player_iter = game->Data.Players.begin(); player_iter != game->Data.Players.end(); player_iter ++ )
	{
		std::string display_name = player_iter->second->Name;
		Color display_color;
		
		std::string ship = player_iter->second->PropertyAsString("ship");
		std::string team = player_team.empty() ? player_iter->second->PropertyAsString("team") : player_team;
		
		if( ship == "Rebel Gunner" )
		{
			display_name += " [Gunner]";
			team = "Rebel";
		}
		else if( ship == "Imperial Gunner" )
		{
			display_name += " [Gunner]";
			team = "Empire";
		}
		else if( ship.length() )
			display_name += " [" + ship + "]";
		else if( team.length() )
		{
			if( player_team.empty() )
				display_name += " [" + team + "]";  // Auto-Assigned
		}
		else if( ! ffa )
			display_name += " [Auto-Assign]";
		
		if( (ship.empty() && team.empty()) || ffa )
		{
			if( player_iter->first == game->PlayerID )
				display_color.Blue = 0.f;
		}
		else if( ship == "Spectator" )
		{
			if( player_iter->first == game->PlayerID )
			{
				display_color.Red = display_color.Green = 0.75f;
				display_color.Blue = 0.f;
			}
			else
				display_color.Red = display_color.Green = display_color.Blue = 0.5f;
		}
		else
		{
			if( team.empty() )
			{
				// Ship selected in a team game, but not playing yet.
				
				const ShipClass *ship_class = game->GetShipClass( ship );
				if( ship_class && (ship_class->Team == XWing::Team::REBEL) )
					team = "Rebel";
				else if( ship_class && (ship_class->Team == XWing::Team::EMPIRE) )
					team = "Empire";
				else if( ship_class ) // Classes without a team, such as CRV and FRG.
					team = "Rebel";
			}
			
			if( team.length() )
			{
				if( Str::EqualsInsensitive( team, "Rebel" ) )
				{
					display_color.Red = 1.f;
					if( player_iter->first == game->PlayerID )
					{
						display_color.Green = 0.37f;
						display_color.Blue  = 0.25f;
						if( Background.Name != "bg_lobby.ani" )
							Background.BecomeInstance( game->Res.GetAnimation("bg_lobby.ani") );
					}
					else
					{
						display_color.Green = 0.12f;
						display_color.Blue  = 0.12f;
					}
				}
				else if( Str::EqualsInsensitive( team, "Empire" ) )
				{
					display_color.Blue = 1.f;
					if( player_iter->first == game->PlayerID )
					{
						display_color.Red   = 0.f;
						display_color.Green = 0.62f;
						if( Background.Name != "bg_lobby2.ani" )
							Background.BecomeInstance( game->Res.GetAnimation("bg_lobby2.ani") );
					}
					else
					{
						display_color.Red   = 0.25f;
						display_color.Green = 0.37f;
					}
				}
				
				int group = player_iter->second->PropertyAsInt("group");
				if( group == 1 )
					display_name += " [" + team + " - Red]";
				else if( group == 2 )
					display_name += " [" + team + " - Gold]";
				else if( group == 3 )
					display_name += " [" + team + " - Green]";
				else if( group == 4 )
					display_name += " [" + team + " - Blue]";
				else if( group )
					display_name += " [" + team + " group " + Num::ToString(group) + "]";
				else
					display_name += " [" + team + "]";
			}
		}
		
		uint8_t voice_channel = player_iter->second->VoiceChannel();
		if( voice_channel )
			display_name += std::string( (voice_channel == Raptor::VoiceChannel::ALL) ? "  (:" : "  (;" )  // Team voice channel uses winking face.
			              + std::string( (((int)( game->RoundTimer.ElapsedMilliseconds() + 30. * player_iter->first ) % 200) < 100) ? "-D)" : "-I )" );
		
		PlayerList->AddItem( Num::ToString( player_iter->second->ID ), display_name, &display_color );
	}
	
	//PlayerList->Select( prev_selected );  // NOTE: Uncomment to allow selecting players in the list (maybe for a mute feature).
}


void LobbyMenu::UpdateMessageList( void )
{
	size_t msg_count = Raptor::Game->Msg.Messages.size();
	if( msg_count != MessageList->Items.size() )
	{
		std::string prev_selected = MessageList->SelectedValue();
		MessageList->Clear();
		
		Color chat_color( 1.f, 1.f, 0.f, 1.f );
		Color team_color( 0.f, 1.f, 0.f, 1.f );
		
		for( size_t i = 0; i < msg_count; i ++ )
		{
			uint32_t msg_type = Raptor::Game->Msg.Messages.at(i)->Type;
			MessageList->AddItem( Num::ToString((int)i), Raptor::Game->Msg.Messages.at(i)->Text, (msg_type == TextConsole::MSG_CHAT) ? &chat_color : ((msg_type == TextConsole::MSG_TEAM) ? &team_color : NULL) );
		}
		
		if( ! prev_selected.empty() )
			MessageList->Select( prev_selected );
		
		MessageList->Scroll = MessageList->MaxScroll();
	}
}


void LobbyMenu::UpdateInfoBoxes( void )
{
	XWingGame *game = (XWingGame*) Raptor::Game;
	
	std::string permissions = Raptor::Game->Data.PropertyAsString("permissions");
	if( permissions == "all" )
		Configs["permissions"]->Value->LabelText = "Anyone Can Change";
	else
		Configs["permissions"]->Value->LabelText = "Only Host May Change";
	
	std::string gametype = Raptor::Game->Data.PropertyAsString("gametype");
	if( gametype == "team_elim" )
		Configs["gametype"]->Value->LabelText = "Team Elimination";
	else if( gametype == "ffa_elim" )
		Configs["gametype"]->Value->LabelText = "FFA Elimination";
	else if( gametype == "team_dm" )
		Configs["gametype"]->Value->LabelText = "Team Deathmatch";
	else if( gametype == "ffa_dm" )
		Configs["gametype"]->Value->LabelText = "FFA Deathmatch";
	else if( gametype == "team_race" )
		Configs["gametype"]->Value->LabelText = "Team Kessel Run";
	else if( gametype == "ffa_race" )
		Configs["gametype"]->Value->LabelText = "FFA Kessel Run";
	else if( gametype == "ctf" )
		Configs["gametype"]->Value->LabelText = "Capture the Flag";
	else if( gametype == "yavin" )
		Configs["gametype"]->Value->LabelText = "Battle of Yavin";
	else if( gametype == "endor" )
		Configs["gametype"]->Value->LabelText = "Battle of Endor";
	else if( gametype == "hunt" )
		Configs["gametype"]->Value->LabelText = "Flagship Hunt";
	else if( gametype == "fleet" )
		Configs["gametype"]->Value->LabelText = "Fleet Battle";
	else if( gametype == "mission" )
		Configs["gametype"]->Value->LabelText = Raptor::Game->Data.PropertyAsString("campaign").length() ? "Campaign" : "Mission";
	else
		Configs["gametype"]->Value->LabelText = gametype;
	
	Configs["mission"]->Value->LabelText = Raptor::Game->Data.PropertyAsString( "mission_name", Raptor::Game->Data.PropertyAsString("mission","Select Mission").c_str() );
	Configs["mission"]->Value->Blue = Str::EndsWith( Raptor::Game->Data.PropertyAsString("mission"), "*" ) ? 0.f : 1.f;
	
	if( (gametype == "mission") && ShipDropDown->Value.length() )
	{
		std::vector<std::string> allowed_ships;
		std::string player_ships = Raptor::Game->Data.PropertyAsString("player_ships");
		if( player_ships.empty() )
			player_ships = Raptor::Game->Data.PropertyAsString("player_ship");
		if( ! player_ships.empty() )
		{
			allowed_ships = Str::SplitToVector( player_ships, " " );
			if( std::find( allowed_ships.begin(), allowed_ships.end(), "rebel_gunner" ) != allowed_ships.end() )
				allowed_ships.push_back( "Rebel Gunner" );
			if( std::find( allowed_ships.begin(), allowed_ships.end(), "empire_gunner" ) != allowed_ships.end() )
				allowed_ships.push_back( "Imperial Gunner" );
			if( (game->Data.Players.size() > 1) && ! Raptor::Server->IsRunning() )
				allowed_ships.push_back( "Spectator" );
		}
		
		// Switch to Auto-Assign if the selected ship isn't allowed on the current mission.
		if( allowed_ships.size() && (std::find( allowed_ships.begin(), allowed_ships.end(), ShipDropDown->Value ) == allowed_ships.end())
		&& ! ((Raptor::Game->Cfg.SettingAsBool("darkside",false) || Raptor::Game->Data.PropertyAsBool("darkside",false)) && ! Raptor::Game->Data.PropertyAsBool("lightside",false)) )
			ShipDropDown->Select("");
	}
	
	if( gametype == "yavin" )
	{
		Configs["customize_fleet"]->Title->LabelText = Configs["customize_fleet"]->TitleShadow->LabelText = "Yavin Ships";
		if( (Raptor::Game->Data.PropertyAsString("yavin_rebel_fighter")  == "X/W")
		&&  (Raptor::Game->Data.PropertyAsString("yavin_rebel_bomber")   == "Y/W")
		&&  (Raptor::Game->Data.PropertyAsString("yavin_empire_fighter") == "T/F") )
			Configs["customize_fleet"]->Value->LabelText = "Classic";
		else
		{
			Configs["customize_fleet"]->Value->LabelText = "Altered";
			const ShipClass *rebel_fighter  = game->GetShipClass( Raptor::Game->Data.PropertyAsString("yavin_rebel_fighter") );
			const ShipClass *empire_fighter = game->GetShipClass( Raptor::Game->Data.PropertyAsString("yavin_empire_fighter") );
			const ShipClass *rebel_bomber   = game->GetShipClass( Raptor::Game->Data.PropertyAsString("yavin_rebel_fighter") );
			if(!( rebel_fighter  && (rebel_fighter->Team  != XWing::Team::EMPIRE) && rebel_fighter->PlayersCanFly()
			&&    empire_fighter && (empire_fighter->Team != XWing::Team::REBEL)  && empire_fighter->PlayersCanFly()
			&&    rebel_bomber   && (rebel_bomber->Team   != XWing::Team::EMPIRE) && rebel_bomber->PlayersCanFly() ))
				Configs["customize_fleet"]->Value->LabelText = "Disturbing";
			else if( (rebel_fighter->Category  == ShipClass::CATEGORY_BOMBER)
			&&       (empire_fighter->Category == ShipClass::CATEGORY_BOMBER)
			&&       (rebel_bomber->Category   == ShipClass::CATEGORY_BOMBER) )
				Configs["customize_fleet"]->Value->LabelText = "Oops! All Bombers";
		}
	}
	else
	{
		Configs["customize_fleet"]->Title->LabelText = Configs["customize_fleet"]->TitleShadow->LabelText = "Fleet Ships";
		if( (Raptor::Game->Data.PropertyAsString("rebel_fighter")   == "X/W")
		&& ((Raptor::Game->Data.PropertyAsString("rebel_bomber")    == "Y/W") || ((gametype != "fleet") && (gametype != "hunt")))
		&& ((Raptor::Game->Data.PropertyAsString("rebel_cruiser")   == "CRV") ||  (gametype != "fleet"))
		&& ((Raptor::Game->Data.PropertyAsString("rebel_frigate")   == "FRG") ||  (gametype != "fleet"))
		&& ((Raptor::Game->Data.PropertyAsString("rebel_flagship")  == "FRG") || ((gametype != "fleet") && (gametype != "hunt")))
		&&  (Raptor::Game->Data.PropertyAsString("empire_fighter")  == "T/F")
		&& ((Raptor::Game->Data.PropertyAsString("empire_bomber")   == "T/B") || ((gametype != "fleet") && (gametype != "hunt")))
		&& ((Raptor::Game->Data.PropertyAsString("empire_cruiser")  == "INT") ||  (gametype != "fleet"))
		&& ((Raptor::Game->Data.PropertyAsString("empire_frigate")  == "VSD") ||  (gametype != "fleet"))
		&& ((Raptor::Game->Data.PropertyAsString("empire_flagship") == "ISD") || ((gametype != "fleet") && (gametype != "hunt"))) )
		{
			// NOTE: Could show different text based on gametype.
			Configs["customize_fleet"]->Value->LabelText = "Classic";
		}
		else
		{
			Configs["customize_fleet"]->Value->LabelText = "Altered";
			const ShipClass *rebel_fighter  = game->GetShipClass( Raptor::Game->Data.PropertyAsString("rebel_fighter") );
			const ShipClass *empire_fighter = game->GetShipClass( Raptor::Game->Data.PropertyAsString("empire_fighter") );
			bool ffa = (Str::FindInsensitive( gametype, "ffa_" ) == 0);
			if(!( rebel_fighter  && ((rebel_fighter->Team  != XWing::Team::EMPIRE) || ffa) && rebel_fighter->PlayersCanFly()
			&&    empire_fighter && ((empire_fighter->Team != XWing::Team::REBEL)  || ffa) && empire_fighter->PlayersCanFly() ))
				Configs["customize_fleet"]->Value->LabelText = "Disturbing";
			else if( (gametype == "fleet") || (gametype == "hunt") )
			{
				const ShipClass *rebel_bomber    = game->GetShipClass( Raptor::Game->Data.PropertyAsString("rebel_bomber") );
				const ShipClass *empire_bomber   = game->GetShipClass( Raptor::Game->Data.PropertyAsString("empire_bomber") );
				const ShipClass *rebel_flagship  = game->GetShipClass( Raptor::Game->Data.PropertyAsString("rebel_flagship") );
				const ShipClass *empire_flagship = game->GetShipClass( Raptor::Game->Data.PropertyAsString("empire_flagship") );
				if(!( rebel_bomber    && (rebel_bomber->Team    != XWing::Team::EMPIRE) && (rebel_bomber->Category    != ShipClass::CATEGORY_TARGET) && (! rebel_bomber->Secret)
				&&    empire_bomber   && (empire_bomber->Team   != XWing::Team::REBEL)  && (empire_bomber->Category   != ShipClass::CATEGORY_TARGET) && (! empire_bomber->Secret)
				&&    rebel_flagship  && (rebel_flagship->Team  != XWing::Team::EMPIRE) && (rebel_flagship->Category  != ShipClass::CATEGORY_TARGET) && (! rebel_flagship->Secret)
				&&    empire_flagship && (empire_flagship->Team != XWing::Team::REBEL)  && (empire_flagship->Category != ShipClass::CATEGORY_TARGET) && (! empire_flagship->Secret) ))
					Configs["customize_fleet"]->Value->LabelText = "Disturbing";
				else if( gametype == "fleet" )
				{
					const ShipClass *rebel_cruiser  = game->GetShipClass( Raptor::Game->Data.PropertyAsString("rebel_cruiser") );
					const ShipClass *empire_cruiser = game->GetShipClass( Raptor::Game->Data.PropertyAsString("empire_cruiser") );
					const ShipClass *rebel_frigate  = game->GetShipClass( Raptor::Game->Data.PropertyAsString("rebel_frigate") );
					const ShipClass *empire_frigate = game->GetShipClass( Raptor::Game->Data.PropertyAsString("empire_frigate") );
					if(!( rebel_cruiser  && (rebel_cruiser->Team  != XWing::Team::EMPIRE) && (rebel_cruiser->Category  != ShipClass::CATEGORY_TARGET) && (! rebel_cruiser->Secret)
					&&    empire_cruiser && (empire_cruiser->Team != XWing::Team::REBEL)  && (empire_cruiser->Category != ShipClass::CATEGORY_TARGET) && (! empire_cruiser->Secret)
					&&    rebel_frigate  && (rebel_frigate->Team  != XWing::Team::EMPIRE) && (rebel_frigate->Category  != ShipClass::CATEGORY_TARGET) && (! rebel_frigate->Secret)
					&&    empire_frigate && (empire_frigate->Team != XWing::Team::REBEL)  && (empire_frigate->Category != ShipClass::CATEGORY_TARGET) && (! empire_frigate->Secret) ))
						Configs["customize_fleet"]->Value->LabelText = "Disturbing";
				}
				if( (Configs["customize_fleet"]->Value->LabelText != "Disturbing") 
				&&  (rebel_fighter->Category  == ShipClass::CATEGORY_BOMBER)
				&&  (empire_fighter->Category == ShipClass::CATEGORY_BOMBER)
				&&  (rebel_bomber->Category   == ShipClass::CATEGORY_BOMBER)
				&&  (empire_bomber->Category  == ShipClass::CATEGORY_BOMBER) )
					Configs["customize_fleet"]->Value->LabelText = "Oops! All Bombers";
			}
			else if( (rebel_fighter->Category  == ShipClass::CATEGORY_BOMBER)
			&&       (empire_fighter->Category == ShipClass::CATEGORY_BOMBER) )
				Configs["customize_fleet"]->Value->LabelText = "Oops! All Bombers";
		}
	}
	
	Configs["race_circuit"]->Value->LabelText = Raptor::Game->Data.PropertyAsBool("race_circuit") ? "Classic Circuit" : "First Touch";
	
	int race_lap = Raptor::Game->Data.PropertyAsInt("race_lap");
	Configs["race_lap"]->Value->LabelText = race_lap ? (Num::ToString(race_lap) + std::string(" Checkpoints")) : "Random Scatter";
	
	int team_rate_checkpoints = Raptor::Game->Data.PropertyAsInt("team_race_checkpoints");
	Configs["team_race_checkpoints"]->Value->LabelText = team_rate_checkpoints ? (Num::ToString(team_rate_checkpoints) + std::string(" Checkpoints")) : "Unlimited";
	
	int ffa_race_checkpoints = Raptor::Game->Data.PropertyAsInt("ffa_race_checkpoints");
	Configs["ffa_race_checkpoints"]->Value->LabelText = ffa_race_checkpoints ? (Num::ToString(ffa_race_checkpoints) + std::string(" Checkpoints")) : "Unlimited";
	
	int race_time_limit = Raptor::Game->Data.PropertyAsInt("race_time_limit");
	Configs["race_time_limit"]->Value->LabelText = race_time_limit ? (Num::ToString(race_time_limit) + std::string(" min")) : "Unlimited";
	
	int tdm_kill_limit = Raptor::Game->Data.PropertyAsInt("tdm_kill_limit");
	Configs["tdm_kill_limit"]->Value->LabelText = tdm_kill_limit ? Num::ToString(tdm_kill_limit) : "Unlimited";
	
	int dm_kill_limit = Raptor::Game->Data.PropertyAsInt("dm_kill_limit");
	Configs["dm_kill_limit"]->Value->LabelText = dm_kill_limit ? Num::ToString(dm_kill_limit) : "Unlimited";
	
	int ai_waves = Raptor::Game->Data.PropertyAsInt("ai_waves");
	if( ai_waves <= 0 )
		Configs["ai_waves"]->Value->LabelText = "None";
	else if( ai_waves == 1 )
		Configs["ai_waves"]->Value->LabelText = "Very Few";
	else if( ai_waves == 2 )
		Configs["ai_waves"]->Value->LabelText = "Few";
	else if( ai_waves == 3 )
		Configs["ai_waves"]->Value->LabelText = "Some";
	else if( ai_waves == 4 )
		Configs["ai_waves"]->Value->LabelText = "Many";
	else if( ai_waves == 5 )
		Configs["ai_waves"]->Value->LabelText = "Very Many";
	else if( ai_waves == 6 )
		Configs["ai_waves"]->Value->LabelText = "Holy Crap, Tons of Them!";
	else if( ai_waves == 7 )
		Configs["ai_waves"]->Value->LabelText = "Borderline Ridiculous";
	else if( ai_waves == 8 )
		Configs["ai_waves"]->Value->LabelText = "Undoubtedly Ridiculous";
	else if( ai_waves == 9 )
		Configs["ai_waves"]->Value->LabelText = "Beyond Ridiculous";
	else
		Configs["ai_waves"]->Value->LabelText = "The Whole Friggin' Empire";
	
	int ai_skill = Raptor::Game->Data.PropertyAsInt("ai_skill",1);
	if( ai_skill < 0 )
	{
		Configs["ai_skill"]->Value->LabelText = "Bantha Fodder";
		Configs["ai_skill"]->Value->Red   = 0.75f;
		Configs["ai_skill"]->Value->Green = 0.5f;
		Configs["ai_skill"]->Value->Blue  = 0.25f;
	}
	else if( ai_skill == 0 )
	{
		Configs["ai_skill"]->Value->LabelText = "Rookie";
		Configs["ai_skill"]->Value->Red = Configs["ai_skill"]->Value->Blue = 0.75f;
		Configs["ai_skill"]->Value->Green = 1.f;
	}
	else if( ai_skill == 1 )
	{
		Configs["ai_skill"]->Value->LabelText = "Veteran";
		Configs["ai_skill"]->Value->Red = Configs["ai_skill"]->Value->Green = Configs["ai_skill"]->Value->Blue = 1.f;
	}
	else if( ai_skill == 2 )
	{
		Configs["ai_skill"]->Value->LabelText = "Ace";
		Configs["ai_skill"]->Value->Red = Configs["ai_skill"]->Value->Green = 1.f;
		Configs["ai_skill"]->Value->Blue = 0.25f;
	}
	else if( ai_skill == 3 )
	{
		Configs["ai_skill"]->Value->LabelText = "Jedi Knight";
		Configs["ai_skill"]->Value->Red = 1.f;
		Configs["ai_skill"]->Value->Green = Configs["ai_skill"]->Value->Blue = 0.5f;
	}
	else if( ai_skill >= 4 )
	{
		Configs["ai_skill"]->Value->LabelText = "Jedi Master";
		Configs["ai_skill"]->Value->Red = 1.f;
		Configs["ai_skill"]->Value->Green = Configs["ai_skill"]->Value->Blue = 0.125f;
	}
	
	Configs["rebel_cruisers"]->Value->LabelText  = Raptor::Game->Data.PropertyAsString("rebel_cruisers", "0");
	Configs["empire_cruisers"]->Value->LabelText = Raptor::Game->Data.PropertyAsString("empire_cruisers","0");
	Configs["rebel_frigates"]->Value->LabelText  = Raptor::Game->Data.PropertyAsString("rebel_frigates", "0");
	Configs["empire_frigates"]->Value->LabelText = Raptor::Game->Data.PropertyAsString("empire_frigates","0");
	
	std::string respawn = Raptor::Game->Data.PropertyAsString("respawn");
	Configs["respawn"]->Value->Blue = 1.f;
	if( gametype == "team_elim" )
	{
		Configs["respawn"]->Title->LabelText = "Dead Players";
		Configs["respawn"]->TitleShadow->LabelText = "Dead Players";
		
		if( respawn == "true" )
			Configs["respawn"]->Value->LabelText = "Control AI Ships";
		else if( respawn == "false" )
		{
			Configs["respawn"]->Value->LabelText = "Observe Only";
			Configs["respawn"]->Value->Blue = 0.25f;
		}
		else
			Configs["respawn"]->Value->LabelText = respawn;
	}
	else
	{
		Configs["respawn"]->Title->LabelText = "Respawn";
		Configs["respawn"]->TitleShadow->LabelText = "Respawn";
		
		if( respawn == "true" )
			Configs["respawn"]->Value->LabelText = "Yes";
		else if( respawn == "false" )
		{
			Configs["respawn"]->Value->LabelText = "No";
			Configs["respawn"]->Value->Blue = 0.25f;
		}
		else
			Configs["respawn"]->Value->LabelText = respawn;
		
		int respawn_time = Raptor::Game->Data.PropertyAsInt("respawn_time");
		Configs["respawn_time"]->Value->LabelText = respawn_time ? (Num::ToString(respawn_time) + std::string(" Seconds")) : std::string("Default");
	}
	
	Configs["allow_team_change"]->Value->LabelText = Raptor::Game->Data.PropertyAsBool("allow_team_change") ? "Yes" : "No";
	
	int asteroids = Raptor::Game->Data.PropertyAsInt("asteroids");
	if( asteroids <= 0 )
		Configs["asteroids"]->Value->LabelText = "None";
	else if( asteroids <= 8 )
		Configs["asteroids"]->Value->LabelText = "Very Few";
	else if( asteroids <= 16 )
		Configs["asteroids"]->Value->LabelText = "Few";
	else if( asteroids <= 32 )
		Configs["asteroids"]->Value->LabelText = "Some";
	else if( asteroids <= 64 )
		Configs["asteroids"]->Value->LabelText = "Many";
	else if( asteroids <= 128 )
		Configs["asteroids"]->Value->LabelText = "Very Many";
	else if( asteroids <= 256 )
		Configs["asteroids"]->Value->LabelText = "Uncomfortably Many";
	else if( asteroids <= 512 )
		Configs["asteroids"]->Value->LabelText = "Insanely Many";
	else if( asteroids <= 1024 )
		Configs["asteroids"]->Value->LabelText = "Hardly Worth Playing";
	else
		Configs["asteroids"]->Value->LabelText = "Not Worth Playing";
	
	int yavin_time_limit = Raptor::Game->Data.PropertyAsInt("yavin_time_limit");
	Configs["yavin_time_limit"]->Value->LabelText = yavin_time_limit ? (Num::ToString(yavin_time_limit) + std::string(" min")) : "Unlimited";
	
	int yavin_turrets = Raptor::Game->Data.PropertyAsInt("yavin_turrets");
	if( yavin_turrets <= 0 )
		Configs["yavin_turrets"]->Value->LabelText = "None";
	else if( yavin_turrets <= 60 )
		Configs["yavin_turrets"]->Value->LabelText = "Minimal";
	else if( yavin_turrets <= 90 )
		Configs["yavin_turrets"]->Value->LabelText = "Reduced";
	else if( yavin_turrets <= 120 )
		Configs["yavin_turrets"]->Value->LabelText = "Normal";
	else if( yavin_turrets <= 150 )
		Configs["yavin_turrets"]->Value->LabelText = "Extra";
	else if( yavin_turrets <= 180 )
		Configs["yavin_turrets"]->Value->LabelText = "Plentiful";
	else
		Configs["yavin_turrets"]->Value->LabelText = "Way Too Many";
	
	int hunt_time_limit = Raptor::Game->Data.PropertyAsInt("hunt_time_limit");
	Configs["hunt_time_limit"]->Value->LabelText = hunt_time_limit ? (Num::ToString(hunt_time_limit) + std::string(" min")) : "Unlimited";
	
	std::string defending_team = Raptor::Game->Data.PropertyAsString("defending_team");
	if( defending_team == "empire" )
		Configs["defending_team"]->Value->LabelText = "Empire";
	else if( defending_team == "rebel" )
		Configs["defending_team"]->Value->LabelText = "Rebels";
	else
		Configs["defending_team"]->Value->LabelText = defending_team;
	
	std::string bg = Raptor::Game->Data.PropertyAsString("bg");
	if( bg == "stars" )
		Configs["bg"]->Value->LabelText = "Deep Space";
	else if( bg == "nebula" )
		Configs["bg"]->Value->LabelText = "Red Nebula";
	else if( bg == "nebula2" )
		Configs["bg"]->Value->LabelText = "Blue Nebula";
	else
		Configs["bg"]->Value->LabelText = bg;
	
	// Determine player permissions.
	const Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	bool admin = (player && player->PropertyAsBool("admin"));
	bool permissions_all = (Raptor::Game->Data.PropertyAsString("permissions") == "all");
	bool flying = false;
	for( std::map<uint32_t,GameObject*>::const_iterator obj = Raptor::Game->Data.GameObjects.begin(); obj != Raptor::Game->Data.GameObjects.end(); obj ++ )
	{
		if( obj->second->Type() != XWing::Object::SHIP_CLASS )
		{
			flying = true;
			break;
		}
	}
	
	// Show "Fly" button for admin or late joiners.
	FlyButton->Enabled = (admin || permissions_all || flying);
	
	// Show "Change" buttons for admin.
	FlyButton->Visible = FlyButton->Enabled;
	for( std::map<std::string,LobbyMenuConfiguration*>::iterator config_iter = Configs.begin(); config_iter != Configs.end(); config_iter ++ )
		config_iter->second->ShowButton = (admin || permissions_all) && ! flying;
	Configs["permissions"]->ShowButton = admin;
	Configs["customize_fleet"]->ShowButton = true;

	// Show "Reset to Defaults" button for admin.
	if( (admin || permissions_all) && ((gametype != "mission") /* || ! Raptor::Game->Data.PropertyAsString("mission_opts").empty() */) )  // FIXME: Disabled for mission_ops because it gets wrong defaults!
	{
		Configs["defaults"]->Value->LabelText = "Reset to Defaults";
		Configs["defaults"]->ShowButton = true;
		Configs["defaults"]->Visible = true;
		Configs["defaults"]->Enabled = true;
	}
	else
	{
		Configs["defaults"]->Value->LabelText = "";
		Configs["defaults"]->ShowButton = false;
		Configs["defaults"]->Visible = false;
		Configs["defaults"]->Enabled = false;
	}
	
	// Show "Respawn" unless it's Deathmatch or FFA Elimination.
	if( (gametype == "team_dm") || (gametype == "ffa_dm") || (gametype == "ffa_elim") )
	{
		Configs["respawn"]->ShowButton = false;
		Configs["respawn"]->Visible = false;
		Configs["respawn"]->Enabled = false;
	}
	else
	{
		Configs["respawn"]->Visible = true;
		Configs["respawn"]->Enabled = true;
	}
	
	// Show "Allow Team Change" unless it's FFA.
	if( Str::BeginsWith( gametype, "ffa_" ) )
	{
		Configs["allow_team_change"]->ShowButton = false;
		Configs["allow_team_change"]->Visible = false;
		Configs["allow_team_change"]->Enabled = false;
	}
	else
	{
		Configs["allow_team_change"]->Visible = true;
		Configs["allow_team_change"]->Enabled = true;
	}
	
	// Hide "Team Kill Limit" unless it's Team Deathmatch.
	if( gametype != "team_dm" )
	{
		Configs["tdm_kill_limit"]->ShowButton = false;
		Configs["tdm_kill_limit"]->Visible = false;
		Configs["tdm_kill_limit"]->Enabled = false;
	}
	else
	{
		Configs["tdm_kill_limit"]->Visible = true;
		Configs["tdm_kill_limit"]->Enabled = true;
	}
	
	// Hide "Kill Limit" unless it's FFA Deathmatch.
	if( gametype != "ffa_dm" )
	{
		Configs["dm_kill_limit"]->ShowButton = false;
		Configs["dm_kill_limit"]->Visible = false;
		Configs["dm_kill_limit"]->Enabled = false;
	}
	else
	{
		Configs["dm_kill_limit"]->Visible = true;
		Configs["dm_kill_limit"]->Enabled = true;
	}
	
	// Hide "Asteroids" if Battle of Yavin and show "Time Limit" and "Surface Turrets" instead.
	if( gametype == "yavin" )
	{
		Configs["asteroids"]->ShowButton = false;
		Configs["asteroids"]->Visible = false;
		Configs["asteroids"]->Enabled = false;
		
		Configs["yavin_time_limit"]->Visible = true;
		Configs["yavin_time_limit"]->Enabled = true;
		Configs["yavin_turrets"]->Visible = true;
		Configs["yavin_turrets"]->Enabled = true;
	}
	else
	{
		Configs["asteroids"]->Visible = true;
		Configs["asteroids"]->Enabled = true;
		
		Configs["yavin_time_limit"]->ShowButton = false;
		Configs["yavin_time_limit"]->Visible = false;
		Configs["yavin_time_limit"]->Enabled = false;
		Configs["yavin_turrets"]->ShowButton = false;
		Configs["yavin_turrets"]->Visible = false;
		Configs["yavin_turrets"]->Enabled = false;
	}
	
	// Show "Defending Team" and "Time Limit" for Hunt mode.
	if( gametype == "hunt" )
	{
		Configs["defending_team"]->Visible = true;
		Configs["defending_team"]->Enabled = true;
		
		Configs["hunt_time_limit"]->Visible = true;
		Configs["hunt_time_limit"]->Enabled = true;
	}
	else
	{
		Configs["defending_team"]->ShowButton = false;
		Configs["defending_team"]->Visible = false;
		Configs["defending_team"]->Enabled = false;
		
		Configs["hunt_time_limit"]->ShowButton = false;
		Configs["hunt_time_limit"]->Visible = false;
		Configs["hunt_time_limit"]->Enabled = false;
	}
	
	// Show "Rebel Cruisers" and "Empire Cruisers" for Fleet Battle mode.
	if( gametype == "fleet" )
	{
		Configs["rebel_cruisers"]->Visible = true;
		Configs["rebel_cruisers"]->Enabled = true;
		
		Configs["empire_cruisers"]->Visible = true;
		Configs["empire_cruisers"]->Enabled = true;
		
		Configs["rebel_frigates"]->Visible = true;
		Configs["rebel_frigates"]->Enabled = true;
		
		Configs["empire_frigates"]->Visible = true;
		Configs["empire_frigates"]->Enabled = true;
	}
	else
	{
		Configs["rebel_cruisers"]->ShowButton = false;
		Configs["rebel_cruisers"]->Visible = false;
		Configs["rebel_cruisers"]->Enabled = false;
		
		Configs["empire_cruisers"]->ShowButton = false;
		Configs["empire_cruisers"]->Visible = false;
		Configs["empire_cruisers"]->Enabled = false;
		
		Configs["rebel_frigates"]->ShowButton = false;
		Configs["rebel_frigates"]->Visible = false;
		Configs["rebel_frigates"]->Enabled = false;
		
		Configs["empire_frigates"]->ShowButton = false;
		Configs["empire_frigates"]->Visible = false;
		Configs["empire_frigates"]->Enabled = false;
	}
	
	// Only show Race settings for Kessel Run.
	if( gametype == "team_race" )
	{
		Configs["race_circuit"]->Visible = true;
		Configs["race_circuit"]->Enabled = true;
		
		Configs["race_lap"]->Visible = true;
		Configs["race_lap"]->Enabled = true;
		
		Configs["team_race_checkpoints"]->Visible = true;
		Configs["team_race_checkpoints"]->Enabled = true;
		
		Configs["ffa_race_checkpoints"]->ShowButton = false;
		Configs["ffa_race_checkpoints"]->Visible = false;
		Configs["ffa_race_checkpoints"]->Enabled = false;
		
		Configs["race_time_limit"]->Visible = true;
		Configs["race_time_limit"]->Enabled = true;
	}
	else if( gametype == "ffa_race" )
	{
		Configs["race_circuit"]->Visible = true;
		Configs["race_circuit"]->Enabled = true;
		
		Configs["race_lap"]->Visible = true;
		Configs["race_lap"]->Enabled = true;
		
		Configs["team_race_checkpoints"]->ShowButton = false;
		Configs["team_race_checkpoints"]->Visible = false;
		Configs["team_race_checkpoints"]->Enabled = false;
		
		Configs["ffa_race_checkpoints"]->Visible = true;
		Configs["ffa_race_checkpoints"]->Enabled = true;
		
		Configs["race_time_limit"]->Visible = true;
		Configs["race_time_limit"]->Enabled = true;
	}
	else
	{
		Configs["race_circuit"]->ShowButton = false;
		Configs["race_circuit"]->Visible = false;
		Configs["race_circuit"]->Enabled = false;
		
		Configs["race_lap"]->ShowButton = false;
		Configs["race_lap"]->Visible = false;
		Configs["race_lap"]->Enabled = false;
		
		Configs["team_race_checkpoints"]->ShowButton = false;
		Configs["team_race_checkpoints"]->Visible = false;
		Configs["team_race_checkpoints"]->Enabled = false;
		
		Configs["ffa_race_checkpoints"]->ShowButton = false;
		Configs["ffa_race_checkpoints"]->Visible = false;
		Configs["ffa_race_checkpoints"]->Enabled = false;
		
		Configs["race_time_limit"]->ShowButton = false;
		Configs["race_time_limit"]->Visible = false;
		Configs["race_time_limit"]->Enabled = false;
	}
	
	if( (gametype == "team_dm") || (gametype == "ffa_dm") )
	{
		Configs["respawn_time"]->ShowButton = true;
		Configs["respawn_time"]->Visible = true;
		Configs["respawn_time"]->Enabled = true;
	}
	else if( (gametype == "team_elim") || (gametype == "ffa_elim") || (respawn == "false") )
	{
		Configs["respawn_time"]->ShowButton = false;
		Configs["respawn_time"]->Visible = false;
		Configs["respawn_time"]->Enabled = false;
	}
	else
	{
		Configs["respawn_time"]->ShowButton = Configs["respawn"]->ShowButton;
		Configs["respawn_time"]->Visible = Configs["respawn"]->Visible;
		Configs["respawn_time"]->Enabled = Configs["respawn"]->Enabled;
	}
	
	// Missions hide most settings by default.
	if( gametype == "mission" )
	{
		Configs["mission"]->Visible = true;
		Configs["mission"]->Enabled = true;
		
		bool allow_upload = (admin || permissions_all) && (Raptor::Game->Cfg.SettingAsBool("debug") || ! Raptor::Server->IsRunning());
		// FIXME: Check if there are any custom missions to upload.
		Configs["upload_mission"]->ShowButton = allow_upload;
		Configs["upload_mission"]->Visible = allow_upload;
		Configs["upload_mission"]->Enabled = allow_upload;
		Configs["upload_mission"]->Value->LabelText = "Upload Custom Mission";
		
		Configs["respawn"]->ShowButton = false;
		Configs["respawn"]->Visible = false;
		Configs["respawn"]->Enabled = false;
		Configs["respawn_time"]->ShowButton = false;
		Configs["respawn_time"]->Visible = false;
		Configs["respawn_time"]->Enabled = false;
		Configs["allow_team_change"]->ShowButton = false;
		Configs["allow_team_change"]->Visible = false;
		Configs["allow_team_change"]->Enabled = false;
		Configs["customize_fleet"]->ShowButton = false;
		Configs["customize_fleet"]->Visible = false;
		Configs["customize_fleet"]->Enabled = false;
		Configs["ai_waves"]->ShowButton = false;
		Configs["ai_waves"]->Visible = false;
		Configs["ai_waves"]->Enabled = false;
		Configs["asteroids"]->ShowButton = false;
		Configs["asteroids"]->Visible = false;
		Configs["asteroids"]->Enabled = false;
		Configs["bg"]->ShowButton = false;
		Configs["bg"]->Visible = false;
		Configs["bg"]->Enabled = false;
		
		Configs["dm_kill_limit"]->ShowButton = false;
		Configs["dm_kill_limit"]->Visible = false;
		Configs["dm_kill_limit"]->Enabled = false;
		Configs["tdm_kill_limit"]->ShowButton = false;
		Configs["tdm_kill_limit"]->Visible = false;
		Configs["tdm_kill_limit"]->Enabled = false;
		
		Configs["yavin_time_limit"]->ShowButton = false;
		Configs["yavin_time_limit"]->Visible = false;
		Configs["yavin_time_limit"]->Enabled = false;
		Configs["yavin_turrets"]->ShowButton = false;
		Configs["yavin_turrets"]->Visible = false;
		Configs["yavin_turrets"]->Enabled = false;
		
		Configs["defending_team"]->ShowButton = false;
		Configs["defending_team"]->Visible = false;
		Configs["defending_team"]->Enabled = false;
		Configs["hunt_time_limit"]->ShowButton = false;
		Configs["hunt_time_limit"]->Visible = false;
		Configs["hunt_time_limit"]->Enabled = false;
		
		Configs["rebel_cruisers"]->ShowButton = false;
		Configs["rebel_cruisers"]->Visible = false;
		Configs["rebel_cruisers"]->Enabled = false;
		Configs["empire_cruisers"]->ShowButton = false;
		Configs["empire_cruisers"]->Visible = false;
		Configs["empire_cruisers"]->Enabled = false;
		Configs["rebel_frigates"]->ShowButton = false;
		Configs["rebel_frigates"]->Visible = false;
		Configs["rebel_frigates"]->Enabled = false;
		Configs["empire_frigates"]->ShowButton = false;
		Configs["empire_frigates"]->Visible = false;
		Configs["empire_frigates"]->Enabled = false;
		
		Configs["race_circuit"]->ShowButton = false;
		Configs["race_circuit"]->Visible = false;
		Configs["race_circuit"]->Enabled = false;
		Configs["race_lap"]->ShowButton = false;
		Configs["race_lap"]->Visible = false;
		Configs["race_lap"]->Enabled = false;
		Configs["team_race_checkpoints"]->ShowButton = false;
		Configs["team_race_checkpoints"]->Visible = false;
		Configs["team_race_checkpoints"]->Enabled = false;
		Configs["ffa_race_checkpoints"]->ShowButton = false;
		Configs["ffa_race_checkpoints"]->Visible = false;
		Configs["ffa_race_checkpoints"]->Enabled = false;
		Configs["race_time_limit"]->ShowButton = false;
		Configs["race_time_limit"]->Visible = false;
		Configs["race_time_limit"]->Enabled = false;
		
		// Missions may allow customization by defining "mission_opts".
		std::vector<std::string> opts = Str::SplitToVector( Raptor::Game->Data.PropertyAsString("mission_opts"), " ," );
		for( std::vector<std::string>::const_iterator opt_iter = opts.begin(); opt_iter != opts.end(); opt_iter ++ )
		{
			std::map<std::string,LobbyMenuConfiguration*>::iterator config_iter = Configs.find( *opt_iter );
			if( config_iter != Configs.end() )
			{
				config_iter->second->ShowButton = (admin || permissions_all) && ! flying;
				config_iter->second->Visible = true;
				config_iter->second->Enabled = true;
			}
		}
	}
	else
	{
		Configs["mission"]->ShowButton = false;
		Configs["mission"]->Visible = false;
		Configs["mission"]->Enabled = false;
		
		Configs["upload_mission"]->ShowButton = false;
		Configs["upload_mission"]->Visible = false;
		Configs["upload_mission"]->Enabled = false;
		
		Configs["customize_fleet"]->Visible = true;
		Configs["customize_fleet"]->Enabled = true;
		Configs["ai_waves"]->Visible = true;
		Configs["ai_waves"]->Enabled = true;
		Configs["bg"]->Visible = true;
		Configs["bg"]->Enabled = true;
	}
}


void LobbyMenu::Draw( void )
{
	UpdateRects();
	UpdateInfoBoxes();
	UpdatePlayerName();
	UpdatePlayerList();
	UpdateMessageList();
	
	if( Raptor::Game->Head.VR && Raptor::Game->Gfx.DrawTo )
	{
		// In VR, show 3D background behind the menu.
		glPushMatrix();
		Pos3D origin;
		Raptor::Game->Cam.Copy( &origin );
		Raptor::Game->Cam.FOV = Raptor::Game->Cfg.SettingAsDouble("vr_fov");
		Raptor::Game->Gfx.Setup3D( &(Raptor::Game->Cam) );
		Animation bg;
		bg.BecomeInstance( Raptor::Game->Res.GetAnimation("stars.ani") );
		double bg_dist = Raptor::Game->Cfg.SettingAsDouble( "g_bg_dist", std::min<double>( 50000., Raptor::Game->Gfx.ZFar * 0.875 ) );
		Raptor::Game->Gfx.DrawSphere3D( 0,0,0, bg_dist, 32, bg.CurrentFrame(), Graphics::TEXTURE_MODE_Y_ASIN );
		glPopMatrix();
	}
	else
		Raptor::Game->Gfx.DrawRect2D( Rect.w / 2 - Rect.h, 0, Rect.w / 2 + Rect.h, Rect.h, Background.CurrentFrame(), 1.f, 1.f, 1.f, 1.f );
	
	Layer::Draw();
	
	bool flying = false;
	for( std::map<uint32_t,GameObject*>::const_iterator obj = Raptor::Game->Data.GameObjects.begin(); obj != Raptor::Game->Data.GameObjects.end(); obj ++ )
	{
		if( obj->second->Type() != XWing::Object::SHIP_CLASS )
		{
			flying = true;
			break;
		}
	}
	std::string title = flying ? "Game-In-Progress Lobby" : "Pre-Game Lobby";
	
	TitleFont->DrawText( title, Rect.w/2 + 2, 12, Font::ALIGN_TOP_CENTER, 0.f,0.f,0.f,0.8f );
	TitleFont->DrawText( title, Rect.w/2, 10, Font::ALIGN_TOP_CENTER );
	
	TitleFont->DrawText( "Game Settings", PlayerList->Rect.x + PlayerList->Rect.w*1.4 + 12, PlayerName->Rect.y + 2, Font::ALIGN_TOP_CENTER, 0.f,0.f,0.f,0.8f );
	TitleFont->DrawText( "Game Settings", PlayerList->Rect.x + PlayerList->Rect.w*1.4 + 10, PlayerName->Rect.y, Font::ALIGN_TOP_CENTER );
}


bool LobbyMenu::HandleEvent( SDL_Event *event )
{
	if( (event->type == SDL_JOYBUTTONDOWN)
	&&  Raptor::Game->Cfg.SettingAsBool("joy_enable")
	&&  (Str::FindInsensitive( Raptor::Game->Joy.Joysticks[ event->jbutton.which ].Name, "Xbox" ) >= 0) )
	{
		if( (event->jbutton.button == 7) /* Start */ && FlyButton && FlyButton->Enabled && FlyButton->Visible )
			FlyButton->Clicked();
		else if( (event->jbutton.button == 4) /* LB */ && ShipDropDown )
		{
			ShipDropDown->Update();
			if( ShipDropDown->Items.size() && (ShipDropDown->Value == ShipDropDown->Items.begin()->Value) )
				ShipDropDown->Select( ShipDropDown->Items.rbegin()->Value );
			else
				ShipDropDown->Clicked( SDL_BUTTON_WHEELUP );
		}
		else if( (event->jbutton.button == 5) /* RB */ && ShipDropDown && ShipDropDown )
		{
			ShipDropDown->Update();
			if( ShipDropDown->Items.size() && (ShipDropDown->Value == ShipDropDown->Items.rbegin()->Value) )
				ShipDropDown->Select( ShipDropDown->Items.begin()->Value );
			else
				ShipDropDown->Clicked( SDL_BUTTON_WHEELDOWN );
		}
		else if( event->jbutton.button == 9 ) // Right Thumbstick Click
			Raptor::Game->Head.Recenter();
		else
			return false;
		return true;
	}
	
	return Layer::HandleEvent( event );
}


bool LobbyMenu::KeyDown( SDLKey key )
{
	if( MessageInput->IsSelected() )
	{
		if( (key == SDLK_RETURN) || (key == SDLK_KP_ENTER) )
		{
			std::string msg = MessageInput->Text;
			MessageInput->Text = "";
			
			if( ! msg.empty() )
			{
				const Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
				
				Packet message = Packet( Raptor::Packet::MESSAGE );
				message.AddString( ((player ? player->Name : std::string("Anonymous")) + Raptor::Game->ChatSeparator + msg).c_str() );
				message.AddUInt( TextConsole::MSG_CHAT );  // FIXME: MSG_TEAM ?
				Raptor::Game->Net.Send( &message );
			}
			
			return true;
		}
		else if( key == SDLK_F8 )
		{
			MissionUploader *mission_uploader = new MissionUploader();
			mission_uploader->Draggable = ! Raptor::Game->Head.VR;
			Raptor::Game->Layers.Add( mission_uploader );
			return true;
		}
		else if( key == SDLK_F10 )
		{
			PrefsMenu *prefs = new PrefsMenu();
			prefs->Draggable = ! Raptor::Game->Head.VR;
			Raptor::Game->Layers.Add( prefs );
			return true;
		}
		else if( key == SDLK_F11 )
		{
			FleetMenu *fleet_menu = new FleetMenu();
			fleet_menu->Draggable = ! Raptor::Game->Head.VR;
			Raptor::Game->Layers.Add( fleet_menu );
			return true;
		}
	}
	else if( PlayerName->IsSelected() )
	{
		if( (key == SDLK_RETURN) || (key == SDLK_KP_ENTER) )
		{
			PlayerName->Deselected();
			Selected = MessageInput;
			return true;
		}
		else if( key == SDLK_ESCAPE )
		{
			Selected = MessageInput;
			UpdatePlayerName();
			return true;
		}
	}
	else if( key == SDLK_F8 )
	{
		MissionUploader *mission_uploader = new MissionUploader();
		mission_uploader->Draggable = ! Raptor::Game->Head.VR;
		Raptor::Game->Layers.Add( mission_uploader );
		return true;
	}
	else if( key == SDLK_F10 )
	{
		PrefsMenu *prefs = new PrefsMenu();
		prefs->Draggable = ! Raptor::Game->Head.VR;
		Raptor::Game->Layers.Add( prefs );
		return true;
	}
	else if( key == SDLK_F11 )
	{
		FleetMenu *fleet_menu = new FleetMenu();
		fleet_menu->Draggable = ! Raptor::Game->Head.VR;
		Raptor::Game->Layers.Add( fleet_menu );
		return true;
	}
	
	// Return false if the event was not handled.
	return false;
}


// ---------------------------------------------------------------------------


LobbyMenuFlyButton::LobbyMenuFlyButton( void )
: LabelledButton( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", 32 ), "Fly", Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


LobbyMenuFlyButton::~LobbyMenuFlyButton()
{
}


void LobbyMenuFlyButton::Clicked( Uint8 button )
{
	if( !( Enabled && Visible ) )
		return;
	
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Packet fly = Packet( XWing::Packet::FLY );
	Raptor::Game->Net.Send( &fly );
}


// ---------------------------------------------------------------------------


LobbyMenuLeaveButton::LobbyMenuLeaveButton( void )
: LabelledButton( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", 32 ), "Leave", Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


LobbyMenuLeaveButton::~LobbyMenuLeaveButton()
{
}


void LobbyMenuLeaveButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Raptor::Game->Net.DisconnectNice();
}


// ---------------------------------------------------------------------------


LobbyMenuPrefsButton::LobbyMenuPrefsButton( std::string title )
: LabelledButton( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", 32 ), title, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


LobbyMenuPrefsButton::~LobbyMenuPrefsButton()
{
}


void LobbyMenuPrefsButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	PrefsMenu *prefs = (PrefsMenu*) Raptor::Game->Layers.Find("PrefsMenu");
	if( prefs )
		prefs->Remove();
	else
		Raptor::Game->Layers.Add( new PrefsMenu() );
}


// ---------------------------------------------------------------------------


LobbyMenuPlayerTextBox::LobbyMenuPlayerTextBox( SDL_Rect *rect, Font *font, uint8_t align, std::string variable ) : TextBox( rect, font, align )
{
	Variable = variable;
}


LobbyMenuPlayerTextBox::~LobbyMenuPlayerTextBox()
{
}


void LobbyMenuPlayerTextBox::Deselected( void )
{
	if( (Variable == "name") && Text.empty() )
	{
		const Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
		if( player )
			Text = player->Name;
	}
	else
		Raptor::Game->SetPlayerProperty( Variable, Text );
}


// ---------------------------------------------------------------------------


LobbyMenuPlayerDropDown::LobbyMenuPlayerDropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, std::string variable ) : DropDown( rect, font, align, scroll_bar_size, NULL, NULL )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
	
	Variable = variable;
	
	const Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	if( player )
		Value = player->PropertyAsString( Variable );
}


LobbyMenuPlayerDropDown::~LobbyMenuPlayerDropDown()
{
}


void LobbyMenuPlayerDropDown::Changed( void )
{
	Raptor::Game->SetPlayerProperty( Variable, Value );
}


// ---------------------------------------------------------------------------


LobbyMenuShipDropDown::LobbyMenuShipDropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, std::string variable )
: LobbyMenuPlayerDropDown( rect, font, align, scroll_bar_size, variable )
{
	Update();
}


LobbyMenuShipDropDown::~LobbyMenuShipDropDown()
{
}


void LobbyMenuShipDropDown::Update( void )
{
	const Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	Value = player ? player->PropertyAsString("ship") : "";
	
	Items.clear();
	AddItem( "", "[Auto-Assign]" );
	
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
	
	if( allowed_ships.size() && ! darkside )
	{
		for( std::vector<std::string>::const_iterator allowed_iter = allowed_ships.begin(); allowed_iter != allowed_ships.end(); allowed_iter ++ )
		{
			const ShipClass *sc = ((const XWingGame*)( Raptor::Game ))->GetShipClass( *allowed_iter );
			if( sc )
				AddItem( sc->ShortName, sc->LongName );
			else if( *allowed_iter == "rebel_gunner" )
				AddItem( "Rebel Gunner", "Rebel Turret Gunner" );
			else if( *allowed_iter == "empire_gunner" )
				AddItem( "Imperial Gunner", "Imperial Turret Gunner" );
			else if( *allowed_iter == "spectator" )
				AddItem( "Spectator", "[Spectator]" );
			else
				AddItem( *allowed_iter, *allowed_iter );
		}
		
		// Allow spectating someone else's campaign.
		if( (FindItem("Spectator") < 0) && (Raptor::Game->Data.Players.size() > 1) && ! Raptor::Server->IsRunning() )
			AddItem( "Spectator", "[Spectator]" );
	}
	else
	{
		for( std::map<uint32_t,GameObject*>::const_iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
		{
			if( obj_iter->second->Type() == XWing::Object::SHIP_CLASS )
			{
				const ShipClass *sc = (const ShipClass*) obj_iter->second;
				if( darkside || sc->PlayersCanFly() )
					AddItem( sc->ShortName, sc->LongName );
			}
		}
		
		AddItem( "Rebel Gunner",    "Rebel Turret Gunner" );
		AddItem( "Imperial Gunner", "Imperial Turret Gunner" );
		AddItem( "Spectator",       "[Spectator]" );
	}
	
	LobbyMenuPlayerDropDown::Update();
}


void LobbyMenuShipDropDown::Clicked( Uint8 button )
{
	if( ! MyListBox )
		Update();
	
	LobbyMenuPlayerDropDown::Clicked( button );
}


// ---------------------------------------------------------------------------


LobbyMenuShipView::LobbyMenuShipView( SDL_Rect *rect ) : Layer( rect )
{
	CurrentGroup = 0;
	Rotation = 0.;
}


LobbyMenuShipView::~LobbyMenuShipView()
{
}


void LobbyMenuShipView::Update( void )
{
	XWingGame *game = (XWingGame*) Raptor::Game;
	const Player *player = game->Data.GetPlayer( Raptor::Game->PlayerID );
	
	std::string ship = player ? player->PropertyAsString("ship") : "";
	uint8_t group = Raptor::Game->Cfg.SettingAsBool("g_group_skins",true) ? Str::AsInt( player->PropertyAsString("group") ) : 0;
	
	if( (ship == "Rebel Gunner") && (Raptor::Game->Data.PropertyAsString("gametype") == "mission") )
		ship = "YT1300";
	
	if( ship.empty() && (Raptor::Game->Data.PropertyAsString("gametype") == "mission") )
	{
		std::string player_ships = Raptor::Game->Data.PropertyAsString("player_ships");
		if( player_ships.empty() )
			player_ships = Raptor::Game->Data.PropertyAsString("player_ship");
		if( ! player_ships.empty() )
		{
			ship = Str::SplitToVector( player_ships, " " ).front();
			if( ship == "rebel_gunner" )
				ship = "YT1300";
		}
	}
	
	if( (ship != CurrentShip) || (group != CurrentGroup) )
	{
		CurrentShip = ship;
		CurrentGroup = group;
		
		Shape.Clear();
		
		const ShipClass *sc = game->GetShipClass( ship );
		if( sc && sc->ExternalModel.length() )
		{
			Model *model = NULL;
			
			uint8_t group = Raptor::Game->Cfg.SettingAsBool("g_group_skins",true) ? Str::AsInt( player->PropertyAsString("group") ) : 0;
			if( group )
			{
				std::map<uint8_t,std::string>::const_iterator skin_iter = sc->GroupSkins.find( group );
				if( (skin_iter != sc->GroupSkins.end()) && skin_iter->second.length() )
					model = Raptor::Game->Res.GetModel( skin_iter->second );
			}
			
			if( !( model && model->VertexCount() ) )
				model = Raptor::Game->Res.GetModel( sc->ExternalModel );
			
			if( model )
			{
				Shape.BecomeInstance( model );
				Shape.ScaleBy( sc->ModelScale );
			}
		}
		else if( ship == "Rebel Gunner" )
			Shape.BecomeInstance( Raptor::Game->Res.GetModel("logo_rebel.obj") );
		else if( ship == "Imperial Gunner" )
			Shape.BecomeInstance( Raptor::Game->Res.GetModel("logo_empire.obj") );
	}
}


void LobbyMenuShipView::Draw( void )
{
	Update();
	
	if( Shape.Objects.empty() || ! Raptor::Game->Cfg.SettingAsBool("ui_ship_preview",true) )
	{
		Rotation = -270.;
		return;
	}
	
	glPushMatrix();
	glPushAttrib( GL_VIEWPORT_BIT );
	Raptor::Game->Gfx.SetViewport( CalcRect.x, CalcRect.y, CalcRect.w, CalcRect.h );
	
	Camera cam;
	cam.FOV = 20.;
	if( CalcRect.w > CalcRect.h )
		cam.FOV *= CalcRect.w / (double) CalcRect.h;
	Pos3D pos = cam + cam.Fwd * std::max<double>( Shape.GetHeight() * 3.5, Shape.GetTriagonal() * 2.2 );
	if( Raptor::Game->Gfx.DrawTo )
	{
		if( Raptor::Game->Gfx.DrawTo == Raptor::Game->Head.EyeL )
			pos.MoveAlong( &(cam.Right), Raptor::Game->Cfg.SettingAsDouble("vr_ship_sep",0.01) );
		else if( Raptor::Game->Gfx.DrawTo == Raptor::Game->Head.EyeR )
			pos.MoveAlong( &(cam.Right), Raptor::Game->Cfg.SettingAsDouble("vr_ship_sep",0.01) * -1. );
	}
	pos.RotateAround( &(pos.Up), Rotation );
	if( Str::BeginsWith( CurrentShip, "B/W" ) )
		pos.MoveAlong( &(pos.Up), 2. );
	if( ! Str::ContainsInsensitive( CurrentShip, "Gunner" ) )
		pos.Pitch( -4. );
	Raptor::Game->Gfx.Setup3D( &cam, CalcRect.h ? (CalcRect.w / (double) CalcRect.h) : 1. );
	
	bool use_shaders = Raptor::Game->Cfg.SettingAsBool("g_shader_enable");
	if( use_shaders )
	{
		Raptor::Game->ShaderMgr.ResumeShaders();
		Raptor::Game->ShaderMgr.Set3f( "CamPos", cam.X, cam.Y, cam.Z );
		Vec3D dir;
		dir.Set( -0.1, 0.7, 0.3 ); // Right Fwd Up
		dir.ScaleTo( 1. );
		Raptor::Game->ShaderMgr.Set3f( "AmbientLight", 0.125, 0.125, 0.125 );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight0Dir", dir.X, dir.Y, dir.Z );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight0Color", 0.75, 0.75, 0.75 );
		Raptor::Game->ShaderMgr.Set1f( "DirectionalLight0WrapAround", 0.125 );
		dir.Set( 0., 0.8, -0.3 ); // Right Fwd Up
		dir.ScaleTo( 1. );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight1Dir", dir.X, dir.Y, dir.Z );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight1Color", 0.5, 0.5, 0.5 );
		Raptor::Game->ShaderMgr.Set1f( "DirectionalLight1WrapAround", 0.125 );
		dir.Set( -0.2, -0.8, 0.2 ); // Right Fwd Up
		dir.ScaleTo( 1. );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight2Dir", dir.X, dir.Y, dir.Z );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight2Color", 1., 1., 1. );
		Raptor::Game->ShaderMgr.Set1f( "DirectionalLight2WrapAround", 0.125 );
		dir.Set( -0.2, -0.7, 0.3 ); // Right Fwd Up
		dir.ScaleTo( 1. );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight3Dir", dir.X, dir.Y, dir.Z );
		Raptor::Game->ShaderMgr.Set3f( "DirectionalLight3Color", 0.75, 0.75, 0.75 );
		Raptor::Game->ShaderMgr.Set1f( "DirectionalLight3WrapAround", 0.125 );
		
		int blastpoints = ((XWingGame*)( Raptor::Game ))->BlastPoints;
		for( int i = 0; i < blastpoints; i ++ )
		{
			std::string index = std::string("[") + Num::ToString( i ) + std::string("]");
			Raptor::Game->ShaderMgr.Set3f( (std::string("BlastPoint")  + index).c_str(), 0., 0., 0. );
			Raptor::Game->ShaderMgr.Set1f( (std::string("BlastRadius") + index).c_str(), 0. );
		}
	}
	
	Shape.DrawAt( &pos );
	
	if( use_shaders )
		Raptor::Game->ShaderMgr.StopShaders();
	
	double rotation_speed = Raptor::Game->Cfg.SettingAsDouble("ui_ship_rotate");
	if( rotation_speed )
	{
		double frame_time = std::min<double>( 0.125, Raptor::Game->FrameTime );
		Rotation += frame_time * rotation_speed;
		if( Rotation > 360. )
			Rotation -= 360.;
	}
	else
		Rotation = 180.;
	
	glPopAttrib();
	glPopMatrix();
}


// ---------------------------------------------------------------------------


LobbyMenuConfiguration::LobbyMenuConfiguration( std::string property, std::string desc, int label_size, int value_size )
: Layer()
{
	Property = property;
	
	uint8_t value_align = Font::ALIGN_MIDDLE_LEFT;
	
	if( label_size )
	{
		AddElement( TitleShadow = new Label( &Rect, desc, Raptor::Game->Res.GetFont( "Verdana.ttf", label_size ), Font::ALIGN_MIDDLE_RIGHT ) );
		AddElement( Title = new Label( &Rect, desc, TitleShadow->LabelFont, TitleShadow->LabelAlign ) );
	
		Title->Alpha = 0.9f;
		TitleShadow->Red = 0.f;
		TitleShadow->Green = 0.f;
		TitleShadow->Blue = 0.f;
		TitleShadow->Alpha = 0.8f;
	}
	else
	{
		TitleShadow = NULL;
		Title = NULL;
		
		value_align = Font::ALIGN_MIDDLE_CENTER;
	}
	
	AddElement( ValueShadow = new Label( &Rect, "", Raptor::Game->Res.GetFont( "Verdana.ttf", value_size ), value_align ) );
	AddElement( Value = new Label( &Rect, "", ValueShadow->LabelFont, ValueShadow->LabelAlign ) );
	
	Value->Alpha = 0.9f;
	ValueShadow->Red = 0.f;
	ValueShadow->Green = 0.f;
	ValueShadow->Blue = 0.f;
	ValueShadow->Alpha = 0.8f;
	
	AddElement( ChangeButton = new LobbyMenuConfigChangeButton( Value->LabelFont, Value->LabelAlign ) );
	ShowButton = false;
	
	Update();
}


LobbyMenuConfiguration::~LobbyMenuConfiguration()
{
}


void LobbyMenuConfiguration::Update( void )
{
	if( Title && TitleShadow )
	{
		Title->Rect.w = Rect.w * 0.4 - 5;
		Title->Rect.h = Value->LabelFont->GetHeight();
		Title->Rect.x = 0;
		Title->Rect.y = 0;
		
		TitleShadow->LabelText = Title->LabelText;
		TitleShadow->Rect.w = Title->Rect.w;
		TitleShadow->Rect.h = Title->Rect.h;
		TitleShadow->Rect.x = Title->Rect.x + 2;
		TitleShadow->Rect.y = Title->Rect.y + 2;
		
		Value->Rect.w = Rect.w * 0.6 - 5;
		Value->Rect.h = Title->Rect.h;
		Value->Rect.x = Rect.w * 0.4 + 5;
		Value->Rect.y = 0;
	}
	else
	{
		Value->Rect.w = Rect.w * 0.8;
		Value->Rect.h = Value->LabelFont->GetHeight();
		Value->Rect.x = 0;
		Value->Rect.y = 0;
	}
	
	ValueShadow->LabelText = Value->LabelText;
	ValueShadow->Rect.w = Value->Rect.w;
	ValueShadow->Rect.h = Value->Rect.h;
	ValueShadow->Rect.x = Value->Rect.x + 2;
	ValueShadow->Rect.y = Value->Rect.y + 2;
	
	ChangeButton->Rect.w = Value->Rect.w;
	ChangeButton->Rect.h = Value->Rect.h;
	ChangeButton->Rect.x = Value->Rect.x;
	ChangeButton->Rect.y = Value->Rect.y;
	ChangeButton->Enabled = (ShowButton && Enabled);
	ChangeButton->Visible = (ShowButton && Enabled);
	ChangeButton->LabelText = Value->LabelText;
	
	Rect.h = Value->Rect.h;
}


// ---------------------------------------------------------------------------


LobbyMenuConfigChangeButton::LobbyMenuConfigChangeButton( Font *font, uint8_t align )
: LabelledButton( NULL, font, "", align, NULL )
{
	Alpha = 0.f;
	AlphaNormal = 0.f;
}


LobbyMenuConfigChangeButton::~LobbyMenuConfigChangeButton()
{
}


void LobbyMenuConfigChangeButton::Clicked( Uint8 button )
{
	LobbyMenuConfiguration *config = (LobbyMenuConfiguration*) Container;
	
	if( !( Enabled && Visible && config->Enabled && config->Visible && config->ShowButton ) )
		return;
	
	std::string value = Raptor::Game->Data.PropertyAsString( config->Property );
	
	bool go_prev = ((button == SDL_BUTTON_RIGHT) || (button == SDL_BUTTON_WHEELDOWN));
	
	bool darkside = (Raptor::Game->Cfg.SettingAsBool("darkside",false) || Raptor::Game->Data.PropertyAsBool("darkside",false)) && ! Raptor::Game->Data.PropertyAsBool("lightside",false);
	
	if( config->Property == "defaults" )
	{
		Packet reset_defaults( XWing::Packet::RESET_DEFAULTS );
		Raptor::Game->Net.Send( &reset_defaults );
		return;
	}
	
	if( config->Property == "gametype" )
	{
		if( value == "fleet" )
			value = go_prev ? "mission" : "yavin";
		else if( value == "yavin" )
			value = go_prev ? "fleet" : "hunt";
		else if( value == "hunt" )
			value = go_prev ? "yavin" : "team_dm";
		else if( value == "team_dm" )
			value = go_prev ? "hunt" : "team_elim";
		else if( value == "team_elim" )
			value = go_prev ? "team_dm" : "team_race";
		else if( value == "team_race" )
			value = go_prev ? "team_elim" : "ffa_dm";
		else if( value == "ffa_dm" )
			value = go_prev ? "team_race" : "ffa_elim";
		else if( value == "ffa_elim" )
			value = go_prev ? "ffa_dm" : "ffa_race";
		else if( value == "ffa_race" )
			value = go_prev ? "ffa_elim" : "mission";
		else if( value == "mission" )
			value = go_prev ? "ffa_race" : "fleet";
		else
			value = "fleet";
	}
	
	else if( config->Property == "mission" )
	{
		std::string campaign = Raptor::Game->Data.PropertyAsString("campaign");
		std::map<std::string,std::string> mission_list = ((XWingGame*)( Raptor::Game ))->MissionList;
		if( campaign.length() )
		{
			for( std::map<std::string,std::string>::iterator mission_iter = mission_list.begin(); mission_iter != mission_list.end(); )
			{
				std::map<std::string,std::string>::iterator mission_next = mission_iter;
				mission_next ++;
				
				if( ! Str::BeginsWith( mission_iter->first, campaign ) )
					mission_list.erase( mission_iter );
				
				mission_iter = mission_next;
			}
		}
		
		std::map<std::string,std::string>::const_iterator find_mission = mission_list.find( value );
		if( find_mission == mission_list.end() )
			value = mission_list.size() ? (go_prev ? mission_list.rbegin()->first : mission_list.begin()->first) : "rebel1";
		else if( go_prev && (find_mission == mission_list.begin()) )
			value = mission_list.rbegin()->first;
		else if( (! go_prev) && (value == mission_list.rbegin()->first) )
			value = mission_list.begin()->first;
		else
		{
			if( go_prev )
				find_mission --;
			else
				find_mission ++;
			value = find_mission->first;
		}
	}
	
	else if( config->Property == "customize_fleet" )
	{
		FleetMenu *fleet_menu = (FleetMenu*) Raptor::Game->Layers.Find("FleetMenu");
		if( fleet_menu )
			fleet_menu->Remove();
		else
		{
			FleetMenu *fleet_menu = new FleetMenu();
			fleet_menu->Draggable = ! Raptor::Game->Head.VR;
			Raptor::Game->Layers.Add( fleet_menu );
		}
		return;
	}
	
	else if( config->Property == "upload_mission" )
	{
		MissionUploader *mission_uploader = (MissionUploader*) Raptor::Game->Layers.Find("MissionUploader");
		if( mission_uploader )
			mission_uploader->Remove();
		else
		{
			MissionUploader *mission_uploader = new MissionUploader();
			mission_uploader->Draggable = ! Raptor::Game->Head.VR;
			Raptor::Game->Layers.Add( mission_uploader );
		}
		return;
	}
	
	else if( (config->Property == "tdm_kill_limit") || (config->Property == "dm_kill_limit") )
	{
		int new_kill_limit = atoi( value.c_str() ) + (go_prev ? -10 : 10);
		
		if( new_kill_limit == 0 )
			new_kill_limit = 5;
		else if( new_kill_limit < 0 )
			new_kill_limit = 1;
		else if( new_kill_limit == 11 )
			new_kill_limit = 5;
		else if( new_kill_limit == 15 )
			new_kill_limit = 10;
		
		value = Num::ToString( new_kill_limit );
	}
	
	else if( config->Property == "race_circuit" )
	{
		value = Str::AsBool(value) ? "false" : "true";
	}
	
	else if( config->Property == "race_lap" )
	{
		int new_checkpoints = atoi( value.c_str() ) + (go_prev ? -1 : 1);
		
		// Allow 0 (scatter) or 3-7 checkpoints.
		if( new_checkpoints == 1 )
			new_checkpoints = 3;
		else if( new_checkpoints == 2 )
			new_checkpoints = 0;
		else if( new_checkpoints < 0 )
			new_checkpoints = 7;
		else if( new_checkpoints > 7 )
			new_checkpoints = 0;
		
		value = Num::ToString( new_checkpoints );
	}
	
	else if( (config->Property == "team_race_checkpoints") || (config->Property == "ffa_race_checkpoints") )
	{
		int new_checkpoints = atoi( value.c_str() ) + (go_prev ? -10 : 10);
		
		if( new_checkpoints <= -10 )
			new_checkpoints = 200;
		else if( new_checkpoints <= 0 )
			new_checkpoints = 0;
		else if( new_checkpoints > 200 )
			new_checkpoints = 0;
		
		value = Num::ToString( new_checkpoints );
	}
	
	else if( config->Property == "ai_waves" )
	{
		int new_ai_waves = atoi( value.c_str() ) + (go_prev ? -1 : 1);
		
		if( new_ai_waves > 10 )
			new_ai_waves = 0;
		if( new_ai_waves < 0 )
			new_ai_waves = 10;
		
		value = Num::ToString( new_ai_waves );
	}
	
	else if( (config->Property == "rebel_cruisers") || (config->Property == "empire_cruisers") )
	{
		int new_ai_cruisers = atoi( value.c_str() ) + (go_prev ? -1 : 1);
		
		if( new_ai_cruisers > 7 )
			new_ai_cruisers = 0;
		if( new_ai_cruisers < 0 )
			new_ai_cruisers = 7;
		
		value = Num::ToString( new_ai_cruisers );
	}
	
	else if( (config->Property == "rebel_frigates") || (config->Property == "empire_frigates") )
	{
		int new_ai_frigates = atoi( value.c_str() ) + (go_prev ? -1 : 1);
		
		if( new_ai_frigates > 5 )
			new_ai_frigates = 0;
		if( new_ai_frigates < 0 )
			new_ai_frigates = 5;
		
		value = Num::ToString( new_ai_frigates );
	}
	
	else if( config->Property == "ai_skill" )
	{
		int new_ai_skill = atoi( value.c_str() ) + (go_prev ? -1 : 1);
		int min_ai_skill = -1, max_ai_skill = 4;
		
		if( new_ai_skill > max_ai_skill )
			new_ai_skill = min_ai_skill;
		else if( new_ai_skill < min_ai_skill )
			new_ai_skill = max_ai_skill;
		
		value = Num::ToString( new_ai_skill );
		
		if( Raptor::Game->Data.PropertyAsString("gametype") == "mission" )
		{
			// Change stored difficulty if we are hosting a campaign.
			
			std::string campaign = Raptor::Game->Data.PropertyAsString("campaign");
			if( campaign.length() )  // FIXME: Make sure it's a valid campaign?
				Raptor::Game->Cfg.Settings[ campaign + std::string("_difficulty") ] = value;
		}
	}
	
	else if( (config->Property == "respawn") || (config->Property == "allow_team_change") )
	{
		if( value == "false" )
			value = "true";
		else
			value = "false";
	}
	
	else if( (config->Property == "respawn_time") )
	{
		int respawn_time = Str::AsInt( value );
		if( respawn_time < 5 )
			value = go_prev ? "65" : "5";
		else if( respawn_time == 5 )
			value = go_prev ? "" : "7";
		else if( respawn_time >= 65 )
			value = go_prev ? "62" : "";
		else
		{
			if( ! go_prev )
				respawn_time += (respawn_time % 5) ? 3 : 2;
			else
				respawn_time -= (respawn_time % 5) ? 2 : 3;
			value = Num::ToString( respawn_time );
		}
	}
	
	else if( config->Property == "asteroids" )
	{
		int new_asteroids = atoi( value.c_str() );
		if( ! go_prev )
		{
			new_asteroids *= 2;
			if( new_asteroids > 2048 )
				new_asteroids = 0;
			else if( ! new_asteroids )
				new_asteroids = 8;
		}
		else
		{
			if( new_asteroids == 0 )
				new_asteroids = 2048;
			else if( new_asteroids <= 8 )
				new_asteroids = 0;
			else
				new_asteroids /= 2;
		}
		
		value = Num::ToString( new_asteroids );
	}
	
	else if( config->Property == "yavin_time_limit" )
	{
		if( value == "1" )
			value = go_prev ? "30" : "5";
		else if( value == "5" )
			value = go_prev ? (darkside ? "1" : "30") : "7";
		else if( value == "7" )
			value = go_prev ? "5" : "15";
		else if( value == "15" )
			value = go_prev ? "7" : "30";
		else
			value = go_prev ? "15" : (darkside ? "1" : "5");
	}
	
	else if( config->Property == "yavin_turrets" )
	{
		int new_yavin_turrets = atoi( value.c_str() ) + (go_prev ? -30 : 30);
		if( new_yavin_turrets < 60 )
			new_yavin_turrets = 210;
		else if( new_yavin_turrets > 210 )
			new_yavin_turrets = 60;
		value = Num::ToString( new_yavin_turrets );
	}
	
	else if( (config->Property == "hunt_time_limit") || (config->Property == "race_time_limit") )
	{
		int new_time_limit = atoi( value.c_str() ) + (go_prev ? -1 : 1);
		if( new_time_limit > 20 )
			new_time_limit = 0;
		else if( new_time_limit < 0 )
			new_time_limit = 20;
		value = Num::ToString( new_time_limit );
	}
	
	else if( config->Property == "defending_team" )
	{
		if( value == "empire" )
			value = "rebel";
		else
			value = "empire";
	}
	
	else if( config->Property == "permissions" )
	{
		if( value == "all" )
			value = "admin";
		else
			value = "all";
	}
	
	else if( config->Property == "bg" )
	{
		if( value == "stars" )
			value = go_prev ? "nebula" : "nebula2";
		else if( value == "nebula2" )
			value = go_prev ? "stars" : "nebula";
		else
			value = go_prev ? "nebula2" : "stars";
	}
	
	Packet info = Packet( Raptor::Packet::INFO );
	info.AddUShort( 1 );
	info.AddString( config->Property );
	info.AddString( value );
	Raptor::Game->Net.Send( &info );
}
