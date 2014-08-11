// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QFile>

namespace OrientView
{
	class MainWindow;

	// Log all the messages to file and to other possible debug outputs.
	class SimpleLogger
	{

	public:

		~SimpleLogger();

		void initialize(const QString& fileName, MainWindow* mainWindow);
		void handleMessage(QtMsgType type, const QMessageLogContext& context, const QString& message);

	private:

		QFile logFile;
		MainWindow* mainWindow;
	};
}
