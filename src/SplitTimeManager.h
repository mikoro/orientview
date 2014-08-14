// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <vector>

#include <QString>

namespace OrientView
{
	class Settings;

	struct SplitTime
	{
		double time = 0.0;
		int position = 0;
	};

	struct SplitTimes
	{
		QString name = "";
		int position = 0;
		std::vector<SplitTime> splitTimes;
	};

	class SplitTimeManager
	{

	public:

		void initialize(Settings* settings);

		const SplitTimes& getDefaultSplitTimes() const;

	private:

		SplitTimes defaultSplitTimes;
	};
}
