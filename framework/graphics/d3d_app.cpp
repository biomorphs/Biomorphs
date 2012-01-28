#include "d3d_app.h"
#include "device.h"
#include "core\message_pump.h"
#include "core\strings.h"

D3DApp::D3DApp(D3DAppConfig config)
	: m_config(config)
	, m_quit(false)
{
	m_config.m_app = this;
}

D3DApp::~D3DApp()
{
}

bool D3DApp::registerModuleFactories()
{
	return true;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

bool D3DApp::createWindow()
{
	std::wstring className = Strings::StringToWide(m_config.m_windowTitle + "Cls");
	std::wstring windowName = Strings::StringToWide(m_config.m_windowTitle);

	// Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
	wcex.hInstance = m_config.m_hInst;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
	wcex.lpszClassName = className.c_str();
    wcex.hIconSm = NULL;
    if( !RegisterClassEx( &wcex ) )
        return false;

    // Create window
	DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER;

    m_hInst = m_config.m_hInst;
    RECT rc = { 0, 0, 640, 480 };
	rc.right = m_config.m_windowWidth;
	rc.bottom = m_config.m_windowHeight;
    AdjustWindowRect( &rc, windowStyle, FALSE );

	m_window = CreateWindow( className.c_str(), windowName.c_str(), windowStyle,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, m_config.m_hInst,
                           NULL );
    if( !m_window )
        return false;

    ShowWindow( m_window, SW_NORMAL );

	// now the device can access the window
	Device::s_appWindow = m_window;

	return true;
}

bool D3DApp::execute()
{
	if(!init())
	{
		return false;
	}

	m_appTimer.reset();

	MessagePump win32msgPump;
	while( win32msgPump.update() && !m_quit)
	{
		m_appTimer.nextTick();

		m_quit |= !m_moduleManager.update( m_appTimer );
	}

	return cleanup();
}	

bool D3DApp::init()
{
	if(!createWindow())
	{
		return false;
	}

	if( !registerModuleFactories() )
	{
		return false;
	}

	m_moduleManager.appendFromFile( "modules.xml" );
	if(!m_moduleManager.connect())
	{
		return false;
	}

	if(!m_moduleManager.postConnect())
	{
		return false;
	}

	if(!m_moduleManager.startup())
	{
		return 0;
	}

    return true;
}

bool D3DApp::cleanup()
{
	m_moduleManager.shutdown();

	return true;
}