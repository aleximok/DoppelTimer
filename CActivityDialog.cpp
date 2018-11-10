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

#include "CActivityDialog.h"

#include <QHeaderView>
#include <QTextStream>
#include <QClipboard>

#include <QMenu>
#include <QMessageBox>

#ifdef Q_OS_MAC
const QString iconPath = ":/art/mac";
#else
const QString iconPath = ":/art/win";
#endif


//
//	class CActivityDialog
//

CActivityDialog::CActivityDialog (qlonglong inActivityId, int inTUMins, QWidget *parent) :
	QDialog (parent),
	mSettings ("AAM", APPNAME "Activity")
{
	mPriorityMap ["low"] = 0;
	mPriorityMap ["normal"] = 1;
	mPriorityMap ["high"] = 2;
	mPriorityMap ["urgent"] = 3;
	
	mId  = inActivityId;
	
	mUi.setupUi (this);
	
	setWindowFlags (Qt::Dialog | Qt::WindowTitleHint);
	
    mModel.setTable ("Activity");
	
	mModel.setRelation (CActivityMod::kState, QSqlRelation ("State", "id", "State"));
	mModel.setRelation (CActivityMod::kPriority, QSqlRelation ("Priority", "id", "Priority"));
	
	mModel.setHeaderData (CActivityMod::kWTU, Qt::Horizontal, "Time Units");
	
	readSettings ();
	
	updateFilters ();
	
	mModel.select ();
	mModel.setEditStrategy (QSqlTableModel::OnManualSubmit);
	
	mUi.mTableViewTV->setModel (&mModel);
	
	mDelegate = new CActivityDelegate (mId, inTUMins, mUi.mTableViewTV);
	
	mUi.mTableViewTV->setItemDelegate (mDelegate);
	mUi.mTableViewTV->setSelectionMode (QAbstractItemView::ExtendedSelection);
	mUi.mTableViewTV->setSelectionBehavior (QAbstractItemView::SelectRows);

	mUi.mTableViewTV->setColumnHidden (CActivityMod::kId, true);
	mUi.mTableViewTV->setSortingEnabled (true);
	
	if (mHVState.isEmpty ())
	{
		mUi.mTableViewTV->resizeColumnsToContents ();
	}
	else
	{
		mUi.mTableViewTV->horizontalHeader ()->restoreState (mHVState);
	}
	
	mModel.sort (CActivityMod::kDate, Qt::AscendingOrder);
	
	connect (mUi.mTableViewTV->selectionModel (),
		SIGNAL (selectionChanged (const QItemSelection &, const QItemSelection &)),
		SLOT (trackSelection ()));
	
	connect (mUi.mTableViewTV->verticalHeader (),
		SIGNAL (sectionDoubleClicked (int)),
		SLOT (pick ()));

	mUi.mFiltersGB->hide ();
	
	updateDailyStats ();

	connect (mDelegate, SIGNAL (updateDailyStats ()), SLOT (updateDailyStats ()));
	connect (QApplication::clipboard (), SIGNAL (dataChanged ()), SLOT (clipboardDataChanged ()));

	// Set OS denpendent actions attributes

	mUi.mActionCut->setShortcut (QKeySequence::Cut);
	mUi.mActionCopy->setShortcut (QKeySequence::Copy);
	mUi.mActionPaste->setShortcut (QKeySequence::Paste);
	mUi.mActionDelete->setShortcut (QKeySequence::Delete);
	
	mUi.mActionCut->setIcon (QIcon::fromTheme ("edit-cut", QIcon (iconPath + "/editcut.png")));
	mUi.mActionCopy->setIcon (QIcon::fromTheme ("edit-copy", QIcon (iconPath + "/editcopy.png")));
	mUi.mActionPaste->setIcon (QIcon::fromTheme ("edit-paste", QIcon (iconPath + "/editpaste.png")));

	addAction (mUi.mActionCut);
	addAction (mUi.mActionCopy);
	addAction (mUi.mActionPaste);
	addAction (mUi.mActionDelete);

	mUi.mPickTB->setDefaultAction (mUi.mActionPick);
	mUi.mCloneTB->setDefaultAction (mUi.mActionClone);
	mUi.mDeleteTB->setDefaultAction (mUi.mActionDelete);

	for (int i = mModel.rowCount () - 1; i >= 0; i--)
	{
		if (mModel.data (mModel.index (i, CActivityMod::kId), Qt::DisplayRole).toLongLong () == mId)
		{
			mUi.mCurrentALE->setText (mModel.data (mModel.index (i, CActivityMod::kActivity), Qt::DisplayRole).toString ());
			break;
		}
	}
	
	//	mUi.mTableViewTV->scrollToBottom ();
	mUi.mTableViewTV->selectRow (mModel.rowCount () - 1);
	
	on_mEstimationCB_stateChanged (mUi.mEstimationCB->checkState ());
	trackSelection ();
	
	mUi.mTableViewTV->horizontalHeader ()->setVisible (true);
	
	mTimer.setSingleShot (true);
	connect (&mTimer, SIGNAL (timeout ()), this, SLOT (showPanel ()));
}


