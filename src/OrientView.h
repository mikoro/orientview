#ifndef ORIENTVIEW_H
#define ORIENTVIEW_H

#include <QtWidgets/QMainWindow>
#include "ui_OrientView.h"

class OrientView : public QMainWindow
{
	Q_OBJECT

public:
	OrientView(QWidget *parent = 0);
	~OrientView();

private:
	Ui::OrientViewClass ui;
};

#endif // ORIENTVIEW_H
