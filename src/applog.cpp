#include "applog.h"
#include "addlogevent.h"
#include "kommute.h"

extern Kommute *pMainWin;

static int mLoggingLevel=AppLog::CRITICAL_ERROR_LEVEL;


void AppLog::criticalError( const char *inString )
{
  if(mLoggingLevel >= CRITICAL_ERROR_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2").arg(tr("CRITICAL ERROR"),(inString))));
}

void AppLog::criticalError( const char *inLoggerName, const char *inString )
{
  if(mLoggingLevel >= CRITICAL_ERROR_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2 : %3").arg(tr("CRITICAL ERROR"), (inLoggerName),inString)));
}

void AppLog::error( const char *inString )
{
  if(mLoggingLevel >= ERROR_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2").arg(tr("ERROR"),inString)));

}

void AppLog::error( const char *inLoggerName, const char *inString )
{
  if(mLoggingLevel >= ERROR_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2 : %3").arg(tr("ERROR"),inLoggerName,inString)));
}

void AppLog::warning( const char *inString )
{
  if(mLoggingLevel >= WARNING_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2").arg(tr("WARNING"),inString)));
}

void AppLog::warning( const char *inLoggerName, const char *inString )
{
  if(mLoggingLevel >= WARNING_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2 : %3").arg(tr("WARNING"),inLoggerName,inString)));
}

void AppLog::info( const char *inString )
{
  if(mLoggingLevel >= INFO_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2").arg(tr("INFO"),inString)));
}

void AppLog::info( const char *inLoggerName, const char *inString )
{
  if(mLoggingLevel >= INFO_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2 : %3").arg(tr("INFO"),inLoggerName,inString)));
}

void AppLog::detail( const char *inString )
{
  if(mLoggingLevel >= DETAIL_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2").arg(tr("DETAIL"),inString)));
}

void AppLog::detail( const char *inLoggerName, const char *inString )
{
  if(mLoggingLevel >= DETAIL_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2 : %3").arg(tr("DETAIL"),inLoggerName,inString)));
}

void AppLog::trace( const char *inString )
{
  if(mLoggingLevel >= TRACE_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2").arg(tr("TRACE"),inString)));
}

void AppLog::trace( const char *inLoggerName, const char *inString )
{
  if(mLoggingLevel >= TRACE_LEVEL)
    QApplication::postEvent (pMainWin,
               new AddLogEvent (QString("%1 : %2 : %3").arg(tr("TRACE"),inLoggerName,inString)));
}


void AppLog::setLoggingLevel( int inLevel )
{
  mLoggingLevel = inLevel;
}
