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

	map.imageFilePath = settings->value("map/imageFilePath", defaultSettings.map.imageFilePath).toString();
	map.relativeWidth = settings->value("map/relativeWidth", defaultSettings.map.relativeWidth).toDouble();
	map.x = settings->value("map/x", defaultSettings.map.x).toDouble();
	map.y = settings->value("map/y", defaultSettings.map.y).toDouble();
	map.angle = settings->value("map/angle", defaultSettings.map.angle).toDouble();
	map.scale = settings->value("map/scale", defaultSettings.map.scale).toDouble();
	map.backgroundColor = settings->value("map/backgroundColor", defaultSettings.map.backgroundColor).value<QColor>();
	map.rescaleShader = settings->value("map/rescaleShader", defaultSettings.map.rescaleShader).toString();

	route.quickRouteJpegFilePath = settings->value("route/quickRouteJpegFilePath", defaultSettings.route.quickRouteJpegFilePath).toString();
	route.controlsTimeOffset = settings->value("route/controlsTimeOffset", defaultSettings.route.controlsTimeOffset).toDouble();
	route.runnerTimeOffset = settings->value("route/runnerTimeOffset", defaultSettings.route.runnerTimeOffset).toDouble();
	route.scale = settings->value("route/scale", defaultSettings.route.scale).toDouble();
	route.highPace = settings->value("route/highPace", defaultSettings.route.highPace).toDouble();
	route.lowPace = settings->value("route/lowPace", defaultSettings.route.lowPace).toDouble();
	route.wholeRouteRenderMode = (RouteRenderMode)settings->value("route/wholeRouteRenderMode", defaultSettings.route.wholeRouteRenderMode).toInt();
	route.showRunner = settings->value("route/showRunner", defaultSettings.route.showRunner).toBool();
	route.showControls = settings->value("route/showControls", defaultSettings.route.showControls).toBool();
	route.wholeRouteColor = settings->value("route/wholeRouteColor", defaultSettings.route.wholeRouteColor).value<QColor>();
	route.wholeRouteWidth = settings->value("route/wholeRouteWidth", defaultSettings.route.wholeRouteWidth).toDouble();
	route.controlBorderColor = settings->value("route/controlBorderColor", defaultSettings.route.controlBorderColor).value<QColor>();
	route.controlRadius = settings->value("route/controlRadius", defaultSettings.route.controlRadius).toDouble();
	route.controlBorderWidth = settings->value("route/controlBorderWidth", defaultSettings.route.controlBorderWidth).toDouble();
	route.runnerColor = settings->value("route/runnerColor", defaultSettings.route.runnerColor).value<QColor>();
	route.runnerBorderColor = settings->value("route/runnerBorderColor", defaultSettings.route.runnerBorderColor).value<QColor>();
	route.runnerBorderWidth = settings->value("route/runnerBorderWidth", defaultSettings.route.runnerBorderWidth).toDouble();
	route.runnerScale = settings->value("route/runnerScale", defaultSettings.route.runnerScale).toDouble();
	
	video.inputVideoFilePath = settings->value("video/inputVideoFilePath", defaultSettings.video.inputVideoFilePath).toString();
	video.startTimeOffset = settings->value("video/startTimeOffset", defaultSettings.video.startTimeOffset).toDouble();
	video.seekToAnyFrame = settings->value("video/seekToAnyFrame", defaultSettings.video.seekToAnyFrame).toBool();
	video.x = settings->value("video/x", defaultSettings.video.x).toDouble();
	video.y = settings->value("video/y", defaultSettings.video.y).toDouble();
	video.angle = settings->value("video/angle", defaultSettings.video.angle).toDouble();
	video.scale = settings->value("video/scale", defaultSettings.video.scale).toDouble();
	video.backgroundColor = settings->value("video/backgroundColor", defaultSettings.video.backgroundColor).value<QColor>();
	video.rescaleShader = settings->value("video/rescaleShader", defaultSettings.video.rescaleShader).toString();
	video.enableClipping = settings->value("video/enableClipping", defaultSettings.video.enableClipping).toBool();
	video.enableClearing = settings->value("video/enableClearing", defaultSettings.video.enableClearing).toBool();
	video.frameCountDivisor = settings->value("video/frameCountDivisor", defaultSettings.video.frameCountDivisor).toInt();
	video.frameDurationDivisor = settings->value("video/frameDurationDivisor", defaultSettings.video.frameDurationDivisor).toInt();
	video.frameSizeDivisor = settings->value("video/frameSizeDivisor", defaultSettings.video.frameSizeDivisor).toInt();
	video.enableVerboseLogging = settings->value("video/enableVerboseLogging", defaultSettings.video.enableVerboseLogging).toBool();

	splits.type = (SplitTimeType)settings->value("splits/type", defaultSettings.splits.type).toInt();
	splits.splitTimes = settings->value("splits/splitTimes", defaultSettings.splits.splitTimes).toString();

	window.width = settings->value("window/width", defaultSettings.window.width).toInt();
	window.height = settings->value("window/height", defaultSettings.window.height).toInt();
	window.multisamples = settings->value("window/multisamples", defaultSettings.window.multisamples).toInt();
	window.fullscreen = settings->value("window/fullscreen", defaultSettings.window.fullscreen).toBool();
	window.hideCursor = settings->value("window/hideCursor", defaultSettings.window.hideCursor).toBool();
	window.showInfoPanel = settings->value("window/showInfoPanel", defaultSettings.window.showInfoPanel).toBool();
	
	stabilizer.enabled = settings->value("stabilizer/enabled", defaultSettings.stabilizer.enabled).toBool();
	stabilizer.mode = (VideoStabilizerMode)settings->value("stabilizer/mode", defaultSettings.stabilizer.mode).toInt();
	stabilizer.inputDataFilePath = settings->value("stabilizer/inputDataFilePath", defaultSettings.stabilizer.inputDataFilePath).toString();
	stabilizer.averagingFactor = settings->value("stabilizer/averagingFactor", defaultSettings.stabilizer.averagingFactor).toDouble();
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

	inputHandler.smallSeekAmount = settings->value("inputHandler/smallSeekAmount", defaultSettings.inputHandler.smallSeekAmount).toDouble();
	inputHandler.normalSeekAmount = settings->value("inputHandler/normalSeekAmount", defaultSettings.inputHandler.normalSeekAmount).toDouble();
	inputHandler.largeSeekAmount = settings->value("inputHandler/largeSeekAmount", defaultSettings.inputHandler.largeSeekAmount).toDouble();
	inputHandler.veryLargeSeekAmount = settings->value("inputHandler/veryLargeSeekAmount", defaultSettings.inputHandler.veryLargeSeekAmount).toDouble();
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

	settings->setValue("map/imageFilePath", map.imageFilePath);
	settings->setValue("map/relativeWidth", map.relativeWidth);
	settings->setValue("map/x", map.x);
	settings->setValue("map/y", map.y);
	settings->setValue("map/angle", map.angle);
	settings->setValue("map/scale", map.scale);
	settings->setValue("map/backgroundColor", map.backgroundColor);
	settings->setValue("map/rescaleShader", map.rescaleShader);

	settings->setValue("route/quickRouteJpegFilePath", route.quickRouteJpegFilePath);
	settings->setValue("route/controlsTimeOffset", route.controlsTimeOffset);
	settings->setValue("route/runnerTimeOffset", route.runnerTimeOffset);
	settings->setValue("route/scale", route.scale);
	settings->setValue("route/highPace", route.highPace);
	settings->setValue("route/lowPace", route.lowPace);
	settings->setValue("route/wholeRouteRenderMode", route.wholeRouteRenderMode);
	settings->setValue("route/showRunner", route.showRunner);
	settings->setValue("route/showControls", route.showControls);
	settings->setValue("route/wholeRouteColor", route.wholeRouteColor);
	settings->setValue("route/wholeRouteWidth", route.wholeRouteWidth);
	settings->setValue("route/controlBorderColor", route.controlBorderColor);
	settings->setValue("route/controlRadius", route.controlRadius);
	settings->setValue("route/controlBorderWidth", route.controlBorderWidth);
	settings->setValue("route/runnerColor", route.runnerColor);
	settings->setValue("route/runnerBorderColor", route.runnerBorderColor);
	settings->setValue("route/runnerBorderWidth", route.runnerBorderWidth);
	settings->setValue("route/runnerScale", route.runnerScale);

	settings->setValue("video/inputVideoFilePath", video.inputVideoFilePath);
	settings->setValue("video/startTimeOffset", video.startTimeOffset);
	settings->setValue("video/seekToAnyFrame", video.seekToAnyFrame);
	settings->setValue("video/x", video.x);
	settings->setValue("video/y", video.y);
	settings->setValue("video/angle", video.angle);
	settings->setValue("video/scale", video.scale);
	settings->setValue("video/backgroundColor", video.backgroundColor);
	settings->setValue("video/rescaleShader", video.rescaleShader);
	settings->setValue("video/enableClipping", video.enableClipping);
	settings->setValue("video/enableClearing", video.enableClearing);
	settings->setValue("video/frameCountDivisor", video.frameCountDivisor);
	settings->setValue("video/frameDurationDivisor", video.frameDurationDivisor);
	settings->setValue("video/frameSizeDivisor", video.frameSizeDivisor);
	settings->setValue("video/enableVerboseLogging", video.enableVerboseLogging);

	settings->setValue("splits/type", splits.type);
	settings->setValue("splits/splitTimes", splits.splitTimes);

	settings->setValue("window/width", window.width);
	settings->setValue("window/height", window.height);
	settings->setValue("window/multisamples", window.multisamples);
	settings->setValue("window/fullscreen", window.fullscreen);
	settings->setValue("window/hideCursor", window.hideCursor);
	settings->setValue("window/showInfoPanel", window.showInfoPanel);

	settings->setValue("stabilizer/enabled", stabilizer.enabled);
	settings->setValue("stabilizer/mode", stabilizer.mode);
	settings->setValue("stabilizer/inputDataFilePath", stabilizer.inputDataFilePath);
	settings->setValue("stabilizer/averagingFactor", stabilizer.averagingFactor);
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
	map.imageFilePath = ui->lineEditMapImageFile->text();
	map.relativeWidth = ui->doubleSpinBoxMapPanelWidth->value();
	map.scale = ui->doubleSpinBoxMapPanelScale->value();
	map.backgroundColor = QColor(ui->lineEditMapPanelBackgroundColor->text());
	map.rescaleShader = ui->comboBoxMapPanelRescaleShader->currentText();

	route.quickRouteJpegFilePath = ui->lineEditQuickRouteJpegFile->text();
	route.controlsTimeOffset = ui->doubleSpinBoxRouteControlsTimeOffset->value();
	route.runnerTimeOffset = ui->doubleSpinBoxRouteRunnerTimeOffset->value();
	route.scale = ui->doubleSpinBoxRouteScale->value();
	
	video.inputVideoFilePath = ui->lineEditInputVideoFile->text();
	video.startTimeOffset = ui->doubleSpinBoxVideoStartTimeOffset->value();
	video.scale = ui->doubleSpinBoxVideoPanelScale->value();
	video.backgroundColor = QColor(ui->lineEditVideoPanelBackgroundColor->text());
	video.rescaleShader = ui->comboBoxVideoPanelRescaleShader->currentText();
	video.enableClipping = ui->checkBoxVideoPanelEnableClipping->isChecked();
	video.enableClearing = ui->checkBoxVideoPanelEnableClearing->isChecked();
	video.frameCountDivisor = ui->spinBoxVideoDecoderFrameCountDivisor->value();
	video.frameDurationDivisor = ui->spinBoxVideoDecoderFrameDurationDivisor->value();
	video.frameSizeDivisor = ui->spinBoxVideoDecoderFrameSizeDivisor->value();
	video.enableVerboseLogging = ui->checkBoxVideoDecoderEnableVerboseLogging->isChecked();

	splits.type = (SplitTimeType)ui->comboBoxSplitTimeType->currentIndex();
	splits.splitTimes = ui->lineEditSplitTimes->text();

	window.width = ui->spinBoxWindowWidth->value();
	window.height = ui->spinBoxWindowHeight->value();
	window.multisamples = ui->comboBoxWindowMultisamples->currentText().toInt();
	window.fullscreen = ui->checkBoxWindowFullscreen->isChecked();
	window.hideCursor = ui->checkBoxWindowHideCursor->isChecked();
	window.showInfoPanel = ui->checkBoxWindowShowInfoPanel->isChecked();

	stabilizer.enabled = ui->checkBoxVideoStabilizerEnabled->isChecked();
	stabilizer.mode = (VideoStabilizerMode)ui->comboBoxVideoStabilizerMode->currentIndex();
	stabilizer.inputDataFilePath = ui->lineEditVideoStabilizerInputDataFile->text();
	stabilizer.averagingFactor = ui->doubleSpinBoxVideoStabilizerAveragingFactor->value();
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
	ui->lineEditMapImageFile->setText(map.imageFilePath);
	ui->doubleSpinBoxMapPanelWidth->setValue(map.relativeWidth);
	ui->doubleSpinBoxMapPanelScale->setValue(map.scale);
	ui->lineEditMapPanelBackgroundColor->setText(map.backgroundColor.name());
	ui->comboBoxMapPanelRescaleShader->setCurrentText(map.rescaleShader);

	ui->lineEditQuickRouteJpegFile->setText(route.quickRouteJpegFilePath);
	ui->doubleSpinBoxRouteControlsTimeOffset->setValue(route.controlsTimeOffset);
	ui->doubleSpinBoxRouteRunnerTimeOffset->setValue(route.runnerTimeOffset);
	ui->doubleSpinBoxRouteScale->setValue(route.scale);

	ui->lineEditInputVideoFile->setText(video.inputVideoFilePath);
	ui->doubleSpinBoxVideoStartTimeOffset->setValue(video.startTimeOffset);
	ui->doubleSpinBoxVideoPanelScale->setValue(video.scale);
	ui->lineEditVideoPanelBackgroundColor->setText(video.backgroundColor.name());
	ui->comboBoxVideoPanelRescaleShader->setCurrentText(video.rescaleShader);
	ui->checkBoxVideoPanelEnableClipping->setChecked(video.enableClipping);
	ui->checkBoxVideoPanelEnableClearing->setChecked(video.enableClearing);
	ui->spinBoxVideoDecoderFrameCountDivisor->setValue(video.frameCountDivisor);
	ui->spinBoxVideoDecoderFrameDurationDivisor->setValue(video.frameDurationDivisor);
	ui->spinBoxVideoDecoderFrameSizeDivisor->setValue(video.frameSizeDivisor);
	ui->checkBoxVideoDecoderEnableVerboseLogging->setChecked(video.enableVerboseLogging);

	ui->comboBoxSplitTimeType->setCurrentIndex(splits.type);
	ui->lineEditSplitTimes->setText(splits.splitTimes);

	ui->spinBoxWindowWidth->setValue(window.width);
	ui->spinBoxWindowHeight->setValue(window.height);
	ui->comboBoxWindowMultisamples->setCurrentText(QString::number(window.multisamples));
	ui->checkBoxWindowFullscreen->setChecked(window.fullscreen);
	ui->checkBoxWindowHideCursor->setChecked(window.hideCursor);
	ui->checkBoxWindowShowInfoPanel->setChecked(window.showInfoPanel);

	ui->checkBoxVideoStabilizerEnabled->setChecked(stabilizer.enabled);
	ui->comboBoxVideoStabilizerMode->setCurrentIndex(stabilizer.mode);
	ui->lineEditVideoStabilizerInputDataFile->setText(stabilizer.inputDataFilePath);
	ui->doubleSpinBoxVideoStabilizerAveragingFactor->setValue(stabilizer.averagingFactor);
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
