// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>

#include <QTextStream>

#include "VideoStabilizer.h"
#include "Settings.h"
#include "FrameData.h"

#define sign(a) (((a) < 0) ? -1 : ((a) > 0))

using namespace OrientView;

bool VideoStabilizer::initialize(Settings* settings, bool isPreprocessing)
{
	mode = settings->stabilizer.mode;
	isEnabled = settings->stabilizer.enabled;
	cumulativeXAverage.setAlpha(settings->stabilizer.averagingFactor);
	cumulativeYAverage.setAlpha(settings->stabilizer.averagingFactor);
	cumulativeAngleAverage.setAlpha(settings->stabilizer.averagingFactor);
	dampingFactor = settings->stabilizer.dampingFactor;
	maxDisplacementFactor = settings->stabilizer.maxDisplacementFactor;

	reset();

	if (!isPreprocessing && mode == VideoStabilizerMode::Preprocessed)
	{
		if (!readNormalizedFramePositions(settings->stabilizer.inputDataFilePath))
			return false;
	}

	return true;
}

void VideoStabilizer::preProcessFrame(const FrameData& frameDataGrayscale, QFile& file)
{
	FramePosition cumulativeFramePosition = calculateCumulativeFramePosition(frameDataGrayscale);

	char buffer[1024];
	sprintf(buffer, "%lld;%.16le;%.16le;%.16le\n", (long long int)cumulativeFramePosition.timeStamp, cumulativeFramePosition.x, cumulativeFramePosition.y, cumulativeFramePosition.angle);
	file.write(buffer);
}

void VideoStabilizer::processFrame(const FrameData& frameDataGrayscale)
{
	if (!isEnabled)
		return;

	processTimer.restart();

	if (mode == VideoStabilizerMode::Preprocessed)
		normalizedFramePosition = searchNormalizedFramePosition(frameDataGrayscale);
	else
	{
		FramePosition cumulativeFramePosition = calculateCumulativeFramePosition(frameDataGrayscale);

		normalizedFramePosition.x = cumulativeXAverage.getAverage() - cumulativeFramePosition.x;
		normalizedFramePosition.y = cumulativeYAverage.getAverage() - cumulativeFramePosition.y;
		normalizedFramePosition.angle = cumulativeAngleAverage.getAverage() - cumulativeFramePosition.angle;

		cumulativeXAverage.addMeasurement(cumulativeFramePosition.x);
		cumulativeYAverage.addMeasurement(cumulativeFramePosition.y);
		cumulativeAngleAverage.addMeasurement(cumulativeFramePosition.angle);
	}

	normalizedFramePosition.x *= dampingFactor;
	normalizedFramePosition.y *= dampingFactor;

	normalizedFramePosition.x = std::max(-maxDisplacementFactor, std::min(normalizedFramePosition.x, maxDisplacementFactor));
	normalizedFramePosition.y = std::max(-maxDisplacementFactor, std::min(normalizedFramePosition.y, maxDisplacementFactor));

	lastProcessTime = processTimer.nsecsElapsed() / 1000000.0;
}

