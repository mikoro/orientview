// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <typeinfo>

#include <QApplication>
#include <QFontDatabase>

#include "MainWindow.h"
#include "SimpleLogger.h"

namespace
{
	OrientView::SimpleLogger logger("orientview.log");

	void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
	{
		logger.handleMessage(type, context, message);
	}
}

int main(int argc, char *argv[])
{
	try
	{
		qInstallMessageHandler(messageHandler);

		QCoreApplication::setOrganizationName("Mikko Ronkainen");
		QCoreApplication::setOrganizationDomain("mikkoronkainen.com");
		QCoreApplication::setApplicationName("OrientView");
		QCoreApplication::addLibraryPath("data/plugins");

		QApplication app(argc, argv);

		QFontDatabase::addApplicationFont("data/fonts/dejavu-sans.ttf");
		QFontDatabase::addApplicationFont("data/fonts/dejavu-sans-bold.ttf");
		QFontDatabase::addApplicationFont("data/fonts/dejavu-sans-mono.ttf");
		QFontDatabase::addApplicationFont("data/fonts/dejavu-sans-mono-bold.ttf");
		
		OrientView::MainWindow mainWindow;
		mainWindow.show();
		app.exec();
	}
	catch (const std::exception& ex)
	{
		qFatal("Exception (%s): %s", typeid(ex).name(), ex.what());
	}
}
