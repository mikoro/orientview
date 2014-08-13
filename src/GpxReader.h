// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>
#include <QDateTime>

namespace OrientView
{
	struct TrackPoint
	{
		QDateTime dateTime;

		double latitude = 0.0;
		double longitude = 0.0;
		double elevation = 0.0;
		double heartRate = 0.0;
	};

	// Read track point data from GPX files.
	class GpxReader
	{

	public:

		bool initialize(const QString& fileName);
		const std::vector<TrackPoint>& getTrackPoints() const;

	private:

		std::vector<TrackPoint> trackPoints;
	};
}
