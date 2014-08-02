// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>
#include <QDateTime>

#include "TrackPoint.h"

namespace OrientView
{
	class GpxReader
	{

	public:

		bool initialize(const QString& fileName);
		const std::vector<TrackPoint>& getTrackPoints();

	private:

		std::vector<TrackPoint> trackPoints;
	};
}
