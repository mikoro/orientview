// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <vector>

namespace OrientView
{
	class Settings;

	class SplitTimeManager
	{

	public:

		void initialize(Settings* settings);

		const std::vector<double>& getSplitTimes() const;

	private:

		std::vector<double> splitTimes;
	};
}
