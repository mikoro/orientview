// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QDateTime>
#include <QPointF>
#include <QColor>

namespace OrientView
{
	struct RoutePoint
	{
		QDateTime dateTime;			// Actual date and time of the point.
		double time = 0.0;			// Time in seconds since the start.
		QPointF coordinate;			// Coordinate of the point in world coordinates.
		QPointF position;			// Position of the point in map pixel units.
		double elevation = 0.0;		// Elevation in meters.
		double heartRate = 0.0;		// Heart rate bpm.
		double pace = 0.0;			// Pace in min / km.
		QColor color;				// Color representing the pace.
	};
}