CActivityDialog::~CActivityDialog ()
{
}


void
CActivityDialog::contextMenuEvent (QContextMenuEvent *event)
{
	QMenu menu (this);

	menu.addAction (mUi.mActionPick);
	menu.addAction (mUi.mActionClone);
	menu.addSeparator ();
	menu.addAction (mUi.mActionCut);
	menu.addAction (mUi.mActionCopy);
	menu.addAction (mUi.mActionPaste);
	menu.addAction (mUi.mActionDelete);

	menu.exec (event->globalPos ());
}


void 
CActivityDialog::readSettings ()
{
	mSettings.beginGroup ("/Settings");
	
	if (mSettings.contains ("/actWidth"))
	{
		int     nWidth     = mSettings.value ("/actWidth", width ()).toInt ();
		int     nHeight    = mSettings.value ("/actHeight", height ()).toInt ();
		
		resize (nWidth, nHeight);
	}
	
	mUi.mPeriodCOB->setCurrentIndex (mSettings.value ("/actPeriod", CActivityPeriod::kAll).toInt ());
	mUi.mEstimationCB->setCheckState ((Qt::CheckState) mSettings.value ("/actEstimation", Qt::Checked).toInt ());
	
	mUi.mLowCB->setCheckState ((Qt::CheckState) mSettings.value ("/actLow", Qt::Unchecked).toInt ());
	mUi.mNormalCB->setCheckState ((Qt::CheckState) mSettings.value ("/actNormal", Qt::Unchecked).toInt ());
	mUi.mHighCB->setCheckState ((Qt::CheckState) mSettings.value ("/actHigh", Qt::Unchecked).toInt ());
	mUi.mUrgentCB->setCheckState ((Qt::CheckState) mSettings.value ("/actUrgent", Qt::Unchecked).toInt ());
	mUi.mInProgressCB->setCheckState ((Qt::CheckState) mSettings.value ("/actInProgress", Qt::Unchecked).toInt ());
	mUi.mFinishedCB->setCheckState ((Qt::CheckState) mSettings.value ("/actFinished", Qt::Unchecked).toInt ());
	mUi.mCanceledCB->setCheckState ((Qt::CheckState) mSettings.value ("/actCanceled", Qt::Unchecked).toInt ());
	mUi.mPostponedCB->setCheckState ((Qt::CheckState) mSettings.value ("/actPostponed", Qt::Unchecked).toInt ());
	
	bool bShowFilters = mSettings.value ("/actShowFilters", Qt::Checked).toBool ();
	
	mUi.mFiltersTB->setChecked (bShowFilters);
	
	if (bShowFilters)
	{
		mTimer.start (0);
	}
	
	if (mSettings.value ("/actHeaderV", 0).isValid ())
	{
		mHVState = mSettings.value ("/actHeaderV", 0).toByteArray ();
	}
	
    mSettings.endGroup ();
}


void
CActivityDialog::writeSettings ()
{
	mSettings.beginGroup ("/Settings");
	
	mSettings.setValue ("/actWidth", width ());
    mSettings.setValue ("/actHeight", height ());
	
	mSettings.setValue ("/actPeriod", mUi.mPeriodCOB->currentIndex ());
	mSettings.setValue ("/actEstimation", mUi.mEstimationCB->checkState ());

	mSettings.setValue ("/actLow", mUi.mLowCB->checkState ());
	mSettings.setValue ("/actNormal", mUi.mNormalCB->checkState ());
	mSettings.setValue ("/actHigh", mUi.mHighCB->checkState ());
	mSettings.setValue ("/actUrgent", mUi.mUrgentCB->checkState ());
	mSettings.setValue ("/actInProgress", mUi.mInProgressCB->checkState ());
	mSettings.setValue ("/actFinished", mUi.mFinishedCB->checkState ());
	mSettings.setValue ("/actCanceled", mUi.mCanceledCB->checkState ());
	mSettings.setValue ("/actPostponed", mUi.mPostponedCB->checkState ());
	
	mSettings.setValue ("/actShowFilters", mUi.mFiltersTB->isChecked ());
	
	mHVState = mUi.mTableViewTV->horizontalHeader ()->saveState ();
	mSettings.setValue ("/actHeaderV", mHVState);
	
	mSettings.endGroup ();
}


