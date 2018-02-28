//
//	DoppelTimer - Personal time tracking app
//	Copyright (C) 2018  Oleksii Mokrintsev
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef CWINDOWSTICKER_H
#define CWINDOWSTICKER_H

#include <QTimer>
#include <QPoint>

// Default distance from which window do sticking to the edge of screen
#define DEFAULT_STICKY_DELTA	15


//
//	class CWindowSticker
//

//
//	Implements configurable window "sticky" behaviour on moving.
//	As a bonus, adds possibility of dragging frameless windows.
//
//	Sets itself as an object child, so it destroyed automatically with window.
//
//		Usage:
//
//		CMyWindow win;
//		new CWindowSticker (&win, kStickVerticalSides | kDragFramelessWindow);
//

class CWindowSticker : public QObject
{
	Q_OBJECT

public:
	
	enum		// construction flag parameters
	{
		kStickLeftSide					= 0x1,
		kStickRightSide					= 0x2,
		kStickTopSide					= 0x4,
		kStickBottomSide				= 0x8,
		
		kDragFramelessWindow	= 0x10,
		
		kStickHorizontalSides		= (kStickLeftSide | kStickRightSide),
		kStickVerticalSides			= (kStickTopSide | kStickBottomSide),
		kStickAll							= (kStickHorizontalSides | kStickVerticalSides)
	};
	
	CWindowSticker (QWidget *inWindow, uint inFlags = kStickAll,
					int inStickyDelta = DEFAULT_STICKY_DELTA);
	
	virtual ~CWindowSticker ();
	
protected:
	
	virtual bool eventFilter (QObject *obj, QEvent *event);
	
	// Returns rectangular frame to stick to (window's desktop area by default)
	virtual QRect stickyArea ();
	
	virtual bool isReady2Stick ();
	
	// Checks intersection of two line segments
	bool isIntersects (int inA1, int inA2, int inB1, int inB2)
	{
		return not (inA2 < inB1  ||  inA1 > inB2);
	}
	
public slots:
	
	void updatePosition ();
	
protected:
	
	QWidget		*mWindow;
	uint				mFlags;
	int					mStickyDelta;
	
	QPoint			mPtPosition;
	QTimer			mMoveTimer;
};

#endif // CWINDOWSTICKER_H
