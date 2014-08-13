// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QFile>
#include <QElapsedTimer>

#include "opencv2/opencv.hpp"

#include "MovingAverage.h"

namespace OrientView
{
	class Settings;
	struct FrameData;

	// Use the OpenCV library to do real-time video stabilization.
	class VideoStabilizer
	{

	public:

		void initialize(Settings* settings);
		~VideoStabilizer();

		void processFrame(FrameData* frameDataGrayscale);

		void toggleEnabled();
		void reset();

		double getX() const;
		double getY() const;
		double getAngle() const;
		double getScale() const;
		double getLastProcessTime() const;

	private:

		bool isFirstImage = true;
		bool isEnabled = true;

		double currentX = 0.0;
		double currentY = 0.0;
		double currentAngle = 0.0;
		double normalizedX = 0.0;
		double normalizedY = 0.0;
		double normalizedAngle = 0.0;

		double dampingFactor = 0.0;
		double maxDisplacementFactor = 0.0;

		MovingAverage currentXAverage;
		MovingAverage currentYAverage;
		MovingAverage currentAngleAverage;

		cv::Mat previousImage;
		cv::Mat previousTransformation;
		std::vector<cv::Point2f> previousCorners;
		std::vector<cv::Point2f> currentCorners;
		std::vector<cv::Point2f> previousCornersFiltered;
		std::vector<cv::Point2f> currentCornersFiltered;
		std::vector<uchar> opticalFlowStatus;
		std::vector<float> opticalFlowError;
		
		QElapsedTimer processTimer;
		double lastProcessTime = 0.0;

		QFile dataOutputFile;
		bool outputData = false;
	};
}