void
CActivityDialog::updateDailyStats ()
{
	mModel.submitAll ();

	int totalUnits, totalTime;

	CDbOperations::getDailyStats (QDate::currentDate (), totalUnits, totalTime);

	mUi.mDailyWLE->setText (QString ("%1:%2").
		arg ((long)(totalTime / 60), 2, 10, QLatin1Char ('0')).
		arg ((long)(totalTime % 60), 2, 10, QLatin1Char ('0')));

	mUi.mDailyTULE->setText (QString::number (totalUnits));
}


void
CActivityDialog::clipboardDataChanged ()
{
    if (const QMimeData *md = QApplication::clipboard()->mimeData ())
	{
		mUi.mActionPaste->setEnabled (md->hasText ());
	}
}


void
CActivityDialog::copy ()
{
	QList<QIntPair> optSel;
	
	getSelectedRows (optSel);
	
	QString str;
	QTextStream textStream (&str, QIODevice::WriteOnly);
	
	// Selection sorted in reverse order

	for (auto pos = optSel.crbegin (); pos != optSel.crend(); ++pos)
	{
		for (int i = 0; i < pos->second; i++)
		{
			textStream << mModel.data (mModel.index (pos->first + i, CActivityMod::kDate), Qt::DisplayRole).toString () << '\t';
			textStream << mModel.data (mModel.index (pos->first + i, CActivityMod::kTime), Qt::DisplayRole).toString () << '\t';
			textStream << mModel.data (mModel.index (pos->first + i, CActivityMod::kActivity), Qt::DisplayRole).toString () << '\t';
			textStream << mModel.data (mModel.index (pos->first + i, CActivityMod::kWTU), Qt::DisplayRole).toString () << '\t';
			textStream << mModel.data (mModel.index (pos->first + i, CActivityMod::kWorktime), Qt::DisplayRole).toString () << '\t';
			textStream << mModel.data (mModel.index (pos->first + i, CActivityMod::kState), Qt::DisplayRole).toString () << '\t';
			textStream << mModel.data (mModel.index (pos->first + i, CActivityMod::kPriority), Qt::DisplayRole).toString () << '\t';
			textStream << mModel.data (mModel.index (pos->first + i, CActivityMod::kEstimateWTU), Qt::DisplayRole).toString () << '\t';
			textStream << mModel.data (mModel.index (pos->first + i, CActivityMod::kDifference), Qt::DisplayRole).toString () << '\n';
		}
	}
	
	QApplication::clipboard ()->setText (str);
}


