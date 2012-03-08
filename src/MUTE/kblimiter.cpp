/*
 * Modification History
 *
 * 2003-October-9   Jason Rohrer
 * Created.
 *
 * 2003-October-12   Jason Rohrer
 * Switched to a floating point limit.
 *
 * 2004-January-2   Jason Rohrer
 * Added seprate mutex for transmission function to prevent UI freeze.
 *
 * 2005-April-15   Jason Rohrer
 * Changed to use updated Thread interface.
 */



#include "MUTE/kblimiter.h"


#include "minorGems/system/Thread.h"



KbLimiter::KbLimiter (double inLimitPerSecond):
mLock (new QMutex ()),
mTransmitLock (new QMutex ()),
mLimitPerSecond (inLimitPerSecond),
mTimer (new QTime ()),
totBytes (0)
{

  mTimer->start ();
}



KbLimiter::~KbLimiter ()
{
  mTransmitLock->tryLock (1000);
  mTransmitLock->tryLock (1000);
  delete mLock;
  delete mTransmitLock;
  delete mTimer;
}



void
KbLimiter::setLimit (double inLimitPerSecond)
{
  bool locked=mLock->tryLock (1000);
  mLimitPerSecond = inLimitPerSecond;

  if (locked) mLock->unlock ();
}



double
KbLimiter::getLimit ()
{
  bool locked=mLock->tryLock (200);
  double limit = mLimitPerSecond;
  if (locked) mLock->unlock ();

  return limit;
}



void
KbLimiter::bytesTransmitted (long bytes)
{
  // allow only one transmitter to report at a time
  bool locked = mTransmitLock->tryLock (500);


  // protect our variables (make sure settings functions are not
  // called while we touch the variables)
  bool locked2 = mLock->tryLock (200);


  unsigned long millisecondsSinceLastMessage = mTimer->elapsed ();
  if (millisecondsSinceLastMessage > 1000)
    {
      mTimer->start ();
      totBytes = bytes;
    }
  else
    {
      totBytes += bytes;
      if ( (mLimitPerSecond > 0) && (totBytes > (mLimitPerSecond * 1024)))
	{
	  unsigned long sleepTime = 1000 - millisecondsSinceLastMessage+(totBytes-mLimitPerSecond*1024)/mLimitPerSecond;
	  // unlock main lock befor sleeping so that settings can be changed
	  if (locked2) mLock->unlock ();
	  locked2 = false;

	  Thread::staticSleep (sleepTime);
	  locked2 = mLock->tryLock (50);
          mTimer->start ();
          totBytes = bytes;

	}

    }

  if(locked2)  mLock->unlock ();


  if (locked)mTransmitLock->unlock ();
}
