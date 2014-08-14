// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "SplitTimeManager.h"
#include "Settings.h"

using namespace OrientView;

void SplitTimeManager::initialize(Settings* settings)
{
	QStringList timeStrings = settings->splits.splitTimes.split(QRegExp("[;|]"), QString::SkipEmptyParts);

	for (int i = 0; i < timeStrings.size(); ++i)
	{
		QStringList timeParts = timeStrings.at(i).split(QRegExp("[.:]"), QString::SkipEmptyParts);

		if (timeParts.length() == 2)
		{
			double minutes = timeParts.at(0).toDouble();
			double seconds = timeParts.at(1).toDouble();

			splitTimes.push_back(minutes * 60.0 + seconds);
		}
	}
}

const std::vector<double>& SplitTimeManager::getSplitTimes() const
{
	return splitTimes;
}
