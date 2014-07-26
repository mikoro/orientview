// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>

namespace OrientView
{
	class EncodeWindow;
	class VideoRenderer;
	class VideoDecoderThread;

	class RenderOffScreenThread : public QThread
	{
		Q_OBJECT

	public:

		RenderOffScreenThread();

		bool initialize(EncodeWindow* encodeWindow, VideoRenderer* videoRenderer, VideoDecoderThread* videoDecoderThread);
		void shutdown();

	protected:

		void run();

	private:

		EncodeWindow* encodeWindow = nullptr;
		VideoRenderer* videoRenderer = nullptr;
		VideoDecoderThread* videoDecoderThread = nullptr;
	};
}
