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
	smoothValue = alpha * value + (1.0 - alpha) * previousSmoothValue;
	previousSmoothValue = smoothValue;
}

double MovingAverage::getAverage() const
{
	return smoothValue;
}

void MovingAverage::reset()
{
	smoothValue = previousSmoothValue = 0.0;
}
