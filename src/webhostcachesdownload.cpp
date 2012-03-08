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

/**
@author Andy Gebauer - feivelda@googlemail.com
*/

#include <QUrl>
//#include <QByteArray>
#include <QtDebug>
#include <QMessageBox>
#include <time.h>

#include "minorGems/network/HostAddress.h"
#include "minorGems/network/p2pParts/HostCatcher.h"

#include "webhostcachesdownload.h"
#include "defaultsettings.h"
#include "settings.h"
#include "applog.h"

bool newRelease=false;

extern HostCatcher *muteHostCatcher;
extern class Settings *settings;

WebHostCachesDownload::WebHostCachesDownload( QStringList oldData, QStringList downloadLocations )
:running(false)
{
    downloadID=0;
    myIP_ID=0;
    listCount=0;
    mTimeToWaitBetweenAddressPostsInSeconds = settings->getWebHostCachePostIntervalSetting();
    // 0-time (very long ago) to ensure that we post once at startup
    mTimeOfLastAddressPostInSeconds=0;
    // initialize http object
    http = new QHttp();
    if (! settings->getProxyHost().isEmpty())
    {
      http->setProxy(settings->getProxyHost(),settings->getProxyPort(),settings->getProxyUser(),settings->getProxyPassword());
    }
    // initialize a byte array for the buffer
    byteArray = new QByteArray();
    // initialize the read buffer from the byteArray
    buffer = new QBuffer(byteArray);
    // initialize the webhostcacheslist empty
    webHostCachesList.clear();
    // initialize the downloadList
    list = downloadLocations;
    /* initialize the semaphore object with 0 free objects
       the semaphore is needed because the settings class
       should only start when the list is downloaded or when
       this download fails*/
    /* if you don't know what a semaphore is or how it works,
       get a book about Operating System Design*/
    semaphore = new QSemaphore(1);
    // connect slots
    connect(http, SIGNAL(requestFinished(int, bool)),
            this, SLOT(httpRequestFinished(int, bool)));
    // prepate the standard list from the static values in defaultsettings.h
    prepareList( oldData );
}

WebHostCachesDownload::~WebHostCachesDownload()
{
    delete http;
    delete byteArray;
    delete buffer;
    delete semaphore;
}

void WebHostCachesDownload::run()
{
  running = true;
  QString temp;
  // when the given list is empty, use the static one
  if ( list.isEmpty() )
  {
        // the url where we get the WebHostCaches
        temp = WEBHOSTCACHESLISTDOWNLOAD;
        // make a list out of the string
        list = temp.split(LIST_SEPARATOR);
  }
  qDebug() <<"search for new release";
  QUrl *url = new QUrl("http://kommute.sourceforge.net/release.txt");
  http->setHost( url->host() );
  // and get the file. The ID of the transaction is stored in myIP_ID
  semaphore->tryAcquire(1,1000);
  release_ID = http->get( url->path());
  delete url;
  while (running)
  {
    if (settings->getMyIP() == "")
    {
      qDebug() <<"search for my IP";
      url = new QUrl("http://kommute.sourceforge.net/getip.php");
      http->setHost( url->host() );
      // and get the file. The ID of the transaction is stored in myIP_ID
      semaphore->tryAcquire(1,1000);
      myIP_ID = http->get( url->path());
      delete url;
    }
    if (settings->getReloadWebHostCachesSetting())
    {
      // test if the list contains list items, should now
      if ( ! list.isEmpty() )
      {
        // for every object in the list
        for ( int index = 0; running && index < list.size(); index++ )
        {
            // request a free object. If there is none, wait for
            semaphore->tryAcquire(1,1000);
            // download the file at the list position index
            download( list.at(index) );
        }
      }
    }
    if(settings->getEnableWebHostCache())
      getSeedNodes();
    /* this function is only exited when the last download call
       was successfull or exited with an error
    */

    #ifdef DEBUG
    qDebug() << "WebHostCaches in static default List:" << standardListCount;
    if ( listCount > 0 )
    {
        qDebug() << "WebHostCaches extra downloaded:" << listCount - standardListCount;
    }
    #endif
        // check if we should post our address to web caches again
        unsigned long currentTimeInSeconds = time( NULL );

        unsigned long timeSinceLastPost =
                currentTimeInSeconds - mTimeOfLastAddressPostInSeconds;

        if( timeSinceLastPost > mTimeToWaitBetweenAddressPostsInSeconds ) {

            // time to post address again

            // only post if we're not behind a firewall
            int behindFirewallFlag =
                    settings->getFirewallSetting();
            if(behindFirewallFlag != 1  && settings->getEnableWebHostCache() ){
                // not behind a firewall, so post our address
                // to the web caches

                AppLog::info( "ConnectionMaintainer",
                              "Posting our address to web caches." );
                postLocalAddress();
            }

            mTimeOfLastAddressPostInSeconds = currentTimeInSeconds;
        }

    // wait half an hour before load more hosts
    int cpt = 3600000;
    while (running && cpt-- > 0)
      usleep (500);
  }

  return;
}

