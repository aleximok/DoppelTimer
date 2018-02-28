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

#include <QtGui>

#include "CDigitalTimer.h"
#include "CSounder.h"


CDigitalTimer::CDigitalTimer (QWidget *parent) :
	QLCDNumber (parent)
{
	mTotalMinutes = value ();
	mSecondsLeft = mTotalMinutes * 60;
	
	mPlaySound = false;
	
	mTimer = new QTimer (this);
    connect (mTimer, SIGNAL (timeout ()), this, SLOT (updateSeconds ()));
	
	display (timeString (true));
}


CDigitalTimer::~CDigitalTimer ()
{
}


QString
CDigitalTimer::timeString (bool inBlink)
{
	QString text; 
	
	if (mSecondsLeft > 3600)
	{
		// Show hours/minutes
		
		text = QString ("%1:%2").
			arg ((long)(mSecondsLeft / 3600), 2, 10, QLatin1Char ('0')).
			arg ((long)(mSecondsLeft % 3600)/60, 2, 10, QLatin1Char ('0'));
	}
	else
	{
		// Show minutes/seconds
		
		text = QString ("%1:%2").
			arg ((long)(mSecondsLeft / 60), 2, 10, QLatin1Char ('0')).
			arg ((long)(mSecondsLeft % 60), 2, 10, QLatin1Char ('0'));
	}
	
	if (inBlink  &&  ((mSecondsLeft % 2) != 0))
	{
        text [2] = ' ';
	}
	
	return text;
}


void
CDigitalTimer::setTotalMinutes (int inMins)
{
	mTotalMinutes = ((inMins >= 0) ? inMins : 0);
	
	if (not mTimer->isActive ())
	{
		mSecondsLeft = mTotalMinutes * 60;
		emit totalSeconds (mSecondsLeft);
		emit secondsDone (0);
	}
	
	display (timeString (true));
}


void 
CDigitalTimer::setPlaySound (int inPlaySound)
{
	mPlaySound = (inPlaySound != 0); 
	
	if (not mPlaySound)
	{
		CSounder::setSound (kStopSnd);
	}
}


void
CDigitalTimer::start ()
{
	mSecondsLeft = mTotalMinutes * 60;
	mSecondsDone = 0;
	mTimer->start (1000);
	
	if (mPlaySound)
	{
		CSounder::setSound (kTickDecSnd);
	}
	
	emit totalSeconds (mSecondsLeft);
	emit secondsDone (0);
	emit updateToolTip (QString ("%1 %2").arg (mTrayString).arg (timeString (false)));
	
	display (timeString (true));
}


void
CDigitalTimer::rewind ()
{
	reset ();
	
	if (mPlaySound)
	{
		CSounder::setSound (kWindUpSnd);
	}
}


void
CDigitalTimer::reset ()
{
	mSecondsLeft = mTotalMinutes * 60;
	mTimer->stop ();
	
	emit 	totalSeconds (mSecondsLeft);
	emit secondsDone (0);
	emit updateToolTip ("");
	
	display (timeString (true));
}


void
CDigitalTimer::updateSeconds ()
{
	if (mSecondsLeft > 0)
	{
		mSecondsLeft --;
	}
	
	if (mPlaySound)
	{
		if (mSecondsLeft == CSounder::getPlayTime (kTickIncSnd))
		{
			CSounder::setSound (kTickIncSnd);
		}
		else if (mSecondsLeft == 0)
		{
			CSounder::setSound (kRingSnd);
		}
	}
	
	emit secondsDone (++mSecondsDone);
	
	display (timeString (true));
	
	if (mSecondsLeft == 0)
	{
		mTimer->stop ();
		emit finished ();
		emit updateToolTip ("");
	}
	else
	{
		emit updateToolTip (QString ("%1 %2").arg (mTrayString).arg (timeString (false)));
	}
}
