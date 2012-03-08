/*
 * Modification History
 *
 * 2004-December-13   Jason Rohrer
 * Created.
 */



#include "SocketManager.h"



// static initialization
SocketManagerDataWrapper SocketManager::mDataWrapper;



SocketManagerDataWrapper::SocketManagerDataWrapper()
    : mLock( new QMutex() ),
      mSocketVector( new QVector<Socket *>() ) {

    }



SocketManagerDataWrapper::~SocketManagerDataWrapper() {
    QMutex *lock = mLock;
    mLock=NULL;
    int numSockets = mSocketVector->size();


    for( int i=0; i<numSockets ; i++ )
	delete mSocketVector->at( i ) ;
    mSocketVector->clear();
    delete mSocketVector;
    mSocketVector=NULL;

    delete lock;
    }



void SocketManager::addSocket( Socket *inSocket ) {

    QMutex *lock = mDataWrapper.mLock;
    QVector<Socket *> *socketVector = mDataWrapper.mSocketVector;

    
    lock->lock();

    socketVector->push_back( inSocket );
    
    lock->unlock();
    }



void SocketManager::breakConnection( Socket *inSocket ) {

    QMutex *lock = mDataWrapper.mLock;
    QVector<Socket *> *socketVector = mDataWrapper.mSocketVector;
    if(!socketVector) return;

    if (lock) lock->lock();

    int numSockets = socketVector->size();
    char found = false;
    
    for( int i=0; i<numSockets && !found; i++ ) { 

        Socket *currentSocket = ( socketVector->at( i ) );

        if( currentSocket == inSocket ) {
            currentSocket->breakConnection();
            found = true;
            }
        }
    
    if (lock) lock->unlock();
    }



void SocketManager::destroySocket( Socket *inSocket ) {

    QMutex *lock = mDataWrapper.mLock;
    QVector<Socket *> *socketVector = mDataWrapper.mSocketVector;
    if(!socketVector) return;

    if (lock) lock->lock();

    int numSockets = socketVector->size();
    char found = false;
    
    for( int i=0; i<numSockets && !found; i++ ) { 

        Socket *currentSocket = ( socketVector->at( i ) );

        if( currentSocket == inSocket ) {
            socketVector->remove( i );
            delete currentSocket;
            
            found = true;
            }
        }
    
    if (lock) lock->unlock();
    }
