// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QFile>

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
		double getScaleX() const;
		double getScaleY() const;
		double getLastProcessTime() const;

	private:

		bool isFirstImage = true;

		double dxCum = 0.0;
		double dyCum = 0.0;
		double daCum = 0.0;
		double dsxCum = 1.0;
		double dsyCum = 1.0;

		double lastProcessTime = 0.0;

		cv::Mat previousImage;

		std::vector<cv::Point2f> previousCorners;
		std::vector<cv::Point2f> currentCorners;
		std::vector<cv::Point2f> previousCornersFiltered;
		std::vector<cv::Point2f> currentCornersFiltered;
		std::vector<uchar> opticalFlowStatus;
		std::vector<float> opticalFlowError;
		cv::Mat previousTransform;

		QFile dataOutputFile;
		bool outputData = true;
	};
}
