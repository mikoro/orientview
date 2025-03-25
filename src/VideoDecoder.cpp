// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

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

namespace
{
	bool enableVerboseLogging = false;

	void ffmpegLogCallback(void* ptr, int level, const char* fmt, va_list vl)
	{
		int printPrefix = 1;
		char line[1024] = { 0 };
		av_log_format_line(ptr, level, fmt, vl, line, 1024, &printPrefix);
		size_t length = strlen(line);
		char lineClipped[1024] = { 0 };
		strncpy(lineClipped, line, length - 1);

		if (level <= AV_LOG_WARNING)
			qWarning("%s", lineClipped);
		else if (enableVerboseLogging && level <= AV_LOG_DEBUG)
			qDebug("%s", lineClipped);
	}

	bool openCodecContext(int* streamIndex, AVFormatContext* formatContext, AVMediaType mediaType, AVCodecContext** codecContext)
	{
		*streamIndex = av_find_best_stream(formatContext, mediaType, -1, -1, nullptr, 0);

		if (*streamIndex < 0)
		{
			qWarning("Could not find %s stream in input file", av_get_media_type_string(mediaType));
			return false;
		}
		else
		{
			AVCodecParameters* codecParams = formatContext->streams[*streamIndex]->codecpar;
			const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);

			if (!codec)
			{
				qWarning("Could not find %s codec", av_get_media_type_string(mediaType));
				return false;
			}

			*codecContext = avcodec_alloc_context3(codec);
			if (!*codecContext)
			{
				qWarning("Could not allocate codec context for %s", av_get_media_type_string(mediaType));
				return false;
			}

			if (avcodec_parameters_to_context(*codecContext, codecParams) < 0)
			{
				qWarning("Could not copy codec parameters to context for %s", av_get_media_type_string(mediaType));
				avcodec_free_context(codecContext);
				return false;
			}

			AVDictionary* opts = nullptr;

			if (avcodec_open2(*codecContext, codec, &opts) < 0)
			{
				qWarning("Could not open %s codec", av_get_media_type_string(mediaType));
				avcodec_free_context(codecContext);
				return false;
			}
		}

		return true;
	}
}

bool VideoDecoder::initialize(Settings* settings)
{
	qDebug("Initializing video decoder (%s)", qPrintable(settings->video.inputVideoFilePath));

	enableVerboseLogging = settings->video.enableVerboseLogging;
	seekToAnyFrame = settings->video.seekToAnyFrame;

	av_log_set_level(enableVerboseLogging ? AV_LOG_DEBUG : AV_LOG_WARNING);
	av_log_set_callback(ffmpegLogCallback);
	// av_register_all() is deprecated and not needed in newer FFmpeg versions

	if (avformat_open_input(&formatContext, settings->video.inputVideoFilePath.toUtf8().constData(), nullptr, nullptr) < 0)
	{
		qWarning("Could not open source file");
		return false;
	}

	if (avformat_find_stream_info(formatContext, nullptr) < 0)
	{
		qWarning("Could not find stream information");
		return false;
	}

	if (!openCodecContext(&videoStreamIndex, formatContext, AVMEDIA_TYPE_VIDEO, &videoCodecContext))
	{
		qWarning("Could not open video codec context");
		return false;
	}

	frame = av_frame_alloc();

	if (!frame)
	{
		qWarning("Could not allocate frame");
		return false;
	}

	// Initialize packet
	av_init_packet(&packet);
	packet.data = nullptr;
	packet.size = 0;

	videoStream = formatContext->streams[(size_t)videoStreamIndex];
	// videoCodecContext is now set by openCodecContext

	frameWidth = videoCodecContext->width / settings->video.frameSizeDivisor;
	frameHeight = videoCodecContext->height / settings->video.frameSizeDivisor;

	swsContext = sws_getContext(videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt, frameWidth, frameHeight, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);

	if (!swsContext)
	{
		qWarning("Could not get sws context");
		return false;
	}

	convertedPicture = av_frame_alloc();
	if (!convertedPicture)
	{
		qWarning("Could not allocate conversion frame");
		return false;
	}

	convertedPicture->format = AV_PIX_FMT_RGBA;
	convertedPicture->width = frameWidth;
	convertedPicture->height = frameHeight;

	if (av_frame_get_buffer(convertedPicture, 0) < 0)
	{
		qWarning("Could not allocate conversion frame buffer");
		return false;
	}

	grayscaleFrameWidth = videoCodecContext->width / settings->stabilizer.frameSizeDivisor;
	grayscaleFrameHeight = videoCodecContext->height / settings->stabilizer.frameSizeDivisor;

	swsContextGrayscale = sws_getContext(videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt, grayscaleFrameWidth, grayscaleFrameHeight, AV_PIX_FMT_GRAY8, SWS_BILINEAR, nullptr, nullptr, nullptr);

	if (!swsContextGrayscale)
	{
		qWarning("Could not get sws grayscale context");
		return false;
	}

	convertedPictureGrayscale = av_frame_alloc();
	if (!convertedPictureGrayscale)
	{
		qWarning("Could not allocate grayscale conversion frame");
		return false;
	}

	convertedPictureGrayscale->format = AV_PIX_FMT_GRAY8;
	convertedPictureGrayscale->width = grayscaleFrameWidth;
	convertedPictureGrayscale->height = grayscaleFrameHeight;

	if (av_frame_get_buffer(convertedPictureGrayscale, 0) < 0)
	{
		qWarning("Could not allocate grayscale conversion frame buffer");
		return false;
	}

	frameCountDivisor = settings->video.frameCountDivisor;
	frameDurationDivisor = settings->video.frameDurationDivisor;

	totalFrameCount = videoStream->nb_frames / frameCountDivisor;

	frameRateNum = (int64_t)videoStream->r_frame_rate.num / frameCountDivisor * frameDurationDivisor;
	frameRateDen = (int64_t)videoStream->r_frame_rate.den;
	frameDuration = frameRateDen * 1000000 / frameRateNum;

	totalDurationInSeconds = ((double)videoStream->time_base.num / videoStream->time_base.den) * videoStream->duration;

	isInitialized = true;
	isFinished = false;

	if (settings->video.startTimeOffset > 0.0)
		seekRelative(settings->video.startTimeOffset);

	return true;
}

