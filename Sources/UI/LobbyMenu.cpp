/*
 *  LobbyMenu.cpp
 */

#include "LobbyMenu.h"

#include "XWingDefs.h"
#include "XWingGame.h"
#include "Num.h"


LobbyMenu::LobbyMenu( void )
{
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
	
	Background.BecomeInstance( Raptor::Game->Res.GetAnimation("bg_lobby.ani") );
	
	bool tiny = (Rect.h < 720) || (Rect.w < 800);
	TitleFont = Raptor::Game->Res.GetFont( "Verdana.ttf", tiny ? 24 : 30 );
	
	AddElement( LeaveButton = new LobbyMenuLeaveButton() );
	AddElement( FlyButton = new LobbyMenuFlyButton() );
	AddElement( TeamButton = new LobbyMenuTeamButton( tiny ? 12 : 17 ) );
	AddElement( GroupButton = new LobbyMenuGroupButton( tiny ? 12 : 17 ) );
	AddElement( ShipButton = new LobbyMenuShipButton( tiny ? 12 : 17 ) );
	
	AddElement( PlayerName = new TextBox( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", tiny ? 13 : 19 ), Font::ALIGN_MIDDLE_LEFT ) );
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
	int value_size = tiny ? 16 : 22;
	ConfigOrder.push_back( new LobbyMenuConfiguration( "permissions", "", 0, tiny ? 14 : 17 ) );
	ConfigOrder.push_back( NULL );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "gametype", "Game Type", tiny ? 12 : 17, tiny ? 22 : 27 ) );
	ConfigOrder.push_back( NULL );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "defending_team", "Defending Team", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "rebel_ship", "Rebel Capital Ship", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "empire_ship", "Empire Capital Ship", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "tdm_kill_limit", "Team Kill Limit", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "dm_kill_limit", "Kill Limit", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "hunt_time_limit", "Time Limit", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "yavin_time_limit", "Time Limit", label_size, value_size ) );
	ConfigOrder.push_back( NULL );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "ai_waves", "AI Ships", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "yavin_turrets", "Surface Turrets", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "asteroids", "Asteroids", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "bg", "Environment", label_size, value_size ) );
	ConfigOrder.push_back( new LobbyMenuConfiguration( "respawn", "Respawn", label_size, value_size ) );
	
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
	FlyButton->Rect.h = LeaveButton->Rect.h;
	
	LeaveButton->Rect.x = 32;
	
	if( Rect.w >= 1024 )
		LeaveButton->Rect.w = 384;
	else
		LeaveButton->Rect.w = Rect.w / 2 - TeamButton->Rect.w / 2 - LeaveButton->Rect.x * 2;
	FlyButton->Rect.w = LeaveButton->Rect.w;
	
	LeaveButton->Rect.y = Rect.h - LeaveButton->Rect.h - 32;
	FlyButton->Rect.y = LeaveButton->Rect.y;
	
	FlyButton->Rect.x = Rect.w - FlyButton->Rect.w - LeaveButton->Rect.x;
	
	PlayerName->Rect.x = LeaveButton->Rect.x;
	PlayerName->Rect.w = (FlyButton->Rect.x + FlyButton->Rect.w - PlayerList->Rect.x) / 2 - 5;
	PlayerName->Rect.y = TitleFont->GetHeight() + 20;
	
	TeamButton->Rect.x = LeaveButton->Rect.x;
	TeamButton->Rect.y = PlayerName->Rect.y + PlayerName->Rect.h + 10;
	TeamButton->Rect.w = tiny ? 100 : 140;
	TeamButton->Rect.h = TeamButton->LabelFont->GetHeight() + 6;
	
	GroupButton->Rect.x = TeamButton->Rect.x + TeamButton->Rect.w + 10;
	GroupButton->Rect.y = TeamButton->Rect.y;
	GroupButton->Rect.w = TeamButton->Rect.w;
	GroupButton->Rect.h = TeamButton->Rect.h;
	
	ShipButton->Rect.x = GroupButton->Rect.x + GroupButton->Rect.w + 10;
	ShipButton->Rect.y = GroupButton->Rect.y;
	ShipButton->Rect.w = GroupButton->Rect.w;
	ShipButton->Rect.h = GroupButton->Rect.h;
	
	GroupButton->Enabled = true;
	ShipButton->Enabled = true;
	Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	if( player )
	{
		bool ffa = ( Raptor::Game->Data.Properties["gametype"].find("ffa_") == 0 );
		if( (player->Properties["team"] == "Spectator") || ((! ffa) && (player->Properties["team"] == "")) )
		{
			GroupButton->Enabled = false;
			ShipButton->Enabled = false;
		}
		else if( ffa )
			GroupButton->Enabled = false;
		
		// FIXME: Remove this when there are more Empire ships to choose from.
		if( (! ffa) && (player->Properties["team"] == "Empire") )
			ShipButton->Enabled = false;
	}
	
	if( GroupButton->Enabled )
	{
		GroupButton->Alpha = TeamButton->Alpha;
		GroupButton->AlphaNormal = TeamButton->AlphaNormal;
		GroupButton->RedOver = TeamButton->RedOver;
		GroupButton->BlueOver = TeamButton->BlueOver;
		GroupButton->GreenOver = TeamButton->GreenOver;
		GroupButton->AlphaOver = TeamButton->AlphaOver;
	}
	else
	{
		GroupButton->Alpha = 0.5f;
		GroupButton->AlphaNormal = 0.5f;
		GroupButton->RedOver = TeamButton->RedNormal;
		GroupButton->BlueOver = TeamButton->BlueNormal;
		GroupButton->GreenOver = TeamButton->GreenNormal;
		GroupButton->AlphaOver = 0.5f;
	}
	
	if( ShipButton->Enabled )
	{
		ShipButton->Alpha = TeamButton->Alpha;
		ShipButton->AlphaNormal = TeamButton->AlphaNormal;
		ShipButton->RedOver = TeamButton->RedOver;
		ShipButton->BlueOver = TeamButton->BlueOver;
		ShipButton->GreenOver = TeamButton->GreenOver;
		ShipButton->AlphaOver = TeamButton->AlphaOver;
	}
	else
	{
		ShipButton->Alpha = 0.5f;
		ShipButton->AlphaNormal = 0.5f;
		ShipButton->RedOver = TeamButton->RedNormal;
		ShipButton->BlueOver = TeamButton->BlueNormal;
		ShipButton->GreenOver = TeamButton->GreenNormal;
		ShipButton->AlphaOver = 0.5f;
	}
	
	PlayerList->Rect.x = LeaveButton->Rect.x;
	PlayerList->Rect.w = (FlyButton->Rect.x + FlyButton->Rect.w - PlayerList->Rect.x) / 2 - 5;
	PlayerList->Rect.y = TeamButton->Rect.y + TeamButton->Rect.h + 10;
	PlayerList->Rect.h = 400 - (PlayerName->Rect.h + TeamButton->Rect.h + 20);
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
	
	LobbyMenuConfiguration *prev = NULL;
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
	
	UpdateCalcRects();
}


