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

#ifndef CACTIVITYDELEGATE_H
#define CACTIVITYDELEGATE_H

#include <QtGui>

#include <QItemDelegate>
#include <QSqlRelationalDelegate>

#include "CDbOperations.h"


//
//	class CActivityDelegate
//

class CActivityDelegate : public QSqlRelationalDelegate
{
	Q_OBJECT
	
public:
	
	CActivityDelegate (qlonglong inActivityId, int inTUMins, QObject *parent = 0);
	
	virtual void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	
	virtual QSize sizeHint (const QStyleOptionViewItem & option, const QModelIndex & index) const;

	QWidget *createEditor (QWidget *parent,
					const QStyleOptionViewItem &option,	const QModelIndex &index) const;
	
    void setEditorData (QWidget *editor, const QModelIndex &index) const;
	
    void setModelData (QWidget *editor, QAbstractItemModel *model,
					const QModelIndex &index) const;

	void setActivityId (qlonglong inActivityId)
	{
		mActivityId = inActivityId;
	}

	qlonglong getIndexActivityId (const QModelIndex &index) const;

signals:
	
	void updateDailyStats () const;
	
protected:
	
	int						mTUMins;
	mutable int		mTU;
	
	qlonglong			mActivityId;
};


#endif // CACTIVITYDELEGATE_H
