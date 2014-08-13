// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QFile>
#include <QSettings>

#include "Settings.h"
#include "ui_MainWindow.h"

using namespace OrientView;

void Settings::readFromQSettings(QSettings* settings)
{
	qDebug("Reading settings from %s", qPrintable(settings->fileName()));

	Settings defaultSettings;

	videoDecoder.inputVideoFilePath = settings->value("videoDecoder/inputVideoFilePath", defaultSettings.videoDecoder.inputVideoFilePath).toString();
	videoDecoder.frameCountDivisor = settings->value("videoDecoder/frameCountDivisor", defaultSettings.videoDecoder.frameCountDivisor).toInt();
	videoDecoder.frameDurationDivisor = settings->value("videoDecoder/frameDurationDivisor", defaultSettings.videoDecoder.frameDurationDivisor).toInt();
	videoDecoder.frameSizeDivisor = settings->value("videoDecoder/frameSizeDivisor", defaultSettings.videoDecoder.frameSizeDivisor).toInt();
	videoDecoder.enableVerboseLogging = settings->value("videoDecoder/enableVerboseLogging", defaultSettings.videoDecoder.enableVerboseLogging).toBool();

	mapAndRoute.quickRouteJpegFilePath = settings->value("mapAndRoute/quickRouteJpegFilePath", defaultSettings.mapAndRoute.quickRouteJpegFilePath).toString();
	mapAndRoute.mapImageFilePath = settings->value("mapAndRoute/mapImageFilePath", defaultSettings.mapAndRoute.mapImageFilePath).toString();
	mapAndRoute.routeStartOffset = settings->value("mapAndRoute/routeStartOffset", defaultSettings.mapAndRoute.routeStartOffset).toDouble();

	splitTimes.splitTimes = settings->value("splitTimes/splitTimes", defaultSettings.splitTimes.splitTimes).toString();

	window.width = settings->value("window/width", defaultSettings.window.width).toInt();
	window.height = settings->value("window/height", defaultSettings.window.height).toInt();
	window.multisamples = settings->value("window/multisamples", defaultSettings.window.multisamples).toInt();
	window.fullscreen = settings->value("window/fullscreen", defaultSettings.window.fullscreen).toBool();
	window.hideCursor = settings->value("window/hideCursor", defaultSettings.window.hideCursor).toBool();

	appearance.showInfoPanel = settings->value("appearance/showInfoPanel", defaultSettings.appearance.showInfoPanel).toBool();
	appearance.mapPanelWidth = settings->value("appearance/mapPanelWidth", defaultSettings.appearance.mapPanelWidth).toDouble();
	appearance.videoPanelScale = settings->value("appearance/videoPanelScale", defaultSettings.appearance.videoPanelScale).toDouble();
	appearance.mapPanelScale = settings->value("appearance/mapPanelScale", defaultSettings.appearance.mapPanelScale).toDouble();
	appearance.videoPanelBackgroundColor = settings->value("appearance/videoPanelBackgroundColor", defaultSettings.appearance.videoPanelBackgroundColor).value<QColor>();
	appearance.mapPanelBackgroundColor = settings->value("appearance/mapPanelBackgroundColor", defaultSettings.appearance.mapPanelBackgroundColor).value<QColor>();
	appearance.videoPanelShader = settings->value("appearance/videoPanelShader", defaultSettings.appearance.videoPanelShader).toString();
	appearance.mapPanelShader = settings->value("appearance/mapPanelShader", defaultSettings.appearance.mapPanelShader).toString();

	videoStabilizer.enabled = settings->value("videoStabilizer/enabled", defaultSettings.videoStabilizer.enabled).toBool();
	videoStabilizer.frameSizeDivisor = settings->value("videoStabilizer/frameSizeDivisor", defaultSettings.videoStabilizer.frameSizeDivisor).toInt();
	videoStabilizer.averagingFactor = settings->value("videoStabilizer/averagingFactor", defaultSettings.videoStabilizer.averagingFactor).toDouble();
	videoStabilizer.dampingFactor = settings->value("videoStabilizer/dampingFactor", defaultSettings.videoStabilizer.dampingFactor).toDouble();
	videoStabilizer.maxDisplacementFactor = settings->value("videoStabilizer/maxDisplacementFactor", defaultSettings.videoStabilizer.maxDisplacementFactor).toDouble();
	videoStabilizer.enableClipping = settings->value("videoStabilizer/enableClipping", defaultSettings.videoStabilizer.enableClipping).toBool();
	videoStabilizer.enableClearing = settings->value("videoStabilizer/enableClearing", defaultSettings.videoStabilizer.enableClearing).toBool();
	videoStabilizer.inpaintBorderWidth = settings->value("videoStabilizer/inpaintBorderWidth", defaultSettings.videoStabilizer.inpaintBorderWidth).toInt();

	videoEncoder.outputVideoFilePath = settings->value("videoEncoder/outputVideoFilePath", defaultSettings.videoEncoder.outputVideoFilePath).toString();
	videoEncoder.preset = settings->value("videoEncoder/preset", defaultSettings.videoEncoder.preset).toString();
	videoEncoder.profile = settings->value("videoEncoder/profile", defaultSettings.videoEncoder.profile).toString();
	videoEncoder.constantRateFactor = settings->value("videoEncoder/constantRateFactor", defaultSettings.videoEncoder.constantRateFactor).toInt();

	inputHandler.smallSeekAmount = settings->value("inputHandler/smallSeekAmount", defaultSettings.inputHandler.smallSeekAmount).toInt();
	inputHandler.normalSeekAmount = settings->value("inputHandler/normalSeekAmount", defaultSettings.inputHandler.normalSeekAmount).toInt();
	inputHandler.largeSeekAmount = settings->value("inputHandler/largeSeekAmount", defaultSettings.inputHandler.largeSeekAmount).toInt();
	inputHandler.veryLargeSeekAmount = settings->value("inputHandler/veryLargeSeekAmount", defaultSettings.inputHandler.veryLargeSeekAmount).toInt();
	inputHandler.slowTranslateVelocity = settings->value("inputHandler/slowTranslateVelocity", defaultSettings.inputHandler.slowTranslateVelocity).toDouble();
	inputHandler.normalTranslateVelocity = settings->value("inputHandler/normalTranslateVelocity", defaultSettings.inputHandler.normalTranslateVelocity).toDouble();
	inputHandler.fastTranslateVelocity = settings->value("inputHandler/fastTranslateVelocity", defaultSettings.inputHandler.fastTranslateVelocity).toDouble();
	inputHandler.slowRotateVelocity = settings->value("inputHandler/slowRotateVelocity", defaultSettings.inputHandler.slowRotateVelocity).toDouble();
	inputHandler.normalRotateVelocity = settings->value("inputHandler/normalRotateVelocity", defaultSettings.inputHandler.normalRotateVelocity).toDouble();
	inputHandler.fastRotateVelocity = settings->value("inputHandler/fastRotateVelocity", defaultSettings.inputHandler.fastRotateVelocity).toDouble();
	inputHandler.smallScaleConstant = settings->value("inputHandler/smallScaleConstant", defaultSettings.inputHandler.smallScaleConstant).toDouble();
	inputHandler.normalScaleConstant = settings->value("inputHandler/normalScaleConstant", defaultSettings.inputHandler.normalScaleConstant).toDouble();
	inputHandler.largeScaleConstant = settings->value("inputHandler/largeScaleConstant", defaultSettings.inputHandler.largeScaleConstant).toDouble();
}