void LobbyMenu::UpdatePlayerName( void )
{
	if( ! PlayerName->IsSelected() )
	{
		Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
		if( player )
			PlayerName->Text = player->Name;
	}
}


void LobbyMenu::UpdatePlayerList( void )
{
	std::string prev_selected = PlayerList->SelectedValue();
	PlayerList->Clear();
	
	bool ffa = ( Raptor::Game->Data.Properties["gametype"].find("ffa_") == 0 );
	
	for( std::map<uint16_t,Player*>::iterator player_iter = Raptor::Game->Data.Players.begin(); player_iter != Raptor::Game->Data.Players.end(); player_iter ++ )
	{
		std::string display_name = player_iter->second->Name;
		
		std::string team = player_iter->second->Properties["team"];
		int group = ffa ? 0 : atoi( player_iter->second->Properties["group"].c_str() );
		if( (! ffa) || (team == "Spectator") )
		{
			if( team.empty() )
				display_name += " [Auto-Team]";
			else if( group )
				display_name += " [" + team + " group " + Num::ToString(group) + "]";
			else
				display_name += " [" + team + "]";
		}
		
		if( team != "Spectator" )
		{
			std::string ship = player_iter->second->Properties["ship"];
			if( ! ship.empty() )
				display_name += " [" + ship + "]";
		}
		
		PlayerList->AddItem( Num::ToString( player_iter->second->ID ), display_name );
	}
	
	PlayerList->Select( prev_selected );
}


