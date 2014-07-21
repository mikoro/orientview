// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QMainWindow>

namespace Ui
{
	class MainView;
}

class MainView : public QMainWindow
{
	Q_OBJECT

public:

	explicit MainView(QWidget *parent = 0);
	~MainView();

private slots:

	void on_pushButtonTest_clicked();

private:

	Ui::MainView *ui;
};
