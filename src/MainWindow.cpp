// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QFileDialog>
#include <QtGUI>

#include "MainWindow.h"

using namespace OrientView;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	ui = std::unique_ptr<Ui::MainWindow>(new Ui::MainWindow());
	ui->setupUi(this);

	connect(&videoWindow, SIGNAL(closing()), this, SLOT(videoWindowClosing()));

	readSettings();
}

MainWindow::~MainWindow()
{
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
		if(!ffmpegDecoder.initialize(ui->lineEditVideoFile->text().toStdString()))
			throw std::runtime_error("Could not initialize FFmpegDecoder");

		videoWindow.showNormal();

		if (!videoWindow.initialize())
			throw std::runtime_error("Could not initialize VideoWindow");

		if (!videoRenderer.initialize())
			throw std::runtime_error("Could not initialize VideoRenderer");

		renderOnScreenThread.initialize(&videoWindow, &videoRenderer, &ffmpegDecoder);
		videoWindow.getContext()->doneCurrent();
		videoWindow.getContext()->moveToThread(&renderOnScreenThread);
		renderOnScreenThread.start();
	}
	catch (const std::exception& ex)
	{
		qWarning("Could not run video: %s", ex.what());
		videoWindow.close();
		videoWindowClosing();
	}

	this->setCursor(Qt::ArrowCursor);
}

void MainWindow::on_pushButtonEncode_clicked()
{
}

void MainWindow::videoWindowClosing()
{
	renderOnScreenThread.requestInterruption();
	renderOnScreenThread.wait();

	videoWindow.shutdown();
	ffmpegDecoder.shutdown();

	this->activateWindow();
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
