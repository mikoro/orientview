// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#define _USE_MATH_DEFINES
#include <cmath>

#include "RouteManager.h"
#include "QuickRouteReader.h"
#include "Renderer.h"
#include "Settings.h"

using namespace OrientView;

bool RouteManager::initialize(QuickRouteReader* quickRouteReader, SplitsManager* splitsManager, Renderer* renderer, Settings* settings)
{
	this->renderer = renderer;

	viewMode = settings->routeManager.viewMode;
	useSmoothSplitTransition = settings->routeManager.useSmoothSplitTransition;
	smoothSplitTransitionSpeed = settings->routeManager.smoothSplitTransitionSpeed;
	topBottomMargin = settings->routeManager.topBottomMargin;
	leftRightMargin = settings->routeManager.leftRightMargin;
	maximumAutomaticZoom = settings->routeManager.maximumAutomaticZoom;
	runnerAveragingFactor = settings->routeManager.runnerAveragingFactor;
	runnerVerticalOffset = settings->routeManager.runnerVerticalOffset;
	windowWidth = settings->window.width;
	windowHeight = settings->window.height;

	runnerAverageX.setAlpha(runnerAveragingFactor / 1000.0);
	runnerAverageY.setAlpha(runnerAveragingFactor / 1000.0);
	runnerAverageAngle.setAlpha(runnerAveragingFactor / 1000.0);

	routes.push_back(Route());
	Route& defaultRoute = routes.at(0);

	defaultRoute.routePoints = quickRouteReader->getRoutePoints();
	defaultRoute.runnerInfo = splitsManager->getDefaultRunnerInfo();
	defaultRoute.discreetColor = settings->route.discreetColor;
	defaultRoute.highlightColor = settings->route.highlightColor;
	defaultRoute.routeRenderMode = settings->route.routeRenderMode;
	defaultRoute.routeWidth = settings->route.routeWidth;
	defaultRoute.tailRenderMode = settings->route.tailRenderMode;
	defaultRoute.tailWidth = settings->route.tailWidth;
	defaultRoute.tailLength = settings->route.tailLength;
	defaultRoute.controlBorderColor = settings->route.controlBorderColor;
	defaultRoute.controlRadius = settings->route.controlRadius;
	defaultRoute.controlBorderWidth = settings->route.controlBorderWidth;
	defaultRoute.showControls = settings->route.showControls;
	defaultRoute.runnerColor = settings->route.runnerColor;
	defaultRoute.runnerBorderColor = settings->route.runnerBorderColor;
	defaultRoute.runnerRadius = settings->route.runnerRadius;
	defaultRoute.runnerBorderWidth = settings->route.runnerBorderWidth;
	defaultRoute.runnerScale = settings->route.runnerScale;
	defaultRoute.showRunner = settings->route.showRunner;
	defaultRoute.controlTimeOffset = settings->route.controlTimeOffset;
	defaultRoute.runnerTimeOffset = settings->route.runnerTimeOffset;
	defaultRoute.userScale = settings->route.scale;
	defaultRoute.lowPace = settings->route.lowPace;
	defaultRoute.highPace = settings->route.highPace;

	for (Route& route : routes)
	{
		calculateAlignedRoutePoints(route);
		calculateRoutePointColors(route);
		calculateRoutePath(route);
	}

	update(0.0, 0.0);

	if (viewMode == ViewMode::FixedSplit && currentSplitTransformationIndex == -1 && defaultRoute.splitTransformations.size() > 0)
	{
		currentSt = defaultRoute.splitTransformations.at(0);
		currentSplitTransformationIndex = 0;
	}

	return true;
}

void RouteManager::update(double currentTime, double frameTime)
{
	if (fullUpdateRequested)
	{
		for (Route& route : routes)
		{
			calculateControlPositions(route);
			calculateSplitTransformations(route);
		}

		fullUpdateRequested = false;
	}

	for (Route& route : routes)
	{
		calculateCurrentRunnerPosition(route, currentTime);
		calculateTailPath(route, currentTime);
	}

	calculateCurrentSplitTransformation(routes.at(0), currentTime, frameTime);
}

