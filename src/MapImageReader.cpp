// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "MapImageReader.h"
#include "Settings.h"

using namespace OrientView;

bool MapImageReader::initialize(Settings* settings)
{
	qDebug("Initializing map image reader (%s)", qPrintable(settings->map.imageFilePath));

	if (!mapImage.load(settings->map.imageFilePath))
	{
		qWarning("Could not load map image");
		return false;
	}

	return true;
}

QImage MapImageReader::getMapImage() const
{
	return mapImage;
}
