// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "MainView.h"
#include "SimpleLogger.h"
#include <QApplication>

namespace
{
	OrientView::SimpleLogger logger("orientview.log");

	void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
	{
		logger.handleMessage(type, context, message);
	}
}

int main(int argc, char *argv[])
{
	try
	{
		qInstallMessageHandler(messageHandler);
		QApplication app(argc, argv);
		MainView mainView;
		mainView.show();
		app.exec();
	}
	catch (const std::exception& ex)
	{
		qFatal("Exception (%s): %s", typeid(ex).name(), ex.what());
	}
}
