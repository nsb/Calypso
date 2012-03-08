/*
 * Modification History
 *
 * 2003-October-13   Jason Rohrer
 * Created.
 *
 * 2003-October-14   Jason Rohrer
 * Finished implementation and got it to compile.
 *
 * 2003-October-16   Jason Rohrer
 * Fixed a deletion bug.
 * Added support for a file size return pointer in muteShareGetFile.
 *
 * 2003-November-4   Jason Rohrer
 * Added a function for getting upload info.
 *
 * 2003-November-6   Jason Rohrer
 * Added hashes and file sizes to search results.
 *
 * 2003-November-11   Jason Rohrer
 * Added b flag to fopen of binary files.
 *
 * 2003-December-23   Jason Rohrer
 * Added hash checking for incoming files.
 *
 * 2004-January-12   Jason Rohrer
 * Increased default timeout from 10 seconds to 30 seconds.
 *
 * 2004-February-12   Jason Rohrer
 * Added upload stats patch submitted by Mycroftxxx.
 *
 * 2004-February-13   Jason Rohrer
 * Cleaned up the patched upload-tracking code.
 *
 * 2004-February-21   Jason Rohrer
 * Changed GetFile callback spec to support fine-grained retry reporting.
 *
 * 2004-March-8   Jason Rohrer
 * Added support for passing fileInfo timeouts to file chunk handler.
 *
 * 2004-December-8   Jason Rohrer
 * Added some support for resumable downloads.
 *
 * 2006-January-23   Jason Rohrer
 * Worked on download queuing.
 *
 * 2006-February-7   Jason Rohrer
 * Changed so that -1 specifies no max active download limit.
 *
 * 2006-February-14   Jason Rohrer
 * Added functions for querying multi-source state.
 *
 * 2006-March-3   Jason Rohrer
 * Added function for checking hibernation state.
 *
 * 2006-March-26   Jason Rohrer
 * Added support for searching a specific host.
 * Added function for getting local virtual address.
 */



/**
 * The API for the file sharing layer of MUTE.
 *
 * All calls are thread-safe.
 *
 * All function parameters must be destroyed by caller.
 * All string parameters must be \0-terminated.
 *
 * Before calling this API, application should call
 * muteSeedRandomGenerator and muteStart
 * from the messageRouting layer
 *
 * After using this API, application should call
 * muteStop
 * from the messageRouting layer.
 * Application may also optionally call
 * muteGetRandomGeneratorState
 * from the messageRouting layer to save the state of the random generator.
 *
 * @author Jason Rohrer.
 */



#ifndef MUTE_FILE_SHARING_API
#define MUTE_FILE_SHARING_API



#include "minorGems/util/SimpleVector.h"
#include <time.h>
#include <QMutex>
#include <QTime>
#include <QSemaphore>



/**
 * Starts the file sharing layer of this node.
 *
 * Must be called before using the file sharing API.
 *
 * Should be called after calling muteSeedRandomGenerator and muteStart
 * from the messageRouting layer.
 */
void muteShareStart();



/**
 * Stops the file sharing layer of this node.
 *
 * Must be called after using the file sharing API.
 *
 * Should be called before calling muteStop from the messageRouting layer.
 */
void muteShareStop();



/**
 * Gets the virtual address that this node is using.
 *
 * @return the address.
 *   Must be destroyed by caller.
 */
char *muteShareGetLocalVirtualAddress();



/**
 * Sets the root path that this node will share files from.
 *
 * If no path is set, this node defaults to sharing files from the "files"
 * directory.
 *
 * This path is saved between startups by this API (in the settings folder).
 *
 * @param inPath the platform-dependent path, relative to the main directory
 *   of this node.
 *
 * Example: <PRE>
 * muteSetSharingPath( "temp/myFiles" );
 * </PRE> 
 */
void muteShareSetSharingPath( char *inPath );



/**
 * Gets the root path that this node is sharing files from.
 *
 * @return the platform-dependent path, relative to the main directory
 *   of this node.
 */
char *muteShareGetSharingPath();



/**
 * Sets the maximum number of downloads that can be active before
 * additional downloads are automatically queued.
 *
 * Setting a limit of 0 means that all downloads are queued (and must
 * be started manually).
 *
 * @param inLimit the maximum number of active downloads, or -1 for no limit.
 */
