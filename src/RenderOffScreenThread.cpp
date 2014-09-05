// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QElapsedTimer>

#include "RenderOffScreenThread.h"
#include "MainWindow.h"
#include "EncodeWindow.h"
#include "VideoDecoder.h"
#include "VideoDecoderThread.h"
#include "VideoStabilizer.h"
#include "RouteManager.h"
#include "Renderer.h"
#include "VideoEncoder.h"
#include "FrameData.h"

using namespace OrientView;

void RenderOffScreenThread::initialize(MainWindow* mainWindow, EncodeWindow* encodeWindow, VideoDecoder* videoDecoder, VideoDecoderThread* videoDecoderThread, VideoStabilizer* videoStabilizer, RouteManager* routeManager, Renderer* renderer, VideoEncoder* videoEncoder)
{
	this->mainWindow = mainWindow;
	this->encodeWindow = encodeWindow;
	this->videoDecoder = videoDecoder;
	this->videoDecoderThread = videoDecoderThread;
	this->videoStabilizer = videoStabilizer;
	this->routeManager = routeManager;
	this->renderer = renderer;
	this->videoEncoder = videoEncoder;

	frameReadSemaphore = new QSemaphore();
	frameAvailableSemaphore = new QSemaphore();
}

RenderOffScreenThread::~RenderOffScreenThread()
{
	if (frameAvailableSemaphore != nullptr)
	{
		delete frameAvailableSemaphore;
		frameAvailableSemaphore = nullptr;
	}

	if (frameReadSemaphore != nullptr)
	{
		delete frameReadSemaphore;
		frameReadSemaphore = nullptr;
	}
}

void RenderOffScreenThread::run()
{
	FrameData decodedFrameData;
	FrameData decodedFrameDataGrayscale;

	double frameDuration = videoDecoder->getFrameDuration();

	frameReadSemaphore->release(1);

	while (!isInterruptionRequested())
	{
		if (videoDecoderThread->tryGetNextFrame(decodedFrameData, decodedFrameDataGrayscale, 100))
		{
			videoStabilizer->processFrame(decodedFrameDataGrayscale);
			encodeWindow->getContext()->makeCurrent(encodeWindow->getSurface());
			renderer->startRendering(videoDecoder->getCurrentTime(), frameDuration, videoDecoder->getDecodeDuration(), videoStabilizer->getProcessDuration(), videoEncoder->getEncodeDuration(), 0.0);
			renderer->uploadFrameData(decodedFrameData);
			videoDecoderThread->signalFrameRead();
			renderer->renderAll();
			renderer->stopRendering();
			routeManager->update(videoDecoder->getCurrentTime(), frameDuration);

			while (!frameReadSemaphore->tryAcquire(1, 100) && !isInterruptionRequested()) {}

			if (isInterruptionRequested())
				break;

			renderedFrameData = renderer->getRenderedFrame();
			renderedFrameData.duration = decodedFrameData.duration;
			renderedFrameData.cumulativeNumber = decodedFrameData.cumulativeNumber;

			frameAvailableSemaphore->release(1);
		}
	}

	encodeWindow->getContext()->doneCurrent();
	encodeWindow->getContext()->moveToThread(mainWindow->thread());
}

bool RenderOffScreenThread::tryGetNextFrame(FrameData& frameData, int timeout)
{
	if (frameAvailableSemaphore->tryAcquire(1, timeout))
	{
		frameData = renderedFrameData;
		return true;
	}
	else
		return false;
}

void RenderOffScreenThread::signalFrameRead()
{
	frameReadSemaphore->release(1);
}
