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
	route.controlTimeOffset = settings->value("route/controlTimeOffset", defaultSettings.route.controlTimeOffset).toDouble();
	route.runnerTimeOffset = settings->value("route/runnerTimeOffset", defaultSettings.route.runnerTimeOffset).toDouble();
	route.scale = settings->value("route/scale", defaultSettings.route.scale).toDouble();
	route.topBottomMargin = settings->value("route/topBottomMargin", defaultSettings.route.topBottomMargin).toDouble();
	route.leftRightMargin = settings->value("route/leftRightMargin", defaultSettings.route.leftRightMargin).toDouble();
	route.minimumZoom = settings->value("route/minimumZoom", defaultSettings.route.minimumZoom).toDouble();
	route.maximumZoom = settings->value("route/maximumZoom", defaultSettings.route.maximumZoom).toDouble();
	route.lowPace = settings->value("route/lowPace", defaultSettings.route.lowPace).toDouble();
	route.highPace = settings->value("route/highPace", defaultSettings.route.highPace).toDouble();
	route.useSmoothTransition = settings->value("route/useSmoothTransition", defaultSettings.route.useSmoothTransition).toBool();
	route.smoothTransitionSpeed = settings->value("route/smoothTransitionSpeed", defaultSettings.route.smoothTransitionSpeed).toDouble();
	route.showRunner = settings->value("route/showRunner", defaultSettings.route.showRunner).toBool();
	route.showControls = settings->value("route/showControls", defaultSettings.route.showControls).toBool();
	route.wholeRouteRenderMode = (RouteRenderMode)settings->value("route/wholeRouteRenderMode", defaultSettings.route.wholeRouteRenderMode).toInt();
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
	inputHandler.slowTranslateSpeed = settings->value("inputHandler/slowTranslateSpeed", defaultSettings.inputHandler.slowTranslateSpeed).toDouble();
	inputHandler.normalTranslateSpeed = settings->value("inputHandler/normalTranslateSpeed", defaultSettings.inputHandler.normalTranslateSpeed).toDouble();
	inputHandler.fastTranslateSpeed = settings->value("inputHandler/fastTranslateSpeed", defaultSettings.inputHandler.fastTranslateSpeed).toDouble();
	inputHandler.veryFastTranslateSpeed = settings->value("inputHandler/veryFastTranslateSpeed", defaultSettings.inputHandler.fastTranslateSpeed).toDouble();
	inputHandler.slowRotateSpeed = settings->value("inputHandler/slowRotateSpeed", defaultSettings.inputHandler.slowRotateSpeed).toDouble();
	inputHandler.normalRotateSpeed = settings->value("inputHandler/normalRotateSpeed", defaultSettings.inputHandler.normalRotateSpeed).toDouble();
	inputHandler.fastRotateSpeed = settings->value("inputHandler/fastRotateSpeed", defaultSettings.inputHandler.fastRotateSpeed).toDouble();
	inputHandler.veryFastRotateSpeed = settings->value("inputHandler/veryFastRotateSpeed", defaultSettings.inputHandler.fastRotateSpeed).toDouble();
	inputHandler.slowScaleSpeed = settings->value("inputHandler/slowScaleSpeed", defaultSettings.inputHandler.slowScaleSpeed).toDouble();
	inputHandler.normalScaleSpeed = settings->value("inputHandler/normalScaleSpeed", defaultSettings.inputHandler.normalScaleSpeed).toDouble();
	inputHandler.fastScaleSpeed = settings->value("inputHandler/fastScaleSpeed", defaultSettings.inputHandler.fastScaleSpeed).toDouble();
	inputHandler.veryFastScaleSpeed = settings->value("inputHandler/veryFastScaleSpeed", defaultSettings.inputHandler.veryFastScaleSpeed).toDouble();
	inputHandler.smallTimeOffset = settings->value("inputHandler/smallTimeOffset", defaultSettings.inputHandler.smallTimeOffset).toDouble();
	inputHandler.normalTimeOffset = settings->value("inputHandler/normalTimeOffset", defaultSettings.inputHandler.normalTimeOffset).toDouble();
	inputHandler.largeTimeOffset = settings->value("inputHandler/largeTimeOffset", defaultSettings.inputHandler.largeTimeOffset).toDouble();
	inputHandler.veryLargeTimeOffset = settings->value("inputHandler/veryLargeTimeOffset", defaultSettings.inputHandler.veryLargeTimeOffset).toDouble();
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
	settings->setValue("route/controlTimeOffset", route.controlTimeOffset);
	settings->setValue("route/runnerTimeOffset", route.runnerTimeOffset);
	settings->setValue("route/scale", route.scale);
	settings->setValue("route/topBottomMargin", route.topBottomMargin);
	settings->setValue("route/leftRightMargin", route.leftRightMargin);
	settings->setValue("route/minimumZoom", route.minimumZoom);
	settings->setValue("route/maximumZoom", route.maximumZoom);
	settings->setValue("route/lowPace", route.lowPace);
	settings->setValue("route/highPace", route.highPace);
	settings->setValue("route/useSmoothTransition", route.useSmoothTransition);
	settings->setValue("route/smoothTransitionSpeed", route.smoothTransitionSpeed);
	settings->setValue("route/showRunner", route.showRunner);
	settings->setValue("route/showControls", route.showControls);
	settings->setValue("route/wholeRouteRenderMode", route.wholeRouteRenderMode);
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
	settings->setValue("inputHandler/slowTranslateSpeed", inputHandler.slowTranslateSpeed);
	settings->setValue("inputHandler/normalTranslateSpeed", inputHandler.normalTranslateSpeed);
	settings->setValue("inputHandler/fastTranslateSpeed", inputHandler.fastTranslateSpeed);
	settings->setValue("inputHandler/veryFastTranslateSpeed", inputHandler.veryFastTranslateSpeed);
	settings->setValue("inputHandler/slowRotateSpeed", inputHandler.slowRotateSpeed);
	settings->setValue("inputHandler/normalRotateSpeed", inputHandler.normalRotateSpeed);
	settings->setValue("inputHandler/fastRotateSpeed", inputHandler.fastRotateSpeed);
	settings->setValue("inputHandler/veryFastRotateSpeed", inputHandler.veryFastRotateSpeed);
	settings->setValue("inputHandler/slowScaleSpeed", inputHandler.slowScaleSpeed);
	settings->setValue("inputHandler/normalScaleSpeed", inputHandler.normalScaleSpeed);
	settings->setValue("inputHandler/fastScaleSpeed", inputHandler.fastScaleSpeed);
	settings->setValue("inputHandler/veryFastScaleSpeed", inputHandler.veryFastScaleSpeed);
	settings->setValue("inputHandler/smallTimeOffset", inputHandler.smallTimeOffset);
	settings->setValue("inputHandler/normalTimeOffset", inputHandler.normalTimeOffset);
	settings->setValue("inputHandler/largeTimeOffset", inputHandler.largeTimeOffset);
	settings->setValue("inputHandler/veryLargeTimeOffset", inputHandler.veryLargeTimeOffset);
}