void LobbyMenu::UpdateMessageList( void )
{
	size_t msg_count = Raptor::Game->Msg.Messages.size();
	if( msg_count != MessageList->Items.size() )
	{
		std::string prev_selected = MessageList->SelectedValue();
		MessageList->Clear();
		
		for( size_t i = 0; i < msg_count; i ++ )
			MessageList->AddItem( Num::ToString((int)i), Raptor::Game->Msg.Messages.at(i)->Text );
		
		if( ! prev_selected.empty() )
			MessageList->Select( prev_selected );
		
		MessageList->Scroll = MessageList->MaxScroll();
	}
}


void LobbyMenu::UpdateInfoBoxes( void )
{
	std::string permissions = Raptor::Game->Data.Properties["permissions"];
	if( permissions == "all" )
		Configs["permissions"]->Value->LabelText = "Anyone Can Change";
	else
		Configs["permissions"]->Value->LabelText = "Only Host May Change";
	
	std::string gametype = Raptor::Game->Data.Properties["gametype"];
	if( gametype == "team_elim" )
		Configs["gametype"]->Value->LabelText = "Team Elimination";
	else if( gametype == "ffa_elim" )
		Configs["gametype"]->Value->LabelText = "FFA Elimination";
	else if( gametype == "team_dm" )
		Configs["gametype"]->Value->LabelText = "Team Deathmatch";
	else if( gametype == "ffa_dm" )
		Configs["gametype"]->Value->LabelText = "FFA Deathmatch";
	else if( gametype == "yavin" )
		Configs["gametype"]->Value->LabelText = "Battle of Yavin";
	else if( gametype == "hunt" )
		Configs["gametype"]->Value->LabelText = "Capital Ship Hunt";
	else if( gametype == "def_des" )
		Configs["gametype"]->Value->LabelText = "Defend/Destroy";
	else
		Configs["gametype"]->Value->LabelText = gametype;
	
	int tdm_kill_limit = atoi( Raptor::Game->Data.Properties["tdm_kill_limit"].c_str() );
	Configs["tdm_kill_limit"]->Value->LabelText = tdm_kill_limit ? Num::ToString(tdm_kill_limit) : "Unlimited";
	
	int dm_kill_limit = atoi( Raptor::Game->Data.Properties["dm_kill_limit"].c_str() );
	Configs["dm_kill_limit"]->Value->LabelText = dm_kill_limit ? Num::ToString(dm_kill_limit) : "Unlimited";
	
	int ai_waves = atoi( Raptor::Game->Data.Properties["ai_waves"].c_str() );
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
	
	std::string respawn = Raptor::Game->Data.Properties["respawn"];
	if( gametype == "team_elim" )
	{
		Configs["respawn"]->Title->LabelText = "Dead Players";
		Configs["respawn"]->TitleShadow->LabelText = "Dead Players";
		
		if( respawn == "true" )
			Configs["respawn"]->Value->LabelText = "Control AI Ships";
		else if( respawn == "false" )
			Configs["respawn"]->Value->LabelText = "Observe Only";
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
			Configs["respawn"]->Value->LabelText = "No";
		else
			Configs["respawn"]->Value->LabelText = respawn;
	}
	
	int asteroids = atoi( Raptor::Game->Data.Properties["asteroids"].c_str() );
	if( asteroids <= 0 )
		Configs["asteroids"]->Value->LabelText = "None";
	else if( asteroids <= 16 )
		Configs["asteroids"]->Value->LabelText = "Very Few";
	else if( asteroids <= 32 )
		Configs["asteroids"]->Value->LabelText = "Few";
	else if( asteroids <= 64 )
		Configs["asteroids"]->Value->LabelText = "Some";
	else if( asteroids <= 128 )
		Configs["asteroids"]->Value->LabelText = "Many";
	else if( asteroids <= 256 )
		Configs["asteroids"]->Value->LabelText = "Very Many";
	else if( asteroids <= 512 )
		Configs["asteroids"]->Value->LabelText = "Uncomfortably Many";
	else if( asteroids <= 1024 )
		Configs["asteroids"]->Value->LabelText = "Insanely Many";
	else if( asteroids <= 2048 )
		Configs["asteroids"]->Value->LabelText = "Hardly Worth Playing";
	else
		Configs["asteroids"]->Value->LabelText = "Not Worth Playing";
	
	int yavin_time_limit = atoi( Raptor::Game->Data.Properties["yavin_time_limit"].c_str() );
	Configs["yavin_time_limit"]->Value->LabelText = yavin_time_limit ? (Num::ToString(yavin_time_limit) + std::string(" min")) : "Unlimited";
	
	int yavin_turrets = atoi( Raptor::Game->Data.Properties["yavin_turrets"].c_str() );
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
	
	int hunt_time_limit = atoi( Raptor::Game->Data.Properties["hunt_time_limit"].c_str() );
	Configs["hunt_time_limit"]->Value->LabelText = hunt_time_limit ? (Num::ToString(hunt_time_limit) + std::string(" min")) : "Unlimited";
	
	std::string defending_team = Raptor::Game->Data.Properties["defending_team"];
	if( defending_team == "empire" )
		Configs["defending_team"]->Value->LabelText = "Empire";
	else if( defending_team == "rebel" )
		Configs["defending_team"]->Value->LabelText = "Rebels";
	else
		Configs["defending_team"]->Value->LabelText = defending_team;
	
	std::string rebel_ship = Raptor::Game->Data.Properties["rebel_ship"];
	if( rebel_ship == "isd2" )
		Configs["rebel_ship"]->Value->LabelText = "Imperial Star Destroyer";
	else if( rebel_ship == "crv" )
		Configs["rebel_ship"]->Value->LabelText = "Corellian Corvette";
	else if( rebel_ship == "frg" )
		Configs["rebel_ship"]->Value->LabelText = "Nebulon B Frigate";
	else if( rebel_ship == "crs" )
		Configs["rebel_ship"]->Value->LabelText = "Mon Calamari Cruiser";
	else
		Configs["rebel_ship"]->Value->LabelText = rebel_ship;
	
	std::string empire_ship = Raptor::Game->Data.Properties["empire_ship"];
	if( empire_ship == "isd2" )
		Configs["empire_ship"]->Value->LabelText = "Imperial Star Destroyer";
	else if( empire_ship == "crv" )
		Configs["empire_ship"]->Value->LabelText = "Corellian Corvette";
	else if( empire_ship == "frg" )
		Configs["empire_ship"]->Value->LabelText = "Nebulon B Frigate";
	else if( empire_ship == "crs" )
		Configs["empire_ship"]->Value->LabelText = "Mon Calamari Cruiser";
	else
		Configs["empire_ship"]->Value->LabelText = empire_ship;
	
	std::string bg = Raptor::Game->Data.Properties["bg"];
	if( bg == "stars" )
		Configs["bg"]->Value->LabelText = "Deep Space";
	else if( bg == "nebula" )
		Configs["bg"]->Value->LabelText = "Nebula";
	else
		Configs["bg"]->Value->LabelText = bg;
	
	// Determine player permissions.
	Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	bool admin = (player && player->Properties["admin"] == "true");
	bool permissions_all = (Raptor::Game->Data.Properties["permissions"] == "all");
	bool flying = Raptor::Game->Data.GameObjects.size();
	
	// Show "Fly" button for admin or late joiners.
	FlyButton->Enabled = (admin || permissions_all || flying);
	
	// Show "Change" buttons for admin.
	FlyButton->Visible = FlyButton->Enabled;
	for( std::map<std::string,LobbyMenuConfiguration*>::iterator config_iter = Configs.begin(); config_iter != Configs.end(); config_iter ++ )
		config_iter->second->ShowButton = (admin || permissions_all) && (! flying);
	Configs["permissions"]->ShowButton = admin;
	
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
	
	// Show "Defending Team", "Capital Ship", and "Time Limit" for hunt mode.
	// Show "Rebel Capital Ship" and "Empire Capital Ship" for defend/destroy mode.
	if( gametype == "hunt" )
	{
		Configs["defending_team"]->Visible = true;
		Configs["defending_team"]->Enabled = true;
		
		Configs["hunt_time_limit"]->Visible = true;
		Configs["hunt_time_limit"]->Enabled = true;
		
		if( Raptor::Game->Data.Properties["defending_team"] == "rebel" )
		{
			Configs["rebel_ship"]->Visible = true;
			Configs["rebel_ship"]->Enabled = true;
			
			Configs["empire_ship"]->ShowButton = false;
			Configs["empire_ship"]->Visible = false;
			Configs["empire_ship"]->Enabled = false;
		}
		else
		{
			Configs["empire_ship"]->Visible = true;
			Configs["empire_ship"]->Enabled = true;
			
			Configs["rebel_ship"]->ShowButton = false;
			Configs["rebel_ship"]->Visible = false;
			Configs["rebel_ship"]->Enabled = false;
		}
	}
	else
	{
		Configs["defending_team"]->ShowButton = false;
		Configs["defending_team"]->Visible = false;
		Configs["defending_team"]->Enabled = false;
		
		Configs["hunt_time_limit"]->ShowButton = false;
		Configs["hunt_time_limit"]->Visible = false;
		Configs["hunt_time_limit"]->Enabled = false;
		
		if( gametype == "def_des" )
		{
			Configs["rebel_ship"]->Visible = true;
			Configs["rebel_ship"]->Enabled = true;
			
			Configs["empire_ship"]->Visible = true;
			Configs["empire_ship"]->Enabled = true;
		}
		else
		{
			Configs["rebel_ship"]->ShowButton = false;
			Configs["rebel_ship"]->Visible = false;
			Configs["rebel_ship"]->Enabled = false;
			
			Configs["empire_ship"]->ShowButton = false;
			Configs["empire_ship"]->Visible = false;
			Configs["empire_ship"]->Enabled = false;
		}
	}
}


