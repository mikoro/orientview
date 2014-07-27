// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "VideoDecoderThread.h"
#include "VideoDecoder.h"

using namespace OrientView;

VideoDecoderThread::VideoDecoderThread()
{
}

bool VideoDecoderThread::initialize(VideoDecoder* videoDecoder)
{
	qDebug("Initializing VideoDecoderThread");

	this->videoDecoder = videoDecoder;

	return true;
}

void VideoDecoderThread::shutdown()
{
	qDebug("Shutting down VideoDecoderThread");
}

void VideoDecoderThread::run()
{
	frameReadSemaphore.release(1);

	while (!isInterruptionRequested())
	{
		while (!frameReadSemaphore.tryAcquire(1, 20) && !isInterruptionRequested()) {}

		if (isInterruptionRequested())
			break;

		if (videoDecoder->getNextFrame(&frameData))
			frameAvailableSemaphore.release(1);
		else
			QThread::msleep(20);
	}
}

bool VideoDecoderThread::getNextFrame(FrameData* frameDataPtr)
{
	if (frameAvailableSemaphore.tryAcquire(1, 20))
	{
		frameDataPtr->data = frameData.data;
		frameDataPtr->dataLength = frameData.dataLength;
		frameDataPtr->rowLength = frameData.rowLength;
		frameDataPtr->width = frameData.width;
		frameDataPtr->height = frameData.height;
		frameDataPtr->duration = frameData.duration;
		frameDataPtr->number = frameData.number;

		return true;
	}
	else
		return false;
}

void VideoDecoderThread::signalFrameRead()
{
	frameReadSemaphore.release(1);
}
