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
	class RouteManager;
	class Renderer;
	class InputHandler;
	
	// Run renderer on a thread and draw to a visible window.
	class RenderOnScreenThread : public QThread
	{
		Q_OBJECT

	public:

		void initialize(MainWindow* mainWindow, VideoWindow* videoWindow, VideoDecoder* videoDecoder, VideoDecoderThread* videoDecoderThread, VideoStabilizer* videoStabilizer, RouteManager* routeManager, Renderer* renderer, InputHandler* inputHandler);

		bool getIsPaused();
		void togglePaused();
		void advanceOneFrame();

	public slots:

		void windowResized(int newWidth, int newHeight);

	protected:

		void run();

	private:

		MainWindow* mainWindow = nullptr;
		VideoWindow* videoWindow = nullptr;
		VideoDecoder* videoDecoder = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;
		RouteManager* routeManager = nullptr;
		Renderer* renderer = nullptr;
		InputHandler* inputHandler = nullptr;

		bool isPaused = false;
		bool shouldAdvanceOneFrame = false;
		bool shouldResizeWindow = false;

		int newWindowWidth = 0;
		int newWindowHeight = 0;
	};
}