void WebHostCachesDownload::stop ()
{
  running = false;
  http->abort();
}

void WebHostCachesDownload::download(QString file)
{
    // a new QUrl object for the URL of the file given as parameter
    QUrl *url = new QUrl(file);
    // test if the url is valid
    if ( ! url->isValid() )
    {
        // if the url isn't valid, add one free object to the semaphore
        semaphore->release();
        // and exit
        return;
    }
    // if the url is valid, set the host
    http->setHost( url->host() );
    // and get the file. The ID of the transaction is stored in downloadID
    downloadID = http->get( url->path(), buffer );
    delete url;
}

void WebHostCachesDownload::prepareList( QStringList oldData )
{
    if ( !oldData.isEmpty() )
    {
        webHostCachesList = oldData;
    }
    else
    {
        // create temp string with the standard values from defaultsettings.h
        QString temp = DEFAULT_SETTINGS_STANDARDWEBHOSTCACHE;
        // split the list with the list_separator in defaultsettings.h
        webHostCachesList = temp.split(LIST_SEPARATOR);
    }

    // only for debugging purpose
    #ifdef DEBUG
    standardListCount = webHostCachesList.size();
    #endif

    return;
}

void WebHostCachesDownload::sortList()
{
    // is the list empty? If yes, exit function
    if ( webHostCachesList.isEmpty() ) return;
    // sort the list
    webHostCachesList.sort();
    // if there are empty strings, they are at the beginning
    while ( webHostCachesList.at(0).isEmpty() )
    {
        // delete it
        webHostCachesList.removeAt(0);
        // exit the while loop when we deleted the last entry, even if it was empty
        if ( webHostCachesList.isEmpty() ) break;
    }
    // our index
    int index = 0;
    // if the point we are still not at the second item from the end
    while ( index < webHostCachesList.size() - 1 )
    {
        /* if the string at index and the next are equal, delete the second
           we don't need a complex algorithem because we sorted the list before
           so all equal strings are behind each other*/
        if ( webHostCachesList.at(index) == webHostCachesList.at(index+1) )
        {
            // delete the double value
            webHostCachesList.removeAt(index+1);
        }
        else
        {
            // go to the next position
            index++;
        }
    }

    // only for debugging purpose
    #ifdef DEBUG
    listCount = webHostCachesList.size();
    #endif
}

void WebHostCachesDownload::parseBuffer()
{
    // create temp string from the buffer
    QString temp = byteArray->constData();
    if(temp.startsWith("http://",Qt::CaseInsensitive))
    {
      QStringList list = temp.split('\n');
      // append the splitted temp string to the webHostCachesList
      for (int i = 0; i < list.size(); ++i)
      {
	if(list.at(i).startsWith("http:"))
	{
          qWarning() << "add cache host : "<<list.at(i);
          webHostCachesList += list.at(i);
	}
      }
    }
    // add one free object to the semaphore
    semaphore->release();
}

void WebHostCachesDownload::parseSeedBuffer()
{
    // create temp string from the buffer
    QByteArray result=http->readAll();
    QString temp = result.constData();
    QStringList list = temp.split('\n');
    // append the splitted temp string to the webHostCachesList
    for (int i = 0; i < list.size(); ++i)
    {
      int colonPos=list.at(i).indexOf(":");
      if(colonPos>0)
      {
         // read the port
         int port = list.at(i).mid(colonPos+1,5).toInt();
         QString host=list.at(i).mid(0,colonPos);
         char * hostaddr=strdup((char *)host.toAscii().constData());
         HostAddress address ( hostaddr, port );
	 if(port >0)
	 {
           muteHostCatcher->addHost( &address );
	 }
      }
    }
}

QStringList WebHostCachesDownload::getWebHostCaches()
{
    // sort the list and delete double values
    sortList();
    // return the webHostCachesList
    return webHostCachesList;
}

