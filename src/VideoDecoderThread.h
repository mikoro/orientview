// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>
#include <QSemaphore>

#include "FrameData.h"

namespace OrientView
{
	class VideoDecoder;

	// Run video decoder on a thread.
	class VideoDecoderThread : public QThread
	{
		Q_OBJECT

	public:

		void initialize(VideoDecoder* videoDecoder);
		~VideoDecoderThread();

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
