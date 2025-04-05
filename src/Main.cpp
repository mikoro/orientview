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
		QCoreApplication::setOrganizationDomain("orientview.com");
		
#ifdef Q_OS_WIN32
		QCoreApplication::setOrganizationName("OrientView");
		QCoreApplication::setApplicationName("OrientView");
		QCoreApplication::addLibraryPath("data/plugins");
#else
		QCoreApplication::setOrganizationName("orientview");
		QCoreApplication::setApplicationName("orientview");
#endif
		// TODO: Make compatible with Wayland. This is a temporary workaround. 
		//qputenv("QT_QPA_PLATFORM", "xcb");

		QApplication app(argc, argv);

		QDir::setCurrent(QCoreApplication::applicationDirPath());

		logger.initialize("orientview.log");
		qInstallMessageHandler(messageHandler);

		QFontDatabase::addApplicationFont("data/fonts/dejavu-sans-bold.ttf");

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
