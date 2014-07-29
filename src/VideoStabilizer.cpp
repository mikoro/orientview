// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QImage>

#include "VideoStabilizer.h"
#include "FrameData.h"

using namespace OrientView;

VideoStabilizer::VideoStabilizer()
{
}

bool VideoStabilizer::initialize(Settings* settings)
{
	qDebug("Initializing VideoStabilizer");

	isFirstImage = true;

	output.setFileName("data.txt");
	output.open(QIODevice::ReadWrite);

	return true;
}

void VideoStabilizer::shutdown()
{
	qDebug("Shutting down VideoStabilizer");

	output.close();
}

void VideoStabilizer::processFrame(FrameData* frameDataGrayscale)
{
	cv::Mat currentImage(frameDataGrayscale->height, frameDataGrayscale->width, CV_8UC1, frameDataGrayscale->data);

	if (isFirstImage)
	{
		previousImage = cv::Mat(frameDataGrayscale->height, frameDataGrayscale->width, CV_8UC1);
		currentImage.copyTo(previousImage);
		isFirstImage = false;

		return;
	}

	cv::goodFeaturesToTrack(previousImage, previousCorners, 200, 0.01, 30.0);
	cv::calcOpticalFlowPyrLK(previousImage, currentImage, previousCorners, currentCorners, opticalFlowStatus, opticalFlowError);

	currentImage.copyTo(previousImage);

	for (size_t i = 0; i < opticalFlowStatus.size(); i++)
	{
		if (opticalFlowStatus[i])
		{
			previousCornersFiltered.push_back(previousCorners[i]);
			currentCornersFiltered.push_back(currentCorners[i]);
		}
	}

	cv::Mat transform = cv::estimateRigidTransform(previousCornersFiltered, currentCornersFiltered, false);
	
	deltaX = transform.at<double>(0, 2) / frameDataGrayscale->width;
	deltaY = transform.at<double>(1, 2) / frameDataGrayscale->height;
	deltaAngle = atan2(transform.at<double>(1, 0), transform.at<double>(0, 0));

	accumulatedX += deltaX;
	accumulatedY += deltaY;
	accumulatedAngle += deltaAngle;

	accumulatedX -= accumulatedX / 10;
	accumulatedY -= accumulatedY / 10;
	accumulatedAngle -= accumulatedAngle / 10;

	previousCorners.clear();
	currentCorners.clear();
	previousCornersFiltered.clear();
	currentCornersFiltered.clear();
	opticalFlowStatus.clear();
	opticalFlowError.clear();

	//output.write(QString::number(da).toLocal8Bit().constData());
	//output.write("\n");
}

double VideoStabilizer::getX() const
{
	return accumulatedX;
}

double VideoStabilizer::getY() const
{
	return accumulatedY;
}

double VideoStabilizer::getAngle() const
{
	return accumulatedAngle;
}