void RouteManager::calculateAlignedRoutePoints(Route& route)
{
	if (route.routePoints.size() < 2)
		return;

	double alignedTime = 0.0;
	RoutePoint currentRoutePoint = route.routePoints.at(0);
	RoutePoint alignedRoutePoint;

	// align and interpolate route point data to one second intervals
	for (int i = 0; i < (int)route.routePoints.size() - 1;)
	{
		int nextIndex = 0;

		for (int j = i + 1; j < (int)route.routePoints.size(); ++j)
		{
			if (route.routePoints.at(j).time - currentRoutePoint.time > 1.0)
			{
				nextIndex = j;
				break;
			}
		}

		if (nextIndex <= i)
			break;

		i = nextIndex;

		RoutePoint nextRoutePoint = route.routePoints.at(nextIndex);

		alignedRoutePoint.dateTime = currentRoutePoint.dateTime;
		alignedRoutePoint.coordinate = currentRoutePoint.coordinate;

		double timeDelta = nextRoutePoint.time - currentRoutePoint.time;
		double alphaStep = 1.0 / timeDelta;
		double alpha = 0.0;
		int stepCount = (int)timeDelta;

		for (int k = 0; k <= stepCount; ++k)
		{
			alignedRoutePoint.time = alignedTime;
			alignedRoutePoint.position.setX((1.0 - alpha) * currentRoutePoint.position.x() + alpha * nextRoutePoint.position.x());
			alignedRoutePoint.position.setY((1.0 - alpha) * currentRoutePoint.position.y() + alpha * nextRoutePoint.position.y());
			alignedRoutePoint.elevation = (1.0 - alpha) * currentRoutePoint.elevation + alpha * nextRoutePoint.elevation;
			alignedRoutePoint.heartRate = (1.0 - alpha) * currentRoutePoint.heartRate + alpha * nextRoutePoint.heartRate;
			alignedRoutePoint.pace = (1.0 - alpha) * currentRoutePoint.pace + alpha * nextRoutePoint.pace;
			alignedRoutePoint.orientation = (1.0 - alpha) * currentRoutePoint.orientation + alpha * nextRoutePoint.orientation;

			alpha += alphaStep;

			if (k < stepCount)
			{
				route.alignedRoutePoints.push_back(alignedRoutePoint);
				alignedTime += 1.0;
			}
		}

		currentRoutePoint = alignedRoutePoint;
		currentRoutePoint.dateTime = nextRoutePoint.dateTime;
		currentRoutePoint.coordinate = nextRoutePoint.coordinate;
	}

	route.alignedRoutePoints.push_back(alignedRoutePoint);
}

void RouteManager::calculateRoutePointColors(Route& route)
{
	for (RoutePoint& rp : route.routePoints)
		rp.color = interpolateFromGreenToRed(route.highPace, route.lowPace, rp.pace);

	for (RoutePoint& rp : route.alignedRoutePoints)
		rp.color = interpolateFromGreenToRed(route.highPace, route.lowPace, rp.pace);
}

void RouteManager::calculateRoutePath(Route& route)
{
	if (route.routePoints.size() < 2)
		return;

	route.routePath = QPainterPath();

	for (size_t i = 0; i < route.routePoints.size(); ++i)
	{
		RoutePoint& rp = route.routePoints.at(i);

		if (i == 0)
			route.routePath.moveTo(rp.position.x(), rp.position.y());
		else
			route.routePath.lineTo(rp.position.x(), rp.position.y());
	}
}

void RouteManager::calculateTailPath(Route& route, double currentTime)
{
	double offsetTime = currentTime + route.runnerTimeOffset;
	double startTime = offsetTime - route.tailLength;
	double endTime = offsetTime;

	int startIndex = (int)floor(startTime);
	int endIndex = (int)floor(endTime);
	int indexMax = (int)route.alignedRoutePoints.size() - 1;

	startIndex = std::max(0, std::min(startIndex, indexMax));
	endIndex = std::max(0, std::min(endIndex, indexMax));

	route.tailPath = QPainterPath();

	if (startIndex == endIndex)
		return;

	RoutePoint startRoutePoint = getInterpolatedRoutePoint(route, startTime);
	RoutePoint endRoutePoint = getInterpolatedRoutePoint(route, endTime);

	route.tailPath.moveTo(startRoutePoint.position.x(), startRoutePoint.position.y());

	for (int i = startIndex + 1; i < endIndex; ++i)
	{
		RoutePoint& rp = route.alignedRoutePoints.at(i);
		route.tailPath.lineTo(rp.position.x(), rp.position.y());
	}

	route.tailPath.lineTo(endRoutePoint.position.x(), endRoutePoint.position.y());
}

