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
	connect(videoEncoderThread, &VideoEncoderThread::progressUpdate, encodeWindow, &EncodeWindow::progressUpdate);
	connect(videoEncoderThread, &VideoEncoderThread::encodingFinished, encodeWindow, &EncodeWindow::encodingFinished);

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

void MainWindow::on_pushButtonEditSettingsFile_clicked()
{
	QString fileName = ui->lineEditSettingsFile->text();

	if (QFile::exists(fileName))
	{
		QFileInfo fileInfo(fileName);
		QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(fileInfo.absoluteFilePath())));
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

		if(!videoDecoder->initialize(ui->lineEditVideoFile->text(), settings))
			throw std::runtime_error("Could not initialize VideoDecoder");

		if (!quickRouteJpegReader->initialize(ui->lineEditMapFile->text()))
			throw std::runtime_error("Could not initialize QuickRouteJpegReader");

		if (!videoStabilizer->initialize(settings))
			throw std::runtime_error("Could not initialize VideoStabilizer");

		videoWindow->show();

		if (!videoWindow->initialize(settings))
			throw std::runtime_error("Could not initialize VideoWindow");

		if (!videoRenderer->initialize(videoDecoder, quickRouteJpegReader, settings))
			throw std::runtime_error("Could not initialize VideoRenderer");

		if (!videoDecoderThread->initialize(videoDecoder))
			throw std::runtime_error("Could not initialize VideoDecoderThread");

		if (!renderOnScreenThread->initialize(this, videoWindow, videoDecoderThread, videoStabilizer, videoRenderer))
			throw std::runtime_error("Could not initialize RenderOnScreenThread");

		videoWindow->getContext()->doneCurrent();
		videoWindow->getContext()->moveToThread(renderOnScreenThread);

		videoRenderer->setFlipOutput(false);

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

		if (!videoDecoder->initialize(ui->lineEditVideoFile->text(), settings))
			throw std::runtime_error("Could not initialize VideoDecoder");

		if (!videoEncoder->initialize(ui->lineEditOutputVideoFile->text(), videoDecoder, settings))
			throw std::runtime_error("Could not initialize VideoEncoder");

		if (!quickRouteJpegReader->initialize(ui->lineEditMapFile->text()))
			throw std::runtime_error("Could not initialize QuickRouteJpegReader");

		if (!videoStabilizer->initialize(settings))
			throw std::runtime_error("Could not initialize VideoStabilizer");

		if (!encodeWindow->initialize(settings))
			throw std::runtime_error("Could not initialize EncodeWindow");

		if (!videoRenderer->initialize(videoDecoder, quickRouteJpegReader, settings))
			throw std::runtime_error("Could not initialize VideoRenderer");

		if (!videoDecoderThread->initialize(videoDecoder))
			throw std::runtime_error("Could not initialize VideoDecoderThread");

		if (!renderOffScreenThread->initialize(this, encodeWindow, videoDecoderThread, videoRenderer, settings))
			throw std::runtime_error("Could not initialize RenderOffScreenThread");

		if (!videoEncoderThread->initialize(videoDecoder, videoEncoder, renderOffScreenThread))
			throw std::runtime_error("Could not initialize VideoEncoderThread");

		encodeWindow->setModal(true);
		encodeWindow->show();

		encodeWindow->getContext()->doneCurrent();
		encodeWindow->getContext()->moveToThread(renderOffScreenThread);

		videoRenderer->setFlipOutput(true);

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

	if (videoWindow->getIsInitialized())
		videoWindow->getContext()->makeCurrent(videoWindow);

	videoRenderer->shutdown();
	videoWindow->shutdown();
	videoStabilizer->shutdown();
	quickRouteJpegReader->shutdown();
	videoDecoder->shutdown();
	settings->shutdown();

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

	if (encodeWindow->getIsInitialized())
		encodeWindow->getContext()->makeCurrent(encodeWindow->getSurface());

	videoRenderer->shutdown();
	encodeWindow->shutdown();
	videoStabilizer->shutdown();
	quickRouteJpegReader->shutdown();
	videoEncoder->shutdown();
	videoDecoder->shutdown();
	settings->shutdown();
}

void MainWindow::readSettings()
{
	QSettings tempSettings;

	ui->lineEditVideoFile->setText(tempSettings.value("mainWindow/videoFile", "").toString());
	ui->lineEditMapFile->setText(tempSettings.value("mainWindow/mapFile", "").toString());
	ui->lineEditSettingsFile->setText(tempSettings.value("mainWindow/settingsFile", "").toString());
	ui->lineEditOutputVideoFile->setText(tempSettings.value("mainWindow/outputVideoFile", "").toString());

	if (ui->lineEditSettingsFile->text().isEmpty())
		ui->lineEditSettingsFile->setText("data/settings/default.ini");
}

void MainWindow::writeSettings()
{
	QSettings tempSettings;

	tempSettings.setValue("mainWindow/videoFile", ui->lineEditVideoFile->text());
	tempSettings.setValue("mainWindow/mapFile", ui->lineEditMapFile->text());
	tempSettings.setValue("mainWindow/settingsFile", ui->lineEditSettingsFile->text());
	tempSettings.setValue("mainWindow/outputVideoFile", ui->lineEditOutputVideoFile->text());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	writeSettings();
}
