#include "Utils.h"
#include <QFileInfo>

QString Utils::getFileNameFromFullName (const QString& fullPath)
{
    QFileInfo fileInfo (fullPath);
    return fileInfo.fileName ();
}

QString Utils::getFilePathFromFullName (const QString& fullPath)
{
    QFileInfo fileInfo (fullPath);
    return fileInfo.absolutePath ();
}

