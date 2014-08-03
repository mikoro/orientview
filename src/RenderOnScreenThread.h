// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>

namespace OrientView
{
	class MainWindow;
	class VideoWindow;
	class VideoDecoder;
	class VideoDecoderThread;
	class VideoStabilizer;
	class Renderer;
	
	class RenderOnScreenThread : public QThread
	{
		Q_OBJECT

	public:

		RenderOnScreenThread();

		bool initialize(MainWindow* mainWindow, VideoWindow* videoWindow, VideoDecoder* videoDecoder, VideoDecoderThread* videoDecoderThread, VideoStabilizer* videoStabilizer, Renderer* renderer);
		void shutdown();

	protected:

		void run();

	private:

		MainWindow* mainWindow = nullptr;
		VideoWindow* videoWindow = nullptr;
		VideoDecoder* videoDecoder = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		Renderer* renderer = nullptr;
	};
}
