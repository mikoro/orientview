// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QFile>
#include <QSettings>

#include "Settings.h"
#include "ui_MainWindow.h"

using namespace OrientView;

void Settings::read(QSettings* settings)
{
	files.inputVideoFilePath = settings->value("files/inputVideoFilePath", "").toString();
	files.quickRouteJpegMapImageFilePath = settings->value("files/quickRouteJpegMapImageFilePath", "").toString();
	files.alternativeMapImageFilePath = settings->value("files/alternativeMapImageFilePath", "").toString();
	files.outputVideoFilePath = settings->value("files/outputVideoFilePath", "").toString();

	window.width = settings->value("window/width", 1280).toInt();
	window.height = settings->value("window/height", 720).toInt();
	window.multisamples = settings->value("window/multisamples", 16).toInt();
	window.fullscreen = settings->value("window/fullscreen", false).toBool();
	window.hideCursor = settings->value("window/hideCursor", false).toBool();

	timing.splitTimes = settings->value("timing/splitTimes", "").toString();

	appearance.showInfoPanel = settings->value("appearance/showInfoPanel", false).toBool();
	appearance.mapPanelWidth = settings->value("appearance/mapPanelWidth", 0.3).toDouble();
	appearance.videoPanelScale = settings->value("appearance/videoPanelScale", 1.0).toDouble();
	appearance.mapPanelScale = settings->value("appearance/mapPanelScale", 1.0).toDouble();
	appearance.videoPanelBackgroundColor = settings->value("appearance/videoPanelBackgroundColor", QColor("#003200")).value<QColor>();
	appearance.mapPanelBackgroundColor = settings->value("appearance/mapPanelBackgroundColor", QColor("#ffffff")).value<QColor>();
	
	decoder.frameCountDivisor = settings->value("decoder/frameCountDivisor", 1).toInt();
	decoder.frameDurationDivisor = settings->value("decoder/frameDurationDivisor", 1).toInt();
	decoder.frameSizeDivisor = settings->value("decoder/frameSizeDivisor", 1).toInt();

	stabilizer.enabled = settings->value("stabilizer/enabled", true).toBool();
	stabilizer.frameSizeDivisor = settings->value("stabilizer/frameSizeDivisor", 8).toInt();
	stabilizer.averagingFactor = settings->value("stabilizer/averagingFactor", 0.1).toDouble();
	stabilizer.dampingFactor = settings->value("stabilizer/dampingFactor", 0.5).toDouble();
	stabilizer.disableVideoClear = settings->value("stabilizer/disableVideoClear", false).toBool();
	stabilizer.inpaintBorderWidth = settings->value("stabilizer/inpaintBorderWidth", 0).toInt();

	shaders.videoPanelShader = settings->value("shaders/videoPanelShader", "default").toString();
	shaders.mapPanelShader = settings->value("shaders/mapPanelShader", "default").toString();

	encoder.preset = settings->value("encoder/preset", "veryfast").toString();
	encoder.profile = settings->value("encoder/profile", "high").toString();
	encoder.constantRateFactor = settings->value("encoder/constantRateFactor", 23).toInt();

	inputHandler.smallSeekAmount = settings->value("inputHandler/smallSeekAmount", 2).toInt();
	inputHandler.normalSeekAmount = settings->value("inputHandler/normalSeekAmount", 10).toInt();
	inputHandler.largeSeekAmount = settings->value("inputHandler/largeSeekAmount", 60).toInt();
	inputHandler.veryLargeSeekAmount = settings->value("inputHandler/veryLargeSeekAmount", 600).toInt();
	inputHandler.slowTranslateVelocity = settings->value("inputHandler/slowTranslateVelocity", 0.1).toDouble();
	inputHandler.normalTranslateVelocity = settings->value("inputHandler/normalTranslateVelocity", 0.5).toDouble();
	inputHandler.fastTranslateVelocity = settings->value("inputHandler/fastTranslateVelocity", 1.0).toDouble();
	inputHandler.slowRotateVelocity = settings->value("inputHandler/slowRotateVelocity", 0.02).toDouble();
	inputHandler.normalRotateVelocity = settings->value("inputHandler/normalRotateVelocity", 0.1).toDouble();
	inputHandler.fastRotateVelocity = settings->value("inputHandler/fastRotateVelocity", 0.5).toDouble();
	inputHandler.smallScaleConstant = settings->value("inputHandler/smallScaleConstant", 5000.0).toDouble();
	inputHandler.normalScaleConstant = settings->value("inputHandler/normalScaleConstant", 500.0).toDouble();
	inputHandler.largeScaleConstant = settings->value("inputHandler/largeScaleConstant", 100.0).toDouble();
}

