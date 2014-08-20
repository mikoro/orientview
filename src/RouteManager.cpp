// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#define _USE_MATH_DEFINES
#include <cmath>

#include "RouteManager.h"
#include "QuickRouteReader.h"
#include "Renderer.h"
#include "Settings.h"

using namespace OrientView;

void RouteManager::initialize(QuickRouteReader* quickRouteReader, SplitTimeManager* splitTimeManager, Renderer* renderer, Settings* settings)
{
	this->renderer = renderer;

	defaultRoute.routePoints = quickRouteReader->getRoutePoints();
	defaultRoute.splitTimes = splitTimeManager->getDefaultSplitTimes();
	defaultRoute.controlTimeOffset = settings->route.controlTimeOffset;
	defaultRoute.runnerTimeOffset = settings->route.runnerTimeOffset;
	defaultRoute.userScale = settings->route.scale;
	defaultRoute.minimumScale = settings->route.minimumScale;
	defaultRoute.maximumScale = settings->route.maximumScale;
	defaultRoute.highPace = settings->route.highPace;
	defaultRoute.lowPace = settings->route.lowPace;
	defaultRoute.topBottomMargin = settings->route.topBottomMargin;
	defaultRoute.leftRightMargin = settings->route.leftRightMargin;
	defaultRoute.smoothTransitionSpeed = settings->route.smoothTransitionSpeed;
	defaultRoute.useSmoothTransition = settings->route.useSmoothTransition;
	defaultRoute.showRunner = settings->route.showRunner;
	defaultRoute.showControls = settings->route.showControls;
	defaultRoute.wholeRouteRenderMode = settings->route.wholeRouteRenderMode;
	defaultRoute.wholeRouteColor = settings->route.wholeRouteColor;
	defaultRoute.wholeRouteWidth = settings->route.wholeRouteWidth;
	defaultRoute.controlBorderColor = settings->route.controlBorderColor;
	defaultRoute.controlRadius = settings->route.controlRadius;
	defaultRoute.controlBorderWidth = settings->route.controlBorderWidth;
	defaultRoute.runnerColor = settings->route.runnerColor;
	defaultRoute.runnerBorderColor = settings->route.runnerBorderColor;
	defaultRoute.runnerBorderWidth = settings->route.runnerBorderWidth;
	defaultRoute.runnerScale = settings->route.runnerScale;

	windowWidth = settings->window.width;
	windowHeight = settings->window.height;

	generateAlignedRoutePoints();
	constructWholeRoutePath();
	calculateRoutePointColors();

	fullUpdateRequested = true;

	update(0.0, 0.0);
}

void RouteManager::update(double currentTime, double frameTime)
{
	if (fullUpdateRequested)
	{
		calculateControlPositions();
		calculateSplitTransformations();

		fullUpdateRequested = false;
	}

	calculateRunnerPosition(currentTime);
	calculateCurrentSplitTransformation(currentTime, frameTime);
}

void RouteManager::requestFullUpdate()
{
	fullUpdateRequested = true;
}

void RouteManager::windowResized(double newWidth, double newHeight)
{
	windowWidth = newWidth;
	windowHeight = newHeight;

	fullUpdateRequested = true;
}

double RouteManager::getX() const
{
	return defaultRoute.currentSplitTransformation.x;
}

double RouteManager::getY() const
{
	return defaultRoute.currentSplitTransformation.y;
}

double RouteManager::getScale() const
{
	return defaultRoute.currentSplitTransformation.scale;
}

double RouteManager::getAngle() const
{
	return defaultRoute.currentSplitTransformation.angle;
}

Route& RouteManager::getDefaultRoute()
{
	return defaultRoute;
}

void RouteManager::generateAlignedRoutePoints()
{
	if (defaultRoute.routePoints.size() < 2)
		return;

	double alignedTime = 0.0;
	RoutePoint currentRoutePoint = defaultRoute.routePoints.at(0);
	RoutePoint alignedRoutePoint;

	// align and interpolate route point data to one second intervals
	for (int i = 0; i < (int)defaultRoute.routePoints.size() - 1;)
	{
		int nextIndex = 0;

		for (int j = i + 1; j < (int)defaultRoute.routePoints.size(); ++j)
		{
			if (defaultRoute.routePoints.at(j).time - currentRoutePoint.time > 1.0)
			{
				nextIndex = j;
				break;
			}
		}

		if (nextIndex <= i)
			break;

		i = nextIndex;

		RoutePoint nextRoutePoint = defaultRoute.routePoints.at(nextIndex);

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

			alpha += alphaStep;

			if (k < stepCount)
			{
				defaultRoute.alignedRoutePoints.push_back(alignedRoutePoint);
				alignedTime += 1.0;
			}
		}

		currentRoutePoint = alignedRoutePoint;
		currentRoutePoint.dateTime = nextRoutePoint.dateTime;
		currentRoutePoint.coordinate = nextRoutePoint.coordinate;
	}

	defaultRoute.alignedRoutePoints.push_back(alignedRoutePoint);
}

