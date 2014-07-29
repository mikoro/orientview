// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include "opencv2/opencv.hpp"

namespace OrientView
{
	class Settings;
	struct FrameData;

	class VideoStabilizer
	{

	public:

		VideoStabilizer();

		bool initialize(Settings* settings);
		void shutdown();

		void processFrame(FrameData* frameDataGrayscale);

		double getX() const;
		double getY() const;
		double getAngle() const;
		double getAverageProcessTime() const;

	private:

		bool isFirstImage = true;

		double deltaX = 0.0;
		double deltaY = 0.0;
		double deltaAngle = 0.0;

		double accumulatedX = 0.0;
		double accumulatedY = 0.0;
		double accumulatedAngle = 0.0;

		double averageProcessTime = 0.0;

		cv::Mat previousImage;

		std::vector<cv::Point2f> previousCorners;
		std::vector<cv::Point2f> currentCorners;
		std::vector<cv::Point2f> previousCornersFiltered;
		std::vector<cv::Point2f> currentCornersFiltered;
		std::vector<uchar> opticalFlowStatus;
		std::vector<float> opticalFlowError;
	};
}
