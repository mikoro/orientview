// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QElapsedTimer>
#include <QTime>

#include "RenderOnScreenThread.h"
#include "MainWindow.h"
#include "VideoWindow.h"
#include "VideoDecoder.h"
#include "VideoDecoderThread.h"
#include "VideoStabilizer.h"
#include "Renderer.h"
#include "Settings.h"

using namespace OrientView;

RenderOnScreenThread::RenderOnScreenThread()
{
}

bool RenderOnScreenThread::initialize(MainWindow* mainWindow, VideoWindow* videoWindow, VideoDecoder* videoDecoder, VideoDecoderThread* videoDecoderThread, VideoStabilizer* videoStabilizer, Renderer* renderer)
{
	qDebug("Initializing RenderOnScreenThread");

	this->mainWindow = mainWindow;
	this->videoWindow = videoWindow;
	this->videoDecoder = videoDecoder;
	this->videoDecoderThread = videoDecoderThread;
	this->videoStabilizer = videoStabilizer;
	this->renderer = renderer;

	paused = false;
	shouldAdvanceOneFrame = false;

	return true;
}

void RenderOnScreenThread::shutdown()
{
	qDebug("Shutting down RenderOnScreenThread");
}

void RenderOnScreenThread::run()
{
	FrameData frameData;
	FrameData frameDataGrayscale;

	QElapsedTimer displaySyncTimer;
	QElapsedTimer spareTimer;

	double frameDuration = 0.1;
	double spareTime = 0.0;

	displaySyncTimer.start();
	spareTimer.start();

	while (!isInterruptionRequested())
	{
		if (!videoWindow->isExposed())
		{
			QThread::msleep(20);
			continue;
		}

		bool gotFrame = false;

		if (!paused || shouldAdvanceOneFrame)
		{
			gotFrame = videoDecoderThread->tryGetNextFrame(&frameData, &frameDataGrayscale, 0);
			shouldAdvanceOneFrame = false;
		}

		if (gotFrame)
			videoStabilizer->processFrame(&frameDataGrayscale);

		videoWindow->getContext()->makeCurrent(videoWindow);
		renderer->startRendering(videoWindow->width(), videoWindow->height(), frameDuration, spareTime);

		if (gotFrame)
		{
			renderer->uploadFrameData(&frameData);
			videoDecoderThread->signalFrameRead();
		}

		renderer->renderAll();
		renderer->stopRendering();

		spareTime = videoDecoder->getAverageFrameDuration() - (spareTimer.nsecsElapsed() / 1000000.0);

		renderer->handleInput();

		// use combination of normal and spinning wait to sync the frame rate accurately
		while (true)
		{
			int64_t timeToSleep = frameData.duration - (displaySyncTimer.nsecsElapsed() / 1000);

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

		frameDuration = displaySyncTimer.nsecsElapsed() / 1000000.0;
		displaySyncTimer.restart();
		spareTimer.restart();

		videoWindow->getContext()->swapBuffers(videoWindow);
	}

	videoWindow->getContext()->doneCurrent();
	videoWindow->getContext()->moveToThread(mainWindow->thread());
}

bool RenderOnScreenThread::isPaused()
{
	return paused;
}

void RenderOnScreenThread::togglePaused()
{
	paused = !paused;
	shouldAdvanceOneFrame = false;
}

void RenderOnScreenThread::advanceOneFrame()
{
	shouldAdvanceOneFrame = true;
}
