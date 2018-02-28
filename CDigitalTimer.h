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

#ifndef CDIGITALTIMER_H
#define CDIGITALTIMER_H

#include <QtWidgets/QLCDNumber>

class CDigitalTimer : public QLCDNumber
{
	Q_OBJECT

	Q_PROPERTY (QString trayString READ trayString WRITE setTrayString)
	
public:
	
	explicit CDigitalTimer (QWidget *parent = 0);
	
	virtual ~CDigitalTimer ();

	QString trayString () const
	{
		return mTrayString;
	}
	
	QString timeString (bool inBlink);
	
signals:

	void secondsDone (int inSecs);
	void updateToolTip (QString inString);
	void totalSeconds (int inSecs);
	void finished ();
	
public slots:

	void setTrayString (QString inStr)
	{
		mTrayString = inStr;
	}

	void setTotalMinutes (int inMins);
	
	void setPlaySound (int inPlay);
	void updateSeconds ();
	
	void start ();
	void rewind ();
	void reset ();
	
protected:
	
	QTimer		*mTimer;
	
	int				mTotalMinutes;
	long			mSecondsLeft;
	long			mSecondsDone;
	bool			mPlaySound;
	
	QString		mTrayHeader;
	QString		mTrayString;
};

#endif // CDIGITALTIMER_H
