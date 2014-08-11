// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "MapImageReader.h"
#include "Settings.h"

using namespace OrientView;

bool MapImageReader::initialize(Settings* settings)
{
	QString fileName = settings->files.alternativeMapImageFilePath.isEmpty() ? settings->files.quickRouteJpegMapImageFilePath : settings->files.alternativeMapImageFilePath;

	qDebug("Initializing the map image reader (%s)", qPrintable(fileName));

	if (!mapImage.load(fileName))
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
