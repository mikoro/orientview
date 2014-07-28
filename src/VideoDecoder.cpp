// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <windows.h>

#include <QtGlobal>

extern "C"
{
#define __STDC_CONSTANT_MACROS
#include <libavutil/imgutils.h>
}

#include "VideoDecoder.h"
#include "Settings.h"
#include "FrameData.h"

using namespace OrientView;

bool VideoDecoder::isRegistered = false;

namespace
{
	void ffmpegLogCallback(void* ptr, int level, const char* fmt, va_list vl)
	{
		if (level <= AV_LOG_WARNING)
		{
			int print_prefix = 0;
			char line[1024] = { 0 };
			av_log_format_line(ptr, level, fmt, vl, line, 1024, &print_prefix);
			qWarning(line);
		}
	}

	bool openCodecContext(int* streamIndex, AVFormatContext* formatContext, AVMediaType mediaType)
	{
		*streamIndex = av_find_best_stream(formatContext, mediaType, -1, -1, nullptr, 0);

		if (*streamIndex < 0)
		{
			qWarning("Could not find %s stream in input file", av_get_media_type_string(mediaType));
			return false;
		}
		else
		{
			AVCodecContext* codecContext = formatContext->streams[(size_t)*streamIndex]->codec;
			AVCodec* codec = avcodec_find_decoder(codecContext->codec_id);

			if (!codec)
			{
				qWarning("Could not find %s codec", av_get_media_type_string(mediaType));
				return false;
			}

			AVDictionary* opts = nullptr;

			if (avcodec_open2(codecContext, codec, &opts) < 0)
			{
				qWarning("Could not open %s codec", av_get_media_type_string(mediaType));
				return false;
			}
		}

		return true;
	}
}

VideoDecoder::VideoDecoder()
{
}

bool VideoDecoder::initialize(const QString& fileName, Settings* settings)
{
	qDebug("Initializing VideoDecoder (%s)", fileName.toLocal8Bit().constData());

	if (!isRegistered)
	{
		av_log_set_callback(ffmpegLogCallback);
		av_register_all();

		isRegistered = true;
	}

	try
	{
		if (avformat_open_input(&formatContext, fileName.toLocal8Bit().constData(), nullptr, nullptr) < 0)
			throw std::runtime_error("Could not open source file");

		if (avformat_find_stream_info(formatContext, nullptr) < 0)
			throw std::runtime_error("Could not find stream information");

		if (!openCodecContext(&videoStreamIndex, formatContext, AVMEDIA_TYPE_VIDEO))
			throw std::runtime_error("Could not open video codec context");

		videoStream = formatContext->streams[(size_t)videoStreamIndex];
		videoCodecContext = videoStream->codec;
		videoInfo.frameWidth = videoCodecContext->width;
		videoInfo.frameHeight = videoCodecContext->height;

		frame = av_frame_alloc();

		if (!frame)
			throw std::runtime_error("Could not allocate frame");

		av_init_packet(&packet);
		packet.data = nullptr;
		packet.size = 0;

		swsContext = sws_getContext(videoInfo.frameWidth, videoInfo.frameHeight, videoCodecContext->pix_fmt, videoInfo.frameWidth, videoInfo.frameHeight, PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);

		if (!swsContext)
			throw std::runtime_error("Could not get sws context");

		convertedPicture = new AVPicture();

		if (avpicture_alloc(convertedPicture, PIX_FMT_RGBA, videoInfo.frameWidth, videoInfo.frameHeight) < 0)
			throw std::runtime_error("Could not allocate conversion picture");

		stabilizationEnabled = settings->stabilization.enabled;
		imageSizeDivisor = settings->stabilization.imageSizeDivisor;
		
		swsContextGrayscale = sws_getContext(videoInfo.frameWidth, videoInfo.frameHeight, videoCodecContext->pix_fmt, videoInfo.frameWidth / imageSizeDivisor, videoInfo.frameHeight / imageSizeDivisor, PIX_FMT_GRAY8, SWS_BILINEAR, nullptr, nullptr, nullptr);

		if (!swsContextGrayscale)
			throw std::runtime_error("Could not get sws grayscale context");

		convertedPictureGrayscale = new AVPicture();

		if (avpicture_alloc(convertedPictureGrayscale, PIX_FMT_GRAY8, videoInfo.frameWidth / imageSizeDivisor, videoInfo.frameHeight / imageSizeDivisor) < 0)
			throw std::runtime_error("Could not allocate grayscale conversion picture");

		frameCountDivisor = settings->decoder.frameCountDivisor;
		frameDurationDivisor = settings->decoder.frameDurationDivisor;

		videoInfo.frameDataLength = videoInfo.frameHeight * convertedPicture->linesize[0];
		videoInfo.totalFrameCount = videoStream->nb_frames / frameCountDivisor;
		videoInfo.averageFrameDuration = (int)av_rescale(1000000 * frameCountDivisor / frameDurationDivisor, videoStream->avg_frame_rate.den, videoStream->avg_frame_rate.num);
		videoInfo.averageFrameRateNum = videoStream->avg_frame_rate.num / frameCountDivisor * frameDurationDivisor;
		videoInfo.averageFrameRateDen = videoStream->avg_frame_rate.den;
		videoInfo.averageFrameRate = (double)videoInfo.averageFrameRateNum / videoInfo.averageFrameRateDen;
	}
	catch (const std::exception& ex)
	{
		qWarning("Could not initialize VideoDecoder: %s", ex.what());
		return false;
	}

	isInitialized = true;
	return true;
}

