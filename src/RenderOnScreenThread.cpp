// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QElapsedTimer>
#include <QOpenGLPixelTransferOptions>

#include "RenderOnScreenThread.h"
#include "VideoWindow.h"
#include "VideoRenderer.h"
#include "VideoDecoderThread.h"
#include "DecodedFrame.h"

using namespace OrientView;

RenderOnScreenThread::RenderOnScreenThread()
{
}

bool RenderOnScreenThread::initialize(VideoWindow* videoWindow, VideoRenderer* videoRenderer, VideoDecoderThread* videoDecoderThread)
{
	qDebug("Initializing RenderOnScreenThread");

	this->videoWindow = videoWindow;
	this->videoRenderer = videoRenderer;
	this->videoDecoderThread = videoDecoderThread;

	return true;
}

void RenderOnScreenThread::shutdown()
{
	qDebug("Shutting down RenderOnScreenThread");
}

void RenderOnScreenThread::run()
{
	DecodedFrame decodedFrame;
	QOpenGLPixelTransferOptions options;
	QElapsedTimer displayTimer;

	displayTimer.start();

	while (!isInterruptionRequested())
	{
		if (!videoWindow->isExposed())
		{
			QThread::msleep(20);
			continue;
		}

		videoWindow->getContext()->makeCurrent(videoWindow);
		glViewport(0, 0, videoWindow->width(), videoWindow->height());
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (videoDecoderThread->getDecodedFrame(&decodedFrame))
		{
			options.setRowLength(decodedFrame.stride / 4);
			options.setImageHeight(decodedFrame.height);
			options.setAlignment(1);

			videoRenderer->getVideoPanelTexture()->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, decodedFrame.data, &options);
			videoDecoderThread->signalProcessingFinished();

			videoRenderer->update(videoWindow->width(), videoWindow->height());
			videoRenderer->render();

			// use combination of normal and spinning wait to sync the frame rate accurately
			while (true)
			{
				int64_t timeToSleep = decodedFrame.duration - (displayTimer.nsecsElapsed() / 1000);

				if (timeToSleep > 2000)
				{
					QThread::msleep(1);
					continue;
				}
				else if (timeToSleep > 0)
					continue;
				else
					break;
			}
		}

		displayTimer.restart();
		videoWindow->getContext()->swapBuffers(videoWindow);
	}

	videoWindow->getContext()->makeCurrent(videoWindow);
	videoRenderer->shutdown();
}
