/***************************************************************************
 *   Copyright (C) 2009                                                    *
 *   summitsummit@163.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "settings.h"
#include "KommuteApplication.h"
#include <QTranslator>
#include <QDebug>

extern Settings* settings;

QHash <QString, QTranslator*> KommuteApplication::translatorList;
QHash <QString, QTranslator*> KommuteApplication::translatorQtList;
QTranslator* KommuteApplication::currentTranslator = NULL;
QTranslator* KommuteApplication::currentQtTranslator = NULL;
QString KommuteApplication::currentLanguage;

void KommuteApplication::loadTranslations(const QDir& dir)
{
    // Kommute_<locale>.qm
    QString FIX_PREFIX = "Kommute_";
    QString filter = FIX_PREFIX + "*.qm";
    QDir::Filters filters = QDir::Files | QDir::Readable;
    QDir::SortFlags sort = QDir::Name;
    QFileInfoList entries = dir.entryInfoList(QStringList() << filter, filters, sort);

    translatorList.insert (DEFAULT_LANGUAGE, NULL);

    foreach (QFileInfo file, entries)
    {
        // pick locale out of the file name
        QString locale = file.baseName().mid (FIX_PREFIX.size ());

        qDebug () << "try to load translator for: " << locale;

        // construct and load translator
        QTranslator* translator = new QTranslator (instance ());

        if (translator->load (file.absoluteFilePath()))
        {
            translatorList.insert (locale, translator);
        }
    }
    // qt_<locale>.qm
    FIX_PREFIX = "qt_";
    filter = FIX_PREFIX + "*.qm";
    entries = dir.entryInfoList(QStringList() << filter, filters, sort);

    translatorList.insert (DEFAULT_LANGUAGE, NULL);

    foreach (QFileInfo file, entries)
    {
        // pick locale out of the file name
        QString locale = file.baseName().mid (FIX_PREFIX.size ());

        qDebug () << "try to load translator for: " << locale;

        // construct and load translator
        QTranslator* translator = new QTranslator (instance ());

        if (translator->load (file.absoluteFilePath()))
        {
            translatorQtList.insert (locale, translator);
        }
    }
}

const QString& KommuteApplication::getLanguage ()
{
    return currentLanguage;
}

bool KommuteApplication::setLanguage(const QString& locale)
{
    // remove current translator if have.
    //if (currentQtTranslator)
    {
        removeTranslator (currentQtTranslator);
    }

    currentQtTranslator = translatorQtList.value (locale, 0);

    // install new translator refer to locale.
    //if (currentQtTranslator)
    {
        installTranslator (currentQtTranslator);
    }

    // remove current translator if have.
    //if (currentTranslator)
    {
        removeTranslator (currentTranslator);
    }

    currentTranslator = translatorList.value (locale, 0);

    if (!currentTranslator)
        currentLanguage = DEFAULT_LANGUAGE;
    else
        currentLanguage = locale;

    // install new translator refer to locale.
    //if (currentTranslator)
    {
        installTranslator (currentTranslator);
    }

    return currentTranslator != NULL;
}

void KommuteApplication::loadTranslations(const QString& dir)
{
    KommuteApplication::loadTranslations (QDir(dir));
}

const QStringList KommuteApplication::availableLanguages()
{
    return QStringList (translatorList.keys ());
}

KommuteApplication::KommuteApplication (int& argc, char** argv)
: QApplication (argc, argv)
{
}

KommuteApplication::~KommuteApplication ()
{
}

