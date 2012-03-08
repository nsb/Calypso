/***************************************************************************
 *   Copyright (C) 2007 kommute team                                       *
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
#ifndef HASHTHREAD_H
#define HASHTHREAD_H

#include "minorGems/crypto/hashes/sha1.h"
#include "minorGems/io/file/File.h"
#include <QThread>
#include <QTime>
#include <QString>
#include <QDirIterator>

typedef struct sharehash
{
    char *name;
    char *hash;
    qint64 size;
    QDateTime *time;
    QString *comment;
} SHASH;


SHASH * getShare(char * hash);

class HashThread : public QThread
{
	Q_OBJECT
public:
    HashThread( QObject *eventReceiver);

    ~HashThread();

    virtual void run();
    void stop();


private:
    bool running;
    QObject *eventReceiver;
    bool addSharedFile(QString fullName,QString name,QString hash,QString size,QString time,QString comment);
    void doHash();
    void doHashDir (const QString & folderName);
    void doHashDir(QDirIterator &it);
    void loadHash();
    void saveHash();
    void doPost (SHASH * share, const QString& path);
    QTime time0;
};

#endif
