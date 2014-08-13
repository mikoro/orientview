// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <vector>

#include <QPainterPath>
#include <QColor>

#include "RoutePoint.h"

namespace OrientView
{
	class QuickRouteReader;
	class SplitTimeManager;

	struct Route
	{
		std::vector<RoutePoint> routePoints;
		std::vector<RoutePoint> splitRoutePoints;
		std::vector<double> splitTimes;
		double startOffset = 0.0;

		std::vector<QPointF> controlLocations;
		QColor controlColor = QColor(0, 0, 255);
		double controlRadius = 10.0;

		QPointF runnerLocation;
		QColor runnerColor = QColor(0, 0, 255);
		double runnerRadius = 10.0;

		QPainterPath wholeRoutePath;
		QColor wholeRouteColor = QColor(255, 0, 0);
		double wholeRouteWidth = 15.0;

		QPainterPath splitRoutePath;
		QColor splitRouteColor = QColor(255, 0, 0);
		double splitRouteWidth = 15.0;
	};

	class RouteManager
	{

	public:

		void initialize(QuickRouteReader* quickRouteReader, SplitTimeManager* splitTimeManager);
		void update(double currentTime);

		const Route& getDefaultRoute() const;

	private:

		void calculateSplitRoutePoints(Route& route);
		void calculateControlLocations(Route& route);
		void calculateRunnerLocation(Route& route);
		void constructWholeRoutePath(Route& route);
		void constructSplitRoutePath(Route& route);

		Route defaultRoute;
		double currentTime = 0.0;
	};
}
