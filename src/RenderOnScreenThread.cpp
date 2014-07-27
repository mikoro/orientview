// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QElapsedTimer>
#include <QOpenGLPixelTransferOptions>

#include "RenderOnScreenThread.h"
#include "MainWindow.h"
#include "VideoWindow.h"
#include "VideoRenderer.h"
#include "VideoDecoderThread.h"
#include "FrameData.h"

using namespace OrientView;

RenderOnScreenThread::RenderOnScreenThread()
{
}

bool RenderOnScreenThread::initialize(MainWindow* mainWindow, VideoWindow* videoWindow, VideoDecoderThread* videoDecoderThread, VideoRenderer* videoRenderer)
{
	qDebug("Initializing RenderOnScreenThread");

	this->mainWindow = mainWindow;
	this->videoWindow = videoWindow;
	this->videoDecoderThread = videoDecoderThread;
	this->videoRenderer = videoRenderer;

	return true;
}

void RenderOnScreenThread::shutdown()
{
	qDebug("Shutting down RenderOnScreenThread");
}

void RenderOnScreenThread::run()
{
	FrameData frameData;
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		if (videoDecoderThread->getNextFrame(&frameData))
		{
			options.setRowLength(frameData.rowLength / 4);
			options.setImageHeight(frameData.height);
			options.setAlignment(1);

			videoRenderer->getVideoPanelTexture()->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, frameData.data, &options);
			videoDecoderThread->signalFrameRead();

			videoRenderer->update(videoWindow->width(), videoWindow->height());
			videoRenderer->render();

			// use combination of normal and spinning wait to sync the frame rate accurately
			while (true)
			{
				int64_t timeToSleep = frameData.duration - (displayTimer.nsecsElapsed() / 1000);

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

	videoWindow->getContext()->doneCurrent();
	videoWindow->getContext()->moveToThread(mainWindow->thread());
}
