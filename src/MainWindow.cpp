// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QFileDialog>
#include <QtGui>
#include <QMessageBox>
#include <QColorDialog>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Settings.h"
#include "VideoWindow.h"
#include "EncodeWindow.h"
#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "QuickRouteReader.h"
#include "MapImageReader.h"
#include "VideoStabilizer.h"
#include "InputHandler.h"
#include "Renderer.h"
#include "VideoDecoderThread.h"
#include "RenderOnScreenThread.h"
#include "RenderOffScreenThread.h"
#include "VideoEncoderThread.h"

using namespace OrientView;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	logDataModel = new QStandardItemModel(0, 3);
	settings = new Settings();

	readSettings();

	QStringList logLabels;
	logLabels.append("Time");
	logLabels.append("Level");
	logLabels.append("Message");
	logDataModel->setHorizontalHeaderLabels(logLabels);
	ui->treeViewLog->setModel(logDataModel);

	ui->tabWidgetMain->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
	if (logDataModel != nullptr)
	{
		delete logDataModel;
		logDataModel = nullptr;
	}

	if (settings != nullptr)
	{
		delete settings;
		settings = nullptr;
	}

	if (ui != nullptr)
	{
		delete ui;
		ui = nullptr;
	}
}

void MainWindow::addLogMessage(QString timeString, QString typeString, QString messageString)
{
	QList<QStandardItem*> newRow;
	QStandardItem* firstColumn = new QStandardItem(timeString);

	newRow.append(firstColumn);
	newRow.append(new QStandardItem(typeString));
	newRow.append(new QStandardItem(messageString));

	logDataModel->appendRow(newRow);
	ui->treeViewLog->scrollTo(firstColumn->index(), QAbstractItemView::PositionAtBottom);
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

	settings->update(ui);

	try
	{
		videoDecoder = new VideoDecoder();

		if (!videoDecoder->initialize(settings))
		{
			if (QMessageBox::warning(this, "OrientView - Warning", QString("Could not open the video file.\n\nDo you want to continue anyway?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				throw std::runtime_error("Could not initialize the video decoder");
		}

		mapImageReader = new MapImageReader();

		if (!mapImageReader->initialize(settings))
		{
			if (QMessageBox::warning(this, "OrientView - Warning", QString("Could not read the map image file.\n\nDo you want to continue anyway?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				throw std::runtime_error("Could not initialize the map image reader");
		}

		quickRouteReader = new QuickRouteReader();

		if (!quickRouteReader->initialize(mapImageReader, settings))
		{
			if (QMessageBox::warning(this, "OrientView - Warning", QString("Could not read the QuickRoute file.\n\nDo you want to continue anyway?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				throw std::runtime_error("Could not initialize the QuickRoute reader");
		}
	}
	catch (const std::exception& ex)
	{
		qWarning("%s", ex.what());

		this->setCursor(Qt::ArrowCursor);
		playVideoFinished();

		return;
	}

	try
	{
		videoWindow = new VideoWindow();
		renderer = new Renderer();
		videoStabilizer = new VideoStabilizer();
		inputHandler = new InputHandler();
		videoDecoderThread = new VideoDecoderThread();
		renderOnScreenThread = new RenderOnScreenThread();

		videoWindow->show();

		if (!videoWindow->initialize(settings))
			throw std::runtime_error("Could not initialize the video window");

		if (!renderer->initialize(videoDecoder, quickRouteReader, mapImageReader, videoStabilizer, inputHandler, settings))
			throw std::runtime_error("Could not initialize the renderer");

		videoStabilizer->initialize(settings);
		inputHandler->initialize(videoWindow, renderer, videoDecoder, videoDecoderThread, videoStabilizer, renderOnScreenThread, settings);
		videoDecoderThread->initialize(videoDecoder);
		renderOnScreenThread->initialize(this, videoWindow, videoDecoder, videoDecoderThread, videoStabilizer, renderer, inputHandler);

		connect(videoWindow, &VideoWindow::closing, this, &MainWindow::playVideoFinished);
		connect(videoWindow, &VideoWindow::resizing, renderOnScreenThread, &RenderOnScreenThread::windowResized);

		videoWindow->getContext()->doneCurrent();
		videoWindow->getContext()->moveToThread(renderOnScreenThread);

		renderer->setFlipOutput(false);
		renderer->setIsEncoding(false);

		videoDecoderThread->start();
		renderOnScreenThread->start();

		this->hide();
	}
	catch (const std::exception& ex)
	{
		qWarning("%s", ex.what());

		videoWindow->close();
		playVideoFinished();

		QMessageBox::critical(this, "OrientView - Error", QString("%1.\n\nCheck the application log for details.").arg(ex.what()), QMessageBox::Ok);
	}

	this->setCursor(Qt::ArrowCursor);
}

void MainWindow::playVideoFinished()
{
	if (renderOnScreenThread != nullptr)
	{
		renderOnScreenThread->requestInterruption();
		renderOnScreenThread->wait();
		delete renderOnScreenThread;
		renderOnScreenThread = nullptr;
	}

	if (videoDecoderThread != nullptr)
	{
		videoDecoderThread->requestInterruption();
		videoDecoderThread->wait();
		delete videoDecoderThread;
		videoDecoderThread = nullptr;
	}

	if (videoWindow != nullptr && videoWindow->getIsInitialized())
		videoWindow->getContext()->makeCurrent(videoWindow);

	if (inputHandler != nullptr)
	{
		delete inputHandler;
		inputHandler = nullptr;
	}

	if (videoStabilizer != nullptr)
	{
		delete videoStabilizer;
		videoStabilizer = nullptr;
	}

	if (renderer != nullptr)
	{
		delete renderer;
		renderer = nullptr;
	}

	if (videoWindow != nullptr)
	{
		videoWindow->deleteLater();
		videoWindow = nullptr;
	}
	
	if (mapImageReader != nullptr)
	{
		delete mapImageReader;
		mapImageReader = nullptr;
	}

	if (quickRouteReader != nullptr)
	{
		delete quickRouteReader;
		quickRouteReader = nullptr;
	}

	if (videoDecoder != nullptr)
	{
		delete videoDecoder;
		videoDecoder = nullptr;
	}

	this->show();
	this->activateWindow();
}

void MainWindow::on_actionEncodeVideo_triggered()
{
	this->setCursor(Qt::WaitCursor);

	settings->update(ui);

	try
	{
		videoDecoder = new VideoDecoder();

		if (!videoDecoder->initialize(settings))
		{
			if (QMessageBox::warning(this, "OrientView - Warning", QString("Could not open the video file.\n\nDo you want to continue anyway?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				throw std::runtime_error("Could not initialize the video decoder");
		}

		mapImageReader = new MapImageReader();

		if (!mapImageReader->initialize(settings))
		{
			if (QMessageBox::warning(this, "OrientView - Warning", QString("Could not read the map image file.\n\nDo you want to continue anyway?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				throw std::runtime_error("Could not initialize the map image reader");
		}

		quickRouteReader = new QuickRouteReader();

		if (!quickRouteReader->initialize(mapImageReader, settings))
		{
			if (QMessageBox::warning(this, "OrientView - Warning", QString("Could not read the QuickRoute file.\n\nDo you want to continue anyway?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				throw std::runtime_error("Could not initialize the QuickRoute reader");
		}
	}
	catch (const std::exception& ex)
	{
		qWarning("%s", ex.what());

		this->setCursor(Qt::ArrowCursor);
		encodeVideoFinished();

		return;
	}

	try
	{
		encodeWindow = new EncodeWindow(this);
		videoEncoder = new VideoEncoder();
		renderer = new Renderer();
		videoStabilizer = new VideoStabilizer();
		inputHandler = new InputHandler();
		videoDecoderThread = new VideoDecoderThread();
		renderOffScreenThread = new RenderOffScreenThread();
		videoEncoderThread = new VideoEncoderThread();
		
		if (!encodeWindow->initialize(videoDecoder, videoEncoderThread, settings))
			throw std::runtime_error("Could not initialize the encode window");

		if (!videoEncoder->initialize(videoDecoder, settings))
			throw std::runtime_error("Could not initialize the video encoder");

		if (!renderer->initialize(videoDecoder, quickRouteReader, mapImageReader, videoStabilizer, inputHandler, settings))
			throw std::runtime_error("Could not initialize the renderer");

		videoStabilizer->initialize(settings);
		videoDecoderThread->initialize(videoDecoder);
		renderOffScreenThread->initialize(this, encodeWindow, videoDecoder, videoDecoderThread, videoStabilizer, renderer, videoEncoder, settings);
		videoEncoderThread->initialize(videoDecoder, videoEncoder, renderOffScreenThread);

		connect(encodeWindow, &EncodeWindow::closing, this, &MainWindow::encodeVideoFinished);
		connect(videoEncoderThread, &VideoEncoderThread::frameProcessed, encodeWindow, &EncodeWindow::frameProcessed);
		connect(videoEncoderThread, &VideoEncoderThread::encodingFinished, encodeWindow, &EncodeWindow::encodingFinished);

		encodeWindow->setModal(true);
		encodeWindow->show();

		encodeWindow->getContext()->doneCurrent();
		encodeWindow->getContext()->moveToThread(renderOffScreenThread);

		renderer->setFlipOutput(true);
		renderer->setIsEncoding(true);

		videoDecoderThread->start();
		renderOffScreenThread->start();
		videoEncoderThread->start();
	}
	catch (const std::exception& ex)
	{
		qWarning("%s", ex.what());

		encodeWindow->close();
		encodeVideoFinished();

		QMessageBox::critical(this, "OrientView - Error", QString("%1.\n\nCheck the application log for details.").arg(ex.what()), QMessageBox::Ok);
	}

	this->setCursor(Qt::ArrowCursor);
}

void MainWindow::on_actionExit_triggered()
{
	close();
}

void MainWindow::encodeVideoFinished()
{
	if (videoEncoderThread != nullptr)
	{
		videoEncoderThread->requestInterruption();
		videoEncoderThread->wait();
		delete videoEncoderThread;
		videoEncoderThread = nullptr;
	}

	if (renderOffScreenThread != nullptr)
	{
		renderOffScreenThread->requestInterruption();
		renderOffScreenThread->wait();
		delete renderOffScreenThread;
		renderOffScreenThread = nullptr;
	}

	if (videoDecoderThread != nullptr)
	{
		videoDecoderThread->requestInterruption();
		videoDecoderThread->wait();
		delete videoDecoderThread;
		videoDecoderThread = nullptr;
	}

	if (encodeWindow != nullptr && encodeWindow->getIsInitialized())
		encodeWindow->getContext()->makeCurrent(encodeWindow->getSurface());

	if (inputHandler != nullptr)
	{
		delete inputHandler;
		inputHandler = nullptr;
	}

	if (videoStabilizer != nullptr)
	{
		delete videoStabilizer;
		videoStabilizer = nullptr;
	}

	if (renderer != nullptr)
	{
		delete renderer;
		renderer = nullptr;
	}

	if (videoEncoder != nullptr)
	{
		delete videoEncoder;
		videoEncoder = nullptr;
	}

	if (encodeWindow != nullptr)
	{
		encodeWindow->deleteLater();
		encodeWindow = nullptr;
	}

	if (mapImageReader != nullptr)
	{
		delete mapImageReader;
		mapImageReader = nullptr;
	}

	if (quickRouteReader != nullptr)
	{
		delete quickRouteReader;
		quickRouteReader = nullptr;
	}

	if (videoDecoder != nullptr)
	{
		delete videoDecoder;
		videoDecoder = nullptr;
	}
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

void MainWindow::on_pushButtonBrowseQuickRouteJpegFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Select QuickRoute JPEG file"));
	fileDialog.setNameFilter(tr("QuickRoute JPEG files (*.jpg);;All files (*.*)"));
	
	if (fileDialog.exec())
		ui->lineEditQuickRouteJpegFile->setText(fileDialog.selectedFiles().at(0));
}

void MainWindow::on_pushButtonBrowseMapImageFile_clicked()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	fileDialog.setWindowTitle(tr("Select map image file"));
	fileDialog.setNameFilter(tr("Image files (*.jpg *.png *.tiff *.tif);;All files (*.*)"));

	if (fileDialog.exec())
		ui->lineEditMapImageFile->setText(fileDialog.selectedFiles().at(0));
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
