/*
 *  Main.cpp
 */

#include "XWingGame.h"
#include "XWingServer.h"

#include "ServerConsole.h"

#if defined(__APPLE__) && (__GNUC__ >= 4) && ((__GNUC__ > 4) || (__GNUC_MINOR__ > 0))
#include <libproc.h>
#include <unistd.h>
#endif


#define VERSION "0.1.3 Alpha"


#ifdef WIN32
// Set DLL directory to Bin32/Bin64 and make sure the working directory is correct.
// This must happen before main, or DLLs will fail to link!
BOOL windows_init_result = Raptor::WindowsInit();
#endif


int main( int argc, char **argv )
{
	#if defined(__APPLE__) && defined(PROC_PIDPATHINFO_MAXSIZE)
		// Fix the working directory for Mac OS X Intel64.
		char exe_path[ PROC_PIDPATHINFO_MAXSIZE ] = "";
		pid_t pid = getpid();
		if( proc_pidpath( pid, exe_path, sizeof(exe_path) ) > 0 )
		{
			char *real_path = realpath( exe_path, NULL );
			std::string path;
			if( real_path )
			{
				path = real_path;
				free( real_path );
				real_path = NULL;
			}
			else
				path = exe_path;
			
			size_t index = path.rfind( ".app/Contents/MacOS/" );
			if( index != std::string::npos )
			{
				path = path.substr( 0, index );
				index = path.rfind( "/" );
				if( index != std::string::npos )
				{
					path = path.substr( 0, index );
					chdir( path.c_str() );
				}
			}
		}
	#endif
	
	bool dedicated = false;
	if( (argc >= 2) && (strcmp( argv[ 1 ], "-dedicated" ) == 0) )
		dedicated = true;
	
	#ifdef WIN32
		bool terminal = false;
		if( (argc >= 2) && (strcmp( argv[ 1 ], "-terminal" ) == 0) )
			terminal = true;
		
		// Create a Windows command-prompt-style console window.
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
