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

	windowWidth = settings->window.width;
	windowHeight = settings->window.height;

	routes.push_back(Route());
	Route& defaultRoute = routes.at(0);

	defaultRoute.routePoints = quickRouteReader->getRoutePoints();
	defaultRoute.runnerInfo = splitsManager->getDefaultRunnerInfo();
	defaultRoute.wholeRouteRenderMode = settings->route.renderMode;
	defaultRoute.wholeRouteColor = settings->route.color;
	defaultRoute.wholeRouteWidth = settings->route.width;
	defaultRoute.wholeRouteBorderWidth = settings->route.borderWidth;
	defaultRoute.controlBorderColor = settings->route.controlBorderColor;
	defaultRoute.controlRadius = settings->route.controlRadius;
	defaultRoute.controlBorderWidth = settings->route.controlBorderWidth;
	defaultRoute.showControls = settings->route.showControls;
	defaultRoute.runnerColor = settings->route.runnerColor;
	defaultRoute.runnerBorderColor = settings->route.runnerBorderColor;
	defaultRoute.runnerBorderWidth = settings->route.runnerBorderWidth;
	defaultRoute.runnerScale = settings->route.runnerScale;
	defaultRoute.showRunner = settings->route.showRunner;
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

	defaultRoute.wholeRouteRenderMode = RouteRenderMode::None;
	defaultRoute.tailRenderMode = RouteRenderMode::Pace;

	for (Route& route : routes)
	{
		generateAlignedRoutePoints(route);
		calculateRoutePointColors(route);

		if (!initializeShaderAndBuffer(route))
			return false;
	}

	update(0.0, 0.0);

	if (defaultRoute.currentSplitTransformationIndex == -1 && defaultRoute.splitTransformations.size() > 0)
	{
		defaultRoute.currentSplitTransformation = defaultRoute.splitTransformations.at(0);
		defaultRoute.currentSplitTransformationIndex = 0;
	}

	return true;
}

