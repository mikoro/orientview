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

	map.mapImageFilePath = settings->value("map/mapImageFilePath", defaultSettings.map.mapImageFilePath).toString();
	map.mapPanelWidth = settings->value("map/mapPanelWidth", defaultSettings.map.mapPanelWidth).toDouble();
	map.mapPanelScale = settings->value("map/mapPanelScale", defaultSettings.map.mapPanelScale).toDouble();
	map.mapPanelBackgroundColor = settings->value("map/mapPanelBackgroundColor", defaultSettings.map.mapPanelBackgroundColor).value<QColor>();
	map.mapPanelRescaleShader = settings->value("map/mapPanelRescaleShader", defaultSettings.map.mapPanelRescaleShader).toString();

	route.quickRouteJpegFilePath = settings->value("route/quickRouteJpegFilePath", defaultSettings.route.quickRouteJpegFilePath).toString();
	route.startOffset = settings->value("route/startOffset", defaultSettings.route.startOffset).toDouble();

	splits.type = (SplitTimeType)settings->value("splits/type", defaultSettings.splits.type).toInt();
	splits.splitTimes = settings->value("splits/splitTimes", defaultSettings.splits.splitTimes).toString();
	
	video.inputVideoFilePath = settings->value("video/inputVideoFilePath", defaultSettings.video.inputVideoFilePath).toString();
	video.startOffset = settings->value("video/startOffset", defaultSettings.video.startOffset).toDouble();
	video.videoPanelScale = settings->value("video/videoPanelScale", defaultSettings.video.videoPanelScale).toDouble();
	video.videoPanelBackgroundColor = settings->value("video/videoPanelBackgroundColor", defaultSettings.video.videoPanelBackgroundColor).value<QColor>();
	video.videoPanelRescaleShader = settings->value("video/videoPanelRescaleShader", defaultSettings.video.videoPanelRescaleShader).toString();
	video.frameCountDivisor = settings->value("video/frameCountDivisor", defaultSettings.video.frameCountDivisor).toInt();
	video.frameDurationDivisor = settings->value("video/frameDurationDivisor", defaultSettings.video.frameDurationDivisor).toInt();
	video.frameSizeDivisor = settings->value("video/frameSizeDivisor", defaultSettings.video.frameSizeDivisor).toInt();
	video.enableVerboseLogging = settings->value("video/enableVerboseLogging", defaultSettings.video.enableVerboseLogging).toBool();

	window.width = settings->value("window/width", defaultSettings.window.width).toInt();
	window.height = settings->value("window/height", defaultSettings.window.height).toInt();
	window.multisamples = settings->value("window/multisamples", defaultSettings.window.multisamples).toInt();
	window.fullscreen = settings->value("window/fullscreen", defaultSettings.window.fullscreen).toBool();
	window.hideCursor = settings->value("window/hideCursor", defaultSettings.window.hideCursor).toBool();
	window.showInfoPanel = settings->value("window/showInfoPanel", defaultSettings.window.showInfoPanel).toBool();
	
	stabilizer.enabled = settings->value("stabilizer/enabled", defaultSettings.stabilizer.enabled).toBool();
	stabilizer.mode = (VideoStabilizerMode)settings->value("stabilizer/mode", defaultSettings.stabilizer.mode).toInt();
	stabilizer.inputDataFilePath = settings->value("stabilizer/inputDataFilePath", defaultSettings.stabilizer.inputDataFilePath).toString();
	stabilizer.enableClipping = settings->value("stabilizer/enableClipping", defaultSettings.stabilizer.enableClipping).toBool();
	stabilizer.enableClearing = settings->value("stabilizer/enableClearing", defaultSettings.stabilizer.enableClearing).toBool();
	stabilizer.dampingFactor = settings->value("stabilizer/dampingFactor", defaultSettings.stabilizer.dampingFactor).toDouble();
	stabilizer.maxDisplacementFactor = settings->value("stabilizer/maxDisplacementFactor", defaultSettings.stabilizer.maxDisplacementFactor).toDouble();
	stabilizer.frameSizeDivisor = settings->value("stabilizer/frameSizeDivisor", defaultSettings.stabilizer.frameSizeDivisor).toInt();
	stabilizer.passOneOutputFilePath = settings->value("stabilizer/passOneOutputFilePath", defaultSettings.stabilizer.passOneOutputFilePath).toString();
	stabilizer.passTwoInputFilePath = settings->value("stabilizer/passTwoInputFilePath", defaultSettings.stabilizer.passTwoInputFilePath).toString();
	stabilizer.passTwoOutputFilePath = settings->value("stabilizer/passTwoOutputFilePath", defaultSettings.stabilizer.passTwoOutputFilePath).toString();
	stabilizer.smoothingRadius = settings->value("stabilizer/smoothingRadius", defaultSettings.stabilizer.smoothingRadius).toInt();

	encoder.outputVideoFilePath = settings->value("encoder/outputVideoFilePath", defaultSettings.encoder.outputVideoFilePath).toString();
	encoder.preset = settings->value("encoder/preset", defaultSettings.encoder.preset).toString();
	encoder.profile = settings->value("encoder/profile", defaultSettings.encoder.profile).toString();
	encoder.constantRateFactor = settings->value("encoder/constantRateFactor", defaultSettings.encoder.constantRateFactor).toInt();

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

	settings->setValue("map/mapImageFilePath", map.mapImageFilePath);
	settings->setValue("map/mapPanelWidth", map.mapPanelWidth);
	settings->setValue("map/mapPanelScale", map.mapPanelScale);
	settings->setValue("map/mapPanelBackgroundColor", map.mapPanelBackgroundColor);
	settings->setValue("map/mapPanelRescaleShader", map.mapPanelRescaleShader);

	settings->setValue("route/quickRouteJpegFilePath", route.quickRouteJpegFilePath);
	settings->setValue("route/startOffset", route.startOffset);

	settings->setValue("splits/type", splits.type);
	settings->setValue("splits/splitTimes", splits.splitTimes);

	settings->setValue("video/inputVideoFilePath", video.inputVideoFilePath);
	settings->setValue("video/startOffset", video.startOffset);
	settings->setValue("video/videoPanelScale", video.videoPanelScale);
	settings->setValue("video/videoPanelBackgroundColor", video.videoPanelBackgroundColor);
	settings->setValue("video/videoPanelRescaleShader", video.videoPanelRescaleShader);
	settings->setValue("video/frameCountDivisor", video.frameCountDivisor);
	settings->setValue("video/frameDurationDivisor", video.frameDurationDivisor);
	settings->setValue("video/frameSizeDivisor", video.frameSizeDivisor);
	settings->setValue("video/enableVerboseLogging", video.enableVerboseLogging);

	settings->setValue("window/width", window.width);
	settings->setValue("window/height", window.height);
	settings->setValue("window/multisamples", window.multisamples);
	settings->setValue("window/fullscreen", window.fullscreen);
	settings->setValue("window/hideCursor", window.hideCursor);
	settings->setValue("window/showInfoPanel", window.showInfoPanel);

	settings->setValue("stabilizer/enabled", stabilizer.enabled);
	settings->setValue("stabilizer/mode", stabilizer.mode);
	settings->setValue("stabilizer/inputDataFilePath", stabilizer.inputDataFilePath);
	settings->setValue("stabilizer/enableClipping", stabilizer.enableClipping);
	settings->setValue("stabilizer/enableClearing", stabilizer.enableClearing);
	settings->setValue("stabilizer/dampingFactor", stabilizer.dampingFactor);
	settings->setValue("stabilizer/maxDisplacementFactor", stabilizer.maxDisplacementFactor);
	settings->setValue("stabilizer/frameSizeDivisor", stabilizer.frameSizeDivisor);
	settings->setValue("stabilizer/passOneOutputFilePath", stabilizer.passOneOutputFilePath);
	settings->setValue("stabilizer/passTwoInputFilePath", stabilizer.passTwoInputFilePath);
	settings->setValue("stabilizer/passTwoOutputFilePath", stabilizer.passTwoOutputFilePath);
	settings->setValue("stabilizer/smoothingRadius", stabilizer.smoothingRadius);

	settings->setValue("encoder/outputVideoFilePath", encoder.outputVideoFilePath);
	settings->setValue("encoder/preset", encoder.preset);
	settings->setValue("encoder/profile", encoder.profile);
	settings->setValue("encoder/constantRateFactor", encoder.constantRateFactor);

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
	map.mapImageFilePath = ui->lineEditMapImageFile->text();
	map.mapPanelWidth = ui->doubleSpinBoxMapPanelWidth->value();
	map.mapPanelScale = ui->doubleSpinBoxMapPanelScale->value();
	map.mapPanelBackgroundColor = QColor(ui->lineEditMapPanelBackgroundColor->text());
	map.mapPanelRescaleShader = ui->comboBoxMapPanelRescaleShader->currentText();

	route.quickRouteJpegFilePath = ui->lineEditQuickRouteJpegFile->text();
	route.startOffset = ui->doubleSpinBoxRouteStartOffset->value();

	splits.type = (SplitTimeType)ui->comboBoxSplitTimeType->currentIndex();
	splits.splitTimes = ui->lineEditSplitTimes->text();
	
	video.inputVideoFilePath = ui->lineEditInputVideoFile->text();
	video.startOffset = ui->doubleSpinBoxVideoStartOffset->value();
	video.videoPanelScale = ui->doubleSpinBoxVideoPanelScale->value();
	video.videoPanelBackgroundColor = QColor(ui->lineEditVideoPanelBackgroundColor->text());
	video.videoPanelRescaleShader = ui->comboBoxVideoPanelRescaleShader->currentText();
	video.frameCountDivisor = ui->spinBoxVideoDecoderFrameCountDivisor->value();
	video.frameDurationDivisor = ui->spinBoxVideoDecoderFrameDurationDivisor->value();
	video.frameSizeDivisor = ui->spinBoxVideoDecoderFrameSizeDivisor->value();
	video.enableVerboseLogging = ui->checkBoxVideoDecoderEnableVerboseLogging->isChecked();

	window.width = ui->spinBoxWindowWidth->value();
	window.height = ui->spinBoxWindowHeight->value();
	window.multisamples = ui->comboBoxWindowMultisamples->currentText().toInt();
	window.fullscreen = ui->checkBoxWindowFullscreen->isChecked();
	window.hideCursor = ui->checkBoxWindowHideCursor->isChecked();
	window.showInfoPanel = ui->checkBoxWindowShowInfoPanel->isChecked();

	stabilizer.enabled = ui->checkBoxVideoStabilizerEnabled->isChecked();
	stabilizer.mode = (VideoStabilizerMode)ui->comboBoxVideoStabilizerMode->currentIndex();
	stabilizer.inputDataFilePath = ui->lineEditVideoStabilizerInputDataFile->text();
	stabilizer.enableClipping = ui->checkBoxVideoStabilizerEnableClipping->isChecked();
	stabilizer.enableClearing = ui->checkBoxVideoStabilizerEnableClearing->isChecked();
	stabilizer.dampingFactor = ui->doubleSpinBoxVideoStabilizerDampingFactor->value();
	stabilizer.maxDisplacementFactor = ui->doubleSpinBoxVideoStabilizerMaxDisplacementFactor->value();
	stabilizer.frameSizeDivisor = ui->spinBoxVideoStabilizerFrameSizeDivisor->value();
	stabilizer.passOneOutputFilePath = ui->lineEditVideoStabilizerPassOneOutputFile->text();
	stabilizer.passTwoInputFilePath = ui->lineEditVideoStabilizerPassTwoInputFile->text();
	stabilizer.passTwoOutputFilePath = ui->lineEditVideoStabilizerPassTwoOutputFile->text();
	stabilizer.smoothingRadius = ui->spinBoxVideoStabilizerSmoothingRadius->value();

	encoder.outputVideoFilePath = ui->lineEditOutputVideoFile->text();
	encoder.preset = ui->comboBoxVideoEncoderPreset->currentText();
	encoder.profile = ui->comboBoxVideoEncoderProfile->currentText();
	encoder.constantRateFactor = ui->spinBoxVideoEncoderCrf->value();
}

