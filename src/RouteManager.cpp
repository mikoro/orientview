// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#define _USE_MATH_DEFINES
#include <cmath>

#include "RouteManager.h"
#include "QuickRouteReader.h"
#include "Renderer.h"
#include "Settings.h"

using namespace OrientView;

void RouteManager::initialize(QuickRouteReader* quickRouteReader, SplitsManager* splitsManager, Renderer* renderer, Settings* settings)
{
	this->renderer = renderer;

	routes.push_back(Route());
	Route& defaultRoute = routes.at(0);

	defaultRoute.routePoints = quickRouteReader->getRoutePoints();
	defaultRoute.runnerInfo = splitsManager->getDefaultRunnerInfo();
	defaultRoute.controlTimeOffset = settings->route.controlTimeOffset;
	defaultRoute.runnerTimeOffset = settings->route.runnerTimeOffset;
	defaultRoute.userScale = settings->route.scale;
	defaultRoute.minimumZoom = settings->route.minimumZoom;
	defaultRoute.maximumZoom = settings->route.maximumZoom;
	defaultRoute.topBottomMargin = settings->route.topBottomMargin;
	defaultRoute.leftRightMargin = settings->route.leftRightMargin;
	defaultRoute.lowPace = settings->route.lowPace;
	defaultRoute.highPace = settings->route.highPace;
	defaultRoute.useSmoothTransition = settings->route.useSmoothTransition;
	defaultRoute.smoothTransitionSpeed = settings->route.smoothTransitionSpeed;
	defaultRoute.showRunner = settings->route.showRunner;
	defaultRoute.showControls = settings->route.showControls;
	defaultRoute.routeRenderMode = settings->route.routeRenderMode;
	defaultRoute.routeColor = settings->route.routeColor;
	defaultRoute.routeWidth = settings->route.routeWidth;
	defaultRoute.controlBorderColor = settings->route.controlBorderColor;
	defaultRoute.controlRadius = settings->route.controlRadius;
	defaultRoute.controlBorderWidth = settings->route.controlBorderWidth;
	defaultRoute.runnerColor = settings->route.runnerColor;
	defaultRoute.runnerBorderColor = settings->route.runnerBorderColor;
	defaultRoute.runnerBorderWidth = settings->route.runnerBorderWidth;
	defaultRoute.runnerScale = settings->route.runnerScale;

	windowWidth = settings->window.width;
	windowHeight = settings->window.height;

	for (Route& route : routes)
	{
		generateAlignedRoutePoints(route);
		calculateRoutePointColors(route);
		generateRouteVertices(route);
	}

	update(0.0, 0.0);

	if (defaultRoute.currentSplitTransformationIndex == -1 && defaultRoute.splitTransformations.size() > 0)
	{
		defaultRoute.currentSplitTransformation = defaultRoute.splitTransformations.at(0);
		defaultRoute.currentSplitTransformationIndex = 0;
	}
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
		calculateRunnerPosition(route, currentTime);
		calculateCurrentSplitTransformation(route, currentTime, frameTime);
	}
}

void RouteManager::generateAlignedRoutePoints(Route& route)
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