RouteManager::~RouteManager()
{
	for (Route& route : routes)
	{
		if (route.shaderProgram != nullptr)
		{
			delete route.shaderProgram;
			route.shaderProgram = nullptr;
		}

		if (route.wholeRouteVertexBuffer != nullptr)
		{
			delete route.wholeRouteVertexBuffer;
			route.wholeRouteVertexBuffer = nullptr;
		}

		if (route.wholeRouteVertexArrayObject != nullptr)
		{
			delete route.wholeRouteVertexArrayObject;
			route.wholeRouteVertexArrayObject = nullptr;
		}

		if (route.tailVertexBuffer != nullptr)
		{
			delete route.tailVertexBuffer;
			route.tailVertexBuffer = nullptr;
		}

		if (route.tailVertexArrayObject != nullptr)
		{
			delete route.tailVertexArrayObject;
			route.tailVertexArrayObject = nullptr;
		}
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
		calculateCurrentRunnerPosition(route, currentTime);
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

bool RouteManager::initializeShaderAndBuffer(Route& route)
{
	double startTime = 0.0;
	double endTime = route.alignedRoutePoints.empty() ? 0.0 : route.alignedRoutePoints.back().time;
	std::vector<RouteVertex> wholeRouteVertices = strokeRoutePath(route, startTime, endTime);
	route.wholeRouteVertexCount = wholeRouteVertices.size();

	route.shaderProgram = new QOpenGLShaderProgram();
	route.wholeRouteVertexBuffer = new QOpenGLBuffer();
	route.wholeRouteVertexArrayObject = new QOpenGLVertexArrayObject();
	route.tailVertexBuffer = new QOpenGLBuffer();
	route.tailVertexArrayObject = new QOpenGLVertexArrayObject();

	route.wholeRouteVertexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
	route.wholeRouteVertexBuffer->create();
	route.wholeRouteVertexBuffer->bind();
	route.wholeRouteVertexBuffer->allocate(wholeRouteVertices.data(), sizeof(RouteVertex) * wholeRouteVertices.size());
	route.wholeRouteVertexBuffer->release();

	route.tailVertexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
	route.tailVertexBuffer->create();
	route.tailVertexBuffer->bind();
	route.tailVertexBuffer->allocate(sizeof(RouteVertex) * wholeRouteVertices.size());
	route.tailVertexBuffer->release();

	if (!route.shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/route.vert"))
		return false;

	if (!route.shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/route.frag"))
		return false;

	if (!route.shaderProgram->link())
		return false;

	route.wholeRouteVertexArrayObject->create();
	route.wholeRouteVertexArrayObject->bind();
	route.wholeRouteVertexBuffer->bind();
	route.shaderProgram->enableAttributeArray("vertexPosition");
	route.shaderProgram->enableAttributeArray("vertexTextureCoordinate");
	route.shaderProgram->enableAttributeArray("vertexColorPace");
	route.shaderProgram->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2, sizeof(GLfloat) * 8);
	route.shaderProgram->setAttributeBuffer("vertexTextureCoordinate", GL_FLOAT, sizeof(GLfloat) * 2, 2, sizeof(GLfloat) * 8);
	route.shaderProgram->setAttributeBuffer("vertexColorPace", GL_FLOAT, sizeof(GLfloat) * 4, 4, sizeof(GLfloat) * 8);
	route.wholeRouteVertexArrayObject->release();
	route.wholeRouteVertexBuffer->release();

	route.tailVertexArrayObject->create();
	route.tailVertexArrayObject->bind();
	route.tailVertexBuffer->bind();
	route.shaderProgram->enableAttributeArray("vertexPosition");
	route.shaderProgram->enableAttributeArray("vertexTextureCoordinate");
	route.shaderProgram->enableAttributeArray("vertexColorPace");
	route.shaderProgram->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2, sizeof(GLfloat) * 8);
	route.shaderProgram->setAttributeBuffer("vertexTextureCoordinate", GL_FLOAT, sizeof(GLfloat) * 2, 2, sizeof(GLfloat) * 8);
	route.shaderProgram->setAttributeBuffer("vertexColorPace", GL_FLOAT, sizeof(GLfloat) * 4, 4, sizeof(GLfloat) * 8);
	route.tailVertexArrayObject->release();
	route.tailVertexBuffer->release();

	return true;
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

void RouteManager::calculateCurrentRunnerPosition(Route& route, double currentTime)
{
	RoutePoint rp = getInterpolatedRoutePoint(route, currentTime + route.runnerTimeOffset);
	route.runnerPosition = rp.position;
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

std::vector<RouteVertex> RouteManager::strokeRoutePath(Route& route, double startTime, double endTime)
{
	std::vector<RouteVertex> routeVertices;

	int startIndex = (int)floor(startTime);
	int endIndex = (int)floor(endTime);
	int indexMax = (int)route.alignedRoutePoints.size() - 1;

	// limit the indexes inside a valid range
	startIndex = std::max(0, std::min(startIndex, indexMax));
	endIndex = std::max(0, std::min(endIndex, indexMax));

	if (startIndex == endIndex)
		return routeVertices;

	RoutePoint startRoutePoint = getInterpolatedRoutePoint(route, startTime);
	RoutePoint endRoutePoint = getInterpolatedRoutePoint(route, endTime);

	QPointF previousTlVertex, previousTrVertex;
	RouteVertex previousTlRouteVertex, previousTrRouteVertex;
	double previousAngleDeg = 0.0;
	bool isFirstPoint = true;

	// go through the points and generate rectangles of given width between them
	// start the next rectangle from either the top left or top right vertex of the previous rectangle (prevents overdraw when rectangles rotate)
	// fill in the joints with round join to make a smooth line
	for (int i = startIndex; i < endIndex;)
	{
		RoutePoint rp1 = route.alignedRoutePoints.at(i);
		RoutePoint rp2;
		QPointF routePointVector;

		// find the next point that isn't too close
		// if the next point is too close (and delta angle is large) it will be left inside the joint and the next rectangle will be drawn backwards
		for (int j = i + 1; j <= endIndex; ++j)
		{
			i = j;

			rp2 = route.alignedRoutePoints.at(j);
			routePointVector = rp2.position - rp1.position;

			double length = sqrt(routePointVector.x() * routePointVector.x() + routePointVector.y() * routePointVector.y());

			// this seems to work quite well, no good theory for it
			if (length > route.wholeRouteWidth)
				break;
		}

		double angle = atan2(-routePointVector.y(), routePointVector.x());
		double angleDeg = angle * 180.0 / M_PI;
		double angleDelta = angleDeg - previousAngleDeg;
		double finalAngleDelta = angleDelta;

		// if rotating more than 180 degrees, rotate to the opposite direction instead
		if (abs(angleDelta) > 180.0)
		{
			finalAngleDelta = 360.0 - abs(angleDelta);
			finalAngleDelta *= (angleDelta < 0.0) ? 1.0 : -1.0;
		}

		// this is a vector that points 90 degrees left from the center line
		QPointF deltaVertex;
		deltaVertex.setX(sin(angle) * (route.wholeRouteWidth / 2.0 + route.wholeRouteBorderWidth));
		deltaVertex.setY(cos(angle) * (route.wholeRouteWidth / 2.0 + route.wholeRouteBorderWidth));

		if (isFirstPoint)
		{
			previousTlVertex = rp1.position + deltaVertex;
			previousTrVertex = rp1.position - deltaVertex;
		}

		// the corner vertices of the rectangle
		QPointF blVertex;
		QPointF brVertex;
		QPointF tlVertex = rp2.position + deltaVertex;
		QPointF trVertex = rp2.position - deltaVertex;

		if (finalAngleDelta > 0.0)
		{
			// pivot around the top right vertex of the previous rectangle
			blVertex = previousTrVertex + 2.0 * deltaVertex;
			brVertex = previousTrVertex;
		}
		else
		{
			// pivot around the top left vertex of the previous rectangle
			blVertex = previousTlVertex;
			brVertex = previousTlVertex - 2.0 * deltaVertex;
		}

		RouteVertex blRouteVertex, brRouteVertex, tlRouteVertex, trRouteVertex;

		blRouteVertex.x = blVertex.x();
		blRouteVertex.y = -blVertex.y();
		blRouteVertex.u = -1.0f; // indicate left edge

		brRouteVertex.x = brVertex.x();
		brRouteVertex.y = -brVertex.y();
		brRouteVertex.u = 1.0f; // indicate right edge

		tlRouteVertex.x = tlVertex.x();
		tlRouteVertex.y = -tlVertex.y();
		tlRouteVertex.u = -1.0f;

		trRouteVertex.x = trVertex.x();
		trRouteVertex.y = -trVertex.y();
		trRouteVertex.u = 1.0f;

		blRouteVertex.paceR = brRouteVertex.paceR = rp1.color.redF();
		blRouteVertex.paceG = brRouteVertex.paceG = rp1.color.greenF();
		blRouteVertex.paceB = brRouteVertex.paceB = rp1.color.blueF();
		blRouteVertex.paceA = brRouteVertex.paceA = rp1.color.alphaF();

		tlRouteVertex.paceR = trRouteVertex.paceR = rp2.color.redF();
		tlRouteVertex.paceG = trRouteVertex.paceG = rp2.color.greenF();
		tlRouteVertex.paceB = trRouteVertex.paceB = rp2.color.blueF();
		tlRouteVertex.paceA = trRouteVertex.paceA = rp2.color.alphaF();

		QPointF jointOrigoVertex, jointStartVertex, jointEndVertex;
		RouteVertex jointOrigoRouteVertex, jointStartRouteVertex, jointEndRouteVertex;

		// figure out where is the origo of the joint and its start and end points
		if (finalAngleDelta > 0.0)
		{
			jointOrigoVertex = brVertex;
			jointStartVertex = previousTlVertex;
			jointEndVertex = blVertex;

			jointOrigoRouteVertex = brRouteVertex;
			jointStartRouteVertex = previousTlRouteVertex;
			jointEndRouteVertex = blRouteVertex;
		}
		else
		{
			jointOrigoVertex = blVertex;
			jointStartVertex = previousTrVertex;
			jointEndVertex = brVertex;

			jointOrigoRouteVertex = blRouteVertex;
			jointStartRouteVertex = previousTrRouteVertex;
			jointEndRouteVertex = brRouteVertex;
		}

		if (!isFirstPoint)
		{
			// small joints can be just filled in with one triangle
			if (abs(finalAngleDelta) <= 10.0)
			{
				routeVertices.push_back(jointOrigoRouteVertex);
				routeVertices.push_back(jointStartRouteVertex);
				routeVertices.push_back(jointEndRouteVertex);
			}
			else // fill out the joint with a round fan of triangles
			{
				QPointF origoToStart = jointStartVertex - jointOrigoVertex;

				double jointAngleIncrement = 10.0 * ((finalAngleDelta < 0.0) ? -1.0 : 1.0);
				double cumulativeJointAngle = 0.0;
				double routeWidth = route.wholeRouteWidth + 2.0 * route.wholeRouteBorderWidth;
				double currentJointAngle = atan2(-origoToStart.y(), origoToStart.x());

				currentJointAngle += jointAngleIncrement * M_PI / 180.0;
				cumulativeJointAngle += jointAngleIncrement;

				RouteVertex jointNewStartRouteVertex = jointStartRouteVertex;

				while (abs(cumulativeJointAngle) < abs(finalAngleDelta))
				{
					QPointF origoToNew(cos(currentJointAngle) * routeWidth, -sin(currentJointAngle) * routeWidth);
					QPointF newPosition = jointOrigoVertex + origoToNew;

					RouteVertex jointNewEndRouteVertex = jointEndRouteVertex;
					jointNewEndRouteVertex.x = newPosition.x();
					jointNewEndRouteVertex.y = -newPosition.y();

					routeVertices.push_back(jointOrigoRouteVertex);
					routeVertices.push_back(jointNewStartRouteVertex);
					routeVertices.push_back(jointNewEndRouteVertex);

					currentJointAngle += jointAngleIncrement * M_PI / 180.0;
					cumulativeJointAngle += jointAngleIncrement;

					jointNewStartRouteVertex = jointNewEndRouteVertex;
				}

				routeVertices.push_back(jointOrigoRouteVertex);
				routeVertices.push_back(jointNewStartRouteVertex);
				routeVertices.push_back(jointEndRouteVertex);
			}
		}

		routeVertices.push_back(blRouteVertex);
		routeVertices.push_back(brRouteVertex);
		routeVertices.push_back(trRouteVertex);
		routeVertices.push_back(blRouteVertex);
		routeVertices.push_back(trRouteVertex);
		routeVertices.push_back(tlRouteVertex);

		previousTlVertex = tlVertex;
		previousTrVertex = trVertex;
		previousTlRouteVertex = tlRouteVertex;
		previousTrRouteVertex = trRouteVertex;
		previousAngleDeg = angleDeg;

		isFirstPoint = false;
	}

	return routeVertices;
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
		RoutePoint firstRoutePoint = route.alignedRoutePoints.at(firstIndex);
		RoutePoint secondRoutePoint = route.alignedRoutePoints.at(secondIndex);
		RoutePoint interpolatedRoutePoint = firstRoutePoint;

		interpolatedRoutePoint.time = time;
		interpolatedRoutePoint.position = (1.0 - alpha) * firstRoutePoint.position + alpha * secondRoutePoint.position;
		interpolatedRoutePoint.elevation = (1.0 - alpha) * firstRoutePoint.elevation + alpha * secondRoutePoint.elevation;
		interpolatedRoutePoint.heartRate = (1.0 - alpha) * firstRoutePoint.heartRate + alpha * secondRoutePoint.heartRate;
		interpolatedRoutePoint.pace = (1.0 - alpha) * firstRoutePoint.pace + alpha * secondRoutePoint.pace;
		interpolatedRoutePoint.color = interpolateFromGreenToRed(route.highPace, route.lowPace, interpolatedRoutePoint.pace);

		return interpolatedRoutePoint;
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
