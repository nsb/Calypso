/***************************************************************************
 *   Copyright (C) 2007 kommute dev team                                   *
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
#include "kommute.h"
#include "hashthread.h"
#include "mutelayer.h"
#include "settings.h"
#include <QApplication>
#include <QTime>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>
#include <QCryptographicHash>
#include "MUTE/fileShare.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/formats/encodingUtils.h"
#include "addlogevent.h"
#include "sharedfilesevent.h"
#include "Utils.h"

extern char *muteInternalDirectoryTag;
char *muteShare_internalFileToEncodedPath (File * inFile);
extern class Settings *settings;
extern Kommute * pMainWin;

SHASH *Shares = NULL;
int maxShares = 0;
int nbShares = 0;

static bool loading=true;

#define BUFSIZE 65536

int percentCPU = 50;


SHASH *
getShare (char *hash)
{
  for (int i = 0; i < nbShares; i++)
  {
    if (!strcmp (Shares[i].hash, hash))
  	return &Shares[i];
  }
  return NULL;
}


bool HashThread::addSharedFile (QString fullName, QString name, QString hash,
    QString size, QString time, QString comment)
{
    QFileInfo fInfo (fullName);

    if (fInfo.exists ())
    {
        Shares[nbShares].name = strdup (name.toUtf8 ().data ());
        Shares[nbShares].size = size.toLongLong ();
        Shares[nbShares].time = new QDateTime ();
        Shares[nbShares].comment = new QString ("");
        Shares[nbShares].comment->append (comment);

        (*Shares[nbShares].time) =
        Shares[nbShares].time->fromString (time, Qt::ISODate);

        if (fInfo.size () == Shares[nbShares].size
            && qAbs ((fInfo.lastModified ().secsTo (*Shares[nbShares].time))) <= 2)
        {
            doPost (&Shares[nbShares], Utils::getFilePathFromFullName (fullName));
            nbShares++;

            if (nbShares >= maxShares)
            {
                maxShares += 1000;
                Shares =
                (SHASH *) realloc (Shares, (maxShares + 1) * sizeof (SHASH));
            }
            return true;
        }
        else
        {
            qDebug () << "file found but size or time not same " << fullName;
            return false;
        }
    }
    else
    {
        //qDebug () << "file not found " << fullName;
        return false;
    }
}

void HashThread::loadHash ()
{
    loading = true;
    printf ("loading hashes\n");

    QApplication::postEvent (eventReceiver,
       new AddLogEvent (tr("loading hashes\n")));

    Shares = (SHASH *) malloc (5001 * sizeof (SHASH));
    memset (Shares, 0, 5001 * sizeof (SHASH));
    maxShares = 5000;

    QString fullName (settings->getTempDirectory());
    fullName += "/";
    fullName += muteInternalDirectoryTag;
    fullName += "/hashes";
    QFile ficin (fullName);

    if (!ficin.open (QIODevice::ReadOnly))
    {
        loading = false;
        return;
    }

    QTextStream Stream (&ficin);
    Stream.setCodec ("UTF-8");
    QString n, h, s, t, c;

    while (!Stream.atEnd ())
    {
        n = Stream.readLine ();
        h = Stream.readLine ();
        Shares[nbShares].hash = strdup (h.toUtf8 ().data ());
        s = Stream.readLine ();
        t = Stream.readLine ();
        c = QString ("");
        QStringList list1 = s.split (" ");

        if (list1.size () > 1)
        {
            Shares[nbShares].size = list1[0].toLongLong ();
            int nbComm = list1[1].toInt ();

            while (nbComm-- > 0)
            {
                c.append (Stream.readLine () + "\n");
            }
        }

        // don't readd the same hash
        SHASH *prev;

        if ((prev = getShare (Shares[nbShares].hash)))
        {
            if (!strcmp (prev->name, n.toUtf8 ().data ()))
            {
		qWarning()<< "file "<<Shares[nbShares].hash << "already shared!";
                free(Shares[nbShares].hash);
                continue;
            }
        }

        fullName = QString (settings->getIncomingDirectory()) + "/" + n;

        if (addSharedFile (fullName, n, h, list1[0], t, c))
            continue;

        QStringList otherDir = settings->getOtherSharedPathSetting ();

	int i;
        for ( i = 0; i < otherDir.count (); i++)
        {
            if (addSharedFile (otherDir.at (i) + "/" + n, n, h, list1[0], t, c))
                break;
        }
	if (i==otherDir.count())
	{
	  qDebug() << "file not found : "<<n;
	}
    }

    printf ("%d hashes loaded\n", nbShares);
    QApplication::postEvent (eventReceiver,
       new AddLogEvent (tr("%n hashe(s) loaded.","",nbShares)));
    loading = false;
}

void HashThread::doPost (SHASH * share, const QString& path)
{
    QTextCodec *codecutf = QTextCodec::codecForName ("utf8");
    QTextCodec *codeclat = QTextCodec::codecForName ("iso8859-1");
    QString qstr;
    // try to convert string from utf8
    qstr = codecutf->toUnicode (share->name);

    // test if conversion is reversible
    if (strcmp (share->name, qstr.toUtf8 ().data ()))
        // if not reversible, it's not utf8, convert from iso8859 :
        qstr = codeclat->toUnicode (share->name);

    QApplication::postEvent (eventReceiver,
        new SharedFilesEvent (qstr, path, share->size, share->hash));
}

void HashThread::saveHash ()
{
    if(loading)
    {
        printf("don't save hash if loading in progress!\n");
        return;
    }

    QApplication::postEvent (eventReceiver,
        new AddLogEvent (tr("saving %1 hashes.").arg(nbShares)));

    QString fullName (settings->getTempDirectory());
    fullName += "/";
    fullName += muteInternalDirectoryTag;
    fullName += "/hashes.new";
    QFile qficout (fullName);

    if (!qficout.open (QIODevice::WriteOnly))
        return;

    QTextStream Stream (&qficout);

    for (int i = 0; i < nbShares; i++)
    {
        if ((!Shares[i].comment->isEmpty ())
            && (!Shares[i].comment->endsWith ("\n")))
        {
            Shares[i].comment->append ("\n");
        }

        // use write to avoid conversion from iso8859 made by qstream
        qficout.write (Shares[i].name, strlen (Shares[i].name));
        Stream << endl << Shares[i].hash << endl;
        Stream << Shares[i].size << " " << Shares[i].comment->
        count ("\n") << endl;
        Stream << Shares[i].time->toString (Qt::ISODate) << endl;

        if (not Shares[i].comment->isEmpty ())
            Stream << *Shares[i].comment;

        Stream.flush ();
    }

    qficout.close ();
    fullName = QString (settings->getTempDirectory()) + "/" +
        QString (muteInternalDirectoryTag) + "/hashes";

    QFile::remove (fullName);
    qficout.rename (fullName);
    printf ("hashes saved\n");
}

int findFilename (char *filename)
{
    for (int i = 0; i < nbShares; i++)
        if (!strcmp (filename, Shares[i].name))
            return i;

    return -1;
}



HashThread::HashThread (QObject * eventReceiver)
: QThread ()
, running (false)
, eventReceiver (eventReceiver)
{
    time0 = QTime::currentTime ();
}

HashThread::~HashThread ()
{
    wait (2000);

    if (Shares)
    {
        int i;
        for (i=0 ; i<nbShares ; i++)
        {
            free( Shares[i].name );
            delete Shares[i].comment ;
            delete Shares[i].time ;
        }
        free (Shares);
        Shares = NULL;
    }
}

void HashThread::run ()
{
    running = true;

    loadHash ();

    while (running)
    {
        doHash ();

        // sleep half an hour before retry hashing
        qDebug () << "hashes computed, wait half an hour...";

        QApplication::postEvent (eventReceiver,
            new AddLogEvent (tr("hashes computed, wait half an hour...")));

        int cpt = 3600000;

        while (running && cpt-- > 0)
        usleep (500);
    }
}


void HashThread::stop ()
{
  running = false;
  saveHash ();
}

void HashThread::doHashDir (QDirIterator & it)
{
    while (it.hasNext () && running)
    {
	it.next();

        if (!it.fileInfo ().isFile ())
            continue;

        QString fileName = it.filePath ();

        // folders that not shared to others.
        // 1. temp folders which include incomplete files.
        // 2. setting dirs which include kommute settings.
        if (fileName == NULL
                || fileName.contains (settings->getTempDirectory(), Qt::CaseInsensitive)
                || fileName.contains (settings->getSettingsDir (), Qt::CaseInsensitive))
            continue;

        if (fileName.startsWith (it.path ()))
            fileName.remove (0, it.path ().length () + 1);

        int indfic = findFilename (fileName.toUtf8 ().data ());

        if (indfic < 0
            || Shares[indfic].size != it.fileInfo ().size ()
            || qAbs ((it.fileInfo ().lastModified ().secsTo (*Shares[indfic].time)) > 2))
        {
            QFile ficin (it.filePath ());
            QFileInfo ficinfo = QFileInfo (ficin);

            if(!ficinfo.isFile())
                continue;

            qDebug () << "hashing  " << fileName;

            // generate a new hash
            QCryptographicHash hash (QCryptographicHash::Sha1);
            ficin.open (QIODevice::ReadOnly);
            QByteArray buf;

            do
            {
                buf = ficin.read (BUFSIZE);
                hash.addData (buf);

                //limit CPU usage by sleeping when more than 10 ms elapsed
                if (time0.elapsed () > 10)
                {
                    int toSleep = (time0.elapsed () * (100 - percentCPU)) / percentCPU;

                    while (toSleep > 10 && running)
                    {
                        usleep (10000);
                        toSleep -= 10;
                    }
                    time0 = QTime::currentTime ();
                }
            } while (buf.size () == BUFSIZE && running);

            if (running)
            {
                qDebug () << "hashed  " << fileName;

                // cache it
                Shares[nbShares].name = strdup (fileName.toUtf8 ().data ());
                Shares[nbShares].hash = strdup (hash.result ().toHex ().toUpper ().data ());
                Shares[nbShares].time = new QDateTime (ficinfo.lastModified ());
                Shares[nbShares].comment = new QString ("");
                Shares[nbShares++].size = ficin.size ();

                doPost (&Shares[nbShares - 1], it.path ());
                saveHash ();

                if (nbShares >= maxShares)
                {
                    maxShares += 1000;
                    Shares = (SHASH *) realloc (Shares, (maxShares + 1) * sizeof (SHASH));
                }
            }
        }
    }
}

void HashThread::doHash ()
{
    // make sure hash dir exists
    QString hashPath = settings->getTempDirectory().append("/").append(muteInternalDirectoryTag);
    QDir hashDirectory (hashPath);

    if (!hashDirectory.exists ())
        hashDirectory.mkpath (hashPath);

    // Incoming folder is the default sharing folder. (automatially shared around kommute)
    QString incomingPath = settings->getIncomingDirectory ();
    QDir incomingDirectory (incomingPath);

    if (!incomingDirectory.exists ())
        incomingDirectory.mkpath (incomingPath);

    doHashDir (incomingPath);


    QStringList otherDir = settings->getOtherSharedPathSetting ();
    for (int i = 0; i < otherDir.count (); i++)
    {
        doHashDir (otherDir.at (i));
    }
}

void HashThread::doHashDir (const QString & folderName) {
    QDirIterator it (folderName, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    doHashDir (it);
}

