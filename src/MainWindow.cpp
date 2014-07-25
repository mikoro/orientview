// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QFileDialog>
#include <QtGUI>
#include <QMessageBox>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "VideoWindow.h"
#include "EncodeWindow.h"
#include "Settings.h"
#include "VideoDecoder.h"
#include "QuickRouteJpegReader.h"
#include "VideoStabilizer.h"
#include "VideoRenderer.h"
#include "VideoEncoder.h"
#include "VideoDecoderThread.h"
#include "RenderOnScreenThread.h"
#include "RenderOffScreenThread.h"
#include "VideoEncoderThread.h"

using namespace OrientView;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	videoWindow = new VideoWindow();
	encodeWindow = new EncodeWindow(this);
	settings = new Settings();
	videoDecoder = new VideoDecoder();
	quickRouteJpegReader = new QuickRouteJpegReader();
	videoStabilizer = new VideoStabilizer();
	videoRenderer = new VideoRenderer();
	videoEncoder = new VideoEncoder();
	videoDecoderThread = new VideoDecoderThread();
	renderOnScreenThread = new RenderOnScreenThread();
	renderOffScreenThread = new RenderOffScreenThread();
	videoEncoderThread = new VideoEncoderThread();

	connect(videoWindow, &VideoWindow::closing, this, &MainWindow::videoWindowClosing);
	connect(encodeWindow, &EncodeWindow::closing, this, &MainWindow::encodeWindowClosing);

	readSettings();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete videoWindow;
	delete encodeWindow;
	delete settings;
	delete videoDecoder;
	delete quickRouteJpegReader;
	delete videoStabilizer;
	delete videoRenderer;
	delete videoEncoder;
	delete videoDecoderThread;
	delete renderOnScreenThread;
	delete renderOffScreenThread;
	delete videoEncoderThread;
}

void MainWindow::on_pushButtonBrowseVideoFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Open video file"));
	fileDialog.setNameFilter(tr("Video files (*.mp4 *.avi);;All files (*.*)"));
	QStringList fileNames;

	if (fileDialog.exec())
	{
		fileNames = fileDialog.selectedFiles();
		ui->lineEditVideoFile->setText(fileNames.at(0));
	}
}

void MainWindow::on_pushButtonBrowseMapFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Open QuickRoute JPEG file"));
	fileDialog.setNameFilter(tr("QuickRoute JPEG files (*.jpg);;All files (*.*)"));
	QStringList fileNames;

	if (fileDialog.exec())
	{
		fileNames = fileDialog.selectedFiles();
		ui->lineEditMapFile->setText(fileNames.at(0));
	}
}

void MainWindow::on_pushButtonBrowseSettingsFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Open settings file"));
	fileDialog.setNameFilter(tr("Settings files (*.ini);;All files (*.*)"));
	QStringList fileNames;

	if (fileDialog.exec())
	{
		fileNames = fileDialog.selectedFiles();
		ui->lineEditSettingsFile->setText(fileNames.at(0));
	}
}

void MainWindow::on_pushButtonBrowseOutputVideoFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::AnyFile);
	fileDialog.setWindowTitle(tr("Save video file"));
	fileDialog.setNameFilter(tr("Video files (*.mp4)"));
	fileDialog.setDefaultSuffix(tr("mp4"));
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);
	QStringList fileNames;

	if (fileDialog.exec())
	{
		fileNames = fileDialog.selectedFiles();
		ui->lineEditOutputVideoFile->setText(fileNames.at(0));
	}
}

