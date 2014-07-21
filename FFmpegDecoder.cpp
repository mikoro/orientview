// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QtGlobal>
#include <windows.h>

extern "C"
{
#define __STDC_CONSTANT_MACROS
#include <libavutil/imgutils.h>
}

#include "FFmpegDecoder.h"

using namespace OrientView;

bool FFmpegDecoder::isRegistered = false;

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

	int openCodecContext(int* streamIndex, AVFormatContext* formatContext, enum AVMediaType mediaType)
	{
		int result = 0;
		AVStream* stream = nullptr;
		AVCodecContext* codecContext = nullptr;
		AVCodec* codec = nullptr;
		AVDictionary* opts = nullptr;

		if ((result = av_find_best_stream(formatContext, mediaType, -1, -1, nullptr, 0)) < 0)
		{
			qWarning("Could not find %s stream in input file", av_get_media_type_string(mediaType));
			return result;
		}
		else
		{
			*streamIndex = result;
			stream = formatContext->streams[*streamIndex];
			codecContext = stream->codec;
			codec = avcodec_find_decoder(codecContext->codec_id);

			if (!codec)
			{
				qWarning("Failed to find %s codec", av_get_media_type_string(mediaType));
				return AVERROR(EINVAL);
			}

			if ((result = avcodec_open2(codecContext, codec, &opts)) < 0)
			{
				qWarning("Failed to open %s codec", av_get_media_type_string(mediaType));
				return result;
			}
		}

		return 0;
	}
}

FFmpegDecoder::FFmpegDecoder()
{
}

FFmpegDecoder::~FFmpegDecoder()
{
	if (isOpen)
		Close();
}

bool FFmpegDecoder::Open(const std::string& fileName)
{
	if (!isRegistered)
	{
		qDebug("Initializing FFmpeg");

		av_log_set_callback(ffmpegLogCallback);
		av_register_all();

		isRegistered = true;
	}

	if (isOpen)
		Close();

	qDebug("Opening %s", fileName.c_str());

	int result = 0;

	try
	{
		if ((result = avformat_open_input(&formatContext, fileName.c_str(), nullptr, nullptr)) < 0)
			throw std::runtime_error("Could not open source file");

		if ((result = avformat_find_stream_info(formatContext, nullptr)) < 0)
			throw std::runtime_error("Could not find stream information");

		if ((result = openCodecContext(&videoStreamIndex, formatContext, AVMEDIA_TYPE_VIDEO)) < 0)
			throw std::runtime_error("Could not open video codec context");
		else
		{
			videoStream = formatContext->streams[videoStreamIndex];
			videoCodecContext = videoStream->codec;

			if ((result = av_image_alloc(videoData, videoLineSize, videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt, 1)) < 0)
				throw std::runtime_error("Could not allocate raw video buffer");

			videoBufferSize = result;
		}

		if ((result = openCodecContext(&audioStreamIndex, formatContext, AVMEDIA_TYPE_AUDIO)) < 0)
		{
			qWarning("Could not open audio codec context");
			hasAudio = false;
		}
		else
		{
			audioStream = formatContext->streams[audioStreamIndex];
			audioCodecContext = audioStream->codec;
			hasAudio = true;
		}

		if (!(frame = av_frame_alloc()))
		{
			result = AVERROR(ENOMEM);
			throw std::runtime_error("Could not allocate frame");
		}

		av_init_packet(&packet);
		packet.data = nullptr;
		packet.size = 0;

		resizeContext = sws_getContext(videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt, videoCodecContext->width, videoCodecContext->height, PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);

		if (!resizeContext)
			throw std::runtime_error("Could not get resize context");

		if (avpicture_alloc(resizedPicture, PIX_FMT_RGB24, videoCodecContext->width, videoCodecContext->height) < 0)
			throw std::runtime_error("Could not allocate picture");
	}
	catch (const std::exception& ex)
	{
		char message[64] = { 0 };
		av_strerror(result, message, 64);
		qCritical("Could not open FFmpeg decoder: %s: %s", ex.what(), message);
		Close();

		return false;
	}

	qDebug("File opened successfully");

	isOpen = true;
	return true;
}

void FFmpegDecoder::Close()
{
	qDebug("Closing");

	avcodec_close(videoCodecContext);
	avcodec_close(audioCodecContext);
	avformat_close_input(&formatContext);
	av_free(videoData[0]);
	av_frame_free(&frame);
	sws_freeContext(resizeContext);
	avpicture_free(resizedPicture);

	formatContext = nullptr;
	videoCodecContext = nullptr;
	audioCodecContext = nullptr;
	videoStream = nullptr;
	audioStream = nullptr;
	videoStreamIndex = -1;
	audioStreamIndex = -1;
	videoData[0] = nullptr;
	videoData[1] = nullptr;
	videoData[2] = nullptr;
	videoData[3] = nullptr;
	videoLineSize[0] = 0;
	videoLineSize[1] = 0;
	videoLineSize[2] = 0;
	videoLineSize[3] = 0;
	videoBufferSize = 0;
	frame = nullptr;
	resizeContext = nullptr;
	resizedPicture = nullptr;

	isOpen = false;
}

AVPicture* FFmpegDecoder::GetNextFrame()
{
	while (true)
	{
		if (av_read_frame(formatContext, &packet) >= 0)
		{
			if (packet.stream_index == videoStreamIndex)
			{
				int gotPicture = 0;

				if (avcodec_decode_video2(videoCodecContext, frame, &gotPicture, &packet) < 0)
					qWarning("Error decoding video frame");

				av_free_packet(&packet);

				if (gotPicture)
				{
					sws_scale(resizeContext, frame->data, frame->linesize, 0, frame->height, resizedPicture->data, resizedPicture->linesize);
					return resizedPicture;
				}
			}
			else
				av_free_packet(&packet);
		}
		else
		{
			qWarning("Could not read frame");
			break;
		}
	}
	
	return nullptr;
}
