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

#include <QSoundEffect>
#include <QApplication>
#include <QDir>

#include "CSounder.h"


const int soundPriorities [] =
{
	0,		// CSoundState::kStopSnd
	1,		// CSoundState::kWindUpSnd
	2,		// CSoundState::kRingSnd
	3,		// CSoundState::kTickIncSnd
	3		// CSoundState::kTickDecSnd
};

CSoundFile kitchenTimer [] = 
{
	{	"snd/windUp1.wav", 5 },
	{	"snd/ring1.wav", 2 },
	{	"snd/tick10inc1.wav", 10 },
	{	"snd/tick10dec1.wav", 10 }
};

CSoundFile grandfatherClock [] = 
{
	{	"snd/windUp2.wav", 2 },
	{	"snd/ring2.wav", 21 },
	{	"snd/tick10inc2.wav", 10 },
	{	"snd/tick10dec2.wav", 10 }
};


CSounder		*CSounder::sSounder = NULL;


//
//	class CSoundScheme
//

CSoundScheme::CSoundScheme ()
{	
}


void
CSoundScheme::initialize (CSoundFile inFiles[])
{
	QDir dir (QCoreApplication::applicationDirPath ());

	for (int i = 0; i < SOUNDS_NUM; i++)
	{
		mSounds [i] = new QSound (QFileInfo (dir, inFiles [i].name).absoluteFilePath ());
		mDurations [i] = inFiles [i].duration;
		
		Q_ASSERT (mSounds [i]);
	}
}


int
CSoundScheme::getDuration (int inIndex)
{
	Q_ASSERT (inIndex > 0 && inIndex <= SOUNDS_NUM);
	
	return mDurations [inIndex - 1];
}


QSound *
CSoundScheme::getSound (int inIndex)
{
	Q_ASSERT (inIndex > 0 && inIndex <= SOUNDS_NUM);
	
	return mSounds [inIndex - 1];
}


//
//	class CSounder
//

CSounder::CSounder (QObject *parent) :
	QObject (parent), 
	mCurrentScheme (0),
	mCurrentState (CSoundState::kStopSnd),
	mNextState (CSoundState::kStopSnd)
{
	mSchemes [0].initialize (kitchenTimer);
	mSchemes [1].initialize (grandfatherClock);

	mTimer = new QTimer (this);		// destroyed with this
	mTimer->setSingleShot (true);
    connect (mTimer, SIGNAL (timeout()), this, SLOT (updatePlayState ()));
}


CSounder::~CSounder ()
{
	// sounds destroys along with parent (this)
}


bool
CSounder::initialize ()
{
	Q_ASSERT (sSounder == NULL);
	
//	if (QSound::isAvailable ())
	{
		sSounder = new CSounder  ();
	}
	
	return (sSounder != NULL);
}


void
CSounder::destroy ()
{
	delete sSounder;
	sSounder = NULL;
}


void
CSounder::setCurrentScheme (int inScheme)
{
	Q_ASSERT (inScheme >= 0  &&  inScheme < SOUND_SCHEMES_NUM);
	
	if (sSounder != NULL)
	{
		sSounder->mCurrentScheme = inScheme;
	}
}


int
CSounder::getCurrentScheme ()
{
	if (sSounder != NULL)
	{
		return sSounder->mCurrentScheme;
	}
	
	Q_ASSERT (false);
	return 0;
}


int
CSounder::getPlayTime (int inIndex)
{
	Q_ASSERT (inIndex >= CSoundState::kStopSnd  &&  inIndex <= CSoundState::kTickDecSnd);
	
	if (sSounder != NULL)
	{
		return sSounder->mSchemes [sSounder->mCurrentScheme].getDuration (inIndex);
	}
	
	Q_ASSERT (false);
	return 0;
}


void
CSounder::setSound (int inState)
{
	if (sSounder != NULL)
	{	
		sSounder->setPlayState (inState);
	}
}


void
CSounder::setPlayState (int inState)
{
	Q_ASSERT (inState >=0  &&  inState <= SOUNDS_NUM);
	
	if ((inState != CSoundState::kStopSnd)  &&  (inState == mCurrentState  ||  inState == mNextState))
	{
		return;
	}
	
	if (mCurrentState != CSoundState::kStopSnd)
	{
		// Already playing something
		
		if (soundPriorities [inState] < soundPriorities [mCurrentState])
		{
			mNextState = CSoundState::kStopSnd;		// clear the stack and play the sound immediately
		}
		else
		{
			Q_ASSERT (mNextState == CSoundState::kStopSnd);	// state stack overflow
			
			mNextState = inState;
			return;
		}
	}
	
	if (inState == CSoundState::kStopSnd)
	{
		if (mCurrentState != CSoundState::kStopSnd)		// stop playing
		{
			mSchemes [mCurrentScheme].getSound (mCurrentState)->stop ();
			mTimer->stop ();
		}
		
		mCurrentState = mNextState = CSoundState::kStopSnd;
	}
	else
	{
		if (mCurrentState != CSoundState::kStopSnd)		// stop playing
		{
			mSchemes [mCurrentScheme].getSound (mCurrentState)->stop ();
		}
		
		mCurrentState = inState;
		mSchemes [mCurrentScheme].getSound (mCurrentState)->play ();
		mTimer->start (mSchemes [mCurrentScheme].getDuration (mCurrentState) * 1000);
	}
}


void
CSounder::updatePlayState ()
{
	int state = mNextState;
	
	mCurrentState = mNextState = CSoundState::kStopSnd;
	
	if (state != CSoundState::kStopSnd)
	{
		setPlayState (state);
	}
}
