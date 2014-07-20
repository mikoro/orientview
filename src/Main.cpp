#include "MainView.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainView w;
	w.show();
	return a.exec();
}
