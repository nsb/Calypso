/***************************************************************************
 *   Copyright (C) 2006 by Niels Sandholt Busch                            *
 *   niels@diku.dk                                                         *
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

#include <QtGui>
#include <QtDebug>

#include "kommute.h"
#include "defaultsettings.h"
#include "KommuteApplication.h"

char *settingsDir;

int main(int argc, char *argv[])
{
    if (argc >= 3 && !strcmp(argv[1],"-s"))
    {
        settingsDir = strdup(argv[2]);
    }
    else
    {
        settingsDir = DEFAULT_SETTINGS_SETTINGSDIR;
    }

    KommuteApplication app(argc, argv);
    app.setApplicationName("Kommute");

    QString locale = QLocale::system().name();
    QString localefile = QString(TRANSLATIONS_NAME) + locale;

    qDebug() << "Language code found:" << locale;
    qDebug() << "Try to load language file:" << localefile;

    KommuteApplication::loadTranslations (TRANSLATIONS_PATH);
    KommuteApplication::loadTranslations ("./translations");
    KommuteApplication::loadTranslations ("/usr/share/qt4/translations");

    if (!KommuteApplication::setLanguage (locale))
    {
        qDebug () << "Load language code fails.";
	if(locale.length() >2) locale=locale.left(2);
	{
          qDebug () << "Try short name.";
          if (!KommuteApplication::setLanguage (locale))
            qDebug () << "Load language code fails.";
	}
    }

    Kommute mainWin;

    if (2 == argc)
    {
        mainWin.handleDownloadRequest(argv[1]);
    }

    if(Kommute::checkIfAutorunWithWindows())mainWin.hide();
    else mainWin.show();

    int ret = app.exec();
    return ret;
}
