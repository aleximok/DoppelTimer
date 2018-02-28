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

#ifndef CTIMERWINDOW_H
#define CTIMERWINDOW_H

#include <QtGui>
#include <QtSql>

#include <QtWidgets/QSystemTrayIcon>
#include <QtWidgets/QMenu>

#include "ui_CTimerWindow.h"

//
//	class CTimerWindow
//

class CTimerWindow : public QMainWindow
{
	Q_OBJECT
	
public:

	explicit CTimerWindow (QWidget *parent = 0);
	virtual ~CTimerWindow ();
	
	bool isHidden ()
	{
		return mHidden;
	}
	
	bool isQuitRequired ()
	{
		return mQuitRequired;
	}	
	
protected:
    
	void readSettings ();
	void writeSettings ();
	bool createDatabase ();
	
    virtual void closeEvent (QCloseEvent*);
	
public slots:
	
	void showHide ();
	void trayActivation (QSystemTrayIcon::ActivationReason reason);
	void updateTrayToolTip (QString inString);
	void soundSwitch ();
	void showTimerTooltip ();
	void setSoundScheme (int inN);
	
	void on_mActivityTB_clicked ();
	void on_mInfoTB_clicked ();
	
	void updateTimeUnits ();
	void startTimeUnit ();
	
	void workFinished ();
	void relaxFinished ();
	
protected:
	
	Ui::CTimerWindow	mUi;
	QSqlDatabase			mDb;
	
	QSettings					mSettings;
	
	qlonglong					mActivityId;
	int								mPendingTU;
	bool							mActivityDialogOn;
	bool							mHidden;
	bool							mQuitRequired;
	QTime						mStartTime;
	
    QSystemTrayIcon		*mTrayIconP;
    QMenu						*mTrayMenuP;
};

#endif // CTIMERWINDOW_H
