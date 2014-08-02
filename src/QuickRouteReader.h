// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>

namespace OrientView
{
	struct QuickRouteReaderResult
	{
		double topLeftLat = 0.0;
		double topLeftLong = 0.0;
		double bottomRightLat = 0.0;
		double bottomRightLong = 0.0;
	};

	class QuickRouteReader
	{

	public:

		static bool read(const QString& fileName, QuickRouteReaderResult* result);

	private:

		static bool readFromJpeg(const QString& fileName, QuickRouteReaderResult* result);
		static bool extractDataFromJpeg(QFile& file, QByteArray& buffer);
		static bool processDataFromJpeg(QByteArray& buffer, QuickRouteReaderResult* result);

		static bool readFromQrt(const QString& fileName, QuickRouteReaderResult* result);
	};
}
