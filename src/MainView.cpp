// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "MainView.h"
#include "ui_MainView.h"

MainView::MainView(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainView)
{
	ui->setupUi(this);
}

MainView::~MainView()
{
	delete ui;
}

void MainView::on_pushButtonOpen_clicked()
{
	decoder.Open("testvideo.mp4");
}

void MainView::on_pushButtonClose_clicked()
{
	decoder.Close();
}

void MainView::on_pushButtonGet_clicked()
{
	decoder.GetNextFrame();
}
