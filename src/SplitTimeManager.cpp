// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "SplitTimeManager.h"
#include "Settings.h"

using namespace OrientView;

void SplitTimeManager::initialize(Settings* settings)
{
	QStringList timeStrings = settings->splits.splitTimes.split(QRegExp("[;|]"), QString::SkipEmptyParts);
	SplitTimeType type = settings->splits.type;

	double totalTime = 0.0;

	for (const QString& timeString : timeStrings)
	{
		QStringList timeParts = timeString.split(QRegExp("[.:]"), QString::SkipEmptyParts);

		double hours = 0.0;
		double minutes = 0.0;
		double seconds = 0.0;

		if (timeParts.length() == 1)
		{
			seconds = timeParts.at(1).toDouble();
		}

		if (timeParts.length() == 2)
		{
			minutes = timeParts.at(0).toDouble();
			seconds = timeParts.at(1).toDouble();
		}

		if (timeParts.length() == 3)
		{
			hours = timeParts.at(0).toDouble();
			minutes = timeParts.at(1).toDouble();
			seconds = timeParts.at(2).toDouble();
		}

		SplitTime splitTime;
		splitTime.position = 0;

		if (type == SplitTimeType::Absolute)
		{
			splitTime.time = (hours * 3600.0 + minutes * 60.0 + seconds);
		}
		else
		{
			totalTime += (hours * 3600.0 + minutes * 60.0 + seconds);
			splitTime.time = totalTime;
		}

		defaultSplitTimes.splitTimes.push_back(splitTime);
	}
}

const SplitTimes& SplitTimeManager::getDefaultSplitTimes() const
{
	return defaultSplitTimes;
}