VideoDecoder::~VideoDecoder()
{
	if (videoCodecContext != nullptr)
	{
		avcodec_free_context(&videoCodecContext);
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
		av_frame_free(&convertedPictureGrayscale);
		convertedPictureGrayscale = nullptr;
	}

	if (swsContext != nullptr)
	{
		sws_freeContext(swsContext);
		swsContext = nullptr;
	}

	if (convertedPicture != nullptr)
	{
		av_frame_free(&convertedPicture);
		convertedPicture = nullptr;
	}
}

bool VideoDecoder::getNextFrame(FrameData* frameData, FrameData* frameDataGrayscale)
{
	QMutexLocker locker(&decoderMutex);

	if (!isInitialized)
		return false;

	decodeDurationTimer.restart();

	int framesRead = 0;
	int readResult = 0;

	while (true)
	{
		if ((readResult = av_read_frame(formatContext, &packet)) >= 0)
		{
			if (packet.stream_index == videoStreamIndex)
			{
				// Send packet to decoder
				int sendResult = avcodec_send_packet(videoCodecContext, &packet);
				if (sendResult < 0)
				{
					qWarning("Error sending packet for decoding: %d", sendResult);
					av_packet_unref(&packet);
					return false;
				}

				// Receive frame from decoder
				int receiveResult = avcodec_receive_frame(videoCodecContext, frame);
				if (receiveResult == AVERROR(EAGAIN) || receiveResult == AVERROR_EOF)
				{
					// Need more packets or end of file
					av_packet_unref(&packet);
					continue;
				}
				else if (receiveResult < 0)
				{
					qWarning("Error during decoding: %d", receiveResult);
					av_packet_unref(&packet);
					return false;
				}

				if (++framesRead < frameCountDivisor)
				{
					av_packet_unref(&packet);
					continue;
				}

				framesRead = 0;
				cumulativeFrameNumber++;

				// We have a valid frame
				if (frameData != nullptr)
				{
					sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, convertedPicture->data, convertedPicture->linesize);

					frameData->data = convertedPicture->data[0];
					frameData->dataLength = (size_t)(frameHeight * convertedPicture->linesize[0]);
					frameData->rowLength = (size_t)(convertedPicture->linesize[0]);
					frameData->width = frameWidth;
					frameData->height = frameHeight;
					frameData->duration = av_rescale((frame->best_effort_timestamp - previousFrameTimestamp) * 1000000 / frameDurationDivisor, videoStream->time_base.num, videoStream->time_base.den);
					frameData->timeStamp = frame->best_effort_timestamp;
					frameData->cumulativeNumber = cumulativeFrameNumber;

					if (frameData->duration <= 0 || frameData->duration > 1000000)
						frameData->duration = frameDuration;
				}

				if (frameDataGrayscale != nullptr)
				{
					sws_scale(swsContextGrayscale, frame->data, frame->linesize, 0, frame->height, convertedPictureGrayscale->data, convertedPictureGrayscale->linesize);

					frameDataGrayscale->data = convertedPictureGrayscale->data[0];
					frameDataGrayscale->dataLength = (size_t)(grayscaleFrameHeight * convertedPictureGrayscale->linesize[0]);
					frameDataGrayscale->rowLength = (size_t)(convertedPictureGrayscale->linesize[0]);
					frameDataGrayscale->width = grayscaleFrameWidth;
					frameDataGrayscale->height = grayscaleFrameHeight;
					frameDataGrayscale->duration = (int)av_rescale((frame->best_effort_timestamp - previousFrameTimestamp) * 1000000 / frameDurationDivisor, videoStream->time_base.num, videoStream->time_base.den);
					frameDataGrayscale->timeStamp = frame->best_effort_timestamp;
					frameDataGrayscale->cumulativeNumber = cumulativeFrameNumber;

					if (frameDataGrayscale->duration <= 0 || frameDataGrayscale->duration > 1000000)
						frameDataGrayscale->duration = frameDuration;
				}

				double totalDurationInSeconds = ((double)videoStream->time_base.num / videoStream->time_base.den) * videoStream->duration;
				currentTimeInSeconds = ((double)frame->best_effort_timestamp / videoStream->duration) * totalDurationInSeconds;
				previousFrameTimestamp = frame->best_effort_timestamp;
				decodeDuration = decodeDurationTimer.nsecsElapsed() / 1000000.0;
				isFinished = false;

				av_packet_unref(&packet);
				return true;
			}

			av_packet_unref(&packet);
		}
		else
		{
			if (readResult != AVERROR_EOF)
				qWarning("Could not read a frame: %d", readResult);

			decodeDuration = decodeDurationTimer.nsecsElapsed() / 1000000.0;
			isFinished = true;

			return false;
		}
	}
}