void
CActivityDialog::paste ()
{
	mModel.submitAll ();
	
    int row = mModel.rowCount ();
    
	QString cliStr = QApplication::clipboard ()->text ();
	
	QTextStream textStream (&cliStr, QIODevice::ReadOnly);
	QString sLine;
	
	sLine = textStream.readLine ();
	
	while (not sLine.isNull ())
	{
		QStringList parts = sLine.split ("\t");
		
		int n = parts.size ();
		
		if (n > 0  &&  not sLine.trimmed ().isEmpty ())
		{
			mModel.insertRow (row);
			
			// Note: We always insert activity with current date, "in progress" state
			//    and cleared time/worktime, time units and estimations
			
			mModel.setData (mModel.index (row, CActivityMod::kDate),
							QVariant (QDate::currentDate ().toString (Qt::ISODate)));
			
			mModel.setData (mModel.index (row, CActivityMod::kWorktime), QVariant (0));
			mModel.setData (mModel.index (row, CActivityMod::kWTU), QVariant (0));
			mModel.setData (mModel.index (row, CActivityMod::kState), QVariant (0));
			
			if ((n >= CActivityMod::kPriority - 1)  &&  mPriorityMap.contains (parts.at (CActivityMod::kPriority - 1).toLower ()))
			{
				// Native activity table format
				
				mModel.setData (mModel.index (row, CActivityMod::kActivity), QVariant (parts.at (CActivityMod::kActivity - 1)));
				
				mModel.setData (mModel.index (row, CActivityMod::kPriority),
								QVariant (mPriorityMap [parts.at (CActivityMod::kPriority - 1).toLower ()]));
			}
			else	if (n >= 2  &&  mPriorityMap.contains (parts.at (1).toLower ()))
			{
				// Other option: first column -- activity, second -- priority (tab delimeted)
				
				mModel.setData (mModel.index (row, CActivityMod::kActivity), QVariant (parts.at (0).trimmed ()));
				
				mModel.setData (mModel.index (row, CActivityMod::kPriority),
								QVariant (mPriorityMap [parts.at (1).toLower ()]));
			}
			else
			{
				// Otherwise insert activity title "as is" (though, trimmed)
				
				mModel.setData (mModel.index (row, CActivityMod::kActivity), QVariant (sLine.trimmed ()));
				
				mModel.setData (mModel.index (row, CActivityMod::kPriority), QVariant (1));
			}
			
			mModel.submitAll ();
			row++;
		}
		
		sLine = textStream.readLine ();
	}

	trackSelection ();
}


void
CActivityDialog::cut ()
{
	copy ();
	deleteSelection ();
}


void 
CActivityDialog::on_mAddTB_clicked ()
{
	mModel.submitAll ();
	
    int row = mModel.rowCount ();
    mModel.insertRow (row);
	
	mModel.setData (mModel.index (row, CActivityMod::kDate),
					QVariant (QDate::currentDate ().toString (Qt::ISODate)));
	
	mModel.setData (mModel.index (row, CActivityMod::kWorktime), QVariant (0));
	mModel.setData (mModel.index (row, CActivityMod::kWTU), QVariant (0));
	mModel.setData (mModel.index (row, CActivityMod::kState), QVariant (0));
	mModel.setData (mModel.index (row, CActivityMod::kPriority), QVariant (1));
	
	mModel.submitAll ();
	
	QModelIndex index = mModel.index (row, CActivityMod::kActivity);
	
	mUi.mTableViewTV->setCurrentIndex (index);
	mUi.mTableViewTV->scrollTo (index, QAbstractItemView::PositionAtBottom);
    mUi.mTableViewTV->edit (index);
}


// Get the list of selected row ranges as a sequence of pairs 'start'/'quantity'

void
CActivityDialog::getSelectedRows (QList<QIntPair> &selectionList) const
{
	QItemSelection selection = mUi.mTableViewTV->selectionModel ()->selection ();
	
	selectionList.clear ();
	
	if (selection.size () == 0)
	{
		return;
	}

	// Sort selection range in reverse order

	std::sort (selection.begin (), selection.end (), [](const QItemSelectionRange &t1, const QItemSelectionRange &t2)
		{
			return (t2.top () < t1.top ());
		}
	);

	int top = -1;
	int h = 0;
	
	foreach (const QItemSelectionRange &pos, selection)
	{
		if (top == -1)
		{
			top = pos.top ();
			h = pos.height ();
		}
		else
		{
			int topI = pos.top ();
			int hI = pos.height ();
			
			if (topI + hI == top)
			{
				top = topI;
				h += hI;
			}
			else
			{
				selectionList << QIntPair (top, h);

				top = topI;
				h = hI;
			}
		}
	}
	
	selectionList << QIntPair (top, h);
}


void 
CActivityDialog::deleteSelection ()
{
	QList<QIntPair> optSel;
	
	getSelectedRows (optSel);
	
	foreach (const QIntPair &pos, optSel)
	{
		if (mId != kUndefinedActivity)
		{
			for (int i = 0; i < pos.second; i++)
			{
				if (mModel.data (mModel.index (pos.first + i, CActivityMod::kId), Qt::DisplayRole).toLongLong () == mId)
				{
					mId = kUndefinedActivity;
					
					mUi.mCurrentALE->setText ("");
					mDelegate->setActivityId (mId);
					
					break;
				}
			}
		}
		
		mModel.removeRows (pos.first, pos.second);
	}
	
    mModel.submitAll ();

	trackSelection ();
	updateDailyStats ();
}


