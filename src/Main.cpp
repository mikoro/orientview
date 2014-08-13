// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <typeinfo>

#include <QApplication>
#include <QDir>
#include <QFontDatabase>

#include "MainWindow.h"
#include "SimpleLogger.h"

namespace
{
	OrientView::SimpleLogger logger;

	void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
	{
		logger.handleMessage(type, context, message);
	}
}

int main(int argc, char *argv[])
{
	try
	{
		QApplication app(argc, argv);

		QDir::setCurrent(QCoreApplication::applicationDirPath());

		logger.initialize("orientview.log");
		qInstallMessageHandler(messageHandler);

		QCoreApplication::setOrganizationName("OrientView");
		QCoreApplication::setOrganizationDomain("orientview.com");
		QCoreApplication::setApplicationName("OrientView");
		QCoreApplication::addLibraryPath("data/plugins");

		QFontDatabase::addApplicationFont("data/fonts/dejavu-sans.ttf");
		QFontDatabase::addApplicationFont("data/fonts/dejavu-sans-bold.ttf");
		QFontDatabase::addApplicationFont("data/fonts/dejavu-sans-mono.ttf");
		QFontDatabase::addApplicationFont("data/fonts/dejavu-sans-mono-bold.ttf");

		OrientView::MainWindow mainWindow;

		logger.setMainWindow(&mainWindow);

		if (argc >= 2 && QFile::exists(QString(argv[1])))
			mainWindow.readSettingsFromIniFile(QString(argv[1]));
		else
			mainWindow.readSettingsFromLocal();

		mainWindow.show();
		app.exec();

		mainWindow.writeSettingsToLocal();
	}
	catch (const std::exception& ex)
	{
		qFatal("Exception (%s): %s", typeid(ex).name(), ex.what());
	}
}
