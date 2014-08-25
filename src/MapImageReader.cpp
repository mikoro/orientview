// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "MapImageReader.h"
#include "Settings.h"

using namespace OrientView;

bool MapImageReader::initialize(Settings* settings)
{
	qDebug("Initializing map image reader (%s)", qPrintable(settings->map.imageFilePath));

	QImage tempImage;

	if (!tempImage.load(settings->map.imageFilePath))
	{
		qWarning("Could not load map image");

		mapImage = QImage(1024, 1024, QImage::Format::Format_ARGB32);
		mapImage.fill(Qt::red);

		return false;
	}
	else
	{
		if (settings->map.headerCrop > 0 && tempImage.height() > settings->map.headerCrop)
			tempImage = tempImage.copy(0, settings->map.headerCrop, tempImage.width(), tempImage.height() - settings->map.headerCrop);

		mapImage = tempImage;
		return true;
	}
}

QImage MapImageReader::getMapImage() const
{
	return mapImage;
}
