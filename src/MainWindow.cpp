// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QFileDialog>
#include <QtGui>
#include <QMessageBox>
#include <QColorDialog>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Settings.h"
#include "VideoDecoder.h"
#include "QuickRouteReader.h"
#include "MapImageReader.h"
#include "VideoStabilizer.h"
#include "InputHandler.h"
#include "Renderer.h"
#include "VideoEncoder.h"
#include "VideoDecoderThread.h"
#include "RenderOnScreenThread.h"
#include "RenderOffScreenThread.h"
#include "VideoEncoderThread.h"
#include "VideoWindow.h"
#include "EncodeWindow.h"
#include "QuickRouteReader.h"

using namespace OrientView;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	settings = new Settings();
	videoDecoder = new VideoDecoder();
	quickRouteReader = new QuickRouteReader();
	mapImageReader = new MapImageReader();
	videoStabilizer = new VideoStabilizer();
	inputHandler = new InputHandler();
	renderer = new Renderer();
	videoEncoder = new VideoEncoder();
	videoDecoderThread = new VideoDecoderThread();
	renderOnScreenThread = new RenderOnScreenThread();
	renderOffScreenThread = new RenderOffScreenThread();
	videoEncoderThread = new VideoEncoderThread();
	videoWindow = new VideoWindow();
	encodeWindow = new EncodeWindow(this);

	connect(videoWindow, &VideoWindow::closing, this, &MainWindow::videoWindowClosing);
	connect(encodeWindow, &EncodeWindow::closing, this, &MainWindow::encodeWindowClosing);
	connect(videoEncoderThread, &VideoEncoderThread::frameProcessed, encodeWindow, &EncodeWindow::frameProcessed);
	connect(videoEncoderThread, &VideoEncoderThread::encodingFinished, encodeWindow, &EncodeWindow::encodingFinished);

	readSettings();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete settings;
	delete videoDecoder;
	delete quickRouteReader;
	delete mapImageReader;
	delete videoStabilizer;
	delete inputHandler;
	delete renderer;
	delete videoEncoder;
	delete videoDecoderThread;
	delete renderOnScreenThread;
	delete renderOffScreenThread;
	delete videoEncoderThread;
	delete videoWindow;
	delete encodeWindow;
}

void MainWindow::on_actionLoadSettings_triggered()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Load OrientView settings"));
	fileDialog.setNameFilter(tr("OrientView settings files (*.orv)"));

	if (fileDialog.exec())
	{
		QSettings userSettings(fileDialog.selectedFiles().at(0), QSettings::IniFormat);
		settings->read(&userSettings);
		settings->apply(ui);
	}
}

void MainWindow::on_actionSaveSettings_triggered()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::AnyFile);
	fileDialog.setWindowTitle(tr("Save OrientView settings"));
	fileDialog.setNameFilter(tr("OrientView settings files (*.orv)"));
	fileDialog.setDefaultSuffix(tr("orv"));
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);
	QStringList fileNames;

	if (fileDialog.exec())
	{
		QSettings userSettings(fileDialog.selectedFiles().at(0), QSettings::IniFormat);
		settings->update(ui);
		settings->write(&userSettings);
	}
}

