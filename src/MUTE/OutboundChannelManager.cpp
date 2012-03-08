/***************************************************************************
 *   Copyright (C) 2007 kommute dev team and others                        *
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
 */

#include "settings.h"

#include "MUTE/OutboundChannelManager.h"
#include "applog.h"

#include "minorGems/network/SocketManager.h"

#include "MUTE/messageRouter.h"

extern class Settings * settings;

OutboundChannelManager::OutboundChannelManager (RandomSource * inRandSource):mRandSource (inRandSource),
mNodeUniqueNames (new SimpleVector < char *>()),
mChannelVector (new SimpleVector < OutboundChannel * >()),
mStreamVector (new SimpleVector < OutputStream * >()),
mSocketVector (new SimpleVector < Socket * >()),
mLock (new QMutex ()),
mTableSizeLimit (100),
mTableEntrySizeLimit (50),
mUniformProbability (0.0),
mUseMajorityRouting (false),
mRoutingTable (new SimpleVector < RoutingTableEntry * >()),
mHistoryOutputFile (NULL)
{

  int
    logRoutingHistory = settings->getLogRoutingHistorySetting();
  if (logRoutingHistory == 1)
    {
      mHistoryOutputFile = fopen ("routingHistory.log", "w");
    }


    mUniformProbability = settings->getBroadcastProbabilitySetting();

    mUseMajorityRouting = settings->getUseMajorityRoutingSetting();

    mMaxDroppedMessageFraction = settings->getMaxDroppedMessageFractionSetting();
}



OutboundChannelManager::~OutboundChannelManager ()
{
  mLock->lock ();

  int i;
  for (i = 0; i < mChannelVector->size (); i++)
    {
      delete[] * (mNodeUniqueNames->getElement (i));

      OutboundChannel *chan = *(mChannelVector->getElement (i));
      delete chan;
    }
  for (i = 0; i < mStreamVector->size (); i++)
    {
      OutputStream *stream = *(mStreamVector->getElement (i));
      delete stream;
      // do not delete in sockets : deleted by ChannelReceivingThread
    }
  delete mNodeUniqueNames;
  delete mChannelVector;
  delete mStreamVector;
  delete mSocketVector;

  for (i = 0; i < mRoutingTable->size (); i++)
    {
      RoutingTableEntry *entry = *(mRoutingTable->getElement (i));
      delete[]entry->mFromAddress;
      delete entry->mReceivingChannels;
      delete entry;
    }
  delete mRoutingTable;

  mLock->unlock ();

  delete mLock;
}



char
OutboundChannelManager::channelCreated (char *inNodeUniqueName,
					OutboundChannel * inChannel,
					OutputStream * inOutputStream,
					Socket * inSocket,
					char *inFirstMessage)
{
  mLock->lock ();

  // make sure a connection does not already exist to the remote
  // host attached to inChannel
  HostAddress *proposedHost = inChannel->getHost ();

  char exists = false;
  int numChan = mChannelVector->size ();
  for (int i = 0; i < numChan && !exists; i++)
    {
      OutboundChannel *chan = *(mChannelVector->getElement (i));

      HostAddress *existingHost = chan->getHost ();
      char *existingName = *(mNodeUniqueNames->getElement (i));

      // check for either address or unique name match
      // if( existingHost->equals( proposedHost ) ||
      //    strcmp( existingName, inNodeUniqueName ) == 0 ) 

      // only check unique name to avoid blocking on DNS lookup
      if (strcmp (existingName, inNodeUniqueName) == 0)
	{
	  exists = true;
	}
      delete existingHost;
    }

  delete proposedHost;

  if (!exists)
    {
      if (inFirstMessage != NULL)
	{
	  inChannel->sendMessage (inFirstMessage);
	}
      mNodeUniqueNames->push_back (stringDuplicate (inNodeUniqueName));
      mChannelVector->push_back (inChannel);
      mStreamVector->push_back (inOutputStream);
      mSocketVector->push_back (inSocket);
    }

  mLock->unlock ();

  // return false if a connection already exists to the remote host
  return !exists;
}