void Settings::writeToQSettings(QSettings* settings)
{
	qDebug("Writing settings to %s", qPrintable(settings->fileName()));

	settings->setValue("videoDecoder/inputVideoFilePath", videoDecoder.inputVideoFilePath);
	settings->setValue("videoDecoder/frameCountDivisor", videoDecoder.frameCountDivisor);
	settings->setValue("videoDecoder/frameDurationDivisor", videoDecoder.frameDurationDivisor);
	settings->setValue("videoDecoder/frameSizeDivisor", videoDecoder.frameSizeDivisor);
	settings->setValue("videoDecoder/enableVerboseLogging", videoDecoder.enableVerboseLogging);

	settings->setValue("mapAndRoute/quickRouteJpegFilePath", mapAndRoute.quickRouteJpegFilePath);
	settings->setValue("mapAndRoute/mapImageFilePath", mapAndRoute.mapImageFilePath);
	settings->setValue("mapAndRoute/routeStartOffset", mapAndRoute.routeStartOffset);

	settings->setValue("splitTimes/splitTimes", splitTimes.splitTimes);

	settings->setValue("window/width", window.width);
	settings->setValue("window/height", window.height);
	settings->setValue("window/multisamples", window.multisamples);
	settings->setValue("window/fullscreen", window.fullscreen);
	settings->setValue("window/hideCursor", window.hideCursor);

	settings->setValue("appearance/showInfoPanel", appearance.showInfoPanel);
	settings->setValue("appearance/mapPanelWidth", appearance.mapPanelWidth);
	settings->setValue("appearance/videoPanelScale", appearance.videoPanelScale);
	settings->setValue("appearance/mapPanelScale", appearance.mapPanelScale);
	settings->setValue("appearance/videoPanelBackgroundColor", appearance.videoPanelBackgroundColor);
	settings->setValue("appearance/mapPanelBackgroundColor", appearance.mapPanelBackgroundColor);
	settings->setValue("appearance/videoPanelShader", appearance.videoPanelShader);
	settings->setValue("appearance/mapPanelShader", appearance.mapPanelShader);

	settings->setValue("videoStabilizer/enabled", videoStabilizer.enabled);
	settings->setValue("videoStabilizer/frameSizeDivisor", videoStabilizer.frameSizeDivisor);
	settings->setValue("videoStabilizer/averagingFactor", videoStabilizer.averagingFactor);
	settings->setValue("videoStabilizer/dampingFactor", videoStabilizer.dampingFactor);
	settings->setValue("videoStabilizer/maxDisplacementFactor", videoStabilizer.maxDisplacementFactor);
	settings->setValue("videoStabilizer/enableClipping", videoStabilizer.enableClipping);
	settings->setValue("videoStabilizer/enableClearing", videoStabilizer.enableClearing);
	settings->setValue("videoStabilizer/inpaintBorderWidth", videoStabilizer.inpaintBorderWidth);

	settings->setValue("videoEncoder/outputVideoFilePath", videoEncoder.outputVideoFilePath);
	settings->setValue("videoEncoder/preset", videoEncoder.preset);
	settings->setValue("videoEncoder/profile", videoEncoder.profile);
	settings->setValue("videoEncoder/constantRateFactor", videoEncoder.constantRateFactor);

	settings->setValue("inputHandler/smallSeekAmount", inputHandler.smallSeekAmount);
	settings->setValue("inputHandler/normalSeekAmount", inputHandler.normalSeekAmount);
	settings->setValue("inputHandler/largeSeekAmount", inputHandler.largeSeekAmount);
	settings->setValue("inputHandler/veryLargeSeekAmount", inputHandler.veryLargeSeekAmount);
	settings->setValue("inputHandler/slowTranslateVelocity", inputHandler.slowTranslateVelocity);
	settings->setValue("inputHandler/normalTranslateVelocity", inputHandler.normalTranslateVelocity);
	settings->setValue("inputHandler/fastTranslateVelocity", inputHandler.fastTranslateVelocity);
	settings->setValue("inputHandler/slowRotateVelocity", inputHandler.slowRotateVelocity);
	settings->setValue("inputHandler/normalRotateVelocity", inputHandler.normalRotateVelocity);
	settings->setValue("inputHandler/fastRotateVelocity", inputHandler.fastRotateVelocity);
	settings->setValue("inputHandler/smallScaleConstant", inputHandler.smallScaleConstant);
	settings->setValue("inputHandler/normalScaleConstant", inputHandler.normalScaleConstant);
	settings->setValue("inputHandler/largeScaleConstant", inputHandler.largeScaleConstant);
}

