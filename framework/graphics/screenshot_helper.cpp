#include "screenshot_helper.h"
#include "device.h"

ScreenshotHelper::ScreenshotHelper()
{
	m_stagingTexture.Invalidate();
}

ScreenshotHelper::~ScreenshotHelper()
{
}

void ScreenshotHelper::Initialise( Device* d )
{
	// get the back buffer so we can grab its parameters
	Texture2D backBuffer = d->GetBackBufferTexture();

	// now make a staging texture with similar properties
	Texture2D::Parameters stagingParams;
	stagingParams.access = Texture2D::CpuRead;
	stagingParams.bindFlags = 0;
	stagingParams.format = backBuffer.GetParameters().format;
	stagingParams.height = backBuffer.GetParameters().height;
	stagingParams.msaaCount = backBuffer.GetParameters().msaaCount;
	stagingParams.msaaQuality = backBuffer.GetParameters().msaaQuality;
	stagingParams.numMips = 1;
	stagingParams.width = backBuffer.GetParameters().width;
	stagingParams.stagingTexture = true;
	m_stagingTexture = d->CreateTexture( stagingParams );

	m_device = d;
}

void ScreenshotHelper::Release()
{
	m_device->Release( m_stagingTexture );
}

void ScreenshotHelper::TakeScreenshot( const char* fileName )
{
	// build the full file name
	char fullFilename[512] = {'\0'};
	sprintf_s(fullFilename, "%s.jpg", fileName);

	// Copy the back buffer to the staging texture (so we can read it)
	m_device->CopyTextureToTexture( m_device->GetBackBufferTexture(), m_stagingTexture );

	// Finally, write the texture to the file
	m_device->SaveTextureToFile( m_stagingTexture, fullFilename, Device::TextureTypeJPG );
}