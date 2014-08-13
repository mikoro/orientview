// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>

namespace OrientView
{
	class VideoDecoder;
	class VideoEncoder;
	class RenderOffScreenThread;

	// Run video encoder on a thread.
	class VideoEncoderThread : public QThread
	{
		Q_OBJECT

	public:

		void initialize(VideoDecoder* videoDecoder, VideoEncoder* videoEncoder, RenderOffScreenThread* renderOffScreenThread);

	signals:

		void frameProcessed(int frameNumber, int frameSize);
		void encodingFinished();

	protected:

		void run();

	private:

		VideoDecoder* videoDecoder = nullptr;
		VideoEncoder* videoEncoder = nullptr;
		RenderOffScreenThread* renderOffScreenThread = nullptr;
	};
}
