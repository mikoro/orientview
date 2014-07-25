// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

namespace OrientView
{
	struct DecodedFrame;

	class VideoDecoder
	{
	public:

		VideoDecoder();

		bool initialize(const QString& fileName);
		void shutdown();

		bool getNextFrame(DecodedFrame* decodedFrame);

		int getFrameWidth() const;
		int getFrameHeight() const;
		int getFrameDataLength() const;
		int getTotalFrameCount() const;
		int getProcessedFrameCount() const;
		int getFrameDuration() const;
		double getFrameRate() const;

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
		int frameWidth = 0;
		int frameHeight = 0;
		int frameDataLength = 0;
		int totalFrameCount = 0;
		int processedFrameCount = 0;
		int frameDuration = 0;
		double frameRate = 0.0;
	};
}
