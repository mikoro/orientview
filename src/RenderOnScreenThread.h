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
	class InputHandler;
	
	class RenderOnScreenThread : public QThread
	{
		Q_OBJECT

	public:

		bool initialize(MainWindow* mainWindow, VideoWindow* videoWindow, VideoDecoder* videoDecoder, VideoDecoderThread* videoDecoderThread, VideoStabilizer* videoStabilizer, Renderer* renderer, InputHandler* inputHandler);
		void shutdown();

		bool isPaused() const;
		void togglePaused();
		void advanceOneFrame();

	protected:

		void run();

	private:

		MainWindow* mainWindow = nullptr;
		VideoWindow* videoWindow = nullptr;
		VideoDecoder* videoDecoder = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		Renderer* renderer = nullptr;
		InputHandler* inputHandler = nullptr;

		bool paused = false;
		bool shouldAdvanceOneFrame = false;
	};
}