void muteShareSetMaxSimultaneousDownloads( long inLimit );



/**
 * Gets the maximum number of downloads that can be active before
 * additional downloads are automatically queued.
 *
 * @return the maximum number of active downloads, or 0 for no limit.
 */
unsigned long muteShareGetMaxSimultaneousDownloads(); 



/**
 * Initiates a search on the network.
 *
 * This call returns after the search has been canceled by the handler.
 *
 * @param inSearchString a string of words to search for.
 * @param inResultHandler a callback function to pass received results to.
 *   This function must return (char) and take the following arguments:
 *   (char *inFileHostVirtualAddress, char *inFilePath,
 *    unsigned long inFileLengthInBytes, char *inFileSHA1Hash,
 *    void *inExtraArgument ).
 *   inFileHostVirtualAddress is the virtual address of the host
 *      returning the result.  Will be destroyed by handler's caller.
 *   inFilePath is the /-delimited file path of the result.
 *      Will be destroyed by handler's caller.
 *   inFileLengthInBytes is the length of the file in bytes.
 *   inFileSHA1Hash is the SHA1 hash of the file contents as a hex-encoded
 *      string.
 *      Will be destroyed by handler's caller.
 *   If no new results have been received after inTimeoutInMilliseconds
 *   passes since the last handler call, the handler will be called
 *   with inFileHostVirtualAddress, inFilePath, and inFileSHA1Hash set to NULL.
 *   The handler should return true to continue the search or false
 *   to cancel the search and ignore future results.
 * @param inExtraHandlerArgument pointer to an extra argument to be passed
 *   in to the handler function each time it is called.
 *   Must be destroyed by caller after the search has been canceled.
 * @param inTimeoutInMilliseconds the time to wait between results before
 *   calling the handler with NULL arguments.
 *   Defaults to 10 seconds (10,000 milliseconds).
 * @param inAddressToSearch the virtual address of the host to search, or
 *   ALL to search all hosts in the network.
 *   Defaults to ALL.
 *   Must be destroyed by caller.
 *
 * Example: <PRE>
 * // define a handler function (this one writes results to a file)
 * char myResultHandler( char *inFileHostVirtualAddress, char *inFilePath,
 *                       unsigned long inFileLength, char *inFileHash,
 *                       void *inExtraArgument ) {
 *     // unwrap a file handle from our extra argument
 *     FILE *file = (FILE *)inExtraArgument;
 *         
 *     // check that arguments are not NULL
 *     // (in other words, new results have arrived)
 *     if( inFileHostVirtualAddress != NULL && inFilePath != NULL ) {
 *         fprintf( file, "%s : %s\n", inFileHostVirtualAddress, inFilePath );
 *         return true;
 *         }
 *     else {
 *         // no new results, must have timed out
 *         fclose( file );
 *         // cancel this search
 *         return false;
 *         }
 *     }
 *
 * // elsewhere in code, register the handler function to catch the results
 * // wrap file handle in extra argument
 * FILE *file = fopen( "searchResults", "w" );
 *
 * // use default timeout of 10 seconds
 * muteShareSearch( "test tone mp3", myChunkHandler, (void *)file );
 * </PRE>
 */
void muteShareSearch(
    char *inSearchString,
    char (*inResultHandler)( char *, char *, unsigned long, char *,void * ),
    void *inExtraHandlerArgument,
    int inTimeoutInMilliseconds = 10000,
    char *inAddressToSearch = "ALL" );



// constants returned by muteGetSharedFile
const int MUTE_SHARE_FILE_NOT_FOUND = 0;
const int MUTE_SHARE_FILE_TRANSFER_FAILED = 1;
const int MUTE_SHARE_FILE_TRANSFER_CANCELED = 2;
const int MUTE_SHARE_FILE_TRANSFER_COMPLETE = 3;
const int MUTE_SHARE_FILE_TRANSFER_HASH_MISMATCH = 4;



