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

#include <QSharedMemory>

#include "CWindowSticker.h"
#include "CTimerWindow.h"
#include "CSounder.h"


int
main (int argc, char *argv [])
{
	QApplication instance (argc, argv);
	
	QSharedMemory sharedMemory;
	
	sharedMemory.setKey (APPNAME "_ID_SMEM");
	
	if (sharedMemory.attach ()  ||  not sharedMemory.create (1))
	{
		return -1;		// exit, process is already running
	}
	
	CSounder::initialize ();
	
	QApplication::setQuitOnLastWindowClosed (false);
	
	CTimerWindow win;
	
	new CWindowSticker (&win, CWindowSticker::kStickVerticalSides | CWindowSticker::kDragFramelessWindow);
	
	int rc = 0;
	
	if (not win.isQuitRequired ())
	{
		if (win.isHidden ())
		{
			win.hide ();
		}
		else
		{
			win.show ();
		}
		
		rc = instance.exec ();
	}
	
	CSounder::destroy ();

	return rc;
}
