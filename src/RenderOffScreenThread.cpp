// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QOpenGLPixelTransferOptions>

#include "RenderOffScreenThread.h"
#include "EncodeWindow.h"
#include "VideoRenderer.h"
#include "VideoDecoderThread.h"
#include "DecodedFrame.h"

using namespace OrientView;

RenderOffScreenThread::RenderOffScreenThread()
{
}

bool RenderOffScreenThread::initialize(EncodeWindow* encodeWindow, VideoRenderer* videoRenderer, VideoDecoderThread* videoDecoderThread)
{
	qDebug("Initializing RenderOffScreenThread");

	this->encodeWindow = encodeWindow;
	this->videoRenderer = videoRenderer;
	this->videoDecoderThread = videoDecoderThread;

	return true;
}

void RenderOffScreenThread::shutdown()
{
	qDebug("Shutting down RenderOffScreenThread");
}

void RenderOffScreenThread::run()
{
	DecodedFrame decodedFrame;
	QOpenGLPixelTransferOptions options;

	while (!isInterruptionRequested())
	{
		encodeWindow->getContext()->makeCurrent(encodeWindow->getSurface());
		encodeWindow->getFramebuffer()->bind();

		glViewport(0, 0, 1280, 720);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (videoDecoderThread->getDecodedFrame(&decodedFrame))
		{
			options.setRowLength(decodedFrame.stride / 4);
			options.setImageHeight(decodedFrame.height);
			options.setAlignment(1);

			videoRenderer->getVideoPanelTexture()->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, decodedFrame.data, &options);
			videoDecoderThread->signalProcessingFinished();

			videoRenderer->update(1280, 720);
			videoRenderer->render();

			QImage result = encodeWindow->getFramebuffer()->toImage();
			result.save("test.jpg");

			break;
		}
	}

	encodeWindow->getContext()->makeCurrent(encodeWindow->getSurface());
	videoRenderer->shutdown();
}