/**
 * Gets a file from a node on the network.
 *
 * This call returns after the file has been completely fetched and
 * passed through the handler.
 *
 * @param inVirtualAddress the virtual address of the file host.
 * @param inFilePath a /-delimited file path, relative to the host's
 *   root sharing directory.
 * @param inFileHash the SHA1 hash of the file as a hex-encoded
 *   ascii string.  The received file will be checked against this hash.
 * @param inFileChunkHandler a callback function to pass received file
 *   chunks to. This function must return (char) and take the following
 *   arguments:
 *   (unsigned char *inChunk, int inChunkLengthInBytes,
 *    void *inExtraArgument ).
 *   inChunk is the raw bytes of chunk data, or NULL to indicate that
 *      the current chunk (or file info request) has timed out and will be
 *      retried.
 *      Will be destroyed by handler's caller.
 *   inChunkLengthInBytes is the length of the chunk in bytes, or -1 if
 *      the current chunk (or file info request) has timed out and will be
 *      retried.
 *      Will be set to -2 if the file transfer is currently queued locally.
 *      Will be set to -3 to inform handler that download has just been
 *      dequeued (download now active).
 *   The handler should return true to continue the transfer or false
 *   to cancel the transfer.
 *   The handler can also return 2 to force a queued transfer to start
 *   immediately.
 * @param inExtraHandlerArgument pointer to an extra argument to be passed
 *   in to the handler function each time it is called.
 *   Must be destroyed by caller after the file transmission is complete.
 * @param outDownloadID pointer to where the download ID should be returned.
 *   This value is set before any network operations are performed by this
 *   function, so the handler and other UI elements can use this value
 *   before muteShareGetFile returns.
 *   This ID can be passed to muteShareInterruptDownload.
 * @param outFileSizeInBytes pointer to where the file size should be
 *   returned.  This value is set before the handler is called, so the
 *   handler can use this value.
 * @param inTimeoutInMilliseconds the time to wait for the file info
 *   to return.  Defaults to 30 seconds (30,000 milliseconds).
 * @param inPartialFilePath platform-dependent path to an existing partial
 *   download of this file, or NULL if no partial file exists.
 *   Defaults to NULL.
 * @param inPartialFileSizeInBytes the number of valid bytes in the partial
 *   file, or 0 if there is no partial file.  Defaults to 0.
 *
 * @return one of MUTE_SHARE_FILE_NOT_FOUND,
 *   MUTE_SHARE_FILE_TRANSFER_CANCELED, MUTE_SHARE_FILE_TRANSFER_FAILED,
 *   MUTE_SHARE_FILE_TRANSFER_COMPLETE,
 *   or MUTE_SHARE_FILE_TRANSFER_HASH_MISMATCH.
 *
 * Example: <PRE>
 * // define a handler function
 * char myChunkHandler( unsigned char *inChunk, int inChunkLengthInBytes,
 *                      void *inExtraArgument ) {
 *     // unwrap a file handle from our extra argument
 *     FILE *file = (FILE *)inExtraArgument;
 *     if( inChunk != NULL ) {
 *         // we have real data for this chunk, write it to file
 *         fwrite( inChunk, 1, inChunkLengthInBytes, file );
 *         }
 *     else {
 *         // chunk timed out, no data yet, do nothing
 *         }
 *     // do not cancel
 *     return true;
 *     }
 *
 * // elsewhere in code, register the handler function to fetch the file
 * // wrap file handle in extra argument
 * FILE *file = fopen( "receivedFile", "wb" );
 * int fileSize;
 * // use default timeout of 10 seconds
 * int result = muteShareGetFile( "C0D62D331359BF2A27BEA46693F1CD32E3B7519E",
 *                                "songs/test.mp3",
 *                                "9E6D1F25D46633BA1EC296A1D3813D887EBB8417",
 *                                myChunkHandler, (void *)file, &fileSize );
 * // after it returns, the file transfer is complete
 * fclose( file );
 * if( result != MUTE_SHARE_FILE_TRANSFER_COMPLETE ) {
 *     // handle a failure
 *     ...
 *     }
 * </PRE>
 */
int muteShareGetFile(
    char *inVirtualAddress, char *inFilePath,
    char *inFileHash,
    char (*inFileChunkHandler)( unsigned char *, int, int, void * ),
    void *inExtraHandlerArgument,
    int *outDownloadID,
    int *outFileSizeInBytes,
    int inTimeoutInMilliseconds = 30000,
    char *inPartialFilePath = NULL,
    int inPartialFileSizeInBytes = 0 );



