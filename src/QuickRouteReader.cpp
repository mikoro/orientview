// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>

#include "QuickRouteReader.h"
#include "Settings.h"

using namespace OrientView;

bool QuickRouteReader::initialize(Settings* settings)
{
	qDebug("Initializing the QuickRoute reader (%s)", qPrintable(settings->files.quickRouteJpegMapImageFilePath));

	QFile file(settings->files.quickRouteJpegMapImageFilePath);

	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning("Could not open the file");
		return false;
	}

	QByteArray buffer;

	if (!extractDataPartFromJpeg(file, buffer))
		return false;

	QDataStream dataStream(buffer);
	dataStream.setByteOrder(QDataStream::LittleEndian);

	processDataPart(dataStream);

	if (projectionOriginCoordinate.isNull())
	{
		qWarning("Could not find projection origin");
		return false;
	}

	processRoutePoints();

	return true;
}

const std::vector<RoutePoint>& QuickRouteReader::getRoutePoints() const
{
	return routePoints;
}

bool QuickRouteReader::extractDataPartFromJpeg(QFile& file, QByteArray& buffer)
{
	const int quickRouteIdLength = 10;
	char quickRouteId[quickRouteIdLength] { 0x51, 0x75, 0x69, 0x63, 0x6b, 0x52, 0x6f, 0x75, 0x74, 0x65 };
	QByteArray quickRouteIdBuffer(quickRouteId, quickRouteIdLength);
	QByteArray readBuffer;

	if (!readBytes(file, readBuffer, 2))
		return false;

	if ((uint8_t)readBuffer.at(0) != 0xff || (uint8_t)readBuffer.at(1) != 0xd8)
	{
		qWarning("Not a supported JPEG file format");
		return false;
	}

	while ((file.size() - file.pos()) >= 2)
	{
		if (!readBytes(file, readBuffer, 2))
			return false;

		if ((uint8_t)readBuffer.at(0) != 0xff)
		{
			qWarning("Could not find QuickRoute Jpeg Extension Data");
			return false;
		}

		if ((uint8_t)readBuffer.at(1) == 0xe0)
		{
			if (!readBytes(file, readBuffer, 2))
				return false;

			uint32_t length = (uint32_t)readBuffer.at(1) + 256 * (uint32_t)readBuffer.at(0);

			if (length >= (quickRouteIdLength + 2))
			{
				if (!readBytes(file, readBuffer, quickRouteIdLength))
					return false;

				bool match = readBuffer.startsWith(quickRouteIdBuffer);

				if (!readBytes(file, readBuffer, length - 2 - quickRouteIdLength))
					return false;

				if (match)
				{
					buffer = readBuffer;
					return true;
				}
			}
			else
			{
				if (!readBytes(file, readBuffer, length - 2))
					return false;
			}

		}
	}

	qWarning("Could not find QuickRoute Jpeg Extension Data");
	return false;
}

bool QuickRouteReader::readBytes(QFile& file, QByteArray& buffer, int count)
{
	if ((file.size() - file.pos()) < count)
	{
		qWarning("Could not read enough bytes");
		return false;
	}

	buffer = file.read(count);

	return true;
}

void QuickRouteReader::processDataPart(QDataStream& dataStream)
{
	while (!dataStream.atEnd())
	{
		uint8_t tag;
		uint32_t tagLength;

		dataStream >> tag;
		dataStream >> tagLength;

		if (tag == 5) // sessions
		{
			uint32_t sessionCount;
			dataStream >> sessionCount;

			for (uint32_t i = 0; i < sessionCount; i++)
			{
				dataStream >> tag;
				dataStream >> tagLength;

				if (tag == 6) // session
					readSession(dataStream, tagLength);
				else
					dataStream.skipRawData(tagLength);
			}
		}
		else
			dataStream.skipRawData(tagLength);
	}
}

void QuickRouteReader::readSession(QDataStream& dataStream, uint32_t length)
{
	uint32_t readLength = 0;

	do
	{
		uint8_t tag;
		uint32_t tagLength;

		dataStream >> tag;
		dataStream >> tagLength;

		if (tag == 7) // route
			readRoute(dataStream);
		else if (tag == 8) // handles
			readHandles(dataStream);
		else if (tag == 9) // projection origin
		{
			double lon = readCoordinate(dataStream);
			double lat = readCoordinate(dataStream);

			projectionOriginCoordinate.setX(lon);
			projectionOriginCoordinate.setY(lat);
		}
		else
			dataStream.skipRawData(tagLength);

		readLength += (1 + 4 + tagLength);

	} while (readLength < length);
}

void QuickRouteReader::readRoute(QDataStream& dataStream)
{
	uint16_t attributes, extraWaypointAttributesLength;
	uint32_t segmentCount;

	dataStream >> attributes;
	dataStream >> extraWaypointAttributesLength;
	dataStream >> segmentCount;

	QDateTime previousTime = QDateTime::fromMSecsSinceEpoch(0);

	for (uint32_t i = 0; i < segmentCount; i++)
	{
		uint32_t waypointCount;
		dataStream >> waypointCount;

		for (uint32_t j = 0; j < waypointCount; j++)
		{
			RoutePoint rp;

			double lon = readCoordinate(dataStream);
			double lat = readCoordinate(dataStream);

			rp.coordinate.setX(lon);
			rp.coordinate.setY(lat);

			rp.dateTime = readDateTime(dataStream, previousTime);
			previousTime = rp.dateTime;

			// heart rate
			if (attributes & 0x04)
			{
				uint8_t heartRate;
				dataStream >> heartRate;
				rp.heartRate = heartRate;
			}

			// elevation
			if (attributes & 0x08)
			{
				uint16_t elevation;
				dataStream >> elevation;
				rp.elevation = elevation;
			}

			if (i == 0)
				routePoints.push_back(rp);

			dataStream.skipRawData(extraWaypointAttributesLength);
		}
	}
}

