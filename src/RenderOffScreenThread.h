// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>
#include <QSemaphore>
#include <QOpenGLFramebufferObject>

#include "FrameData.h"

namespace OrientView
{
	class MainWindow;
	class EncodeWindow;
	class VideoDecoderThread;
	class VideoRenderer;
	class Settings;

	class RenderOffScreenThread : public QThread
	{
		Q_OBJECT

	public:

		RenderOffScreenThread();

		bool initialize(MainWindow* mainWindow, EncodeWindow* encodeWindow, VideoDecoderThread* videoDecoderThread, VideoRenderer* videoRenderer, Settings* settings);
		void shutdown();

		bool getNextFrame(FrameData* frameData);
		void signalFrameRead();

	protected:

		void run();

		void readDataFromFramebuffer(QOpenGLFramebufferObject* sourceFbo);

	private:

		MainWindow* mainWindow = nullptr;
		EncodeWindow* encodeWindow = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
		VideoRenderer* videoRenderer = nullptr;

		QOpenGLFramebufferObject* mainFramebuffer = nullptr;
		QOpenGLFramebufferObject* secondaryFramebuffer = nullptr;

		QSemaphore frameReadSemaphore;
		QSemaphore frameAvailableSemaphore;

		int framebufferWidth = 0;
		int framebufferHeight = 0;

		FrameData renderedFrameData;
	};
}
