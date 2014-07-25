// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QElapsedTimer>
#include <QOpenGLPixelTransferOptions>

#include "RenderOnScreenThread.h"

using namespace OrientView;

RenderOnScreenThread::RenderOnScreenThread()
{
}

void RenderOnScreenThread::initialize(VideoWindow* videoWindow, VideoRenderer* videoRenderer, VideoDecoder* videoDecoder)
{
	this->videoWindow = videoWindow;
	this->videoRenderer = videoRenderer;
	this->videoDecoder = videoDecoder;
}

void RenderOnScreenThread::run()
{
	DecodedFrame decodedFrame;
	QOpenGLPixelTransferOptions options;
	QElapsedTimer displayTimer;

	displayTimer.start();

	while (!isInterruptionRequested())
	{
		if (videoWindow->isExposed() && videoDecoder->getNextFrame(&decodedFrame))
		{
			videoWindow->getContext()->makeCurrent(videoWindow);

			options.setRowLength(decodedFrame.stride / 4);
			options.setImageHeight(decodedFrame.height);
			options.setAlignment(1);

			videoRenderer->getVideoPanelTexture()->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, decodedFrame.data, &options);

			glViewport(0, 0, videoWindow->width(), videoWindow->height());
			videoRenderer->update(videoWindow->width(), videoWindow->height());
			videoRenderer->render();

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

			displayTimer.restart();
			videoWindow->getContext()->swapBuffers(videoWindow);

			continue;
		}

		QThread::msleep(100);
	}

	videoWindow->getContext()->makeCurrent(videoWindow);
	videoRenderer->shutdown();
}
