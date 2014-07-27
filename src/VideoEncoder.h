// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>

namespace OrientView
{
	class VideoDecoder;
	class Settings;

	class VideoEncoder
	{

	public:

		VideoEncoder();

		bool initialize(const QString& fileName, VideoDecoder* videoDecoder, Settings* settings);
		void shutdown();

	private:

	};
}
