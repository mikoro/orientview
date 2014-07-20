#ifndef ORIENTVIEW_H
#define ORIENTVIEW_H

#include <QtWidgets/QMainWindow>
#include "ui_MainView.h"

class MainView : public QMainWindow
{
	Q_OBJECT

public:
	MainView(QWidget *parent = 0);
	~MainView();

private:
	Ui::MainViewClass ui;
};

#endif // ORIENTVIEW_H
