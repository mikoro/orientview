// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>

namespace OrientView
{
	struct FrameData;

	class VideoStabilizer
	{

	public:

		VideoStabilizer();

		bool initialize();
		void shutdown();

		void processFrame(FrameData* frameDataGrayscale);

	private:

	};
}
