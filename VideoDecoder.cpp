// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QtGlobal>

#include "VideoDecoder.h"

extern "C"
{
	#include <libavformat/avformat.h>
}

using namespace OrientView;

bool VideoDecoder::isRegistered = false;

VideoDecoder::VideoDecoder()
{
}

VideoDecoder::~VideoDecoder()
{
}

void VideoDecoder::LoadVideo(const std::string& fileName)
{
	if (!isRegistered)
	{
		qDebug("Initializing libavformat");

		av_register_all();
		isRegistered = true;
	}
}
