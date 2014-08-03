// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>

#include <QFile>

#include "QuickRouteReader.h"
#include "Settings.h"

using namespace OrientView;

namespace
{
	void projectRoutePoint(RoutePoint& rp, const QPointF& projectionOriginCoordinate)
	{
		const double R = 6378200;
		double lambda0 = projectionOriginCoordinate.x() * M_PI / 180.0;
		double phi0 = projectionOriginCoordinate.y() * M_PI / 180.0;
		double lambda = rp.coordinate.x() * M_PI / 180.0;
		double phi = rp.coordinate.y() * M_PI / 180.0;

		rp.position.setX(R * cos(phi) * sin(lambda - lambda0));
		rp.position.setY(R * (cos(phi0) * sin(phi) - sin(phi0) * cos(phi) * cos(lambda - lambda0)));
	}

	bool readBytesFromFile(QFile& file, QByteArray& buffer, int count, int& bytesRead)
	{
		buffer = file.read(count);
		bytesRead += count;

		if (buffer.size() != count)
		{
			qWarning("Could not read enough bytes");
			return false;
		}

		return true;
	}

	bool readUint8(QByteArray& buffer, int& index, uint8_t& result)
	{
		if (index >= buffer.size())
		{
			qWarning("Could not read enough bytes");
			return false;
		}

		result = 0;
		result |= (uint8_t)buffer.at(index) & 0xff;
		index += 1;

		return true;
	}

	bool readUint16(QByteArray& buffer, int& index, uint16_t& result)
	{
		if ((index + 1) >= buffer.size())
		{
			qWarning("Could not read enough bytes");
			return false;
		}

		result = 0;
		result |= ((uint16_t)buffer.at(index + 1) << 8) & 0xff00;
		result |= (uint16_t)buffer.at(index) & 0x00ff;
		index += 2;

		return true;
	}

	bool readUint32(QByteArray& buffer, int& index, uint32_t& result)
	{
		if ((index + 3) >= buffer.size())
		{
			qWarning("Could not read enough bytes");
			return false;
		}

		result = 0;
		result |= ((uint32_t)buffer.at(index + 3) << 24) & 0xff000000;
		result |= ((uint32_t)buffer.at(index + 2) << 16) & 0x00ff0000;
		result |= ((uint32_t)buffer.at(index + 1) << 8) & 0x0000ff00;
		result |= (uint32_t)buffer.at(index) & 0x000000ff;
		index += 4;

		return true;
	}

	bool readUint64(QByteArray& buffer, int& index, uint64_t& result)
	{
		if ((index + 7) >= buffer.size())
		{
			qWarning("Could not read enough bytes");
			return false;
		}

		result = 0;
		result |= ((uint64_t)buffer.at(index + 7) << 56) & 0xff00000000000000;
		result |= ((uint64_t)buffer.at(index + 6) << 48) & 0x00ff000000000000;
		result |= ((uint64_t)buffer.at(index + 5) << 40) & 0x0000ff0000000000;
		result |= ((uint64_t)buffer.at(index + 4) << 32) & 0x000000ff00000000;
		result |= ((uint64_t)buffer.at(index + 3) << 24) & 0x00000000ff000000;
		result |= ((uint64_t)buffer.at(index + 2) << 16) & 0x0000000000ff0000;
		result |= ((uint64_t)buffer.at(index + 1) << 8) & 0x000000000000ff00;
		result |= (uint64_t)buffer.at(index) & 0x00000000000000ff;
		index += 8;

		return true;
	}

	bool readCoordinate(QByteArray& buffer, int& index, double& result)
	{
		uint32_t tempValue;

		if (!readUint32(buffer, index, tempValue))
			return false;

		result = ((double)((int32_t)tempValue)) / 3600000.0;

		return true;
	}

