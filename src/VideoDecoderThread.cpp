// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "VideoDecoderThread.h"
#include "VideoDecoder.h"

using namespace OrientView;

void VideoDecoderThread::initialize(VideoDecoder* videoDecoder)
{
	this->videoDecoder = videoDecoder;

	frameReadSemaphore = new QSemaphore();
	frameAvailableSemaphore = new QSemaphore();
	decodedFrameData = FrameData();
	decodedFrameDataGrayscale = FrameData();
}

VideoDecoderThread::~VideoDecoderThread()
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

void VideoDecoderThread::run()
{
	frameReadSemaphore->release(1);

	while (!isInterruptionRequested())
	{
		while (!frameReadSemaphore->tryAcquire(1, 100) && !isInterruptionRequested()) {}

		if (isInterruptionRequested())
			break;

		if (videoDecoder->getNextFrame(&decodedFrameData, &decodedFrameDataGrayscale))
			frameAvailableSemaphore->release(1);
		else
			QThread::msleep(100);
	}
}

bool VideoDecoderThread::tryGetNextFrame(FrameData* frameData, FrameData* frameDataGrayscale, int timeout)
{
	if (frameAvailableSemaphore->tryAcquire(1, timeout))
	{
		*frameData = decodedFrameData;
		*frameDataGrayscale = decodedFrameDataGrayscale;

		return true;
	}
	else
		return false;
}

void VideoDecoderThread::signalFrameRead()
{
	frameReadSemaphore->release(1);
}
