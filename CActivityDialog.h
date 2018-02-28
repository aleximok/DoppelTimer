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

#ifndef CACTIVITYDIALOG_H
#define CACTIVITYDIALOG_H

#include <QtSql>

#include "ui_CActivityDialog.h"

#include "CActivityDelegate.h"


// Activity id unsetted value 

#define UNDEFINED_ACTIVITY (-1)

const char kTimeFormat [] = "hh:mm";

enum
{
	kId					= 0,
	kDate				= 1,
	kTime				= 2,
	kActivity			= 3,
	kWTU				= 4,
	kWorktime		= 5,
	kState				= 6,
	kPriority			= 7,
	kEstimateWTU	= 8,
	kDifference		= 9
};


enum
{
	kToday				= 0,
	k3Days				= 1,
	kWeek				= 2,
	kMonth				= 3,
	k3Months			= 4,
	kYear				= 5,
	kAll					= 6
};


class CActivityDialog : public QDialog
{
	Q_OBJECT
	
public:
	
	explicit CActivityDialog (qlonglong inActivityId, int inTUMins, QWidget *parent = 0);
	virtual ~CActivityDialog ();
	
	qlonglong	getActivityId ()
	{
		return mId;
	}
	
	QString getActivityText () const
	{
		return mUi.mCurrentALE->displayText ();
	}

protected:

	void readSettings ();
	void writeSettings ();
	
	typedef QPair<int, int> QIntPair;
	
	void getSelectedRows (QList<QIntPair> &selectionList);

protected:
	
	QSqlRelationalTableModel	mModel;
	QSettings							mSettings;
	
	QMap <QString, int>			mPriorityMap;
	
	qlonglong							mId;

	CActivityDelegate				*mDelegate;
	QByteArray						mHVState;
	
	QTimer								mTimer;
	
public slots:

	void on_mAddTB_clicked ();
	
	void on_mCloneTB_clicked ();
	void on_mPickTB_clicked ();
	
	void on_mClearStateTB_clicked ();
	void on_mClearPriorityTB_clicked ();
	void on_mEstimationCB_stateChanged (int state);
	
	void trackSelection (const QItemSelection &selected, const QItemSelection &deselected);

	void updateFilters ();
	
	void updateDailyStats ();
	
	void clipboardDataChanged ();
	void copy ();
	void paste ();
	void cut ();
	void deleteSelection ();
	
	void accept ();
	void done (int r);
	
	void showPanel ();
	
private:
	
	Ui::CActivityDialog mUi;
};

#endif // CACTIVITYDIALOG_H
