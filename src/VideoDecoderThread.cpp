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
	localDataLength = videoDecoder->getFrameDataLength();
	localDecodedFrame.data = new uint8_t[localDataLength];

	return true;
}

void VideoDecoderThread::shutdown()
{
	qDebug("Shutting down VideoDecoderThread");

	if (localDecodedFrame.data != nullptr)
	{
		delete localDecodedFrame.data;
		localDecodedFrame.data = nullptr;
	}

	localDataLength = 0;
}

void VideoDecoderThread::run()
{
	DecodedFrame decodedFrame;
	processingFinishedSemaphore.release(1);

	while (!isInterruptionRequested())
	{
		if (videoDecoder->getNextFrame(&decodedFrame))
		{
			while (!processingFinishedSemaphore.tryAcquire(1, 20) && !isInterruptionRequested()) {}

			if (isInterruptionRequested())
				break;

			memcpy(localDecodedFrame.data, decodedFrame.data, localDataLength);
			localDecodedFrame.dataLength = decodedFrame.dataLength;
			localDecodedFrame.stride = decodedFrame.stride;
			localDecodedFrame.width = decodedFrame.width;
			localDecodedFrame.height = decodedFrame.height;
			localDecodedFrame.duration = decodedFrame.duration;

			frameUpdatedSemaphore.release(1);
		}
		else
			QThread::msleep(100);
	}
}

bool VideoDecoderThread::getDecodedFrame(DecodedFrame* decodedFrame)
{
	if (frameUpdatedSemaphore.tryAcquire(1, 20))
	{
		decodedFrame->data = localDecodedFrame.data;
		decodedFrame->dataLength = localDecodedFrame.dataLength;
		decodedFrame->stride = localDecodedFrame.stride;
		decodedFrame->width = localDecodedFrame.width;
		decodedFrame->height = localDecodedFrame.height;
		decodedFrame->duration = localDecodedFrame.duration;

		return true;
	}
	else
		return false;
}

void VideoDecoderThread::signalProcessingFinished()
{
	processingFinishedSemaphore.release(1);
}
