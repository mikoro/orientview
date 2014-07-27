// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLPixelTransferOptions>

#include "RenderOffScreenThread.h"
#include "MainWindow.h"
#include "EncodeWindow.h"
#include "VideoDecoderThread.h"
#include "VideoRenderer.h"
#include "FrameData.h"

using namespace OrientView;

RenderOffScreenThread::RenderOffScreenThread()
{
}

bool RenderOffScreenThread::initialize(MainWindow* mainWindow, EncodeWindow* encodeWindow, VideoDecoderThread* videoDecoderThread, VideoRenderer* videoRenderer)
{
	qDebug("Initializing RenderOffScreenThread");

	this->mainWindow = mainWindow;
	this->encodeWindow = encodeWindow;
	this->videoDecoderThread = videoDecoderThread;
	this->videoRenderer = videoRenderer;

	framebufferWidth = 1280;
	framebufferHeight = 720;

	QOpenGLFramebufferObjectFormat fboFormat;
	fboFormat.setSamples(4);
	fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

	framebuffer = new QOpenGLFramebufferObject(framebufferWidth, framebufferHeight, fboFormat);

	if (!framebuffer->isValid())
	{
		qWarning("Could not create normal framebuffer");
		return false;
	}

	QOpenGLFramebufferObjectFormat convertFboFormat;
	convertFboFormat.setSamples(0);
	convertFboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

	convertFramebuffer = new QOpenGLFramebufferObject(framebufferWidth, framebufferHeight, convertFboFormat);

	if (!convertFramebuffer->isValid())
	{
		qWarning("Could not create convert framebuffer");
		return false;
	}

	return true;
}

void RenderOffScreenThread::shutdown()
{
	qDebug("Shutting down RenderOffScreenThread");

	if (convertFramebuffer != nullptr)
	{
		delete convertFramebuffer;
		convertFramebuffer = nullptr;
	}

	if (framebuffer != nullptr)
	{
		delete framebuffer;
		framebuffer = nullptr;
	}
}

void RenderOffScreenThread::run()
{
	FrameData frameData;
	QOpenGLPixelTransferOptions options;

	while (!isInterruptionRequested())
	{
		encodeWindow->getContext()->makeCurrent(encodeWindow->getSurface());
		framebuffer->bind();

		glViewport(0, 0, framebufferWidth, framebufferHeight);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (videoDecoderThread->getNextFrame(&frameData))
		{
			options.setRowLength(frameData.rowLength / 4);
			options.setImageHeight(frameData.height);
			options.setAlignment(1);

			videoRenderer->getVideoPanelTexture()->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, frameData.data, &options);
			videoDecoderThread->signalFrameRead();

			videoRenderer->update(1280, 720);
			videoRenderer->render();

			QImage result = framebuffer->toImage();
			result.save("test.jpg");

			break;
		}
	}

	encodeWindow->getContext()->doneCurrent();
	encodeWindow->getContext()->moveToThread(mainWindow->thread());
}
