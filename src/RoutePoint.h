// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QDateTime>
#include <QPointF>

namespace OrientView
{
	struct RoutePoint
	{
		QDateTime dateTime;
		QPointF coordinate;
		QPointF projectedPosition;
		QPointF transformedPosition;
		double elevation = 0.0;
		double heartRate = 0.0;
		double distanceToPrevious = 0.0;
		double timeToPrevious = 0.0;
		double timeSinceStart = 0.0;
		double pace = 0.0;
	};
}
