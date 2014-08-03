// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <windows.h>

#include <QtGlobal>
#include <QElapsedTimer>

extern "C"
{
#define __STDC_CONSTANT_MACROS
#include <libavutil/imgutils.h>
}

#include "VideoDecoder.h"
#include "Settings.h"
#include "FrameData.h"

using namespace OrientView;

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
	qDebug("Initializing VideoDecoder (%s)", qPrintable(fileName));

	av_log_set_callback(ffmpegLogCallback);
	av_register_all();

	if (avformat_open_input(&formatContext, fileName.toUtf8().constData(), nullptr, nullptr) < 0)
	{
		qWarning("Could not open source file");
		return false;
	}

	if (avformat_find_stream_info(formatContext, nullptr) < 0)
	{
		qWarning("Could not find stream information");
		return false;
	}

	if (!openCodecContext(&videoStreamIndex, formatContext, AVMEDIA_TYPE_VIDEO))
	{
		qWarning("Could not open video codec context");
		return false;
	}

	videoStream = formatContext->streams[(size_t)videoStreamIndex];
	videoCodecContext = videoStream->codec;

	videoInfo = VideoInfo();
	videoInfo.frameWidth = videoCodecContext->width / settings->decoder.frameSizeDivisor;
	videoInfo.frameHeight = videoCodecContext->height / settings->decoder.frameSizeDivisor;

	frame = av_frame_alloc();

	if (!frame)
	{
		qWarning("Could not allocate frame");
		return false;
	}

	av_init_packet(&packet);
	packet.data = nullptr;
	packet.size = 0;

	swsContext = sws_getContext(videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt, videoInfo.frameWidth, videoInfo.frameHeight, PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);

	if (!swsContext)
	{
		qWarning("Could not get sws context");
		return false;
	}

	convertedPicture = new AVPicture();

	if (avpicture_alloc(convertedPicture, PIX_FMT_RGBA, videoInfo.frameWidth, videoInfo.frameHeight) < 0)
	{
		qWarning("Could not allocate conversion picture");
		return false;
	}

	generateGrayscalePicture = settings->stabilizer.enabled;
	grayscalePictureWidth = videoCodecContext->width / settings->stabilizer.frameSizeDivisor;
	grayscalePictureHeight = videoCodecContext->height / settings->stabilizer.frameSizeDivisor;

	swsContextGrayscale = sws_getContext(videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt, grayscalePictureWidth, grayscalePictureHeight, PIX_FMT_GRAY8, SWS_BILINEAR, nullptr, nullptr, nullptr);

	if (!swsContextGrayscale)
	{
		qWarning("Could not get sws grayscale context");
		return false;
	}

	convertedPictureGrayscale = new AVPicture();

	if (avpicture_alloc(convertedPictureGrayscale, PIX_FMT_GRAY8, grayscalePictureWidth, grayscalePictureHeight) < 0)
	{
		qWarning("Could not allocate grayscale conversion picture");
		return false;
	}

	frameCountDivisor = settings->decoder.frameCountDivisor;
	frameDurationDivisor = settings->decoder.frameDurationDivisor;

	videoInfo.frameDataLength = videoInfo.frameHeight * convertedPicture->linesize[0];
	videoInfo.totalFrameCount = videoStream->nb_frames / frameCountDivisor;
	videoInfo.averageFrameRateNum = videoStream->avg_frame_rate.num / frameCountDivisor * frameDurationDivisor;
	videoInfo.averageFrameRateDen = videoStream->avg_frame_rate.den;
	videoInfo.averageFrameDuration = (double)videoInfo.averageFrameRateDen / videoInfo.averageFrameRateNum * 1000.0;
	videoInfo.averageFrameRate = (double)videoInfo.averageFrameRateNum / videoInfo.averageFrameRateDen;

	lastDecodeTime = 0.0;

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
}

bool VideoDecoder::getNextFrame(FrameData* frameData, FrameData* frameDataGrayscale)
{
	QElapsedTimer decodeTimer;
	int framesRead = 0;
	bool result = true;

	while (true)
	{
		decodeTimer.restart();

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
					result = false;
					break;
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
						frameData->duration = (int)(videoInfo.averageFrameDuration * 1000);

					if (generateGrayscalePicture && frameDataGrayscale != nullptr)
					{
						sws_scale(swsContextGrayscale, frame->data, frame->linesize, 0, frame->height, convertedPictureGrayscale->data, convertedPictureGrayscale->linesize);

						frameDataGrayscale->data = convertedPictureGrayscale->data[0];
						frameDataGrayscale->dataLength = grayscalePictureHeight * convertedPictureGrayscale->linesize[0];
						frameDataGrayscale->rowLength = convertedPictureGrayscale->linesize[0];
						frameDataGrayscale->width = grayscalePictureWidth;
						frameDataGrayscale->height = grayscalePictureHeight;
						frameDataGrayscale->duration = frameData->duration;
						frameDataGrayscale->number = frameData->number;
					}

					lastDecodeTime = decodeTimer.nsecsElapsed() / 1000000.0;

					av_free_packet(&packet);
					break;
				}
			}

			av_free_packet(&packet);
		}
		else
		{
			result = false;
			break;
		}
	}

	return result;
}

VideoInfo VideoDecoder::getVideoInfo() const
{
	return videoInfo;
}

double VideoDecoder::getLastDecodeTime() const
{
	return lastDecodeTime;
}
