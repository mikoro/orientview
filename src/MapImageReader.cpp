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
		return false;
	}

	if (settings->map.headerCrop > 0)
		mapImage = tempImage.copy(0, settings->map.headerCrop, tempImage.width(), tempImage.height() - settings->map.headerCrop);
	else
		mapImage = tempImage;

	return true;
}

QImage MapImageReader::getMapImage() const
{
	return mapImage;
}
