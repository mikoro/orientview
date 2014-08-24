// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "SplitsManager.h"
#include "Settings.h"

using namespace OrientView;

void SplitsManager::initialize(Settings* settings)
{
	QStringList timeStrings = settings->splits.splitTimes.split(QRegExp("[;|]"), QString::SkipEmptyParts);
	SplitTimeType timeType = settings->splits.type;

	double totalTime = 0.0;

	// implicit first split at time zero
	defaultRunnerInfo.splits.push_back(Split());

	for (const QString& timeString : timeStrings)
	{
		QStringList timeParts = timeString.split(QRegExp("[.:]"), QString::SkipEmptyParts);

		double hours = 0.0;
		double minutes = 0.0;
		double seconds = 0.0;

		if (timeParts.length() == 1)
		{
			seconds = timeParts.at(0).toDouble();
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

		Split split;

		if (timeType == SplitTimeType::Absolute)
		{
			split.absoluteTime = (hours * 3600.0 + minutes * 60.0 + seconds);
		}
		else
		{
			totalTime += (hours * 3600.0 + minutes * 60.0 + seconds);
			split.absoluteTime = totalTime;
		}

		defaultRunnerInfo.splits.push_back(split);
	}
}

const RunnerInfo& SplitsManager::getDefaultRunnerInfo() const
{
	return defaultRunnerInfo;
}
