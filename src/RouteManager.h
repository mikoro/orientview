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
	class Renderer;
	class Settings;

	enum RouteRenderMode { Normal, Pace, None };

	struct SplitTransformation
	{
		double x = 0.0;
		double y = 0.0;
		double angle = 0.0;
		double scale = 0.0;
	};

	struct Route
	{
		std::vector<RoutePoint> routePoints;
		std::vector<RoutePoint> alignedRoutePoints;
		std::vector<SplitTransformation> splitTransformations;
		SplitTimes splitTimes;

		double controlsTimeOffset = 0.0;
		double runnerTimeOffset = 0.0;
		double userScale = 1.0;
		double highPace = 5.0;
		double lowPace = 15.0;
		double topBottomMargin = 30.0;
		double leftRightMargin = 10.0;

		SplitTransformation currentSplitTransformation;

		RouteRenderMode wholeRouteRenderMode = RouteRenderMode::Normal;
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

		void initialize(QuickRouteReader* quickRouteReader, SplitTimeManager* splitTimeManager, Renderer* renderer, Settings* settings);

		void update(double currentTime);
		void requestFullUpdate();

		void windowResized(double newWidth, double newHeight);

		double getX() const;
		double getY() const;
		double getScale() const;
		double getAngle() const;

		Route& getDefaultRoute();

	private:

		void generateAlignedRoutePoints();
		void constructWholeRoutePath();
		void calculateControlPositions();
		void calculateSplitTransformations();
		void calculateRoutePointColors();
		void calculateRunnerPosition(double currentTime);
		void calculateCurrentSplitTransformation(double currentTime);
		QColor interpolateFromGreenToRed(double greenValue, double redValue, double value);

		Renderer* renderer = nullptr;

		Route defaultRoute;

		bool fullUpdateRequested = false;

		double windowWidth = 0.0;
		double windowHeight = 0.0;
	};
}
