// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLPixelTransferOptions>

#include "RenderOffScreenThread.h"
#include "MainWindow.h"
#include "EncodeWindow.h"
#include "VideoDecoderThread.h"
#include "VideoRenderer.h"
#include "Settings.h"
#include "FrameData.h"

using namespace OrientView;

RenderOffScreenThread::RenderOffScreenThread()
{
}

bool RenderOffScreenThread::initialize(MainWindow* mainWindow, EncodeWindow* encodeWindow, VideoDecoderThread* videoDecoderThread, VideoRenderer* videoRenderer, Settings* settings)
{
	qDebug("Initializing RenderOffScreenThread");

	this->mainWindow = mainWindow;
	this->encodeWindow = encodeWindow;
	this->videoDecoderThread = videoDecoderThread;
	this->videoRenderer = videoRenderer;

	framebufferWidth = settings->display.width;
	framebufferHeight = settings->display.height;

	QOpenGLFramebufferObjectFormat mainFboFormat;
	mainFboFormat.setSamples(settings->display.multisamples);
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

	renderedFrameData.dataLength = framebufferWidth * framebufferHeight * 4;
	renderedFrameData.rowLength = framebufferWidth * 4;
	renderedFrameData.data = new uint8_t[renderedFrameData.dataLength];
	renderedFrameData.width = framebufferWidth;
	renderedFrameData.height = framebufferHeight;

	return true;
}

void RenderOffScreenThread::shutdown()
{
	qDebug("Shutting down RenderOffScreenThread");

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
	QOpenGLPixelTransferOptions options;

	frameReadSemaphore.release(1);

	while (!isInterruptionRequested())
	{
		encodeWindow->getContext()->makeCurrent(encodeWindow->getSurface());
		mainFramebuffer->bind();

		glViewport(0, 0, framebufferWidth, framebufferHeight);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		if (videoDecoderThread->getNextFrame(&decodedFrameData))
		{
			options.setRowLength(decodedFrameData.rowLength / 4);
			options.setImageHeight(decodedFrameData.height);
			options.setAlignment(1);

			videoRenderer->getVideoPanelTexture()->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, decodedFrameData.data, &options);
			videoDecoderThread->signalFrameRead();

			videoRenderer->update(framebufferWidth, framebufferHeight);
			videoRenderer->render();

			while (!frameReadSemaphore.tryAcquire(1, 20) && !isInterruptionRequested()) {}

			if (isInterruptionRequested())
				break;

			readDataFromFramebuffer(mainFramebuffer);
			renderedFrameData.duration = decodedFrameData.duration;
			renderedFrameData.number = decodedFrameData.number;

			frameAvailableSemaphore.release(1);
		}
	}

	encodeWindow->getContext()->doneCurrent();
	encodeWindow->getContext()->moveToThread(mainWindow->thread());
}

bool RenderOffScreenThread::getNextFrame(FrameData* frameData)
{
	if (frameAvailableSemaphore.tryAcquire(1, 20))
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
	frameReadSemaphore.release(1);
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
