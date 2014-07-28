// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QImage>

#include "opencv2/opencv.hpp"

#include "VideoStabilizer.h"
#include "FrameData.h"

using namespace OrientView;

VideoStabilizer::VideoStabilizer()
{
}

bool VideoStabilizer::initialize()
{
	qDebug("Initializing VideoStabilizer");

	return true;
}

void VideoStabilizer::shutdown()
{
	qDebug("Shutting down VideoStabilizer");
}

void VideoStabilizer::processFrame(FrameData* frameDataGrayscale)
{
	cv::Mat(frameDataGrayscale->width, frameDataGrayscale->height, CV_8UC1, frameDataGrayscale->data, frameDataGrayscale->rowLength);
}