void
CActivityDialog::clone ()
{
	QList<QIntPair> optSel;
	
	getSelectedRows (optSel);
	
	mModel.submitAll ();
	
	int row = mModel.rowCount ();
	
	foreach (const QIntPair &pos, optSel)
	{
		for (int i = 0; i < pos.second; i++)
		{
		    mModel.insertRow (row);
			
			mModel.setData (mModel.index (row, CActivityMod::kActivity),
					mModel.data (mModel.index (pos.first + i, CActivityMod::kActivity), Qt::DisplayRole));
			
			mModel.setData (mModel.index (row, CActivityMod::kPriority),
					mPriorityMap [mModel.data (mModel.index (pos.first + i, CActivityMod::kPriority), Qt::DisplayRole).toString ().toLower ()]);
			
			mModel.setData (mModel.index (row, CActivityMod::kDate),
							QVariant (QDate::currentDate ().toString (Qt::ISODate)));
			
			mModel.setData (mModel.index (row, CActivityMod::kWorktime), QVariant (0));
			mModel.setData (mModel.index (row, CActivityMod::kWTU), QVariant (0));
			mModel.setData (mModel.index (row, CActivityMod::kState), QVariant (0));
			mModel.submitAll ();
			
			row ++;
		}
	}

	trackSelection ();
}


void
CActivityDialog::on_mEstimationCB_stateChanged (int state)
{
	mUi.mTableViewTV->setColumnHidden (CActivityMod::kEstimateWTU, state == Qt::Unchecked);
	mUi.mTableViewTV->setColumnHidden (CActivityMod::kDifference, state == Qt::Unchecked);
}


// Pick (select) current activity

void
CActivityDialog::pick ()
{
	QModelIndex index = mUi.mTableViewTV->currentIndex ();
	
	if (index.isValid ()  &&  mModel.data (mModel.index (index.row (), CActivityMod::kDate),
				Qt::DisplayRole).toDate () == QDate::currentDate ())
	{
		QString str = mModel.data (
					mModel.index (index.row (), CActivityMod::kActivity), Qt::DisplayRole).toString ();
		
		mId = mModel.data (
					mModel.index (index.row (), CActivityMod::kId), Qt::DisplayRole).toLongLong ();
		
		mUi.mCurrentALE->setText (str);
		
		mDelegate->setActivityId (mId);
		
		// Force table update:
		
		mUi.mTableViewTV->verticalHeader ()->hide ();
		mUi.mTableViewTV->verticalHeader ()->show ();
	}
}


// Set toolbuttons enabled state according selection

void
CActivityDialog::trackSelection ()
{
	QItemSelection selection = mUi.mTableViewTV->selectionModel ()->selection ();
	
	if (selection.size () == 0)
	{
		mUi.mActionPick->setEnabled (false);
		mUi.mActionClone->setEnabled (false);
		mUi.mActionDelete->setEnabled (false);
		mUi.mActionCopy->setEnabled (false);
		mUi.mActionCut->setEnabled (false);
	}
	else
	{
		mUi.mActionClone->setEnabled (true);
		mUi.mActionDelete->setEnabled (true);
		mUi.mActionCopy->setEnabled (true);
		mUi.mActionCut->setEnabled (true);
		
		int top = -1;
		
		foreach (const QItemSelectionRange &pos, selection)
		{
			if (top == -1)
			{
				top = pos.top ();
			}
			
			if (top != pos.top ()  ||  pos.height () != 1)
			{
				mUi.mActionPick->setEnabled (false);
				return;
			}
		}
		
		if (mModel.data (mModel.index (mUi.mTableViewTV->currentIndex ().row (), CActivityMod::kDate),
						 Qt::DisplayRole).toDate () != QDate::currentDate ())
		{
			mUi.mActionPick->setEnabled (false);
		}
		else
		{
			mUi.mActionPick->setEnabled (true);
		}
	}
}


