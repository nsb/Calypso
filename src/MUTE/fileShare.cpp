/***************************************************************************
 *   Copyright (C) 2007 kommute dev team                                   *
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

#include "fileShare.h"
#include "MUTE/messageRouter.h"
#include "MUTE/common/CryptoUtils.h"


#include <QFile>
#include "minorGems/io/file/File.h"
#include "minorGems/io/file/Path.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/network/web/URLUtils.h"
#include "MUTE/MimeTyper.h"
#include "minorGems/formats/encodingUtils.h"

#include <QSemaphore>
#include <QMutex>
#include <QTime>
#include <QThread>
#include <QString>
#include "MUTE/FinishedSignalThread.h"
#include "MUTE/StopSignalThread.h"

#include "minorGems/util/SimpleVector.h"

#include "applog.h"

#include "MUTE/sha1.h"
#include <QCryptographicHash>
#include <QtDebug>

#include "minorGems/util/random/RandomSource.h"
#include "minorGems/util/random/StdRandomSource.h"

#include <QDateTime>
#include "settings.h"
#include "statistic.h"
#include "hashthread.h"
#include "download.h"

extern class Settings *settings;

extern SHASH *Shares;
extern int maxShares;
extern int nbShares;

extern Download **Downloads;
extern int maxDownloads;
extern int nbDownloads;

int muteShareFileInfoSenderID = -1;
int muteShareFileChunkSenderID = -1;
int muteShareSearchResultsSenderID = -1;
MimeTyper *muteShareMimeTyper = NULL;
char *muteShareVirtualAddress = NULL;

QMutex muteUploadDataLock;
int muteNextUploadID = 0;
SimpleVector < long >*muteUploadIDs = NULL;
SimpleVector < char *>*muteUploadFilePaths = NULL;
SimpleVector < char *>*muteUploadHostAddresses = NULL;
SimpleVector < long >*muteUploadLastChunksSent = NULL;
SimpleVector < long >*muteUploadChunksInFiles = NULL;
SimpleVector < unsigned long >*muteUploadFirstChunkTimes = NULL;
SimpleVector < unsigned long >*muteUploadLastChunkTimes = NULL;

QMutex muteDownloadDataLock;
int muteNextDownloadID = 0;
SimpleVector < long >*muteDownloadIDs = NULL;
SimpleVector < QSemaphore * >*muteDownloadSemaphores = NULL;
SimpleVector < char >*muteDownloadInterruptedFlags = NULL;
SimpleVector < SourceQueue * >*muteDownloadSourceQueues = NULL;

long muteNumActiveDownloads = 0;
long muteMaxSimultaneousDownloads = 0;
// queue of download IDs that are waiting for a slot
SimpleVector < long >*muteDownloadQueue = NULL;


// chunk size is 16 KiB.  After base64 encoding, it will take up less than
// 22 KiB in a MUTE message body, leaving room for various headers.
// The size limit on a MUTE message is 32 KiB
//
// In previous versions, the chunk size was 22 KiB before encoding.
// We now use 16KiB so that the chunk size is a power of 2 and therefore
// compatible with many partial hashing schemes (TigerTrees, in particular)
int muteShareChunkSize = 16384;


char muteSharePrintSearchSyncTrace = false;




// tag that is present in all internal directories that should not
// be remotely searchable
const char *  muteInternalDirectoryTag = "MUTE_INTERNAL";

// sub-directory in shared directory where file hashes are cached
//char *muteHashDirectoryName = "MUTE_INTERNAL_hashes";
// sub-directory in shared directory where incoming files are kept
const char * muteIncomingDirectoryName = "MUTE_INTERNAL_incoming";

#define SEARCH_TIME_MS 20000

/**
 * Translates a URL-safe encoded path into a file object.
 *
 * @param inEncodedPath the URL-safe path.  Must be destroyed by caller.
 *
 * @return a file object, or NULL if the file does not exist.
 *   Must be destroyed by caller.
 */
QFile *
muteShare_internalEncodedPathToFile (char *inEncodedPath)
{
  char *path = URLUtils::hexDecode (inEncodedPath);

  if (strstr (path, ".."))
    {
      // path may be trying to escape from our file directory
      // block it

      // note:  this may also block file names with "..", like,
      // test..txt
      // but who really names files like this anyway?
      // should really look for /.. or ../, but this might not be as
      // safe (what if, on windows, someone requests \.. directly,
      //       even though this isn't a valid MUTE path)

      delete[]path;
      return NULL;
    }



  char *sharingPathString = muteShareGetSharingPath ();

  QFile *file = NULL;


  QString fullName = QString (sharingPathString) + "/" + path;
  {
    QFileInfo fInfo (fullName);
    if (fInfo.exists () && fInfo.isFile())
      {
	file = new QFile (fullName);
      }
  }

  if (!file)
    {
      QStringList otherDir = settings->getOtherSharedPathSetting ();
      for (int i = 0; i < otherDir.count (); i++)
	{
	  fullName = otherDir.at (i) + "/" + path;
	  QFileInfo fInfo (fullName);
	  if (fInfo.exists () && fInfo.isFile())
	    {
	      file = new QFile (fullName);
	      break;
	    }
	}
    }
  delete[]sharingPathString;
  delete[]path;
  return file;
}



/**
 * Translates a file object into a URL-safe encoded path.
 *
 * Note that this call returns NULL for files that are inside
 * MUTE_INTERNAL_hashes and MUTE_INTERNAL_incoming directories.
 *
 * @param inFile a file object.  Must be destroyed by caller.
 *
 * @return the URL-safe path, or NULL on error.
 *   Must be destroyed by caller.
 */
char *
muteShare_internalFileToEncodedPath (File * inFile)
{

  char platformDelim = Path::getDelimeter ();

  char *sharingPathString = muteShareGetSharingPath ();

  char *fullFilePathString = inFile->getFullFileName ();


  if (strstr (fullFilePathString, muteInternalDirectoryTag) != NULL)
    {

      // ignore files in our internal directories

      delete[]sharingPathString;
      delete[]fullFilePathString;

      return NULL;
    }


  // remove sharing path from file name
  char *pointerToSharingPath = strstr (fullFilePathString,
				       sharingPathString);

  char *returnPathString = NULL;
  if (pointerToSharingPath != NULL)
    {

      // skip the sharing path
      char *partialFilePath =
	&(fullFilePathString[strlen (sharingPathString)]);


      if (strlen (partialFilePath) > 0)
	{
	  // make sure we didn't miss the final delimiter
	  if (partialFilePath[0] == platformDelim)
	    {

	      // skip the first character (delimeter
	      partialFilePath = &(partialFilePath[1]);
	    }
	}

      if (platformDelim == '/')
	{
	  // already using universal path delimeter
	  returnPathString = stringDuplicate (partialFilePath);
	}
      else
	{
	  // replace the platform-specific path delimeter with the universal
	  // delimeter "/"
	  char *platformDelimString = autoSprintf ("%c", platformDelim);

	  char found;

	  returnPathString = replaceAll (partialFilePath,
					 platformDelimString, "/", &found);

	  delete[]platformDelimString;
	}
    }

  delete[]sharingPathString;
  delete[]fullFilePathString;

  return returnPathString;
}