void
OutboundChannelManager::channelBroken (OutboundChannel * inChannel)
{
  mLock->lock ();

  char foundChannel = false;
  int foundIndex = -1;

  char *foundUniqueName = NULL;

  OutputStream *foundStream = NULL;
  Socket *foundSocket = NULL;

  int numChannels = mChannelVector->size ();
  for (int i = 0; i < numChannels && !foundChannel; i++)
    {

      OutboundChannel *chan = *(mChannelVector->getElement (i));

      if (chan == inChannel)
	{
	  foundChannel = true;
	  foundIndex = i;
	  foundStream = *(mStreamVector->getElement (i));
	  foundUniqueName = *(mNodeUniqueNames->getElement (i));
	  foundSocket = *(mSocketVector->getElement (i));

	  // remove elements from all vectors
	  mChannelVector->deleteElement (i);
	  mStreamVector->deleteElement (i);
	  mNodeUniqueNames->deleteElement (i);
	  mSocketVector->deleteElement (i);
	}

    }

  // unlock before deleting, since the delete could block for a long
  // time while we wait for the channel's thread to finish it's
  // last write and eventually return
  // We only want to block the thread that's calling channelBroken
  // in this case, and we don't want to block other OutboundChannelManager
  // function calls.
  mLock->unlock ();


  // make sure we only delete the channel once
  // we delete the first time it is found/removed from mChannelVector
  // even though more channelBroken calls may occur later
  if (foundChannel)
    {

      // break the socket's connection using the manager for safety
      SocketManager::breakConnection (foundSocket);

      // now delete the channel.
      // this should not block, since the channel can no longer
      // be blocked on a socket send (because we called breakConnection)
      delete inChannel;

      // delete the corresponding stream and name.
      delete foundStream;
      delete[]foundUniqueName;
    }

}



void
OutboundChannelManager::addRoutingInformation (char *inFromAddress,
					       OutboundChannel *
					       inReceivingChannel)
{

  mLock->lock ();

  // add a new routing table entry
  RoutingTableEntry *entry = NULL;

  int tableSize = mRoutingTable->size ();

  for (int i = 0; i < tableSize && entry == NULL; i++)
    {

      RoutingTableEntry *currentEntry = *(mRoutingTable->getElement (i));

      if (strcmp (currentEntry->mFromAddress, inFromAddress) == 0)
	{
	  entry = currentEntry;

	  // move to end of queue
	  mRoutingTable->deleteElement (i);
	  mRoutingTable->push_back (entry);
	}
    }

  if (entry == NULL)
    {
      // add new entry to table
      entry = new RoutingTableEntry ();
      entry->mFromAddress = stringDuplicate (inFromAddress);
      entry->mReceivingChannels = new SimpleVector < OutboundChannel * >();

      mRoutingTable->push_back (entry);

      // remove extra entries
      while (mRoutingTable->size () > mTableSizeLimit)
	{
	  RoutingTableEntry *oldestEntry = *(mRoutingTable->getElement (0));

	  delete[]oldestEntry->mFromAddress;
	  delete oldestEntry->mReceivingChannels;

	  mRoutingTable->deleteElement (0);

	  delete oldestEntry;
	}
    }

  // add the receiving channel to this table entry
  entry->mReceivingChannels->push_back (inReceivingChannel);

  // make sure the entry for this address is not too big
  while (entry->mReceivingChannels->size () > mTableEntrySizeLimit)
    {
      // delete oldest entry
      entry->mReceivingChannels->deleteElement (0);
    }

  if (mHistoryOutputFile != NULL)
    {
      HostAddress *address = inReceivingChannel->getHost ();
      fprintf (mHistoryOutputFile, "Received from %s on %s:%d\n",
	       inFromAddress, address->mAddressString, address->mPort);
      delete address;
    }

  mLock->unlock ();
}



void
OutboundChannelManager::clearRoutingInformation (char *inAddress)
{
  mLock->lock ();

  char found = false;

  int tableSize = mRoutingTable->size ();

  for (int i = 0; i < tableSize && !found; i++)
    {

      RoutingTableEntry *currentEntry = *(mRoutingTable->getElement (i));

      if (strcmp (currentEntry->mFromAddress, inAddress) == 0)
	{

	  // remove from queue
	  mRoutingTable->deleteElement (i);

	  delete[]currentEntry->mFromAddress;
	  delete currentEntry->mReceivingChannels;
	  delete currentEntry;

	  found = true;
	}
    }

  if (found)
    {
      if (mHistoryOutputFile != NULL)
	{
	  fprintf (mHistoryOutputFile, "Cleared route info for %s\n",
		   inAddress);
	}
    }

  mLock->unlock ();
}



