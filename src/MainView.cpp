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

void MainView::on_pushButtonTest_clicked()
{
	qDebug("Test!");
}