void RouteManager::calculateControlPositions(Route& route)
{
	route.controlPositions.clear();

	for (const Split& split : route.runnerInfo.splits)
	{
		RoutePoint rp = getInterpolatedRoutePoint(route, split.absoluteTime + route.controlTimeOffset);
		route.controlPositions.push_back(rp.position);
	}
}

void RouteManager::calculateSplitTransformations(Route& route)
{
	if (route.runnerInfo.splits.empty() || route.alignedRoutePoints.empty())
		return;

	route.splitTransformations.clear();

	// take two consecutive controls and then figure out the transformation needed
	// to make the line from start to stop control vertical, centered and zoomed appropriately
	for (int i = 0; i < (int)route.runnerInfo.splits.size() - 1; ++i)
	{
		Split split1 = route.runnerInfo.splits.at(i);
		Split split2 = route.runnerInfo.splits.at(i + 1);

		int startIndex = (int)round(split1.absoluteTime + route.controlTimeOffset);
		int stopIndex = (int)round(split2.absoluteTime + route.controlTimeOffset);
		int indexMax = (int)route.alignedRoutePoints.size() - 1;

		startIndex = std::max(0, std::min(startIndex, indexMax));
		stopIndex = std::max(0, std::min(stopIndex, indexMax));

		SplitTransformation splitTransformation;

		if (startIndex != stopIndex)
		{
			RoutePoint startRoutePoint = route.alignedRoutePoints.at(startIndex);
			RoutePoint stopRoutePoint = route.alignedRoutePoints.at(stopIndex);
			QPointF startToStop = stopRoutePoint.position - startRoutePoint.position; // vector pointing from start to stop

			// rotate towards positive y-axis
			double angle = atan2(-startToStop.y(), startToStop.x()) * (180.0 / M_PI);
			angle = 90.0 - angle;

			// offset so that left quadrants rotate cw and right quadrants ccw
			if (angle > 180.0)
				angle = angle - 360.0;

			QMatrix rotateMatrix;
			rotateMatrix.rotate(-angle);

			double minX = std::numeric_limits<double>::max();
			double maxX = -std::numeric_limits<double>::max();
			double minY = std::numeric_limits<double>::max();
			double maxY = -std::numeric_limits<double>::max();

			// find the bounding box for the split route
			for (int j = startIndex; j <= stopIndex; ++j)
			{
				// points need to be rotated
				QPointF position = rotateMatrix.map(route.alignedRoutePoints.at(j).position);

				minX = std::min(minX, position.x());
				maxX = std::max(maxX, position.x());
				minY = std::min(minY, position.y());
				maxY = std::max(maxY, position.y());
			}

			QPointF startPosition = rotateMatrix.map(startRoutePoint.position); // rotated starting position
			QPointF middlePoint = (startRoutePoint.position + stopRoutePoint.position) / 2.0; // doesn't need to be rotated

			// split width is taken from the maximum deviation from center line to either left or right side
			double splitWidthLeft = abs(minX - startPosition.x()) * 2.0 + 2.0 * leftRightMargin;
			double splitWidthRight = abs(maxX - startPosition.x()) * 2.0 + 2.0 * leftRightMargin;
			double splitWidth = std::max(splitWidthLeft, splitWidthRight);

			// split height is the maximum vertical delta
			double splitHeight = maxY - minY + 2.0 * topBottomMargin;

			double scaleX = (windowWidth * renderer->getMapPanel().relativeWidth) / splitWidth;
			double scaleY = windowHeight / splitHeight;
			double finalScale = std::min(scaleX, scaleY);
			finalScale = std::min(finalScale, maximumAutomaticZoom);

			splitTransformation.x = -middlePoint.x();
			splitTransformation.y = middlePoint.y();
			splitTransformation.angle = angle;
			splitTransformation.scale = finalScale;
		}

		route.splitTransformations.push_back(splitTransformation);
	}

	instantSplitTransitionRequested = true;
}

