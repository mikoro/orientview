// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QFile>
#include <QSettings>

#include "Settings.h"
#include "ui_MainWindow.h"

using namespace OrientView;

void Settings::read(QSettings* settings)
{
	files.videoFilePath = settings->value("files/videoFilePath", "").toString();
	files.mapFilePath = settings->value("files/mapFilePath", "").toString();
	files.gpxFilePath = settings->value("files/gpxFilePath", "").toString();
	files.outputFilePath = settings->value("files/outputFilePath", "").toString();

	window.width = settings->value("window/width", 1280).toInt();
	window.height = settings->value("window/height", 720).toInt();
	window.multisamples = settings->value("window/multisamples", 4).toInt();
	window.fullscreen = settings->value("window/fullscreen", false).toBool();
	window.hideCursor = settings->value("window/hideCursor", false).toBool();

	mapCalibration.topLeftLat = settings->value("mapCalibration/topLeftLat", 0.0).toDouble();
	mapCalibration.topLeftLong = settings->value("mapCalibration/topLeftLong", 0.0).toDouble();
	mapCalibration.bottomRightLat = settings->value("mapCalibration/bottomRightLat", 0.0).toDouble();
	mapCalibration.bottomRightLong = settings->value("mapCalibration/bottomRightLong", 0.0).toDouble();

	videoCalibration.startOffset = settings->value("videoCalibration/startOffset", 0.0).toDouble();

	appearance.showInfoPanel = settings->value("appearance/showInfoPanel", false).toBool();
	appearance.mapPanelWidth = settings->value("appearance/mapPanelWidth", 0.3).toDouble();
	appearance.mapHeaderCrop = settings->value("appearance/mapHeaderCrop", 0).toInt();
	
	decoder.frameCountDivisor = settings->value("decoder/frameCountDivisor", 1).toInt();
	decoder.frameDurationDivisor = settings->value("decoder/frameDurationDivisor", 1).toInt();

	stabilization.enabled = settings->value("stabilization/enabled", true).toBool();
	stabilization.imageSizeDivisor = settings->value("stabilization/imageSizeDivisor", 8).toInt();

	shaders.videoPanelShader = settings->value("shaders/videoPanelShader", "default").toString();
	shaders.mapPanelShader = settings->value("shaders/mapPanelShader", "default").toString();

	encoder.preset = settings->value("encoder/preset", "veryfast").toString();
	encoder.profile = settings->value("encoder/profile", "high").toString();
	encoder.constantRateFactor = settings->value("encoder/constantRateFactor", 23).toInt();
}

void Settings::write(QSettings* settings)
{
	settings->setValue("files/videoFilePath", files.videoFilePath);
	settings->setValue("files/mapFilePath", files.mapFilePath);
	settings->setValue("files/gpxFilePath", files.gpxFilePath);
	settings->setValue("files/outputFilePath", files.outputFilePath);

	settings->setValue("window/width", window.width);
	settings->setValue("window/height", window.height);
	settings->setValue("window/multisamples", window.multisamples);
	settings->setValue("window/fullscreen", window.fullscreen);
	settings->setValue("window/hideCursor", window.hideCursor);

	settings->setValue("mapCalibration/topLeftLat", mapCalibration.topLeftLat);
	settings->setValue("mapCalibration/topLeftLong", mapCalibration.topLeftLong);
	settings->setValue("mapCalibration/bottomRightLat", mapCalibration.bottomRightLat);
	settings->setValue("mapCalibration/bottomRightLong", mapCalibration.bottomRightLong);

	settings->setValue("videoCalibration/startOffset", videoCalibration.startOffset);

	settings->setValue("appearance/showInfoPanel", appearance.showInfoPanel);
	settings->setValue("appearance/mapPanelWidth", appearance.mapPanelWidth);
	settings->setValue("appearance/mapHeaderCrop", appearance.mapHeaderCrop);

	settings->setValue("decoder/frameCountDivisor", decoder.frameCountDivisor);
	settings->setValue("decoder/frameDurationDivisor", decoder.frameDurationDivisor);

	settings->setValue("stabilization/enabled", stabilization.enabled);
	settings->setValue("stabilization/imageSizeDivisor", stabilization.imageSizeDivisor);

	settings->setValue("shaders/videoPanelShader", shaders.videoPanelShader);
	settings->setValue("shaders/mapPanelShader", shaders.mapPanelShader);

	settings->setValue("encoder/preset", encoder.preset);
	settings->setValue("encoder/profile", encoder.profile);
	settings->setValue("encoder/constantRateFactor", encoder.constantRateFactor);
}

