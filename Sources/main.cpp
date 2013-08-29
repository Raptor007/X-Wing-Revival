/*
 *  Main.cpp
 */

#include "XWingGame.h"
#include "XWingServer.h"

#include "ServerConsole.h"


#define VERSION "0.1 Alpha"


#ifdef WIN32
// Avoid conflicts between 32-bit and 64-bit by using two different DLL directories.
BOOL SetDllDirectoryResult = SetDllDirectory( (sizeof(void*) == 8) ? L"Bin64" : L"Bin32" );
#endif


int main( int argc, char **argv )
{
	bool dedicated = false;
	if( (argc >= 2) && (strcmp( argv[ 1 ], "-dedicated" ) == 0) )
		dedicated = true;
	
	#ifdef WIN32
		bool terminal = false;
		if( (argc >= 2) && (strcmp( argv[ 1 ], "-terminal" ) == 0) )
			terminal = true;
		
		if( (terminal || dedicated) && AllocConsole() )
		{
			freopen( "CONIN$", "rt", stdin );
			freopen( "CONOUT$", "wt", stderr );
			freopen( "CONOUT$", "wt", stdout );
			std::ios::sync_with_stdio();
		}
	#endif
	
	Raptor::Game = new XWingGame( VERSION );
	Raptor::Server = new XWingServer( VERSION );
	Raptor::Game->Console.OutFile = stdout;
	Raptor::Game->SetServer( Raptor::Server );
	
	if( ! dedicated )
	{
		Raptor::Game->Initialize( argc, argv );
		Raptor::Game->Run();
	}
	else
	{
		Raptor::Server->Console = new ServerConsole();
		Raptor::Server->Console->OutFile = stdout;
		Raptor::Server->Start( "Dedicated Server" );
		((ServerConsole*)( Raptor::Server->Console ))->Run();
	}
	
	return 0;
}
