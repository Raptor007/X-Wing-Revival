/*
 *  FleetMenu.cpp
 */

#include "FleetMenu.h"

#include "GroupBox.h"
#include "Label.h"
#include "Num.h"
#include "XWingDefs.h"
#include "XWingGame.h"
#include "ShipClass.h"


FleetMenu::FleetMenu( void )
{
	Name = "FleetMenu";
	
	Rect.x = Raptor::Game->Gfx.W/2 - 512/2;
	Rect.y = Raptor::Game->Gfx.H/2 - 384/2;
	Rect.w = 512;
	Rect.h = 384;
	
	Red = 0.f;
	Green = 0.f;
	Blue = 1.f;
	Alpha = 0.5f;
	
	TitleFont  = Raptor::Game->Res.GetFont( "Verdana.ttf", 30 );
	LabelFont  = Raptor::Game->Res.GetFont( "Verdana.ttf", 16 );
	ItemFont   = Raptor::Game->Res.GetFont( "Verdana.ttf", 16 );
	ButtonFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 30 );
	
	const Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
	Admin = (player && player->PropertyAsBool("admin"));
	
	UpdateContents();
}


FleetMenu::~FleetMenu()
{
}


void FleetMenu::UpdateContents( void )
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
	AddElement( new FleetMenuDoneButton( &rect, ButtonFont, "Done" ) );
	
	rect.x -= (rect.w + 10);
	DefaultsButton = new FleetMenuDefaultsButton( &rect, ButtonFont, "Defaults" );
	AddElement( DefaultsButton );
	
	// --------------------------------------------------------------------------------------------------------------------
	// Fleet Battle Ships
	
	group_rect.x = 10;
	group_rect.y = 40;
	group_rect.w = Rect.w - 20;
	group_rect.h = 299;
	group = new GroupBox( &group_rect, "Fleet Battle Ships", ItemFont );
	AddElement( group );
	
	rect.h = ItemFont ? ItemFont->GetHeight() : 18;
	rect.y = 10 + group->TitleFont->GetAscent();
	rect.x = 10;
	rect.w = 90;
	group->AddElement( new Label( &rect, "Fighter:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 185;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "rebel_fighter" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	rect.x += rect.w + 5;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "empire_fighter" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	
	rect.y += rect.h + 10;
	rect.x = 10;
	rect.w = 90;
	group->AddElement( new Label( &rect, "Bomber:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 185;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "rebel_bomber" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	rect.x += rect.w + 5;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "empire_bomber" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	
	rect.y += rect.h + 10;
	rect.x = 10;
	rect.w = 90;
	group->AddElement( new Label( &rect, "Cruiser:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 185;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "rebel_cruiser" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	rect.x += rect.w + 5;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "empire_cruiser" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	
	rect.y += rect.h + 10;
	rect.x = 10;
	rect.w = 90;
	group->AddElement( new Label( &rect, "Battleship:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 185;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "rebel_frigate" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	rect.x += rect.w + 5;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "empire_frigate" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	
	// FIXME: Move rebel_cruisers/empire_cruisers/etc here from LobbyMenu?
	
	rect.y += rect.h + 10;
	rect.x = 10;
	rect.w = 90;
	group->AddElement( new Label( &rect, "Flagship:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 185;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "rebel_flagship" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	rect.x += rect.w + 5;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "empire_flagship" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	
	group->Rect.h = rect.y + rect.h + 10;
	group_rect.h = group->Rect.h;
	
	// --------------------------------------------------------------------------------------------------------------------
	// Battle of Yavin Ships
	
	group_rect.y += group_rect.h + 7;
	group = new GroupBox( &group_rect, "Battle of Yavin Ships", ItemFont );
	AddElement( group );
	
	rect.h = ItemFont ? ItemFont->GetHeight() : 18;
	rect.y = 10 + group->TitleFont->GetAscent();
	rect.x = 10;
	rect.w = 90;
	group->AddElement( new Label( &rect, "Fighter:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 185;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "yavin_rebel_fighter" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	rect.x += rect.w + 5;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "yavin_empire_fighter" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	
	rect.y += rect.h + 10;
	rect.x = 10;
	rect.w = 90;
	group->AddElement( new Label( &rect, "Bomber:", LabelFont, Font::ALIGN_MIDDLE_LEFT ) );
	rect.x += rect.w + 5;
	rect.w = 185;
	group->AddElement( new FleetMenuDropDown( &rect, ItemFont, "yavin_rebel_bomber" ) );
	DropDowns.insert( (FleetMenuDropDown*) *(group->Elements.rbegin()) );
	
	group->Rect.h = rect.y + rect.h + 10;
	group_rect.h = group->Rect.h;
}


void FleetMenu::Draw( void )
{
	// Keep the size and position up-to-date.
	if( ! Draggable )
	{
		Rect.x = Raptor::Game->Gfx.W/2 - 512/2;
		Rect.y = Raptor::Game->Gfx.H/2 - 384/2;
	}
	Rect.w = 512;
	Rect.h = 384;
	
	// Enforce permissions about who can alter game settings.
	bool enabled = Admin || (Raptor::Game->Data.PropertyAsString("permissions") == "all");
	if( enabled )
	{
		// Don't allow changing fleet ships while a game is in progress.
		for( std::map<uint32_t,GameObject*>::const_iterator obj = Raptor::Game->Data.GameObjects.begin(); obj != Raptor::Game->Data.GameObjects.end(); obj ++ )
		{
			if( obj->second->Type() != XWing::Object::SHIP_CLASS )
			{
				enabled = false;
				break;
			}
		}
	}
	for( std::set<FleetMenuDropDown*>::iterator dropdown_iter = DropDowns.begin(); dropdown_iter != DropDowns.end(); dropdown_iter ++ )
	{
		(*dropdown_iter)->Enabled = enabled;
		(*dropdown_iter)->AlphaOver = enabled ? 1.f : (*dropdown_iter)->AlphaNormal;
	}
	DefaultsButton->Visible = DefaultsButton->Enabled = enabled;
	DefaultsButton->AlphaOver = enabled ? 1.f : DefaultsButton->AlphaNormal;
	
	Window::Draw();
	
	TitleFont->DrawText( "Fleet Configuration", Rect.w/2 + 2, 3, Font::ALIGN_TOP_CENTER, 0,0,0,0.8f );
	TitleFont->DrawText( "Fleet Configuration", Rect.w/2,     1, Font::ALIGN_TOP_CENTER );
}


bool FleetMenu::KeyDown( SDLKey key )
{
	if( key == SDLK_F11 )
	{
		Remove();
		return true;
	}
	
	return false;
}


bool FleetMenu::KeyUp( SDLKey key )
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


bool FleetMenu::MouseDown( Uint8 button )
{
	return true;
}


bool FleetMenu::MouseUp( Uint8 button )
{
	return true;
}


// ---------------------------------------------------------------------------


FleetMenuDropDown::FleetMenuDropDown( SDL_Rect *rect, Font *font, std::string variable ) : DropDown( rect, font, Font::ALIGN_MIDDLE_CENTER, 0, NULL, NULL )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
	
	Name = variable;
	Value = Raptor::Game->Data.PropertyAsString( Name );
	
	std::set<uint8_t> teams, categories;
	teams.insert( XWing::Team::NONE );
	teams.insert( (Str::FindInsensitive( Name, "rebel_" ) >= 0) ? XWing::Team::REBEL : XWing::Team::EMPIRE );
	if( (Str::FindInsensitive( Name, "_fighter" ) >= 0) || (Str::FindInsensitive( Name, "_bomber" ) >= 0) || (Str::FindInsensitive( Name, "_cruiser" ) >= 0) )
	{
		categories.insert( ShipClass::CATEGORY_FIGHTER );
		categories.insert( ShipClass::CATEGORY_BOMBER );
		categories.insert( ShipClass::CATEGORY_GUNBOAT );
	}
	if( (Str::FindInsensitive( Name, "_flagship" ) >= 0) || (Str::FindInsensitive( Name, "_cruiser" ) >= 0) || (Str::FindInsensitive( Name, "_frigate" ) >= 0) )
		categories.insert( ShipClass::CATEGORY_CAPITAL );
	
	std::set<std::string> special_allowed;
	if( variable == "rebel_flagship" )
		special_allowed.insert( "YT1300" );
	else if( variable == "empire_flagship" )
		special_allowed.insert( "T/A" );
	
	bool darkside = (Raptor::Game->Cfg.SettingAsBool("darkside",false) || Raptor::Game->Data.PropertyAsBool("darkside",false)) && ! Raptor::Game->Data.PropertyAsBool("lightside",false);
	
	for( std::map<uint32_t,GameObject*>::const_iterator obj_iter = Raptor::Game->Data.GameObjects.begin(); obj_iter != Raptor::Game->Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->Type() == XWing::Object::SHIP_CLASS )
		{
			const ShipClass *sc = (const ShipClass*) obj_iter->second;
			if( darkside || (sc->ShortName == Value) || special_allowed.count(sc->ShortName)
			|| (teams.count(sc->Team) && categories.count(sc->Category) && ! sc->Secret) )
				AddItem( sc->ShortName, sc->LongName );
		}
	}
	
	Update();
}


FleetMenuDropDown::~FleetMenuDropDown()
{
}


void FleetMenuDropDown::Changed( void )
{
	bool enabled = ((FleetMenu*)( Container->Container ))->Admin || (Raptor::Game->Data.PropertyAsString("permissions") == "all");
	if( ! enabled )
		return;
	
	Packet info = Packet( Raptor::Packet::INFO );
	info.AddUShort( 1 );
	info.AddString( Name );
	info.AddString( Value );
	Raptor::Game->Net.Send( &info );
}


// ---------------------------------------------------------------------------


FleetMenuDoneButton::FleetMenuDoneButton( SDL_Rect *rect, Font *button_font, const char *label ) : LabelledButton( rect, button_font, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


FleetMenuDoneButton::~FleetMenuDoneButton()
{
}


void FleetMenuDoneButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	Container->Remove();
}


// ---------------------------------------------------------------------------


FleetMenuDefaultsButton::FleetMenuDefaultsButton( SDL_Rect *rect, Font *button_font, const char *label ) : LabelledButton( rect, button_font, label, Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Name = "FleetMenuDefaultsButton";
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


FleetMenuDefaultsButton::~FleetMenuDefaultsButton()
{
}


void FleetMenuDefaultsButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	bool enabled = ((FleetMenu*)( Container ))->Admin || (Raptor::Game->Data.PropertyAsString("permissions") == "all");
	if( ! enabled )
		return;
	
	FleetMenuDropDown *rebel_fighter = (FleetMenuDropDown*) Container->FindElement( "rebel_fighter" );
	if( rebel_fighter )
		rebel_fighter->Select( "X/W" );
	FleetMenuDropDown *rebel_bomber = (FleetMenuDropDown*) Container->FindElement( "rebel_bomber" );
	if( rebel_bomber )
		rebel_bomber->Select( "Y/W" );
	FleetMenuDropDown *rebel_cruiser = (FleetMenuDropDown*) Container->FindElement( "rebel_cruiser" );
	if( rebel_cruiser )
		rebel_cruiser->Select( "CRV" );
	FleetMenuDropDown *rebel_frigate = (FleetMenuDropDown*) Container->FindElement( "rebel_frigate" );
	if( rebel_frigate )
		rebel_frigate->Select( "FRG" );
	FleetMenuDropDown *rebel_flagship = (FleetMenuDropDown*) Container->FindElement( "rebel_flagship" );
	if( rebel_flagship )
		rebel_flagship->Select( "FRG" );
	
	FleetMenuDropDown *empire_fighter = (FleetMenuDropDown*) Container->FindElement( "empire_fighter" );
	if( empire_fighter )
		empire_fighter->Select( "T/F" );
	FleetMenuDropDown *empire_bomber = (FleetMenuDropDown*) Container->FindElement( "empire_bomber" );
	if( empire_bomber )
		empire_bomber->Select( "T/B" );
	FleetMenuDropDown *empire_cruiser = (FleetMenuDropDown*) Container->FindElement( "empire_cruiser" );
	if( empire_cruiser )
		empire_cruiser->Select( "INT" );
	FleetMenuDropDown *empire_frigate = (FleetMenuDropDown*) Container->FindElement( "empire_frigate" );
	if( empire_frigate )
		empire_frigate->Select( "FRG" );
	FleetMenuDropDown *empire_flagship = (FleetMenuDropDown*) Container->FindElement( "empire_flagship" );
	if( empire_flagship )
		empire_flagship->Select( "ISD" );
	
	FleetMenuDropDown *yavin_rebel_fighter = (FleetMenuDropDown*) Container->FindElement( "yavin_rebel_fighter" );
	if( yavin_rebel_fighter )
		yavin_rebel_fighter->Select( "X/W" );
	FleetMenuDropDown *yavin_rebel_bomber = (FleetMenuDropDown*) Container->FindElement( "yavin_rebel_bomber" );
	if( yavin_rebel_bomber )
		yavin_rebel_bomber->Select( "Y/W" );
	FleetMenuDropDown *yavin_empire_fighter = (FleetMenuDropDown*) Container->FindElement( "yavin_empire_fighter" );
	if( yavin_empire_fighter )
		yavin_empire_fighter->Select( "T/F" );
}
