/*
 *	xProgressBar: A custom progress bar for Qt 4.
 *	Author: xEsk (Xesc & Technology 2008)
 *
 *	Changelog:
 *
 *	v1.0:
 *	-----
 *		- First release
 *
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

#ifndef XPROGRESSBAR_H
#define XPROGRESSBAR_H
//
#include <QRect>
#include <QColor>
#include <QPainter>
#include <QLinearGradient>
#include <QLocale>
//
class xProgressBar : public QObject
{
Q_OBJECT
	private:
		// progress vlues
		float progressValue;
		// 3 parts :
		float pLength1,pBegin2,pLength2,pBegin3;
		int schemaIndex;
		bool displayText;
		int vSpan;
		int hSpan;
		// painter config
		QRect rect;
		QPainter *painter;
		// text color
		QColor textColor;
		// progress colors
		QColor backgroundBorderColor;
		QColor backgroundColor;
		QColor gradbackgroundColor1;
		QColor gradbackgroundColor2;
		QColor gradBorderColor;
		QColor gradColor1;
		QColor gradColor2;
		// configure the color
		void setColor();
	public:
		xProgressBar(QRect rect, QPainter *painter, int schemaIndex = 0);
		void paint();
		void setValue(const float value);
    void setColorSchema(const int value);
		void set3Parts(const float length1,const float begin2,const float length2, const float begin3);
		void setDisplayText(const bool display);
		void setVerticalSpan(const int value);
		void setHorizontalSpan(const int value);
		
		inline void setBackgroundColor(int R,int G, int B){
		gradbackgroundColor2.setRgb(R, G, B);
	    gradbackgroundColor1.setRgb(R*1/2, G*1/2, B*1/2);  
		backgroundBorderColor.setRgb(R/2, G/2, B/2);
		}; 
		inline void setProgressColor(int R,int G, int B){
		gradColor2.setRgb(R, G, B); 
		gradColor1.setRgb(R*2/3, G*2/3, B*2/3); 
		gradBorderColor.setRgb(R/3, G/3, B/3); 
		};
		static QPixmap getPixmap(QSize size, float progressValue, bool displayText = false, int schemaIndex = 0);
};
#endif