void Settings::update(Ui::MainWindow* ui)
{
	files.videoFilePath = ui->lineEditVideoFile->text();
	files.mapFilePath = ui->lineEditMapFile->text();
	files.gpxFilePath = ui->lineEditGpxFile->text();
	files.outputFilePath = ui->lineEditOutputFile->text();

	window.width = ui->spinBoxWindowWidth->value();
	window.height = ui->spinBoxWindowHeight->value();
	window.multisamples = ui->comboBoxMultisamples->currentText().toInt();
	window.fullscreen = ui->checkBoxFullscreen->isChecked();
	window.hideCursor = ui->checkBoxHideCursor->isChecked();

	mapCalibration.topLeftLat = ui->doubleSpinBoxTopLeftLat->value();
	mapCalibration.topLeftLong = ui->doubleSpinBoxTopLeftLong->value();
	mapCalibration.bottomRightLat = ui->doubleSpinBoxBottomRightLat->value();
	mapCalibration.bottomRightLong = ui->doubleSpinBoxBottomRightLong->value();

	videoCalibration.startOffset = ui->doubleSpinBoxVideoStartOffset->value();

	appearance.showInfoPanel = ui->checkBoxShowInfoPanel->isChecked();
	appearance.mapPanelWidth = ui->doubleSpinBoxMapPanelWidth->value();
	appearance.mapHeaderCrop = ui->spinBoxMapHeaderCrop->value();

	decoder.frameCountDivisor = ui->spinBoxFrameCountDivisor->value();
	decoder.frameDurationDivisor = ui->spinBoxFrameDurationDivisor->value();

	stabilization.enabled = ui->checkBoxStabilizationEnabled->isChecked();
	stabilization.imageSizeDivisor = ui->spinBoxStabilizationImageSizeDivisor->value();

	shaders.videoPanelShader = ui->comboBoxVideoPanelShader->currentText();
	shaders.mapPanelShader = ui->comboBoxMapPanelShader->currentText();

	encoder.preset = ui->comboBoxEncoderPreset->currentText();
	encoder.profile = ui->comboBoxEncoderProfile->currentText();
	encoder.constantRateFactor = ui->spinBoxConstantRateFactor->value();
}

void Settings::apply(Ui::MainWindow* ui)
{
	ui->lineEditVideoFile->setText(files.videoFilePath);
	ui->lineEditMapFile->setText(files.mapFilePath);
	ui->lineEditGpxFile->setText(files.gpxFilePath);
	ui->lineEditOutputFile->setText(files.outputFilePath);

	ui->spinBoxWindowWidth->setValue(window.width);
	ui->spinBoxWindowHeight->setValue(window.height);
	ui->comboBoxMultisamples->setCurrentText(QString::number(window.multisamples));
	ui->checkBoxFullscreen->setChecked(window.fullscreen);
	ui->checkBoxHideCursor->setChecked(window.hideCursor);

	ui->doubleSpinBoxTopLeftLat->setValue(mapCalibration.topLeftLat);
	ui->doubleSpinBoxTopLeftLong->setValue(mapCalibration.topLeftLong);
	ui->doubleSpinBoxBottomRightLat->setValue(mapCalibration.bottomRightLat);
	ui->doubleSpinBoxBottomRightLong->setValue(mapCalibration.bottomRightLong);

	ui->doubleSpinBoxVideoStartOffset->setValue(videoCalibration.startOffset);

	ui->checkBoxShowInfoPanel->setChecked(appearance.showInfoPanel);
	ui->doubleSpinBoxMapPanelWidth->setValue(appearance.mapPanelWidth);
	ui->spinBoxMapHeaderCrop->setValue(appearance.mapHeaderCrop);

	ui->spinBoxFrameCountDivisor->setValue(decoder.frameCountDivisor);
	ui->spinBoxFrameDurationDivisor->setValue(decoder.frameDurationDivisor);

	ui->checkBoxStabilizationEnabled->setChecked(stabilization.enabled);
	ui->spinBoxStabilizationImageSizeDivisor->setValue(stabilization.imageSizeDivisor);

	ui->comboBoxVideoPanelShader->setCurrentText(shaders.videoPanelShader);
	ui->comboBoxMapPanelShader->setCurrentText(shaders.mapPanelShader);

	ui->comboBoxEncoderPreset->setCurrentText(encoder.preset);
	ui->comboBoxEncoderProfile->setCurrentText(encoder.profile);
	ui->spinBoxConstantRateFactor->setValue(encoder.constantRateFactor);
}
