// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <cstdint>

namespace OrientView
{
	// Contains the frame data that is passed around from one stage to another.
	struct FrameData
	{
		uint8_t* data = nullptr;		// Raw data, format depends on context (RGBA32 or GRAY8)
		size_t dataLength = 0;			// Data length in bytes
		size_t rowLength = 0;			// Length of the row in bytes (could be larger than width)
		int width = 0;					// Width in pixels
		int height = 0;					// Height in pixels
		int64_t duration = 0;			// Duration in microseconds
		int64_t timeStamp = 0;			// Time stamp given by FFmpeg (no unit)
		int64_t cumulativeNumber = 0;	// Total number of frames produced (doesn't reset on seek)
	};
}
