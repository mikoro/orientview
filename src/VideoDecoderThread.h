// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>
#include <QSemaphore>

#include "DecodedFrame.h"

namespace OrientView
{
	class VideoDecoder;
	struct DecodedFrame;

	class VideoDecoderThread : public QThread
	{
		Q_OBJECT

	public:

		VideoDecoderThread();

		bool initialize(VideoDecoder* videoDecoder);
		void shutdown();

		bool getDecodedFrame(DecodedFrame* decodedFrame);
		void signalProcessingFinished();

	protected:

		void run();

	private:

		VideoDecoder* videoDecoder = nullptr;
		QSemaphore processingFinishedSemaphore;
		QSemaphore frameUpdatedSemaphore;
		DecodedFrame localDecodedFrame;
		int localDataLength = 0;
	};
}
