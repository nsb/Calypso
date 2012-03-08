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

#include <QFileDialog>
#include <QStringList>

#include "settingshelper.h"

/**
@author Andy Gebauer - feivelda@googlemail.com
*/

bool SettingsHelper::sharePathDialog(QWidget *parent, QString &path)
{
    path = "";
    QStringList pathList;
    QFileDialog *fd = new QFileDialog(parent);
    // set the options for the QFileDialog
    fd->setFileMode(QFileDialog::DirectoryOnly);
    fd->setViewMode(QFileDialog::List);
    fd->setReadOnly(true);
    fd->setDirectory(QDir::homePath());
    fd->setLabelText(QFileDialog::FileType, QObject::tr("Choose a Directory"));

    // show the QFileDialog modal
    if ( fd->exec() )
    {
        // give the directory back, as QString
        pathList = fd->selectedFiles();
        // only one dir can be selected -> position 0
        path = pathList.at(0);
        // show that the QFileDialog was exited by Ok button
        return true;
    }
    // show that the QFileDialog was exited by Cancel button
    return false;
}
