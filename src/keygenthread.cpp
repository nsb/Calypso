/***************************************************************************
 *   Copyright (C) 2005 by Niels Sandholt Busch                            *
 *   niels@guti                                                            *
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
#include "keygenthread.h"

#include "MUTE/common/CryptoUtils.h"

KeyGenThread::KeyGenThread( int inKeyLength ) : QThread(),
                                                   mKeyLength( inKeyLength ),
                                                   mNodePublicKey( NULL ),
                                                   mNodePrivateKey( NULL )                                                   
{
    start();
}

KeyGenThread::~KeyGenThread()
{
    delete [] mNodePublicKey;
    delete [] mNodePrivateKey;
}

void KeyGenThread::run()
{
    CryptoUtils::generateRSAKey( mKeyLength, &mNodePublicKey, &mNodePrivateKey );
}

QString KeyGenThread::getNodePublicKey()
{
    return QString::fromLatin1( mNodePublicKey );
}

QString KeyGenThread::getNodePrivateKey()
{
    return QString::fromLatin1( mNodePrivateKey );
}
