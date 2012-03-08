/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (c) 2008, defnax
 * Copyright (C) 2006  Christophe Dumez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *************************************************************************/

#ifndef SEARCHLISTDELEGATE_H
#define SEARCHLISTDELEGATE_H

#include <QItemDelegate>
#include <QStyleOptionProgressBarV2>
#include <QStyleOptionViewItemV2>
#include <QModelIndex>
#include <QPainter>
#include <QProgressBar>
#include "misc.h"

// Defines for search results list columns
#define SNAME 0
#define SSIZE 1
#define SHASH 2
#define STYPE 3
#define SHOST 4
#define HOSTHASH 5

class SearchListDelegate: public QItemDelegate {
  Q_OBJECT

  public:
    SearchListDelegate(QObject *parent=0) : QItemDelegate(parent){}

    ~SearchListDelegate(){}

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const{
      QStyleOptionViewItemV2 opt = QItemDelegate::setOptions(index, option);
      QString temp;
      switch(index.column()){
        case SSIZE:
          QItemDelegate::drawBackground(painter, opt, index);
          QItemDelegate::drawDisplay(painter, opt, option.rect, misc::friendlyUnit(index.data().toLongLong()));
          //painter->drawText(option.rect, Qt::AlignRight, temp);
          painter->drawText( option.rect, Qt::AlignRight | Qt::AlignRight, temp);
          break;
        default:
          QItemDelegate::paint(painter, option, index);
      }
    }

    QWidget* createEditor(QWidget*, const QStyleOptionViewItem &, const QModelIndex &) const {
      // No editor here
      return 0;
    }
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const {
      return QSize(50,17);
    }
};

#endif
