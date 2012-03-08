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

#include "DLListDelegate.h"
#include "xprogressbar.h"
#include "download.h"

extern int muteShareChunkSize;

const char* DLListDelegate::downloadColumnNames[DLListDelegate::NUMBER_OF_DONLOAD_COLUMNS] =
{
    ("#"),
    QT_TR_NOOP("File Name"),
    QT_TR_NOOP("Size"),
    QT_TR_NOOP("Completed"),
    QT_TR_NOOP("Speed"),
    QT_TR_NOOP("Progress"),
    QT_TR_NOOP("Priority"),
    QT_TR_NOOP("Sources"),
    QT_TR_NOOP("Status"),
    QT_TR_NOOP("Route Quality"),
    QT_TR_NOOP("Remaining"),
    QT_TR_NOOP("Done"),
    QT_TR_NOOP("Host"),
    QT_TR_NOOP("Chunks"),
    QT_TR_NOOP("Hash"),
    ("ptr")
};

DLListDelegate::DLListDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{
}

DLListDelegate::~DLListDelegate(void)
{
}

void DLListDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QString byteUnits[4] = {tr("B"), tr("KB"), tr("MB"), tr("GB")};
    QStyleOptionViewItem opt = option;
    QStyleOptionProgressBarV2 newopt;
    QRect pixmapRect;
    QPixmap pixmap;
    qlonglong fileSize;
    double progress, dlspeed, multi, sources;
    int minutes, hours, days;
    qlonglong remaining;
    QString temp , status,routequality, host, hash, ptr;
    qlonglong completed;

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
    if(index.column() != PROGRESS)
    {
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
    }
    /* add a 2 pixel margin : */
    opt.rect.setWidth(option.rect.width()-4);
    opt.rect.setLeft(option.rect.left()+2);
    /* draw data : */
    switch(index.column()) {
        case SYMBOL:
            value = index.data(Qt::DecorationRole);
            pixmap = qvariant_cast<QIcon>(value).pixmap(option.decorationSize, option.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled, option.state & QStyle::State_Open ? QIcon::On : QIcon::Off);
            pixmapRect = (pixmap.isNull() ? QRect(0, 0, 0, 0): QRect(QPoint(0, 0), option.decorationSize));

            if (pixmapRect.isValid())
            {
                QPoint p = QStyle::alignedRect(option.direction, Qt::AlignLeft, pixmap.size(), option.rect).topLeft();
                painter->drawPixmap(p, pixmap);
            }
            break;
        case SIZE:
            fileSize = index.data().toLongLong();

            if(fileSize < 0)
            {
                temp = "Unknown";
            }
            else
            {
                multi = 1.0;

                for(int i = 0; i < 5; ++i)
                {
                    if (fileSize < 1024)
                    {
                        fileSize = index.data().toLongLong();
                        temp.sprintf("%.2f ", fileSize / multi);
                        temp += byteUnits[i];
                        break;
                    }
                    fileSize /= 1024;
                    multi *= 1024.0;
                }
            }
            painter->drawText(opt.rect, Qt::AlignRight, temp);
            break;
        case REMAINING:
            remaining = index.data().toLongLong();
            minutes = remaining / 60;
            hours = minutes / 60;
            minutes = minutes - hours * 60;
            days = hours / 24;
            hours = hours - days * 24;

            if (days > 0)
            {
                temp.clear();
                temp.sprintf("%dd %2dh %dm", days, hours, minutes);
            }
            else if (hours > 0 || days > 0)
            {
                temp.clear();
                temp.sprintf("%dh %dm", hours, minutes);
            }
            else if (minutes > 0 || hours > 0)
            {
                temp.clear();
                temp.sprintf("%dm", minutes);
            }
            else
            {
                temp = tr("Unknown");
            }
            painter->drawText(opt.rect, Qt::AlignRight, temp);
            break;
        case COMPLETED:
            completed = index.data().toLongLong();
            if(completed < 0){
                temp = "Unknown";
            } else {
                multi = 1.0;
                for(int i = 0; i < 5; ++i) {
                    if (completed < 1024) {
                        completed = index.data().toLongLong();
                        temp.sprintf("%.2f ", completed / multi);
                        temp += byteUnits[i];
                    break;
                    }
                    completed /= 1024;
                    multi *= 1024.0;
                }
            }
            painter->drawText(opt.rect, Qt::AlignRight, temp);
            break;
        case DLSPEED:
            dlspeed = index.data().toDouble();
            temp.clear();
            temp.sprintf("%.2f", dlspeed/1024.);
            temp += " KB/s";
            painter->drawText(opt.rect, Qt::AlignRight, temp);
            break;
        case PROGRESS:
            progress = index.data().toDouble();
            sources = index.model()->index(index.row(),SOURCES).data().toDouble();
            {
                // create a xProgressBar
                xProgressBar progressBar(option.rect, painter);

                if (progress <100)
                    progressBar.setProgressColor(104,104,104+127*static_cast<int>(progress)/100);
                else
                    progressBar.setProgressColor(64,128+127*static_cast<int>(progress)/100,64);
                if(sources <1.0)
                    progressBar.setBackgroundColor(255,0,0);
                if(sources <2.0)
                    progressBar.setBackgroundColor(0,210,255);
                else
                    progressBar.setBackgroundColor(0,0,255);

                progressBar.setDisplayText(false); // should display % text?
                Download *dl =
                    (Download *) (index.model ()->
                  data (index.model ()->index (index.row (), PTR)).value <
                  quintptr > ());

                // calculate 3 parts
                float length1,begin2,length2,begin3;
                int chunksInFile = 1+ (dl->getSize()-1) / muteShareChunkSize;

                length1=100.0*(float)dl->nextChunk1/chunksInFile;

                if(dl->firstChunk3<0)
                    begin3=100;
                else
                    begin3=100.0*(float)dl->firstChunk3/chunksInFile;
                if(dl->firstChunk2<0)
                    length2=0,begin2=0;
                else
                {
                    begin2=100.0*(float)dl->firstChunk2/chunksInFile;
                    length2=100.0*((float)dl->nextChunk2-(float)dl->firstChunk2)/chunksInFile;
                }
                progressBar.set3Parts( length1, begin2, length2, begin3);

                //progressBar.setValue(progress); // set the progress value
                progressBar.setVerticalSpan(1);
                progressBar.paint(); // paint the progress bar
            }
            painter->drawText(opt.rect, Qt::AlignCenter, newopt.text);
            break;
        case NAME:
            // decoration
            value = index.data(Qt::DecorationRole);
            pixmap = qvariant_cast<QIcon>(value).pixmap(option.decorationSize, option.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled, option.state & QStyle::State_Open ? QIcon::On : QIcon::Off);
            pixmapRect = (pixmap.isNull() ? QRect(0, 0, 0, 0): QRect(QPoint(0, 0), option.decorationSize));

            if (pixmapRect.isValid())
            {
                QPoint p = QStyle::alignedRect(option.direction, Qt::AlignLeft, pixmap.size(), option.rect).topLeft();
                painter->drawPixmap(p, pixmap);
            }
            painter->drawText(opt.rect.translated(pixmap.size().width(), 0), Qt::AlignLeft, index.data().toString());
            break;
        case STATUS:
            painter->drawText(opt.rect, Qt::AlignLeft, index.data().toString());
            break;
        case ROUTEQUALITY:
            painter->drawText(opt.rect, Qt::AlignLeft, index.data().toString());
            break;
        case DONE:
            painter->drawText(opt.rect, Qt::AlignRight, index.data().toString()+" %");
            break;
        case HOST:
            painter->drawText(opt.rect, Qt::AlignLeft, index.data().toString());
            break;
        case CHUNKS:
            painter->drawText(opt.rect, Qt::AlignRight, index.data().toString());
            break;
        default:
            painter->drawText(opt.rect, Qt::AlignCenter, index.data().toString());
    }
}

QSize DLListDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QSize(50,17);
    //Variant value = index.data(Qt::FontRole);
    //Font fnt = value.isValid() ? qvariant_cast<QFont>(value) : option.font;
    //FontMetrics fontMetrics(fnt);
    //const QString text = index.data(Qt::DisplayRole).toString();
    //Rect textRect = QRect(0, 0, 0, fontMetrics.lineSpacing() * (text.count(QLatin1Char('\n')) + 1));
    //return textRect.size();
}

