/*
 * Modification History
 *
 * 2003-July-27    Jason Rohrer
 * Created.
 * Fixed indexing bugs.
 *
 * 2004-December-13    Jason Rohrer
 * Switched to use a binary semaphore for sleeping.
 */



#include "ChannelReceivingThread.h"
#include "ChannelReceivingThreadManager.h"



ChannelReceivingThreadManager::ChannelReceivingThreadManager()
    : mLock( new QMutex() ),
      mThreadVector( new SimpleVector<ChannelReceivingThread *>() ),
      mStopSignal( false ),
      mSleepSemaphore( new QSemaphore() ) {

    this->start();
    }



ChannelReceivingThreadManager::~ChannelReceivingThreadManager() {
    mLock->lock();
    mStopSignal = true;
    mLock->unlock();

    int numThreads = mThreadVector->size();
    for( int i=0; i<numThreads; i++ ) {
          (** mThreadVector->getElement( i ) ).setStopSignal();
        }
    // signal the sleeping semaphore to wake up the thread
    mSleepSemaphore->release();
    
    terminate();
    wait(2000);

    
    mLock->lock();

    // destroy all remaining threads

    numThreads = mThreadVector->size();
    for( int i=0; i<numThreads; i++ ) {
        delete *( mThreadVector->getElement( i ) );
        }
    delete mThreadVector;

    
    mLock->unlock();
    delete mLock;    
    mLock=NULL;
    delete mSleepSemaphore;
    mSleepSemaphore=NULL;
    }



void ChannelReceivingThreadManager::addThread(
    ChannelReceivingThread *inThread ) {

    mLock->lock();

    mThreadVector->push_back( inThread );

    mLock->unlock();
    }



void ChannelReceivingThreadManager::run() {
    
    char stopped;

    mLock->lock();
    stopped = mStopSignal;
    mLock->unlock();


    while( !stopped ) {
        // wait for 10 seconds
        int wasSignaled = mSleepSemaphore->tryAcquire(1, 10000 );

        if( wasSignaled == 1 ) {
            // signaled... we should stop
            return;
            }
        
        char foundFinished = true;

        while( foundFinished ) {
            foundFinished = false;
            
            mLock->lock();
            
            int numThreads = mThreadVector->size();

            for( int i=0; i<numThreads && !foundFinished; i++ ) {
                
                ChannelReceivingThread *currentThread =
                    *( mThreadVector->getElement( i ) );

                if( currentThread->isFinished() ) {
                    delete currentThread;
                    mThreadVector->deleteElement( i );
                    foundFinished = true;
                    }
                }
            mLock->unlock();
            }

        mLock->lock();
        stopped = mStopSignal;
        mLock->unlock();
        }
    }