// handler for FileInfoRequests
int
muteShare_internalFileInfoSender (char *inFromAddress, char *inToAddress,
				  char *inBody, void *inExtraArgument)
{

  // is the message a FileInfoRequest?
  // look at second token
  SimpleVector < char *>*tokens = tokenizeString (inBody);
  int numTokens = tokens->size ();

  if (numTokens >= 4)
    {
      char *typeToken = *(tokens->getElement (1));
      if (strcmp ("FileInfoRequest", typeToken) == 0)
	{

	  char *encodedPath = *(tokens->getElement (3));
	  //char *path = URLUtils::hexDecode (encodedPath);

	  QFile *file = muteShare_internalEncodedPathToFile (encodedPath);

	  if (file != NULL)
	    {

	      // file exists, send an info message
	      int fileLength;
	      char *mimeType;
	      int numChunks;


	      fileLength = file->size ();

	      mimeType = muteShareMimeTyper->getFileNameMimeType (file->fileName ().toUtf8().data());

	      if (mimeType == NULL)
		{
		  // default type
		  mimeType = stringDuplicate ("application/octet-stream");
		}

	      numChunks = fileLength / muteShareChunkSize;

	      if (fileLength % muteShareChunkSize != 0)
		{
		  // extra partial chunk
		  numChunks += 1;
		}

	      char *message = autoSprintf ("MessageType: FileInfo\n"
					   "FilePath: %s\n"
					   "FileStatus: Found\n"
					   "FileSize: %d\n"
					   "ChunkCount: %d\n" "MimeType: %s",
					   encodedPath,
					   fileLength,
					   numChunks,
					   mimeType);

	      delete[]mimeType;

	      // use ROUTE_ONLY flag when sending back FileInfo
	      muteSendMessage (muteShareVirtualAddress,
			       inFromAddress, message, "ROUTE_ONLY");

	      delete[]message;
	    }
	  else
	    {
	      if(strcmp(inToAddress,"ALL"))
	      {
//TODO : try to find file in  downloads
	      AppLog::error ("fileTransfer",
			     "FileInfoRequest does not contain a valid file path");

	      // send back a not found message
	      char *message = autoSprintf ("MessageType: FileInfo\n"
					   "FilePath: %s\n"
					   "FileStatus: NotFound",
					   encodedPath);

	      // use ROUTE_ONLY flag when sending back FileInfo
	      muteSendMessage (muteShareVirtualAddress,
			       inFromAddress, message, "ROUTE_ONLY");

	      delete[]message;
	      }
	    }

	  if (file != NULL)
	    {
	      delete file;
	    }

	}

    }

  for (int i = 0; i < numTokens; i++)
    {
      delete[] * (tokens->getElement (i));
    }

  delete tokens;

  // no utility generated
  return 0;
}



// reports that a chunk has been sent for a file.
// inFilePath and inHostAddress must be destroyed by caller
void
muteShare_internalChunkSent (char *inFilePath,
			     char *inHostAddress,
			     int inChunkNumber, int inChunksInFile)
{
  // get current time in seconds
  unsigned long currentSeconds = time (NULL);

  // Search for the file in our upload data; both file path and destination
  // address must match to be considered the same entry.  Multiple uploads
  // of the same file to different destinations are recorded separately.
  char found = false;
  int foundIndex = -1;

  muteUploadDataLock.lock ();

  int numUploads = muteUploadIDs->size ();
  for (int i = 0; i < numUploads && !found; i++)
    {

      if (strcmp (inFilePath,
		  *(muteUploadFilePaths->getElement (i))) == 0 &&
	  strcmp (inHostAddress,
		  *(muteUploadHostAddresses->getElement (i))) == 0)
	{

	  // both file path and host address match

	  found = true;
	  foundIndex = i;
	}
    }

  if (found)
    {
      // save last chunk sent index
      long *indexPointer = muteUploadLastChunksSent->getElement (foundIndex);
      *indexPointer = inChunkNumber;

      // save time of last chunk send
      unsigned long *timePointer
	= muteUploadLastChunkTimes->getElement (foundIndex);
      *timePointer = currentSeconds;
    }
  else
    {
      // add a new upload to end of vector
      muteUploadIDs->push_back (muteNextUploadID);
      muteNextUploadID++;

      muteUploadFilePaths->push_back (stringDuplicate (inFilePath));
      muteUploadHostAddresses->push_back (stringDuplicate (inHostAddress));
      muteUploadLastChunksSent->push_back (inChunkNumber);
      muteUploadChunksInFiles->push_back (inChunksInFile);
      muteUploadFirstChunkTimes->push_back (currentSeconds);
      muteUploadLastChunkTimes->push_back (currentSeconds);
    }

  muteUploadDataLock.unlock ();
}



// handler for FileChunkRequests
int
muteShare_internalFileChunkSender (char *inFromAddress, char *inToAddress,
				   char *inBody, void *inExtraArgument)
{
 int utility=0;
  // is the message a FileChunkRequest?
  // look at second token
  SimpleVector < char *>*tokens = tokenizeString (inBody);
  int numTokens = tokens->size ();

  if (numTokens >= 6)
    {
      char *typeToken = *(tokens->getElement (1));
      if (strcmp ("FileChunkRequest", typeToken) == 0)
	{

	  char *encodedPath = *(tokens->getElement (3));
	  char *path = URLUtils::hexDecode (encodedPath);
	  char *typeReq = *(tokens->getElement (2));
	  char *chunkNumberString = *(tokens->getElement (5));

	  int chunkNumber = strtol (chunkNumberString, (char **) NULL, 10);

	  QFile *file = muteShare_internalEncodedPathToFile (encodedPath);
	  if (file == NULL && (!strcmp ("FileHash", typeReq)
			       || !strncmp (path, "hash_", 5)))
	    {
	      // search file by Hash in shares
	      for (int i = 0; i < nbShares; i++)
		{
		  char *hashString = Shares[i].hash;
		  if (hashString == NULL)
		    {
		      hashString = "";
		    }
		  if (strncasecmp (hashString, path + 5, 40) == 0)
		    {
		      file = new QFile (Shares[i].name);
		      break;
		    }
		}
	      // search file in current downloads
	      if(file ==NULL)
	      {
	       for (int i = 0; i < nbDownloads; i++)
		{
		  char *hashString =
		    strdup (Downloads[i]->getHash ().toLocal8Bit ().constData ());
		  if (strncasecmp (hashString, path + 5, 40) == 0)
		    {
		      // have we this chunk ?
		      if (Downloads[i]->haveChunk (chunkNumber))
			{
			  QString filePath = Downloads[i]->getPartialFilePath ();
			  file =
			    new QFile (filePath);
			}
		      free (hashString);
		      break;
		    }
		  free (hashString);
		}
	      }
	    }

	  if (file != NULL)
	    {


		  qint64 fileLength = file->size ();

		  int chunksInFile = fileLength / muteShareChunkSize;

		  if (fileLength % muteShareChunkSize != 0)
		    {
		      // extra partial chunk
		      chunksInFile += 1;
		    }

		  if (chunkNumber >= 0 && chunkNumber < chunksInFile)
		    {

		      // send a FileChunk message

		      char *fileName = strdup(file->fileName ().toLocal8Bit().data());
		      FILE *fileHandle = fopen (fileName , "rb");

		      if (fileHandle != NULL)
			{
			  unsigned long bytesToSkip =
			    chunkNumber * muteShareChunkSize;

			  fseek (fileHandle, bytesToSkip, SEEK_SET);

			  int chunkSize = muteShareChunkSize;
			  if (bytesToSkip + muteShareChunkSize > fileLength)
			    {

			      // partial chunk
			      chunkSize = fileLength - bytesToSkip;
			    }

			  unsigned char *rawChunkData =
			    new unsigned char[chunkSize];

			  int numRead = fread (rawChunkData, 1, chunkSize,
					       fileHandle);
			  fclose (fileHandle);

			  if (numRead == chunkSize)
			    {

			      // base64 encode with no line breaks
			      char *encodedChunkData =
				base64Encode (rawChunkData, chunkSize,
					      false);

			      char *message =
				autoSprintf ("MessageType: FileChunk\n"
					     "FilePath: %s\n"
					     "ChunkNumber: %d\n"
					     "ChunkLength: %d\n"
					     "ChunkData: %s",
					     encodedPath,
					     chunkNumber,
					     chunkSize,
					     encodedChunkData);

			      delete[]encodedChunkData;

			      // use ROUTE_ONLY flag to avoid wasting
			      // bandwidth on lost chunks
			      muteSendMessage (muteShareVirtualAddress,
					       inFromAddress,
					       message, "ROUTE_ONLY");
			      delete[]message;

			      muteShare_internalChunkSent (fileName,
							   inFromAddress,
							   chunkNumber,
							   chunksInFile);
			      utility += 1000;
			      StatLock.lock ();
			      TotBytesUploaded += chunkSize;
			      StatLock.unlock ();
			    }

			  delete[]rawChunkData;

			}

		      free (fileName);
		    }
	      delete file;
	    }
	  else
	    {
	    }
	}
    }

  for (int i = 0; i < numTokens; i++)
    {
      delete[] * (tokens->getElement (i));
    }

  delete tokens;

  return utility;
}



