#ifndef WINDOW_INCLUDED
#define WINDOW_INCLUDED

#include <string>
#include <Windows.h>

class Window
{
public:
	Window();
	virtual ~Window();

	void show();
	void hide();

	struct WindowProperties
	{
		WindowProperties( HINSTANCE appInstance, unsigned int width, unsigned int height, std::string titleText )
			: Width(width), Height(height), TitleText(titleText), AppInstance(appInstance)
		{
		}

		WindowProperties()
			: Width(640), Height(480)
		{
		}

		unsigned int Width;
		unsigned int Height;
		HINSTANCE AppInstance;
		std::string TitleText;
	};

	bool createWindow( WindowProperties& initValues );
	inline HWND getWindowHandle() const
	{
		return m_hWnd;
	}

	void redraw();

	unsigned int getWidthPixels();
	unsigned int getHeightPixels();

	void resize( int w, int h );
	
protected:
	WindowProperties m_properties;

	virtual void onPaint(HDC hdc);
	static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

private:
	HWND m_hWnd;
};

#endif