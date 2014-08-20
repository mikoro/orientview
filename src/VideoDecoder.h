// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QMutex>
#include <QElapsedTimer>

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

	// Encapsulate the FFmpeg library for reading and decoding video files.
	class VideoDecoder
	{

	public:

		bool initialize(Settings* settings);
		~VideoDecoder();

		bool getNextFrame(FrameData* frameData, FrameData* frameDataGrayscale);
		void seekRelative(double seconds);

		bool getIsFinished();
		double getCurrentTime();
		double getLastDecodeTime();
		int getFrameWidth() const;
		int getFrameHeight() const;
		int getTotalFrameCount() const;
		int getFrameRateNum() const;
		int getFrameRateDen() const;
		double getFrameDuration() const;
		
	private:

		QMutex decoderMutex;

		AVFormatContext* formatContext = nullptr;
		AVCodecContext* videoCodecContext = nullptr;
		AVStream* videoStream = nullptr;
		AVFrame* frame = nullptr;
		AVPacket packet;
		int videoStreamIndex = 0;

		SwsContext* swsContext = nullptr;
		SwsContext* swsContextGrayscale = nullptr;
		AVPicture* convertedPicture = nullptr;
		AVPicture* convertedPictureGrayscale = nullptr;

		int frameWidth = 0;
		int frameHeight = 0;
		int grayscaleFrameWidth = 0;
		int grayscaleFrameHeight = 0;

		int frameCountDivisor = 0;
		int frameDurationDivisor = 0;

		int64_t totalFrameCount = 0;
		int64_t cumulativeFrameNumber = 0;

		int64_t frameRateNum = 0; // no unit
		int64_t frameRateDen = 0; // no unit
		int64_t frameDuration = 0.0; // microseconds
		int64_t previousFrameTimestamp = 0; // video stream time base units

		double currentTimeInSeconds = 0.0;

		bool isInitialized = false;
		bool isFinished = false;
		bool seekToAnyFrame = false;

		QElapsedTimer decodeTimer;
		double lastDecodeTime = 0.0;
	};
}
