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
#ifndef SEARCHTHREAD_H
#define SEARCHTHREAD_H

#include <QThread>
#include <QObject>


/**
@author Niels Sandholt Busch
*/
class SearchThread : public QThread
{


public:
    SearchThread( QObject *eventReceiver, QString address, QString searchTerms );

    ~SearchThread();

    virtual void run();
    void stop();

    static char muteSearchHandler( char *inFileHostVirtualAddress,
                                   char *inFilePath,
                                   unsigned long inFileLength,
                                   char *inFileHash,
                                   void *inExtraArgument );

    void eventsAllowed( bool );


protected:


private:
    char searchHandler( char *inFileHostVirtualAddress,
                        char *inFilePath,
                        unsigned long inFileLength,
                        char *inFileHash );
    bool running;
    QObject *eventReceiver;
    QString address,searchTerms;
    bool events;
};

#endif
