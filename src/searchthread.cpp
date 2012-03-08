/***************************************************************************
 *   Copyright (C) 2005 by Niels Sandholt Busch   *
 *   niels@diku.dk   *
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
#include "searchthread.h"
#include "searchresultevent.h"

#include "mutelayer.h"

#include <QApplication>
#include <QString>
#include <QTextCodec>
#include <QMessageBox>
#include <QSemaphore>
#include "cache.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"

const int   DEFAULT_SEARCHTHREAD_SLEEP = 2;
extern QSemaphore semCache;

SearchThread::SearchThread( QObject *eventReceiver, QString address, QString searchTerms )
 : QThread(), eventReceiver( eventReceiver ), running( false ), address( address ), searchTerms( searchTerms ), events( true )
{
}


SearchThread::~SearchThread()
{
    wait();
}

void SearchThread::run()
{
  running = true;


  /*  search in cache */
  char *searchString = strdup((char *) this->searchTerms.toUtf8 ().constData ());
  SimpleVector < char *>*tokens = tokenizeString (searchString);
  int numSearchTerms = tokens->size ();
  int pos;

  bool locked = semCache.tryAcquire(1,1000);
  for (pos = 0; pos < nbCache; pos++)
  {

	  if(address!="" && address !="ALL" && Cache[pos].host != address) continue;
      // check if this is a hash-only search
      char hashOnlySearch = false;
      const char *hashStart = "hash_";
      char *hashString = (char *)malloc(41);
      char *pointerToHashStart = strstr (searchString, hashStart);

      if (pointerToHashStart != NULL)
      {

    	  // search contains a hash-only search.
    	  // extract hash, and ignore any other search terms
    	  char *pointerToHash = &(pointerToHashStart[strlen (hashStart)]);

    	  // hash should be at most 40 characters long
    	  int numRead = sscanf (pointerToHash, "%40s", hashString);
    	  if (numRead == 1)
    	  {
    		  delete[]searchString;
    		  searchString = hashString;
    		  hashOnlySearch = true;
    	  }
    	  else
    	  {
    		  free(hashString);
    	  }
      }
      else
      {
    	  free(hashString);
      }

      char hitAll = true;

      if (!hashOnlySearch)
      {

    	  // check each term
    	  for (int j = 0; j < numSearchTerms && hitAll; j++)
    	  {
    		  char *term = *(tokens->getElement (j));
    		  char * tmpstr = stringLocateIgnoreCase (Cache[pos].name, term);
    		  if (tmpstr == NULL)
    		  {

    			  // missed this term
    			  hitAll = false;
    		  }
    		  else delete [] tmpstr;
    	  }
      }
      else
      {
    	  char *hashTerm = *(tokens->getElement (0));
    	  if (strcasecmp (hashString, hashTerm) != 0)
    	  {
    		  hitAll = false;
    	  }
      }
      if (hitAll)
      {
            QTextCodec *codecutf=QTextCodec::codecForName("utf8");
            QTextCodec *codeclat=QTextCodec::codecForName("iso8859-1");
            QString qstr;
	    // try to convert string from utf8
            qstr=codecutf->toUnicode(Cache[pos].name);
	    // test if conversion is reversible
            if(strcmp(Cache[pos].name,qstr.toUtf8().data()))
	      // if not reversible, it's not utf8, convert from iso8859 :
              qstr=codeclat->toUnicode(Cache[pos].name);
	  QApplication::postEvent (eventReceiver,
				   new SearchResultEvent (qstr,
							  Cache[pos].size,
							  QString::fromLatin1
							  (Cache[pos].hash),
							  QString::fromLatin1
							  (Cache[pos].host)));
      }
  }
  if(locked) semCache.release(1);
  for (int i = 0; i < numSearchTerms; i++)
  {
      delete[](*(tokens->getElement (i)));
  }
  delete tokens;
  free( searchString );

  MuteLayer::muteLayerSearch ((char *) this->searchTerms.
			      toLatin1 ().constData (),
			      SearchThread::muteSearchHandler, this, 3000,
			      (char *) this->address.
			      toLatin1 ().constData ());



}

void SearchThread::stop()
{
    running = false;

}

char SearchThread::searchHandler( char *inFileHostVirtualAddress,
                                  char *inFilePath,
                                  unsigned long inFileLength,
                                  char *inFileHash ) {

    if(inFilePath != NULL && inFileHash != NULL)
    {
        if(strcmp(inFileHash,"(null)") && events) // only post events if eventReceiver has not been deleted
        {
            QTextCodec *codecutf=QTextCodec::codecForName("utf8");
            QTextCodec *codeclat=QTextCodec::codecForName("iso8859-1");
            QString qstr;
	    // try to convert string from utf8
            qstr=codecutf->toUnicode(inFilePath);
	    // test if conversion is reversible
            if(strcmp(inFilePath,qstr.toUtf8().data()))
	      // if not reversible, it's not utf8, convert from iso8859 :
              qstr=codeclat->toUnicode(inFilePath);
            QApplication::postEvent( eventReceiver,
                                     new SearchResultEvent(qstr,
                                                       inFileLength,
                                                       QString::fromLatin1(inFileHash),
                                                       QString::fromLatin1(
                                                                inFileHostVirtualAddress)));
        }
    }
  //  else running = false;

    //Means timeout in search
    if ( inFilePath == NULL && inFileHash == NULL && inFileLength == 0 && inFileHash == NULL )
    	running = false;

    return running;
}

char SearchThread::muteSearchHandler( char *inFileHostVirtualAddress,
                                             char *inFilePath,
                                             unsigned long inFileLength,
                                             char *inFileHash,
                                             void *inExtraArgument ) {
    return static_cast<SearchThread*>(inExtraArgument)->searchHandler( inFileHostVirtualAddress,
                                                            inFilePath,
                                                            inFileLength,
                                                            inFileHash );
}

void SearchThread::eventsAllowed( bool allowed )
{
    events = allowed;
}