void WebHostCachesDownload::httpRequestFinished( int ID, bool error )
{
    // if the ID isn't the downloadID from our job, then this isn't the
    // get request, so finish here. The file isn't here
    // if an error occured
    if ( error )
    {
        #ifdef DEBUG
        qDebug() << http->errorString();
        #endif
        // add one free object to the semaphore
        if(ID == downloadID || seedIDs.contains(ID) || postIDs.contains(ID))
          semaphore->release();
        // exit the function
        return;
    }
    if ( seedIDs.contains(ID) )
    {
        parseSeedBuffer();
	seedIDs.removeOne(ID);
	http->abort();
        semaphore->release();
        return;
    }
    if ( postIDs.contains(ID) )
    {
        QByteArray result=http->readAll();
	postIDs.removeOne(ID);
	http->abort();
        semaphore->release();
        return;
    }
    if ( ID == release_ID )
    {
	// found last release number
        QByteArray result=http->readAll();
        semaphore->release();
        if(result.isEmpty() || result.length() >10 || !isdigit(result.at(0)))
                return;
	int newlinepos=result.indexOf("\n");
	if (newlinepos >0) result.truncate(newlinepos);
	// compare actual and last release
	QByteArray actual(DEFAULT_SETTINGS_STANDARDMUTEVERSION);
	int endMajorActual=actual.indexOf(".");
	int endMajorNew=result.indexOf(".");
	if( actual.mid(0,endMajorActual) < result.mid(0,endMajorNew))
	{
	  newRelease = true;
	}
	else if( actual.mid(0,endMajorActual) == result.mid(0,endMajorNew))
	{
	  int endMinorActual=actual.indexOf(".",endMajorActual+1);
	  int endMinorNew=result.indexOf(".",endMajorNew+1);
	  if( actual.mid(endMajorActual,endMinorActual) < result.mid(endMajorNew,endMinorNew))
	  {
	    newRelease = true;
	  }
	}
        return;
    }
    if ( ID == myIP_ID )
    {
        QByteArray result=http->readAll();
        qDebug()<< "myIP:="<<result.constData();
	settings->setMyIP(result);
        semaphore->release();
        return;
    }
    if ( ID != downloadID )
    {
        // exit the function
        return;
    }
    // parse the buffer
    parseBuffer();
}

void WebHostCachesDownload::postLocalAddress()
{
  int portNumber = settings->getPortNumberSetting();
  for (int i = 0; running && i < webHostCachesList.size(); ++i)
  {
     post1LocalAddress(webHostCachesList.at(i), portNumber);
  }
}

void WebHostCachesDownload::post1LocalAddress(QString stringUrl,int port)
{
  // request a free object. If there is none, wait for
  semaphore->tryAcquire(1,1000);
  QUrl *url = new QUrl(stringUrl);
  QString file=url->path();
  if(file.contains("gwc.php"))
  {
    file += "?client=mutekomm&net=mute&pv=1&spec=1&version="+settings->getMuteVersionSetting ()
             + "&ip=" + settings->getMyIP() + "%3A" + QString::number(port);
  }
  else
  {
    file += "?client=MUTE&version=komm_"+settings->getMuteVersionSetting ()
             + "&ip=" + settings->getMyIP() + "%3A" + QString::number(port);
  }
  QHttpRequestHeader header("GET", file.toAscii().constData());
  header.setValue("Host", url->host());
  header.setValue("accept", "application/xhtml+xml");
  header.setValue("user_agent", "kommute");
  http->setHost( url->host() );
  int postID=http->request(header,0,0);
  postIDs << postID;
  delete url;
}


void WebHostCachesDownload::getSeedNodes()
{
  if(!settings) return;
  for (int i = 0; running && i < webHostCachesList.size(); ++i)
  {
    QUrl *url = new QUrl(webHostCachesList.at(i));
    QString file=url->path();
    if(file.contains("gwc.php"))
     {
	file += "?client=mutekomm&hostfile=1&version="+settings->getMuteVersionSetting ();
     }
    else
     {
	file += "?client=MUTE&hostfile=1&version=komm_"+settings->getMuteVersionSetting ();
     }
    QHttpRequestHeader header("GET", file.toAscii().constData());
    header.setValue("Host", url->host());
    header.setValue("accept", "application/xhtml+xml");
    header.setValue("user_agent", "kommute");
    // request a free object. If there is none, wait for
    semaphore->tryAcquire(1,1000);
    http->setHost( url->host() );
    int seedID=http->request(header,0,0);
    seedIDs << seedID;
    delete url;
  }
}
