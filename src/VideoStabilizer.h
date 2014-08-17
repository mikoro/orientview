// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <cstdint>

#include <QFile>
#include <QElapsedTimer>

#include "opencv2/opencv.hpp"

namespace OrientView
{
	class Settings;
	struct FrameData;

	struct FramePosition
	{
		int64_t timeStamp = 0;
		double x = 0.0;
		double y = 0.0;
		double angle = 0.0;
	};

	enum VideoStabilizerMode { RealTime, Preprocessed };

	// Use the OpenCV library to do real-time video stabilization.
	class VideoStabilizer
	{

	public:

		bool initialize(Settings* settings, bool isPreprocessing);

		void preProcessFrame(const FrameData& frameDataGrayscale, QFile& file);
		void processFrame(const FrameData& frameDataGrayscale);

		static void convertCumulativeFramePositionsToNormalized(QFile& fileIn, QFile& fileOut, int smoothingRadius);
		bool readNormalizedFramePositions(const QString& fileName);

		void toggleEnabled();
		void reset();

		double getX() const;
		double getY() const;
		double getAngle() const;

		double getLastProcessTime() const;

	private:

		FramePosition calculateCumulativeFramePosition(const FrameData& frameDataGrayscale);
		FramePosition searchNormalizedFramePosition(const FrameData& frameDataGrayscale);

		VideoStabilizerMode mode = VideoStabilizerMode::Preprocessed;

		bool isFirstImage = true;
		bool isEnabled = true;

		double dampingFactor = 0.0;
		double maxDisplacementFactor = 0.0;

		double cumulativeX = 0.0;
		double cumulativeY = 0.0;
		double cumulativeAngle = 0.0;

		std::vector<FramePosition> normalizedFramePositions;

		FramePosition normalizedFramePosition;
		
		cv::Mat previousImage;
		cv::Mat previousTransformation;
		
		QElapsedTimer processTimer;
		double lastProcessTime = 0.0;
	};
}
