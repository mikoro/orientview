// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
}

namespace OrientView
{
	struct FrameData;

	struct VideoInfo
	{
		int frameWidth = 0;
		int frameHeight = 0;
		int frameDataLength = 0;
		int totalFrameCount = 0;
		int currentFrameNumber = 0;
		int averageFrameDuration = 0; // ms
		int averageFrameRateNum = 0;
		int averageFrameRateDen = 0;
		double averageFrameRate = 0.0;
	};

	class VideoDecoder
	{
	public:

		VideoDecoder();

		bool initialize(const QString& fileName);
		void shutdown();

		bool getNextFrame(FrameData* frameData);

		VideoInfo getVideoInfo() const;

	private:

		static bool isRegistered;
		bool isInitialized = false;
		AVFormatContext* formatContext = nullptr;
		AVCodecContext* videoCodecContext = nullptr;
		AVStream* videoStream = nullptr;
		int videoStreamIndex = -1;
		AVFrame* frame = nullptr;
		AVPacket packet;
		SwsContext* resizeContext = nullptr;
		AVPicture resizedPicture;
		int64_t lastFrameTimestamp = 0;
		VideoInfo videoInfo;
	};
}
