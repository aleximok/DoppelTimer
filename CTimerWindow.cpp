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
#include <QMessageBox>

#include "CTimerWindow.h"

#include "CActivityDialog.h"
#include "CSounder.h"
#include "CDbOperations.h"
#include "CSkyScreensaver.h"


//
//	class CTimerWindow
//

CTimerWindow::CTimerWindow (QWidget *parent) :
	QMainWindow (parent),
	mSettings ("AAM", APPNAME),
	mActivityId (kUndefinedActivity),
	mPendingTU (0),
	mActivityDialogOn (false),
	mScreensaver (nullptr)
{
	mUi.setupUi (this);

	layout ()->setSizeConstraint (QLayout::SetFixedSize);
	setWindowFlags (Qt::Tool | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
	
	readSettings ();
	
	if (CSounder::getSounder ()	== NULL)
	{
		// No sound available
		
		mUi.mPlaySoundCB->setCheckState (Qt::Unchecked);
		mUi.mPlaySoundCB->setEnabled (false);
	}
	
	mTrayIconP = new QSystemTrayIcon (this);
    mTrayIconP->setToolTip (APPNAME);
	mTrayIconP->setIcon (QPixmap (QString (":/art/DoppelTimer5.png")));
	
	mUi.mActionPlaySound->setChecked (mUi.mPlaySoundCB->isChecked ());
	mUi.mActionShowTimerNotifications->setChecked (mUi.mShowTimerNotificationsCB->isChecked ());
	mUi.mActionShowRelaxScreensaver->setChecked (mUi.mShowRelaxScreensaverCB->isChecked ());

	connect (mTrayIconP, SIGNAL (activated(QSystemTrayIcon::ActivationReason)),
			 this, SLOT (trayActivation (QSystemTrayIcon::ActivationReason)));
	
	QAction* pActQuit = new QAction ("&Quit", this);
    connect (pActQuit, SIGNAL (triggered ()), qApp, SLOT (quit ()));
	
	mTrayMenuP = new QMenu (this);
	mTrayMenuP->addAction (mUi.mActionStart);
	mTrayMenuP->addAction (mUi.mActionRewind);
	mTrayMenuP->addAction (mUi.mActionActivity);
	mTrayMenuP->addSeparator ();
    mTrayMenuP->addAction (mUi.mActionShowHide);
	mTrayMenuP->addAction (mUi.mActionPlaySound);
	mTrayMenuP->addAction (mUi.mActionShowRelaxScreensaver);
	mTrayMenuP->addAction (mUi.mActionShowTimerNotifications);
	mTrayMenuP->addSeparator ();
    mTrayMenuP->addAction (pActQuit);
	
	mTrayIconP->setContextMenu (mTrayMenuP);
	mTrayIconP->show ();
	
	if (isHidden ())
	{
		mHidden = not mHidden;
		showHide ();
	}
	
	mUi.mOptionsGB->hide ();
}


CTimerWindow::~CTimerWindow ()
{
	delete mScreensaver;		// it's legit to delete nullptr
	writeSettings ();
}


CSkyScreensaver *
CTimerWindow::checkScreensaver ()
{
	// Initialization on demand

	if (mScreensaver == nullptr)
	{
		mScreensaver = new CSkyScreensaver ();

		connect (this, SIGNAL (showScreensaver ()), mScreensaver, SLOT (activate ()));
		connect (this, SIGNAL (hideScreensaver ()), mScreensaver, SLOT (deactivate ()));
	}

	return mScreensaver;
}


void 
CTimerWindow::readSettings ()
{
    mSettings.beginGroup ("/Settings");
	
	if (mSettings.contains ("/position"))
	{
		QPoint posPt  = mSettings.value ("/position").toPoint ();
		
		move (posPt);
	}
	
    mUi.mFirstSB->setValue (mSettings.value ("/firstTimer", 40).toInt ());
	mUi.mSecondSB->setValue (mSettings.value ("/secondTimer", 10).toInt ());
	
	mUi.mFirstLN->setTotalMinutes (mSettings.value ("/firstTimer", 40).toInt ());
	mUi.mSecondLN->setTotalMinutes (mSettings.value ("/secondTimer", 10).toInt ());
	
	mUi.mCloseToTrayCB->setCheckState ((Qt::CheckState)mSettings.value ("/closeToTray", Qt::Unchecked).toInt ());
	mUi.mPlaySoundCB->setCheckState ((Qt::CheckState)mSettings.value ("/playSound", Qt::Checked).toInt ());
	
	mUi.mShowTimerNotificationsCB->setCheckState ((Qt::CheckState)mSettings.value ("/showTimerNotifications", Qt::Unchecked).toInt ());
	mUi.mShowRelaxScreensaverCB->setCheckState ((Qt::CheckState)mSettings.value ("/showRelaxScreensaver", Qt::Unchecked).toInt ());

	mUi.mSoundSchemeCB->setCurrentIndex (mSettings.value ("/soundScheme", 0).toInt ());
	
	mHidden = (mSettings.value ("/hidden", 0).toInt () != 0);
	
	int panelSize = mSettings.value ("/panelSize", 1).toInt ();
	
	switch (panelSize)
	{
		case 0:		mUi.mShortRB->click ();				break;
		case 1:		mUi.mNormalRB->click ();			break;
		case 2:		mUi.mExtendedRB->click ();			break;
	}
	
    mSettings.endGroup ();
}


void
CTimerWindow::writeSettings ()
{
	mSettings.beginGroup ("/Settings");
	
	mSettings.setValue ("/position", pos ());

	mSettings.setValue ("/firstTimer", mUi.mFirstSB->value ());
	mSettings.setValue ("/secondTimer", mUi.mSecondSB->value ());
	
	mSettings.setValue ("/closeToTray", mUi.mCloseToTrayCB->checkState ());
	mSettings.setValue ("/playSound", mUi.mPlaySoundCB->checkState ());
	
	mSettings.setValue ("/showTimerNotifications", mUi.mShowTimerNotificationsCB->checkState ());
	mSettings.setValue ("/showRelaxScreensaver", mUi.mShowRelaxScreensaverCB->checkState ());
	mSettings.setValue ("/soundScheme", mUi.mSoundSchemeCB->currentIndex ());
	
	mSettings.setValue ("/hidden", mHidden ? 1 : 0);
	
	int panelSize = 0;
	
	if (mUi.mNormalRB->isChecked ())
	{
		panelSize = 1;
	}
	else if (mUi.mExtendedRB->isChecked ())
	{
		panelSize = 2;
	}
	
	mSettings.setValue ("/panelSize", panelSize);

	mSettings.endGroup ();
}


void
CTimerWindow::closeEvent (QCloseEvent */*pe*/)
{
	if (mUi.mCloseToTrayCB->isChecked ())
	{
		hide ();
	}
	else
	{
		QApplication::exit ();
	}
}


void
CTimerWindow::updateTimeUnits ()
{
	if (mActivityId == kUndefinedActivity  ||  mActivityDialogOn)
	{
		mPendingTU ++;
	}
	else
	{
		int timeUnits = (mPendingTU == 0) ? 1 : mPendingTU;
		mPendingTU = 0;

		CDbOperations::updateTimeUnits (mActivityId, timeUnits, timeUnits * mUi.mFirstSB->value (), mStartTime);
	}
}


void
CTimerWindow::showHide ()
{
	mHidden = not mHidden;
	
	mUi.mActionShowHide->setText (QString ((mHidden) ? "Show" : "Hide"));
	setVisible (not mHidden);
}


void
CTimerWindow::trayActivation (QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::DoubleClick)
	{
		showHide ();
	}
}