void
OutboundChannelManager::routeMessage (char *inUniqueID,
				      char *inFromAddress,
				      char *inToAddress,
				      char *inFlags,
				      int inUtilityCounter,
				      char *inMessage,
				      OutboundChannel * inReceivingChannel)
{

  mLock->lock ();

  if (mChannelVector->size () == 0)
    {
      // no outbound channels to route message through
      mLock->unlock ();
      return;
    }

  // add chaff to utility to thwart analysis
  int utilityChaff = mRandSource->getRandomBoundedInt (0, 05);

  int utilityCounter = inUtilityCounter + utilityChaff;
  //int utilityCounter = inUtilityCounter;

  char *fullMessage = autoSprintf ("UniqueID: %s\n"
				   "From: %s\n"
				   "To:   %s\n"
				   "Flags: %s\n"
				   "UtilityCounter: %d\n"
				   "Length: %d\n" "Body:%s",
				   inUniqueID,
				   inFromAddress,
				   inToAddress,
				   inFlags,
				   utilityCounter,
				   strlen (inMessage),
				   inMessage);

  if (strcmp (inToAddress, "ALL") == 0)
    {
      // broadcast to all


      if (strstr (inFlags, "DROP_CHAIN") != NULL)
	{
	  // send the unmodified drop chain message to a fixed-sized
	  // subset of our neighbors, not including the receiving channel
	  int numToSendDropChainTo = muteGetNumNeighborsToSendDropTailsTo ();

	  // make a DROP_TTL message, with the starting TTL value,
	  // and send the message to the rest of our neighbors.

	  char *dropTTLFlag = autoSprintf ("DROP_TTL_%d",
					   muteGetDropTailTreeStartingTTL ());
	  char *tempFlags = muteRemoveFlag (inFlags, "DROP_CHAIN");
	  char *newFlags = muteAddFlag (tempFlags, dropTTLFlag);
	  delete[]tempFlags;
	  delete[]dropTTLFlag;

	  char *newMessage = autoSprintf ("UniqueID: %s\n"
					  "From: %s\n"
					  "To:   %s\n"
					  "Flags: %s\n"
					  "UtilityCounter: %d\n"
					  "Length: %d\n" "Body:%s",
					  inUniqueID,
					  inFromAddress,
					  inToAddress,
					  newFlags,
					  utilityCounter,
					  strlen (inMessage),
					  inMessage);


	  // count how many neighbors we have sent the DROP_CHAIN
	  // version of our message to so far
	  int numSentDropChainTo = 0;

	  int numChannels = mChannelVector->size ();
	  int first = 0;
	  if (numChannels >3)
	      first = mRandSource->getRandomBoundedInt (0,
							  numChannels
							  - 3);
	  //for (int i = first, send=0; i < numChannels && send<3; i++)
	  for (int i = first, send=0; i < numChannels ; i++)
	    {
	      OutboundChannel *channel = *(mChannelVector->getElement (i));

	      // if inReceivingChannel is NULL, this formula will
	      // always be true
	      if (channel != inReceivingChannel)
		{
		  if (numSentDropChainTo < numToSendDropChainTo)
		    {
		      // send DROP_CHAIN message to an additional neighbor
		      channel->sendMessage (fullMessage);
		      numSentDropChainTo++;
		      send++;

		      if (mHistoryOutputFile != NULL)
			{
			  HostAddress *address = channel->getHost ();
			  fprintf (mHistoryOutputFile,
				   "%s, From %s to %s, Flags: %s, "
				   "drop tree pick: %s:%d\n",
				   inUniqueID, inFromAddress,
				   inToAddress,
				   inFlags,
				   address->mAddressString, address->mPort);
			  delete address;
			}
		    }
		  else
		    {
		      // Already sent the DROP_CHAIN message to enough
		      // of our neighbors.
		      // Send the DROP_TTL message to the remaining
		      // neighbors
		      // NOTE:  if numToSendDropChainTo is 0, then
		      // we end up sending a DROP_TTL message to all
		      // neighbors.  We must do this to preserve anonymity
		      // (so attackers cannot tell whether or not we
		      // are sending on DROP_CHAIN messages or not).
		      channel->sendMessage (newMessage);
		      send++;
		    }
		}
	    }

	  if (mHistoryOutputFile != NULL)
	    {
	      fprintf (mHistoryOutputFile,
		       "%s, From %s to %s, Flags: %s, "
		       "broadcast to other neighbors\n",
		       inUniqueID, inFromAddress, inToAddress, newFlags);
	    }

	  delete[]newFlags;
	  delete[]newMessage;
	}
      else
	{
	  // no DROP_CHAIN, send same message to all neighbors
	  // (even if DROP_TTL, since the TTL was decremented
	  //  before routeMessage was called)

	  int numChannels = mChannelVector->size ();
	  int first = 0;
	  if (numChannels >3)
	      first = mRandSource->getRandomBoundedInt (0,
							  numChannels);
	  for (int i = first,send=0,count=0; count < numChannels && send<3; i++,count++)
	    {
              if (i >= numChannels) i=0;
	      OutboundChannel *channel = *(mChannelVector->getElement (i));

	      // if inReceivingChannel is NULL, this formula will
	      // always be true
	      if (channel != inReceivingChannel
	         // we try to not overload connections
                 && ( channel->getQueuedMessageCount()<10 || count >= (numChannels-2)))
		{
		  channel->sendMessage (fullMessage);
		  send++;
		}
	    }

	  if (mHistoryOutputFile != NULL)
	    {
	      fprintf (mHistoryOutputFile,
		       "%s, From %s to %s, Flags: %s, "
		       "broadcast to all\n",
		       inUniqueID, inFromAddress, inToAddress, inFlags);
	    }
	}
    }
  else
    {
      // route

      // we need to look up the to-address to decide how to route it
      RoutingTableEntry *entry = NULL;

      int tableSize = mRoutingTable->size ();

      for (int i = 0; i < tableSize && entry == NULL; i++)
	{

	  RoutingTableEntry *currentEntry = *(mRoutingTable->getElement (i));

	  if (strcmp (currentEntry->mFromAddress, inToAddress) == 0)
	    {
	      entry = currentEntry;
	    }
	}


      // if we have an entry and it has some routing information in it
      if (entry != NULL && entry->mReceivingChannels->size () != 0)
	{



	  // pick a channel at random until we find a good one

	  // make sure we don't loop too long making random choices
	  // in certain situations, we can actually loop forever
	  int loopCount = 0;
	  int maxLoopCount = 20;

	  int index;
	  char sent = false;
	  while (!sent)
	    {
	      int numReceivingChannels = entry->mReceivingChannels->size ();

	      // with small probability (or by default if we've emptied the
	      // queue or looped too many times), broadcast to all
	      // this will make our routing algorithm robust in the long
	      // term (we can discover new, faster routes as they become
	      // available)
	      if (numReceivingChannels == 0 ||
		  mRandSource->getRandomDouble () < mUniformProbability ||
		  loopCount >= maxLoopCount)
		{

		  // make sure this message is not marked as ROUTE_ONLY
		  // but ignore ROUTE_ONLY if FRESH_ROUTE is set
		  if (strstr (inFlags, "ROUTE_ONLY") != NULL &&
		      strstr (inFlags, "FRESH_ROUTE") == NULL)
		    {
		      // drop the message

		      if (mHistoryOutputFile != NULL)
			{
			  fprintf (mHistoryOutputFile,
				   "%s, From %s to %s, Flags: %s, "
				   "chose broadcast to all, "
				   "but ROUTE_ONLY set, "
				   "so dropping message\n",
				   inUniqueID, inFromAddress, inToAddress,
				   inFlags);
			}

		    }
		  else
		    {

		      int numChannels = mChannelVector->size ();
	              int first = 0;
	              if (numChannels >3)
	                  first = mRandSource->getRandomBoundedInt (0,
							  numChannels );
		      for (int i = first,send=0,count=0; count < numChannels && send<3; i++,count++)
			{
			  if(i>= numChannels) i=0;
			  OutboundChannel *channel =
			    *(mChannelVector->getElement (i));

			  // make sure we don't route message back to
			  // the node that sent it to us
			  // if inReceivingChannel is NULL, this formula will
			  // always be true
			  if (channel != inReceivingChannel
				// we try to not overload connections
				&& ( channel->getQueuedMessageCount()<10 || count >= (numChannels-2)))
			    {
			      channel->sendMessage (fullMessage);
			      send++;
			    }
			}

		      if (mHistoryOutputFile != NULL)
			{
			  fprintf (mHistoryOutputFile,
				   "%s, From %s to %s, Flags: %s, "
				   "chose broadcast to all\n",
				   inUniqueID, inFromAddress, inToAddress,
				   inFlags);
			}
		    }

		  // set to true even though we may have dropped the message
		  // if ROUTE_ONLY was set
		  // Thus, we are really using the sent variable to indicate
		  // that the message has been dealt with
		  sent = true;
		}
	      else
		{

		  if (mUseMajorityRouting)
		    {
		      int numChannels = mChannelVector->size ();

		      int i;

		      // count how many messages from this destination
		      // node have come through each channel
		      int *channelRouteCounts = new int[numChannels];
		      for (i = 0; i < numChannels; i++)
			{
			  channelRouteCounts[i] = 0;
			}

		      // walk through the table entries
		      int numChannelsInEntry =
			entry->mReceivingChannels->size ();

		      for (i = 0; i < numChannelsInEntry; i++)
			{
			  OutboundChannel *channel =
			    *(entry->mReceivingChannels->getElement (i));

			  // make sure channel is still live
			  int channelIndex =
			    mChannelVector->getElementIndex (channel);

			  if (channelIndex != -1)
			    {
			      channelRouteCounts[channelIndex]++;
			    }
			  else
			    {
			      // channel bad, but still in this
			      // from-address' queue
			      // clean up the queue
			      entry->mReceivingChannels->
				deleteElement (i);
			    }
			}

		      // now find the majority
		      int majorityIndex = -1;
		      int majorityRouteCount = 0;
		      for (i = 0; i < numChannels; i++)
			{
			  if (channelRouteCounts[i] > majorityRouteCount)
			    {
			      majorityRouteCount = channelRouteCounts[i];
			      majorityIndex = i;
			    }
			}

		      if (majorityIndex != -1)
			{
			  // found a majority
			  OutboundChannel *channel =
			    *(mChannelVector->getElement (majorityIndex));

			  if (channel != inReceivingChannel)
			    {
			      channel->sendMessage (fullMessage);

			      if (mHistoryOutputFile != NULL)
				{
				  HostAddress *address = channel->getHost ();
				  fprintf (mHistoryOutputFile,
					   "%s, From %s to %s, Flags: %s, "
					   "majority backroute pick: %s:%d\n",
					   inUniqueID, inFromAddress,
					   inToAddress,
					   inFlags,
					   address->mAddressString,
					   address->mPort);
				  delete address;
				}
			    }
			  else
			    {
			      AppLog::error ("OutboundChannelManager",
					     "Majority backroute pick is the same"
					     " as the receiving channel.  "
					     "Dropping message.");
			    }

			  // flag as sent even if we didn't actually
			  // send it because channel == inReceivingChannel
			  // (flagging as sent essentially drops the message
			  //  here)
			  sent = true;
			}

		      delete[]channelRouteCounts;
		    }
		  else
		    {
		      // use probabalistic routing choice
		      index =
			mRandSource->getRandomBoundedInt (0,
							  numReceivingChannels
							  - 1);

		      OutboundChannel *channel =
			*(entry->mReceivingChannels->getElement (index));

		      // make sure channel is still live
		      int channelIndex =
			mChannelVector->getElementIndex (channel);

		      if (channelIndex != -1)
			{
			  // channel still good

			  // make sure we don't route message back to
			  // the node that sent it to us
			  // if inReceivingChannel is NULL, this formula will
			  // always be true
			  if (channel != inReceivingChannel)
			    {
			      channel->sendMessage (fullMessage);

			      if (mHistoryOutputFile != NULL)
				{
				  HostAddress *address = channel->getHost ();
				  fprintf (mHistoryOutputFile,
					   "%s, From %s to %s, Flags: %s, "
					   "probability backroute pick: %s:%d\n",
					   inUniqueID, inFromAddress,
					   inToAddress,
					   inFlags,
					   address->mAddressString,
					   address->mPort);
				  delete address;
				}

			      sent = true;
			    }
			  // else pick another
			}
		      else
			{
			  // channel bad, but still in this
			  // from-address' queue
			  // clean up the queue
			  entry->mReceivingChannels->deleteElement (index);
			  // and try again
			}
		    }
		}

	      loopCount++;
	    }
	}
      else
	{
	  // no route information for this to-address

	  // default to broadcast

	  // make sure this message is not marked as ROUTE_ONLY
	  // but ignore ROUTE_ONLY if FRESH_ROUTE is set
	  if (strstr (inFlags, "ROUTE_ONLY") != NULL &&
	      strstr (inFlags, "FRESH_ROUTE") == NULL)
	    {
	      // drop the message

	      if (mHistoryOutputFile != NULL)
		{
		  fprintf (mHistoryOutputFile,
			   "%s, From %s to %s, Flags: %s, no route info, "
			   "dropping message\n",
			   inUniqueID, inFromAddress, inToAddress, inFlags);
		}

	    }
	  else
	    {
	      // broadcast to all neighbors to search for a new route

	      int numChannels = mChannelVector->size ();
	      int first = 0;
	      if (numChannels >3)
	          first = mRandSource->getRandomBoundedInt (0,
							  numChannels
							  - 3);
	      for (int i = first,send=0; i < numChannels && send<3; i++)
		{
		  OutboundChannel *channel =
		    *(mChannelVector->getElement (i));

		  // make sure we don't route message back to
		  // the node that sent it to us
		  // if inReceivingChannel is NULL, this formula will
		  // always be true
		  if (channel != inReceivingChannel)
		    {
		      channel->sendMessage (fullMessage);
		      send++;
		    }
		}

	      if (mHistoryOutputFile != NULL)
		{
		  fprintf (mHistoryOutputFile,
			   "%s, From %s to %s, Flags: %s, no route info, "
			   "broadcast to all\n",
			   inUniqueID, inFromAddress, inToAddress, inFlags);
		}
	    }

	}
    }

  delete[]fullMessage;

  if (mHistoryOutputFile != NULL)
    {
      fflush (mHistoryOutputFile);
    }



  // check for channels that have too many dropped messages
  int numChan = mChannelVector->size ();

  // extract a copy of the host from each channel
  for (int i = 0; i < numChan; i++)
    {
      OutboundChannel *chan = *(mChannelVector->getElement (i));

      int sentCount = chan->getSentMessageCount ();
      int droppedCount = chan->getDroppedMessageCount ();

      // watch for drop counts that are too high
      if (droppedCount > sentCount * mMaxDroppedMessageFraction)
	{
	  AppLog::error ("OutboundChannelManager",
			 "Too many dropped messages, "
			 "forcing connection to break.");

	  SocketManager::breakConnection (*(mSocketVector->getElement (i)));
	}
    }


  mLock->unlock ();
}



