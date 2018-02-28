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

#include "CWindowSticker.h"

#include <QWidget>
#include <QMouseEvent>
#include <QApplication>

#include <QDesktopWidget>

//
//	class CWindowSticker
//

CWindowSticker::CWindowSticker (QWidget *inWindow, uint inFlags, int inStickyDelta) :
	QObject (inWindow),
	mWindow (inWindow),
	mFlags (inFlags),
	mStickyDelta (inStickyDelta)
{
	Q_ASSERT(mWindow);
	
	mWindow->installEventFilter (this);
	mMoveTimer.setSingleShot (true);
	
	connect (&mMoveTimer, SIGNAL (timeout ()), this, SLOT (updatePosition ()));
}


CWindowSticker::~CWindowSticker ()
{
}


bool
CWindowSticker::eventFilter (QObject *, QEvent *event)
{
	switch (event->type ())
	{
		case QEvent::MouseButtonPress:
		{
			if ((mFlags & kDragFramelessWindow) != 0)
			{
				mPtPosition = mWindow->geometry ().topLeft () - mWindow->pos () + ((QMouseEvent *)event)->pos ();
				return true;
			}
			
			return false;
		}
		
		case QEvent::MouseMove:
		{
			if ((mFlags & kDragFramelessWindow) != 0)
			{
				mWindow->move (((QMouseEvent *)event)->globalPos () - mPtPosition);
				return true;
			}
			
			return false;
		}
		
		case QEvent::Move:
		{
			if (isReady2Stick ())
			{
				mMoveTimer.start (0);
			}
			
			return false;
		}
		
		default:
			
			return false;
	}
}


QRect
CWindowSticker::stickyArea ()
{
	return QApplication::desktop ()->availableGeometry (mWindow);
}


bool
CWindowSticker::isReady2Stick ()
{
	QRect screen = stickyArea ();
	QRect frame = mWindow->frameGeometry ();
	
	return ( (( ((mFlags & kStickLeftSide) != 0  &&  abs (frame.left () - screen.left ()) < mStickyDelta)  ||
		((mFlags & kStickRightSide) != 0  &&  abs (frame.right () - screen.right ()) < mStickyDelta) )  &&  
			  isIntersects (screen.top (), screen.bottom (), frame.top (), frame.bottom ())) ||
		(( ((mFlags & kStickTopSide) != 0  &&  abs (frame.top () - screen.top ()) < mStickyDelta)  ||
		((mFlags & kStickBottomSide) != 0  &&  abs (frame.bottom () - screen.bottom ()) < mStickyDelta) )  &&
			isIntersects (screen.left (), screen.right (), frame.left (), frame.right ())) );
}


void
CWindowSticker::updatePosition ()
{
	// Window has been moved, update its position
	
	QRect screen = stickyArea ();
	QRect frame = mWindow->frameGeometry ();
	QRect orgFrame (frame);
	
	if (((mFlags & kStickLeftSide) != 0  &&  abs (frame.left () - screen.left ()) < mStickyDelta))
	{
		frame.moveLeft (screen.left ());
	}
	else if ((mFlags & kStickRightSide) != 0  &&  abs (frame.right () - screen.right ()) < mStickyDelta)
	{
		frame.moveRight (screen.right ());
	}
	
	if ((mFlags & kStickTopSide) != 0  &&  abs (frame.top () - screen.top ()) < mStickyDelta)
	{
		frame.moveTop (screen.top ());
	}
	else if ((mFlags & kStickBottomSide) != 0  &&  abs (frame.bottom () - screen.bottom ()) < mStickyDelta)
	{
		frame.moveBottom (screen.bottom ());
	}
	
	if (frame != orgFrame)
	{
		mWindow->move (frame.left (), frame.top ());
	}
}