void Settings::write(QSettings* settings)
{
	settings->setValue("files/inputVideoFilePath", files.inputVideoFilePath);
	settings->setValue("files/quickRouteJpegMapImageFilePath", files.quickRouteJpegMapImageFilePath);
	settings->setValue("files/alternativeMapImageFilePath", files.alternativeMapImageFilePath);
	settings->setValue("files/outputVideoFilePath", files.outputVideoFilePath);

	settings->setValue("window/width", window.width);
	settings->setValue("window/height", window.height);
	settings->setValue("window/multisamples", window.multisamples);
	settings->setValue("window/fullscreen", window.fullscreen);
	settings->setValue("window/hideCursor", window.hideCursor);

	settings->setValue("timing/splitTimes", timing.splitTimes);

	settings->setValue("appearance/showInfoPanel", appearance.showInfoPanel);
	settings->setValue("appearance/mapPanelWidth", appearance.mapPanelWidth);
	settings->setValue("appearance/videoPanelScale", appearance.videoPanelScale);
	settings->setValue("appearance/mapPanelScale", appearance.mapPanelScale);
	settings->setValue("appearance/videoPanelBackgroundColor", appearance.videoPanelBackgroundColor);
	settings->setValue("appearance/mapPanelBackgroundColor", appearance.mapPanelBackgroundColor);

	settings->setValue("decoder/frameCountDivisor", decoder.frameCountDivisor);
	settings->setValue("decoder/frameDurationDivisor", decoder.frameDurationDivisor);
	settings->setValue("decoder/frameSizeDivisor", decoder.frameSizeDivisor);

	settings->setValue("stabilizer/enabled", stabilizer.enabled);
	settings->setValue("stabilizer/frameSizeDivisor", stabilizer.frameSizeDivisor);
	settings->setValue("stabilizer/averagingFactor", stabilizer.averagingFactor);
	settings->setValue("stabilizer/dampingFactor", stabilizer.dampingFactor);
	settings->setValue("stabilizer/disableVideoClear", stabilizer.disableVideoClear);
	settings->setValue("stabilizer/inpaintBorderWidth", stabilizer.inpaintBorderWidth);

	settings->setValue("shaders/videoPanelShader", shaders.videoPanelShader);
	settings->setValue("shaders/mapPanelShader", shaders.mapPanelShader);

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

void Settings::update(Ui::MainWindow* ui)
{
	files.inputVideoFilePath = ui->lineEditInputVideoFile->text();
	files.quickRouteJpegMapImageFilePath = ui->lineEditQuickRouteJpegMapImageFile->text();
	files.alternativeMapImageFilePath = ui->lineEditAlternativeMapImageFile->text();
	files.outputVideoFilePath = ui->lineEditOutputVideoFile->text();

	window.width = ui->spinBoxWindowWidth->value();
	window.height = ui->spinBoxWindowHeight->value();
	window.multisamples = ui->comboBoxMultisamples->currentText().toInt();
	window.fullscreen = ui->checkBoxFullscreen->isChecked();
	window.hideCursor = ui->checkBoxHideCursor->isChecked();

	timing.splitTimes = ui->lineEditSplitTimes->text();

	appearance.showInfoPanel = ui->checkBoxShowInfoPanel->isChecked();
	appearance.mapPanelWidth = ui->doubleSpinBoxMapPanelWidth->value();
	appearance.videoPanelScale = ui->doubleSpinBoxVideoPanelScale->value();
	appearance.mapPanelScale = ui->doubleSpinBoxMapPanelScale->value();
	appearance.videoPanelBackgroundColor = QColor(ui->lineEditBackgroundColorVideoPanel->text());
	appearance.mapPanelBackgroundColor = QColor(ui->lineEditBackgroundColorMapPanel->text());

	decoder.frameCountDivisor = ui->spinBoxDecoderFrameCountDivisor->value();
	decoder.frameDurationDivisor = ui->spinBoxDecoderFrameDurationDivisor->value();
	decoder.frameSizeDivisor = ui->spinBoxDecoderFrameSizeDivisor->value();

	stabilizer.enabled = ui->checkBoxStabilizerEnabled->isChecked();
	stabilizer.frameSizeDivisor = ui->spinBoxStabilizerFrameSizeDivisor->value();
	stabilizer.averagingFactor = ui->doubleSpinBoxStabilizerAveragingFactor->value();
	stabilizer.dampingFactor = ui->doubleSpinBoxStabilizerDampingFactor->value();
	stabilizer.disableVideoClear = ui->checkBoxStabilizerDisableVideoClear->isChecked();
	stabilizer.inpaintBorderWidth = ui->spinBoxStabilizerInpaintBorderWidth->value();

	shaders.videoPanelShader = ui->comboBoxVideoPanelShader->currentText();
	shaders.mapPanelShader = ui->comboBoxMapPanelShader->currentText();

	encoder.preset = ui->comboBoxEncoderPreset->currentText();
	encoder.profile = ui->comboBoxEncoderProfile->currentText();
	encoder.constantRateFactor = ui->spinBoxEncoderCrf->value();
}

void Settings::apply(Ui::MainWindow* ui)
{
	ui->lineEditInputVideoFile->setText(files.inputVideoFilePath);
	ui->lineEditQuickRouteJpegMapImageFile->setText(files.quickRouteJpegMapImageFilePath);
	ui->lineEditAlternativeMapImageFile->setText(files.alternativeMapImageFilePath);
	ui->lineEditOutputVideoFile->setText(files.outputVideoFilePath);

	ui->spinBoxWindowWidth->setValue(window.width);
	ui->spinBoxWindowHeight->setValue(window.height);
	ui->comboBoxMultisamples->setCurrentText(QString::number(window.multisamples));
	ui->checkBoxFullscreen->setChecked(window.fullscreen);
	ui->checkBoxHideCursor->setChecked(window.hideCursor);

	ui->lineEditSplitTimes->setText(timing.splitTimes);

	ui->checkBoxShowInfoPanel->setChecked(appearance.showInfoPanel);
	ui->doubleSpinBoxMapPanelWidth->setValue(appearance.mapPanelWidth);
	ui->doubleSpinBoxVideoPanelScale->setValue(appearance.videoPanelScale);
	ui->doubleSpinBoxMapPanelScale->setValue(appearance.mapPanelScale);
	ui->lineEditBackgroundColorVideoPanel->setText(appearance.videoPanelBackgroundColor.name());
	ui->lineEditBackgroundColorMapPanel->setText(appearance.mapPanelBackgroundColor.name());

	ui->spinBoxDecoderFrameCountDivisor->setValue(decoder.frameCountDivisor);
	ui->spinBoxDecoderFrameDurationDivisor->setValue(decoder.frameDurationDivisor);
	ui->spinBoxDecoderFrameSizeDivisor->setValue(decoder.frameSizeDivisor);

	ui->checkBoxStabilizerEnabled->setChecked(stabilizer.enabled);
	ui->spinBoxStabilizerFrameSizeDivisor->setValue(stabilizer.frameSizeDivisor);
	ui->doubleSpinBoxStabilizerAveragingFactor->setValue(stabilizer.averagingFactor);
	ui->doubleSpinBoxStabilizerDampingFactor->setValue(stabilizer.dampingFactor);
	ui->checkBoxStabilizerDisableVideoClear->setChecked(stabilizer.disableVideoClear);
	ui->spinBoxStabilizerInpaintBorderWidth->setValue(stabilizer.inpaintBorderWidth);

	ui->comboBoxVideoPanelShader->setCurrentText(shaders.videoPanelShader);
	ui->comboBoxMapPanelShader->setCurrentText(shaders.mapPanelShader);

	ui->comboBoxEncoderPreset->setCurrentText(encoder.preset);
	ui->comboBoxEncoderProfile->setCurrentText(encoder.profile);
	ui->spinBoxEncoderCrf->setValue(encoder.constantRateFactor);
}
