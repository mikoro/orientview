// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <fstream>
#include <iostream>
#include <windows.h>

namespace OrientView
{
	class SimpleLogger
	{
	public:

		SimpleLogger(const std::string& fileName)
		{
			logFile.open(fileName, std::ios_base::out);
		}

		~SimpleLogger()
		{
			logFile.close();
		}

		void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &message)
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

			std::string messageText = QString("%1: %2 (%3:%4, %5)\n").arg(typeString, message, QString::fromLatin1(context.file), QString::number(context.line), QString::fromLatin1(context.function)).toStdString();
			//std::string messageText = QString("%1: %2\n").arg(typeString, message).toStdString();

			if (logFile.is_open())
			{
				logFile << messageText;
				logFile.flush();
			}

			std::cout << messageText;
			std::cout.flush();

			OutputDebugStringA(messageText.c_str());

			if (type == QtFatalMsg)
				abort();
		}

	private:

		std::ofstream logFile;
	};
}