void
CTimerWindow::updateTrayToolTip (QString inString)
{
	mTrayIconP->setToolTip (inString.isEmpty () ? 
		QString (APPNAME) : QString (APPNAME "\n%1").arg (inString));
}


void
CTimerWindow::startTimeUnit ()
{
	if (mPendingTU == 0)
	{
		mStartTime = QTime::currentTime ();
	}
	
	mUi.mSecondLN->reset ();
	
	if (mActivityId == kUndefinedActivity)
	{
		mTrayIconP->showMessage (APPNAME " Warning!",
			QString ("Current activity not selected! Time units set on hold until you pick an activity - %1").arg (mPendingTU + 1),
			QSystemTrayIcon::Warning);
	}
}


void
CTimerWindow::soundSwitch ()
{
	mUi.mPlaySoundCB->setCheckState (mUi.mPlaySoundCB->isChecked () ? 
			Qt::Unchecked : Qt::Checked);
	
	mUi.mActionPlaySound->setChecked (mUi.mPlaySoundCB->checkState ());
}


void
CTimerWindow::updateTimerNotifications ()
{
	mUi.mShowTimerNotificationsCB->setCheckState (mUi.mShowTimerNotificationsCB->isChecked () ?
			Qt::Unchecked : Qt::Checked);
	
	mUi.mActionShowTimerNotifications->setChecked (mUi.mShowTimerNotificationsCB->checkState ());
}