/**
 * Interrupts a download, forcing it to abort its current chunk fetching
 * operation and call the file chunk handler right away.
 *
 * If the chunk handler callback function returns a cancel flag, the download
 * will be canceled.  Otherwise, the download will continue.
 *
 * This function is intended to allow faster cancel operations (without
 * waiting for the current chunk fetch to complete).
 *
 * @param inDownloadID the ID of the download to cancel (as returned in
 *   outDownloadID by muteShareGetFile).
 */
void muteShareInterruptDownload( int inDownloadID );



/**
 * Gets the number of sources available in the network for a given download.
 *
 * @param inDownloadID the ID of the download to get the source count for
 *  (as returned in outDownloadID by muteShareGetFile).
 *
 * @return the source count.
 */
int muteShareGetSourceCount( int inDownloadID );



/**
 * Gets the current best source for a given download.
 *
 * @param inDownloadID the ID of the download to get the best soruce for
 *  (as returned in outDownloadID by muteShareGetFile).
 *
 * @return the virtual address of the best source.
 *   Must be destroyed by caller.
 */
char *muteShareGetBestSource( int inDownloadID );



/**
 * Gets whether this download is in a hibernation state waiting for more
 * sources to come online.
 *
 * @param inDownloadID the ID to check
 *  (as returned in outDownloadID by muteShareGetFile).
 *
 * @return true if hibernating, or false if not.
 */
char muteShareIsDownloadHibernating( int inDownloadID );



/**
 * Gets a list of upload status information.
 *
 * @param outUploadIDs pointer to location where an array of unique IDs
 *   for the uploads should be returned.
 *   The array must be destroyed by caller.
 * @param outHostAddresses pointer to location where an array of host virutal
 *   addresses strings, one for each upload, should be returned.
 *   The array and the strings it contains must be destroyed by caller.
 * @param outFilePaths pointer to location where an array of file path
 *   strings, one for each upload, should be returned.
 *   The array and the strings it contains must be destroyed by caller.
 * @param outChunksInFile pointer to location where an array of chunk counts,
 *   one for each upload, should be returned.
 *   The returned values indicate how many chunks are in each file.
 *   The array must be destroyed by caller.
 * @param outLastChunks pointer to location where an array of indexes of last
 *   chunk sent, one for each upload should be returned.
 *   These values give an estimate of how close an upload is to completion.
 *   The array must be destroyed by caller.
 * @param outFirstChunkTimes pointer to location where an array of
 *   first chunk times, one for each upload should be returned.
 *   These values are times in seconds as returned by the system call
 *   time( NULL ).
 *   The array must be destroyed by caller.
 * @param outLastChunkTimes pointer to location where an array of
 *   last chunk times, one for each upload should be returned.
 *   These values are times in seconds as returned by the system call
 *   time( NULL ).
 *   The array must be destroyed by caller.
 *
 * @return the number of uploads.
 *   (in other words, the length of each returned array).
 *
 * Example: <PRE>
 * int *uploadIDs;
 * char **hostAddresses;
 * char **filePaths;
 * int *chunkCounts;
 * int *lastChunks;
 * unsigned long *firstChunkTimes;
 * unsigned long *lastChunkTimes;
 * int numUploads = muteShareGetUploadStatus( &uploadIDs, &hostAddresses,
 *                                            &filePaths, &chunkCounts,
 *                                            &lastChunks, &firstChunkTimes,
 *                                            &lastChunkTimes );
 *
 * // process information here
 * ...
 * // now destroyed returned arrays
 * for( int i=0; i<numUploads; i++ ) {
 *     delete [] filePaths[i];
 *     delete [] hostAddresses[i];
 *     }
 * delete [] uploadIDs;
 * delete [] hostAddresses;
 * delete [] filePaths;
 * delete [] chunkCounts;
 * delete [] lastChunks;
 * delete [] firstChunkTimes;
 * delete [] lastChunkTimes;
 * </PRE>
 */
int muteShareGetUploadStatus( int **outUploadIDs,
                              char ***outHostAddresses,
                              char ***outFilePaths,
                              int **outChunksInFile,
                              int **outLastChunks,
                              unsigned long **outFirstChunkTimes,
                              unsigned long **outLastChunkTimes );


class SourceQueue;

class ShareFileChunkWrapper
{

public:


  ShareFileChunkWrapper ()
  {
  }


   ~ShareFileChunkWrapper ()
  {

  }

  char *mVirtualAddress;
  SourceQueue *sources;

  int mChunkNumber;