void VideoDecoder::seekRelative(double seconds)
{
	QMutexLocker locker(&decoderMutex);

	if (!isInitialized)
		return;

	int64_t targetTimeStamp = previousFrameTimestamp + (int64_t)(((double)videoStream->time_base.den / videoStream->time_base.num) * seconds + 0.5);
	targetTimeStamp = std::max((int64_t)0, std::min(targetTimeStamp, videoStream->duration));

	if (avformat_seek_file(formatContext, (int)videoStreamIndex, 0, targetTimeStamp, targetTimeStamp, (seekToAnyFrame ? AVSEEK_FLAG_ANY : 0)) >= 0)
	{
		avcodec_flush_buffers(videoCodecContext);

		bool gotPicture = false;

		while (!gotPicture)
		{
			int readResult;

			if ((readResult = av_read_frame(formatContext, &packet)) >= 0)
			{
				if (packet.stream_index == videoStreamIndex)
				{
					// Send packet to decoder
					int sendResult = avcodec_send_packet(videoCodecContext, &packet);
					if (sendResult < 0)
					{
						qWarning("Error sending packet for decoding during seek: %d", sendResult);
						av_packet_unref(&packet);
						continue;
					}

					// Receive frame from decoder
					int receiveResult = avcodec_receive_frame(videoCodecContext, frame);
					if (receiveResult == AVERROR(EAGAIN) || receiveResult == AVERROR_EOF)
					{
						// Need more packets or end of file
						av_packet_unref(&packet);
						continue;
					}
					else if (receiveResult < 0)
					{
						qWarning("Error during decoding during seek: %d", receiveResult);
						av_packet_unref(&packet);
						continue;
					}

					// We have a valid frame
					gotPicture = true;
				}

				av_packet_unref(&packet);
			}
			else
			{
				if (readResult != AVERROR_EOF)
					qWarning("Could not read a frame: %d", readResult);

				isFinished = true;
				return;
			}
		}

		isFinished = false;
	}
	else
		qWarning("Could not seek video");
}

bool VideoDecoder::getIsFinished()
{
	QMutexLocker locker(&decoderMutex);

	return isFinished;
}

double VideoDecoder::getCurrentTime()
{
	QMutexLocker locker(&decoderMutex);

	return currentTimeInSeconds;
}

double VideoDecoder::getDecodeDuration()
{
	QMutexLocker locker(&decoderMutex);

	return decodeDuration;
}

void  VideoDecoder::resetDecodeDuration()
{
	QMutexLocker locker(&decoderMutex);

	decodeDuration = 0.0;
}

int VideoDecoder::getFrameWidth() const
{
	return frameWidth;
}

int VideoDecoder::getFrameHeight() const
{
	return frameHeight;
}

int64_t VideoDecoder::getTotalFrameCount() const
{
	return totalFrameCount;
}

int64_t VideoDecoder::getFrameRateNum() const
{
	return frameRateNum;
}

int64_t VideoDecoder::getFrameRateDen() const
{
	return frameRateDen;
}

double VideoDecoder::getFrameDuration() const
{
	return (double)frameDuration / 1000.0;
}

double VideoDecoder::getTotalDuration() const
{
	return totalDurationInSeconds;
}
