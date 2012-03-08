/*
 * Modification History
 *
 * 2003-June-22    Jason Rohrer
 * Copied from the konspire2b project and modified.
 *
 * 2003-July-27    Jason Rohrer
 * Fixed a bug when catcher is empty.
 *
 * 2003-December-21    Jason Rohrer
 * Fixed a memory leak when catcher is full.
 *
 * 2004-January-11   Jason Rohrer
 * Made include paths explicit to help certain compilers.
 *
 * 2004-December-20   Jason Rohrer
 * Changed to convert to numerical form before comparing against host list.
 * Changed getHost to return hosts in random order.
 * Added a getOrderedHost function that returns hosts in linear order.
 */


#include <QString>
#include <QFile>
#include <QTextStream>
#include "minorGems/network/p2pParts/HostCatcher.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/random/StdRandomSource.h"
#include "settings.h"

extern class Settings *settings;
extern char *muteInternalDirectoryTag;



HostCatcher::HostCatcher( int inMaxListSize )
    : mMaxListSize( inMaxListSize ),
      mHostVector( new QVector<HostAddress *>() ),
      mLock( new QMutex() ),
      mRandSource( new StdRandomSource() ) {
   lastReturned=-1;
   lastOrdReturned=-1;
   firstBad=0; 
   firstNotTried=0; 
    }


        
HostCatcher::~HostCatcher() {
    mLock->lock();
    
    int numHosts = mHostVector->size();

    for( int i=0; i<numHosts; i++ ) {
        delete  mHostVector->at( i ) ;
        }

    delete mHostVector;
    
    mLock->unlock();

    delete mLock;
    delete mRandSource;
    }

    

void HostCatcher::addHost( HostAddress * inHost ) {
    

    // convert to numerical form once and for all here
    // (to avoid converting over and over in equals checks below)
    HostAddress *numericalAddress = inHost->getNumericalAddress();

    if( numericalAddress != NULL ) {

        mLock->lock();
        
        // make sure this host doesn't already exist in our list
        char exists = false;
    
        int numHosts = mHostVector->size();
    
        for( int i=0; i<numHosts; i++ ) {
            HostAddress *otherHost =   mHostVector->at( i ) ;

            if( otherHost->equals( numericalAddress ) ) {
                exists = true;
                // jump out of loop
                i = numHosts;
                }
            }
    
    
    
        if( !exists ) {
	  int days = inHost->lastPing.daysTo(QDateTime::currentDateTime());
	  if(days <30)
	  {
            // good host : add host to begin of queue
            mHostVector->prepend( numericalAddress->copy() );
	    firstNotTried++;
	    firstBad++;
	  }
	  else
	  {
	    days = inHost->lastTry.daysTo(QDateTime::currentDateTime());
	    if(days >30)
	    {
              mHostVector->insert(firstNotTried, numericalAddress->copy() );
	      firstBad++;
	    }
	    else
	    {
              // bad host : add host at end of queue
              mHostVector->append( numericalAddress->copy() );
	    }
	  }
        }
        
        // remove hosts if list is full.
        for ( int i=1 ; mHostVector->size() > mMaxListSize  && i <mHostVector->size() ; ) {
            // remove host from queue
            HostAddress *host = mHostVector->at( mMaxListSize-i );
	    if(host->dontDestroy <=0)
            {
               mHostVector->remove( mMaxListSize-i );
               delete host;
	      if ( mMaxListSize-i < firstBad) firstBad--;
	      if ( mMaxListSize-i < firstNotTried) firstNotTried--;
            }
	    else
              i++;
        }
    
        mLock->unlock();

        delete numericalAddress;
        }
    }



HostAddress * HostCatcher::getHostOrdered(  ) {

    mLock->lock();
    
    int numHosts = mHostVector->size();

    if( numHosts == 0 ) {
        mLock->unlock();
        return NULL;
        }
    if (lastOrdReturned<0) lastOrdReturned=0;
    else lastOrdReturned++;
    if(lastOrdReturned >numHosts-1) lastOrdReturned=0;
    HostAddress *host = mHostVector->at( lastOrdReturned );

    HostAddress *hostCopy = host->copy();

    
    mLock->unlock();
    
    return hostCopy;   
    }



HostAddress * HostCatcher::getHost(  ) {

    mLock->lock();
    
    int numHosts = mHostVector->size();

    if( numHosts == 0 ) {
        mLock->unlock();
        return NULL;
        }
    if ( numHosts <3)
    {
	firstNotTried = firstBad = numHosts;
    }
    // test if previous not tried returned host has been pinged
    if(lastReturned >=firstNotTried && lastReturned <firstBad )
    {
      HostAddress *host = mHostVector->at( lastReturned );
      int days = host->lastPing.daysTo(QDateTime::currentDateTime());
      if(days <30)
      {
	// this host has been pinged : move it with good hosts
        mHostVector->remove( lastReturned );
	// reinsert at begin of list (good host)
        mHostVector->prepend( host );
	if ( lastReturned >= firstBad) firstBad++;
	if ( lastReturned >= firstNotTried) firstNotTried++;
      }
      else
      {
	days = host->lastTry.daysTo(QDateTime::currentDateTime());
        if(days <30)
        {
	  // this host has been tried but not pinged : move it with bad hosts
          mHostVector->remove( lastReturned );
	  if ( lastReturned < firstBad) firstBad--;
	  if ( lastReturned < firstNotTried) firstNotTried--;
          mHostVector->insert(firstBad, host );
        }
      }
    }
    if((lastReturned%5)==0)
    {
      if(firstNotTried <firstBad)
      {
        // pick random host from queue in never tried hosts
        lastReturned = mRandSource->getRandomBoundedInt( firstNotTried, firstBad - 1 ); 
      }
      else
      {
        // pick random host from queue in bad hosts
        lastReturned = mRandSource->getRandomBoundedInt( firstBad, numHosts-1 );
      }
    }
    else
    {
      // pick random host from queue in good hosts
      lastReturned = mRandSource->getRandomBoundedInt( 0, firstNotTried - 1 ); 
    }
    HostAddress *host = mHostVector->at( lastReturned );
    // mark this host as used :
    host->dontDestroy++;
    
    mLock->unlock();
    
    return host;   
    }


