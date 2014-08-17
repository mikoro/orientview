// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "VideoStabilizerThread.h"
#include "VideoDecoder.h"
#include "VideoStabilizer.h"
#include "Settings.h"
#include "FrameData.h"

using namespace OrientView;

bool VideoStabilizerThread::initialize(VideoDecoder* videoDecoder, VideoStabilizer* videoStabilizer, Settings* settings)
{
	this->videoDecoder = videoDecoder;
	this->videoStabilizer = videoStabilizer;

	outputFile.setFileName(settings->stabilizer.passOneOutputFilePath);

	if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
	{
		qWarning("Could not open output file");
		return false;
	}

	outputFile.write("timeStamp;cumulativeX;cumulativeY;cumulativeAngle\n");
	return true;
}

void VideoStabilizerThread::run()
{
	FrameData frameDataGrayscale;

	while (!isInterruptionRequested())
	{
		if (videoDecoder->getNextFrame(nullptr, &frameDataGrayscale))
		{
			videoStabilizer->preProcessFrame(frameDataGrayscale, outputFile);
			emit frameProcessed(frameDataGrayscale.cumulativeNumber);
		}
		else if (videoDecoder->getIsFinished())
			break;
	}

	if (outputFile.isOpen())
		outputFile.close();

	emit processingFinished();
}
