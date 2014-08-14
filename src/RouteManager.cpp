// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "RouteManager.h"
#include "QuickRouteReader.h"
#include "SplitTimeManager.h"

using namespace OrientView;

void RouteManager::initialize(QuickRouteReader* quickRouteReader, SplitTimeManager* splitTimeManager)
{
	defaultRoute.routePoints = quickRouteReader->getRoutePoints();
	defaultRoute.splitTimes = splitTimeManager->getSplitTimes();

	constructWholeRoutePath(defaultRoute);
}

const Route& RouteManager::getDefaultRoute() const
{
	return defaultRoute;
}

void RouteManager::calculateSplitRoutePoints(Route& route)
{

}

void RouteManager::calculateControlLocations(Route& route)
{

}

void RouteManager::calculateRunnerLocation(Route& route)
{

}

void RouteManager::constructWholeRoutePath(Route& route)
{
	size_t routePointCount = route.routePoints.size();
	route.wholeRoutePath = QPainterPath();

	if (routePointCount >= 2)
	{
		for (size_t i = 0; i < routePointCount; ++i)
		{
			RoutePoint rp = route.routePoints.at(i);

			double x = rp.position.x();
			double y = rp.position.y();

			if (i == 0)
				route.wholeRoutePath.moveTo(x, y);
			else
				route.wholeRoutePath.lineTo(x, y);
		}
	}
}

void RouteManager::constructSplitRoutePath(Route& route)
{

}
