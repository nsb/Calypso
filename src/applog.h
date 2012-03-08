/*
 * Modification History
 *
 * 2009-March-25
 * Created.
 *
 */


#ifndef APP_LOG_INCLUDED
#define APP_LOG_INCLUDED

#include <QCoreApplication>

/**
 */
class AppLog {


    Q_DECLARE_TR_FUNCTIONS(AppLog)


    public:

        static const int DEACTIVATE_LEVEL=0;
        static const int CRITICAL_ERROR_LEVEL=1;
        static const int ERROR_LEVEL=2;
        static const int WARNING_LEVEL=3;
        static const int INFO_LEVEL=4;
        static const int DETAIL_LEVEL=5;
        static const int TRACE_LEVEL=6;

        /**
         * These log errors at various levels.
         *
         * All char* parameters must be \0-terminated and destroyed by caller.
         */

        static void criticalError( const char *inString );
        static void criticalError( const char *inLoggerName, const char *inString );

        static void error( const char *inString );
        static void error( const char *inLoggerName, const char *inString );

        static void warning( const char *inString );
        static void warning( const char *inLoggerName, const char *inString );

        static void info( const char *inString );
        static void info( const char *inLoggerName, const char *inString );

        static void detail( const char *inString );
        static void detail( const char *inLoggerName, const char *inString );

        static void trace( const char *inString );
        static void trace( const char *inLoggerName, const char *inString );



        /**
         * Sets the logging level of the current log.
         *
         * Messages with levels above the current level will not be logged.
         *
         * @param inLevel one of the defined logging levels.
         */
        static void setLoggingLevel( int inLevel );



        /**
         * Gets the logging level of the current log.
         *
         * Messages with levels above the current level will not be logged.
         *
         * @return one of the defined logging levels.
         */
//        static int  getLoggingLevel();



    protected:

    };



#endif
