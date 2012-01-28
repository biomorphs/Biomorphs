#ifndef SCREENSHOT_HELPER_INCLUDED
#define SCREENSHOT_HELPER_INCLUDED

#include "device_types.h"

class ScreenshotHelper
{
public:
	ScreenshotHelper();
	~ScreenshotHelper();

	void Initialise( Device* d );
	void Release();

	void TakeScreenshot( const char* fileName );

private:

	Texture2D m_stagingTexture;
	Device* m_device;
};

#endif