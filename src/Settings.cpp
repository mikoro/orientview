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
	display.fullscreen = settings.value("display/fullscreen", false).toBool();
	display.hideCursor = settings.value("display/hideCursor", false).toBool();

	shaders.videoPanelShader = settings.value("shaders/videoPanelShader", "basic").toString();
	shaders.mapPanelShader = settings.value("shaders/mapPanelShader", "basic").toString();

	appearance.mapImageHeaderCrop = settings.value("appearance/mapImageHeaderCrop", 0).toInt();
	appearance.mapPanelWidth = settings.value("appearance/mapPanelWidth", 0.3).toDouble();
	appearance.showInfoPanel = settings.value("appearance/showInfoPanel", false).toBool();

	stabilization.enabled = settings.value("stabilization/enabled", true).toBool();
	stabilization.imageSizeDivisor = settings.value("stabilization/imageSizeDivisor", 1).toInt();

	decoder.frameCountDivisor = settings.value("decoder/frameCountDivisor", 1).toInt();
	decoder.frameDurationDivisor = settings.value("decoder/frameDurationDivisor", 1).toInt();

	encoder.preset = settings.value("encoder/preset", "veryfast").toString();
	encoder.profile = settings.value("encoder/profile", "high").toString();
	encoder.constantRateFactor = settings.value("encoder/constantRateFactor", 23).toInt();

	return true;
}

void Settings::shutdown()
{
	qDebug("Shutting down Settings");
}