void LobbyMenu::Draw( void )
{
	UpdateRects();
	UpdateInfoBoxes();
	UpdatePlayerName();
	UpdatePlayerList();
	UpdateMessageList();
	
	Raptor::Game->Gfx.DrawRect2D( Rect.w / 2 - Rect.h, 0, Rect.w / 2 + Rect.h, Rect.h, Background.CurrentFrame(), 1.f, 1.f, 1.f, 1.f );
	
	Layer::Draw();
	
	std::string title = "Pre-Game Lobby";
	if( Raptor::Game->Data.GameObjects.size() )
		title = "Game-In-Progress Lobby";
	
	TitleFont->DrawText( title, Rect.w/2 + 2, 12, Font::ALIGN_TOP_CENTER, 0.f,0.f,0.f,0.8f );
	TitleFont->DrawText( title, Rect.w/2, 10, Font::ALIGN_TOP_CENTER );
	
	TitleFont->DrawText( "Game Settings", PlayerList->Rect.x + PlayerList->Rect.w*1.4 + 12, PlayerName->Rect.y + 2, Font::ALIGN_TOP_CENTER, 0.f,0.f,0.f,0.8f );
	TitleFont->DrawText( "Game Settings", PlayerList->Rect.x + PlayerList->Rect.w*1.4 + 10, PlayerName->Rect.y, Font::ALIGN_TOP_CENTER );
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
				Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
				
				Packet message = Packet( Raptor::Packet::MESSAGE );
				message.AddString( ((player ? player->Name : std::string("Anonymous")) + std::string(":  ") + msg).c_str() );
				message.AddUInt( TextConsole::MSG_CHAT );
				Raptor::Game->Net.Send( &message );
			}
			
			return true;
		}
	}
	else if( PlayerName->IsSelected() )
	{
		if( (key == SDLK_RETURN) || (key == SDLK_KP_ENTER) )
		{
			if( PlayerName->Text.size() )
			{
				Raptor::Game->Cfg.Settings[ "name" ] = PlayerName->Text;
				
				Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
				if( player )
				{
					player->Name = PlayerName->Text;
					
					Packet player_properties = Packet( Raptor::Packet::PLAYER_PROPERTIES );
					player_properties.AddUShort( Raptor::Game->PlayerID );
					player_properties.AddUInt( 1 );
					player_properties.AddString( "name" );
					player_properties.AddString( player->Name );
					Raptor::Game->Net.Send( &player_properties );
				}
			}
			else
				UpdatePlayerName();
			
			Selected = MessageInput;
			return true;
		}
		else if( key == SDLK_ESCAPE )
		{
			UpdatePlayerName();
			Selected = MessageInput;
			return true;
		}
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


LobbyMenuTeamButton::LobbyMenuTeamButton( int font_size )
: LabelledButton( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", font_size ), "Change Team", Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
}


LobbyMenuTeamButton::~LobbyMenuTeamButton()
{
}


void LobbyMenuTeamButton::Clicked( Uint8 button )
{
	Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	if( ! player )
		return;
	
	std::string team = player->Properties["team"];
	
	bool ffa = ( Raptor::Game->Data.Properties["gametype"].find("ffa_") == 0 );
	
	if( ! ffa )
	{
		bool go_prev = ((button == SDL_BUTTON_RIGHT) || (button == SDL_BUTTON_WHEELUP));
		
		if( player && (player->Properties["team"] == "Rebel") )
			team = go_prev ? "" : "Empire";
		else if( player && (player->Properties["team"] == "Empire") )
			team = go_prev ? "Rebel" : "Spectator";
		else if( player && (player->Properties["team"] == "Spectator") )
			team = go_prev ? "Empire" : "";
		else if( player )
			team = go_prev ? "Spectator" : "Rebel";
	}
	else
	{
		if( team == "Spectator" )
			team = "";
		else
			team = "Spectator";
	}
	
	std::string ship = "";
	if( team == "Rebel" )
		ship = "X/W";
	else if( team == "Empire" )
		ship = "T/F";
	
	Packet player_properties = Packet( Raptor::Packet::PLAYER_PROPERTIES );
	player_properties.AddUShort( Raptor::Game->PlayerID );
	player_properties.AddUInt( 2 );
	player_properties.AddString( "team" );
	player_properties.AddString( team );
	player_properties.AddString( "ship" );
	player_properties.AddString( ship );
	Raptor::Game->Net.Send( &player_properties );
}


// ---------------------------------------------------------------------------


LobbyMenuGroupButton::LobbyMenuGroupButton( int font_size )
: LabelledButton( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", font_size ), "Change Group", Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
}


LobbyMenuGroupButton::~LobbyMenuGroupButton()
{
}


void LobbyMenuGroupButton::Clicked( Uint8 button )
{
	Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	if( ! player )
		return;
	
	std::string team = player->Properties["team"];
	
	if( team != "Spectator" )
	{
		bool ffa = ( Raptor::Game->Data.Properties["gametype"].find("ffa_") == 0 );
		int group = atoi( player->Properties["group"].c_str() );
		
		if( ffa )
		{
			team = "";
			group = 0;
		}
		else
		{
			bool go_prev = ((button == SDL_BUTTON_RIGHT) || (button == SDL_BUTTON_WHEELUP));
			group += go_prev ? -1 : 1;
			
			if( group < 0 )
				group = 4;
			else if( group > 4 )
				group = 0;
		}
		
		Packet player_properties = Packet( Raptor::Packet::PLAYER_PROPERTIES );
		player_properties.AddUShort( Raptor::Game->PlayerID );
		player_properties.AddUInt( 2 );
		player_properties.AddString( "team" );
		player_properties.AddString( team );
		player_properties.AddString( "group" );
		player_properties.AddString( Num::ToString(group).c_str() );
		Raptor::Game->Net.Send( &player_properties );
	}
}


// ---------------------------------------------------------------------------


LobbyMenuShipButton::LobbyMenuShipButton( int font_size )
: LabelledButton( NULL, Raptor::Game->Res.GetFont( "Verdana.ttf", font_size ), "Change Ship", Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
}


LobbyMenuShipButton::~LobbyMenuShipButton()
{
}


void LobbyMenuShipButton::Clicked( Uint8 button )
{
	Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	if( ! player )
		return;
	
	std::string team = player->Properties["team"];
	
	if( team != "Spectator" )
	{
		bool ffa = ( Raptor::Game->Data.Properties["gametype"].find("ffa_") == 0 );
		
		std::string ship = "";
		
		if( ffa )
		{
			team = "";
			
			bool go_prev = ((button == SDL_BUTTON_RIGHT) || (button == SDL_BUTTON_WHEELUP));
			
			if( player->Properties["ship"] == "X/W" )
				ship = go_prev ? "T/F" : "Y/W";
			else if( player->Properties["ship"] == "Y/W" )
				ship = go_prev ? "X/W" : "T/F";
			else
				ship = go_prev ? "Y/W" : "X/W";
		}
		else if( player->Properties["team"] == "Rebel" )
		{
			if( player->Properties["ship"] == "X/W" )
				ship = "Y/W";
			else
				ship = "X/W";
		}
		else if( player->Properties["team"] == "Empire" )
		{
			ship = "T/F";
		}
		
		Packet player_properties = Packet( Raptor::Packet::PLAYER_PROPERTIES );
		player_properties.AddUShort( Raptor::Game->PlayerID );
		player_properties.AddUInt( 2 );
		player_properties.AddString( "team" );
		player_properties.AddString( team );
		player_properties.AddString( "ship" );
		player_properties.AddString( ship );
		Raptor::Game->Net.Send( &player_properties );
	}
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
	
	std::string value = Raptor::Game->Data.Properties[ config->Property ];
	
	bool go_prev = ((button == SDL_BUTTON_RIGHT) || (button == SDL_BUTTON_WHEELUP));
	
	if( config->Property == "gametype" )
	{
		if( value == "yavin" )
			value = go_prev ? "ffa_dm" : "hunt";
		else if( value == "hunt" )
			value = go_prev ? "yavin" : "def_des";
		else if( value == "def_des" )
			value = go_prev ? "hunt" : "team_elim";
		else if( value == "team_elim" )
			value = go_prev ? "def_des" : "ffa_elim";
		else if( value == "ffa_elim" )
			value = go_prev ? "team_elim" : "team_dm";
		else if( value == "team_dm" )
			value = go_prev ? "ffa_elim" : "ffa_dm";
		else if( value == "ffa_dm" )
			value = go_prev ? "team_dm" : "yavin";
		else
			value = "yavin";
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
	
	else if( config->Property == "ai_waves" )
	{
		int new_ai_waves = atoi( value.c_str() ) + (go_prev ? -1 : 1);
		
		if( new_ai_waves > 10 )
			new_ai_waves = 0;
		if( new_ai_waves < 0 )
			new_ai_waves = 10;
		
		value = Num::ToString( new_ai_waves );
	}
	
	else if( config->Property == "respawn" )
	{
		if( value == "false" )
			value = "true";
		else
			value = "false";
	}
	
	else if( config->Property == "asteroids" )
	{
		int new_asteroids = atoi( value.c_str() );
		if( ! go_prev )
		{
			new_asteroids *= 2;
			if( new_asteroids > 4096 )
				new_asteroids = 0;
			else if( ! new_asteroids )
				new_asteroids = 16;
		}
		else
		{
			if( new_asteroids == 0 )
				new_asteroids = 4096;
			else if( new_asteroids <= 16 )
				new_asteroids = 0;
			else
				new_asteroids /= 2;
		}
		
		value = Num::ToString( new_asteroids );
	}
	
	else if( config->Property == "yavin_time_limit" )
	{
		if( value == "7" )
			value = go_prev ? "30" : "15";
		else if( value == "15" )
			value = go_prev ? "7" : "30";
		else
			value = go_prev ? "15" : "7";
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
	
	else if( config->Property == "hunt_time_limit" )
	{
		int new_hunt_time_limit = atoi( value.c_str() ) + (go_prev ? -1 : 1);
		if( new_hunt_time_limit > 20 )
			new_hunt_time_limit = 0;
		else if( new_hunt_time_limit < 0 )
			new_hunt_time_limit = 20;
		value = Num::ToString( new_hunt_time_limit );
	}
	
	else if( config->Property == "defending_team" )
	{
		if( value == "empire" )
			value = "rebel";
		else
			value = "empire";
	}
	
	else if( config->Property == "empire_ship" )
	{
		if( value == "isd2" )
			value = go_prev ? "frg" : "crv";
		else if( value == "crv" )
			value = go_prev ? "isd2" : "frg";
		else if( value == "frg" )
			value = go_prev ? "crv" : "isd2";
		else
			value = "isd2";
	}
	
	else if( config->Property == "rebel_ship" )
	{
		if( value == "crs" )
			value = go_prev ? "frg" : "crv";
		else if( value == "crv" )
			value = go_prev ? "crs" : "frg";
		else if( value == "frg" )
			value = go_prev ? "crv" : "crs";
		else
			value = "crs";
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
			value = "nebula";
		else
			value = "stars";
	}
	
	Packet info = Packet( Raptor::Packet::INFO );
	info.AddUShort( 1 );
	info.AddString( config->Property );
	info.AddString( value );
	Raptor::Game->Net.Send( &info );
}