// utility penalty for searches that contain no terms
// should cause them to be dropped quickly
int muteShareBlankSearchUtilityPenalty = 1000;



// handler for SearchRequests
int
muteShare_internalSearchResultsSender (char *inFromAddress,
				       char *inToAddress,
				       char *inBody, void *inExtraArgument)
{
  int utility = 0;

  // is the message a SearchRequest?
  // look at second token
  SimpleVector < char *>*tokens = tokenizeString (inBody);
  int numTokens = tokens->size ();

  if (numTokens >= 6)
    {
      char *typeToken = *(tokens->getElement (1));
      if (strcmp ("SearchRequest", typeToken) == 0)
	{

	  char *searchID = *(tokens->getElement (3));
	  char *encodedSearchString = *(tokens->getElement (5));

	  char *searchString = URLUtils::hexDecode (encodedSearchString);

	  char *logMessage =
	    autoSprintf ("Got search request for (%s), id=%s",
			 searchString, searchID);
	  AppLog::detail ("fileShare", logMessage);
	  delete[]logMessage;


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

	  char *sharedPath = muteShareGetSharingPath ();

	  File *sharedDirectory = new File (NULL, sharedPath);
	  delete[]sharedPath;


	  // penalize searches that contain no terms
	  if (numSearchTerms == 0)
	    {
	      utility += muteShareBlankSearchUtilityPenalty;
	    }

	  // ignore searches that contain no terms
	  // only search if we have a valid shared directory
	  if (numSearchTerms > 0 &&
	      sharedDirectory->exists () && sharedDirectory->isDirectory ())
	    {

	      SimpleVector < char *>*hits = new SimpleVector < char *>();

	      // for now, trim our results so that they fit
	      // in one message

	      // save 4 KiB for our headers, 28 KiB for results
	      int maxResultsLength = 28672;
	      int totalResultLength = 0;

	      char hitLimit = false;

	      int i;
	      for (i = 0; i < nbShares && !hitLimit; i++)
		{

		  char *fileName = Shares[i].name;

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

		      if (hitAll)
			{

			  char *hashString = Shares[i].hash;
			  if (hashString == NULL)
			    {
			      hashString = "";
			    }


			  // check hash if this is a hash-only search
			  char resultIsAHit = true;

			  if (hashOnlySearch)
			    {
			      char *hashTerm = *(searchTerms->getElement (0));
			      if (strcasecmp (hashString,
							   hashTerm) != 0)
				{

				  resultIsAHit = false;
				}
			    }


			  if (resultIsAHit)
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
								Shares[i].
								size,
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
	      if (hashOnlySearch)
		{
		  char *hashTerm = *(searchTerms->getElement (0));
		  // search file in current downloads
		  for (int i = 0; i < nbDownloads; i++)
		    {
		      char *hashString =
			strdup (Downloads[i]->getHash ().toLocal8Bit ().constData ());
		      if (strncasecmp (hashString, hashTerm, 40) == 0)
			{
			  char *resultName =
			    autoSprintf ("hash_%s", hashTerm);
			  char *encodedFileName =
			    URLUtils::hexEncode (resultName);
			  char *resultString = 
#ifdef __MINGW32__
				    autoSprintf ("%s %I64d %s",
#else
				    autoSprintf ("%s %lld %s",
#endif
							    encodedFileName,
							    Downloads[i]->
							    getSize (),
							    hashString);
			  int resultLength = strlen (resultString);
			  delete[]encodedFileName;
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
			  break;
			}
		      free (hashString);
		    }
		}


	      int numHits = hits->size ();

	      if (numHits != 0)
		{

		  logMessage =
		    autoSprintf
		    ("Sending response with %d results, id=%s", numHits,
		     searchID);
		  AppLog::detail ("fileShare", logMessage);
		  delete[]logMessage;


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
		  muteSendMessage (muteShareVirtualAddress,
				   inFromAddress, message, "ROUTE_ONLY");

		  // utility is number of results we return
		  utility += numHits;

		  delete[]message;
		}

	      delete hits;
	    }

	  delete sharedDirectory;

	  for (int j = 0; j < numSearchTerms; j++)
	    {
	      char *term = *(searchTerms->getElement (j));
	      delete[]term;
	    }
	  delete searchTerms;
	}
    }

  for (int i = 0; i < numTokens; i++)
    {
      delete[] * (tokens->getElement (i));
    }

  delete tokens;

  return utility;
}









/**
 * Checks if current address and message signing keypair are still safe
 * to use (checks if replay attacks have become possible).
 *
 * If the address is no longer safe, a new one is generated.
 *
 * If the existing address is NULL, this call can be used to generate our
 * first address upon startup.
 */
void
muteShare_internalMakeAddressSafe ()
{

  char generateNew = false;

  if (muteHasCounterWrappedAround ())
    {
      generateNew = true;
      if (muteShareVirtualAddress != NULL)
	{

	  muteRemoveReceiveAddress (muteShareVirtualAddress);

	  delete[]muteShareVirtualAddress;
	  muteShareVirtualAddress = NULL;
	}
    }
  else if (muteShareVirtualAddress == NULL)
    {
      generateNew = true;
    }


  if (!generateNew)
    {
      return;
    }


  // the messageRouter layer will sign all of our outbound messages
  // we need to generate our keys for this first (our address is the
  // SHA1 hash of our public key)


  int keyLength = settings->getKeySizeSetting();

  printf ("Generating new %d-bit RSA key pair for signing messages...\n",
	  keyLength);

  char *pubKey;
  char *privKey;

  CryptoUtils::generateRSAKey (keyLength, &pubKey, &privKey);

  printf ("...done\n");

  char *pubKeyHash = computeSHA1Digest (pubKey);

  // our address is PKH followed by the hash of our pubkey
  muteShareVirtualAddress = autoSprintf ("PKH%s", pubKeyHash);

  delete[]pubKeyHash;

  muteSetMessageIDSigningKeys (pubKey, privKey);
  delete[]pubKey;
  delete[]privKey;


  muteAddReceiveAddress (muteShareVirtualAddress);
}



/**
 * Thread that wakes periodically and makes sure our address is still safe.
 */
class AddressSafetyThread:public StopSignalThread
{

public:



	/**
         * Creates and starts a thread.
         */
  AddressSafetyThread ()
  {
    start ();
  }



	/**
         * Stops and destroys a thread.
         */
   ~AddressSafetyThread ()
  {
    stop ();
    //qDebug("prima del wait");
    wait();
  }



  // implements the Thread interface
  virtual void run ()
  {

    while (!isStopped ())
      {
	// wake every 10 minutes
	for (int i = 0; i < 120; i++)
	  {
	    sleep (5000);
	    if (isStopped ())
	      break;
	  }

	if (!isStopped ())
	  {

	    AppLog::info ("AddressSafetyThread",
			  "Checking to make sure address is still safe.");

	    muteShare_internalMakeAddressSafe ();
	  }
      }
  }

};



AddressSafetyThread *muteShareAddressSafetyThread = NULL;



void
muteShareStart ()
{

  muteMaxSimultaneousDownloads = muteShareGetMaxSimultaneousDownloads ();

  muteSharePrintSearchSyncTrace = settings->getPrintSearchSyncTraceSetting();


  muteUploadIDs = new SimpleVector < long >();
  muteUploadFilePaths = new SimpleVector < char *>();
  muteUploadHostAddresses = new SimpleVector < char *>();
  muteUploadLastChunksSent = new SimpleVector < long >();
  muteUploadChunksInFiles = new SimpleVector < long >();
  muteUploadFirstChunkTimes = new SimpleVector < unsigned long >();
  muteUploadLastChunkTimes = new SimpleVector < unsigned long >();

  muteDownloadIDs = new SimpleVector < long >();
  muteDownloadSemaphores = new SimpleVector < QSemaphore * >();
  muteDownloadInterruptedFlags = new SimpleVector < char >();
  muteDownloadSourceQueues = new SimpleVector < SourceQueue * >();

  muteDownloadQueue = new SimpleVector < long >();

  muteShareMimeTyper = new MimeTyper (settings->getMimeTypesSetting());

  muteShareFileInfoSenderID =
    muteAddMessageHandler (muteShare_internalFileInfoSender, (void *) NULL);
  muteShareFileChunkSenderID =
    muteAddMessageHandler (muteShare_internalFileChunkSender, (void *) NULL);
  muteShareSearchResultsSenderID =
    muteAddMessageHandler (muteShare_internalSearchResultsSender,
			   (void *) NULL);


  // receive broadcasts
  muteAddReceiveAddress ("ALL");


  // generate our first message-signing key pair and virtual address
  muteShare_internalMakeAddressSafe ();

  // start thread to keep address safe
  muteShareAddressSafetyThread = new AddressSafetyThread ();
}



void
muteShareStop ()
{

  if (muteShareAddressSafetyThread != NULL)
    {
      delete muteShareAddressSafetyThread;
      muteShareAddressSafetyThread = NULL;
    }

  if (muteShareVirtualAddress != NULL)
    {
      muteRemoveReceiveAddress (muteShareVirtualAddress);
      delete[]muteShareVirtualAddress;
      muteShareVirtualAddress = NULL;
    }
  muteRemoveReceiveAddress ("ALL");

  if (muteShareSearchResultsSenderID != -1)
    {
      muteRemoveMessageHandler (muteShareSearchResultsSenderID);
      muteShareSearchResultsSenderID = -1;
    }
  if (muteShareFileChunkSenderID != -1)
    {
      muteRemoveMessageHandler (muteShareFileChunkSenderID);
      muteShareFileChunkSenderID = -1;
    }
  if (muteShareFileInfoSenderID != -1)
    {
      muteRemoveMessageHandler (muteShareFileInfoSenderID);
      muteShareFileInfoSenderID = -1;
    }

  if (muteShareMimeTyper != NULL)
    {
      delete muteShareMimeTyper;
      muteShareMimeTyper = NULL;
    }

  if (muteUploadIDs != NULL)
    {
      delete muteUploadIDs;
      muteUploadIDs = NULL;
    }
  if (muteUploadFilePaths != NULL)
    {
      int numPaths = muteUploadFilePaths->size ();
      for (int i = 0; i < numPaths; i++)
	{
	  delete[] * (muteUploadFilePaths->getElement (i));
	}
      delete muteUploadFilePaths;
      muteUploadFilePaths = NULL;
    }
  if (muteUploadHostAddresses != NULL)
    {
      int numPaths = muteUploadHostAddresses->size ();
      for (int i = 0; i < numPaths; i++)
	{
	  delete[] * (muteUploadHostAddresses->getElement (i));
	}
      delete muteUploadHostAddresses;
      muteUploadHostAddresses = NULL;
    }
  if (muteUploadLastChunksSent != NULL)
    {
      delete muteUploadLastChunksSent;
      muteUploadLastChunksSent = NULL;
    }
  if (muteUploadChunksInFiles != NULL)
    {
      delete muteUploadChunksInFiles;
      muteUploadChunksInFiles = NULL;
    }
  if (muteUploadFirstChunkTimes != NULL)
    {
      delete muteUploadFirstChunkTimes;
      muteUploadFirstChunkTimes = NULL;
    }
  if (muteUploadLastChunkTimes != NULL)
    {
      delete muteUploadLastChunkTimes;
      muteUploadLastChunkTimes = NULL;
    }
  if (muteDownloadIDs != NULL)
    {
      delete muteDownloadIDs;
      muteDownloadIDs = NULL;
    }
  if (muteDownloadSemaphores != NULL)
    {
      int numSems = muteDownloadSemaphores->size ();
      for (int i = 0; i < numSems; i++)
	{
	  delete *(muteDownloadSemaphores->getElement (i));
	}
      delete muteDownloadSemaphores;
      muteDownloadSemaphores = NULL;
    }
  if (muteDownloadInterruptedFlags != NULL)
    {
      delete muteDownloadInterruptedFlags;
      muteDownloadInterruptedFlags = NULL;
    }
  if (muteDownloadSourceQueues != NULL)
    {
      int numQueues = muteDownloadSourceQueues->size ();
      for (int i = 0; i < numQueues; i++)
	{
	  delete *(muteDownloadSourceQueues->getElement (i));
	}
      delete muteDownloadSourceQueues;
      muteDownloadSourceQueues = NULL;
    }

  if (muteDownloadQueue != NULL)
    {
      delete muteDownloadQueue;
      muteDownloadQueue = NULL;
    }
}



char *
muteShareGetLocalVirtualAddress ()
{
  return stringDuplicate (muteShareVirtualAddress);
}



void
muteShareSetSharingPath (char *inPath)
{
  // encode to protect spaces and special characters
  char *safePath = URLUtils::hexEncode (inPath);
  delete[]safePath;
}



char *
muteShareGetSharingPath ()
{
    QString shareFolder = settings->getOtherSharedPathSetting (). isEmpty () ? ""
        : settings->getOtherSharedPathSetting ().at (0);

    char *safeSetPath = strdup (shareFolder.toLocal8Bit ().data ());

    if (safeSetPath != NULL)
    {
        char *decodedPath = URLUtils::hexDecode (safeSetPath);
        free (safeSetPath);

        return decodedPath;
    }
    else
    {
        return stringDuplicate ("");
    }
}



unsigned long
muteShareGetMaxSimultaneousDownloads ()
{
  unsigned long limit = settings->getMaxSimultaneousDownloadsSetting();
  return limit;
}



class ShareFileInfoWrapper
{

public:

  ShareFileInfoWrapper ():mInfoReceived (false)
  {

  }


   ~ShareFileInfoWrapper ()
  {
  }


  QMutex mLock;

  char *mVirtualAddress;
  char *mFilePath;

  // true if chunk data has already been received and set here
  // for this file
  // used by FileInfoHandler to avoid overwritting data twice
  char mInfoReceived;

  char mFound;
  int mLength;
  int mChunkCount;
  char *mMimeType;

  QSemaphore *mSemaphore;
};



// handler for FileInfo messages, used by muteGetFileInfo
int
muteShare_internalFileInfoHandler (char *inFromAddress,
				   char *inToAddress,
				   char *inBody, void *inExtraArgument)
{

  // is this a FileInfo message?
  // look at second token
  SimpleVector < char *>*tokens = tokenizeString (inBody);
  int numTokens = tokens->size ();

  if (numTokens >= 6)
    {
      char *typeToken = *(tokens->getElement (1));
      if (strcmp ("FileInfo", typeToken) == 0)
	{
	  // unwrap info object from extra arg
	  ShareFileInfoWrapper *fileInfo =
	    (ShareFileInfoWrapper *) inExtraArgument;


	  fileInfo->mLock.lock ();
	  char infoLocked = true;


	  if (fileInfo->mInfoReceived)
	    {
	      // we already received info for this file
	      // ignore this new message

	      fileInfo->mLock.unlock ();
	      infoLocked = false;

	      // no utility in any case
	      return 0;
	    }


	  if (strcmp (inFromAddress, fileInfo->mVirtualAddress) != 0)
	    {
	      // not the address we're looking for

	      fileInfo->mLock.unlock ();
	      infoLocked = false;

	      // no utility in any case
	      return 0;
	    }



	  char *encodedPath = *(tokens->getElement (3));
	  char *filePath = URLUtils::hexDecode (encodedPath);

	  // is this info about our file?
	  if (strcmp (filePath, fileInfo->mFilePath) == 0)
	    {

	      char *statusString = *(tokens->getElement (5));

	      // was file found?
	      if (strcmp (statusString, "Found") == 0)
		{

		  if (numTokens >= 12)
		    {
		      fileInfo->mFound = true;

		      // flag so we won't overwrite it a second time
		      fileInfo->mInfoReceived = true;

		      char *lengthString = *(tokens->getElement (7));
		      // default to 0
		      int length = 0;
		      sscanf (lengthString, "%d", &length);

		      fileInfo->mLength = length;

		      char *chunkCountString = *(tokens->getElement (9));
		      // default to 0
		      int chunkCount = 0;
		      sscanf (chunkCountString, "%d", &chunkCount);

		      fileInfo->mChunkCount = chunkCount;

		      fileInfo->mMimeType =
			stringDuplicate (*(tokens->getElement (11)));

		      fileInfo->mLock.unlock ();
		      infoLocked = false;

		      fileInfo->mSemaphore->release ();
		    }
		  // else info formatted incorrectly, ignore

		}
	      else
		{
		  // not found, but we got the info
		  fileInfo->mFound = false;

		  fileInfo->mLock.unlock ();
		  infoLocked = false;

		  fileInfo->mSemaphore->release ();
		}
	    }
	  delete[]filePath;
	  if (infoLocked)
	    {
	      fileInfo->mLock.unlock ();
	      infoLocked = false;
	    }
	}
    }

  for (int i = 0; i < numTokens; i++)
    {
      delete[] * (tokens->getElement (i));
    }

  delete tokens;



  // no utility generated
  return 0;
}



/**
 * Gets whether a download is interrupted and clears the interrupted flag.
 *
 * @param inDownloadID the ID of the download to check.
 */
char
muteShare_internalIsDownloadInterrupted (int inDownloadID)
{
  char found = false;
  char returnValue = false;
  if (!muteDownloadIDs)
    return false;
  muteDownloadDataLock.lock ();
  for (int i = 0; i < muteDownloadIDs->size () && !found; i++)
    {
      if (inDownloadID == *(muteDownloadIDs->getElement (i)))
	{

	  char *flag = muteDownloadInterruptedFlags->getElement (i);
	  returnValue = *flag;

	  // reset flag
	  *flag = false;
	}
    }

  muteDownloadDataLock.unlock ();

  return returnValue;
}



/**
 * Gets the semaphore associated with a download.
 *
 * @param inDownloadID the ID of the download.
 *
 * @return the semaphore, or NULL if the download is not found.
 *   Should not be destroyed by caller.
 */
QSemaphore *
muteShare_internalGetDownloadSemaphore (int inDownloadID)
{

  muteDownloadDataLock.lock ();
  char found = false;
  QSemaphore *returnValue = NULL;
  for (int i = 0; i < muteDownloadIDs->size () && !found; i++)
    {
      if (inDownloadID == *(muteDownloadIDs->getElement (i)))
	{

	  returnValue = *(muteDownloadSemaphores->getElement (i));
	  found = true;
	}
    }

  muteDownloadDataLock.unlock ();

  return returnValue;
}



/**
 * Gets the source queue associated with a download.
 *
 * Since this call returns heap-allocated data, it is NOT thread-safe.
 * muteDownloadDataLock must be locked by caller to ensure safety.
 *
 * @param inDownloadID the ID of the download.
 *
 * @return the source queue, or NULL if the download is not found.
 *   Should not be destroyed by caller.
 */
SourceQueue *
muteShare_internalGetDownloadSourceQueue (int inDownloadID)
{
  char found = false;
  SourceQueue *returnValue = NULL;
  for (int i = 0; i < muteDownloadIDs->size () && !found; i++)
    {
      if (inDownloadID == *(muteDownloadIDs->getElement (i)))
	{

	  returnValue = *(muteDownloadSourceQueues->getElement (i));
	  found = true;
	}
    }

  return returnValue;
}

int MUTE_SHARE_FILE_INFO_TIMEOUT = 0;
int MUTE_SHARE_FILE_INFO_FOUND = 1;
int MUTE_SHARE_FILE_INFO_NOT_FOUND = 2;
int MUTE_SHARE_FILE_INFO_CANCELED = 3;

class ShareSearchWrapper
{

public:
  char *mSearchID;

  QMutex mLock;
  char mCanceled;
  char *mFromAddress;
  int mNumResults;
  char **mFilePaths;
  unsigned long *mLengthsInBytes;
  char **mHashes;

  QSemaphore *mResultsReadySemaphore;

  QSemaphore *mResultsConsumedSemaphore;
};



// handler for SearchResults messages, used by muteShareSearch
int
muteShare_internalSearchResultsHandler (char *inFromAddress,
					char *inToAddress,
					char *inBody, void *inExtraArgument)
{
  // unwrap info object from extra arg
  ShareSearchWrapper *search = (ShareSearchWrapper *) inExtraArgument;

  // is this a SearchResults message?
  // look at second token
  SimpleVector < char *>*tokens = tokenizeString (inBody);
  int numTokens = tokens->size ();

  if (numTokens >= 6)
    {
      char *typeToken = *(tokens->getElement (1));
      if (strcmp ("SearchResults", typeToken) == 0)
	{


	  char *searchID = *(tokens->getElement (3));

	  // are these results for our search?
	  if (strcmp (searchID, search->mSearchID) == 0)
	    {

	      char *resultCountString = *(tokens->getElement (5));
	      // default to 0
	      int resultCount = 0;
	      sscanf (resultCountString, "%d", &resultCount);

	      int firstResultToken = 7;
	      // 3 tokens per result
	      int lastResultToken = firstResultToken + 3 * resultCount;

	      if (numTokens >= lastResultToken)
		{
		  if (muteSharePrintSearchSyncTrace)
		    {
		      printf ("  Result handler locking search mutex\n");
		    }
		  search->mLock.lock ();

		  if (!search->mCanceled)
		    {

		      search->mFromAddress = stringDuplicate (inFromAddress);
		      search->mNumResults = resultCount;
		      search->mFilePaths = new char *[resultCount];
		      search->mLengthsInBytes =
			new unsigned long[resultCount];
		      search->mHashes = new char *[resultCount];

		      int resultIndex = 0;
		      for (int i = firstResultToken;
			   i < lastResultToken - 2; i += 3)
			{

			  char *encodedFileName = *(tokens->getElement (i));
			  char *lengthString = *(tokens->getElement (i + 1));
			  char *hashString = *(tokens->getElement (i + 2));

			  search->mFilePaths[resultIndex] =
			    URLUtils::hexDecode (encodedFileName);

			  // default to 0 if scan fails
			  unsigned long length = 0;
			  sscanf (lengthString, "%lu", &length);
			  search->mLengthsInBytes[resultIndex] = length;

			  search->mHashes[resultIndex] =
			    stringDuplicate (hashString);


			  resultIndex++;
			}

		      if (muteSharePrintSearchSyncTrace)
			{
			  printf
			    ("  Result handler unlocking search mutex\n");
			}
		      search->mLock.unlock ();

		      // signal that results are ready
		      if (muteSharePrintSearchSyncTrace)
			{
			  printf ("  Result handler signaling that "
				  "results are ready\n");
			}
		      search->mResultsReadySemaphore->release ();

		      // wait for results to be consumed
		      if (muteSharePrintSearchSyncTrace)
			{
			  printf ("  Result handler waiting for "
				  "results to be consumed\n");
			}
		      search->mResultsConsumedSemaphore->tryAcquire ();
		      if (muteSharePrintSearchSyncTrace)
			{
			  printf ("  Result handler done waiting for "
				  "results to be consumed\n");
			}

		    }
		  else
		    {
		      // canceled, don't return results
		      search->mLock.unlock ();
		    }
		}
	    }
	}
    }

  for (int i = 0; i < numTokens; i++)
    {
      delete[] * (tokens->getElement (i));
    }

  delete tokens;

  // no utility generated
  return 0;
}






void
muteShareSearch (char *inSearchString,
		 char (*inResultHandler) (char *, char *, unsigned long,
					  char *, void *),
		 void *inExtraHandlerArgument, int inTimeoutInMilliseconds,
		 char *inAddressToSearch)
{

  char *logMessage = autoSprintf ("Searching for %s", inSearchString);
  AppLog::info ("fileShare", logMessage);
  delete[]logMessage;


  char *searchID = muteGetUniqueName ();

  // register a handler for the chunk
  ShareSearchWrapper *search = new ShareSearchWrapper ();
  search->mSearchID = searchID;
  search->mCanceled = false;
  search->mFromAddress = NULL;
  search->mNumResults = 0;
  search->mFilePaths = NULL;
  search->mLengthsInBytes = NULL;
  search->mHashes = NULL;

  search->mResultsReadySemaphore = new QSemaphore ();
  search->mResultsConsumedSemaphore = new QSemaphore ();

  int handlerID =
    muteAddMessageHandler (muteShare_internalSearchResultsHandler,
			   (void *) search);


  // send out our search
  char *encodedSearchString = URLUtils::hexEncode (inSearchString);

  char *message = autoSprintf ("MessageType:\tSearchRequest\n"
			       "SearchID: %s\n" "SearchString: %s",
			       searchID,
			       encodedSearchString);

  char *forwardFlag;

  if (strcmp (inAddressToSearch, "ALL") == 0)
  {
      // sending to ALL
      // use the FORWARD flag to preserve our anonymity
      char *hashSeed = muteGetForwardHashSeed ();
      forwardFlag = autoSprintf ("FORWARD_%s", hashSeed);
      delete[]hashSeed;
  }
  else
  {
      // sending to one, no FORWARD flag needed
      forwardFlag = stringDuplicate ("");
  }

  muteSendMessage (muteShareVirtualAddress,
		   inAddressToSearch, message, forwardFlag);
  delete[]forwardFlag;
  delete[]message;

  delete[]encodedSearchString;



  char canceled = false;

  int searchRetry  = 0;

  if ( SEARCH_TIME_MS > inTimeoutInMilliseconds ) searchRetry = SEARCH_TIME_MS / inTimeoutInMilliseconds;
  else searchRetry = 1;

  while (!canceled)
  {

      // wait for results
	  if (muteSharePrintSearchSyncTrace)
	  {
		  printf ("Search waiting for " "results to be ready\n");
	  }
      int resultsReceived = search->mResultsReadySemaphore->tryAcquire (1,
									inTimeoutInMilliseconds);

      if (muteSharePrintSearchSyncTrace)
      {
    	  printf ("Search done waiting for " "results to be ready\n");
      }


      if (resultsReceived == 1)
      {
	  // pass the results to the caller's handler
        qDebug () << "Got rezult 1";
    	  if (muteSharePrintSearchSyncTrace)
    	  {
    		  printf ("Search locking search mutex\n");
    	  }
    	  search->mLock.lock ();

    	  int i;
    	  for (i = 0; i < search->mNumResults && !canceled; i++)
    	  {
    		  char keepGoing = inResultHandler (search->mFromAddress,
						search->mFilePaths[i],
						search->mLengthsInBytes[i],
						search->mHashes[i],
						inExtraHandlerArgument);

    		  if (!keepGoing)
    		  {
    			  canceled = true;
    		  }
                  canceled = false;
    	  }

    	  // delete results
    	  delete[]search->mFromAddress;
    	  search->mFromAddress = NULL;
    	  for (i = 0; i < search->mNumResults; i++)
    	  {
    		  delete[]search->mFilePaths[i];
    		  delete[]search->mHashes[i];
    	  }
    	  delete[]search->mFilePaths;
    	  delete[]search->mLengthsInBytes;
    	  delete[]search->mHashes;

    	  search->mFilePaths = NULL;
    	  search->mLengthsInBytes = NULL;
    	  search->mHashes = NULL;

    	  search->mNumResults = 0;

    	  search->mCanceled = canceled;

    	  if (muteSharePrintSearchSyncTrace)
    	  {
    		  printf ("Search unlocking search mutex\n");
    	  }
    	  search->mLock.unlock ();

    	  if (!canceled)
    	  {
	      // signal that results have been consumed
    		  if (muteSharePrintSearchSyncTrace)
    		  {
    			  printf("Search signaling that results have been consumed\n");
    		  }
    		  search->mResultsConsumedSemaphore->release ();
    	  }
    	  // else keep handler blocked until we can remove it below

      }
      else
      {
	  // timed out waiting
	  // tell handler that we timed out
    	  char keepGoing = inResultHandler (NULL,
					    NULL,
					    0,
					    NULL,
					    inExtraHandlerArgument);
    	  if ( (--searchRetry) <= 0 ) keepGoing = false;

    	  if (!keepGoing)
    	  {
    		  canceled = true;

    		  search->mLock.lock ();
    		  search->mCanceled = true;
    		  search->mLock.unlock ();
    	  }

      }
    }


  // handler has canceled us

  // remove the result handler
  muteRemoveMessageHandler (handlerID);


  // delete any extra results
  if (muteSharePrintSearchSyncTrace)
    {
      printf ("Search locking search mutex\n");
    }
  search->mLock.lock ();
  if (search->mFromAddress != NULL)
    {
      delete[]search->mFromAddress;
      search->mFromAddress = NULL;

      for (int i = 0; i < search->mNumResults; i++)
	{
	  delete[]search->mFilePaths[i];
	  delete[]search->mHashes[i];
	}
      delete[]search->mFromAddress;
      delete[]search->mLengthsInBytes;
      delete[]search->mHashes;

      search->mFromAddress = NULL;
      search->mLengthsInBytes = NULL;
      search->mHashes = NULL;
    }
  search->mCanceled = true;
  if (muteSharePrintSearchSyncTrace)
    {
      printf ("Search unlocking search mutex\n");
    }
  search->mLock.unlock ();

  if (muteSharePrintSearchSyncTrace)
    {
      printf ("Search signaling that results have been consumed\n");
    }
  search->mResultsConsumedSemaphore->release ();
  search->mResultsReadySemaphore->release ();


  delete search->mResultsReadySemaphore;
  search->mResultsReadySemaphore = NULL;
  delete search->mResultsConsumedSemaphore;
  search->mResultsConsumedSemaphore = NULL;

  delete[]search->mSearchID;

  delete search;
}

/**
 * Removes info from our list about a download.
 *
 * Does not delete the semaphore associated with the download (simply
 * removes it from the list.
 * Does not delete the source queue.
 *
 * @param inDownloadID the ID to remove.
 */
void
muteShare_internalRemoveDownloadInfo (int inDownloadID)
{
  // remove download info from our list
  muteDownloadDataLock.lock ();
  char found = false;
  for (int i = 0; i < muteDownloadIDs->size () && !found; i++)
    {
      if (inDownloadID == *(muteDownloadIDs->getElement (i)))
	{

	  muteDownloadIDs->deleteElement (i);
	  muteDownloadSemaphores->deleteElement (i);
	  muteDownloadInterruptedFlags->deleteElement (i);
	  muteDownloadSourceQueues->deleteElement (i);
	  found = true;
	}
    }

  muteDownloadDataLock.unlock ();
}





SourceQueue::SourceQueue (char *inFileHash,
			  char *inFirstVirtualAddress, char *inFirstFilePath):
mHibernating (false),
mFileHash (stringDuplicate (inFileHash)),
mSourceVector (new SimpleVector < FileSource * >()),
mLongestSourceList (1)
{

  FileSource *firstSource =
    new FileSource (stringDuplicate (inFirstVirtualAddress),
		    stringDuplicate (inFirstFilePath));
  mSourceVector->push_back (firstSource);

}



SourceQueue::~SourceQueue ()
{

  delete[]mFileHash;


  int numSources = mSourceVector->size ();

  for (int i = 0; i < numSources; i++)
    {
      delete *(mSourceVector->getElement (i));
    }
  delete mSourceVector;

}




void
SourceQueue::reportBestSourceChunkTime (long inMilliseconds)
{
  mLock.lock ();

  FileSource *bestSource = *(mSourceVector->getElement (0));

  // set new time
  bestSource->mChunkTimeInMilliseconds = inMilliseconds;

  mLock.unlock ();


  int activeCount = getActiveSourceCount ();

  // update the current list
  // always do this, whether we started a new search or not,
  // because the search will take a while to update our source list
  findNewBest ();
}



void
SourceQueue::getNextSource (char **inoutVirtualAddress, char **outFilePath)
{
  mLock.lock ();


  int index = 0;
  if (*inoutVirtualAddress)
    {
      int numSources = mSourceVector->size ();
      for (index = 0; index < numSources; index++)
	{
	  FileSource *otherSource = *(mSourceVector->getElement (index));
	  if (!*inoutVirtualAddress
	      || !strcmp (*inoutVirtualAddress, otherSource->mVirtualAddress))
	    break;
	}
      index++;
      for ( ; index < numSources; index++)
	{
	  if( (*mSourceVector->getElement(index))->mChunkTimeInMilliseconds >=0) break;
	}
      if (index >= numSources)
	index = 0;
    }
  FileSource *bestSource = *(mSourceVector->getElement (index));
  //FIXME : where to free this two allocations ?
  *inoutVirtualAddress = stringDuplicate (bestSource->mVirtualAddress);
  *outFilePath = stringDuplicate (bestSource->mFilePath);

  mLock.unlock ();
}



int
SourceQueue::getActiveSourceCount ()
{
  mLock.lock ();

  // count how many we have that are not marked with failure

  int numSources = mSourceVector->size ();

  int notFailedCount = 0;
  int index = 0;
  for (index = 0; index < numSources; index++)
    {
      FileSource *otherSource = *(mSourceVector->getElement (index));

      if (otherSource->mChunkTimeInMilliseconds != -1)
	{
	  notFailedCount++;
	}
    }

  mLock.unlock ();

  return notFailedCount;
}

void
SourceQueue::addSource (FileSource * inSource)
{
  mLock.lock ();


  int numExistingSources = mSourceVector->size ();

  char found = false;
  int foundIndex = -1;



  int index = 0;
  for (index = 0; index < numExistingSources && !found; index++)
    {
      FileSource *otherSource = *(mSourceVector->getElement (index));

      if (strcmp (inSource->mVirtualAddress,
		  otherSource->mVirtualAddress) == 0)
	{
	  found = true;
	  foundIndex = index;
	}
    }

  if (!found)
    {
      // add to vector

      FileSource *copiedSource =
	new FileSource (stringDuplicate (inSource->mVirtualAddress),
			stringDuplicate (inSource->mFilePath));

      mSourceVector->push_back (copiedSource);
    }
  else
    {
      // got result from a source already in our queue
      FileSource *existingSource = *(mSourceVector->getElement (foundIndex));

      if (existingSource->mChunkTimeInMilliseconds == -1)
	{
	  // last report from this source was failure

	  // try this source again, though, since we got results back
	  // again
	  existingSource->mChunkTimeInMilliseconds = 100000;
	}
    }

  // add to count of sources found right now
  mLongestSourceList++;

  mLock.unlock ();

  findNewBest ();
}



void
SourceQueue::findNewBest ()
{
  mLock.lock ();

  FileSource *bestSource = *(mSourceVector->getElement (0));

  long oldBestTime = bestSource->mChunkTimeInMilliseconds;

  // check to see if another source is faster
  // and check if we should desactivate a source

  int numSources = mSourceVector->size ();

  int fastestIndex = -1;
  long fastestTime = oldBestTime;

  int toDeactivate = -1;

  for (int i = 0; i < numSources; i++)
    {
      FileSource *otherSource = *(mSourceVector->getElement (i));

      long otherTime = otherSource->mChunkTimeInMilliseconds;
      if ( otherTime < 0 ) continue;

      if (fastestTime != -1)
	{
	  if (otherTime != -1 && otherTime < fastestTime)
	    {

	      fastestIndex = i;
	      fastestTime = otherTime;
	    }
	}
      else
	{
	  if (otherTime != -1)
	    {
	      // any non-failure time is better than failure
	      fastestIndex = i;
	      fastestTime = otherTime;
	    }
	}
      if( otherSource->mLastReceipt.elapsed()>60000)
      {
	 if(toDeactivate == -1)
	 {
	   toDeactivate = i;
	 }
	 else
	 {
	   if(otherSource->mLastReceipt.elapsed()> (*mSourceVector->getElement(toDeactivate))->mLastReceipt.elapsed())
	     toDeactivate = i;
	 }
      }
    }

  if(toDeactivate >=0)
	   (*mSourceVector->getElement(toDeactivate))->mChunkTimeInMilliseconds = -1;

  if (fastestIndex != -1)
    {
      // found faster

      // swap
      FileSource *otherSource = *(mSourceVector->getElement (fastestIndex));

      *(mSourceVector->getElement (0)) = otherSource;

      *(mSourceVector->getElement (fastestIndex)) = bestSource;
    }
  else
    {
      // none found that were faster

      if (fastestTime == -1)
	{
	  // all must have been -1 (failed last chunk)

	  int numSources = mSourceVector->size ();
	  if (numSources > 1)
	    {
	      // pick one at random to swap with
	      RandomSource *randSource = new StdRandomSource ();

	      int index = randSource->getRandomBoundedInt (1,
							   numSources - 1);

	      delete randSource;


	      FileSource *otherSource = *(mSourceVector->getElement (index));

	      *(mSourceVector->getElement (0)) = otherSource;

	      *(mSourceVector->getElement (index)) = bestSource;
	    }
	}
    }

  mLock.unlock ();
}




void
muteShareInterruptDownload (int inDownloadID)
{
  muteDownloadDataLock.lock ();
  char found = false;
  for (int i = 0; i < muteDownloadIDs->size () && !found; i++)
    {
      if (inDownloadID == *(muteDownloadIDs->getElement (i)))
	{

	  // set interrupted flag
	  char *flag = muteDownloadInterruptedFlags->getElement (i);
	  *flag = true;

	  QSemaphore *sem = *(muteDownloadSemaphores->getElement (i));
	  sem->release ();
	  found = true;
	}
    }

  muteDownloadDataLock.unlock ();
}



int
muteShareGetSourceCount (int inDownloadID)
{
  muteDownloadDataLock.lock ();

  SourceQueue *sources =
    muteShare_internalGetDownloadSourceQueue (inDownloadID);

  int count;

  if (sources == NULL)
    {
      count = 0;
    }
  else
    {
      count = sources->getActiveSourceCount ();
    }

  muteDownloadDataLock.unlock ();

  return count;
}



char *
muteShareGetBestSource (int inDownloadID)
{

  muteDownloadDataLock.lock ();

  SourceQueue *sources =
    muteShare_internalGetDownloadSourceQueue (inDownloadID);

  char *address = NULL;

  if (sources == NULL)
    {
      address = stringDuplicate ("");
    }
  else
    {
      char *path = NULL;

      sources->getNextSource (&address, &path);

      delete[]path;
    }

  muteDownloadDataLock.unlock ();

  return address;
}



char
muteShareIsDownloadHibernating (int inDownloadID)
{
  muteDownloadDataLock.lock ();

  SourceQueue *sources =
    muteShare_internalGetDownloadSourceQueue (inDownloadID);

  char hibernating = false;

  if (sources != NULL)
    {
      hibernating = sources->isHibernating ();
    }

  muteDownloadDataLock.unlock ();

  return hibernating;
}



int
muteShareGetUploadStatus (int **outUploadIDs,
			  char ***outHostAddresses,
			  char ***outFilePaths,
			  int **outChunksInFile,
			  int **outLastChunks,
			  unsigned long **outFirstChunkTimes,
			  unsigned long **outLastChunkTimes)
{
  int *returnIDs;
  char **returnHostAddresses;
  char **returnPaths;
  int *returnLastChunks;
  int *returnChunksInFile;
  unsigned long *returnFirstChunkTimes;
  unsigned long *returnLastChunkTimes;

  muteUploadDataLock.lock ();

  int numUploads = muteUploadIDs->size ();

  returnIDs = new int[numUploads];
  returnHostAddresses = new char *[numUploads];
  returnPaths = new char *[numUploads];
  returnLastChunks = new int[numUploads];
  returnChunksInFile = new int[numUploads];
  returnFirstChunkTimes = new unsigned long[numUploads];
  returnLastChunkTimes = new unsigned long[numUploads];

  for (int i = 0; i < numUploads; i++)
    {
      returnIDs[i] = *(muteUploadIDs->getElement (i));
      returnHostAddresses[i] =
	stringDuplicate (*(muteUploadHostAddresses->getElement (i)));
      returnPaths[i] =
	stringDuplicate (*(muteUploadFilePaths->getElement (i)));

      returnLastChunks[i] = *(muteUploadLastChunksSent->getElement (i));
      returnChunksInFile[i] = *(muteUploadChunksInFiles->getElement (i));
      returnFirstChunkTimes[i] = *(muteUploadFirstChunkTimes->getElement (i));
      returnLastChunkTimes[i] = *(muteUploadLastChunkTimes->getElement (i));
    }

  muteUploadDataLock.unlock ();


  *outUploadIDs = returnIDs;
  *outHostAddresses = returnHostAddresses;
  *outFilePaths = returnPaths;
  *outLastChunks = returnLastChunks;
  *outChunksInFile = returnChunksInFile;
  *outFirstChunkTimes = returnFirstChunkTimes;
  *outLastChunkTimes = returnLastChunkTimes;

  return numUploads;
}
