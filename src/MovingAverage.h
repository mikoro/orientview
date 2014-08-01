// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

namespace OrientView
{
	class MovingAverage
	{

	public:

		void setAlpha(double value);
		void addMeasurement(double value);
		double getAverage() const;

	private:

		double alpha = 1.0;
		double smoothValue = 0.0;
		double previousSmoothValue = 0.0;
	};
}
