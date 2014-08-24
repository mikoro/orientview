// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>

namespace OrientView
{
	struct Mp4Handle;

	// Encapsulate the l-smash library for writing out video files in MP4 format.
	class Mp4File
	{

	public:

		bool open(const QString& fileName);
		bool setParameters(x264_param_t* param);
		bool writeHeaders(x264_nal_t* nal);
		bool writeFrame(uint8_t* payload, size_t size, x264_picture_t* picture);
		void close(int64_t lastPts);

	private:

		Mp4Handle* mp4Handle = nullptr;
	};
}
