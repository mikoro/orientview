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
	window.multisamples = settings->value("window/multisamples", 16).toInt();
	window.fullscreen = settings->value("window/fullscreen", false).toBool();
	window.hideCursor = settings->value("window/hideCursor", false).toBool();

	mapCalibration.topLeftLat = settings->value("mapCalibration/topLeftLat", 0.0).toDouble();
	mapCalibration.topLeftLong = settings->value("mapCalibration/topLeftLong", 0.0).toDouble();
	mapCalibration.bottomRightLat = settings->value("mapCalibration/bottomRightLat", 0.0).toDouble();
	mapCalibration.bottomRightLong = settings->value("mapCalibration/bottomRightLong", 0.0).toDouble();

	videoCalibration.startOffset = settings->value("videoCalibration/startOffset", 0.0).toDouble();

	appearance.showInfoPanel = settings->value("appearance/showInfoPanel", false).toBool();
	appearance.mapPanelWidth = settings->value("appearance/mapPanelWidth", 0.3).toDouble();
	appearance.videoPanelScale = settings->value("appearance/videoPanelScale", 1.0).toDouble();
	appearance.mapHeaderCrop = settings->value("appearance/mapHeaderCrop", 0).toInt();
	
	decoder.frameCountDivisor = settings->value("decoder/frameCountDivisor", 1).toInt();
	decoder.frameDurationDivisor = settings->value("decoder/frameDurationDivisor", 1).toInt();
	decoder.frameSizeDivisor = settings->value("decoder/frameSizeDivisor", 1).toInt();

	stabilizer.enabled = settings->value("stabilizer/enabled", true).toBool();
	stabilizer.frameSizeDivisor = settings->value("stabilizer/frameSizeDivisor", 8).toInt();
	stabilizer.averagingFactor = settings->value("stabilizer/averagingFactor", 0.1).toDouble();
	stabilizer.dampingFactor = settings->value("stabilizer/dampingFactor", 1.0).toDouble();

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
	settings->setValue("appearance/videoPanelScale", appearance.videoPanelScale);
	settings->setValue("appearance/mapHeaderCrop", appearance.mapHeaderCrop);

	settings->setValue("decoder/frameCountDivisor", decoder.frameCountDivisor);
	settings->setValue("decoder/frameDurationDivisor", decoder.frameDurationDivisor);
	settings->setValue("decoder/frameSizeDivisor", decoder.frameSizeDivisor);

	settings->setValue("stabilizer/enabled", stabilizer.enabled);
	settings->setValue("stabilizer/frameSizeDivisor", stabilizer.frameSizeDivisor);
	settings->setValue("stabilizer/averagingFactor", stabilizer.averagingFactor);
	settings->setValue("stabilizer/dampingFactor", stabilizer.dampingFactor);

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

	mapCalibration.topLeftLat = ui->doubleSpinBoxMapTopLeftLat->value();
	mapCalibration.topLeftLong = ui->doubleSpinBoxMapTopLeftLong->value();
	mapCalibration.bottomRightLat = ui->doubleSpinBoxMapBottomRightLat->value();
	mapCalibration.bottomRightLong = ui->doubleSpinBoxMapBottomRightLong->value();

	videoCalibration.startOffset = ui->doubleSpinBoxVideoStartOffset->value();

	appearance.showInfoPanel = ui->checkBoxShowInfoPanel->isChecked();
	appearance.mapPanelWidth = ui->doubleSpinBoxMapPanelWidth->value();
	appearance.videoPanelScale = ui->doubleSpinBoxVideoPanelScale->value();
	appearance.mapHeaderCrop = ui->spinBoxMapHeaderCrop->value();

	decoder.frameCountDivisor = ui->spinBoxDecoderFrameCountDivisor->value();
	decoder.frameDurationDivisor = ui->spinBoxDecoderFrameDurationDivisor->value();
	decoder.frameSizeDivisor = ui->spinBoxDecoderFrameSizeDivisor->value();

	stabilizer.enabled = ui->checkBoxStabilizerEnabled->isChecked();
	stabilizer.frameSizeDivisor = ui->spinBoxStabilizerFrameSizeDivisor->value();
	stabilizer.averagingFactor = ui->doubleSpinBoxStabilizerAveragingFactor->value();
	stabilizer.dampingFactor = ui->doubleSpinBoxStabilizerDampingFactor->value();

	shaders.videoPanelShader = ui->comboBoxVideoPanelShader->currentText();
	shaders.mapPanelShader = ui->comboBoxMapPanelShader->currentText();

	encoder.preset = ui->comboBoxEncoderPreset->currentText();
	encoder.profile = ui->comboBoxEncoderProfile->currentText();
	encoder.constantRateFactor = ui->spinBoxEncoderCrf->value();
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

	ui->doubleSpinBoxMapTopLeftLat->setValue(mapCalibration.topLeftLat);
	ui->doubleSpinBoxMapTopLeftLong->setValue(mapCalibration.topLeftLong);
	ui->doubleSpinBoxMapBottomRightLat->setValue(mapCalibration.bottomRightLat);
	ui->doubleSpinBoxMapBottomRightLong->setValue(mapCalibration.bottomRightLong);

	ui->doubleSpinBoxVideoStartOffset->setValue(videoCalibration.startOffset);

	ui->checkBoxShowInfoPanel->setChecked(appearance.showInfoPanel);
	ui->doubleSpinBoxMapPanelWidth->setValue(appearance.mapPanelWidth);
	ui->doubleSpinBoxVideoPanelScale->setValue(appearance.videoPanelScale);
	ui->spinBoxMapHeaderCrop->setValue(appearance.mapHeaderCrop);

	ui->spinBoxDecoderFrameCountDivisor->setValue(decoder.frameCountDivisor);
	ui->spinBoxDecoderFrameDurationDivisor->setValue(decoder.frameDurationDivisor);
	ui->spinBoxDecoderFrameSizeDivisor->setValue(decoder.frameSizeDivisor);

	ui->checkBoxStabilizerEnabled->setChecked(stabilizer.enabled);
	ui->spinBoxStabilizerFrameSizeDivisor->setValue(stabilizer.frameSizeDivisor);
	ui->doubleSpinBoxStabilizerAveragingFactor->setValue(stabilizer.averagingFactor);
	ui->doubleSpinBoxStabilizerDampingFactor->setValue(stabilizer.dampingFactor);

	ui->comboBoxVideoPanelShader->setCurrentText(shaders.videoPanelShader);
	ui->comboBoxMapPanelShader->setCurrentText(shaders.mapPanelShader);

	ui->comboBoxEncoderPreset->setCurrentText(encoder.preset);
	ui->comboBoxEncoderProfile->setCurrentText(encoder.profile);
	ui->spinBoxEncoderCrf->setValue(encoder.constantRateFactor);
}
