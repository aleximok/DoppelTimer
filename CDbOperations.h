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

#ifndef CDBOPERATIONS_H
#define CDBOPERATIONS_H

#include <QtSql>
#include <QObject>

const char kDbFilename [] = "dblog.db";
const char kTimeFormat [] = "hh:mm";


//
//	class CDbOperations
//

class CDbOperations : public QObject
{
	Q_OBJECT

public:

	static bool initialize ();
	static void destroy ();

	static void updateTimeUnits (qlonglong inActivityId, int inUnits, int inWorktime, const QTime &inStartTime);
	static void getDailyStats (const QDate &inDate, int &outTotalUnits, int &outTotalTime);

signals:

private:

	static bool createDatabase ();

	CDbOperations ();

	QSqlDatabase			sDb;

	static CDbOperations *sOps;
};

#endif // CDBOPERATIONS_H
