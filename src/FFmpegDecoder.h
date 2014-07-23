// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <string>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

namespace OrientView
{
	struct DecodedPicture
	{
		uint8_t* data = nullptr;
		int dataLength = 0;
		int stride = 0;
		int width = 0;
		int height = 0;
	};

	class FFmpegDecoder
	{
	public:

		FFmpegDecoder();
		~FFmpegDecoder();

		bool initialize(const std::string& fileName);
		void shutdown();
		bool getNextPicture(DecodedPicture* decodedPicture);
		double getFrameTime() const;

	private:

		static bool isRegistered;
		bool isInitialized = false;
		double frameTime = 0.0;

		AVFormatContext* formatContext = nullptr;
		AVCodecContext* videoCodecContext = nullptr;
		AVStream* videoStream = nullptr;
		int videoStreamIndex = -1;
		uint8_t* videoData[4] = { nullptr };
		int videoLineSize[4] = { 0 };
		int videoBufferSize = 0;
		AVFrame* frame = nullptr;
		AVPacket packet;
		SwsContext* resizeContext = nullptr;
		AVPicture resizedPicture;
	};
}
