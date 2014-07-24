// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.


#include "QuickRouteReader.h"

using namespace OrientView;

QuickRouteReader::QuickRouteReader()
{
}

bool QuickRouteReader::initialize(const QString& fileName)
{
	qDebug("Initializing QuickRouteReader (%s)", fileName.toLocal8Bit().constData());

	if (!mapImage.load(fileName))
	{
		qWarning("Could not load image file");
		return false;
	}

	return true;
}

void QuickRouteReader::shutdown()
{
	qDebug("Shutting down QuickRouteReader");
}