void RouteManager::generateRouteVertices(Route& route)
{
	if (route.alignedRoutePoints.size() < 2)
		return;

	route.normalRouteVertices.clear();
	route.paceRouteVertices.clear();

	for (int i = 0; i < (int)route.routePoints.size() - 1; ++i)
	{
		RoutePoint& rp1 = route.routePoints.at(i);
		RoutePoint& rp2 = route.routePoints.at(i + 1);

		QPointF routePointVector = rp2.position - rp1.position;

		double angle = atan2(routePointVector.y(), routePointVector.x());

		QPointF deltaVertex;
		deltaVertex.setX(-sin(angle) * route.routeWidth);
		deltaVertex.setY(cos(angle) * route.routeWidth);

		QPointF leftVertex = rp1.position + deltaVertex;
		QPointF rightVertex = rp1.position - deltaVertex;

		RouteVertex rv1, rv2;

		rv1.x = leftVertex.x();
		rv1.y = -leftVertex.y();
		rv1.u = -1.0f;
		rv1.v = 0.0f;

		rv2.x = rightVertex.x();
		rv2.y = -rightVertex.y();
		rv2.u = 1.0f;
		rv2.v = 0.0f;

		rv1.r = rv2.r = route.routeColor.redF();
		rv1.g = rv2.g = route.routeColor.greenF();
		rv1.b = rv2.b = route.routeColor.blueF();
		rv1.a = rv2.a = route.routeColor.alphaF();

		route.normalRouteVertices.push_back(rv1);
		route.normalRouteVertices.push_back(rv2);

		rv1.r = rv2.r = rp1.color.redF();
		rv1.g = rv2.g = rp1.color.greenF();
		rv1.b = rv2.b = rp1.color.blueF();
		rv1.a = rv2.a = rp1.color.alphaF();

		route.paceRouteVertices.push_back(rv1);
		route.paceRouteVertices.push_back(rv2);
	}
	
	route.shaderProgram = new QOpenGLShaderProgram();
	route.shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/route.vert");
	route.shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/route.frag");
	route.shaderProgram->link();

	route.vertexMatrixUniform = route.shaderProgram->uniformLocation("vertexMatrix");
	route.borderColorUniform = route.shaderProgram->uniformLocation("borderColor");
	route.borderRelativeWidthUniform = route.shaderProgram->uniformLocation("borderRelativeWidth");

	route.normalRouteVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
	route.normalRouteVertexBuffer.create();
	route.normalRouteVertexBuffer.bind();
	route.normalRouteVertexBuffer.allocate(route.normalRouteVertices.data(), sizeof(RouteVertex) * route.normalRouteVertices.size());
	route.normalRouteVertexBuffer.release();

	route.paceRouteVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
	route.paceRouteVertexBuffer.create();
	route.paceRouteVertexBuffer.bind();
	route.paceRouteVertexBuffer.allocate(route.paceRouteVertices.data(), sizeof(RouteVertex) * route.paceRouteVertices.size());
	route.paceRouteVertexBuffer.release();
}

void RouteManager::calculateControlPositions(Route& route)
{
	if (route.runnerInfo.splits.empty() || route.alignedRoutePoints.empty())
		return;

	route.controlPositions.clear();

	for (const Split& split : route.runnerInfo.splits)
	{
		double offsetTime = split.absoluteTime + route.controlTimeOffset;
		double previousWholeSecond = floor(offsetTime);
		double alpha = offsetTime - previousWholeSecond;

		int firstIndex = (int)previousWholeSecond;
		int secondIndex = firstIndex + 1;
		int indexMax = (int)route.alignedRoutePoints.size() - 1;

		firstIndex = std::max(0, std::min(firstIndex, indexMax));
		secondIndex = std::max(0, std::min(secondIndex, indexMax));

		if (firstIndex == secondIndex)
			route.controlPositions.push_back(route.alignedRoutePoints.at(firstIndex).position);
		else
		{
			RoutePoint rp1 = route.alignedRoutePoints.at(firstIndex);
			RoutePoint rp2 = route.alignedRoutePoints.at(secondIndex);

			route.controlPositions.push_back((1.0 - alpha) * rp1.position + alpha * rp2.position);
		}
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
			double angle = atan2(-startToStop.y(), startToStop.x());
			angle *= (180.0 / M_PI);
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
			double splitWidthLeft = abs(minX - startPosition.x()) * 2.0 + 2.0 * route.leftRightMargin;
			double splitWidthRight = abs(maxX - startPosition.x()) * 2.0 + 2.0 * route.leftRightMargin;
			double splitWidth = std::max(splitWidthLeft, splitWidthRight);

			// split height is the maximum vertical delta
			double splitHeight = maxY - minY + 2.0 * route.topBottomMargin;

			double scaleX = (windowWidth * renderer->getMapPanel().relativeWidth) / splitWidth;
			double scaleY = windowHeight / splitHeight;
			double finalScale = std::min(scaleX, scaleY);
			finalScale = std::max(route.minimumZoom, std::min(finalScale, route.maximumZoom));

			splitTransformation.x = -middlePoint.x();
			splitTransformation.y = middlePoint.y();
			splitTransformation.angle = angle;
			splitTransformation.scale = finalScale;
		}

		route.splitTransformations.push_back(splitTransformation);
	}

	instantTransitionRequested = true;
}

