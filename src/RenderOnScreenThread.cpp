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
#include "InputHandler.h"
#include "Settings.h"

using namespace OrientView;

void RenderOnScreenThread::initialize(MainWindow* mainWindow, VideoWindow* videoWindow, VideoDecoder* videoDecoder, VideoDecoderThread* videoDecoderThread, VideoStabilizer* videoStabilizer, Renderer* renderer, InputHandler* inputHandler)
{
	this->mainWindow = mainWindow;
	this->videoWindow = videoWindow;
	this->videoDecoder = videoDecoder;
	this->videoDecoderThread = videoDecoderThread;
	this->videoStabilizer = videoStabilizer;
	this->renderer = renderer;
	this->inputHandler = inputHandler;
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
			QThread::msleep(100);
			continue;
		}

		bool gotFrame = false;

		if (!getIsPaused() || getShouldAdvanceOneFrame())
		{
			gotFrame = videoDecoderThread->tryGetNextFrame(&frameData, &frameDataGrayscale, 0);
			setShouldAdvanceOneFrame(false);
		}

		if (gotFrame)
			videoStabilizer->processFrame(&frameDataGrayscale);

		// direct the rendering to the video window
		videoWindow->getContext()->makeCurrent(videoWindow);

		renderer->startRendering(videoWindow->width(), videoWindow->height(), videoDecoder->getCurrentTime(), frameDuration, spareTime, videoDecoder->getLastDecodeTime(), videoStabilizer->getLastProcessTime(), 0.0);

		if (gotFrame)
		{
			renderer->uploadFrameData(&frameData);
			videoDecoderThread->signalFrameRead();
		}

		renderer->renderAll();
		renderer->stopRendering();

		inputHandler->handleInput(frameDuration);

		spareTime = (frameData.duration - (spareTimer.nsecsElapsed() / 1000.0)) / 1000.0;

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

void RenderOnScreenThread::togglePaused()
{
	QMutexLocker locker(&renderOnScreenThreadMutex);

	isPaused = !isPaused;
	shouldAdvanceOneFrame = false;
}

void RenderOnScreenThread::advanceOneFrame()
{
	QMutexLocker locker(&renderOnScreenThreadMutex);

	shouldAdvanceOneFrame = true;
}

bool RenderOnScreenThread::getIsPaused()
{
	QMutexLocker locker(&renderOnScreenThreadMutex);

	return isPaused;
}

bool RenderOnScreenThread::getShouldAdvanceOneFrame()
{
	QMutexLocker locker(&renderOnScreenThreadMutex);

	return shouldAdvanceOneFrame;
}

void RenderOnScreenThread::setShouldAdvanceOneFrame(bool value)
{
	QMutexLocker locker(&renderOnScreenThreadMutex);

	shouldAdvanceOneFrame = value;
}
