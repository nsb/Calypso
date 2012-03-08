/***************************************************************************
 *   Copyright (C) 2005 by Niels Sandholt Busch   *
 *   niels@diku.dk   *
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
#include "connectionsthread.h"
#include "connectionevent.h"
#include "connectioncountevent.h"
#include "connectionattemptevent.h"
#include "kommutedefs.h"

#include "mutelayer.h"


#include <iostream>


#include <QApplication>
#include <QEvent>

const int   DEFAULT_CONNTHREAD_SLEEP = 5;

ConnectionsThread::ConnectionsThread(QObject *eventReceiver)
 : QThread(), eventReceiver( eventReceiver )
{
}


ConnectionsThread::~ConnectionsThread()
{
    wait();
}

void ConnectionsThread::run()
{
    running = true;
    std::cout << "ConnectionsThread started" << std::endl;
    char **addresses;
    int *ports;
    int *sentCounts;
    int *queuedCounts;
    int *droppedCounts;
    int connCt = 0;
    int oldCt = 0;
    
    while( running )
    {
        // Get the connected hosts list from MUTE.
        connCt = MuteLayer::muteLayerGetConnectedHostList( &addresses, &ports,
                                           &sentCounts, &queuedCounts,
                                           &droppedCounts );

        QApplication::postEvent( eventReceiver, new QEvent(static_cast<QEvent::Type>(KommuteDefs::EVENTTYPE_CLEARCONNECTIONS)));
        if( connCt > 0 ) {
            // Add the new connections to the tree view data structure.
            for( int i = 0; i < connCt; i++ ) {
                QApplication::postEvent( eventReceiver,
                                         new ConnectionEvent( addresses[ i ],
                                                              ports[ i ],
                                                              sentCounts[ i ],
                                                              queuedCounts[ i ],
                                                              droppedCounts[ i ] ) );
                delete [] addresses[i];
            }
        }

        if(oldCt != connCt)
        {
            QApplication::postEvent( eventReceiver, new ConnectionCountEvent( connCt ) );
        }

        oldCt = connCt;

        delete [] addresses;
        delete [] ports;
        delete [] sentCounts;
        delete [] queuedCounts;
        delete [] droppedCounts;

        // Get current connection attempt, if any.
        char *currentAttemptAddress;
        int currentAttemptPort;
        char attempting = MuteLayer::muteLayerGetCurrentConnectionAttempt( &currentAttemptAddress,
                                                                           &currentAttemptPort );
        if( attempting )
        {
            QApplication::postEvent( eventReceiver, new ConnectionAttemptEvent( currentAttemptAddress, currentAttemptPort ) );
            delete [] currentAttemptAddress;
        }

        QThread::sleep( DEFAULT_CONNTHREAD_SLEEP );
    }
}

void ConnectionsThread::stop()
{
    running = false;
}
