// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <vector>

#include <QString>

namespace OrientView
{
	class Settings;

	enum SplitTimeType { Absolute, Relative };

	struct Split
	{
		int position = 0;
		double absoluteTime = 0.0;
	};

	struct RunnerInfo
	{
		int finalPosition = 0;
		QString name = "";
		std::vector<Split> splits;
	};

	class SplitsManager
	{

	public:

		void initialize(Settings* settings);

		const RunnerInfo& getDefaultRunnerInfo() const;

	private:

		RunnerInfo defaultRunnerInfo;
	};
}
