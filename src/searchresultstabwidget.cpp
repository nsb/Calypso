/***************************************************************************
 *   Copyright (C) 2006 by Niels Sandholt Busch                            *
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

#include <QIcon>
 #include <QMouseEvent>

#include "searchresultstabwidget.h"
#include "searchresultsview.h"

class MyTabBar:public QTabBar
{
protected:
    void mouseReleaseEvent ( QMouseEvent * event );
};

void MyTabBar::mouseReleaseEvent ( QMouseEvent * event )
{
 int xx= event->x() - tabRect(currentIndex()).x();
 int yy= event->y() - tabRect(currentIndex()).y();
    //FIXME : make a best test for  detecting click on close icon
    if(xx>7 && yy>7 && xx<21 && yy<21)
    {
	QWidget * oldWidget= ((SearchResultsTabWidget*)parent())->widget(currentIndex());
	((SearchResultsTabWidget*)parent())->removeTab(currentIndex());
	delete oldWidget;
    }
}

SearchResultsTabWidget::SearchResultsTabWidget(QWidget *parent) : QTabWidget(parent)
{
	class MyTabBar *tab=new MyTabBar;
	setTabBar((QTabBar*)tab);
}

SearchResultsTabWidget::~SearchResultsTabWidget()
{
}

void SearchResultsTabWidget::updateNumSearchResults(QWidget *w, int numSearches,int numFiltered)
{
    int index = indexOf(static_cast<SearchResultsView*>(w));
    QString searchTerms = static_cast<SearchResultsView*>(w)->getSearchTerms();
    QString labelText = QString("%1 (%2/%3)").arg(searchTerms,QString::number(numFiltered), QString::number(numSearches));
    setTabText(index, labelText);
}

void SearchResultsTabWidget::closeAllTabs()
{
    while(count() > 0)
    {
        QWidget *w =widget(0);
        removeTab(0);
        delete w;
    }
}
