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
#ifndef KOMMUTE_APPLICATION_H
#define KOMMUTE_APPLICATION_H

#include <QApplication>
#include <QStringList>
#include <QHash>
#include <QDir>

#ifdef Q_WS_WIN
#define TRANSLATIONS_PATH   "translations"
#elif defined Q_WS_X11
#define TRANSLATIONS_PATH  "/usr/share/apps/kommute/translations"
#endif

#define TRANSLATIONS_NAME   "kommute_"
#define DEFAULT_LANGUAGE    "Default"

class QTranslator;

class KommuteApplication
    : public QApplication
{
    Q_OBJECT

public:
    KommuteApplication (int& argc, char** argv);
    ~KommuteApplication ();

    static void loadTranslations(const QString& dir);
    static void loadTranslations(const QDir& dir);
    static const QStringList availableLanguages();

    static bool setLanguage(const QString& locale);
    static const QString& getLanguage ();

private:
    static QHash <QString, QTranslator*> translatorList;
    static QHash <QString, QTranslator*> translatorQtList;
    static QTranslator* currentTranslator;
    static QTranslator* currentQtTranslator;
    static QString currentLanguage;
};

#endif
