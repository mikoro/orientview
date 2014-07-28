// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#pragma once

#include <QString>

namespace OrientView
{
	struct MP4Handle;

	class MP4File
	{

	public:

		MP4File();

		bool initialize(const QString& fileName);
		void shutdown();

		bool setParam(x264_param_t* param);
		bool writeHeaders(x264_nal_t* nal);
		bool writeFrame(uint8_t* payload, int size, x264_picture_t* picture);
		bool finalize(int64_t lastPts);
		
	private:

		MP4Handle* mp4Handle = nullptr;
	};
}
