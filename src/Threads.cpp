// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "Threads.h"
#include "VideoWindow.h"
#include "VideoRenderer.h"

using namespace OrientView;

DecodeThread::DecodeThread()
{
}

void DecodeThread::run()
{
}

RenderOnScreenThread::RenderOnScreenThread()
{
}

void RenderOnScreenThread::initialize(VideoWindow* videoWindow, VideoRenderer* videoRenderer)
{
	this->videoWindow = videoWindow;
	this->videoRenderer = videoRenderer;
}

void RenderOnScreenThread::run()
{
	while (!isInterruptionRequested())
	{
		if (videoWindow->isExposed())
		{
			videoWindow->getContext()->makeCurrent(videoWindow);
			glViewport(0, 0, videoWindow->width(), videoWindow->height());
			videoRenderer->render();
			videoWindow->getContext()->swapBuffers(videoWindow);
		}

		QThread::msleep(15);
	}
}

RenderOffScreenThread::RenderOffScreenThread()
{
}

void RenderOffScreenThread::run()
{
}

EncodeThread::EncodeThread()
{
}

void EncodeThread::run()
{
}
