// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>

#include "VideoWindow.h"
#include "VideoRenderer.h"
#include "VideoDecoder.h"

namespace OrientView
{
	class RenderOnScreenThread : public QThread
	{
		Q_OBJECT

	public:

		RenderOnScreenThread();

		void initialize(VideoWindow* videoWindow, VideoRenderer* videoRenderer, VideoDecoder* videoDecoder);

	protected:

		void run();

	private:

		VideoWindow* videoWindow = nullptr;
		VideoRenderer* videoRenderer = nullptr;
		VideoDecoder* videoDecoder = nullptr;
	};
}
