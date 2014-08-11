// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QFile>
#include <QDateTime>
#include <QPointF>
#include <QTransform>

namespace OrientView
{
	struct Settings;

	struct RoutePoint
	{
		int segmentIndex = 0;
		QDateTime dateTime;
		QPointF coordinate;
		QPointF position;
		double elevation = 0.0;
		double heartRate = 0.0;
		double pace = 0.0;
	};

	struct RoutePointHandle
	{
		int segmentIndex = 0;
		double routePointIndex = 0.0;
		QTransform transformationMatrix;
	};

	struct RouteData
	{
		QPointF projectionOriginCoordinate;
		std::vector<RoutePoint> routePoints;
		std::vector<RoutePointHandle> routePointHandles;
	};

	// Read route and calibration data from QuickRoute JPEG files.
	class QuickRouteReader
	{

	public:

		bool initialize(Settings* settings);
		const RouteData& getRouteData() const;

	private:

		bool extractDataPartFromJpeg(QFile& file, QByteArray& buffer);
		bool readBytes(QFile& file, QByteArray& buffer, int count);
		void processDataPart(QDataStream& dataStream);
		void readSession(QDataStream& dataStream, uint32_t length);
		void readRoute(QDataStream& dataStream);
		void readHandles(QDataStream& dataStream);
		double readCoordinate(QDataStream& dataStream);
		QDateTime readDateTime(QDataStream& dataStream, QDateTime& previous);

		static void projectRoutePoint(RoutePoint& rp, const QPointF& projectionOriginCoordinate);

		RouteData routeData;
	};
}
