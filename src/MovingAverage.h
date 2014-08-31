// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

namespace OrientView
{
	// Calculate exponential moving average.
	class MovingAverage
	{

	public:

		void setAlpha(double value);
		void addMeasurement(double value);
		void addMeasurement(double value, double frameTime);
		double getAverage() const;
		void reset(double value);

	private:

		double alpha = 1.0;
		double averageValue = 0.0;
		double previousAverageValue = 0.0;
	};
}