	bool readDateTime(QByteArray& buffer, int& index, QDateTime& previous, QDateTime& result)
	{
		uint8_t timeType;

		if (!readUint8(buffer, index, timeType))
			return false;

		// absolute
		if (timeType == 0)
		{
			uint64_t timeValue;

			if (!readUint64(buffer, index, timeValue))
				return false;

			timeValue &= 0x3FFFFFFFFFFFFFFF; // remove kind data
			timeValue -= 621355968000000000; // offset to epoch start
			result = QDateTime::fromMSecsSinceEpoch(timeValue / 10000);
		}
		// relative
		else
		{
			uint16_t timeValue;

			if (!readUint16(buffer, index, timeValue))
				return false;

			result = previous.addSecs(timeValue / 1000);
		}

		return true;
	}

	bool readRoute(QByteArray& buffer, int& index, RouteData& routeData)
	{
		uint16_t attributes, extraWaypointAttributesLength;
		uint32_t segmentCount, waypointCount;

		if (!readUint16(buffer, index, attributes))
			return false;

		if (!readUint16(buffer, index, extraWaypointAttributesLength))
			return false;

		if (!readUint32(buffer, index, segmentCount))
			return false;

		QDateTime previousTime = QDateTime::fromMSecsSinceEpoch(0);

		for (int i = 0; i < segmentCount; i++)
		{
			if (!readUint32(buffer, index, waypointCount))
				return false;

			for (int j = 0; j < waypointCount; j++)
			{
				RoutePoint rp;
				double lon, lat;

				if (!readCoordinate(buffer, index, lon))
					return false;

				if (!readCoordinate(buffer, index, lat))
					return false;

				rp.coordinate.setX(lon);
				rp.coordinate.setY(lat);

				if (!readDateTime(buffer, index, previousTime, rp.dateTime))
					return false;

				previousTime = rp.dateTime;

				// heart rate
				if (attributes & 0x04)
				{
					uint8_t heartRate;

					if (!readUint8(buffer, index, heartRate))
						return false;

					rp.heartRate = heartRate;
				}

				// elevation
				if (attributes & 0x08)
				{
					uint16_t elevation;

					if (!readUint16(buffer, index, elevation))
						return false;

					rp.elevation = elevation;
				}

				index += extraWaypointAttributesLength;
				routeData.routePoints.push_back(rp);
			}
		}

		return true;
	}

	bool readHandles(QByteArray& buffer, int& index, RouteData& routeData)
	{
		uint32_t handleCount;

		if (!readUint32(buffer, index, handleCount))
			return false;

		for (int i = 0; i < handleCount; i++)
		{
			if (i == 0)
			{
				double matrix[9];

				for (int j = 0; j < 9; j++)
				{
					uint64_t tempValue;

					if (!readUint64(buffer, index, tempValue))
						return false;

					memcpy(&matrix[j], &tempValue, sizeof(double));
				}

				routeData.transformationMatrix.setMatrix(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8]);
			}
			else
				index += 72;
			
			index += 30;
		}