void VideoDecoder::shutdown()
{
	qDebug("Shutting down VideoDecoder");

	if (videoCodecContext != nullptr)
	{
		avcodec_close(videoCodecContext);
		videoCodecContext = nullptr;
	}

	if (formatContext != nullptr)
	{
		avformat_close_input(&formatContext);
		formatContext = nullptr;
	}

	if (frame != nullptr)
	{
		av_frame_free(&frame);
		frame = nullptr;
	}

	if (swsContextGrayscale != nullptr)
	{
		sws_freeContext(swsContextGrayscale);
		swsContextGrayscale = nullptr;
	}

	if (convertedPictureGrayscale != nullptr)
	{
		avpicture_free(convertedPictureGrayscale);
		convertedPictureGrayscale = nullptr;
	}

	if (swsContext != nullptr)
	{
		sws_freeContext(swsContext);
		swsContext = nullptr;
	}

	if (convertedPicture != nullptr)
	{
		avpicture_free(convertedPicture);
		convertedPicture = nullptr;
	}
	
	videoStream = nullptr;
	videoStreamIndex = -1;
	lastFrameTimestamp = 0;
	frameCountDivisor = 0;
	int frameDurationDivisor = 0;
	stabilizationEnabled = false;
	imageSizeDivisor = 0;
	videoInfo = VideoInfo();

	isInitialized = false;
}

bool VideoDecoder::getNextFrame(FrameData* frameData, FrameData* frameDataGrayscale)
{
	if (!isInitialized)
		return false;

	int framesRead = 0;

	while (true)
	{
		if (av_read_frame(formatContext, &packet) >= 0)
		{
			if (packet.stream_index == videoStreamIndex)
			{
				int gotPicture = 0;
				int decodedBytes = avcodec_decode_video2(videoCodecContext, frame, &gotPicture, &packet);

				if (++framesRead < frameCountDivisor)
				{
					av_free_packet(&packet);
					continue;
				}
				
				framesRead = 0;
				videoInfo.currentFrameNumber++;

				if (decodedBytes < 0)
				{
					qWarning("Could not decode video frame");
					av_free_packet(&packet);
					return false;
				}

				if (gotPicture)
				{
					sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, convertedPicture->data, convertedPicture->linesize);

					frameData->data = convertedPicture->data[0];
					frameData->dataLength = videoInfo.frameHeight * convertedPicture->linesize[0];
					frameData->rowLength = convertedPicture->linesize[0];
					frameData->width = videoInfo.frameWidth;
					frameData->height = videoInfo.frameHeight;
					frameData->duration = (int)av_rescale((frame->best_effort_timestamp - lastFrameTimestamp) * 1000000 / frameDurationDivisor, videoStream->time_base.num, videoStream->time_base.den);
					frameData->number = videoInfo.currentFrameNumber;

					lastFrameTimestamp = frame->best_effort_timestamp;

					if (frameData->duration <= 0 || frameData->duration > 1000000)
						frameData->duration = videoInfo.averageFrameDuration;

					if (stabilizationEnabled && frameDataGrayscale != nullptr)
					{
						sws_scale(swsContextGrayscale, frame->data, frame->linesize, 0, frame->height, convertedPictureGrayscale->data, convertedPictureGrayscale->linesize);

						frameDataGrayscale->data = convertedPictureGrayscale->data[0];
						frameDataGrayscale->dataLength = videoInfo.frameHeight / imageSizeDivisor * convertedPictureGrayscale->linesize[0];
						frameDataGrayscale->rowLength = convertedPictureGrayscale->linesize[0];
						frameDataGrayscale->width = videoInfo.frameWidth / imageSizeDivisor;
						frameDataGrayscale->height = videoInfo.frameHeight / imageSizeDivisor;
						frameDataGrayscale->duration = frameData->duration;
						frameDataGrayscale->number = frameData->number;
					}

					av_free_packet(&packet);
					return true;
				}
			}

			av_free_packet(&packet);
		}
		else
			return false;
	}
}

VideoInfo VideoDecoder::getVideoInfo() const
{
	return videoInfo;
}
