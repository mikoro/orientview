// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "VideoEncoder.h"
#include "VideoDecoder.h"
#include "Settings.h"
#include "FrameData.h"
#include "MP4File.h"

namespace
{
	void x264_log(void* unused, int level, const char* format, va_list args)
	{
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), format, args);
		
		switch (level)
		{
			case X264_LOG_ERROR:
				qCritical(buffer);
				break;
			case X264_LOG_WARNING:
				qWarning(buffer);
				break;
			case X264_LOG_INFO:
				qDebug(buffer);
				break;
			case X264_LOG_DEBUG:
				qDebug(buffer);
				break;
			default:
				qDebug(buffer);
				break;
		}
	}
}

using namespace OrientView;

VideoEncoder::VideoEncoder()
{
}

bool VideoEncoder::initialize(const QString& fileName, VideoDecoder* videoDecoder, Settings* settings)
{
	qDebug("Initializing VideoEncoder (%s)", fileName.toLocal8Bit().constData());

	x264_param_t param;
	
	if (x264_param_default_preset(&param, settings->encoder.preset.toLocal8Bit().constData(), "zerolatency") < 0)
	{
		qWarning("Could not apply presets");
		return false;
	}

	param.i_width = settings->display.width;
	param.i_height = settings->display.height;
	param.i_fps_num = videoDecoder->getVideoInfo().averageFrameRateNum;
	param.i_fps_den = videoDecoder->getVideoInfo().averageFrameRateDen;
	param.i_timebase_num = videoDecoder->getVideoInfo().averageFrameRateDen;
	param.i_timebase_den = videoDecoder->getVideoInfo().averageFrameRateNum;
	param.i_csp = X264_CSP_I420;
	param.rc.i_rc_method = X264_RC_CRF;
	param.rc.f_rf_constant = settings->encoder.constantRateFactor;
	//param.pf_log = x264_log; // enabling log makes things unstable
	param.i_log_level = X264_LOG_NONE;
	
	x264_param_apply_fastfirstpass(&param);

	if (x264_param_apply_profile(&param, settings->encoder.profile.toLocal8Bit().constData()) < 0)
	{
		qWarning("Could not apply profile");
		return false;
	}

	param.b_annexb = 0;
	param.b_repeat_headers = 0;

	encoder = x264_encoder_open(&param);

	if (!encoder)
	{
		qWarning("Could not open encoder");
		return false;
	}

	x264_encoder_parameters(encoder, &param);

	convertedPicture = new x264_picture_t();

	if (x264_picture_alloc(convertedPicture, X264_CSP_I420, settings->display.width, settings->display.height) < 0)
	{
		qWarning("Could not allocate encoder picture");
		return false;
	}

	swsContext = sws_getContext(settings->display.width, settings->display.height, PIX_FMT_RGBA, settings->display.width, settings->display.height, PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);

	if (!swsContext)
	{
		qWarning("Could not get sws context");
		return false;
	}

	mp4File = new MP4File();

	if (!mp4File->initialize(fileName))
		return false;

	if (!mp4File->setParam(&param))
		return false;

	x264_nal_t* nal;
	int nalCount;

	if (x264_encoder_headers(encoder, &nal, &nalCount) < 0)
	{
		qWarning("Could not get encoder headers");
		return false;
	}

	if (!mp4File->writeHeaders(nal))
		return false;

	return true;
}

void VideoEncoder::shutdown()
{
	qDebug("Shutting down VideoEncoder");

	if (mp4File != nullptr)
	{
		mp4File->finalize(frameNumber);
		mp4File->shutdown();
		delete mp4File;
		mp4File = nullptr;
	}

	if (swsContext != nullptr)
	{
		sws_freeContext(swsContext);
		swsContext = nullptr;
	}

	if (swsContext != nullptr)
	{
		x264_picture_clean(convertedPicture);
		delete convertedPicture;
		convertedPicture = nullptr;
	}

	if (encoder != nullptr)
	{
		x264_encoder_close(encoder);
		encoder = nullptr;
	}

	frameNumber = 0;
}

void VideoEncoder::loadFrameData(FrameData* frameData)
{
	sws_scale(swsContext, &frameData->data, &frameData->rowLength, 0, frameData->height, convertedPicture->img.plane, convertedPicture->img.i_stride);
}

void VideoEncoder::encodeFrame()
{
	x264_picture_t encodedPicture;
	x264_nal_t* nal;
	int nalCount;

	convertedPicture->i_pts = frameNumber++;

	int frameSize = x264_encoder_encode(encoder, &nal, &nalCount, convertedPicture, &encodedPicture);
	
	if (frameSize > 0)
		mp4File->writeFrame(nal[0].p_payload, frameSize, &encodedPicture);
	else
		qWarning("Could not encode frame");
}
