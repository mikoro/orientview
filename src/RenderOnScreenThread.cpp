// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QElapsedTimer>

#include "RenderOnScreenThread.h"
#include "MainWindow.h"
#include "VideoWindow.h"
#include "VideoDecoder.h"
#include "VideoDecoderThread.h"
#include "VideoStabilizer.h"
#include "VideoRenderer.h"
#include "Settings.h"

using namespace OrientView;

RenderOnScreenThread::RenderOnScreenThread()
{
}

bool RenderOnScreenThread::initialize(MainWindow* mainWindow, VideoWindow* videoWindow, VideoDecoder* videoDecoder, VideoDecoderThread* videoDecoderThread, VideoStabilizer* videoStabilizer, VideoRenderer* videoRenderer, Settings* settings)
{
	qDebug("Initializing RenderOnScreenThread");

	this->mainWindow = mainWindow;
	this->videoWindow = videoWindow;
	this->videoDecoder = videoDecoder;
	this->videoDecoderThread = videoDecoderThread;
	this->videoStabilizer = videoStabilizer;
	this->videoRenderer = videoRenderer;

	stabilizationEnabled = settings->stabilization.enabled;
	renderInfoPanel = settings->appearance.showInfoPanel;

	return true;
}

void RenderOnScreenThread::shutdown()
{
	qDebug("Shutting down RenderOnScreenThread");
}

void RenderOnScreenThread::toggleRenderInfoPanel()
{
	renderInfoPanel = !renderInfoPanel;
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

		bool gotFrame = videoDecoderThread->tryGetNextFrame(&frameData, &frameDataGrayscale);

		if (gotFrame && stabilizationEnabled)
			videoStabilizer->processFrame(&frameDataGrayscale);

		videoWindow->getContext()->makeCurrent(videoWindow);
		videoRenderer->startRendering(videoWindow->width(), videoWindow->height(), frameDuration);

		if (gotFrame)
		{
			videoRenderer->uploadFrameData(&frameData);
			videoDecoderThread->signalFrameRead();
			videoRenderer->renderVideoPanel();
		}

		videoRenderer->renderMapPanel();

		if (renderInfoPanel)
			videoRenderer->renderInfoPanel(spareTime);

		videoRenderer->stopRendering();

		spareTime = videoDecoder->getVideoInfo().averageFrameDuration - (spareTimer.nsecsElapsed() / 1000000.0);

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
