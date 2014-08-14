// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <vector>

#include <QPainterPath>
#include <QColor>

#include "RoutePoint.h"
#include "SplitTimeManager.h"

namespace OrientView
{
	class QuickRouteReader;
	class Settings;

	struct Route
	{
		std::vector<RoutePoint> routePoints;
		std::vector<RoutePoint> alignedRoutePoints;
		SplitTimes splitTimes;
		double startOffset = 0.0;

		std::vector<QPointF> controlPositions;
		QColor controlColor = QColor(255, 0, 0);
		double controlRadius = 10.0;

		QPointF runnerPosition;
		QColor runnerColor = QColor(0, 0, 255);
		double runnerRadius = 10.0;

		QPainterPath wholeRoutePath;
		QColor wholeRouteColor = QColor(255, 0, 0, 128);
		double wholeRouteWidth = 15.0;
	};

	class RouteManager
	{

	public:

		void initialize(QuickRouteReader* quickRouteReader, SplitTimeManager* splitTimeManager, Settings* settings);
		void update(double currentTime);
		void requestFullUpdate();

		Route& getDefaultRoute();

	private:

		void constructWholeRoutePath();
		void calculateControlLocations();

		Route defaultRoute;

		bool fullUpdateRequested = false;
	};
}