SimpleVector <
  HostAddress *
  >*OutboundChannelManager::getConnectedHosts (int **outSentMessageCounts,
					       int **outQueuedMessageCounts,
					       int **outDroppedMessageCounts)
{

  mLock->lock ();


  int
    numChan = mChannelVector->size ();

  SimpleVector < HostAddress * >*returnVector =
    new SimpleVector < HostAddress * >();

  int *
    sentCounts = new int[numChan];
  int *
    queueCounts = new int[numChan];
  int *
    droppedCounts = new int[numChan];

  // extract a copy of the host from each channel
  for (int i = 0; i < numChan; i++)
    {
      OutboundChannel *
	chan = *(mChannelVector->getElement (i));

      returnVector->push_back (chan->getHost ());

      sentCounts[i] = chan->getSentMessageCount ();
      queueCounts[i] = chan->getQueuedMessageCount ();
      droppedCounts[i] = chan->getDroppedMessageCount ();
    }


  mLock->unlock ();

  if (outSentMessageCounts != NULL)
    {
      *outSentMessageCounts = sentCounts;
    }
  else
    {
      delete[]sentCounts;
    }

  if (outQueuedMessageCounts != NULL)
    {
      *outQueuedMessageCounts = queueCounts;
    }
  else
    {
      delete[]queueCounts;
    }

  if (outDroppedMessageCounts != NULL)
    {
      *outDroppedMessageCounts = droppedCounts;
    }
  else
    {
      delete[]droppedCounts;
    }

  return returnVector;
}



int
OutboundChannelManager::getConnectionCount ()
{
  mLock->lock ();

  int count = mChannelVector->size ();

  mLock->unlock ();

  return count;
}
