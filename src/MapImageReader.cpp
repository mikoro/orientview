// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "MapImageReader.h"
#include "Settings.h"

using namespace OrientView;

bool MapImageReader::initialize(Settings* settings)
{
	qDebug("Initializing the map image reader (%s)", qPrintable(settings->mapAndRoute.mapImageFilePath));

	if (!mapImage.load(settings->mapAndRoute.mapImageFilePath))
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