void Settings::readFromUI(Ui::MainWindow* ui)
{
	map.imageFilePath = ui->lineEditMapImageFile->text();
	map.relativeWidth = ui->doubleSpinBoxMapWidth->value();
	map.scale = ui->doubleSpinBoxMapScale->value();
	map.backgroundColor = QColor(ui->lineEditMapBackgroundColor->text());
	map.rescaleShader = ui->comboBoxMapRescaleShader->currentText();

	route.quickRouteJpegFilePath = ui->lineEditQuickRouteJpegFile->text();
	route.controlTimeOffset = ui->doubleSpinBoxRouteControlTimeOffset->value();
	route.runnerTimeOffset = ui->doubleSpinBoxRouteRunnerTimeOffset->value();
	route.scale = ui->doubleSpinBoxRouteScale->value();
	route.topBottomMargin = ui->doubleSpinBoxRouteTopBottomMargin->value();
	route.leftRightMargin = ui->doubleSpinBoxRouteLeftRightMargin->value();

	video.inputVideoFilePath = ui->lineEditInputVideoFile->text();
	video.startTimeOffset = ui->doubleSpinBoxVideoStartTimeOffset->value();
	video.scale = ui->doubleSpinBoxVideoScale->value();
	video.backgroundColor = QColor(ui->lineEditVideoBackgroundColor->text());
	video.rescaleShader = ui->comboBoxVideoRescaleShader->currentText();
	video.enableClipping = ui->checkBoxVideoEnableClipping->isChecked();
	video.enableClearing = ui->checkBoxVideoEnableClearing->isChecked();
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
	ui->doubleSpinBoxMapWidth->setValue(map.relativeWidth);
	ui->doubleSpinBoxMapScale->setValue(map.scale);
	ui->lineEditMapBackgroundColor->setText(map.backgroundColor.name());
	ui->comboBoxMapRescaleShader->setCurrentText(map.rescaleShader);

	ui->lineEditQuickRouteJpegFile->setText(route.quickRouteJpegFilePath);
	ui->doubleSpinBoxRouteControlTimeOffset->setValue(route.controlTimeOffset);
	ui->doubleSpinBoxRouteRunnerTimeOffset->setValue(route.runnerTimeOffset);
	ui->doubleSpinBoxRouteScale->setValue(route.scale);
	ui->doubleSpinBoxRouteTopBottomMargin->setValue(route.topBottomMargin);
	ui->doubleSpinBoxRouteLeftRightMargin->setValue(route.leftRightMargin);

	ui->lineEditInputVideoFile->setText(video.inputVideoFilePath);
	ui->doubleSpinBoxVideoStartTimeOffset->setValue(video.startTimeOffset);
	ui->doubleSpinBoxVideoScale->setValue(video.scale);
	ui->lineEditVideoBackgroundColor->setText(video.backgroundColor.name());
	ui->comboBoxVideoRescaleShader->setCurrentText(video.rescaleShader);
	ui->checkBoxVideoEnableClipping->setChecked(video.enableClipping);
	ui->checkBoxVideoEnableClearing->setChecked(video.enableClearing);
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