void RouteManager::calculateCurrentRunnerPosition(Route& route, double currentTime)
{
	RoutePoint rp = getInterpolatedRoutePoint(route, currentTime + route.runnerTimeOffset);
	route.runnerPosition = rp.position;
}

void RouteManager::calculateCurrentSplitTransformation(Route& route, double currentTime, double frameTime)
{
	if (viewMode == ViewMode::FixedSplit)
	{
		int index = -1;

		if (findCurrentSplitTransformationIndex(route, currentTime, index))
		{
			if (instantSplitTransitionRequested)
			{
				currentSt = route.splitTransformations.at(index);
				currentSplitTransformationIndex = index;
				instantSplitTransitionRequested = false;
				smoothSplitTransitionInProgress = false;
			}
			else if (index != currentSplitTransformationIndex)
			{
				if (useSmoothSplitTransition)
				{
					previousSt = currentSt;
					nextSt = route.splitTransformations.at(index);
					smoothSplitTransitionAlpha = 0.0;
					smoothSplitTransitionInProgress = true;

					double angleDelta = nextSt.angle - previousSt.angle;
					double absoluteAngleDelta = abs(angleDelta);
					double finalAngleDelta = angleDelta;

					// always try to rotate as little as possible
					if (absoluteAngleDelta > 180.0)
					{
						finalAngleDelta = 360.0 - absoluteAngleDelta;
						finalAngleDelta *= (angleDelta < 0.0) ? 1.0 : -1.0;
					}

					previousSt.angleDelta = finalAngleDelta;
				}
				else
					currentSt = route.splitTransformations.at(index);

				currentSplitTransformationIndex = index;
			}
		}

		if (smoothSplitTransitionInProgress)
		{
			if (smoothSplitTransitionAlpha > 1.0)
			{
				currentSt = nextSt;
				smoothSplitTransitionInProgress = false;
			}
			else
			{
				double alpha = smoothSplitTransitionAlpha;
				alpha = alpha * alpha * alpha * (alpha * (alpha * 6 - 15) + 10); // smootherstep

				currentSt.x = (1.0 - alpha) * previousSt.x + alpha * nextSt.x;
				currentSt.y = (1.0 - alpha) * previousSt.y + alpha * nextSt.y;
				currentSt.angle = previousSt.angle + alpha * previousSt.angleDelta;
				currentSt.scale = (1.0 - alpha) * previousSt.scale + alpha * nextSt.scale;

				smoothSplitTransitionAlpha += smoothSplitTransitionSpeed * frameTime / 1000.0;
			}
		}
	}
	else if (viewMode == ViewMode::RunnerCentered || viewMode == ViewMode::RunnerCenteredSplitOriented)
	{
		RoutePoint rp = getInterpolatedRoutePoint(route, currentTime + route.runnerTimeOffset);

		double x = -rp.position.x();
		double y = rp.position.y();
		double angle = rp.orientation;

		if (viewMode == ViewMode::RunnerCenteredSplitOriented)
		{
			int index = -1;

			if (findCurrentSplitTransformationIndex(route, currentTime, index))
				angle = route.splitTransformations.at(index).angle;
		}

		x -= sin(angle * M_PI / 180.0) * -runnerVerticalOffset;
		y -= cos(angle * M_PI / 180.0) * -runnerVerticalOffset;
		
		if (instantSplitTransitionRequested)
		{
			runnerAverageX.reset(x);
			runnerAverageY.reset(y);
			runnerAverageAngle.reset(angle);

			instantSplitTransitionRequested = false;
		}
		else
		{
			runnerAverageX.addMeasurement(x, frameTime);
			runnerAverageY.addMeasurement(y, frameTime);
			runnerAverageAngle.addMeasurement(angle, frameTime);
		}

		currentSt.x = runnerAverageX.getAverage();
		currentSt.y = runnerAverageY.getAverage();
		currentSt.angle = runnerAverageAngle.getAverage();
	}
}