void MainWindow::on_pushButtonRun_clicked()
{
	this->setCursor(Qt::WaitCursor);

	try
	{
		if (!settings->initialize(ui->lineEditSettingsFile->text()))
			throw std::runtime_error("Could not initialize Settings");

		if(!videoDecoder->initialize(ui->lineEditVideoFile->text()))
			throw std::runtime_error("Could not initialize VideoDecoder");

		if (!quickRouteJpegReader->initialize(ui->lineEditMapFile->text()))
			throw std::runtime_error("Could not initialize QuickRouteJpegReader");

		if (!videoDecoderThread->initialize(videoDecoder))
			throw std::runtime_error("Could not initialize VideoDecoderThread");

		if (!renderOnScreenThread->initialize(videoWindow, videoRenderer, videoDecoderThread))
			throw std::runtime_error("Could not initialize RenderOnScreenThread");

		videoWindow->show();

		if (!videoWindow->initialize(videoDecoder))
			throw std::runtime_error("Could not initialize VideoWindow");

		if (!videoRenderer->initialize(videoDecoder, quickRouteJpegReader))
			throw std::runtime_error("Could not initialize VideoRenderer");

		videoWindow->getContext()->doneCurrent();
		videoWindow->getContext()->moveToThread(renderOnScreenThread);

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

void MainWindow::on_pushButtonEncode_clicked()
{
	this->setCursor(Qt::WaitCursor);

	try
	{
		if (!settings->initialize(ui->lineEditSettingsFile->text()))
			throw std::runtime_error("Could not initialize Settings");

		if (!videoDecoder->initialize(ui->lineEditVideoFile->text()))
			throw std::runtime_error("Could not initialize VideoDecoder");

		if (!quickRouteJpegReader->initialize(ui->lineEditMapFile->text()))
			throw std::runtime_error("Could not initialize QuickRouteJpegReader");

		if (!videoDecoderThread->initialize(videoDecoder))
			throw std::runtime_error("Could not initialize VideoDecoderThread");

		if (!renderOffScreenThread->initialize())
			throw std::runtime_error("Could not initialize RenderOffScreenThread");

		if (!encodeWindow->initialize())
			throw std::runtime_error("Could not initialize EncodeWindow");

		if (!videoRenderer->initialize(videoDecoder, quickRouteJpegReader))
			throw std::runtime_error("Could not initialize VideoRenderer");

		if (!videoEncoder->initialize(ui->lineEditOutputVideoFile->text()))
			throw std::runtime_error("Could not initialize VideoEncoder");

		if (!videoEncoderThread->initialize())
			throw std::runtime_error("Could not initialize VideoEncoderThread");

		encodeWindow->setModal(true);
		encodeWindow->show();

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

void MainWindow::videoWindowClosing()
{
	renderOnScreenThread->requestInterruption();
	videoDecoderThread->requestInterruption();
	renderOnScreenThread->wait();
	videoDecoderThread->wait();
	renderOnScreenThread->shutdown();
	videoDecoderThread->shutdown();

	videoWindow->shutdown();
	quickRouteJpegReader->shutdown();
	videoDecoder->shutdown();
	settings->shutdown();

	this->show();
	this->activateWindow();
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

	videoEncoder->shutdown();
	encodeWindow->shutdown();
	quickRouteJpegReader->shutdown();
	videoDecoder->shutdown();
	settings->shutdown();
}

void MainWindow::readSettings()
{
	QSettings settings;

	ui->lineEditVideoFile->setText(settings.value("mainWindow/videoFile", "").toString());
	ui->lineEditMapFile->setText(settings.value("mainWindow/mapFile", "").toString());
	ui->lineEditSettingsFile->setText(settings.value("mainWindow/settingsFile", "").toString());
	ui->lineEditOutputVideoFile->setText(settings.value("mainWindow/outputVideoFile", "").toString());
}

void MainWindow::writeSettings()
{
	QSettings settings;

	settings.setValue("mainWindow/videoFile", ui->lineEditVideoFile->text());
	settings.setValue("mainWindow/mapFile", ui->lineEditMapFile->text());
	settings.setValue("mainWindow/settingsFile", ui->lineEditSettingsFile->text());
	settings.setValue("mainWindow/outputVideoFile", ui->lineEditOutputVideoFile->text());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	writeSettings();
}
