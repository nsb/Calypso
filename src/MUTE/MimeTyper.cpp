/*
 * Modification History
 *
 * 2002-April-20    Jason Rohrer
 * Created.
 *
 * 2002-April-22    Jason Rohrer
 * Fixed a bug with the mime type string.
 *
 * 2002-April-30    Jason Rohrer
 * Removed an unused variable.
 *
 * 2002-September-17    Jason Rohrer
 * Moved mime.ini into settings directory.
 *
 * 2002-October-7    Jason Rohrer
 * Added a function for getting mime types from file names.
 *
 * 2003-September-1   Jason Rohrer
 * Copied into minorGems from the konspire2b project.
 */



#include "MimeTyper.h"
#include "minorGems/util/stringUtils.h"
//#include "minorGems/io/file/File.h"
#include <QFile>
//#include "minorGems/io/file/FileInputStream.h"


#include <string.h>
#include <stdio.h>
#include <malloc.h>


MimeTyper::MimeTyper( QStringList mimesetting ) {
   for (int i=0 ; i< mimesetting.count() ; i ++ )
   {
    QStringList tmpList=mimesetting.at(i).split("|");
       mMimeTypes[tmpList[0]] = tmpList[1];
   }
}

MimeTyper::~MimeTyper() {
    }


char *MimeTyper::getMimeType( char *inFileExtension ) {


    if( mMimeTypes.contains(inFileExtension) ) {

        char *returnString = strdup(mMimeTypes[inFileExtension].toAscii().constData());
        
        return returnString;
        }
    else {
        return NULL;
        }
    }



char *MimeTyper::getFileNameMimeType( char *inFileName ) {

    int fileNameLength = strlen( inFileName );
    
    int lastPeriodIndex = -1;
            
    for( int i=0; i<fileNameLength; i++ ) {
        if( inFileName[i] == '.' ) {
            lastPeriodIndex = i;
            }
        }

    char *mimeType = NULL;
            
    if( lastPeriodIndex != -1 ) {
        char *exension = &( inFileName[lastPeriodIndex] );
        
        mimeType = getMimeType( exension );
        }
    
    return mimeType;
    }
