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

	if (!mapImage.load(fileName))
	{
		qWarning("Could not load image file");
		return false;
	}

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