bool RouteManager::findCurrentSplitTransformationIndex(Route& route, double currentTime, int& index)
{
	for (int i = 0; i < (int)route.runnerInfo.splits.size() - 1; ++i)
	{
		double firstSplitOffsetTime = route.runnerInfo.splits.at(i).absoluteTime + route.controlTimeOffset;
		double secondSplitOffsetTime = route.runnerInfo.splits.at(i + 1).absoluteTime + route.controlTimeOffset;
		double runnerOffsetTime = currentTime + route.runnerTimeOffset;

		// check if we are inside the time range of two consecutive controls
		if (runnerOffsetTime >= firstSplitOffsetTime && runnerOffsetTime < secondSplitOffsetTime)
		{
			if (i >= (int)route.splitTransformations.size())
				return false;

			index = i;
			return true;
		}
	}

	return false;
}

RoutePoint RouteManager::getInterpolatedRoutePoint(Route& route, double time)
{
	if (route.alignedRoutePoints.empty())
		return RoutePoint();

	double previousWholeSecond = floor(time);
	double alpha = time - previousWholeSecond; // the fractional part, i.e milliseconds

	int firstIndex = (int)previousWholeSecond;
	int secondIndex = firstIndex + 1;
	int indexMax = (int)route.alignedRoutePoints.size() - 1;

	// limit the indexes inside a valid range
	firstIndex = std::max(0, std::min(firstIndex, indexMax));
	secondIndex = std::max(0, std::min(secondIndex, indexMax));

	if (firstIndex == secondIndex)
		return route.alignedRoutePoints.at(firstIndex);
	else
	{
		RoutePoint firstRp = route.alignedRoutePoints.at(firstIndex);
		RoutePoint secondRp = route.alignedRoutePoints.at(secondIndex);
		RoutePoint interpolatedRp = firstRp;

		interpolatedRp.time = time;
		interpolatedRp.position = (1.0 - alpha) * firstRp.position + alpha * secondRp.position;
		interpolatedRp.elevation = (1.0 - alpha) * firstRp.elevation + alpha * secondRp.elevation;
		interpolatedRp.heartRate = (1.0 - alpha) * firstRp.heartRate + alpha * secondRp.heartRate;
		interpolatedRp.pace = (1.0 - alpha) * firstRp.pace + alpha * secondRp.pace;
		interpolatedRp.orientation = (1.0 - alpha) * firstRp.orientation + alpha * secondRp.orientation;
		interpolatedRp.color = interpolateFromGreenToRed(route.highPace, route.lowPace, interpolatedRp.pace);

		return interpolatedRp;
	}
}

QColor RouteManager::interpolateFromGreenToRed(double greenValue, double redValue, double value)
{
	double alpha = (value - greenValue) / (redValue - greenValue);
	alpha = std::max(0.0, std::min(alpha, 1.0));

	double r = (alpha > 0.5 ? 1.0 : 2.0 * alpha);
	double g = (alpha > 0.5 ? 1.0 - 2.0 * (alpha - 0.5) : 1.0);
	double b = 0.0;

	return QColor::fromRgbF(r, g, b);
}

void RouteManager::requestFullUpdate()
{
	fullUpdateRequested = true;
}

void RouteManager::requestInstantTransition()
{
	instantSplitTransitionRequested = true;
}

void RouteManager::windowResized(double newWidth, double newHeight)
{
	windowWidth = newWidth;
	windowHeight = newHeight;

	fullUpdateRequested = true;
}

double RouteManager::getX() const
{
	return currentSt.x;
}

double RouteManager::getY() const
{
	return currentSt.y;
}

double RouteManager::getScale() const
{
	return currentSt.scale;
}

double RouteManager::getAngle() const
{
	return currentSt.angle;
}

ViewMode RouteManager::getViewMode() const
{
	return viewMode;
}

void RouteManager::setViewMode(ViewMode value)
{
	viewMode = value;
	instantSplitTransitionRequested = true;
}

Route& RouteManager::getDefaultRoute()
{
	return routes.at(0);
}
