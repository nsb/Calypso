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
 */



#ifndef KBLIMITER_INCLUDED_H
#define KBLIMITER_INCLUDED_H



#include <QMutex>
#include <QTime>



/**
 * Class that limits the number of messages transmitted per second.
 *
 * @author Jason Rohrer
 */
class KbLimiter
{



public:



	/**
         * Constructs a limiter.
         *
         * @param inLimitPerSecond the maximum number of kB
         *   transmitted per second, or -1 for no limit.
         *   Defaults to -1.
         */
  KbLimiter (double inLimitPerSecond = -1);



   ~KbLimiter ();



	/**
         * Sets the limit.
         *
         * Thread safe.
         *
         * @param inLimitPerSecond the maximum number of kB
         *   transmitted per second, or -1 for no limit.
         */
  void setLimit (double inLimitPerSecond);



	/**
         * Gets the limit.
         *
         * Thread safe.
         *
         * @return the maximum number of kB
         *   transmitted per second, or -1 if no limit set.
         */
  double getLimit ();



	/**
         * Called by a message transmitter to indicate that a message
         * is about to be transmitted.  Will block if messages
         * are being transmitted too frequently.
         *
         * Thread safe.
         */
  void bytesTransmitted (long bytes);



protected:

    QMutex * mLock;
  QMutex *mTransmitLock;
  QTime *mTimer;

  double mLimitPerSecond;

  unsigned long totBytes;



};



#endif