void
CActivityDialog::updateFilters ()
{
	mModel.submitAll ();
	
	QString filterStr ("");
	
	// Select activities period

	int period = mUi.mPeriodCOB->currentIndex ();
	
	if (period != CActivityPeriod::kAll)
	{
		if (period == CActivityPeriod::kToday)
		{
			filterStr += QString (" AND Activity.Date='%1'").arg (QDate::currentDate ().toString (Qt::ISODate));
		}
		else
		{
			QDate date;
			
			switch (period)
			{
				case CActivityPeriod::k3Days:
					
					date = QDate::currentDate ().addDays (-3);
					break;
					
				case CActivityPeriod::kWeek:
					
					date = QDate::currentDate ().addDays (-7);
					break;

				case CActivityPeriod::kMonth:
					
					date = QDate::currentDate ().addMonths (-1);
					break;

				case CActivityPeriod::k3Months:
					
					date = QDate::currentDate ().addMonths (-3);
					break;

				case CActivityPeriod::kYear:
					
					date = QDate::currentDate ().addMonths (-12);
					break;
			}
			
			filterStr += QString (" AND Activity.Date>='%1' AND Activity.Date<='%2'").
					arg (date.toString (Qt::ISODate)).arg (QDate::currentDate ().toString (Qt::ISODate));
		}
	}
	
	// Select activities priority

	QString filterAuxStr ("");
	
	if (mUi.mLowCB->isChecked ())
	{
		filterAuxStr += QString (" OR Activity.Priority=0");
	}

	if (mUi.mNormalCB->isChecked ())
	{
		filterAuxStr += QString (" OR Activity.Priority=1");
	}
	
	if (mUi.mHighCB->isChecked ())
	{
		filterAuxStr += QString (" OR Activity.Priority=2");
	}
	
	if (mUi.mUrgentCB->isChecked ())
	{
		filterAuxStr += QString (" OR Activity.Priority=3");
	}

	if (not filterAuxStr.isEmpty ())
	{
		filterAuxStr.remove (0, 4);
		filterStr += QString (" AND (%1)").arg (filterAuxStr);
	}
	
	// Select activities state

	filterAuxStr.clear ();
	
	if (mUi.mInProgressCB->isChecked ())
	{
		filterAuxStr += QString (" OR Activity.State=0");
	}
	
	if (mUi.mFinishedCB->isChecked ())
	{
		filterAuxStr += QString (" OR Activity.State=1");
	}

	if (mUi.mCanceledCB->isChecked ())
	{
		filterAuxStr += QString (" OR Activity.State=2");
	}
	
	if (mUi.mPostponedCB->isChecked ())
	{
		filterAuxStr += QString (" OR Activity.State=3");
	}
	
	if (not filterAuxStr.isEmpty ())
	{
		filterAuxStr.remove (0, 4);
		filterStr += QString (" AND (%1)").arg (filterAuxStr);
	}
	
	if (not filterStr.isEmpty ())
	{
		filterStr.remove (0, 5);
	}
	
	mModel.setFilter (filterStr);
}


void
CActivityDialog::on_mClearPriorityTB_clicked ()
{
	mUi.mLowCB->blockSignals (true);
	mUi.mLowCB->setChecked (false);
	mUi.mLowCB->blockSignals (false);
	mUi.mNormalCB->blockSignals (true);
	mUi.mNormalCB->setChecked (false);
	mUi.mNormalCB->blockSignals (false);
	mUi.mHighCB->blockSignals (true);
	mUi.mHighCB->setChecked (false);
	mUi.mHighCB->blockSignals (false);
	mUi.mUrgentCB->blockSignals (true);
	mUi.mUrgentCB->setChecked (false);
	mUi.mUrgentCB->blockSignals (false);
	
	updateFilters ();
}


void
CActivityDialog::on_mClearStateTB_clicked ()
{
	mUi.mInProgressCB->blockSignals (true);
	mUi.mInProgressCB->setChecked (false);
	mUi.mInProgressCB->blockSignals (false);
	mUi.mFinishedCB->blockSignals (true);
	mUi.mFinishedCB->setChecked (false);
	mUi.mFinishedCB->blockSignals (false);
	mUi.mCanceledCB->blockSignals (true);
	mUi.mCanceledCB->setChecked (false);
	mUi.mCanceledCB->blockSignals (false);
	mUi.mPostponedCB->blockSignals (true);
	mUi.mPostponedCB->setChecked (false);
	mUi.mPostponedCB->blockSignals (false);
	
	updateFilters ();
}


void 
CActivityDialog::accept ()
{
	mModel.submitAll ();
	
	QDialog::accept ();
}


void
CActivityDialog::done (int r)
{
	writeSettings ();
	
	QDialog::done (r);
}


void
CActivityDialog::showPanel ()
{
	mUi.mFiltersGB->show ();
}