  // true if chunk data has already been received and set here
  // for mChunkNumber
  // used by FileChunkHandler to avoid overwritting data twice
  char mDataReceived;

  int mLengthInBytes;
  unsigned char *mChunkData;

  QMutex mLock;
  QSemaphore *mSemaphore;
  QTime startTime;
}
;

/**
 * Container for information about a source.
 */
class FileSource
{

public:

	/**
         * Constructs a source.
         *
         * Chunk time starts at 0.
         *
         * @param inVirtualAddress the virtual address.
         *   Will be destroyed when this class is destroyed.
         * @param inFilePath the file path.
         *   Will be destroyed when this class is destroyed.
         */
  FileSource (char *inVirtualAddress,
	      char *inFilePath):mVirtualAddress (inVirtualAddress),
    mFilePath (inFilePath), mChunkTimeInMilliseconds (100000)
  {
    mCreated.start();
    mLastReceipt.start();
  }



   ~FileSource ()
  {
    delete[]mVirtualAddress;
    delete[]mFilePath;
  }

  void setLastReceipt()
  {
    mLastReceipt.start();
  }


  char *mVirtualAddress;
  char *mFilePath;
  long mChunkTimeInMilliseconds;// >= 0 if active source, < 0 if not.
  QTime mLastReceipt;
  QTime mCreated;
}
;



/**
 * Tracks a list of possible sources for a file.
 *
 * Automatically searches for more sources when too many known sources have
 * failed.
 *
 * All calls are thread-safe.
 *
 * @author Jason Rohrer.
 */
class SourceQueue
{

public:

	/**
         * Constructs a queue with one source in it.
         *
         * Does not automatically search to fill the queue.
         *
         * @param inFileHash the file hash.
         *   Must be destroyed by caller.
         * @param inFirstVirtualAddress the address of the first source
         *   that we know about.
         *   Must be destroyed by caller.
         * @param inFirstFilePath the file path for the first source.
         *   Must be destroyed by caller.
         */
  SourceQueue (char *inFileHash,
	       char *inFirstVirtualAddress, char *inFirstFilePath);


   ~SourceQueue ();



	/**
         * Report the time to receive a chunk from the best source (the last
         * source returned by getNextSource).
         *
         * @param inMilliseconds the time in milliseconds, or -1 if
         *   Fetching the chunk failed.
         */
  void reportBestSourceChunkTime (long inMilliseconds);



	/**
         * Gets the next source from the queue.
         * Since queue is never empty, this call will always return a source.
         *
         * @param outVirtualAddress pointer to where the virtual address
         *   should be returned.
         *   Returned value must be destroyed by caller.
         * @param outFilePath pointer to where the file path
         *   should be returned.
         *   Returned value must be destroyed by caller.
         */
  void getNextSource (char **outVirtualAddress, char **outFilePath);
  FileSource *getSource(int index) {
	  if(index>=0 && index < mSourceVector->size())
		  return *(mSourceVector->getElement (index));
  	  else return NULL;
  };





	/**
         * Adds a source to this queue, or refreshes an existing source as
         * new.
         *
         * @param inSource the source to add.
         *   Must be destroyed by caller.
         */
  void addSource (FileSource * inSource);



	/**
         * Gets the number of active sources (non-failed) in the queue.
         *
         * @return the number of active sources.
         */
  int getActiveSourceCount ();
  int getSourceCount () {return mSourceVector->size();};



	/**
         * Sets the hibernation state for this queue.
         *
         * @param inHibernating true if hibernating, or false otherwise.
         */
  void setHibernating (char inHibernating)
  {
    mLock.lock ();
    mHibernating = inHibernating;
    mLock.unlock ();
  }



	/**
         * Gets the hibernation state for this queue.
         *
         * @return true if hibernating, or false otherwise.
         */
  char isHibernating ()
  {
    mLock.lock ();
    char returnValue = mHibernating;
    mLock.unlock ();

    return returnValue;
  }

	/**
         * Looks in queue for another source to assume position 0.
         */
  void findNewBest ();



protected:
  QMutex mLock;

  char mHibernating;



  char *mFileHash;
  SimpleVector < FileSource * >*mSourceVector;


  // best count of non-failed hosts we have seen so far
  long mLongestSourceList;


  QTime mLastSearchStart;


}
;

#endif