FramePosition VideoStabilizer::calculateCumulativeFramePosition(const FrameData& frameDataGrayscale)
{
	cv::Mat currentImage(frameDataGrayscale.height, frameDataGrayscale.width, CV_8UC1, frameDataGrayscale.data);

	if (isFirstImage)
	{
		previousImage = cv::Mat(frameDataGrayscale.height, frameDataGrayscale.width, CV_8UC1);
		currentImage.copyTo(previousImage);
		isFirstImage = false;
	}

	std::vector<cv::Point2f> previousCorners;
	std::vector<cv::Point2f> previousCornersFiltered;
	std::vector<cv::Point2f> currentCorners;
	std::vector<cv::Point2f> currentCornersFiltered;
	std::vector<uchar> opticalFlowStatus;
	std::vector<float> opticalFlowError;

	// find good trackable feature points from the previous image
	cv::goodFeaturesToTrack(previousImage, previousCorners, 200, 0.01, 30.0);

	// find those same points in the current image
	cv::calcOpticalFlowPyrLK(previousImage, currentImage, previousCorners, currentCorners, opticalFlowStatus, opticalFlowError);

	currentImage.copyTo(previousImage);

	// filter out points which didn't have a good match
	for (size_t i = 0; i < opticalFlowStatus.size(); i++)
	{
		if (opticalFlowStatus.at(i) != 0)
		{
			previousCornersFiltered.push_back(previousCorners.at(i));
			currentCornersFiltered.push_back(currentCorners.at(i));
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
	
	cumulativeX += deltaX;
	cumulativeY += deltaY;
	cumulativeAngle += deltaAngle;

	FramePosition fp;
	fp.timeStamp = frameDataGrayscale.timeStamp;
	fp.x = cumulativeX;
	fp.y = cumulativeY;
	fp.angle = cumulativeAngle;

	return fp;
}

FramePosition VideoStabilizer::searchNormalizedFramePosition(const FrameData& frameDataGrayscale)
{
	FramePosition result;

	auto comparator = [](const OrientView::FramePosition& fp, const int64_t timeStamp) { return fp.timeStamp < timeStamp; };
	auto searchResult = std::lower_bound(normalizedFramePositions.begin(), normalizedFramePositions.end(), frameDataGrayscale.timeStamp, comparator);

	if (searchResult != normalizedFramePositions.end() && (*searchResult).timeStamp >= frameDataGrayscale.timeStamp)
		result = *searchResult;
	
	return result;
}

void VideoStabilizer::convertCumulativeFramePositionsToNormalized(QFile& fileIn, QFile& fileOut, int smoothingRadius)
{
	QTextStream fileInStream(&fileIn);
	QString fileInString = fileInStream.readAll();

	std::vector<FramePosition> positions;

	QStringList lines = fileInString.split('\n');

	for (int i = 1; i < lines.size(); ++i)
	{
		QStringList parts = lines.at(i).split(';');

		if (parts.size() == 4)
		{
			FramePosition fp;

			fp.timeStamp = (int64_t)parts[0].toLongLong();
			fp.x = parts[1].toDouble();
			fp.y = parts[2].toDouble();
			fp.angle = parts[3].toDouble();

			positions.push_back(fp);
		}
	}

	fileOut.write("timeStamp;cumulativeX;averageX;normalizedX;cumulativeY;averageY;normalizedY;cumulativeAngle;averageAngle;normalizedAngle\n");

	for (int i = 0; i < (int)positions.size(); ++i)
	{
		double sumX = 0.0;
		double sumY = 0.0;
		double sumAngle = 0.0;
		int sumCount = 0;

		for (int j = -smoothingRadius; j <= smoothingRadius; ++j)
		{
			if ((i + j) >= 0 && (i + j) < (int)positions.size())
			{
				FramePosition fp = positions.at(i + j);

				sumX += fp.x;
				sumY += fp.y;
				sumAngle += fp.angle;

				sumCount++;
			}
		}

		double averageX = sumX / (double)sumCount;
		double averageY = sumY / (double)sumCount;
		double averageAngle = sumAngle / (double)sumCount;

		FramePosition currentFp = positions.at(i);
		FramePosition normalizedFp;

		normalizedFp.x = averageX - currentFp.x;
		normalizedFp.y = averageY - currentFp.y;
		normalizedFp.angle = averageAngle - currentFp.angle;

		char buffer[1024];
		sprintf(buffer, "%lld;%.16le;%.16le;%.16le;%.16le;%.16le;%.16le;%.16le;%.16le;%.16le\n", (long long int)currentFp.timeStamp, currentFp.x, averageX, normalizedFp.x, currentFp.y, averageY, normalizedFp.y, currentFp.angle, averageAngle, normalizedFp.angle);
		fileOut.write(buffer);
	}
}

bool VideoStabilizer::readNormalizedFramePositions(const QString& fileName)
{
	QFile file(fileName);

	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		qWarning("Could not open input file");
		return false;
	}

	QTextStream fileStream(&file);
	QString fileInString = fileStream.readAll();
	file.close();

	QStringList lines = fileInString.split('\n');

	for (int i = 1; i < lines.size(); ++i)
	{
		QStringList parts = lines.at(i).split(';');

		if (parts.size() == 10)
		{
			FramePosition fp;

			fp.timeStamp = (int64_t)parts[0].toLongLong();
			fp.x = parts[3].toDouble();
			fp.y = parts[6].toDouble();
			fp.angle = parts[9].toDouble();

			normalizedFramePositions.push_back(fp);
		}
	}

	return true;
}

void VideoStabilizer::toggleEnabled()
{
	isEnabled = !isEnabled;
	reset();
}

void VideoStabilizer::reset()
{
	cumulativeX = 0.0;
	cumulativeY = 0.0;
	cumulativeAngle = 0.0;

	cumulativeXAverage.reset();
	cumulativeYAverage.reset();
	cumulativeAngleAverage.reset();

	normalizedFramePosition = FramePosition();
	previousTransformation = cv::Mat::eye(2, 3, CV_64F);

	isFirstImage = true;
	lastProcessTime = 0.0;
}

double VideoStabilizer::getX() const
{
	return normalizedFramePosition.x;
}

double VideoStabilizer::getY() const
{
	return normalizedFramePosition.y;
}

double VideoStabilizer::getAngle() const
{
	return normalizedFramePosition.angle;
}

double VideoStabilizer::getLastProcessTime() const
{
	return lastProcessTime;
}
