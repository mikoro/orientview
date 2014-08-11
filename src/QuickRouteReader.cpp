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

	for (RoutePoint& rp : routeData.routePoints)
		projectRoutePoint(rp, routeData.projectionOriginCoordinate);

	return true;
}

const RouteData& QuickRouteReader::getRouteData() const
{
	return routeData;
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

			routeData.projectionOriginCoordinate.setX(lon);
			routeData.projectionOriginCoordinate.setY(lat);
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
			rp.segmentIndex = i;

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

			routeData.routePoints.push_back(rp);

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

		rph.segmentIndex = segmentIndex;
		rph.routePointIndex = routePointIndex;
		rph.transformationMatrix.setMatrix(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8]);

		routeData.routePointHandles.push_back(rph);

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

void QuickRouteReader::projectRoutePoint(RoutePoint& rp, const QPointF& projectionOriginCoordinate)
{
	const double R = 6378200;
	double lambda0 = projectionOriginCoordinate.x() * M_PI / 180.0;
	double phi0 = projectionOriginCoordinate.y() * M_PI / 180.0;
	double lambda = rp.coordinate.x() * M_PI / 180.0;
	double phi = rp.coordinate.y() * M_PI / 180.0;

	rp.position.setX(R * cos(phi) * sin(lambda - lambda0));
	rp.position.setY(R * (cos(phi0) * sin(phi) - sin(phi0) * cos(phi) * cos(lambda - lambda0)));
}
