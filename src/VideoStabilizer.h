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

	private:

		bool isFirstImage = true;
		cv::Mat previousImage;
	};
}
