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

	return true;
}

void VideoStabilizer::shutdown()
{
	qDebug("Shutting down VideoStabilizer");
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

	//cv::imshow("Display window", previousImage);
	//cv::waitKey(0);

	std::vector<cv::Point2f> previousCorners;
	std::vector<cv::Point2f> currentCorners;
	std::vector<cv::Point2f> previousCornersFiltered;
	std::vector<cv::Point2f> currentCornersFiltered;
	std::vector<uchar> status;
	std::vector<float> error;

	cv::goodFeaturesToTrack(previousImage, previousCorners, 200, 0.01, 30.0);
	cv::calcOpticalFlowPyrLK(previousImage, currentImage, previousCorners, currentCorners, status, error);

	currentImage.copyTo(previousImage);

	for (size_t i = 0; i < status.size(); i++)
	{
		if (status[i])
		{
			previousCornersFiltered.push_back(previousCorners[i]);
			currentCornersFiltered.push_back(currentCorners[i]);
		}
	}

	cv::Mat transform = cv::estimateRigidTransform(previousCornersFiltered, currentCornersFiltered, false);
	
	double dx = transform.at<double>(0, 2);
	double dy = transform.at<double>(1, 2);
	double da = atan2(transform.at<double>(1, 0), transform.at<double>(0, 0));

	qDebug("dx: %f dy: %f da: %f", dx, dy, da);
}
