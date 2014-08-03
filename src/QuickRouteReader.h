// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>

namespace OrientView
{
	struct QuickRouteReaderResult
	{
		double topLeftLat = 0.0;
		double topLeftLon = 0.0;
		double topRightLat = 0.0;
		double topRightLon = 0.0;
		double bottomRightLat = 0.0;
		double bottomRightLon = 0.0;
		double bottomLeftLat = 0.0;
		double bottomLeftLon = 0.0;
		double projectionOriginLat = 0.0;
		double projectionOriginLon = 0.0;
	};

	class QuickRouteReader
	{

	public:

		static bool readFromJpeg(const QString& fileName, QuickRouteReaderResult* result, bool includeBorders);
	};
}
