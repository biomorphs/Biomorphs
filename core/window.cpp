#include "core/window.h"

Window::Window()
{
}

Window::~Window()
{
}

void Window::onPaint(HDC hdc)
{

}

LRESULT CALLBACK Window::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

	Window* window=(Window*) GetWindowLong (hWnd, 0);
    switch( message )
    {
		case WM_SIZE:
		{
			if( window )
			{
				window->resize( LOWORD(lParam), HIWORD(lParam) );
			}
			break;
		}

        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
			if( window )
			{
				window->onPaint(hdc);
			}
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

void Window::redraw()
{
	InvalidateRect ( m_hWnd, NULL, TRUE );
}

bool Window::createWindow( WindowProperties& initValues )
{
	// Register class
    WNDCLASSEX wcex;
	memset( &wcex, 0, sizeof(WNDCLASSEX) );
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(PVOID);
	wcex.hInstance = initValues.AppInstance;
    wcex.hbrBackground = ( HBRUSH )( COLOR_BACKGROUND );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"WindowClass";

    if( !RegisterClassEx( &wcex ) )
        return false;

	wchar_t* wideTitle = (wchar_t*)malloc( sizeof(wchar_t) * 256 );
	size_t charsReturned=0;
	mbstowcs_s( &charsReturned, wideTitle, sizeof(wchar_t) * 64, initValues.TitleText.c_str(), initValues.TitleText.length() );

    // Create window
	RECT rc = { 0, 0, initValues.Width, initValues.Height };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	m_hWnd = CreateWindow( L"WindowClass", wideTitle, WS_OVERLAPPEDWINDOW,
						   CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, initValues.AppInstance,
                           NULL );
    if( !m_hWnd )
        return false;

	SetWindowLong (m_hWnd, 0, (LONG)this );

	SetWindowText( m_hWnd, wideTitle );

	free( wideTitle );

	m_properties = initValues;

	return true;
}

void Window::resize( int w, int h )
{
	m_properties.Width=w;
	m_properties.Height=h;
}

void Window::show()
{
	ShowWindow( m_hWnd, SW_SHOW );
	UpdateWindow(m_hWnd);

}

unsigned int Window::getWidthPixels()
{
	return m_properties.Width;
}

unsigned int Window::getHeightPixels()
{
	return m_properties.Height;
}

void Window::hide()
{
	ShowWindow( m_hWnd, SW_HIDE );
}