		return true;
	}

	bool readSession(QByteArray& buffer, int& index, RouteData& routeData)
	{
		bool routeRead = false;
		bool handlesRead = false;
		bool originRead = false;

		while (index < buffer.size())
		{
			uint8_t tag;
			uint32_t tagLength;

			if (!readUint8(buffer, index, tag))
				return false;

			if (!readUint32(buffer, index, tagLength))
				return false;

			// route
			if (tag == 7)
			{
				if (!readRoute(buffer, index, routeData))
					return false;

				routeRead = true;
			}
			// handles
			else if (tag == 8)
			{
				if (!readHandles(buffer, index, routeData))
					return false;

				handlesRead = true;
			}
			// projection origin
			else if (tag == 9)
			{
				double lon, lat;

				if (!readCoordinate(buffer, index, lon))
					return false;

				if (!readCoordinate(buffer, index, lat))
					return false;

				routeData.projectionOriginCoordinate.setX(lon);
				routeData.projectionOriginCoordinate.setY(lat);

				originRead = true;
			}
			else
				index += tagLength;

			if (routeRead && handlesRead && originRead)
				return true;
		}

		return false;
	}

	bool processExtractedDataPart(QByteArray& buffer, RouteData& routeData)
	{
		int index = 0;

		while (index < buffer.size())
		{
			uint8_t tag;
			uint32_t tagLength;

			if (!readUint8(buffer, index, tag))
				return false;

			if (!readUint32(buffer, index, tagLength))
				return false;

			// sessions
			if (tag == 5)
			{
				uint32_t sessionCount;

				if (!readUint32(buffer, index, sessionCount))
					return false;

				for (int i = 0; i < sessionCount; i++)
				{
					if (!readUint8(buffer, index, tag))
						return false;

					if (!readUint32(buffer, index, tagLength))
						return false;

					// session
					if (tag == 6)
						return readSession(buffer, index, routeData);
					else
						index += tagLength;
				}
			}
			else
				index += tagLength;
		}

		qWarning("Could not find coordinate values");
		return false;
	}

	bool extractDataPartFromJpeg(QFile& file, QByteArray& buffer)
	{
		int bytesRead = 0;
		const int quickRouteIdLength = 10;
		uint8_t quickRouteId[quickRouteIdLength] { 0x51, 0x75, 0x69, 0x63, 0x6b, 0x52, 0x6f, 0x75, 0x74, 0x65 };

		if (!readBytesFromFile(file, buffer, 2, bytesRead))
			return false;

		if ((uint8_t)buffer.at(0) != 0xff || (uint8_t)buffer.at(1) != 0xd8)
		{
			qWarning("Not a supported JPEG file format");
			return false;
		}

		while (bytesRead < file.size())
		{
			if (!readBytesFromFile(file, buffer, 2, bytesRead))
				return false;

			if ((uint8_t)buffer.at(0) != 0xff)
			{
				qWarning("Could not find QuickRoute Jpeg Extension Data");
				return false;
			}

			if ((uint8_t)buffer.at(1) == 0xe0)
			{
				if (!readBytesFromFile(file, buffer, 2, bytesRead))
					return false;

				uint32_t length = (uint32_t)buffer.at(1) + 256 * (uint32_t)buffer.at(0);

				if (length >= quickRouteIdLength + 2)
				{
					if (!readBytesFromFile(file, buffer, quickRouteIdLength, bytesRead))
						return false;

					bool match = true;

					for (int i = 0; i < quickRouteIdLength; i++)
					{
						if ((uint8_t)buffer.at(i) != quickRouteId[i])
						{
							match = false;
							break;
						}
					}

					if (!readBytesFromFile(file, buffer, length - 2 - quickRouteIdLength, bytesRead))
						return false;

					if (match)
						return true;
				}
				else
				{
					if (!readBytesFromFile(file, buffer, length - 2, bytesRead))
						return false;
				}
			}
		}

		qWarning("Could not find QuickRoute Jpeg Extension Data");
		return false;
	}
}

bool QuickRouteReader::initialize(Settings* settings)
{
	qDebug("Initializing QuickRouteReader (%s)", qPrintable(settings->files.quickRouteJpegMapImageFilePath));

	QFile file(settings->files.quickRouteJpegMapImageFilePath);

	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning("Could not open the file");
		return false;
	}

	routeData = RouteData();
	QByteArray buffer;

	if (!extractDataPartFromJpeg(file, buffer))
		return false;

	if (!processExtractedDataPart(buffer, routeData))
		return false;

	for (RoutePoint& rp : routeData.routePoints)
	{
		projectRoutePoint(rp, routeData.projectionOriginCoordinate);
		rp.position = routeData.transformationMatrix.map(rp.position);
	}

	return true;
}

const RouteData& QuickRouteReader::getRouteData() const
{
	return routeData;
}