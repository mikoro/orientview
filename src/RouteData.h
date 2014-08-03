// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QDateTime>
#include <QPointF>
#include <QTransform>

namespace OrientView
{
	struct RoutePoint
	{
		QDateTime dateTime;
		QPointF coordinate;
		QPointF position;
		double elevation = 0.0;
		double heartRate = 0.0;
	};

	struct RouteData
	{
		QPointF projectionOriginCoordinate;
		QTransform transformationMatrix;
		std::vector<RoutePoint> routePoints;
	};
}
