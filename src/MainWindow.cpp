// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QFileDialog>
#include <QtGUI>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "FFmpegDecoder.h"

using namespace OrientView;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	readSettings();
}

MainWindow::~MainWindow()
{
	delete ui;
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
	
}

void MainWindow::on_pushButtonEncode_clicked()
{

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
