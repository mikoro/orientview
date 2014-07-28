// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QFile>
#include <QSettings>

#include "Settings.h"

using namespace OrientView;

Settings::Settings()
{
}

bool Settings::initialize(const QString& fileName)
{
	qDebug("Initializing Settings (%s)", fileName.toLocal8Bit().constData());

	if (!QFile::exists(fileName))
	{
		qWarning("Settings file doesn't exist");
		return false;
	}

	QSettings settings(fileName, QSettings::IniFormat);

	display.width = settings.value("display/width", 1280).toInt();
	display.height = settings.value("display/height", 720).toInt();
	display.multisamples = settings.value("display/multisamples", 0).toInt();

	shaders.vertexShaderPath = settings.value("shaders/vertexShaderPath", "data/shaders/basic120.vert").toString();
	shaders.fragmentShaderPath = settings.value("shaders/fragmentShaderPath", "data/shaders/basic120.frag").toString();

	encoder.preset = settings.value("encoder/preset", "veryfast").toString();
	encoder.profile = settings.value("encoder/profile", "high").toString();
	encoder.constantRateFactor = settings.value("encoder/constantRateFactor", 23).toInt();

	appearance.mapPanelWidth = settings.value("appearance/mapPanelWidth", 0.3f).toFloat();

	return true;
}

void Settings::shutdown()
{
	qDebug("Shutting down Settings");
}