void
CTimerWindow::updateRelaxScreensaver ()
{
	mUi.mShowRelaxScreensaverCB->setCheckState (mUi.mShowRelaxScreensaverCB->isChecked () ?
			Qt::Unchecked : Qt::Checked);

	mUi.mActionShowRelaxScreensaver->setChecked (mUi.mShowRelaxScreensaverCB->checkState ());
}


void
CTimerWindow::updateTimerNotificationsAction (int inState)
{
	bool bChecked (inState == Qt::Checked);

	if (mUi.mActionShowTimerNotifications->isChecked () != bChecked)
	{
		mUi.mActionShowTimerNotifications->setChecked (bChecked);
	}
}


void
CTimerWindow::updateRelaxScreensaverAction (int inState)
{
	bool bChecked (inState == Qt::Checked);

	if (mUi.mActionShowRelaxScreensaver->isChecked () != bChecked)
	{
		mUi.mActionShowRelaxScreensaver->setChecked (bChecked);
	}
}


void
CTimerWindow::setSoundScheme (int inN)
{
	CSounder::setCurrentScheme (inN);
}


void
CTimerWindow::workFinished ()
{
	if (mUi.mShowRelaxScreensaverCB->isChecked ())
	{
		checkScreensaver ();
		emit showScreensaver ();
	}
	else if (mUi.mShowTimerNotificationsCB->isChecked ())
	{
		mTrayIconP->showMessage (APPNAME, "Time to relax!", QSystemTrayIcon::Information, 3000);
	}
}


void
CTimerWindow::relaxFinished ()
{
	mUi.mStartTB->setEnabled (true);
	mUi.mActionStart->setEnabled (not mActivityDialogOn);
	
	emit hideScreensaver ();

	if (mUi.mShowTimerNotificationsCB->isChecked ())
	{
		mTrayIconP->showMessage (APPNAME, "Relax is over!", QSystemTrayIcon::Information, 3000);
	}
}


void
CTimerWindow::on_mActivityTB_clicked ()
{
	// Track reentrance and action timer finish postponed update
	
	bool bActivityPrevSelected = (mActivityId != kUndefinedActivity);
	
	if (not mActivityDialogOn)
	{
		mActivityDialogOn = true;
		
		mUi.mActionStart->setEnabled (false);
		mUi.mActionRewind->setEnabled (false);
		mUi.mActionActivity->setEnabled (false);
		
		QSqlDatabase::database ().transaction ();
		
		CActivityDialog actDlg (mActivityId, mUi.mFirstSB->value ());
		
		if (actDlg.exec () == QDialog::Accepted)
		{
			mActivityId = actDlg.getActivityId ();

			mUi.mFirstLbl->setToolTip (actDlg.getActivityText ());

			QSqlDatabase::database ().commit ();
		}
		else
		{
			QSqlDatabase::database ().rollback ();
		}
		
		mUi.mActionStart->setEnabled (mUi.mStartTB->isEnabled ());
		mUi.mActionRewind->setEnabled (true);
		mUi.mActionActivity->setEnabled (true);
		
		mActivityDialogOn = false;
	}
	
	if (mPendingTU != 0  &&  mActivityId != kUndefinedActivity)
	{
		// If activity was previously selected, simply update it
		
		if (bActivityPrevSelected)
		{
			updateTimeUnits ();
		}
		else
		{
			QMessageBox::StandardButton ret = QMessageBox::warning (NULL, APPNAME,
				QString ("There's %1 time units on hold. Update assigned current activity?").arg (mPendingTU),
				QMessageBox::Yes | QMessageBox::No);
			
			if (ret == QMessageBox::Yes)
			{
				updateTimeUnits ();
			}
			else
			{
				mPendingTU = 0;
			}	
		}
	}
}


void
CTimerWindow::on_mInfoTB_clicked ()
{
	QMessageBox box (QMessageBox::Information, "About " APPNAME, "");
	
	box.setText (
				"<h1>" APPNAME "</h1>"
				"<b>v. " APPVERSION "</b><br />"
				"Built on " __DATE__ "<br />"
				"<b>&copy;</b> " AUTHOR "<br />"
				"License GNU GPLv3"
				);
	
	box.setIconPixmap (QPixmap (":/art/DTimer128.png"));
	box.addButton ("Close", QMessageBox::AcceptRole);
	box.exec ();
}