void RouteManager::calculateRunnerPosition(Route& route, double currentTime)
{
	if (route.alignedRoutePoints.empty())
		return;

	double offsetTime = currentTime + route.runnerTimeOffset;
	double previousWholeSecond = floor(offsetTime);
	double alpha = offsetTime - previousWholeSecond;

	int firstIndex = (int)previousWholeSecond;
	int secondIndex = firstIndex + 1;
	int indexMax = (int)route.alignedRoutePoints.size() - 1;

	firstIndex = std::max(0, std::min(firstIndex, indexMax));
	secondIndex = std::max(0, std::min(secondIndex, indexMax));

	if (firstIndex == secondIndex)
		route.runnerPosition = route.alignedRoutePoints.at(firstIndex).position;
	else
	{
		RoutePoint rp1 = route.alignedRoutePoints.at(firstIndex);
		RoutePoint rp2 = route.alignedRoutePoints.at(secondIndex);

		route.runnerPosition = (1.0 - alpha) * rp1.position + alpha * rp2.position;
	}
}

void RouteManager::calculateCurrentSplitTransformation(Route& route, double currentTime, double frameTime)
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
				break;

			if (instantTransitionRequested)
			{
				route.currentSplitTransformation = route.splitTransformations.at(i);
				route.currentSplitTransformationIndex = i;
				instantTransitionRequested = false;
			}
			else if (i != route.currentSplitTransformationIndex)
			{
				if (route.useSmoothTransition)
				{
					route.previousSplitTransformation = route.currentSplitTransformation;
					route.nextSplitTransformation = route.splitTransformations.at(i);
					route.transitionAlpha = 0.0;
					route.transitionInProgress = true;

					double angleDelta = route.nextSplitTransformation.angle - route.previousSplitTransformation.angle;
					double absoluteAngleDelta = abs(angleDelta);
					double finalAngleDelta = angleDelta;

					// always try to rotate as little as possible
					if (absoluteAngleDelta > 180.0)
					{
						finalAngleDelta = 360.0 - absoluteAngleDelta;
						finalAngleDelta *= (angleDelta < 0.0) ? 1.0 : -1.0;
					}

					route.previousSplitTransformation.angleDelta = finalAngleDelta;
				}
				else
					route.currentSplitTransformation = route.splitTransformations.at(i);

				route.currentSplitTransformationIndex = i;
			}

			break;
		}
	}

	if (route.useSmoothTransition && route.transitionInProgress)
	{
		if (route.transitionAlpha > 1.0)
		{
			route.currentSplitTransformation = route.nextSplitTransformation;
			route.transitionInProgress = false;
		}
		else
		{
			double alpha = route.transitionAlpha;
			alpha = alpha * alpha * alpha * (alpha * (alpha * 6 - 15) + 10); // smootherstep

			route.currentSplitTransformation.x = (1.0 - alpha) * route.previousSplitTransformation.x + alpha * route.nextSplitTransformation.x;
			route.currentSplitTransformation.y = (1.0 - alpha) * route.previousSplitTransformation.y + alpha * route.nextSplitTransformation.y;
			route.currentSplitTransformation.angle = route.previousSplitTransformation.angle + alpha * route.previousSplitTransformation.angleDelta;
			route.currentSplitTransformation.scale = (1.0 - alpha) * route.previousSplitTransformation.scale + alpha * route.nextSplitTransformation.scale;

			route.transitionAlpha += route.smoothTransitionSpeed * frameTime;
		}
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
	instantTransitionRequested = true;
}

void RouteManager::windowResized(double newWidth, double newHeight)
{
	windowWidth = newWidth;
	windowHeight = newHeight;

	fullUpdateRequested = true;
}

double RouteManager::getX() const
{
	return routes.at(0).currentSplitTransformation.x;
}

double RouteManager::getY() const
{
	return routes.at(0).currentSplitTransformation.y;
}

double RouteManager::getScale() const
{
	return routes.at(0).currentSplitTransformation.scale;
}

double RouteManager::getAngle() const
{
	return routes.at(0).currentSplitTransformation.angle;
}

Route& RouteManager::getDefaultRoute()
{
	return routes.at(0);
}