void RouteManager::constructWholeRoutePath()
{
	if (defaultRoute.routePoints.size() < 2)
		return;

	for (size_t i = 0; i < defaultRoute.routePoints.size(); ++i)
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

void RouteManager::calculateControlPositions()
{
	if (defaultRoute.splitTimes.splitTimes.empty() || defaultRoute.alignedRoutePoints.empty())
		return;

	defaultRoute.controlPositions.clear();

	for (const SplitTime& splitTime : defaultRoute.splitTimes.splitTimes)
	{
		double offsetTime = splitTime.time + defaultRoute.controlTimeOffset;
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

void RouteManager::calculateSplitTransformations()
{
	if (defaultRoute.splitTimes.splitTimes.empty() || defaultRoute.alignedRoutePoints.empty())
		return;

	defaultRoute.splitTransformations.clear();

	// take two consecutive controls and then figure out the transformation needed
	// to make the line from start to stop control vertical, centered and zoomed appropriately
	for (int i = 0; i < (int)defaultRoute.splitTimes.splitTimes.size() - 1; ++i)
	{
		SplitTime st1 = defaultRoute.splitTimes.splitTimes.at(i);
		SplitTime st2 = defaultRoute.splitTimes.splitTimes.at(i + 1);

		int startIndex = (int)round(st1.time + defaultRoute.controlTimeOffset);
		int stopIndex = (int)round(st2.time + defaultRoute.controlTimeOffset);
		int indexMax = (int)defaultRoute.alignedRoutePoints.size() - 1;

		startIndex = std::max(0, std::min(startIndex, indexMax));
		stopIndex = std::max(0, std::min(stopIndex, indexMax));

		SplitTransformation splitTransformation;

		if (startIndex != stopIndex)
		{
			RoutePoint startRp = defaultRoute.alignedRoutePoints.at(startIndex);
			RoutePoint stopRp = defaultRoute.alignedRoutePoints.at(stopIndex);
			QPointF startToStop = stopRp.position - startRp.position; // vector pointing from start to stop

			double angle = atan2(-startToStop.y(), startToStop.x());
			double finalAngle = 0.0;

			// always rotate towards positive y-axis
			// start control should be below the stop control
			if (angle >= 0.0 && angle < (M_PI / 2.0))
				finalAngle = (M_PI / 2.0) - angle;
			else if (angle >= (M_PI / 2.0))
				finalAngle = -(angle - (M_PI / 2.0));
			else if (angle < 0.0 && angle >= -(M_PI / 2.0))
				finalAngle = (M_PI / 2.0) + (-angle);
			else if (angle < -(M_PI / 2.0))
				finalAngle = -((M_PI + angle) + (M_PI / 2.0));

			finalAngle *= 180.0 / M_PI;

			QMatrix rotateMatrix;
			rotateMatrix.rotate(-finalAngle);

			double minX = std::numeric_limits<double>::max();
			double maxX = -std::numeric_limits<double>::max();
			double minY = std::numeric_limits<double>::max();
			double maxY = -std::numeric_limits<double>::max();

			// find the bounding box for the split route
			for (int j = startIndex; j <= stopIndex; ++j)
			{
				// points need to be rotated
				QPointF position = rotateMatrix.map(defaultRoute.alignedRoutePoints.at(j).position);

				minX = std::min(minX, position.x());
				maxX = std::max(maxX, position.x());
				minY = std::min(minY, position.y());
				maxY = std::max(maxY, position.y());
			}

			QPointF startPosition = rotateMatrix.map(startRp.position); // rotated starting position
			QPointF middlePoint = (startRp.position + stopRp.position) / 2.0; // doesn't need to be rotated

			// split width is taken from the maximum deviation from center line to either left or right side
			double splitWidthLeft = abs(minX - startPosition.x()) * 2.0 + 2 * defaultRoute.leftRightMargin;
			double splitWidthRight = abs(maxX - startPosition.x()) * 2.0 + 2 * defaultRoute.leftRightMargin;
			double splitWidth = std::max(splitWidthLeft, splitWidthRight);

			// split height is the maximum vertical delta
			double splitHeight = maxY - minY + 2 * defaultRoute.topBottomMargin;

			double scaleX = (windowWidth * renderer->getMapPanel().relativeWidth) / splitWidth;
			double scaleY = windowHeight / splitHeight;
			double finalScale = std::min(scaleX, scaleY);
			finalScale = std::max(defaultRoute.minimumScale, std::min(finalScale, defaultRoute.maximumScale));

			splitTransformation.x = -middlePoint.x();
			splitTransformation.y = middlePoint.y();
			splitTransformation.angle = finalAngle;
			splitTransformation.scale = finalScale;
		}

		defaultRoute.splitTransformations.push_back(splitTransformation);
	}

	shouldRestartTransition = true;
}

void RouteManager::calculateRoutePointColors()
{
	for (RoutePoint& rp : defaultRoute.routePoints)
		rp.color = interpolateFromGreenToRed(defaultRoute.highPace, defaultRoute.lowPace, rp.pace);

	for (RoutePoint& rp : defaultRoute.alignedRoutePoints)
		rp.color = interpolateFromGreenToRed(defaultRoute.highPace, defaultRoute.lowPace, rp.pace);
}

void RouteManager::calculateRunnerPosition(double currentTime)
{
	if (defaultRoute.alignedRoutePoints.empty())
		return;

	double offsetTime = currentTime + defaultRoute.runnerTimeOffset;
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

void RouteManager::calculateCurrentSplitTransformation(double currentTime, double frameTime)
{
	for (int i = 0; i < (int)defaultRoute.splitTimes.splitTimes.size() - 1; ++i)
	{
		double firstSplitOffsetTime = defaultRoute.splitTimes.splitTimes.at(i).time + defaultRoute.controlTimeOffset;
		double secondSplitOffsetTime = defaultRoute.splitTimes.splitTimes.at(i + 1).time + defaultRoute.controlTimeOffset;
		double runnerOffsetTime = currentTime + defaultRoute.runnerTimeOffset;

		if (runnerOffsetTime >= firstSplitOffsetTime && runnerOffsetTime < secondSplitOffsetTime)
		{
			if (i != defaultRoute.currentSplitTransformationIndex || shouldRestartTransition)
			{
				if (defaultRoute.useSmoothTransition)
				{
					defaultRoute.previousSplitTransformation = defaultRoute.currentSplitTransformation;
					defaultRoute.nextSplitTransformation = defaultRoute.splitTransformations.at(i);
					defaultRoute.transitionAlpha = 0.0;
					defaultRoute.transitionInProgress = true;
				}
				else
					defaultRoute.currentSplitTransformation = defaultRoute.splitTransformations.at(i);

				defaultRoute.currentSplitTransformationIndex = i;
				shouldRestartTransition = false;
			}

			break;
		}
	}

	if (defaultRoute.useSmoothTransition && defaultRoute.transitionInProgress)
	{
		if (defaultRoute.transitionAlpha > 1.0)
		{
			defaultRoute.transitionAlpha = 1.0;
			defaultRoute.transitionInProgress = false;
		}

		double alpha = defaultRoute.transitionAlpha;
		alpha = alpha * alpha * alpha * (alpha * (alpha * 6 - 15) + 10); // smootherstep

		defaultRoute.currentSplitTransformation.x = (1.0 - alpha) * defaultRoute.previousSplitTransformation.x + alpha * defaultRoute.nextSplitTransformation.x;
		defaultRoute.currentSplitTransformation.y = (1.0 - alpha) * defaultRoute.previousSplitTransformation.y + alpha * defaultRoute.nextSplitTransformation.y;
		defaultRoute.currentSplitTransformation.angle = (1.0 - alpha) * defaultRoute.previousSplitTransformation.angle + alpha * defaultRoute.nextSplitTransformation.angle;
		defaultRoute.currentSplitTransformation.scale = (1.0 - alpha) * defaultRoute.previousSplitTransformation.scale + alpha * defaultRoute.nextSplitTransformation.scale;

		defaultRoute.transitionAlpha += defaultRoute.smoothTransitionSpeed * frameTime;
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
