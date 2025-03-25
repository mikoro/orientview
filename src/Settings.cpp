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
	map.headerCrop = settings->value("map/headerCrop", defaultSettings.map.headerCrop).toInt();
	map.rescaleShader = settings->value("map/rescaleShader", defaultSettings.map.rescaleShader).toString();

	route.quickRouteJpegFilePath = settings->value("route/quickRouteJpegFilePath", defaultSettings.route.quickRouteJpegFilePath).toString();
	route.discreetColor = settings->value("route/discreetColor", defaultSettings.route.discreetColor).value<QColor>();
	route.highlightColor = settings->value("route/highlightColor", defaultSettings.route.highlightColor).value<QColor>();
	route.routeRenderMode = (RouteRenderMode)settings->value("route/routeRenderMode", defaultSettings.route.routeRenderMode).toInt();
	route.routeWidth = settings->value("route/routeWidth", defaultSettings.route.routeWidth).toDouble();
	route.tailRenderMode = (RouteRenderMode)settings->value("route/tailRenderMode", defaultSettings.route.tailRenderMode).toInt();
	route.tailWidth = settings->value("route/tailWidth", defaultSettings.route.tailWidth).toDouble();
	route.tailLength = settings->value("route/tailLength", defaultSettings.route.tailLength).toDouble();
	route.controlBorderColor = settings->value("route/controlBorderColor", defaultSettings.route.controlBorderColor).value<QColor>();
	route.controlRadius = settings->value("route/controlRadius", defaultSettings.route.controlRadius).toDouble();
	route.controlBorderWidth = settings->value("route/controlBorderWidth", defaultSettings.route.controlBorderWidth).toDouble();
	route.showControls = settings->value("route/showControls", defaultSettings.route.showControls).toBool();
	route.runnerColor = settings->value("route/runnerColor", defaultSettings.route.runnerColor).value<QColor>();
	route.runnerBorderColor = settings->value("route/runnerBorderColor", defaultSettings.route.runnerBorderColor).value<QColor>();
	route.runnerRadius = settings->value("route/runnerRadius", defaultSettings.route.runnerRadius).toDouble();
	route.runnerBorderWidth = settings->value("route/runnerBorderWidth", defaultSettings.route.runnerBorderWidth).toDouble();
	route.runnerScale = settings->value("route/runnerScale", defaultSettings.route.runnerScale).toDouble();
	route.showRunner = settings->value("route/showRunner", defaultSettings.route.showRunner).toBool();
	route.controlTimeOffset = settings->value("route/controlTimeOffset", defaultSettings.route.controlTimeOffset).toDouble();
	route.runnerTimeOffset = settings->value("route/runnerTimeOffset", defaultSettings.route.runnerTimeOffset).toDouble();
	route.scale = settings->value("route/scale", defaultSettings.route.scale).toDouble();
	route.lowPace = settings->value("route/lowPace", defaultSettings.route.lowPace).toDouble();
	route.highPace = settings->value("route/highPace", defaultSettings.route.highPace).toDouble();
	
	routeManager.viewMode = (ViewMode)settings->value("routeManager/viewMode", defaultSettings.routeManager.viewMode).toInt();
	routeManager.useSmoothSplitTransition = settings->value("routeManager/useSmoothSplitTransition", defaultSettings.routeManager.useSmoothSplitTransition).toBool();
	routeManager.smoothSplitTransitionSpeed = settings->value("routeManager/smoothSplitTransitionSpeed", defaultSettings.routeManager.smoothSplitTransitionSpeed).toDouble();
	routeManager.topBottomMargin = settings->value("routeManager/topBottomMargin", defaultSettings.routeManager.topBottomMargin).toDouble();
	routeManager.leftRightMargin = settings->value("routeManager/leftRightMargin", defaultSettings.routeManager.leftRightMargin).toDouble();
	routeManager.maximumAutomaticZoom = settings->value("routeManager/maximumAutomaticZoom", defaultSettings.routeManager.maximumAutomaticZoom).toDouble();
	routeManager.runnerAveragingFactor = settings->value("routeManager/runnerAveragingFactor", defaultSettings.routeManager.runnerAveragingFactor).toDouble();
	routeManager.runnerVerticalOffset = settings->value("routeManager/runnerVerticalOffset", defaultSettings.routeManager.runnerVerticalOffset).toDouble();

	video.inputVideoFilePath = settings->value("video/inputVideoFilePath", defaultSettings.video.inputVideoFilePath).toString();
	video.startTimeOffset = settings->value("video/startTimeOffset", defaultSettings.video.startTimeOffset).toDouble();
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
	video.seekToAnyFrame = settings->value("video/seekToAnyFrame", defaultSettings.video.seekToAnyFrame).toBool();

	splits.type = (SplitTimeType)settings->value("splits/type", defaultSettings.splits.type).toInt();
	splits.splitTimes = settings->value("splits/splitTimes", defaultSettings.splits.splitTimes).toString();

	window.width = settings->value("window/width", defaultSettings.window.width).toInt();
	window.height = settings->value("window/height", defaultSettings.window.height).toInt();
	window.multisamples = settings->value("window/multisamples", defaultSettings.window.multisamples).toInt();
	window.fullscreen = settings->value("window/fullscreen", defaultSettings.window.fullscreen).toBool();
	window.hideCursor = settings->value("window/hideCursor", defaultSettings.window.hideCursor).toBool();

	renderer.renderMode = (RenderMode)settings->value("renderer/renderMode", defaultSettings.renderer.renderMode).toInt();
	renderer.showInfoPanel = settings->value("renderer/showInfoPanel", defaultSettings.renderer.showInfoPanel).toBool();
	renderer.infoPanelFontSize = settings->value("renderer/infoPanelFontSize", defaultSettings.renderer.infoPanelFontSize).toInt();

	stabilizer.enabled = settings->value("stabilizer/enabled", defaultSettings.stabilizer.enabled).toBool();
	stabilizer.mode = (VideoStabilizerMode)settings->value("stabilizer/mode", defaultSettings.stabilizer.mode).toInt();
	stabilizer.inputDataFilePath = settings->value("stabilizer/inputDataFilePath", defaultSettings.stabilizer.inputDataFilePath).toString();
	stabilizer.averagingFactor = settings->value("stabilizer/averagingFactor", defaultSettings.stabilizer.averagingFactor).toDouble();
	stabilizer.dampingFactor = settings->value("stabilizer/dampingFactor", defaultSettings.stabilizer.dampingFactor).toDouble();
	stabilizer.maxDisplacementFactor = settings->value("stabilizer/maxDisplacementFactor", defaultSettings.stabilizer.maxDisplacementFactor).toDouble();
	stabilizer.maxAngle = settings->value("stabilizer/maxAngle", defaultSettings.stabilizer.maxAngle).toDouble();
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
	settings->setValue("map/headerCrop", map.headerCrop);
	settings->setValue("map/rescaleShader", map.rescaleShader);

	settings->setValue("route/quickRouteJpegFilePath", route.quickRouteJpegFilePath);
	settings->setValue("route/discreetColor", route.discreetColor);
	settings->setValue("route/highlightColor", route.highlightColor);
	settings->setValue("route/routeRenderMode", route.routeRenderMode);
	settings->setValue("route/routeWidth", route.routeWidth);
	settings->setValue("route/tailRenderMode", route.tailRenderMode);
	settings->setValue("route/tailWidth", route.tailWidth);
	settings->setValue("route/tailLength", route.tailLength);
	settings->setValue("route/controlBorderColor", route.controlBorderColor);
	settings->setValue("route/controlRadius", route.controlRadius);
	settings->setValue("route/controlBorderWidth", route.controlBorderWidth);
	settings->setValue("route/showControls", route.showControls);
	settings->setValue("route/runnerColor", route.runnerColor);
	settings->setValue("route/runnerBorderColor", route.runnerBorderColor);
	settings->setValue("route/runnerRadius", route.runnerRadius);
	settings->setValue("route/runnerBorderWidth", route.runnerBorderWidth);
	settings->setValue("route/runnerScale", route.runnerScale);
	settings->setValue("route/showRunner", route.showRunner);
	settings->setValue("route/controlTimeOffset", route.controlTimeOffset);
	settings->setValue("route/runnerTimeOffset", route.runnerTimeOffset);
	settings->setValue("route/scale", route.scale);
	settings->setValue("route/lowPace", route.lowPace);
	settings->setValue("route/highPace", route.highPace);

	settings->setValue("routeManager/viewMode", routeManager.viewMode);
	settings->setValue("routeManager/useSmoothSplitTransition", routeManager.useSmoothSplitTransition);
	settings->setValue("routeManager/smoothSplitTransitionSpeed", routeManager.smoothSplitTransitionSpeed);
	settings->setValue("routeManager/topBottomMargin", routeManager.topBottomMargin);
	settings->setValue("routeManager/leftRightMargin", routeManager.leftRightMargin);
	settings->setValue("routeManager/maximumAutomaticZoom", routeManager.maximumAutomaticZoom);
	settings->setValue("routeManager/runnerAveragingFactor", routeManager.runnerAveragingFactor);
	settings->setValue("routeManager/runnerVerticalOffset", routeManager.runnerVerticalOffset);

	settings->setValue("video/inputVideoFilePath", video.inputVideoFilePath);
	settings->setValue("video/startTimeOffset", video.startTimeOffset);
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
	settings->setValue("video/seekToAnyFrame", video.seekToAnyFrame);

	settings->setValue("splits/type", splits.type);
	settings->setValue("splits/splitTimes", splits.splitTimes);

	settings->setValue("window/width", window.width);
	settings->setValue("window/height", window.height);
	settings->setValue("window/multisamples", window.multisamples);
	settings->setValue("window/fullscreen", window.fullscreen);
	settings->setValue("window/hideCursor", window.hideCursor);
	
	settings->setValue("renderer/renderMode", renderer.renderMode);
	settings->setValue("renderer/showInfoPanel", renderer.showInfoPanel);
	settings->setValue("renderer/infoPanelFontSize", renderer.infoPanelFontSize);

	settings->setValue("stabilizer/enabled", stabilizer.enabled);
	settings->setValue("stabilizer/mode", stabilizer.mode);
	settings->setValue("stabilizer/inputDataFilePath", stabilizer.inputDataFilePath);
	settings->setValue("stabilizer/averagingFactor", stabilizer.averagingFactor);
	settings->setValue("stabilizer/dampingFactor", stabilizer.dampingFactor);
	settings->setValue("stabilizer/maxDisplacementFactor", stabilizer.maxDisplacementFactor);
	settings->setValue("stabilizer/maxAngle", stabilizer.maxAngle);
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
	map.relativeWidth = ui->doubleSpinBoxMapRelativeWidth->value();
	map.scale = ui->doubleSpinBoxMapScale->value();
	map.headerCrop = ui->spinBoxMapHeaderCrop->value();
	map.rescaleShader = ui->comboBoxMapRescaleShader->currentText();

	route.quickRouteJpegFilePath = ui->lineEditQuickRouteJpegFile->text();
	route.routeRenderMode = (RouteRenderMode)ui->comboBoxRouteRenderMode->currentIndex();
	route.routeWidth = ui->doubleSpinBoxRouteWidth->value();
	route.tailRenderMode = (RouteRenderMode)ui->comboBoxRouteTailRenderMode->currentIndex();
	route.tailWidth = ui->doubleSpinBoxRouteTailWidth->value();
	route.tailLength = ui->doubleSpinBoxRouteTailLength->value();
	route.controlRadius = ui->doubleSpinBoxRouteControlRadius->value();
	route.controlBorderWidth = ui->doubleSpinBoxRouteControlBorderWidth->value();
	route.showControls = ui->checkBoxRouteShowControls->isChecked();
	route.runnerRadius = ui->doubleSpinBoxRouteRunnerRadius->value();
	route.runnerBorderWidth = ui->doubleSpinBoxRouteRunnerBorderWidth->value();
	route.showRunner = ui->checkBoxRouteShowRunner->isChecked();
	route.controlTimeOffset = ui->doubleSpinBoxRouteControlTimeOffset->value();
	route.runnerTimeOffset = ui->doubleSpinBoxRouteRunnerTimeOffset->value();
	route.lowPace = ui->doubleSpinBoxRouteLowPace->value();
	route.highPace = ui->doubleSpinBoxRouteHighPace->value();
	
	routeManager.viewMode = (ViewMode)ui->comboBoxRouteManagerViewMode->currentIndex();
	routeManager.useSmoothSplitTransition = ui->checkBoxRouteManagerUseSmoothSplitTransition->isChecked();
	routeManager.smoothSplitTransitionSpeed = ui->doubleSpinBoxRouteManagerSmoothSplitTransitionSpeed->value();
	routeManager.topBottomMargin = ui->doubleSpinBoxRouteManagerTopBottomMargin->value();
	routeManager.leftRightMargin = ui->doubleSpinBoxRouteManagerLeftRightMargin->value();
	routeManager.maximumAutomaticZoom = ui->doubleSpinBoxRouteManagerMaximumAutomaticZoom->value();
	routeManager.runnerAveragingFactor = ui->doubleSpinBoxRouteManagerRunnerAveragingFactor->value();
	routeManager.runnerVerticalOffset = ui->doubleSpinBoxRouteManagerRunnerVerticalOffset->value();

	video.inputVideoFilePath = ui->lineEditInputVideoFile->text();
	video.startTimeOffset = ui->doubleSpinBoxVideoStartTimeOffset->value();
	video.scale = ui->doubleSpinBoxVideoScale->value();
	video.rescaleShader = ui->comboBoxVideoRescaleShader->currentText();
	video.enableClipping = ui->checkBoxVideoEnableClipping->isChecked();
	video.enableClearing = ui->checkBoxVideoEnableClearing->isChecked();
	video.frameCountDivisor = ui->spinBoxVideoDecoderFrameCountDivisor->value();
	video.frameDurationDivisor = ui->spinBoxVideoDecoderFrameDurationDivisor->value();
	video.frameSizeDivisor = ui->spinBoxVideoDecoderFrameSizeDivisor->value();
	video.enableVerboseLogging = ui->checkBoxVideoDecoderEnableVerboseLogging->isChecked();
	video.seekToAnyFrame = ui->checkBoxVideoDecoderSeekToAnyFrame->isChecked();

	splits.type = (SplitTimeType)ui->comboBoxSplitTimeType->currentIndex();
	splits.splitTimes = ui->lineEditSplitTimes->text();

	window.width = ui->spinBoxWindowWidth->value();
	window.height = ui->spinBoxWindowHeight->value();
	window.multisamples = ui->comboBoxWindowMultisamples->currentText().toInt();
	window.fullscreen = ui->checkBoxWindowFullscreen->isChecked();
	window.hideCursor = ui->checkBoxWindowHideCursor->isChecked();

	renderer.renderMode = (RenderMode)ui->comboBoxRendererRenderMode->currentIndex();
	renderer.showInfoPanel = ui->checkBoxRendererShowInfoPanel->isChecked();
	renderer.infoPanelFontSize = ui->spinBoxRendererInfoPanelFontSize->value();

	stabilizer.enabled = ui->checkBoxVideoStabilizerEnabled->isChecked();
	stabilizer.mode = (VideoStabilizerMode)ui->comboBoxVideoStabilizerMode->currentIndex();
	stabilizer.inputDataFilePath = ui->lineEditVideoStabilizerInputDataFile->text();
	stabilizer.averagingFactor = ui->doubleSpinBoxVideoStabilizerAveragingFactor->value();
	stabilizer.dampingFactor = ui->doubleSpinBoxVideoStabilizerDampingFactor->value();
	stabilizer.maxDisplacementFactor = ui->doubleSpinBoxVideoStabilizerMaxDisplacementFactor->value();
	stabilizer.maxAngle = ui->doubleSpinBoxVideoStabilizerMaxAngle->value();
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
	ui->doubleSpinBoxMapRelativeWidth->setValue(map.relativeWidth);
	ui->doubleSpinBoxMapScale->setValue(map.scale);
	ui->spinBoxMapHeaderCrop->setValue(map.headerCrop);
	ui->labelMapBackgroundColor->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4);").arg(QString::number(map.backgroundColor.red()), QString::number(map.backgroundColor.green()), QString::number(map.backgroundColor.blue()), QString::number(map.backgroundColor.alpha())));
	ui->comboBoxMapRescaleShader->setCurrentText(map.rescaleShader);

	ui->lineEditQuickRouteJpegFile->setText(route.quickRouteJpegFilePath);
	ui->labelRouteDiscreetColor->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4);").arg(QString::number(route.discreetColor.red()), QString::number(route.discreetColor.green()), QString::number(route.discreetColor.blue()), QString::number(route.discreetColor.alpha())));
	ui->labelRouteHighlightColor->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4);").arg(QString::number(route.highlightColor.red()), QString::number(route.highlightColor.green()), QString::number(route.highlightColor.blue()), QString::number(route.highlightColor.alpha())));
	ui->comboBoxRouteRenderMode->setCurrentIndex(route.routeRenderMode);
	ui->doubleSpinBoxRouteWidth->setValue(route.routeWidth);
	ui->comboBoxRouteTailRenderMode->setCurrentIndex(route.tailRenderMode);
	ui->doubleSpinBoxRouteTailWidth->setValue(route.tailWidth);
	ui->doubleSpinBoxRouteTailLength->setValue(route.tailLength);
	ui->labelRouteControlBorderColor->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4);").arg(QString::number(route.controlBorderColor.red()), QString::number(route.controlBorderColor.green()), QString::number(route.controlBorderColor.blue()), QString::number(route.controlBorderColor.alpha())));
	ui->doubleSpinBoxRouteControlRadius->setValue(route.controlRadius);
	ui->doubleSpinBoxRouteControlBorderWidth->setValue(route.controlBorderWidth);
	ui->checkBoxRouteShowControls->setChecked(route.showControls);
	ui->labelRouteRunnerColor->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4);").arg(QString::number(route.runnerColor.red()), QString::number(route.runnerColor.green()), QString::number(route.runnerColor.blue()), QString::number(route.runnerColor.alpha())));
	ui->labelRouteRunnerBorderColor->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4);").arg(QString::number(route.runnerBorderColor.red()), QString::number(route.runnerBorderColor.green()), QString::number(route.runnerBorderColor.blue()), QString::number(route.runnerBorderColor.alpha())));
	ui->doubleSpinBoxRouteRunnerRadius->setValue(route.runnerRadius);
	ui->doubleSpinBoxRouteRunnerBorderWidth->setValue(route.runnerBorderWidth);
	ui->checkBoxRouteShowRunner->setChecked(route.showRunner);
	ui->doubleSpinBoxRouteControlTimeOffset->setValue(route.controlTimeOffset);
	ui->doubleSpinBoxRouteRunnerTimeOffset->setValue(route.runnerTimeOffset);
	ui->doubleSpinBoxRouteLowPace->setValue(route.lowPace);
	ui->doubleSpinBoxRouteHighPace->setValue(route.highPace);

	ui->comboBoxRouteManagerViewMode->setCurrentIndex(routeManager.viewMode);
	ui->checkBoxRouteManagerUseSmoothSplitTransition->setChecked(routeManager.useSmoothSplitTransition);
	ui->doubleSpinBoxRouteManagerSmoothSplitTransitionSpeed->setValue(routeManager.smoothSplitTransitionSpeed);
	ui->doubleSpinBoxRouteManagerTopBottomMargin->setValue(routeManager.topBottomMargin);
	ui->doubleSpinBoxRouteManagerLeftRightMargin->setValue(routeManager.leftRightMargin);
	ui->doubleSpinBoxRouteManagerMaximumAutomaticZoom->setValue(routeManager.maximumAutomaticZoom);
	ui->doubleSpinBoxRouteManagerRunnerAveragingFactor->setValue(routeManager.runnerAveragingFactor);
	ui->doubleSpinBoxRouteManagerRunnerVerticalOffset->setValue(routeManager.runnerVerticalOffset);

	ui->lineEditInputVideoFile->setText(video.inputVideoFilePath);
	ui->doubleSpinBoxVideoStartTimeOffset->setValue(video.startTimeOffset);
	ui->doubleSpinBoxVideoScale->setValue(video.scale);
	ui->labelVideoBackgroundColor->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4);").arg(QString::number(video.backgroundColor.red()), QString::number(video.backgroundColor.green()), QString::number(video.backgroundColor.blue()), QString::number(video.backgroundColor.alpha())));
	ui->comboBoxVideoRescaleShader->setCurrentText(video.rescaleShader);
	ui->checkBoxVideoEnableClipping->setChecked(video.enableClipping);
	ui->checkBoxVideoEnableClearing->setChecked(video.enableClearing);
	ui->spinBoxVideoDecoderFrameCountDivisor->setValue(video.frameCountDivisor);
	ui->spinBoxVideoDecoderFrameDurationDivisor->setValue(video.frameDurationDivisor);
	ui->spinBoxVideoDecoderFrameSizeDivisor->setValue(video.frameSizeDivisor);
	ui->checkBoxVideoDecoderEnableVerboseLogging->setChecked(video.enableVerboseLogging);
	ui->checkBoxVideoDecoderSeekToAnyFrame->setChecked(video.seekToAnyFrame);

	ui->comboBoxSplitTimeType->setCurrentIndex(splits.type);
	ui->lineEditSplitTimes->setText(splits.splitTimes);

	ui->spinBoxWindowWidth->setValue(window.width);
	ui->spinBoxWindowHeight->setValue(window.height);
	ui->comboBoxWindowMultisamples->setCurrentText(QString::number(window.multisamples));
	ui->checkBoxWindowFullscreen->setChecked(window.fullscreen);
	ui->checkBoxWindowHideCursor->setChecked(window.hideCursor);

	ui->comboBoxRendererRenderMode->setCurrentIndex(renderer.renderMode);
	ui->checkBoxRendererShowInfoPanel->setChecked(renderer.showInfoPanel);
	ui->spinBoxRendererInfoPanelFontSize->setValue(renderer.infoPanelFontSize);

	ui->checkBoxVideoStabilizerEnabled->setChecked(stabilizer.enabled);
	ui->comboBoxVideoStabilizerMode->setCurrentIndex(stabilizer.mode);
	ui->lineEditVideoStabilizerInputDataFile->setText(stabilizer.inputDataFilePath);
	ui->doubleSpinBoxVideoStabilizerAveragingFactor->setValue(stabilizer.averagingFactor);
	ui->doubleSpinBoxVideoStabilizerDampingFactor->setValue(stabilizer.dampingFactor);
	ui->doubleSpinBoxVideoStabilizerMaxDisplacementFactor->setValue(stabilizer.maxDisplacementFactor);
	ui->doubleSpinBoxVideoStabilizerMaxAngle->setValue(stabilizer.maxAngle);
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
