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
	connect(videoEncoderThread, &VideoEncoderThread::frameProcessed, encodeWindow, &EncodeWindow::frameProcessed);
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

void MainWindow::on_actionOpen_triggered()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Open OrientView settings"));
	fileDialog.setNameFilter(tr("OrientView settings files (*.orv)"));

	if (fileDialog.exec())
	{
		QSettings userSettings(fileDialog.selectedFiles().at(0), QSettings::IniFormat);
		settings->read(&userSettings);
		settings->apply(ui);
	}
}

void MainWindow::on_actionSaveAs_triggered()
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

void MainWindow::on_actionPlayVideo_triggered()
{
	this->setCursor(Qt::WaitCursor);

	try
	{
		settings->update(ui);

		if (!videoDecoder->initialize(ui->lineEditVideoFile->text(), settings))
			throw std::runtime_error("Could not initialize VideoDecoder");

		if (!quickRouteJpegReader->initialize(ui->lineEditMapFile->text()))
			throw std::runtime_error("Could not initialize QuickRouteJpegReader");

		if (!videoStabilizer->initialize(settings))
			throw std::runtime_error("Could not initialize VideoStabilizer");

		videoWindow->show();

		if (!videoWindow->initialize(renderOnScreenThread, settings))
			throw std::runtime_error("Could not initialize VideoWindow");

		if (!videoRenderer->initialize(videoDecoder, quickRouteJpegReader, videoStabilizer, videoEncoder, videoWindow, settings))
			throw std::runtime_error("Could not initialize VideoRenderer");

		if (!videoDecoderThread->initialize(videoDecoder))
			throw std::runtime_error("Could not initialize VideoDecoderThread");

		if (!renderOnScreenThread->initialize(this, videoWindow, videoDecoder, videoDecoderThread, videoStabilizer, videoRenderer, settings))
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

void MainWindow::on_actionEncodeVideo_triggered()
{
	this->setCursor(Qt::WaitCursor);

	try
	{
		settings->update(ui);

		if (!videoDecoder->initialize(ui->lineEditVideoFile->text(), settings))
			throw std::runtime_error("Could not initialize VideoDecoder");

		if (!videoEncoder->initialize(ui->lineEditOutputFile->text(), videoDecoder, settings))
			throw std::runtime_error("Could not initialize VideoEncoder");

		if (!quickRouteJpegReader->initialize(ui->lineEditMapFile->text()))
			throw std::runtime_error("Could not initialize QuickRouteJpegReader");

		if (!videoStabilizer->initialize(settings))
			throw std::runtime_error("Could not initialize VideoStabilizer");

		if (!encodeWindow->initialize(videoDecoder, videoEncoderThread, settings))
			throw std::runtime_error("Could not initialize EncodeWindow");

		if (!videoRenderer->initialize(videoDecoder, quickRouteJpegReader, videoStabilizer, videoEncoder, videoWindow, settings))
			throw std::runtime_error("Could not initialize VideoRenderer");

		if (!videoDecoderThread->initialize(videoDecoder))
			throw std::runtime_error("Could not initialize VideoDecoderThread");

		if (!renderOffScreenThread->initialize(this, encodeWindow, videoDecoderThread, videoStabilizer, videoRenderer, settings))
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

	videoRenderer->shutdown();
	videoWindow->shutdown();
	videoStabilizer->shutdown();
	quickRouteJpegReader->shutdown();
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

	videoRenderer->shutdown();
	encodeWindow->shutdown();
	videoStabilizer->shutdown();
	quickRouteJpegReader->shutdown();
	videoEncoder->shutdown();
	videoDecoder->shutdown();
}

void MainWindow::on_pushButtonBrowseVideoFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Select video file"));
	fileDialog.setNameFilter(tr("Video files (*.mp4 *.avi *.mkv);;All files (*.*)"));

	if (fileDialog.exec())
		ui->lineEditVideoFile->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonBrowseMapFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Select map image file"));
	fileDialog.setNameFilter(tr("Map image files (*.jpg *.png *.tiff *.tif);;All files (*.*)"));
	
	if (fileDialog.exec())
		ui->lineEditMapFile->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonBrowseGpxFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Select GPX file"));
	fileDialog.setNameFilter(tr("GPX files (*.gpx);;All files (*.*)"));

	if (fileDialog.exec())
		ui->lineEditGpxFile->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonBrowseOutputFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::AnyFile);
	fileDialog.setWindowTitle(tr("Select output file"));
	fileDialog.setNameFilter(tr("Video files (*.mp4)"));
	fileDialog.setDefaultSuffix(tr("mp4"));
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);

	if (fileDialog.exec())
		ui->lineEditOutputFile->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonLoadCalibrationData_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Open QuickRoute file"));
	fileDialog.setNameFilter(tr("QuickRoute files (*.qrt *.jpg);;All files (*.*)"));

	if (fileDialog.exec())
	{
	}
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
	writeSettings();
}