void Settings::writeToUI(Ui::MainWindow* ui)
{
	ui->lineEditMapImageFile->setText(map.mapImageFilePath);
	ui->doubleSpinBoxMapPanelWidth->setValue(map.mapPanelWidth);
	ui->doubleSpinBoxMapPanelScale->setValue(map.mapPanelScale);
	ui->lineEditMapPanelBackgroundColor->setText(map.mapPanelBackgroundColor.name());
	ui->comboBoxMapPanelRescaleShader->setCurrentText(map.mapPanelRescaleShader);

	ui->lineEditQuickRouteJpegFile->setText(route.quickRouteJpegFilePath);
	ui->doubleSpinBoxRouteStartOffset->setValue(route.startOffset);

	ui->comboBoxSplitTimeType->setCurrentIndex(splits.type);
	ui->lineEditSplitTimes->setText(splits.splitTimes);

	ui->lineEditInputVideoFile->setText(video.inputVideoFilePath);
	ui->doubleSpinBoxVideoStartOffset->setValue(video.startOffset);
	ui->doubleSpinBoxVideoPanelScale->setValue(video.videoPanelScale);
	ui->lineEditVideoPanelBackgroundColor->setText(video.videoPanelBackgroundColor.name());
	ui->comboBoxVideoPanelRescaleShader->setCurrentText(video.videoPanelRescaleShader);
	ui->spinBoxVideoDecoderFrameCountDivisor->setValue(video.frameCountDivisor);
	ui->spinBoxVideoDecoderFrameDurationDivisor->setValue(video.frameDurationDivisor);
	ui->spinBoxVideoDecoderFrameSizeDivisor->setValue(video.frameSizeDivisor);
	ui->checkBoxVideoDecoderEnableVerboseLogging->setChecked(video.enableVerboseLogging);

	ui->spinBoxWindowWidth->setValue(window.width);
	ui->spinBoxWindowHeight->setValue(window.height);
	ui->comboBoxWindowMultisamples->setCurrentText(QString::number(window.multisamples));
	ui->checkBoxWindowFullscreen->setChecked(window.fullscreen);
	ui->checkBoxWindowHideCursor->setChecked(window.hideCursor);
	ui->checkBoxWindowShowInfoPanel->setChecked(window.showInfoPanel);

	ui->checkBoxVideoStabilizerEnabled->setChecked(stabilizer.enabled);
	ui->comboBoxVideoStabilizerMode->setCurrentIndex(stabilizer.mode);
	ui->lineEditVideoStabilizerInputDataFile->setText(stabilizer.inputDataFilePath);
	ui->checkBoxVideoStabilizerEnableClipping->setChecked(stabilizer.enableClipping);
	ui->checkBoxVideoStabilizerEnableClearing->setChecked(stabilizer.enableClearing);
	ui->doubleSpinBoxVideoStabilizerDampingFactor->setValue(stabilizer.dampingFactor);
	ui->doubleSpinBoxVideoStabilizerMaxDisplacementFactor->setValue(stabilizer.maxDisplacementFactor);
	ui->spinBoxVideoStabilizerFrameSizeDivisor->setValue(stabilizer.frameSizeDivisor);
	ui->lineEditVideoStabilizerPassOneOutputFile->setText(stabilizer.passOneOutputFilePath);
	ui->lineEditVideoStabilizerPassTwoInputFile->setText(stabilizer.passTwoInputFilePath);
	ui->lineEditVideoStabilizerPassTwoOutputFile->setText(stabilizer.passTwoOutputFilePath);
	ui->spinBoxVideoStabilizerSmoothingRadius->setValue(stabilizer.smoothingRadius);

	ui->lineEditOutputVideoFile->setText(encoder.outputVideoFilePath);
	ui->comboBoxVideoEncoderPreset->setCurrentText(encoder.preset);
	ui->comboBoxVideoEncoderProfile->setCurrentText(encoder.profile);
	ui->spinBoxVideoEncoderCrf->setValue(encoder.constantRateFactor);
}
