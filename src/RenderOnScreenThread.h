// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>

namespace OrientView
{
	class VideoWindow;
	class VideoRenderer;
	class VideoDecoderThread;

	class RenderOnScreenThread : public QThread
	{
		Q_OBJECT

	public:

		RenderOnScreenThread();

		bool initialize(VideoWindow* videoWindow, VideoRenderer* videoRenderer, VideoDecoderThread* videoDecoderThread);
		void shutdown();

	protected:

		void run();

	private:

		VideoWindow* videoWindow = nullptr;
		VideoRenderer* videoRenderer = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
	};
}
