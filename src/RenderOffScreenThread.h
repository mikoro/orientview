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
	class VideoStabilizer;
	class Renderer;
	class Settings;

	class RenderOffScreenThread : public QThread
	{
		Q_OBJECT

	public:

		RenderOffScreenThread();

		bool initialize(MainWindow* mainWindow, EncodeWindow* encodeWindow, VideoDecoderThread* videoDecoderThread, VideoStabilizer* videoStabilizer, Renderer* renderer, Settings* settings);
		void shutdown();

		bool tryGetNextFrame(FrameData* frameData);
		void signalFrameRead();

	protected:

		void run();

	private:

		void readDataFromFramebuffer(QOpenGLFramebufferObject* sourceFbo);

		MainWindow* mainWindow = nullptr;
		EncodeWindow* encodeWindow = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		Renderer* renderer = nullptr;

		QOpenGLFramebufferObject* mainFramebuffer = nullptr;
		QOpenGLFramebufferObject* secondaryFramebuffer = nullptr;

		QSemaphore* frameReadSemaphore = nullptr;
		QSemaphore* frameAvailableSemaphore = nullptr;

		int framebufferWidth = 0;
		int framebufferHeight = 0;

		FrameData renderedFrameData;
	};
}
