// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "Settings.h"

using namespace OrientView;

Settings::Settings()
{
}

bool Settings::initialize(const QString& fileName)
{
	qDebug("Initializing Settings");

	return true;
}

void Settings::shutdown()
{
	qDebug("Shutting down Settings");
}