SimpleVector<HostAddress *> *HostCatcher::getHostList(
    int inMaxHostCount,
    HostAddress *inSkipHost ) {

    HostAddress *hostToSkip;

    if( inSkipHost != NULL ) {
        hostToSkip = inSkipHost->copy();
        }
    else {
        // don't skip any host
        // create a dummy host that won't match any other valid hosts
        // make sure dummy is in numerical form to avoid DNS lookups
        hostToSkip = new HostAddress( stringDuplicate( "1.1.1.1" ), 1 );
        }
             
    
    SimpleVector<HostAddress *> *collectedHosts =
        new SimpleVector<HostAddress *>();

    char repeat = false;
    int numCollected = 0;

    // This function assumes that getHostOrdered() draws
    // hosts in order with no repetition except when we have
    // exhausted the host supply.

    // Note that this will not be true when other threads
    // have getHostOrdered() (or getHost) calls interleaved with ours, but this
    // should be a rare case.  It will simply result
    // in a smaller host list being returned.

    lastOrdReturned=-1;
    HostAddress *firstHost = getHostOrdered();

    if( firstHost == NULL ) {
        // the catcher is empty

        delete hostToSkip;

        // an empty host list
        return collectedHosts;       
        }
    

    if( ! hostToSkip->equals( firstHost ) ) {
        collectedHosts->push_back( firstHost );
        numCollected++;
        }

    
    while( numCollected < inMaxHostCount && !repeat ) {

        HostAddress *nextHost = getHostOrdered();

        if( nextHost->equals( firstHost ) ) {
            delete nextHost;
            repeat = true;
            }
        else {
            if( ! hostToSkip->equals( nextHost ) ) {
                collectedHosts->push_back( nextHost );
                numCollected++;
                }
            else {
                delete nextHost;
                }
            }
        
        }


    if( hostToSkip->equals( firstHost ) ) {
        // we didn't include firstHost in our collectedHosts, so
        // we must delete it.
        delete firstHost;
        }

    
    delete hostToSkip;

    return collectedHosts;
    }



void HostCatcher::addHostList( SimpleVector<HostAddress *> * inHostList ) {
    int numToAdd = inHostList->size();

    for( int i=0; i<numToAdd; i++ ) {
        addHost( *( inHostList->getElement( i ) ) );
        }
    }

void HostCatcher::saveHosts ()
{
    mLock->lock();
    QString fullName (settings->getTempDirectory());
    fullName += "/";
    fullName += muteInternalDirectoryTag;
    fullName += "/hosts.new";
    QFile ficout (fullName);

    if (ficout.open (QIODevice::WriteOnly))
    {

      QTextStream Stream (&ficout);
      Stream.setCodec ("UTF-8");
      int numHosts = mHostVector->size();
      for (int i = 0; i < numHosts ; i++)
      {
	HostAddress *host =  mHostVector->at( i );
	Stream << QString(host->mAddressString) <<",";
	Stream <<host->mPort <<"," ;
	Stream <<host->created.toString (Qt::ISODate) <<"," ;
	Stream <<host->lastTry.toString (Qt::ISODate) <<"," ;
	Stream <<host->lastPing.toString (Qt::ISODate) <<"," ;
	Stream <<host->lastConnection.toString (Qt::ISODate) << endl ;
        Stream.flush();
      }
    }
    ficout.close ();
    fullName = QString (settings->getTempDirectory()) + "/" +
        QString (muteInternalDirectoryTag) + "/hosts";

    QFile::remove (fullName);
    ficout.rename (fullName);
    printf ("hosts saved\n");
    mLock->unlock();

}


void HostCatcher::loadHosts () {
    QString fullName (settings->getTempDirectory());
    fullName += "/";
    fullName += muteInternalDirectoryTag;
    fullName += "/hosts";
    QFile ficin (fullName);

    if (!ficin.open (QIODevice::ReadOnly))
    {
        return;
    }

    QTextStream Stream (&ficin);
    Stream.setCodec ("UTF-8");
    QString temp;

    while (!Stream.atEnd () && mHostVector->size() < mMaxListSize)
    {
        temp = Stream.readLine ();
        QStringList list1 = temp.split (",");

        if (list1.size () > 1)
        {
	  int port= list1[1].toInt();
	  if(port <=0) continue;
	  HostAddress *host = new HostAddress( stringDuplicate(list1[0].toAscii().constData()),port);
          if (list1.size () > 2)
	    host->created= host->created.fromString (list1[2], Qt::ISODate);
          if (list1.size () > 3)
	    host->lastTry= host->created.fromString (list1[3], Qt::ISODate);
          if (list1.size () > 4)
	    host->lastPing= host->created.fromString (list1[4], Qt::ISODate);
          if (list1.size () > 5)
	    host->lastConnection= host->created.fromString (list1[5], Qt::ISODate);
	  addHost( host );
	  delete host;
	}
    }

}


