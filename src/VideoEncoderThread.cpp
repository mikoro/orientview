// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "VideoEncoderThread.h"

using namespace OrientView;

VideoEncoderThread::VideoEncoderThread()
{
}

bool VideoEncoderThread::initialize()
{
	qDebug("Initializing VideoEncoderThread");

	return true;
}

void VideoEncoderThread::shutdown()
{
	qDebug("Shutting down VideoEncoderThread");
}

void VideoEncoderThread::run()
{
	int currentFrame = 0;
	int totalFrames = 100;

	while (!isInterruptionRequested())
	{
		currentFrame++;
		emit progressUpdate(currentFrame, totalFrames);

		if (currentFrame >= totalFrames)
			break;

		QThread::msleep(100);
	}

	emit encodingFinished();
}
