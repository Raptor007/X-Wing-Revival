/*
 *  MissionUploader.cpp
 */

#include "MissionUploader.h"

#include "Label.h"
#include "Num.h"
#include "XWingDefs.h"
#include "XWingGame.h"
#include "Mission.h"
#include "File.h"
#include <dirent.h>


MissionUploader::MissionUploader( void )
{
	Name = "MissionUploader";
	
	Rect.w = 512;
	Rect.h = 160;
	Rect.x = Raptor::Game->Gfx.W/2 - Rect.w/2;
	Rect.y = Raptor::Game->Gfx.H/2 - Rect.h/2;
	
	Red = 0.f;
	Green = 0.f;
	Blue = 1.f;
	Alpha = 0.5f;
	
	TitleFont  = Raptor::Game->Res.GetFont( "Verdana.ttf", 30 );
	ItemFont   = Raptor::Game->Res.GetFont( "Verdana.ttf", 21 );
	ButtonFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 30 );
	
	UpdateContents();
}


MissionUploader::~MissionUploader()
{
}


void MissionUploader::UpdateContents( void )
{
	// Remove existing elements.
	
	Selected = NULL;
	RemoveAllElements();
	
	
	// Add new elements.
	
	SDL_Rect rect;
	
	rect.w = 150;
	rect.h = 50;
	rect.y = Rect.h - rect.h - 10;
	rect.x = Rect.w - rect.w - 10;
	UploadButton = new MissionUploaderUploadButton( &rect, ButtonFont, "Upload" );
	UploadButton->Enabled = false;
	AddElement( UploadButton );
	
	rect.x = 10;
	AddElement( new MissionUploaderCancelButton( &rect, ButtonFont, "Cancel" ) );
	
	rect.h = ItemFont ? ItemFont->GetHeight() : 18;
	rect.y = 10 + TitleFont->GetAscent() + 10;
	rect.x = 10;
	/*
	rect.w = 120;
	AddElement( new Label( &rect, "Mission File:", ItemFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	*/
	rect.w = Rect.w - rect.x - 10;
	AddElement( new MissionUploaderDropDown( &rect, ItemFont ) );
}


void MissionUploader::Draw( void )
{
	// Keep the size and position up-to-date.
	if( ! Draggable )
	{
		Rect.x = Raptor::Game->Gfx.W/2 - Rect.w/2;
		Rect.y = Raptor::Game->Gfx.H/2 - Rect.h/2;
	}
	
	Window::Draw();
	
	TitleFont->DrawText( "Upload Custom Mission", Rect.w/2 + 2, 3, Font::ALIGN_TOP_CENTER, 0,0,0,0.8f );
	TitleFont->DrawText( "Upload Custom Mission", Rect.w/2,     1, Font::ALIGN_TOP_CENTER );
}


bool MissionUploader::KeyDown( SDLKey key )
{
	if( key == SDLK_F8 )
	{
		Remove();
		return true;
	}
	
	return false;
}


bool MissionUploader::KeyUp( SDLKey key )
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


bool MissionUploader::MouseDown( Uint8 button )
{
	return true;
}


bool MissionUploader::MouseUp( Uint8 button )
{
	return true;
}


// ---------------------------------------------------------------------------


MissionUploaderDropDown::MissionUploaderDropDown( SDL_Rect *rect, Font *font, uint8_t align ) : DropDown( rect, font, align, 0, NULL, NULL )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;

	XWingGame *game = (XWingGame*) Raptor::Game;
	
	if( DIR *dir_p = opendir("Missions") )
	{
		bool allow_stock_reupload = game->Cfg.SettingAsBool("darkside");
		
		while( struct dirent *dir_entry_p = readdir(dir_p) )
		{
			if( dir_entry_p->d_name[ 0 ] == '.' )
				continue;
			if( ! CStr::EndsWith( dir_entry_p->d_name, ".def" ) )
				continue;
			
			std::string mission_file = std::string(dir_entry_p->d_name);
			std::string mission_id = mission_file.substr( 0, mission_file.length() - 4 );
			
			if( (game->MissionList.find( mission_id ) != game->MissionList.end()) && ! allow_stock_reupload )
				continue;
			
			Mission mission;
			if( mission.Load( std::string("Missions/") + mission_file ) )
			{
				if( Items.empty() )
					AddItem( "", " --- Select Mission to Upload --- " );
				
				std::map<std::string,std::string>::iterator name_iter = mission.Properties.find("mission_name");
				if( name_iter != mission.Properties.end() )
					AddItem( mission_id, std::string(" ") + mission_id + std::string(": ") + name_iter->second + std::string(" ") );
				else
					AddItem( mission_id, std::string(" ") + mission_id + std::string(" ") );
			}
		}
		
		closedir( dir_p );
	}
	
	if( Items.empty() )
	{
		Enabled = false;
		LabelText = " --- No Custom Missions Found --- ";
	}
	else
		Update();
}


MissionUploaderDropDown::~MissionUploaderDropDown()
{
}


void MissionUploaderDropDown::Changed( void )
{
	MissionUploader *uploader = (MissionUploader*) FindParent("MissionUploader");
	if( uploader )
	{
		uploader->MissionID = Value;
		uploader->UploadButton->Enabled = Value.length();
	}
}


// ---------------------------------------------------------------------------


MissionUploaderUploadButton::MissionUploaderUploadButton( SDL_Rect *rect, Font *button_font, const char *label ) : LabelledButton( rect, button_font, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
	
	Enabled = false;
}


MissionUploaderUploadButton::~MissionUploaderUploadButton()
{
}


void MissionUploaderUploadButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	MissionUploader *uploader = (MissionUploader*) FindParent("MissionUploader");
	if( uploader && uploader->MissionID.length() )
	{
		std::string mission_path = std::string("Missions/") + uploader->MissionID + std::string(".def");
		std::string mission_data = File::AsString( mission_path );
		size_t mission_data_length = mission_data.length();
		if( mission_data_length && ((uploader->MissionID.length() + mission_data_length) <= 0x70000) )
		{
			Mission mission;
			if( mission.Parse( mission_data ) )
			{
				Packet upload_mission( XWing::Packet::UPLOAD_MISSION );
				upload_mission.AddString( uploader->MissionID );
				upload_mission.AddString( mission_data );
				Raptor::Game->Net.Send( &upload_mission );
				
				uploader->Remove();
				return;  // No error beep on successful upload.
			}
		}
	}
	
	Raptor::Game->Snd.Play( Raptor::Game->Res.GetSound("beep_error.wav") );
}


// ---------------------------------------------------------------------------


MissionUploaderCancelButton::MissionUploaderCancelButton( SDL_Rect *rect, Font *button_font, const char *label ) : LabelledButton( rect, button_font, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Name = "MissionUploaderCancelButton";
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


MissionUploaderCancelButton::~MissionUploaderCancelButton()
{
}


void MissionUploaderCancelButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Container->Remove();
}
