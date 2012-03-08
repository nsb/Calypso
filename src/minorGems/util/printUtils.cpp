/*
 * Modification History
 *
 * 2002-April-8   Jason Rohrer
 * Created.
 *
 * 2002-April-11   Jason Rohrer
 * Added a missing return value.
 */



#include "printUtils.h"
#include <QMutex>



#include <stdio.h>

// for variable argument lists
#include <stdarg.h>



QMutex threadPrintFLock;



int threadPrintF( const char* inFormatString, ... )  {
	threadPrintFLock.lock();
	
	va_list argList;
	va_start( argList, inFormatString );
	
	int returnVal = vprintf( inFormatString, argList ); 

	threadPrintFLock.unlock();

    return returnVal;
	}


