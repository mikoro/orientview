// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>

#include "GpxReader.h"

using namespace OrientView;

bool GpxReader::initialize(const QString& fileName)
{
	qDebug("Initializing GpxReader (%s)", qPrintable(fileName));

	QFile file(fileName);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning("Could not open GPX file");
		return false;
	}

	QXmlStreamReader xmlStream(&file);
	trackPoints.clear();

	TrackPoint trackPoint;

	while (!xmlStream.atEnd() && !xmlStream.hasError())
	{
		xmlStream.readNext();

		if (xmlStream.isStartElement())
		{
			if (xmlStream.name() == "trkpt")
			{
				trackPoint = TrackPoint();

				trackPoint.latitude = xmlStream.attributes().value("lat").toDouble();
				trackPoint.longitude = xmlStream.attributes().value("lon").toDouble();

				continue;
			}
			
			if (xmlStream.name() == "time")
			{
				xmlStream.readNext();
				trackPoint.dateTime = QDateTime::fromString(xmlStream.text().toString(), Qt::ISODate);

				continue;
			}

			if (xmlStream.name() == "ele")
			{
				xmlStream.readNext();
				trackPoint.elevation = xmlStream.text().toString().toDouble();

				continue;
			}

			if (xmlStream.name() == "hr")
			{
				xmlStream.readNext();
				trackPoint.heartRate = xmlStream.text().toString().toDouble();
			}
		}

		if (xmlStream.isEndElement())
		{
			if (xmlStream.name() == "trkpt")
				trackPoints.push_back(trackPoint);
		}
	}

	if (xmlStream.hasError())
		qWarning("There was an error while parsing GPX: %s", qPrintable(xmlStream.errorString()));

	return true;
}

const std::vector<TrackPoint>& GpxReader::getTrackPoints() const
{
	return trackPoints;
}
