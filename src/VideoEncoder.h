// Copyright � 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>
#include <QElapsedTimer>

extern "C"
{
#include <stdint.h>
#include "x264/x264.h"
#include "libswscale/swscale.h"
}

namespace OrientView
{
	class VideoDecoder;
	class Settings;
	struct FrameData;
	class MP44File;

	class VideoEncoder
	{

	public:

		VideoEncoder();

		bool initialize(const QString& fileName, VideoDecoder* videoDecoder, Settings* settings);
		void shutdown();

		void loadFrameData(FrameData* frameData);
		int encodeFrame();
		void close();

		double getLastEncodeTime() const;

	private:

		x264_t* encoder = nullptr;
		x264_picture_t* convertedPicture = nullptr;
		SwsContext* swsContext = nullptr;
		MP44File* mp4File = nullptr;
		int64_t frameNumber = 0;

		QElapsedTimer encodeTimer;
		double lastEncodeTime = 0.0;
	};
}
