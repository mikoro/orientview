// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "MapImageReader.h"
#include "Settings.h"

using namespace OrientView;

bool MapImageReader::initialize(const QString& fileName, Settings* settings)
{
	qDebug("Initializing MapImageReader (%s)", fileName.toLocal8Bit().constData());

	QImage tempImage;

	if (!tempImage.load(fileName))
	{
		qWarning("Could not load map image");
		return false;
	}

	mapImage = tempImage.copy(0, settings->appearance.mapHeaderCrop, tempImage.width(), tempImage.height() - settings->appearance.mapHeaderCrop);

	return true;
}

QImage MapImageReader::getMapImage() const
{
	return mapImage;
}
