// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QElapsedTimer>
#include <QOpenGLFramebufferObjectFormat>

#include "RenderOffScreenThread.h"
#include "MainWindow.h"
#include "EncodeWindow.h"
#include "VideoDecoder.h"
#include "VideoDecoderThread.h"
#include "VideoStabilizer.h"
#include "Renderer.h"
#include "VideoEncoder.h"
#include "Settings.h"
#include "FrameData.h"

using namespace OrientView;

bool RenderOffScreenThread::initialize(MainWindow* mainWindow, EncodeWindow* encodeWindow, VideoDecoder* videoDecoder, VideoDecoderThread* videoDecoderThread, VideoStabilizer* videoStabilizer, Renderer* renderer, VideoEncoder* videoEncoder, Settings* settings)
{
	qDebug("Initializing RenderOffScreenThread");

	this->mainWindow = mainWindow;
	this->encodeWindow = encodeWindow;
	this->videoDecoder = videoDecoder;
	this->videoDecoderThread = videoDecoderThread;
	this->videoStabilizer = videoStabilizer;
	this->renderer = renderer;
	this->videoEncoder = videoEncoder;

	framebufferWidth = settings->window.width;
	framebufferHeight = settings->window.height;

	QOpenGLFramebufferObjectFormat mainFboFormat;
	mainFboFormat.setSamples(settings->window.multisamples);
	mainFboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

	mainFramebuffer = new QOpenGLFramebufferObject(framebufferWidth, framebufferHeight, mainFboFormat);

	if (!mainFramebuffer->isValid())
	{
		qWarning("Could not create main frame buffer");
		return false;
	}

	QOpenGLFramebufferObjectFormat secondaryFboFormat;
	secondaryFboFormat.setSamples(0);
	secondaryFboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

	secondaryFramebuffer = new QOpenGLFramebufferObject(framebufferWidth, framebufferHeight, secondaryFboFormat);

	if (!secondaryFramebuffer->isValid())
	{
		qWarning("Could not create secondary frame buffer");
		return false;
	}

	renderedFrameData = FrameData();
	renderedFrameData.dataLength = framebufferWidth * framebufferHeight * 4;
	renderedFrameData.rowLength = framebufferWidth * 4;
	renderedFrameData.data = new uint8_t[renderedFrameData.dataLength];
	renderedFrameData.width = framebufferWidth;
	renderedFrameData.height = framebufferHeight;

	frameReadSemaphore = new QSemaphore();
	frameAvailableSemaphore = new QSemaphore();

	return true;
}

void RenderOffScreenThread::shutdown()
{
	qDebug("Shutting down RenderOffScreenThread");

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

	if (renderedFrameData.data)
	{
		delete renderedFrameData.data;
		renderedFrameData.data = nullptr;
	}

	if (secondaryFramebuffer != nullptr)
	{
		delete secondaryFramebuffer;
		secondaryFramebuffer = nullptr;
	}

	if (mainFramebuffer != nullptr)
	{
		delete mainFramebuffer;
		mainFramebuffer = nullptr;
	}
}

void RenderOffScreenThread::run()
{
	FrameData decodedFrameData;
	FrameData decodedFrameDataGrayscale;
	
	QElapsedTimer frameDurationTimer;
	double frameDuration = 0.1;

	frameDurationTimer.start();
	frameReadSemaphore->release(1);

	while (!isInterruptionRequested())
	{
		if (videoDecoderThread->tryGetNextFrame(&decodedFrameData, &decodedFrameDataGrayscale, 100))
		{
			videoStabilizer->processFrame(&decodedFrameDataGrayscale);

			encodeWindow->getContext()->makeCurrent(encodeWindow->getSurface());
			mainFramebuffer->bind();
			renderer->startRendering(framebufferWidth, framebufferHeight, frameDuration, 0.0, videoDecoder->getLastDecodeTime(), videoStabilizer->getLastProcessTime(), videoEncoder->getLastEncodeTime());
			renderer->uploadFrameData(&decodedFrameData);
			videoDecoderThread->signalFrameRead();
			renderer->renderAll();
			renderer->stopRendering();

			while (!frameReadSemaphore->tryAcquire(1, 20) && !isInterruptionRequested()) {}

			if (isInterruptionRequested())
				break;

			readDataFromFramebuffer(mainFramebuffer);
			renderedFrameData.duration = decodedFrameData.duration;
			renderedFrameData.number = decodedFrameData.number;

			frameAvailableSemaphore->release(1);

			frameDuration = frameDurationTimer.nsecsElapsed() / 1000000.0;
			frameDurationTimer.restart();
		}
	}

	encodeWindow->getContext()->doneCurrent();
	encodeWindow->getContext()->moveToThread(mainWindow->thread());
}

bool RenderOffScreenThread::tryGetNextFrame(FrameData* frameData, int timeout)
{
	if (frameAvailableSemaphore->tryAcquire(1, timeout))
	{
		frameData->data = renderedFrameData.data;
		frameData->dataLength = renderedFrameData.dataLength;
		frameData->rowLength = renderedFrameData.rowLength;
		frameData->width = renderedFrameData.width;
		frameData->height = renderedFrameData.height;
		frameData->duration = renderedFrameData.duration;
		frameData->number = renderedFrameData.number;

		return true;
	}
	else
		return false;
}

void RenderOffScreenThread::signalFrameRead()
{
	frameReadSemaphore->release(1);
}

void RenderOffScreenThread::readDataFromFramebuffer(QOpenGLFramebufferObject* sourceFbo)
{
	if (sourceFbo->format().samples() != 0)
	{
		QRect rect(0, 0, framebufferWidth, framebufferHeight);
		QOpenGLFramebufferObject::blitFramebuffer(secondaryFramebuffer, rect, sourceFbo, rect);
		readDataFromFramebuffer(secondaryFramebuffer);

		return;
	}

	sourceFbo->bind();
	glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, renderedFrameData.data);
}
