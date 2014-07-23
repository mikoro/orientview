// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QOpenGLPixelTransferOptions>

#include "Threads.h"
#include "VideoWindow.h"
#include "VideoRenderer.h"
#include "FFmpegDecoder.h"

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

void RenderOnScreenThread::initialize(VideoWindow* videoWindow, VideoRenderer* videoRenderer, FFmpegDecoder* ffmpegDecoder)
{
	this->videoWindow = videoWindow;
	this->videoRenderer = videoRenderer;
	this->ffmpegDecoder = ffmpegDecoder;
}

void RenderOnScreenThread::run()
{
	DecodedPicture decodedPicture;
	QOpenGLPixelTransferOptions options;

	while (!isInterruptionRequested())
	{
		if (ffmpegDecoder->getNextPicture(&decodedPicture))
		{
			if (videoWindow->isExposed())
			{
				options.setRowLength(decodedPicture.width);
				options.setImageHeight(decodedPicture.height);

				videoRenderer->getVideoPanelTexture()->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, decodedPicture.data, &options);

				videoWindow->getContext()->makeCurrent(videoWindow);
				glViewport(0, 0, videoWindow->width(), videoWindow->height());
				videoRenderer->render();
				videoWindow->getContext()->swapBuffers(videoWindow);
			}
		}

		//QThread::msleep((int)round(ffmpegDecoder->getFrameTime()));
	}

	videoWindow->getContext()->makeCurrent(videoWindow);
	videoRenderer->shutdown();
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