void QuickRouteReader::readHandles(QDataStream& dataStream)
{
	uint32_t handleCount;
	dataStream >> handleCount;

	for (uint32_t i = 0; i < handleCount; i++)
	{
		RoutePointHandle rph;
		double matrix[9];

		for (int j = 0; j < 9; j++)
			dataStream >> matrix[j];

		uint32_t segmentIndex;
		dataStream >> segmentIndex;

		double routePointIndex;
		dataStream >> routePointIndex;

		rph.routePointIndex = routePointIndex;
		rph.transformationMatrix.setMatrix(matrix[0], matrix[1], matrix[3], matrix[4], matrix[2], matrix[5]);

		if (segmentIndex == 0)
			routePointHandles.push_back(rph);

		dataStream.skipRawData(18);
	}
}

double QuickRouteReader::readCoordinate(QDataStream& dataStream)
{
	uint32_t tempValue;
	dataStream >> tempValue;

	return (((double)((int32_t)tempValue)) / 3600000.0);
}

QDateTime QuickRouteReader::readDateTime(QDataStream& dataStream, QDateTime& previous)
{
	uint8_t timeType;
	dataStream >> timeType;

	// absolute
	if (timeType == 0)
	{
		uint64_t timeValue;
		dataStream >> timeValue;

		timeValue &= 0x3FFFFFFFFFFFFFFF; // remove kind data
		timeValue -= 621355968000000000; // offset to epoch start

		return QDateTime::fromMSecsSinceEpoch(timeValue / 10000);
	}
	// relative
	else
	{
		uint16_t timeValue;
		dataStream >> timeValue;

		return previous.addSecs(timeValue / 1000);
	}
}

void QuickRouteReader::processRoutePoints()
{
	int handleIndex = 0;

	RoutePointHandle currentHandle;
	RoutePointHandle nextHandle;
	nextHandle.routePointIndex = DBL_MAX;

	if (handleIndex < routePointHandles.size())
		currentHandle = routePointHandles.at(handleIndex);

	if (++handleIndex < routePointHandles.size())
		nextHandle = routePointHandles.at(handleIndex);

	// transform points using the previous handle
	// use first handle if no previous handle
	// do not use the last handle at all (use second to last handle instead for the last points)
	for (int i = 0; i < routePoints.size(); ++i)
	{
		if ((double)i > nextHandle.routePointIndex)
		{
			if (++handleIndex < routePointHandles.size())
			{
				currentHandle = nextHandle;
				nextHandle = routePointHandles.at(handleIndex);
			}
			else
				nextHandle.routePointIndex = DBL_MAX;
		}

		routePoints.at(i).projectedPosition = projectCoordinate(routePoints.at(i).coordinate, projectionOriginCoordinate);
		routePoints.at(i).transformedPosition = currentHandle.transformationMatrix.map(routePoints.at(i).projectedPosition);
	}

	// calculate delta times, delta distances, and paces
	for (int i = 1; i < routePoints.size(); ++i)
	{
		routePoints.at(i).timeToPrevious = (routePoints.at(i).dateTime.toMSecsSinceEpoch() - routePoints.at(i - 1).dateTime.toMSecsSinceEpoch()) / 1000.0;
		routePoints.at(i).distanceToPrevious = coordinateDistance(routePoints.at(i - 1).coordinate, routePoints.at(i).coordinate);
		routePoints.at(i).timeSinceStart = (routePoints.at(i).dateTime.toMSecsSinceEpoch() - routePoints.at(0).dateTime.toMSecsSinceEpoch()) / 1000.0;

		if (routePoints.at(i).distanceToPrevious > 0.0)
			routePoints.at(i).pace = (routePoints.at(i).timeToPrevious / 60.0) / (routePoints.at(i).distanceToPrevious / 1000.0);
	}
}

QPointF QuickRouteReader::projectCoordinate(const QPointF& coordinate, const QPointF& projectionOriginCoordinate)
{
	const double R = 6378200.0;

	double lambda0 = projectionOriginCoordinate.x() * M_PI / 180.0;
	double phi0 = projectionOriginCoordinate.y() * M_PI / 180.0;
	double lambda = coordinate.x() * M_PI / 180.0;
	double phi = coordinate.y() * M_PI / 180.0;

	QPointF position;

	position.setX(R * cos(phi) * sin(lambda - lambda0));
	position.setY(R * (cos(phi0) * sin(phi) - sin(phi0) * cos(phi) * cos(lambda - lambda0)));

	return position;
}

double QuickRouteReader::coordinateDistance(const QPointF& coordinate1, const QPointF& coordinate2)
{
	const double R = 6378200.0;

	double lat1 = coordinate1.y() * M_PI / 180.0;
	double lat2 = coordinate2.y() * M_PI / 180.0;
	double deltaLon = (coordinate2.x() - coordinate1.x()) * M_PI / 180.0;
	double deltaLat = (coordinate2.y() - coordinate1.y()) * M_PI / 180.0;

	double a = pow(sin(deltaLat / 2.0), 2.0) + cos(lat1) * cos(lat2) * pow(sin(deltaLon / 2.0), 2.0);
	double distance = 2.0 * R * atan2(sqrt(a), sqrt(1.0 - a));

	return distance;
}