void Settings::readFromUI(Ui::MainWindow* ui)
{
	videoDecoder.inputVideoFilePath = ui->lineEditInputVideoFile->text();
	videoDecoder.frameCountDivisor = ui->spinBoxVideoDecoderFrameCountDivisor->value();
	videoDecoder.frameDurationDivisor = ui->spinBoxVideoDecoderFrameDurationDivisor->value();
	videoDecoder.frameSizeDivisor = ui->spinBoxVideoDecoderFrameSizeDivisor->value();
	videoDecoder.enableVerboseLogging = ui->checkBoxVideoDecoderEnableVerboseLogging->isChecked();

	mapAndRoute.quickRouteJpegFilePath = ui->lineEditQuickRouteJpegFile->text();
	mapAndRoute.mapImageFilePath = ui->lineEditMapImageFile->text();
	mapAndRoute.routeStartOffset = ui->doubleSpinBoxRouteStartOffset->value();

	splitTimes.splitTimes = ui->lineEditSplitTimes->text();

	window.width = ui->spinBoxWindowWidth->value();
	window.height = ui->spinBoxWindowHeight->value();
	window.multisamples = ui->comboBoxWindowMultisamples->currentText().toInt();
	window.fullscreen = ui->checkBoxWindowFullscreen->isChecked();
	window.hideCursor = ui->checkBoxWindowHideCursor->isChecked();

	appearance.showInfoPanel = ui->checkBoxShowInfoPanel->isChecked();
	appearance.mapPanelWidth = ui->doubleSpinBoxMapPanelWidth->value();
	appearance.videoPanelScale = ui->doubleSpinBoxVideoPanelScale->value();
	appearance.mapPanelScale = ui->doubleSpinBoxMapPanelScale->value();
	appearance.videoPanelBackgroundColor = QColor(ui->lineEditBackgroundColorVideoPanel->text());
	appearance.mapPanelBackgroundColor = QColor(ui->lineEditBackgroundColorMapPanel->text());
	appearance.videoPanelShader = ui->comboBoxVideoPanelShader->currentText();
	appearance.mapPanelShader = ui->comboBoxMapPanelShader->currentText();

	videoStabilizer.enabled = ui->checkBoxVideoStabilizerEnabled->isChecked();
	videoStabilizer.frameSizeDivisor = ui->spinBoxVideoStabilizerFrameSizeDivisor->value();
	videoStabilizer.averagingFactor = ui->doubleSpinBoxVideoStabilizerAveragingFactor->value();
	videoStabilizer.dampingFactor = ui->doubleSpinBoxVideoStabilizerDampingFactor->value();
	videoStabilizer.maxDisplacementFactor = ui->doubleSpinBoxVideoStabilizerMaxDisplacementFactor->value();
	videoStabilizer.enableClipping = ui->checkBoxVideoStabilizerEnableClipping->isChecked();
	videoStabilizer.enableClearing = ui->checkBoxVideoStabilizerEnableClearing->isChecked();
	videoStabilizer.inpaintBorderWidth = ui->spinBoxVideoStabilizerInpaintBorderWidth->value();

	videoEncoder.outputVideoFilePath = ui->lineEditOutputVideoFile->text();
	videoEncoder.preset = ui->comboBoxVideoEncoderPreset->currentText();
	videoEncoder.profile = ui->comboBoxVideoEncoderProfile->currentText();
	videoEncoder.constantRateFactor = ui->spinBoxVideoEncoderCrf->value();
}

