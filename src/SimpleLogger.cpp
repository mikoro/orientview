// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#ifdef _WIN32
#include <windows.h>
#endif

#include "SimpleLogger.h"

using namespace OrientView;

SimpleLogger::SimpleLogger(const QString& fileName)
{
	logFile.setFileName(fileName);
	logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
}

SimpleLogger::~SimpleLogger()
{
	logFile.close();
}

void SimpleLogger::handleMessage(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
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

	QString messageText = QString("%1: %2\n").arg(typeString, message);

	if (logFile.isOpen())
	{
		logFile.write(messageText.toUtf8());
		logFile.flush();
	}

#ifdef _WIN32
	OutputDebugStringA(qPrintable(messageText));
#endif

	if (type == QtFatalMsg)
		abort();
}
