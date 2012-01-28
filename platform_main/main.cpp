#include "core/config.h"
#include "core/random.h"

#include "biomorphs\app.h"

#include <Windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>

// maximum mumber of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;
bool registerConsole()
{
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
		&coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well
	std::ios::sync_with_stdio();

	return true;
}

bool parseD3DConfig( HINSTANCE hInst, Config& cfg, D3DAppConfig& config )
{
	ConfigElement<unsigned int>* Width = NULL;
	ConfigElement<unsigned int>* Height = NULL;
	ConfigElement<unsigned int>* Bpp = NULL;
	ConfigElement<std::string>* Name = NULL;
	ConfigElement<bool>* Fullscreen = NULL;

	cfg.getElement("Application.Window.WindowTitle", &Name);
	cfg.getElement("Application.Window.Width", &Width);
	cfg.getElement("Application.Window.Height", &Height);
	cfg.getElement("Application.Window.Bpp", &Bpp);
	cfg.getElement("Application.Window.Fullscreen", &Fullscreen);

	if( Name && Width && Height && Bpp && Fullscreen )
	{
		config = D3DAppConfig( Width->getValue(), Height->getValue(), Bpp->getValue(), Fullscreen->getValue(), Name->getValue(), hInst );

		return true;
	}

	return false;
}

int __stdcall WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR    lpCmdLine,
                     int       nCmdShow)
{
	Config appConfig;
	appConfig.appendFromFile( "app_config.xml" );

	ConfigElement<bool>* HasConsole = NULL;
	appConfig.getElement("Application.Window.HasConsole", &HasConsole);

	if( HasConsole->getValue() )
	{
		if(!registerConsole())
		{
			return 0;
		}	
	}

	D3DAppConfig config;
	if( !parseD3DConfig( hInstance, appConfig, config ) )
	{
		return 0;
	}
	
	App myApp( config );
	return myApp.execute();
}