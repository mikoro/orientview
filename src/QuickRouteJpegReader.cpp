// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "QuickRouteJpegReader.h"

using namespace OrientView;

QuickRouteJpegReader::QuickRouteJpegReader()
{
}

bool QuickRouteJpegReader::initialize(const QString& fileName)
{
	qDebug("Initializing QuickRouteJpegReader (%s)", fileName.toLocal8Bit().constData());

	QImage tempImage;

	if (!tempImage.load(fileName))
	{
		qWarning("Could not load map image data");
		return false;
	}

	// remove header from the image
	const int headerHeight = 65;
	mapImage = tempImage.copy(0, headerHeight, tempImage.width(), tempImage.height() - headerHeight);

	mapFile.setFileName(fileName);

	if (!mapFile.open(QIODevice::ReadOnly))
	{
		qWarning("Could not open map image file for reading");
		return false;
	}

	mapFile.close();

	return true;
}

void QuickRouteJpegReader::shutdown()
{
	qDebug("Shutting down QuickRouteJpegReader");
}

QImage QuickRouteJpegReader::getMapImage() const
{
	return mapImage;
}
