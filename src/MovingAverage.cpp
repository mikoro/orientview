// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "MovingAverage.h"

using namespace OrientView;

void MovingAverage::setAlpha(double value)
{
	alpha = value;
}

void MovingAverage::addMeasurement(double value)
{
	averageValue = alpha * value + (1.0 - alpha) * previousAverageValue;
	previousAverageValue = averageValue;
}

void MovingAverage::addMeasurement(double value, double frameTime)
{
	averageValue = (alpha * frameTime) * value + (1.0 - (alpha * frameTime)) * previousAverageValue;
	previousAverageValue = averageValue;
}

double MovingAverage::getAverage() const
{
	return averageValue;
}

void MovingAverage::reset(double value)
{
	averageValue = previousAverageValue = value;
}
