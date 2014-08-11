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
	struct Settings;
	struct FrameData;

	// Encapsulate the FFmpeg library for reading and decoding video files.
	class VideoDecoder
	{

	public:

		bool initialize(Settings* settings);
		~VideoDecoder();

		bool getNextFrame(FrameData* frameData, FrameData* frameDataGrayscale);
		void seekRelative(int seconds);

		int getCurrentFrameNumber();
		double getCurrentTime();
		bool getIsFinished();
		double getLastDecodeTime();

		int getFrameWidth() const;
		int getFrameHeight() const;
		int getFrameDataLength() const;
		int getTotalFrameCount() const;
		int getAverageFrameRateNum() const;
		int getAverageFrameRateDen() const;
		double getAverageFrameDuration() const;
		double getAverageFrameRate() const;
		
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

		bool generateGrayscalePicture = false;
		int grayscalePictureWidth = 0;
		int grayscalePictureHeight = 0;

		int frameWidth = 0;
		int frameHeight = 0;
		int frameDataLength = 0;
		int frameCountDivisor = 0;
		int frameDurationDivisor = 0;
		int totalFrameCount = 0;
		int currentFrameNumber = 0;
		int averageFrameRateNum = 0;
		int averageFrameRateDen = 0;
		double averageFrameDuration = 0.0; // in ms
		double averageFrameRate = 0.0;
		double currentTime = 0.0; // in s
		double totalDurationInSeconds = 0.0;
		int64_t totalDuration = 0; // video stream time base
		int64_t frameDuration = 0; // video stream time base
		int64_t lastFrameTimestamp = 0; // video stream time base
		bool isInitialized = false;
		bool isFinished = false;

		QElapsedTimer decodeTimer;
		double lastDecodeTime = 0.0;
	};
}
