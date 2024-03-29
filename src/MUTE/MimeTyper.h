/*
 * Modification History
 *
 * 2002-April-20    Jason Rohrer
 * Created.
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



#ifndef MIME_TYPER_INCLUDED
#define MIME_TYPER_INCLUDED



#include <stdio.h>
#include <QStringList>
#include <QMap>



/**
 * A class that can resolve file extensions to mime types.
 *
 * @author Jason Rohrer
 */
class MimeTyper {


        
    public:



        /**
         * Constructs a mime typer.
         *
         */
        MimeTyper( QStringList mimesetting );

        ~MimeTyper();
        

        
        /**
         * Gets a mime type string from a file extension string.
         *
         * @param inFileExtension a \0-terminated string containing
         *   a file extension, including the '.'
         *   Must be destroyed by caller if non-const.
         *
         * @return the mime type as a \0-terminated string,
         *   or NULL if there is no match.
         *   Must be destroyed by caller if non-NULL.
         */
        char *getMimeType( char *inFileExtension );



        /**
         * Gets a mime type string from a file name.
         *
         * @param inFileName a \0-terminated string containing
         *   a file name with extension.
         *   Must be destroyed by caller if non-const.
         *
         * @return the mime type as a \0-terminated string,
         *   or NULL if there is no match.
         *   Must be destroyed by caller if non-NULL.
         */
        char *getFileNameMimeType( char *inFileName );

        

    protected:


        
        // a QMap containing all types read from the configuration file
	QMap<QString,QString> mMimeTypes;


        
        
    };



#endif

