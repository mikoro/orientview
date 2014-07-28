// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>

namespace OrientView
{
	class MainWindow;
	class VideoWindow;
	class VideoDecoderThread;
	class VideoStabilizer;
	class VideoRenderer;
	
	class RenderOnScreenThread : public QThread
	{
		Q_OBJECT

	public:

		RenderOnScreenThread();

		bool initialize(MainWindow* mainWindow, VideoWindow* videoWindow, VideoDecoderThread* videoDecoderThread, VideoStabilizer* videoStabilizer, VideoRenderer* videoRenderer);
		void shutdown();

	protected:

		void run();

	private:

		MainWindow* mainWindow = nullptr;
		VideoWindow* videoWindow = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		VideoRenderer* videoRenderer = nullptr;
	};
}
