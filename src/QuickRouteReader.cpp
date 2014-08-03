// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <cstdint>

#include <QFile>

#include "QuickRouteReader.h"

using namespace OrientView;

namespace
{
	bool readBytes(QFile& file, QByteArray& buffer, int count, int* bytesRead)
	{
		buffer = file.read(count);
		*bytesRead += count;

		if (buffer.size() != count)
		{
			qWarning("Could not read enough bytes");
			return false;
		}

		return true;
	}

	bool readUint8(QByteArray& buffer, int* index, uint8_t* result)
	{
		if (*index >= buffer.size())
		{
			qWarning("Could not read enough bytes");
			return false;
		}

		*result = (uint8_t)buffer.at(*index);
		(*index)++;

		return true;
	}

	bool readUint32(QByteArray& buffer, int* index, uint32_t* result)
	{
		if ((*index + 3) >= buffer.size())
		{
			qWarning("Could not read enough bytes");
			return false;
		}

		int j = *index;
		uint32_t k = ((uint8_t)buffer.at(*index + 3)) << 24 | ((uint8_t)buffer.at(*index + 2)) << 16 | ((uint8_t)buffer.at(*index + 1)) << 8 | ((uint8_t)buffer.at(*index));

		*result = ((uint8_t)buffer.at(*index + 3)) << 24 | ((uint8_t)buffer.at(*index + 2)) << 16 | ((uint8_t)buffer.at(*index + 1)) << 8 | ((uint8_t)buffer.at(*index));
		*index += 4;

		return true;
	}

	bool readCoordinate(QByteArray& buffer, int* index, double* result)
	{
		uint32_t tempValue;

		if (!readUint32(buffer, index, &tempValue))
			return false;

		*result = ((double)((int32_t)tempValue)) / 3600000.0;

		return true;
	}

	bool readAllCornerCoordinates(QByteArray& buffer, int* index, QuickRouteReaderResult* result)
	{
		if (!readCoordinate(buffer, index, &result->bottomLeftLon))
			return false;

		if (!readCoordinate(buffer, index, &result->bottomLeftLat))
			return false;

		if (!readCoordinate(buffer, index, &result->topLeftLon))
			return false;

		if (!readCoordinate(buffer, index, &result->topLeftLat))
			return false;

		if (!readCoordinate(buffer, index, &result->topRightLon))
			return false;

		if (!readCoordinate(buffer, index, &result->topRightLat))
			return false;

		if (!readCoordinate(buffer, index, &result->bottomRightLon))
			return false;

		if (!readCoordinate(buffer, index, &result->bottomRightLat))
			return false;

		return true;
	}

	bool extractDataFromJpeg(QFile& file, QByteArray& buffer)
	{
		int bytesRead = 0;
		const int quickRouteIdLength = 10;
		uint8_t quickRouteId[quickRouteIdLength] { 0x51, 0x75, 0x69, 0x63, 0x6b, 0x52, 0x6f, 0x75, 0x74, 0x65 };

		if (!readBytes(file, buffer, 2, &bytesRead))
			return false;

		if ((uint8_t)buffer.at(0) != 0xff || (uint8_t)buffer.at(1) != 0xd8)
		{
			qWarning("Not a supported JPEG file format");
			return false;
		}

		while (bytesRead < file.size())
		{
			if (!readBytes(file, buffer, 2, &bytesRead))
				return false;

			if ((uint8_t)buffer.at(0) != 0xff)
			{
				qWarning("Could not find QuickRoute Jpeg Extension Data");
				return false;
			}

			if ((uint8_t)buffer.at(1) == 0xe0)
			{
				if (!readBytes(file, buffer, 2, &bytesRead))
					return false;

				uint32_t length = (uint32_t)buffer.at(1) + 256 * (uint32_t)buffer.at(0);

				if (length >= quickRouteIdLength + 2)
				{
					if (!readBytes(file, buffer, quickRouteIdLength, &bytesRead))
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

					if (!readBytes(file, buffer, length - 2 - quickRouteIdLength, &bytesRead))
						return false;

					if (match)
						return true;
				}
				else
				{
					if (!readBytes(file, buffer, length - 2, &bytesRead))
						return false;
				}
			}
		}

		qWarning("Could not find QuickRoute Jpeg Extension Data");
		return false;
	}

	bool processExtractedData(QByteArray& buffer, QuickRouteReaderResult* result, bool includeBorders)
	{
		int index = 0;
		bool coordinatesRead = false;
		bool projectionOriginRead = false;

		while (index < buffer.size())
		{
			uint8_t tag1;
			uint32_t tagLength1;

			if (!readUint8(buffer, &index, &tag1))
				return false;

			if (!readUint32(buffer, &index, &tagLength1))
				return false;

			bool tagRead1 = false;

			// map corner coordinates without borders
			if (tag1 == 2 && !includeBorders)
			{
				if (!readAllCornerCoordinates(buffer, &index, result))
					return false;

				coordinatesRead = true;
				tagRead1 = true;
			}

			// map corner coordinates with borders
			if (tag1 == 3 && includeBorders)
			{
				if (!readAllCornerCoordinates(buffer, &index, result))
					return false;

				coordinatesRead = true;
				tagRead1 = true;
			}

			// sessions
			if (tag1 == 5)
			{
				uint32_t sessionCount;
				uint8_t tag2;
				uint32_t tagLength2;

				if (!readUint32(buffer, &index, &sessionCount))
					return false;

				for (int i = 0; i < sessionCount; i++)
				{
					if (!readUint8(buffer, &index, &tag2))
						return false;

					if (!readUint32(buffer, &index, &tagLength2))
						return false;

					// session
					if (tag2 == 6)
					{
						while (index < buffer.size())
						{
							uint8_t tag3;
							uint32_t tagLength3;

							if (!readUint8(buffer, &index, &tag3))
								return false;

							if (!readUint32(buffer, &index, &tagLength3))
								return false;

							// projection origin
							if (tag3 == 9)
							{
								if (!readCoordinate(buffer, &index, &result->projectionOriginLon))
									return false;

								if (!readCoordinate(buffer, &index, &result->projectionOriginLat))
									return false;

								projectionOriginRead = true;
								break;
							}
							else
								index += tagLength3;
						}
					}
					else
						index += tagLength2;

					if (projectionOriginRead)
						break;
				}

				tagRead1 = true;
			}
			
			if (!tagRead1)
				index += tagLength1;

			if (coordinatesRead && projectionOriginRead)
				return true;
		}

		qWarning("Could not find coordinate values");
		return false;
	}
}

bool QuickRouteReader::readFromJpeg(const QString& fileName, QuickRouteReaderResult* result, bool includeBorders)
{
	QFile file(fileName);

	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning("Could not open the file");
		return false;
	}

	QByteArray buffer;

	if (!extractDataFromJpeg(file, buffer))
		return false;

	if (!processExtractedData(buffer, result, includeBorders))
		return false;

	return true;
}
