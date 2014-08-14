// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#define _USE_MATH_DEFINES
#include <cmath>

#include "VideoStabilizer.h"
#include "Settings.h"
#include "FrameData.h"

#define sign(a) (((a) < 0) ? -1 : ((a) > 0))

using namespace OrientView;

void VideoStabilizer::initialize(Settings* settings)
{
	reset();

	isEnabled = settings->videoStabilizer.enabled;
	dampingFactor = settings->videoStabilizer.dampingFactor;
	maxDisplacementFactor = settings->videoStabilizer.maxDisplacementFactor;
	currentXAverage.setAlpha(settings->videoStabilizer.averagingFactor);
	currentYAverage.setAlpha(settings->videoStabilizer.averagingFactor);
	currentAngleAverage.setAlpha(settings->videoStabilizer.averagingFactor);

	if (outputData)
	{
		dataOutputFile.setFileName("stabilizer.txt");
		dataOutputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
		dataOutputFile.write("currentX;currentXAverage;normalizedX;currentY;currentYAverage;normalizedY;currentAngle;currentAngleAverage;normalizedAngle\n");
	}
}

VideoStabilizer::~VideoStabilizer()
{
	if (dataOutputFile.isOpen())
		dataOutputFile.close();
}

void VideoStabilizer::processFrame(const FrameData& frameDataGrayscale)
{
	if (!isEnabled)
		return;

	processTimer.restart();

	cv::Mat currentImage(frameDataGrayscale.height, frameDataGrayscale.width, CV_8UC1, frameDataGrayscale.data);

	if (isFirstImage)
	{
		previousImage = cv::Mat(frameDataGrayscale.height, frameDataGrayscale.width, CV_8UC1);
		currentImage.copyTo(previousImage);
		isFirstImage = false;

		return;
	}

	// find good trackable feature points from the previous image
	cv::goodFeaturesToTrack(previousImage, previousCorners, 200, 0.01, 30.0);

	// find those same points in the current image
	cv::calcOpticalFlowPyrLK(previousImage, currentImage, previousCorners, currentCorners, opticalFlowStatus, opticalFlowError);
	
	currentImage.copyTo(previousImage);

	// filter out points which didn't have a good match
	for (size_t i = 0; i < opticalFlowStatus.size(); i++)
	{
		if (opticalFlowStatus[i])
		{
			previousCornersFiltered.push_back(previousCorners[i]);
			currentCornersFiltered.push_back(currentCorners[i]);
		}
	}

	cv::Mat currentTransformation;

	// estimate the transformation between previous and current images trackable points
	if (previousCornersFiltered.size() > 0 && currentCornersFiltered.size() > 0)
		currentTransformation = cv::estimateRigidTransform(previousCornersFiltered, currentCornersFiltered, false);

	// sometimes the transformation could not be found, just use previous transformation
	if (currentTransformation.data == nullptr)
		previousTransformation.copyTo(currentTransformation);

	// a b tx
	// c d ty
	double c = currentTransformation.at<double>(1, 0);
	double d = currentTransformation.at<double>(1, 1);
	double tx = currentTransformation.at<double>(0, 2);
	double ty = currentTransformation.at<double>(1, 2);

	currentTransformation.copyTo(previousTransformation);

	double deltaX = tx / frameDataGrayscale.width;
	double deltaY = ty / frameDataGrayscale.height;
	double deltaAngle = atan2(c, d) * 180.0 / M_PI;
	//double deltaScale = sign(a) * sqrt(a * a + b * b); // not used, only causes jitter

	// current* values track the cumulative trajectory of the image
	// these are not bounded in anyway
	currentX += deltaX;
	currentY += deltaY;
	currentAngle += deltaAngle;

	// normalized* filter out the trajectory and only give out delta values respect to the average trajectory
	// these are centered at zero
	normalizedX = (currentXAverage.getAverage() - currentX) * dampingFactor;
	normalizedY = (currentYAverage.getAverage() - currentY) * dampingFactor;
	normalizedAngle = (currentAngleAverage.getAverage() - currentAngle) * dampingFactor;

	normalizedX = std::max(-maxDisplacementFactor, std::min(normalizedX, maxDisplacementFactor));
	normalizedY = std::max(-maxDisplacementFactor, std::min(normalizedY, maxDisplacementFactor));

	// calculate exponential moving average of the current values
	currentXAverage.addMeasurement(currentX);
	currentYAverage.addMeasurement(currentY);
	currentAngleAverage.addMeasurement(currentAngle);

	if (outputData)
	{
		char buffer[1024];
		sprintf(buffer, "%f;%f;%f;%f;%f;%f;%f;%f;%f;\n", currentX, currentXAverage.getAverage(), normalizedX, currentY, currentYAverage.getAverage(), normalizedY, currentAngle, currentAngleAverage.getAverage(), normalizedAngle);
		dataOutputFile.write(buffer);
	}

	previousCorners.clear();
	currentCorners.clear();
	previousCornersFiltered.clear();
	currentCornersFiltered.clear();
	opticalFlowStatus.clear();
	opticalFlowError.clear();

	lastProcessTime = processTimer.nsecsElapsed() / 1000000.0;
}

void VideoStabilizer::toggleEnabled()
{
	isEnabled = !isEnabled;
	reset();
}

void VideoStabilizer::reset()
{
	currentX = 0.0;
	currentY = 0.0;
	currentAngle = 0.0;
	normalizedX = 0.0;
	normalizedY = 0.0;
	normalizedAngle = 0.0;
	currentXAverage.reset();
	currentYAverage.reset();
	currentAngleAverage.reset();
	previousTransformation = cv::Mat::eye(2, 3, CV_64F);
}

double VideoStabilizer::getX() const
{
	return normalizedX;
}

double VideoStabilizer::getY() const
{
	return normalizedY;
}

double VideoStabilizer::getAngle() const
{
	return normalizedAngle;
}

double VideoStabilizer::getLastProcessTime() const
{
	return lastProcessTime;
}
