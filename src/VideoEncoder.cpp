// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "VideoEncoder.h"
#include "VideoDecoder.h"
#include "Settings.h"

using namespace OrientView;

VideoEncoder::VideoEncoder()
{
}

bool VideoEncoder::initialize(const QString& fileName, VideoDecoder* videoDecoder, Settings* settings)
{
	return true;
}

void VideoEncoder::shutdown()
{
}
