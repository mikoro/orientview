// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <iostream>

#include <QTime>

#ifdef _WIN32
#include <windows.h>
#endif

#include "SimpleLogger.h"
#include "MainWindow.h"

using namespace OrientView;

SimpleLogger::~SimpleLogger()
{
	if (logFile.isOpen())
		logFile.close();
}

void SimpleLogger::initialize(const QString& fileName)
{
	logFile.setFileName(fileName);
	logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
}

void SimpleLogger::setMainWindow(MainWindow* mainWindow)
{
	this->mainWindow = mainWindow;
}

void SimpleLogger::handleMessage(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
	Q_UNUSED(context);

	QString typeString;

	switch (type)
	{
		case QtDebugMsg:
			typeString = "Debug";
			break;
		case QtWarningMsg:
			typeString = "Warning";
			break;
		case QtCriticalMsg:
			typeString = "Critical";
			break;
		case QtFatalMsg:
			typeString = "Fatal";
	}

	QString timeString = QTime::currentTime().toString("HH:mm:ss.zzz");
	QString messageText = QString("%1 [%2] - %3\n").arg(timeString, typeString, message);

	std::cout << messageText.toStdString() << std::endl;

	if (logFile.isOpen())
	{
		logFile.write(messageText.toUtf8());
		logFile.flush();
	}

#ifdef _WIN32
	OutputDebugStringA(qPrintable(messageText));
#endif

	if (mainWindow != nullptr)
		mainWindow->addLogMessage(timeString, typeString, message);

	if (type == QtFatalMsg)
		abort();
}
