// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QThread>
#include <QFile>

namespace OrientView
{
	class VideoDecoder;
	class VideoStabilizer;
	class Settings;

	class VideoStabilizerThread : public QThread
	{
		Q_OBJECT

	public:

		bool initialize(VideoDecoder* videoDecoder, VideoStabilizer* videoStabilizer, Settings* settings);

	signals:

		void frameProcessed(int frameNumber);
		void processingFinished();

	protected:

		void run();

	private:

		VideoDecoder* videoDecoder = nullptr;
		VideoStabilizer* videoStabilizer = nullptr;

		QFile outputFile;
	};
}
