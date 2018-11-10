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

#include "CDbOperations.h"

#include <QFileInfo>
#include <QMessageBox>


//
//	class CDbOperations
//

CDbOperations *CDbOperations::sOps = nullptr;


CDbOperations::CDbOperations ()
{
}


bool
CDbOperations::initialize ()
{
	Q_ASSERT (sOps == nullptr);		// already inited

	// Create DB connection

	QString dbPath = QFileInfo (QDir (QCoreApplication::applicationDirPath ()), kDbFilename).absoluteFilePath ();
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
			return false;
		}
	}

	sOps = new CDbOperations ();

	sOps->sDb = QSqlDatabase::addDatabase ("QSQLITE");

	sOps->sDb.setDatabaseName (dbPath);

	sOps->sDb.setUserName ("");
	sOps->sDb.setHostName ("");
	sOps->sDb.setPassword ("");

	if (not sOps->sDb.open ())
	{
		QMessageBox::critical (NULL, APPNAME,
			QString ("Cannot open database:\n%1").arg (sOps->sDb.lastError ().databaseText ()));

		delete sOps;
		sOps = nullptr;
		return false;
	}

	if (createDB)
	{
		if (not createDatabase ())
		{
			QMessageBox::critical (NULL, APPNAME,
				QString ("Cannot create database:\n%1").arg (sOps->sDb.lastError ().databaseText ()));

			destroy ();
			return false;
		}
	}

	return true;
}


void
CDbOperations::destroy ()
{
	if (sOps != nullptr)
	{
		QSqlDatabase::removeDatabase ("QSQLITE");

		delete sOps;
		sOps = nullptr;
	}
}


void
CDbOperations::updateTimeUnits (qlonglong inActivityId, int inUnits, int inWorktime, const QTime &inStartTime)
{
	Q_ASSERT (sOps != nullptr);

	QSqlQuery query;
	QString str = "UPDATE ACTIVITY SET WTU=WTU+%1, Worktime=Worktime+%2 WHERE id=%3;";

	if (not query.exec (str.arg (inUnits).arg (inWorktime).arg (inActivityId)))
	{
		QMessageBox::critical (NULL, APPNAME,
			QString ("Can't update database:\n%1").arg (sOps->sDb.lastError ().databaseText ()));
	}

	str = "UPDATE ACTIVITY SET Time='%1' WHERE id=%2 AND Time is null;";

	if (not query.exec (str.arg (inStartTime.toString (kTimeFormat)).arg (inActivityId)))
	{
		QMessageBox::critical (NULL, APPNAME,
			QString ("Can't update database:\n%1").arg (sOps->sDb.lastError ().databaseText ()));
	}
}


void
CDbOperations::getDailyStats (const QDate &inDate, int &outTotalUnits, int &outTotalTime)
{
	Q_ASSERT (sOps != nullptr);

	QSqlQuery query;

	outTotalUnits = outTotalTime = 0;

	QString str ("SELECT SUM (WTU), SUM(Worktime) FROM Activity WHERE DATE='%1';");

	if (not query.exec (str.arg (inDate.toString (Qt::ISODate))))
	{
		QMessageBox::critical (NULL, APPNAME,
			QString ("Cannot get stats:\n%1").arg (QSqlDatabase::database ().lastError ().databaseText ()));
	}
	else
	{
		if (query.next ())
		{
			outTotalUnits = query.value (0).toInt ();		// Total minutes
			outTotalTime = query.value (1).toInt ();		// Total units
		}
	}
}


bool
CDbOperations::createDatabase ()
{
	Q_ASSERT (sOps != nullptr);

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

