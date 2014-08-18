// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "RouteManager.h"
#include "QuickRouteReader.h"
#include "SplitTimeManager.h"
#include "Settings.h"

using namespace OrientView;

void RouteManager::initialize(QuickRouteReader* quickRouteReader, SplitTimeManager* splitTimeManager, Settings* settings)
{
	defaultRoute.routePoints = quickRouteReader->getRoutePoints();
	defaultRoute.alignedRoutePoints = quickRouteReader->getAlignedRoutePoints();
	defaultRoute.splitTimes = splitTimeManager->getDefaultSplitTimes();
	defaultRoute.startOffset = settings->route.startOffset;
	defaultRoute.scale = settings->route.scale;
	defaultRoute.wholeRouteRenderMode = settings->route.wholeRouteRenderMode;
	defaultRoute.showRunner = settings->route.showRunner;
	defaultRoute.showControls = settings->route.showControls;
	defaultRoute.wholeRouteColor = settings->route.wholeRouteColor;
	defaultRoute.wholeRouteWidth = settings->route.wholeRouteWidth;
	defaultRoute.controlBorderColor = settings->route.controlBorderColor;
	defaultRoute.controlRadius = settings->route.controlRadius;
	defaultRoute.controlBorderWidth = settings->route.controlBorderWidth;
	defaultRoute.runnerColor = settings->route.runnerColor;
	defaultRoute.runnerBorderColor = settings->route.runnerBorderColor;
	defaultRoute.runnerBorderWidth = settings->route.runnerBorderWidth;
	defaultRoute.runnerScale = settings->route.runnerScale;

	constructWholeRoutePath();

	fullUpdateRequested = true;
	update(0);
}

void RouteManager::update(double currentTime)
{
	calculateRunnerPosition(currentTime);

	if (fullUpdateRequested)
	{
		calculateControlPositions();
		calculateRoutePointColors();
		fullUpdateRequested = false;
	}
}

void RouteManager::requestFullUpdate()
{
	fullUpdateRequested = true;
}

Route& RouteManager::getDefaultRoute()
{
	return defaultRoute;
}

void RouteManager::constructWholeRoutePath()
{
	size_t routePointCount = defaultRoute.routePoints.size();

	if (routePointCount >= 2)
	{
		for (size_t i = 0; i < routePointCount; ++i)
		{
			RoutePoint rp = defaultRoute.routePoints.at(i);

			double x = rp.position.x();
			double y = rp.position.y();

			if (i == 0)
				defaultRoute.wholeRoutePath.moveTo(x, y);
			else
				defaultRoute.wholeRoutePath.lineTo(x, y);
		}
	}
}

void RouteManager::calculateControlPositions()
{
	defaultRoute.controlPositions.clear();

	for (size_t i = 0; i < defaultRoute.splitTimes.splitTimes.size(); ++i)
	{
		SplitTime splitTime = defaultRoute.splitTimes.splitTimes.at(i);

		double offsetTime = splitTime.time + defaultRoute.startOffset;
		double previousWholeSecond = floor(offsetTime);
		double alpha = offsetTime - previousWholeSecond;

		int firstIndex = (int)previousWholeSecond;
		int secondIndex = firstIndex + 1;
		int indexMax = (int)defaultRoute.alignedRoutePoints.size() - 1;

		firstIndex = std::max(0, std::min(firstIndex, indexMax));
		secondIndex = std::max(0, std::min(secondIndex, indexMax));

		if (firstIndex == secondIndex)
			defaultRoute.controlPositions.push_back(defaultRoute.alignedRoutePoints.at(firstIndex).position);
		else
		{
			RoutePoint rp1 = defaultRoute.alignedRoutePoints.at(firstIndex);
			RoutePoint rp2 = defaultRoute.alignedRoutePoints.at(secondIndex);

			defaultRoute.controlPositions.push_back((1.0 - alpha) * rp1.position + alpha * rp2.position);
		}
	}
}

void RouteManager::calculateRoutePointColors()
{
	for (RoutePoint& rp : defaultRoute.routePoints)
		rp.color = interpolateFromGreenToRed(5.0, 15.0, rp.pace);

	for (RoutePoint& rp : defaultRoute.alignedRoutePoints)
		rp.color = interpolateFromGreenToRed(5.0, 15.0, rp.pace);
}

void RouteManager::calculateRunnerPosition(double currentTime)
{
	double offsetTime = currentTime + defaultRoute.startOffset;
	double previousWholeSecond = floor(offsetTime);
	double alpha = offsetTime - previousWholeSecond;

	int firstIndex = (int)previousWholeSecond;
	int secondIndex = firstIndex + 1;
	int indexMax = (int)defaultRoute.alignedRoutePoints.size() - 1;

	firstIndex = std::max(0, std::min(firstIndex, indexMax));
	secondIndex = std::max(0, std::min(secondIndex, indexMax));

	if (firstIndex == secondIndex)
		defaultRoute.runnerPosition = defaultRoute.alignedRoutePoints.at(firstIndex).position;
	else
	{
		RoutePoint rp1 = defaultRoute.alignedRoutePoints.at(firstIndex);
		RoutePoint rp2 = defaultRoute.alignedRoutePoints.at(secondIndex);

		defaultRoute.runnerPosition = (1.0 - alpha) * rp1.position + alpha * rp2.position;
	}
}

QColor RouteManager::interpolateFromGreenToRed(double lowValue, double highValue, double value)
{
	double alpha = (value - lowValue) / (highValue - lowValue);
	alpha = std::max(0.0, std::min(alpha, 1.0));

	double r = (alpha > 0.5 ? 1.0 : 2.0 * alpha);
	double g = (alpha > 0.5 ? 1.0 - 2.0 * (alpha - 0.5) : 1.0);
	double b = 0.0;

	return QColor::fromRgbF(r, g, b);
}
