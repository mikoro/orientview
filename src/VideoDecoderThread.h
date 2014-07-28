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

		VideoDecoderThread();

		bool initialize(VideoDecoder* videoDecoder);
		void shutdown();

		bool getNextFrame(FrameData* frameData, FrameData* frameDataGrayscale);
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
