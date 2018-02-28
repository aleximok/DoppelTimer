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
#include <QtWidgets>

#include "CActivityDelegate.h"
#include "CActivityDialog.h"


CActivityDelegate::CActivityDelegate (qlonglong inActivityId, int inTUMins, QObject *parent) :
	QSqlRelationalDelegate (parent),
	mTUMins (inTUMins),
	mActivityId (inActivityId)
{
}


void
CActivityDelegate::paint (QPainter *painter, 
	const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem myOption = option;
	
	if (index.model ()->data (index.model ()->
			index (index.row (), 0), Qt::DisplayRole).toLongLong () == mActivityId)
	{
		myOption.font.setWeight (QFont::Bold);
	}
	
	switch (index.column ())
	{
		case kDate:
		{
			QDate date = QDate::fromString (
						index.model ()->data (index, Qt::DisplayRole).toString (), Qt::ISODate);
			
			QString text = date.toString (Qt::SystemLocaleShortDate);
			
			myOption.displayAlignment = Qt::AlignVCenter;

			drawDisplay (painter, myOption, myOption.rect, text);
			drawFocus (painter, myOption, myOption.rect);
			break;
		}
		
		case kWorktime:
		{
			int mins = index.model ()->data (index, Qt::DisplayRole).toInt ();
			QString text = QString("%1:%2").
					arg (mins / 60, 2, 10, QChar ('0')).arg (mins % 60, 2, 10, QChar ('0'));
			
			myOption.displayAlignment = Qt::AlignCenter | Qt::AlignVCenter;
			
			drawDisplay (painter, myOption, myOption.rect, text);
			drawFocus (painter, myOption, myOption.rect);
			break;
		}
		
		case kWTU:
		case kEstimateWTU:
		case kDifference:
		{
			QString text = index.model ()->data (index, Qt::DisplayRole).toString ();
			
			myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
			
			drawDisplay (painter, myOption, myOption.rect, text);
			drawFocus (painter, myOption, myOption.rect);
			break;
		}
			
		case kTime:
		case kState:
		case kPriority:
		{
			QString text = index.model ()->data (index, Qt::DisplayRole).toString ();
			
			myOption.displayAlignment = Qt::AlignCenter | Qt::AlignVCenter;
			
			drawDisplay (painter, myOption, myOption.rect, text);
			drawFocus (painter, myOption, myOption.rect);
			break;
		}
			
		default:
			
	        QSqlRelationalDelegate::paint (painter, myOption, index);
    }
}


QSize
CActivityDelegate::sizeHint (const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	QSize size = QSqlRelationalDelegate::sizeHint (option, index);

	size.setWidth (size.width () * 1.2);	// add by default little more space for bold selection

	return size;
}


QWidget *
CActivityDelegate::createEditor (QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
	switch (index.column ())
	{
		case kDate:
		{
			if (index.model ()->data (index.model ()->
					index (index.row (), kId), Qt::DisplayRole).toLongLong () == mActivityId)
			{
				return NULL;				// Disable changing the date of picked activity
			}
			
			QDateEdit *dateEdit = new QDateEdit (parent);
			return dateEdit;
		}
		
		case kTime:
		{
			QTimeEdit *timeEdit = new QTimeEdit (parent);
			timeEdit->setDisplayFormat (kTimeFormat);
			return timeEdit;
		}
		
		case kWorktime:
		{
			if (index.model ()->data (index, Qt::DisplayRole).toInt () > 1439)
			{
				return NULL;				// QTimeEdit can not deal with time over 23:59
			}
			
			QTimeEdit *timeEdit = new QTimeEdit (parent);
			timeEdit->setDisplayFormat (kTimeFormat);
			return timeEdit;
		}
		
		case kWTU:
		{
			QModelIndex idxWTU = index.model ()->index (index.row (), kWTU);
			mTU = idxWTU.model ()->data (idxWTU, Qt::DisplayRole).toInt ();
			
			QSpinBox *spinEdit = new QSpinBox (parent);
	        spinEdit->setMinimum (0);
	        spinEdit->setMaximum (1000);
			
	        return spinEdit;
		}
		
		case kEstimateWTU:
		{
			QSpinBox *spinEdit = new QSpinBox (parent);
	        spinEdit->setMinimum (0);
	        spinEdit->setMaximum (1000);
			
	        return spinEdit;
		}
		
		case kDifference:
			
			return NULL;
			
		default:
			
	        return QSqlRelationalDelegate::createEditor (parent, option, index);
    }
}


