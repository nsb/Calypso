/***************************************************************************
 *   Copyright (C) 2005 by Niels Sandholt Busch                            *
 *   niels@diku.dk                                                         *
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
#include "connectionquality.h"

#include "mutelayer.h"

#include <QLayout>
#include <QLabel>
#include <QIcon>
#include <QPainter>
#include <QPixmap>

ConnectionQuality::ConnectionQuality(QWidget *parent)
 : QWidget(parent), numConnections(0)
{
    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setMargin(0);
    hbox->setSpacing(6);

    iconLabel = new QLabel( this );
    iconLabel->setPixmap(QPixmap::QPixmap(":/resources/notconnected.png"));
    // iconLabel doesn't change over time, so we didn't need a minimum size
    hbox->addWidget(iconLabel);
    connQualLabel = new QLabel( tr("Connection Quality") + " :", this );
    // connQualLabel doesn't change over time, so we didn't need a minimum size
    hbox->addWidget(connQualLabel);
    progressLabel = new QLabel( tr("Not Connected"), this );
    /* progressLabel changes over time, so we need to define a minimum size
       We set the minimum size to the tr("Not Connected") size, because in some
       languages this might be larger than in english.
    */
    progressLabel->setMinimumSize( progressLabel->frameSize().width() + 5, 10 );
    hbox->addWidget(progressLabel);

    setLayout( hbox );
}

ConnectionQuality::~ConnectionQuality()
{
}

void ConnectionQuality::setQuality( int progress )
{
    numConnections = progress;

    if( 0 == progress )
    {
        progressLabel->setText(tr("Not Connected"));
        iconLabel->setPixmap(QPixmap::QPixmap(":/resources/notconnected.png"));

//         QPainter painter(this);
//         QIcon icon = QIcon(":/emoticons/face-plain.png");
//         QPixmap pixmap = icon.pixmap(QSize(22,22));
//         QRect rect;
//         rect.adjust( 0, 0, 10, 10 );
//         painter.drawPixmap( rect, pixmap);

        return;
    }

    int totalSteps = MuteLayer::muteLayerGetTargetNumberOfConnections();
    float p = static_cast<float>(progress) / static_cast<float>(totalSteps);

    if( p > 0.75 )
    {
        progressLabel->setText(tr("Excellent")+QString("(%1)").arg(progress));
        iconLabel->setPixmap(QPixmap::QPixmap(":/resources/connected4.png"));
    }
    else if( p > 0.5 )
    {
        progressLabel->setText(tr("Good")+QString("(%1)").arg(progress));
        iconLabel->setPixmap(QPixmap::QPixmap(":/resources/connected3.png"));
    }
    else if( p > 0.25 )
    {
        progressLabel->setText(tr("Medium")+QString("(%1)").arg(progress));
        iconLabel->setPixmap(QPixmap::QPixmap(":/resources/connected2.png"));
    }
    else
    {
        progressLabel->setText(tr("Poor")+QString("(%1)").arg(progress));
        iconLabel->setPixmap(QPixmap::QPixmap(":/resources/connected1.png"));
    }
}

void ConnectionQuality::retranslate ()
{
    connQualLabel->setText (tr("Connection Quality") + " :");
    progressLabel->setText (tr("Not Connected"));
}

