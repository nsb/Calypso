/*
 * Modification History
 *
 * 2002-April-4    Jason Rohrer
 * Created.
 * Changed to reflect the fact that the base class
 * destructor is called *after* the derived class destructor.
 *
 * 2002-August-5   Jason Rohrer
 * Fixed member initialization order to match declaration order.
 *
 * 2003-September-5   Jason Rohrer
 * Moved into minorGems.
 *
 * 2005-January-9   Jason Rohrer
 * Changed to sleep on a semaphore to make sleep interruptable by stop.
 */



#include "MUTE/StopSignalThread.h"



StopSignalThread::StopSignalThread()
    : mStopLock( new QMutex() ), mStopped( false ),
      mSleepSemaphore( new QSemaphore() ) {

    }



StopSignalThread::~StopSignalThread() {
    mStopped=TRUE;
    delete mStopLock;
    mStopLock=NULL;
    delete mSleepSemaphore;
    }



void StopSignalThread::sleep( unsigned long inTimeInMilliseconds ) {
    mSleepSemaphore->tryAcquire(1, inTimeInMilliseconds );
    }



char StopSignalThread::isStopped() {
    if(mStopLock) mStopLock->lock();
    char stoped = mStopped;
    if(mStopLock) mStopLock->unlock();

    return stoped;
    }



void StopSignalThread::stop() {
    mStopLock->lock();
    mStopped = true;
    mStopLock->unlock();

    // signal the semaphore to wake up the thread, if it is sleeping
    mSleepSemaphore->release();
    }

