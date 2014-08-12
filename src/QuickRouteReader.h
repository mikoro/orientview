// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QFile>
#include <QDateTime>
#include <QPointF>
#include <QMatrix>

namespace OrientView
{
	struct Settings;

	struct RoutePoint
	{
		QDateTime dateTime;
		QPointF coordinate;
		QPointF projectedPosition;
		QPointF transformedPosition;
		double elevation = 0.0;
		double heartRate = 0.0;
		double distanceToPrevious = 0.0;
		double timeToPrevious = 0.0;
		double timeSinceStart = 0.0;
		double pace = 0.0;
	};

	// Read route and calibration data from QuickRoute JPEG files.
	class QuickRouteReader
	{

	public:

		bool initialize(Settings* settings);
		const std::vector<RoutePoint>& getRoutePoints() const;

	private:

		bool extractDataPartFromJpeg(QFile& file, QByteArray& buffer);
		bool readBytes(QFile& file, QByteArray& buffer, int count);
		void processDataPart(QDataStream& dataStream);
		void readSession(QDataStream& dataStream, uint32_t length);
		void readRoute(QDataStream& dataStream);
		void readHandles(QDataStream& dataStream);
		double readCoordinate(QDataStream& dataStream);
		QDateTime readDateTime(QDataStream& dataStream, QDateTime& previous);
		void processRoutePoints();
		QPointF projectCoordinate(const QPointF& coordinate, const QPointF& projectionOriginCoordinate);
		double coordinateDistance(const QPointF& coordinate1, const QPointF& coordinate2);

		struct RoutePointHandle
		{
			double routePointIndex = 0.0;
			QMatrix transformationMatrix;
		};

		QPointF projectionOriginCoordinate;
		std::vector<RoutePoint> routePoints;
		std::vector<RoutePointHandle> routePointHandles;
	};
}
