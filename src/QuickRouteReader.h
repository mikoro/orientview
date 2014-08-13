// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <cstdint>

#include <QFile>
#include <QMatrix>

#include "RoutePoint.h"

namespace OrientView
{
	class MapImageReader;
	class Settings;

	// Read route point data from QuickRoute JPEG files.
	class QuickRouteReader
	{

	public:

		bool initialize(MapImageReader* mapImageReader, Settings* settings);
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
			QMatrix transformation;
		};

		double mapImageWidth = 0.0;
		double mapImageHeight = 0.0;
		double quickRouteImageWidth = 0.0;
		double quickRouteImageHeight = 0.0;
		QPointF projectionOriginCoordinate;
		std::vector<RoutePoint> routePoints;
		std::vector<RoutePointHandle> routePointHandles;
	};
}
