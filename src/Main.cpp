#include "OrientView.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	OrientView w;
	w.show();
	return a.exec();
}
