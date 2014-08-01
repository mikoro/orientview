// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QElapsedTimer>

#include "VideoStabilizer.h"
#include "FrameData.h"

#define PI 3.14159265358979323846
#define sign(a) (((a) < 0) ? -1 : ((a) > 0))

using namespace OrientView;

VideoStabilizer::VideoStabilizer()
{
}

bool VideoStabilizer::initialize(Settings* settings)
{
	qDebug("Initializing VideoStabilizer");

	isFirstImage = true;

	dxCum = 0.0;
	dyCum = 0.0;
	dsxCum = 1.0;
	dsyCum = 1.0;
	daCum = 0.0;

	averageProcessTime = 0.0;

	previousTransform = cv::Mat::eye(2, 3, CV_64F);

	if (outputData)
	{
		dataOutputFile.setFileName("stabilizer.txt");
		dataOutputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
		dataOutputFile.write("dx;dy;da;dsx;dsy;dxCum;dyCum;daCum;dsxCum;dsyCum\n");
	}

	return true;
}

void VideoStabilizer::shutdown()
{
	qDebug("Shutting down VideoStabilizer");

	if (dataOutputFile.isOpen())
		dataOutputFile.close();
}

void VideoStabilizer::processFrame(FrameData* frameDataGrayscale)
{
	QElapsedTimer processTimer;
	processTimer.start();

	cv::Mat currentImage(frameDataGrayscale->height, frameDataGrayscale->width, CV_8UC1, frameDataGrayscale->data);

	if (isFirstImage)
	{
		previousImage = cv::Mat(frameDataGrayscale->height, frameDataGrayscale->width, CV_8UC1);
		currentImage.copyTo(previousImage);
		isFirstImage = false;

		return;
	}

	bool failed = false;

	try
	{
		cv::goodFeaturesToTrack(previousImage, previousCorners, 200, 0.01, 30.0);
	}
	catch (cv::Exception& ex)
	{
		qWarning("Could not execute goodFeaturesToTrack: %s", ex.what());
		failed = true;
	}

	try
	{
		cv::calcOpticalFlowPyrLK(previousImage, currentImage, previousCorners, currentCorners, opticalFlowStatus, opticalFlowError);
	}
	catch (cv::Exception& ex)
	{
		qWarning("Could not execute calcOpticalFlowPyrLK: %s", ex.what());
		failed = true;
	}

	currentImage.copyTo(previousImage);

	for (size_t i = 0; i < opticalFlowStatus.size(); i++)
	{
		if (opticalFlowStatus[i])
		{
			previousCornersFiltered.push_back(previousCorners[i]);
			currentCornersFiltered.push_back(currentCorners[i]);
		}
	}

	cv::Mat transform;
	
	try
	{
		transform = cv::estimateRigidTransform(previousCornersFiltered, currentCornersFiltered, false);
	}
	catch (cv::Exception& ex)
	{
		qWarning("Could not execute estimateRigidTransform: %s", ex.what());
		failed = true;
	}

	if (transform.data == nullptr)
		failed = true;
		
	if (failed)
		previousTransform.copyTo(transform);

	// a b tx
	// c d ty
	double a = transform.at<double>(0, 0);
	double b = transform.at<double>(0, 1);
	double c = transform.at<double>(1, 0);
	double d = transform.at<double>(1, 1);
	double tx = transform.at<double>(0, 2);
	double ty = transform.at<double>(1, 2);

	transform.copyTo(previousTransform);

	double dx = tx / frameDataGrayscale->width;
	double dy = ty / frameDataGrayscale->height;
	double da = atan2(c, d) * 180.0 / PI;
	double dsx = sign(a) * sqrt(a * a + b * b);
	double dsy = sign(d) * sqrt(c * c + d * d);

	dxCum += dx;
	dyCum += dy;
	daCum += da;
	dsxCum *= dsx;
	dsyCum *= dsy;

	if (outputData)
	{
		char buffer[1024];
		sprintf(buffer, "%f;%f;%f;%f;%f;%f;%f;%f;%f;%f\n", dx, dy, da, dsx, dsy, dxCum, dyCum, daCum, dsxCum, dsyCum);
		dataOutputFile.write(buffer);
	}

	previousCorners.clear();
	currentCorners.clear();
	previousCornersFiltered.clear();
	currentCornersFiltered.clear();
	opticalFlowStatus.clear();
	opticalFlowError.clear();

	averageProcessTime = processTimer.nsecsElapsed() / 1000000.0;
}

double VideoStabilizer::getX() const
{
	return 0;
}

double VideoStabilizer::getY() const
{
	return 0;
}

double VideoStabilizer::getAngle() const
{
	return daCum;
}

double VideoStabilizer::getScaleX() const
{
	return dsxCum;
}

double VideoStabilizer::getScaleY() const
{
	return dsyCum;
}

double VideoStabilizer::getAverageProcessTime() const
{
	return averageProcessTime;
}
