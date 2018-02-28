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

#ifndef CSOUNDER_H
#define CSOUNDER_H

#include <QtGui>
#include <QObject>

#include <QSound>

#define SOUND_SCHEMES_NUM 2
#define SOUNDS_NUM 4

enum
{
	kStopSnd		= 0,
	kWindUpSnd	= 1,
	kRingSnd		= 2,
	kTickIncSnd	= 3,
	kTickDecSnd	= 4
};


struct CSoundFile
{
	const char *name;
	int duration;
};


//
//	class CSoundScheme
//

class CSoundScheme : public QObject
{
	Q_OBJECT
	
public:
	
	CSoundScheme ();
	
	void initialize (CSoundFile inFiles []);
	
	int getDuration (int inIndex);
	
	QSound *getSound (int inIndex);
		
protected:
	
	QSound *mSounds [SOUNDS_NUM];
	int mDurations [SOUNDS_NUM];
};


//
//	class CSounder
//

class CSounder : public QObject
{
	Q_OBJECT

public:

	explicit CSounder(QObject *parent = 0);
	
	virtual ~CSounder ();
	
	static bool		initialize ();
	static void		destroy ();
	
	// Returns time of each sound
	
	static int getPlayTime (int inIndex);
	
	static void setSound (int inState);
	
	static CSounder *getSounder ()
	{
		Q_ASSERT (sSounder != NULL);
		return sSounder;
	}
	
	static void setCurrentScheme (int inScheme);
	static int getCurrentScheme ();
	
signals:
	
public slots:

	void updatePlayState ();
	void setPlayState (int inState);
	
protected:
	
	static	 CSounder		*sSounder;
	
	CSoundScheme mSchemes [SOUND_SCHEMES_NUM];
	QTimer	*mTimer;
	
	int mCurrentScheme;
	int mCurrentState;
	int mNextState;
};

#endif // CSOUNDER_H
