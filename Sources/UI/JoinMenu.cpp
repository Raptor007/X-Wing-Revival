/*
 *  JoinMenu.cpp
 */

#include "JoinMenu.h"

#include <SDL_net/SDL_net.h>

#include "RaptorDefs.h"
#include "Graphics.h"
#include "NetClient.h"
#include "NetUDP.h"
#include "ClientConfig.h"
#include "Str.h"
#include "Num.h"
#include "Rand.h"
#include "Label.h"
#include "RaptorGame.h"
#include "RaptorServer.h"


JoinMenu::JoinMenu( void )
{
	Rect.x = Raptor::Game->Gfx.W/2 - 640/2;
	Rect.y = Raptor::Game->Gfx.H/2 - 480/2;
	Rect.w = 640;
	Rect.h = 480;
	
	Red = 0.0f;
	Green = 0.0f;
	Blue = 1.0f;
	Alpha = 0.5f;
	
	TitleFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 30 );
	ItemFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 18 );
	ButtonFont = Raptor::Game->Res.GetFont( "TimesNR.ttf", 32 );
	
	SDL_Rect rect;
	
	rect.y = 60;
	rect.h = ItemFont->GetHeight();
	rect.x = 10;
	rect.w = Rect.w - rect.x * 2;
	AddElement( new Label( this, &rect, "Your Pilot Name:", ItemFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.y += rect.h + 4;
	AddElement( new JoinMenuTextBox( this, &rect, ItemFont, Font::ALIGN_MIDDLE_LEFT, "name" ) );
	
	rect.y += rect.h + 16;
	AddElement( new Label( this, &rect, "LAN Games:", ItemFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.y += rect.h + 4;
	rect.h = ItemFont->GetHeight() * 8;
	ServerList = new JoinMenuListBox( this, &rect, ItemFont, 20, "host_address" );
	AddElement( ServerList );
	
	rect.y += rect.h + 16;
	rect.h = ItemFont->GetHeight();
	AddElement( new Label( this, &rect, "Connect to IP Address:", ItemFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.y += rect.h + 4;
	JoinMenuTextBox *server_text = new JoinMenuTextBox( this, &rect, ItemFont, Font::ALIGN_MIDDLE_LEFT, "host_address", ServerList );
	ServerList->LinkedTextBox = server_text;
	server_text->Changed();
	AddElement( server_text );
	
	rect.w = 150;
	rect.h = 50;
	rect.y = Rect.h - rect.h - 10;
	rect.x = 10;
	AddElement( new JoinMenuBackButton( this, &rect, "Back" ) );
	
	rect.x = (Rect.w - rect.w) / 2;
	AddElement( new JoinMenuHostButton( this, &rect, "Host" ) );
	
	rect.x = Rect.w - rect.w - 10;
	AddElement( new JoinMenuGoButton( this, &rect, "Join" ) );
	
	ServerFinder.Initialize();
	ServerFinder.StartListening( 7000 );
}


JoinMenu::~JoinMenu()
{
	ServerFinder.StopListening();
}


void JoinMenu::Draw( void )
{
	// Keep the size and position up-to-date.
	Rect.x = Raptor::Game->Gfx.W/2 - 640/2;
	Rect.y = Raptor::Game->Gfx.H/2 - 480/2;
	Rect.w = 640;
	Rect.h = 480;

	// Look for server announcements.
	if( NetUDPPacket *packet = ServerFinder.GetPacket() )
	{
		if( packet->Type() == Raptor::Packet::INFO )
		{
			// This appears to be a valid server announcement, so process it as such.

			// Parse the list of properties.
			std::map<std::string, std::string> properties;
			std::vector<std::string> players;
			uint16_t property_count = packet->NextUShort();
			for( int i = 0; i < property_count; i ++ )
			{
				std::string name = packet->NextString();
				std::string value = packet->NextString();
				
				if( ! name.empty() )
					properties[ name ] = value;
				else
					fprintf( stderr, "JoinMenu::Draw: Server info list of properties shorter than reported!\n" );
			}
			
			bool announced_players = false;
			if( packet->Remaining() )
			{
				announced_players = true;
				
				// Get the list of player names.
				uint16_t player_count = packet->NextUShort();
				for( int i = 0; i < player_count; i ++ )
					players.push_back( packet->NextString() );
			}
			
			// Make sure they are announcing the right game.
			if( (properties.find("game") != properties.end()) && (properties["game"] == Raptor::Game->Game) )
			{
				// Create a string from the IP:Port.
				char host_str[ 128 ] = "";
				int host_ip = Endian::ReadBig32( &(packet->IP) );
				snprintf( host_str, 128, "%i.%i.%i.%i:%s", (host_ip & 0xFF000000) >> 24, (host_ip & 0x00FF0000) >> 16, (host_ip & 0x0000FF00) >> 8, host_ip & 0x000000FF, properties["port"].c_str() );
				
				// This is the name we'll show in the list.
				std::string text;
				
				// If the server name is specified, use that as the list text.
				if( properties.find("name") != properties.end() )
					text = properties["name"];
				else
					text = host_str;
				
				// Check the version reported by the server.
				if( properties.find("version") != properties.end() )
				{
					// If the server version doesn't match ours, append that to the list text.
					if( properties["version"] != Raptor::Game->Version )
						text += " [v " + properties["version"] + "]";
				}
				else
					// If the server reported no version, append a note to the list text.
					text += " [v ?]";
				
				// Show the number of players.
				if( announced_players )
				{
					if( players.size() == 1 )
						text += " [1 player]";
					else
						text += " [" + Num::ToString((int)( players.size() )) + " players]";
				}
				
				// Show the gametype.
				if( properties.find("gametype") != properties.end() )
				{
					if( properties["gametype"] == "team_elim" )
						text += " [Team Elim]";
					else if( properties["gametype"] == "ffa_elim" )
						text += " [FFA Elim]";
					else if( properties["gametype"] == "team_dm" )
						text += " [Team DM]";
					else if( properties["gametype"] == "ffa_dm" )
						text += " [FFA DM]";
					else if( properties["gametype"] == "yavin" )
						text += " [Yavin]";
					else
						text += " [" + properties["gametype"] + "]";
				}
				
				// Look for the server in the list.
				int list_index = ServerList->FindItem( host_str );
				if( list_index >= 0 )
					// If the server is already in the list, update the name.
					ServerList->Items[ list_index ].Text = text;
				else
					// If we've never seen this server before, add it to the list.
					ServerList->AddItem( host_str, text );
			}
		}

		delete packet;
		packet = NULL;
	}
	
	Window::Draw();
	TitleFont->DrawText( "Game Browser", Rect.w/2 + 2, 12, Font::ALIGN_TOP_CENTER, 0,0,0,0.8f );
	TitleFont->DrawText( "Game Browser", Rect.w/2, 10, Font::ALIGN_TOP_CENTER );
}


bool JoinMenu::KeyUp( SDLKey key )
{
	if( key == SDLK_ESCAPE )
	{
		Remove();
		return true;
	}
	
	return false;
}


// ---------------------------------------------------------------------------


JoinMenuTextBox::JoinMenuTextBox( Window *Container, SDL_Rect *rect, Font *font, uint8_t align, std::string variable, ListBox *linked_list_box ) : TextBox( Container, rect, font, align, Raptor::Game->Cfg.Settings[ variable ] )
{
	Variable = variable;
	LinkedListBox = linked_list_box;
	ReturnDeselects = true;
}


void JoinMenuTextBox::Changed( void )
{
	Raptor::Game->Cfg.Settings[ Variable ] = Text;
}


// ---------------------------------------------------------------------------


JoinMenuListBox::JoinMenuListBox( Window *Container, SDL_Rect *rect, Font *font, int scroll_bar_size, std::string variable, TextBox *linked_text_box ) : ListBox( Container, rect, font, scroll_bar_size )
{
	Variable = variable;
	LinkedTextBox = linked_text_box;
}


void JoinMenuListBox::Changed( void )
{
	if( LinkedTextBox )
		LinkedTextBox->Text = SelectedValue();
	
	Raptor::Game->Cfg.Settings[ Variable ] = SelectedValue();
}


// ---------------------------------------------------------------------------


JoinMenuGoButton::JoinMenuGoButton( JoinMenu *menu, SDL_Rect *rect, const char *label ) : LabelledButton( menu, rect, menu->ButtonFont, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


void JoinMenuGoButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	JoinMenu *menu = (JoinMenu*) Container;
	
	if( (Raptor::Game->Cfg.Settings["host_address"] == "") && menu->ServerList->Items.size() )
		menu->ServerList->Select( 0 );
	
	if( Raptor::Game->Cfg.Settings["host_address"] != "" )
	{
		if( Raptor::Game->Cfg.Settings["name"] == "" )
			Raptor::Game->Cfg.Settings["name"] = "Rookie " + Num::ToString(Rand::Int(1,9));
		
		Raptor::Game->Net.Connect( Raptor::Game->Cfg.Settings["host_address"].c_str(), Raptor::Game->Cfg.Settings["name"].c_str(), Raptor::Game->Cfg.Settings["password"].c_str() );
		
		Container->Remove();
	}
}


// ---------------------------------------------------------------------------


JoinMenuHostButton::JoinMenuHostButton( JoinMenu *menu, SDL_Rect *rect, const char *label ) : LabelledButton( menu, rect, menu->ButtonFont, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


void JoinMenuHostButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	if( Raptor::Game->Cfg.Settings["name"] == "" )
		Raptor::Game->Cfg.Settings["name"] = "Rookie " + Num::ToString(Rand::Int(1,9));
	
	Raptor::Game->Host();
	
	Container->Remove();
}


// ---------------------------------------------------------------------------


JoinMenuBackButton::JoinMenuBackButton( JoinMenu *menu, SDL_Rect *rect, const char *label ) : LabelledButton( menu, rect, menu->ButtonFont, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


void JoinMenuBackButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Container->Remove();
}