void
CActivityDelegate::setEditorData (QWidget *editor, const QModelIndex &index) const
{
	switch (index.column ())
	{
		case kDate:
		{
			QString str = index.model ()->data (index, Qt::DisplayRole).toString ();
			QDateEdit *dateEdit = qobject_cast<QDateEdit *> (editor);
			
			dateEdit->setDate (QDate::fromString (str, Qt::ISODate));
			
			emit updateDailyStats ();
			
			break;
		}

		case kTime:
		{
			QString str = index.model ()->data (index, Qt::DisplayRole).toString ();
			QTimeEdit *timeEdit = qobject_cast<QTimeEdit *> (editor);
			
			timeEdit->setTime (QTime::fromString (str, kTimeFormat));
			
			break;
		}

		case kWorktime:
		{
			int mins = index.model()->data (index, Qt::DisplayRole).toInt ();
			QTimeEdit *timeEdit = qobject_cast<QTimeEdit *> (editor);
			timeEdit->setTime (QTime (mins / 60, mins % 60, 0));
			
			emit updateDailyStats ();
			
			break;
		}
		
		case kWTU:
		{
			int value = index.model ()->data (index, Qt::DisplayRole).toInt ();
			
			QSpinBox *spinBox = static_cast<QSpinBox*> (editor);
			spinBox->setValue (value);
			
			emit updateDailyStats ();
			
			break;
		}

		case kEstimateWTU:
		{
			int value = index.model ()->data (index, Qt::DisplayRole).toInt ();
			
			QSpinBox *spinBox = static_cast<QSpinBox*> (editor);
	        spinBox->setValue (value);
			
			break;
		}
		
		default:
			
	        QSqlRelationalDelegate::setEditorData (editor, index);
    }
}


void
CActivityDelegate::setModelData (QWidget *editor,
	QAbstractItemModel *model, const QModelIndex &index) const 
{
	switch (index.column ())
	{
		case kDate:
		{
			QDateEdit *dateEdit = qobject_cast<QDateEdit *> (editor);
			QDate date = dateEdit->date ();
			
			model->setData (index, date.toString (Qt::ISODate));
			break;
		}
		
		case kTime:
		{
			QTimeEdit *timeEdit = qobject_cast<QTimeEdit *> (editor);
			QTime time = timeEdit->time ();

			model->setData (index, time.toString (kTimeFormat));
			
			break;
		}

		case kWorktime:
		{
			QTimeEdit *timeEdit = qobject_cast<QTimeEdit *> (editor);
	        QTime time = timeEdit->time ();
	        int mins = (time.hour () * 60) + time.minute ();

			model->setData (index, mins);
			
			break;
		}
		
		case kWTU:
		case kEstimateWTU:
		{
				QSpinBox *spinBox = qobject_cast<QSpinBox*> (editor);
		        spinBox->interpretText ();
		        int value = spinBox->value ();
				
		        model->setData (index, value);
				
				QModelIndex idxWTU = index.model ()->index (index.row (), kWTU);
				QModelIndex idxEstimate = index.model ()->index (index.row (), kEstimateWTU);
				QModelIndex idxDiff = index.model ()->index (index.row (), kDifference);
				
				int valWTU = idxWTU.model ()->data (idxWTU, Qt::DisplayRole).toInt ();
				int valEstimate = idxWTU.model ()->data (idxEstimate, Qt::DisplayRole).toInt ();
				
				model->setData (idxDiff, valWTU - valEstimate);
				
				if (index.column () == kWTU)
				{
					// Additionally update the worktime
					
					QModelIndex idxWorktime = index.model ()->index (index.row (), kWorktime);
					int worktimeMins = idxWorktime.model ()->data (idxWorktime, Qt::DisplayRole).toInt ();
					
					worktimeMins +=  (value - mTU) * mTUMins;
					
					model->setData (idxWorktime, (worktimeMins < 0) ? 0 : worktimeMins);
				}
				
				break;
		}
			
		default:
			
	        QSqlRelationalDelegate::setModelData (editor, model, index);
    }
}
