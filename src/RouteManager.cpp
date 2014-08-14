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

	constructWholeRoutePath();
	calculateControlLocations();
	update(0);
}

void RouteManager::update(double currentTime)
{
	int timeBasedIndex = (int)round(currentTime + defaultRoute.startOffset);
	int indexMax = (int)defaultRoute.alignedRoutePoints.size() - 1;

	timeBasedIndex = std::max(0, std::min(timeBasedIndex, indexMax));
	defaultRoute.runnerPosition = defaultRoute.alignedRoutePoints.at(timeBasedIndex).position;

	if (fullUpdateRequested)
	{
		calculateControlLocations();
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

void RouteManager::calculateControlLocations()
{
	defaultRoute.controlPositions.clear();

	for (int i = 0; i < defaultRoute.splitTimes.splitTimes.size(); ++i)
	{
		SplitTime splitTime = defaultRoute.splitTimes.splitTimes.at(i);

		int timeBasedIndex = (int)round(splitTime.time + defaultRoute.startOffset);
		int indexMax = (int)defaultRoute.alignedRoutePoints.size() - 1;

		timeBasedIndex = std::max(0, std::min(timeBasedIndex, indexMax));
		defaultRoute.controlPositions.push_back(defaultRoute.alignedRoutePoints.at(timeBasedIndex).position);
	}
}