void Settings::writeToUI(Ui::MainWindow* ui)
{
	ui->lineEditInputVideoFile->setText(videoDecoder.inputVideoFilePath);
	ui->spinBoxVideoDecoderFrameCountDivisor->setValue(videoDecoder.frameCountDivisor);
	ui->spinBoxVideoDecoderFrameDurationDivisor->setValue(videoDecoder.frameDurationDivisor);
	ui->spinBoxVideoDecoderFrameSizeDivisor->setValue(videoDecoder.frameSizeDivisor);
	ui->checkBoxVideoDecoderEnableVerboseLogging->setChecked(videoDecoder.enableVerboseLogging);

	ui->lineEditQuickRouteJpegFile->setText(mapAndRoute.quickRouteJpegFilePath);
	ui->lineEditMapImageFile->setText(mapAndRoute.mapImageFilePath);
	ui->doubleSpinBoxRouteStartOffset->setValue(mapAndRoute.routeStartOffset);

	ui->lineEditSplitTimes->setText(splitTimes.splitTimes);

	ui->spinBoxWindowWidth->setValue(window.width);
	ui->spinBoxWindowHeight->setValue(window.height);
	ui->comboBoxWindowMultisamples->setCurrentText(QString::number(window.multisamples));
	ui->checkBoxWindowFullscreen->setChecked(window.fullscreen);
	ui->checkBoxWindowHideCursor->setChecked(window.hideCursor);

	ui->checkBoxShowInfoPanel->setChecked(appearance.showInfoPanel);
	ui->doubleSpinBoxMapPanelWidth->setValue(appearance.mapPanelWidth);
	ui->doubleSpinBoxVideoPanelScale->setValue(appearance.videoPanelScale);
	ui->doubleSpinBoxMapPanelScale->setValue(appearance.mapPanelScale);
	ui->lineEditBackgroundColorVideoPanel->setText(appearance.videoPanelBackgroundColor.name());
	ui->lineEditBackgroundColorMapPanel->setText(appearance.mapPanelBackgroundColor.name());
	ui->comboBoxVideoPanelShader->setCurrentText(appearance.videoPanelShader);
	ui->comboBoxMapPanelShader->setCurrentText(appearance.mapPanelShader);

	ui->checkBoxVideoStabilizerEnabled->setChecked(videoStabilizer.enabled);
	ui->spinBoxVideoStabilizerFrameSizeDivisor->setValue(videoStabilizer.frameSizeDivisor);
	ui->doubleSpinBoxVideoStabilizerAveragingFactor->setValue(videoStabilizer.averagingFactor);
	ui->doubleSpinBoxVideoStabilizerDampingFactor->setValue(videoStabilizer.dampingFactor);
	ui->doubleSpinBoxVideoStabilizerMaxDisplacementFactor->setValue(videoStabilizer.maxDisplacementFactor);
	ui->checkBoxVideoStabilizerEnableClipping->setChecked(videoStabilizer.enableClipping);
	ui->checkBoxVideoStabilizerEnableClearing->setChecked(videoStabilizer.enableClearing);
	ui->spinBoxVideoStabilizerInpaintBorderWidth->setValue(videoStabilizer.inpaintBorderWidth);

	ui->lineEditOutputVideoFile->setText(videoEncoder.outputVideoFilePath);
	ui->comboBoxVideoEncoderPreset->setCurrentText(videoEncoder.preset);
	ui->comboBoxVideoEncoderProfile->setCurrentText(videoEncoder.profile);
	ui->spinBoxVideoEncoderCrf->setValue(videoEncoder.constantRateFactor);
}
