/****************************************************************
 *  Kommute is distributed under the following license:
 *
 *  Copyright (C) 2008 defnax
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionProgressBarV2>
#include <QProgressBar>
#include <QApplication>

#include "ConnectionsListDelegate.h"

const char* ConnectionsListDelegate::connectionColumnNames[ConnectionsListDelegate::NUMBER_OF_CONNECTION_COLUMNS] =
{
    (""),
    QT_TR_NOOP("Address"),
    QT_TR_NOOP("Port"),
    QT_TR_NOOP("Sent"),
    QT_TR_NOOP("Queued"),
    QT_TR_NOOP("Dropped")
};

ConnectionsListDelegate::ConnectionsListDelegate(QObject *parent)
: QAbstractItemDelegate(parent)
{
}

ConnectionsListDelegate::~ConnectionsListDelegate()
{
}

void ConnectionsListDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QString byteUnits[4] = {tr("B"), tr("KB"), tr("MB"), tr("GB")};
    QStyleOptionViewItem opt = option;
    QStyleOptionProgressBarV2 newopt;
    QRect pixmapRect;
    QPixmap pixmap;
    QString temp ;
    double multi;
    qlonglong sent;


    //set text color
    QVariant value = index.data(Qt::TextColorRole);

    if(value.isValid() && qvariant_cast<QColor>(value).isValid())
    {
        opt.palette.setColor(QPalette::Text, qvariant_cast<QColor>(value));
    }

    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;

    if(option.state & QStyle::State_Selected)
    {
        painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
    }
    else
    {
        painter->setPen(opt.palette.color(cg, QPalette::Text));
    }

    // draw the background color
    if(option.showDecorationSelected && (option.state & QStyle::State_Selected))
    {
        if(cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        {
            cg = QPalette::Inactive;
        }
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    }
    else
    {
        value = index.data(Qt::BackgroundColorRole);

        if(value.isValid() && qvariant_cast<QColor>(value).isValid())
        {
            painter->fillRect(option.rect, qvariant_cast<QColor>(value));
        }
    }


    switch(index.column())
    {
        case CICON:
            value = index.data(Qt::DecorationRole);
            pixmap = qvariant_cast<QIcon>(value).pixmap(option.decorationSize, option.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled, option.state & QStyle::State_Open ? QIcon::On : QIcon::Off);
            pixmapRect = (pixmap.isNull() ? QRect(0, 0, 0, 0): QRect(QPoint(0, 0), option.decorationSize));

            if (pixmapRect.isValid())
            {
                QPoint p = QStyle::alignedRect(option.direction, Qt::AlignLeft, pixmap.size(), option.rect).topLeft();
                painter->drawPixmap(p, pixmap);
            }
            break;
        case ADDRESS:
            painter->drawText(option.rect, Qt::AlignLeft, index.data().toString());
            break;
        case PORT:
            painter->drawText(option.rect, Qt::AlignLeft, index.data().toString());
            break;
        case SENT:
            sent = index.data().toLongLong();

            if(sent < 0)
            {
                temp = "Unknown";
            }
            else
            {
                multi = 1.0;
                for(int i = 0; i < 5; ++i)
                {
                    if (sent < 1024)
                    {
                        sent = index.data().toLongLong();
                        temp.sprintf("%.2f ", sent / multi);
                        temp += byteUnits[i];
                        break;
                    }
                    sent /= 1024;
                    multi *= 1024.0;
                }
            }
            painter->drawText(option.rect, Qt::AlignRight, temp);
            break;
        case QUEUED:
            painter->drawText(option.rect, Qt::AlignRight, index.data().toString());
            break;
        case DROPPED:
            painter->drawText(option.rect, Qt::AlignRight, index.data().toString());
            break;
        default:
            painter->drawText(option.rect, Qt::AlignCenter, index.data().toString());
    }
}

QSize ConnectionsListDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QSize(50,17);
    //Variant value = index.data(Qt::FontRole);
    //Font fnt = value.isValid() ? qvariant_cast<QFont>(value) : option.font;
    //FontMetrics fontMetrics(fnt);
    //const QString text = index.data(Qt::DisplayRole).toString();
    //Rect textRect = QRect(0, 0, 0, fontMetrics.lineSpacing() * (text.count(QLatin1Char('\n')) + 1));
    //return textRect.size();
}

