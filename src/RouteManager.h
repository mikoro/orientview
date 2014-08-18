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

	enum class RouteRenderMode { Normal, Pace, None };

	struct Route
	{
		std::vector<RoutePoint> routePoints;
		std::vector<RoutePoint> alignedRoutePoints;
		SplitTimes splitTimes;

		double startOffset = 0.0;
		double userScale = 1.0;

		RouteRenderMode renderMode = RouteRenderMode::Normal;
		bool showRunner = true;
		bool showControls = true;

		QPainterPath wholeRoutePath;
		QColor wholeRouteColor = QColor(0, 0, 0, 50);
		double wholeRouteWidth = 10.0;

		std::vector<QPointF> controlPositions;
		QColor controlBorderColor = QColor(140, 40, 140, 255);
		double controlRadius = 15.0;
		double controlBorderWidth = 5.0;

		QPointF runnerPosition;
		QColor runnerColor = QColor(0, 100, 255, 220);
		QColor runnerBorderColor = QColor(0, 0, 0, 255);
		double runnerBorderWidth = 1.0;
		double runnerScale = 1.0;
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
