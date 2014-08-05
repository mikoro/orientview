// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>
#include <QSemaphore>

#include "FrameData.h"

namespace OrientView
{
	class VideoDecoder;

	class VideoDecoderThread : public QThread
	{
		Q_OBJECT

	public:

		bool initialize(VideoDecoder* videoDecoder);
		void shutdown();

		bool tryGetNextFrame(FrameData* frameData, FrameData* frameDataGrayscale, int timeout);
		void signalFrameRead();

	protected:

		void run();

	private:

		VideoDecoder* videoDecoder = nullptr;

		QSemaphore* frameReadSemaphore = nullptr;
		QSemaphore* frameAvailableSemaphore = nullptr;

		FrameData decodedFrameData;
		FrameData decodedFrameDataGrayscale;
	};
}
