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


//
//	class CTimerWindow
//

CTimerWindow::CTimerWindow (QWidget *parent) :
	QMainWindow (parent),
	mSettings ("AAM", APPNAME),
	mActivityId (UNDEFINED_ACTIVITY),
	mPendingTU (0),
	mActivityDialogOn (false),
	mQuitRequired (true)
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
	mUi.mActionShowTimerNotifications->setChecked (mUi.mShowTimerTTipCB->isChecked ());
	
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
	
	// Create DB connection
	
	QString dbPath = QFileInfo (QDir (QCoreApplication::applicationDirPath ()), "dblog.db").absoluteFilePath ();
	bool createDB = false;
	
	if (not QFile::exists (dbPath))
	{
		QMessageBox::StandardButton ret;
		
        ret = QMessageBox::warning (NULL, APPNAME,
                     QString ("Database %1 doesn't exist.\nCreate new one?").arg (dbPath),
		QMessageBox::Yes | QMessageBox::No);
        
		if (ret == QMessageBox::Yes)
		{
			createDB = true;
		}
		else
		{
			return;
		}
	}
	
	mDb = QSqlDatabase::addDatabase ("QSQLITE");
	
    mDb.setDatabaseName (dbPath);
	
	mDb.setUserName ("");
	mDb.setHostName ("");
	mDb.setPassword ("");
	
	if (not mDb.open ())
	{
		QMessageBox::critical (NULL, APPNAME,
			QString ("Cannot open database:\n%1").arg (mDb.lastError ().databaseText ()));
		
		return;
    }
	
	if (createDB)
	{
		if (not createDatabase ())
		{
			QMessageBox::critical (NULL, APPNAME,
				QString ("Cannot create database:\n%1").arg (mDb.lastError ().databaseText ()));
	    }
	}
	
	mQuitRequired = false;
}


CTimerWindow::~CTimerWindow ()
{
	writeSettings ();
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
	
	mUi.mShowTimerTTipCB->setCheckState ((Qt::CheckState)mSettings.value ("/showTimerTooltip", Qt::Unchecked).toInt ());
	
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
	
	mSettings.setValue ("/showTimerTooltip", mUi.mShowTimerTTipCB->checkState ());
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


bool
CTimerWindow::createDatabase ()
{
	QSqlQuery query;

	QString str = "CREATE TABLE Priority (id INTEGER, Priority VARCHAR( 60 ), PRIMARY KEY ( id ASC ))";
	
	if (not query.exec (str))
	{
		return false;
	}
	
	QStringList prList;
	
	prList << "Low" << "Normal" << "High" << "Urgent";
	
	str = "INSERT INTO Priority (id, Priority) VALUES (%1, '%2')";
	
	for (int i=0; i < prList.size (); i++)
	{
		if (not query.exec (str.arg (i).arg (prList.at (i))))
		{
			return false;
		}
	}
	
	str = "CREATE TABLE State ( id INTEGER, State VARCHAR( 60 ), PRIMARY KEY ( id ASC ));";
	
	if (not query.exec (str))
	{
		return false;
	}
	
	QStringList stList;
	
	stList << "In Progress" << "Finished" << "Canceled" << "Postponed";
	
	str = "INSERT INTO State (id, State) VALUES (%1, '%2')";
	
	for (int i=0; i < stList.size (); i++)
	{
		if (not query.exec (str.arg (i).arg (stList.at (i))))
		{
			return false;
		}
	}
	
	str ="CREATE TABLE Activity ( id INTEGER PRIMARY KEY ASC AUTOINCREMENT, "
		"Date DATE, Time VARCHAR( 20 ), Activity   VARCHAR( 400 ), WTU INTEGER, Worktime INTEGER, "
		"State INTEGER DEFAULT ( 0 ) REFERENCES State ( id ), "
		"Priority INTEGER DEFAULT ( 0 ) REFERENCES Priority ( id ), "
		"Estimate   INTEGER, Difference INTEGER);";
	
	if (not query.exec (str))
	{
		return false;
	}
	
	return true;
}


void
CTimerWindow::closeEvent (QCloseEvent* /*pe*/)
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
	if (mActivityId == UNDEFINED_ACTIVITY  ||  mActivityDialogOn)
	{
		mPendingTU ++;
	}
	else
	{
		QSqlQuery query;
		QString str = "UPDATE ACTIVITY SET WTU=WTU+%1, Worktime=Worktime+%2 WHERE id=%3;";
		
		int timeUnits = (mPendingTU == 0) ? 1 : mPendingTU;
		mPendingTU = 0;
		
		if (not query.exec (str.arg (timeUnits).arg (timeUnits * mUi.mFirstSB->value ()).arg (mActivityId)))
		{
			QMessageBox::critical (NULL, APPNAME,
				QString ("Can't update database:\n%1").arg (mDb.lastError ().databaseText ()));
		}
		
		str = "UPDATE ACTIVITY SET Time='%1' WHERE id=%2 AND Time is null;";
		
		if (not query.exec (str.arg (mStartTime.toString (kTimeFormat)).arg (mActivityId)))
		{
			QMessageBox::critical (NULL, APPNAME,
				QString ("Can't update database:\n%1").arg (mDb.lastError ().databaseText ()));
		}
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
	
	if (mActivityId == UNDEFINED_ACTIVITY)
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
CTimerWindow::showTimerTooltip ()
{
	mUi.mShowTimerTTipCB->setCheckState (mUi.mShowTimerTTipCB->isChecked () ? 
			Qt::Unchecked : Qt::Checked);
	
	mUi.mActionShowTimerNotifications->setChecked (mUi.mShowTimerTTipCB->checkState ());
}


void
CTimerWindow::setSoundScheme (int inN)
{
	CSounder::setCurrentScheme (inN);
}


void
CTimerWindow::workFinished ()
{
	if (mUi.mShowTimerTTipCB->isChecked ())
	{
		mTrayIconP->showMessage (APPNAME, "Time to relax!", QSystemTrayIcon::Information, 3000);
	}
}


void
CTimerWindow::relaxFinished ()
{
	mUi.mStartTB->setEnabled (true);

	mUi.mActionStart->setEnabled (not mActivityDialogOn);
	
	if (mUi.mShowTimerTTipCB->isChecked ())
	{
		mTrayIconP->showMessage (APPNAME, "Relax is over!", QSystemTrayIcon::Information, 3000);
	}
}


void
CTimerWindow::on_mActivityTB_clicked ()
{
	// Track reentrance and action timer finish postponed update
	
	bool bActivityPrevSelected = (mActivityId != UNDEFINED_ACTIVITY);
	
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
	
	if (mPendingTU != 0  &&  mActivityId != UNDEFINED_ACTIVITY)
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
				"<b>&copy;</b> Oleksii Mokrintsev<br />"
				"License GNU GPLv3"
				);
	
	box.setIconPixmap (QPixmap (":/art/DTimer128.png"));
	box.addButton ("Close", QMessageBox::AcceptRole);
	box.exec ();
}
