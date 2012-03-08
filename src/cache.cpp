/***************************************************************************
 *   Copyright (C) 2008                                                    *
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

#include <QtDebug>
#include <QSemaphore>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include "MUTE/LocalAddressReceiver.h"
#include "MUTE/messageRouter.h"
#include "MUTE/fileShare.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/network/web/URLUtils.h"
#include "minorGems/formats/encodingUtils.h"

#include "download.h"
#include "statistic.h"
#include "cache.h"

extern char *muteShareVirtualAddress;

#define makeRand(max) (int)(((double)((max))*(double)qrand()) / ((double)RAND_MAX + 1.0) )

CHASH *Cache = NULL;
int maxCache = 0;
int nbCache = 0;
QSemaphore semCache(5);

CCHUNK *CacheChunk = NULL;
int maxCacheChunk = 0;
int nbCacheChunk = 0;

int muteHandlerID = -1;

bool
addCachedFile (char *host, char *name, char *hash, char *size, char *comment)
{
  int pos;
  qlonglong llSize = QString (size).toLongLong ();
  if( !hash || strlen(hash) <10) return false;
  
  // search if already in cache
  for (pos = 0; pos < nbCache; pos++)
    {
      if (Cache[pos].size == llSize && !strcmp (Cache[pos].hash, hash)
	  && !strcmp (Cache[pos].host, host))
	{
	  // found : just change time
	  (*Cache[pos].time) = QDateTime::currentDateTime ();
	  return true;
	}
    }
  // not found : add it.
  semCache.acquire(5);
  if (nbCache >= maxCache && nbCache < MAX_SEARCH_CACHE)
    {
      maxCache += 1000;
      Cache = (CHASH *) realloc (Cache, (maxCache + 1) * sizeof (CHASH));
    }
  if (nbCache >= maxCache)
    {
      //TODO : clear up some old values rather than a random value
      // clear up an old value
      int pos0 = makeRand (nbCache-1);
      if (pos0 >= nbCache-1)
	pos0 = nbCache - 2;
      free(Cache[pos0].host);
      free(Cache[pos0].name);
      free(Cache[pos0].hash);
      delete Cache[pos0].time;
      delete Cache[pos0].comment;
      Cache[pos0]=Cache[nbCache-1];
      nbCache--;
    }
  Cache[nbCache].host = strdup (host);
  Cache[nbCache].name = strdup (name);
  Cache[nbCache].hash = strdup (hash);
  Cache[nbCache].size = llSize;
  Cache[nbCache].time = new QDateTime ();
  Cache[nbCache].comment = new QString ("");
  Cache[nbCache].comment->append (comment);

  (*Cache[nbCache].time) = QDateTime::currentDateTime ();

  nbCache++;
  semCache.release(5);

  return true;
}

bool
addCachedChunk (char *host, char *name, int chunk, long size, unsigned char*data)
{
  int pos;
  if( !name || strlen(name) <10) return false;
  
  // search if already in cache
  for (pos = 0; pos < nbCacheChunk; pos++)
    {
      if (CacheChunk[pos].chunkNumber == chunk && !strcmp (CacheChunk[pos].name, name)
	  && !strcmp (CacheChunk[pos].host, host))
	{
	  // found : just change time
	  (*CacheChunk[pos].time) = QDateTime::currentDateTime ();
	  return true;
	}
    }
  // not found : add it.
  semCache.acquire(5);
  if (nbCacheChunk >= maxCacheChunk && nbCacheChunk < MAX_CHUNK_CACHE)
    {
      maxCacheChunk += 1000;
      CacheChunk = (CCHUNK *) realloc (CacheChunk, (maxCacheChunk + 1) * sizeof (CCHUNK));
    }
  if (nbCacheChunk >= maxCacheChunk)
    {
      //TODO : clear up some old values rather than a random value
      // clear up an old value
      int pos0 = makeRand (nbCacheChunk-1);
      if (pos0 >= nbCacheChunk-1)
	pos0 = nbCacheChunk - 2;
      free(CacheChunk[pos0].host);
      free(CacheChunk[pos0].name);
      free(CacheChunk[pos0].data);
      delete CacheChunk[pos0].time;
      CacheChunk[pos0]=CacheChunk[nbCacheChunk-1];
      nbCacheChunk--;
    }
  CacheChunk[nbCacheChunk].host = strdup (host);
  CacheChunk[nbCacheChunk].name = strdup (name);
  CacheChunk[nbCacheChunk].chunkNumber = chunk ;
  CacheChunk[nbCacheChunk].size = size;
  CacheChunk[nbCacheChunk].data = (unsigned char *)malloc(size);
  memcpy(CacheChunk[nbCacheChunk].data,data,size);
  CacheChunk[nbCacheChunk].time = new QDateTime ();

  (*CacheChunk[nbCacheChunk].time) = QDateTime::currentDateTime ();

  nbCacheChunk++;
  semCache.release(5);

  return true;
}

int
searchCacheHandler (char *inFromAddress,
		    char *inToAddress, char *inBody, void *inExtraParam)
{
  SimpleVector < char *>*tokens = tokenizeString (inBody);
  int numTokens = tokens->size ();
  int utility =0;

  if (numTokens >= 6)
    {
      char *typeToken = *(tokens->getElement (1));
      if (strcmp ("SearchResults", typeToken) == 0)
	{
	  char *searchID = *(tokens->getElement (3));
	  char *resultCountString = *(tokens->getElement (5));
	  int resultCount = strtol (resultCountString, (char **) NULL, 10);
	  int firstResultToken = 7;
	  int lastResultToken = firstResultToken + 3 * resultCount;
	  if (numTokens >= lastResultToken)
	    {
	      for (int i = firstResultToken; i < lastResultToken - 2; i += 3)
		{
		  char *encodedFileName = *(tokens->getElement (i));
		  char *filePath = URLUtils::hexDecode (encodedFileName);
		  char *lengthString = *(tokens->getElement (i + 1));
		  char *hashString = *(tokens->getElement (i + 2));
		  addCachedFile (inFromAddress, filePath, hashString,
				 lengthString, NULL);
		  delete [] filePath;
		}

	    }
	}
      else if (strcmp ("FileChunk", typeToken) == 0)
	{
	  // don't treat messages for you
	  if (!strcmp (muteShareVirtualAddress, inToAddress))
	  {
	    goto endHandler;
	  }
	  char *encodedPath = *(tokens->getElement (3));
	  char *filePath = URLUtils::hexDecode (encodedPath);
	  char *chunkNumberString = *(tokens->getElement (5));
	  errno = 0;
	  int chunkNumber = strtol (chunkNumberString, (char **) NULL, 10);
	  if (!errno && numTokens >= 10)
	    {
	      char *lengthString = *(tokens->getElement (7));
	      int length = strtol (lengthString, (char **) NULL, 10);
	      char *encodedData = *(tokens->getElement (9));
	      int dataLength;
	      unsigned char *decodedData =
		base64Decode (encodedData, &dataLength);
	      if (decodedData != NULL)
		{
		  if (dataLength == length)
		    {
                        char * vides = (char *) calloc(1, Download::BYTE_PER_CHUNK);
 			if (!memcmp(decodedData,vides,Download::BYTE_PER_CHUNK))
			{
			  qWarning() << QTime::currentTime().toString(QString::fromUtf8("hh:mm:ss")) <<" 16384 zeros chunk for file "<<filePath;
			}
			else
			{
		          // save chunk in cache
                          addCachedChunk (inFromAddress, filePath, chunkNumber, dataLength, decodedData);
		        }
		    delete[]decodedData;
		    }
		  else
		    delete[]decodedData;
		}
	    }
	   delete[]filePath;
	}
      else if ((strcmp ("SearchRequest", typeToken) == 0) && (nbCache > 0))
	{

	  // don't treat messages for you
	  if (!strcmp (muteShareVirtualAddress, inToAddress))
	  {
	    goto endHandler;
	  }
	  char *searchID = *(tokens->getElement (3));
	  char *encodedSearchString = *(tokens->getElement (5));

	  char *searchString = URLUtils::hexDecode (encodedSearchString);

	  // check if this is a hash-only search
	  char hashOnlySearch = false;

	  const char *hashStart = "hash_";

	  char *pointerToHashStart = strstr (searchString, hashStart);
	  if (pointerToHashStart != NULL)
	    {
	      // search contains a hash-only search.
	      // extract hash, and ignore any other search terms

	      char *pointerToHash = &(pointerToHashStart[strlen (hashStart)]);

	      // hash should be at most 40 characters long
	      char *hashString = new char[41];

	      int numRead = sscanf (pointerToHash, "%40s", hashString);

	      if (numRead == 1)
		{
		  delete[]searchString;
		  searchString = hashString;

		  hashOnlySearch = true;
		}
	      else
		{
		  delete[]hashString;
		}
	    }



	  SimpleVector < char *>*searchTerms = tokenizeString (searchString);
	  int numSearchTerms = searchTerms->size ();

	  delete[]searchString;

	  char *virtualHost = NULL;
	  if (strcmp (inToAddress, "ALL"))
	    virtualHost = inToAddress;
	  // ignore searches that contain no terms
	  if (numSearchTerms > 0)
	    {

	      SimpleVector < char *>*hits = new SimpleVector < char *>();

	      // for now, trim our results so that they fit
	      // in one message

	      // save 4 KiB for our headers, 28 KiB for results
	      int maxResultsLength = 28672;
	      int totalResultLength = 0;

	      char hitLimit = false;

	      // search for results in cache
	      int pos0 = makeRand (nbCache);
	      if (pos0 >= nbCache)
		pos0 = nbCache - 1;
	      int pos;
	      int i;
	      bool locked = semCache.tryAcquire(1,1000);
	      for (i = 0, pos = pos0 + 1; !hitLimit && i < nbCache; pos++, i++)
		{
		  if (pos >= nbCache)
		    pos = 0;
		  char *fileName = Cache[pos].name;

		  if (fileName != NULL)
		    {

		      char hitAll = true;

		      if (!hashOnlySearch)
			{
			  // check each term
			  for (int j = 0; j < numSearchTerms && hitAll; j++)
			    {

			      char *term = *(searchTerms->getElement (j));
			      if (stringLocateIgnoreCase (fileName,
							  term) == NULL)
				{

				  // missed this term
				  hitAll = false;
				}
			    }
			}

		      if (hitAll && Cache[pos].hash)
			{

			  char *hashString = Cache[pos].hash;

			  // check hash if this is a hash-only search
			  char resultIsAHit = true;

			  if (hashOnlySearch)
			    {
			      char *hashTerm = *(searchTerms->getElement (0));
			      if (strcasecmp (hashString, hashTerm) != 0)
				{

				  resultIsAHit = false;
				}
			    }


			  if (resultIsAHit)
			    {

			      if (!virtualHost)
				{
				  virtualHost = Cache[pos].host;
				}
			      if (!strcmp (virtualHost, Cache[pos].host))
				{
				  char *encodedFileName =
				    URLUtils::hexEncode (fileName);

				  char *resultString =
#ifdef __MINGW32__
				    autoSprintf ("%s %I64d %s",
#else
				    autoSprintf ("%s %lld %s",
#endif
						 encodedFileName,
						 Cache[pos].size,
						 hashString);
				  delete[]encodedFileName;

				  int resultLength = strlen (resultString);

				  if (totalResultLength +
				      resultLength + 1 < maxResultsLength)
				    {
				      // not at limit yet
				      hits->push_back (resultString);
				      totalResultLength += resultLength + 1;
				    }
				  else
				    {
				      // hit limit
				      hitLimit = true;
				      delete[]resultString;
				    }
				}
			    }
			}
		    }
		}
	      if (locked) semCache.release(1);

	      int numHits = hits->size ();

	      if (numHits != 0)
		{
		  char **hitArray = hits->getElementArray ();

		  // entries delimited by newlines
		  char *hitString = join (hitArray,
					  numHits,
					  "\n");
		  for (int i = 0; i < numHits; i++)
		    {
		      delete[]hitArray[i];
		    }
		  delete[]hitArray;


		  char *message =
		    autoSprintf ("MessageType: SearchResults\n"
				 "SearchID: %s\n" "ResultCount: %d\n"
				 "Results: %s",
				 searchID,
				 numHits,
				 hitString);
		  delete[]hitString;

		  // use ROUTE_ONLY flag to reduce excess traffic
		  // from lost search results
		  muteSendMessage (virtualHost,
				   inFromAddress, message, "ROUTE_ONLY");

		  delete[]message;
		  utility += numHits;
		}

	      delete hits;
	    }

	  for (int j = 0; j < numSearchTerms; j++)
	    {
	      char *term = *(searchTerms->getElement (j));
	      delete[]term;
	    }
	  delete searchTerms;
	}
      else if ((strcmp ("FileChunkRequest", typeToken) == 0) && (nbCacheChunk > 0))
	{
	  char *encodedPath = *(tokens->getElement (3));
	  char *path = URLUtils::hexDecode (encodedPath);
	  char *typeReq = *(tokens->getElement (2));
	  char *chunkNumberString = *(tokens->getElement (5));

	  int chunkNumber = strtol (chunkNumberString, (char **) NULL, 10);
	  //TODO : search chunk in cache
	  for (int pos=0 ; pos<nbCacheChunk ; pos++)
	  {
	    if(strcmp(path,CacheChunk[pos].name)) continue;
	    if(strcmp(inToAddress,CacheChunk[pos].host)) continue;
	    if(chunkNumber != CacheChunk[pos].chunkNumber) continue;
	    if(CacheChunk[pos].size >0) continue;
	    {
	      // base64 encode with no line breaks
	      char *encodedChunkData =
		base64Encode (CacheChunk[pos].data, CacheChunk[pos].size,
			      false);

	      char *message =
		autoSprintf ("MessageType: FileChunk\n"
			     "FilePath: %s\n"
			     "ChunkNumber: %d\n"
			     "ChunkLength: %d\n"
			     "ChunkData: %s",
			     encodedPath,
			     chunkNumber,
			     CacheChunk[pos].size,
			     encodedChunkData);

	      delete[]encodedChunkData;

	      // use ROUTE_ONLY flag to avoid wasting
	      // bandwidth on lost chunks
	      // use inToAddress as sender
	      muteSendMessage (inToAddress,
			       inFromAddress,
			       message, "ROUTE_ONLY");
	      delete[]message;

	      StatLock.lock ();
	      TotBytesUploaded += CacheChunk[pos].size;
	      StatLock.unlock ();
	      // increase utility to avoid to route this request
	      utility += 1000;
	    }
	  }
	}
    }
endHandler:
  for (int i = 0; i < numTokens; i++)
    {
      delete[](*(tokens->getElement (i)));
    }
  delete tokens;

  return utility;
}

void
startCache ()
{
  uint seed =
    QDateTime::currentDateTime ().toTime_t () + QTime::currentTime ().msec ();
  qsrand (seed);
  if (muteHandlerID == -1)
    muteHandlerID =
      muteAddGlobalMessageHandler (searchCacheHandler, (void *) NULL);
}

void
stopCache ()
{
  if (muteHandlerID != -1)
    {
      muteRemoveGlobalMessageHandler (muteHandlerID);
      muteHandlerID = -1;
    }
  // clear cache
  int savnb=nbCache;
  nbCache=0;
  maxCache=0;
  int savnbChunk=nbCacheChunk;
  nbCacheChunk=0;
  maxCacheChunk=0;
  bool locked = semCache.tryAcquire(5,1000);
  for (int pos=0 ; pos<savnb ; pos++)
  {
      free(Cache[pos].host);
      free(Cache[pos].name);
      free(Cache[pos].hash);
      delete Cache[pos].time;
      delete Cache[pos].comment;
  }
  free (Cache);
  Cache=NULL;
  for (int pos=0 ; pos<savnbChunk ; pos++)
  {
      free(CacheChunk[pos].host);
      free(CacheChunk[pos].name);
      free(CacheChunk[pos].data);
      delete CacheChunk[pos].time;
  }
  free (CacheChunk);
  CacheChunk=NULL;
  if (locked) semCache.release(5);
}
