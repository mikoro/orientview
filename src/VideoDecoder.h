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
	class Settings;
	struct FrameData;

	struct VideoInfo
	{
		int frameWidth = 0;
		int frameHeight = 0;
		int frameDataLength = 0;
		int totalFrameCount = 0;
		int currentFrameNumber = 0;
		int averageFrameRateNum = 0;
		int averageFrameRateDen = 0;
		double averageFrameDuration = 0.0;
		double averageFrameRate = 0.0;
	};

	class VideoDecoder
	{
	public:

		VideoDecoder();

		bool initialize(const QString& fileName, Settings* settings);
		void shutdown();

		bool getNextFrame(FrameData* frameData, FrameData* frameDataGrayscale);
		VideoInfo getVideoInfo() const;
		double getAverageDecodeTime() const;

	private:

		AVFormatContext* formatContext = nullptr;
		AVCodecContext* videoCodecContext = nullptr;
		AVStream* videoStream = nullptr;
		int videoStreamIndex = 0;
		AVFrame* frame = nullptr;
		AVPacket packet;
		SwsContext* swsContext = nullptr;
		SwsContext* swsContextGrayscale = nullptr;
		AVPicture* convertedPicture = nullptr;
		AVPicture* convertedPictureGrayscale = nullptr;
		bool generateGrayscalePicture = false;
		int grayscalePictureSizeDivisor = 0;
		int64_t lastFrameTimestamp = 0;
		int frameCountDivisor = 0;
		int frameDurationDivisor = 0;
		double averageDecodeTime = 0.0;
		VideoInfo videoInfo;
	};
}
