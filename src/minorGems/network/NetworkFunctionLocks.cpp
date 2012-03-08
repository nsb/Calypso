/*
 * Modification History
 *
 * 2004-January-1   Jason Rohrer
 * Created.
 */



#include "NetworkFunctionLocks.h"


// instantiate static members
QMutex NetworkFunctionLocks::mGetHostByNameLock;

QMutex NetworkFunctionLocks::mInet_ntoaLock;