void MainWindow::on_actionDefaultSettings_triggered()
{
	if (QMessageBox::warning(this, "OrientView - Warning", QString("Do you really want reset all settings to defaults?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		delete settings;
		settings = new Settings();
		settings->apply(ui);
	}
}

void MainWindow::on_actionPlayVideo_triggered()
{
	this->setCursor(Qt::WaitCursor);

	try
	{
		settings->update(ui);

		if (!videoDecoder->initialize(settings))
			throw std::runtime_error("Could not initialize VideoDecoder");

		if (!quickRouteReader->initialize(settings))
			throw std::runtime_error("Could not initialize QuickRouteReader");

		if (!mapImageReader->initialize(settings))
			throw std::runtime_error("Could not initialize MapImageReader");

		if (!videoStabilizer->initialize(settings))
			throw std::runtime_error("Could not initialize VideoStabilizer");

		if (!inputHandler->initialize(videoWindow, renderer, videoDecoder, videoDecoderThread, renderOnScreenThread, settings))
			throw std::runtime_error("Could not initialize InputHandler");

		videoWindow->show();

		if (!videoWindow->initialize(settings))
			throw std::runtime_error("Could not initialize VideoWindow");

		if (!renderer->initialize(videoWindow, videoDecoder, quickRouteReader, mapImageReader, videoStabilizer, inputHandler, settings))
			throw std::runtime_error("Could not initialize Renderer");

		if (!videoDecoderThread->initialize(videoDecoder))
			throw std::runtime_error("Could not initialize VideoDecoderThread");

		if (!renderOnScreenThread->initialize(this, videoWindow, videoDecoder, videoDecoderThread, videoStabilizer, renderer, inputHandler))
			throw std::runtime_error("Could not initialize RenderOnScreenThread");

		videoWindow->getContext()->doneCurrent();
		videoWindow->getContext()->moveToThread(renderOnScreenThread);

		renderer->setFlipOutput(false);

		videoDecoderThread->start();
		renderOnScreenThread->start();

		this->hide();
	}
	catch (const std::exception& ex)
	{
		qWarning("Could not run video: %s", ex.what());

		videoWindow->close();
		videoWindowClosing();

		QMessageBox::critical(this, "OrientView - Error", QString("Could not run video: %1\n\nPlease check the application log for details.").arg(ex.what()), QMessageBox::Ok);
	}

	this->setCursor(Qt::ArrowCursor);
}

void MainWindow::on_actionEncodeVideo_triggered()
{
	this->setCursor(Qt::WaitCursor);

	try
	{
		settings->update(ui);

		if (!videoDecoder->initialize(settings))
			throw std::runtime_error("Could not initialize VideoDecoder");

		if (!quickRouteReader->initialize(settings))
			throw std::runtime_error("Could not initialize QuickRouteReader");

		if (!mapImageReader->initialize(settings))
			throw std::runtime_error("Could not initialize MapImageReader");

		if (!videoEncoder->initialize(videoDecoder, settings))
			throw std::runtime_error("Could not initialize VideoEncoder");

		if (!videoStabilizer->initialize(settings))
			throw std::runtime_error("Could not initialize VideoStabilizer");

		if (!encodeWindow->initialize(videoDecoder, videoEncoderThread, settings))
			throw std::runtime_error("Could not initialize EncodeWindow");

		if (!renderer->initialize(videoWindow, videoDecoder, quickRouteReader, mapImageReader, videoStabilizer, inputHandler, settings))
			throw std::runtime_error("Could not initialize Renderer");

		if (!videoDecoderThread->initialize(videoDecoder))
			throw std::runtime_error("Could not initialize VideoDecoderThread");

		if (!renderOffScreenThread->initialize(this, encodeWindow, videoDecoder, videoDecoderThread, videoStabilizer, renderer, videoEncoder, settings))
			throw std::runtime_error("Could not initialize RenderOffScreenThread");

		if (!videoEncoderThread->initialize(videoDecoder, videoEncoder, renderOffScreenThread))
			throw std::runtime_error("Could not initialize VideoEncoderThread");

		encodeWindow->setModal(true);
		encodeWindow->show();

		encodeWindow->getContext()->doneCurrent();
		encodeWindow->getContext()->moveToThread(renderOffScreenThread);

		renderer->setFlipOutput(true);

		videoDecoderThread->start();
		renderOffScreenThread->start();
		videoEncoderThread->start();
	}
	catch (const std::exception& ex)
	{
		qWarning("Could not encode video: %s", ex.what());

		encodeWindow->close();
		encodeWindowClosing();

		QMessageBox::critical(this, "OrientView - Error", QString("Could not encode video: %1\n\nPlease check the application log for details.").arg(ex.what()), QMessageBox::Ok);
	}

	this->setCursor(Qt::ArrowCursor);
}

void MainWindow::on_actionExit_triggered()
{
	close();
}

void MainWindow::videoWindowClosing()
{
	renderOnScreenThread->requestInterruption();
	videoDecoderThread->requestInterruption();

	renderOnScreenThread->wait();
	videoDecoderThread->wait();

	renderOnScreenThread->shutdown();
	videoDecoderThread->shutdown();

	if (videoWindow->isInitialized())
		videoWindow->getContext()->makeCurrent(videoWindow);

	renderer->shutdown();
	videoWindow->shutdown();
	videoStabilizer->shutdown();
	videoDecoder->shutdown();

	this->show();
	this->activateWindow();

	QApplication::restoreOverrideCursor();
}

void MainWindow::encodeWindowClosing()
{
	videoEncoderThread->requestInterruption();
	renderOffScreenThread->requestInterruption();
	videoDecoderThread->requestInterruption();

	videoEncoderThread->wait();
	renderOffScreenThread->wait();
	videoDecoderThread->wait();

	videoEncoderThread->shutdown();
	renderOffScreenThread->shutdown();
	videoDecoderThread->shutdown();

	if (encodeWindow->isInitialized())
		encodeWindow->getContext()->makeCurrent(encodeWindow->getSurface());

	renderer->shutdown();
	encodeWindow->shutdown();
	videoStabilizer->shutdown();
	videoEncoder->shutdown();
	videoDecoder->shutdown();
}

void MainWindow::on_pushButtonBrowseInputVideoFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Select input video file"));
	fileDialog.setNameFilter(tr("Video files (*.mp4 *.avi *.mkv);;All files (*.*)"));

	if (fileDialog.exec())
		ui->lineEditInputVideoFile->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonBrowseQuickRouteJpegMapImageFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Select QuickRoute JPEG map image file"));
	fileDialog.setNameFilter(tr("QuickRoute JPEG map image files (*.jpg);;All files (*.*)"));
	
	if (fileDialog.exec())
		ui->lineEditQuickRouteJpegMapImageFile->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonBrowseAlternativeMapImageFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Select alternative map image file"));
	fileDialog.setNameFilter(tr("Image files (*.jpg *.png *.tiff *.tif);;All files (*.*)"));

	if (fileDialog.exec())
		ui->lineEditAlternativeMapImageFile->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonBrowseOutputVideoFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::AnyFile);
	fileDialog.setWindowTitle(tr("Select output video file"));
	fileDialog.setNameFilter(tr("MP4 video files (*.mp4)"));
	fileDialog.setDefaultSuffix(tr("mp4"));
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);

	if (fileDialog.exec())
		ui->lineEditOutputVideoFile->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonPickVideoPanelBackgroundColor_clicked()
{
	settings->update(ui);

	QColorDialog colorDialog;
	QColor resultColor = colorDialog.getColor(settings->appearance.videoPanelBackgroundColor, this, "Pick video panel background color");

	if (resultColor.isValid())
		settings->appearance.videoPanelBackgroundColor = resultColor;

	settings->apply(ui);
}

void MainWindow::on_pushButtonPickMapPanelBackgroundColor_clicked()
{
	settings->update(ui);

	QColorDialog colorDialog;
	QColor resultColor = colorDialog.getColor(settings->appearance.mapPanelBackgroundColor, this, "Pick map panel background color");

	if (resultColor.isValid())
		settings->appearance.mapPanelBackgroundColor = resultColor;

	settings->apply(ui);
}

void MainWindow::readSettings()
{
	QSettings localSettings;
	settings->read(&localSettings);
	settings->apply(ui);
}

void MainWindow::writeSettings()
{
	QSettings localSettings;
	settings->update(ui);
	settings->write(&localSettings);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	Q_UNUSED(event);

	writeSettings();
}
