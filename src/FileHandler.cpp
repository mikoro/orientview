// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "FileHandler.h"

#include <qcoreapplication.h>
#include <QStandardPaths>

static QDir* getDataDirectory()
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    // Fallback: try to find data in the application directory
    paths.append(QCoreApplication::applicationDirPath());

    for (const QString& basePath : paths)
    {
        QDir* dataDir = new QDir(QDir(basePath).filePath("data"));
        if (dataDir->exists())
            return dataDir;
    }

    return nullptr;
}

QString getDataFilePath(QString relativeFilePath)
{
    QDir* dataDir = getDataDirectory();
    if (dataDir == nullptr)
    {
        qWarning("Data directory not found");
        return QString();
    }

    QString dataFilePath = dataDir->filePath(relativeFilePath);
    delete dataDir;
    
    return dataFilePath;
}