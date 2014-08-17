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
		QColor controlBorderColor = QColor(140, 40, 140);
		double controlRadius = 15.0;
		double controlBorderWidth = 5.0;

		QPointF runnerPosition;
		QColor runnerColor = QColor(60, 100, 255);
		QColor runnerBorderColor = QColor(0, 0, 0);
		double runnerRadius = 6.0;
		double runnerBorderWidth = 2.0;

		QPainterPath wholeRoutePath;
		QPainterPath wholeRoutePathStroked;
		QColor wholeRouteColor = QColor(0, 0, 0, 50);
		QColor wholeRouteBorderColor = QColor(0, 0, 0, 100);
		double wholeRouteWidth = 10.0;
		double wholeRouteBorderWidth = 1.0;

		bool shouldRenderPace = false;
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
		void calculateRoutePointColors();

		QColor interpolateFromGreenToRed(double lowValue, double highValue, double value);

		Route defaultRoute;

		bool fullUpdateRequested = false;
	};
}
