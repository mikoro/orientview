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

		if (videoDecoder->getNextFrame(&decodedFrameData))
			frameAvailableSemaphore.release(1);
		else
			QThread::msleep(20);
	}
}

bool VideoDecoderThread::getNextFrame(FrameData* frameData)
{
	if (frameAvailableSemaphore.tryAcquire(1, 20))
	{
		frameData->data = decodedFrameData.data;
		frameData->dataLength = decodedFrameData.dataLength;
		frameData->rowLength = decodedFrameData.rowLength;
		frameData->width = decodedFrameData.width;
		frameData->height = decodedFrameData.height;
		frameData->duration = decodedFrameData.duration;
		frameData->number = decodedFrameData.number;

		return true;
	}
	else
		return false;
}

void VideoDecoderThread::signalFrameRead()
{
	frameReadSemaphore.release(1);
}
